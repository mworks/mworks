//
//  VideoStimulus.cpp
//  VideoStimulus
//
//  Created by Christopher Stawarz on 9/22/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "VideoStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string VideoStimulus::PATH("path");
const std::string VideoStimulus::VOLUME("volume");
const std::string VideoStimulus::LOOP("loop");
const std::string VideoStimulus::REPEATS("repeats");
const std::string VideoStimulus::ENDED("ended");


void VideoStimulus::describeComponent(ComponentInfo &info) {
    VideoStimlusBase::describeComponent(info);
    
    info.setSignature("stimulus/video");
    
    info.addParameter(PATH);
    info.addParameter(VOLUME, "0.0");
    info.addParameter(LOOP, "NO");
    info.addParameter(REPEATS, "0");
    info.addParameter(ENDED, false);
}


VideoStimulus::VideoStimulus(const ParameterValueMap &parameters) :
    VideoStimlusBase(parameters),
    path(variableOrText(parameters[PATH])),
    volume(parameters[VOLUME]),
    loop(registerVariable(parameters[LOOP])),
    repeats(registerVariable(parameters[REPEATS])),
    ended(optionalVariable(parameters[ENDED])),
    playedToEndObserver(nil),
    player(nil),
    videoOutput(nil),
    colorConversionContext(nil),
    colorConvertedTexture(nil),
    colorConvertedTextureSRGBView(nil),
    currentTexture(nil),
    lastVolume(0.0),
    lastOutputItemTime(kCMTimeInvalid)
{
    @autoreleasepool {
        auto observerCallback = [this](NSNotification *note) {
            boost::mutex::scoped_lock locker(stim_lock);
            if (note.object && player && note.object == player.currentItem) {
                handleVideoEnded();
            }
        };
        playedToEndObserver = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification
                                                                                object:nil
                                                                                 queue:nullptr
                                                                            usingBlock:observerCallback];
    }
}


VideoStimulus::~VideoStimulus() {
    @autoreleasepool {
        currentTexture = nil;
        colorConvertedTextureSRGBView = nil;
        colorConvertedTexture = nil;
        colorConversionContext = nil;
        videoOutput = nil;
        player = nil;
        [[NSNotificationCenter defaultCenter] removeObserver:playedToEndObserver];
        playedToEndObserver = nil;
    }
}


bool VideoStimulus::needDraw(boost::shared_ptr<StimulusDisplay> display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    if (!VideoStimlusBase::needDraw(display)) {
        return false;
    }
    
    @autoreleasepool {
        return (checkForNewPixelBuffer(getMetalDisplay(display)) || (videoEnded && !didDrawAfterEnding));
    }
}


Datum VideoStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    auto announceData = VideoStimlusBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "video");
    announceData.addElement(STIM_FILENAME, filePath.string());
    announceData.addElement(VOLUME, lastVolume);
    
    double currentVideoTimeSeconds = -1.0;
    if (CMTIME_IS_NUMERIC(lastOutputItemTime)) {
        currentVideoTimeSeconds = double(lastOutputItemTime.value) / double(lastOutputItemTime.timescale);
    }
    announceData.addElement("current_video_time_seconds", currentVideoTimeSeconds);
    
    return announceData;
}


void VideoStimulus::loadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    BaseImageStimulus::loadMetal(display);
    
    filePath = pathFromParameterValue(path);
    NSURL *fileURL = [NSURL fileURLWithPath:@(filePath.string().c_str())];
    player = [AVPlayer playerWithURL:fileURL];
    player.actionAtItemEnd = AVPlayerActionAtItemEndNone;
    
    videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:@{
        (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
        (NSString *)kCVPixelBufferMetalCompatibilityKey: @(YES)
    }];
    
    AVPlayerItem *item = player.currentItem;
    [item addOutput:videoOutput];
    {
        auto clock = Clock::instance();
        auto deadline = clock->getCurrentTimeUS() + 5000000;  // 5s from now
        while (item.status == AVPlayerItemStatusUnknown) {
            // Don't loop forever
            if (clock->getCurrentTimeUS() >= deadline) {
                throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Timeout while waiting for video file to load");
            }
            clock->sleepMS(100);
        }
    }
    if (item.status == AVPlayerItemStatusFailed) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              "Cannot load video file",
                              item.error.localizedDescription.UTF8String);
    }
    
    {
        CVMetalTextureCacheRef _metalTextureCache = nullptr;
        auto status = CVMetalTextureCacheCreate(kCFAllocatorDefault,
                                                nullptr,
                                                display.getMetalDevice(),
                                                nullptr,
                                                &_metalTextureCache);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create Metal texture cache (error = %d)") % status);
        }
        metalTextureCache = CVMetalTextureCachePtr::created(_metalTextureCache);
    }
    
    if (display.getUseColorManagement()) {
        colorConversionContext = [CIContext contextWithMTLDevice:display.getMetalDevice()];
        colorConvertedTextureColorSpace = CGColorSpacePtr::created(CGColorSpaceCreateWithName(kCGColorSpaceSRGB));
        
        // NOTE: The documentation for presentationSize doesn't specify its units, but in practice
        // it seems to give the dimensions of the video's frames in pixels
        auto expectedSize = item.presentationSize;
        expectedWidth = expectedSize.width;
        expectedHeight = expectedSize.height;
        
        auto textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                    width:expectedWidth
                                                                                   height:expectedHeight
                                                                                mipmapped:NO];
        textureDescriptor.storageMode = MTLStorageModePrivate;
        textureDescriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
        
        colorConvertedTexture = [display.getMetalDevice() newTextureWithDescriptor:textureDescriptor];
        colorConvertedTextureSRGBView = [colorConvertedTexture newTextureViewWithPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    }
}


void VideoStimulus::unloadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    currentTexture = nil;
    currentMetalTexture.reset();
    currentPixelBuffer.reset();
    
    colorConvertedTextureSRGBView = nil;
    colorConvertedTexture = nil;
    colorConvertedTextureColorSpace.reset();
    colorConversionContext = nil;
    
    metalTextureCache.reset();
    
    [player replaceCurrentItemWithPlayerItem:nil];
    videoOutput = nil;
    player = nil;
    
    BaseImageStimulus::unloadMetal(display);
}


bool VideoStimulus::prepareCurrentTexture(MetalDisplay &display) {
    if (videoEnded) {
        didDrawAfterEnding = true;
        return false;
    }
    
    checkForNewPixelBuffer(display);
    if (!currentMetalTexture) {
        return false;
    }
    
    if (!currentTexture) {
        currentTexture = CVMetalTextureGetTexture(currentMetalTexture.get());
        
        if (display.getUseColorManagement()) {
            auto textureColorSpace = CGColorSpacePtr::borrowed(CVImageBufferGetColorSpace(currentPixelBuffer.get()));
            if (textureColorSpace) {
                if (currentWidth != expectedWidth || currentHeight != expectedHeight) {
                    if (!didWarnAboutSizeMismatch) {
                        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                                 "Current video frame size (%lux%lu) does not match expected size (%lux%lu); "
                                 "video output quality may be affected",
                                 currentWidth, currentHeight,
                                 expectedWidth, expectedHeight);
                        didWarnAboutSizeMismatch = true;
                    }
                }
                
                auto image = [CIImage imageWithMTLTexture:currentTexture options:@{
                    kCIImageColorSpace: static_cast<id>(textureColorSpace.get())
                }];
                [colorConversionContext render:image
                                  toMTLTexture:colorConvertedTexture
                                 commandBuffer:display.getCurrentMetalCommandBuffer()
                                        bounds:CGRectMake(0, 0, expectedWidth, expectedHeight)
                                    colorSpace:colorConvertedTextureColorSpace.get()];
                
                currentTexture = colorConvertedTextureSRGBView;
            }
        }
    }
    
    return true;
}


double VideoStimulus::getCurrentAspectRatio() const {
    return double(currentWidth) / double(currentHeight);
}


id<MTLTexture> VideoStimulus::getCurrentTexture() const {
    return currentTexture;
}


void VideoStimulus::startPlaying() {
    @autoreleasepool {
        player.volume = lastVolume = volume->getValue().getFloat();
        lastOutputItemTime = kCMTimeInvalid;
        repeatCount = 0;
        videoEnded = false;
        didDrawAfterEnding = false;
        didWarnAboutSizeMismatch = false;
        [player play];
        VideoStimlusBase::startPlaying();
    }
}


void VideoStimulus::stopPlaying() {
    @autoreleasepool {
        VideoStimlusBase::stopPlaying();
        [player pause];
        [player seekToTime:kCMTimeZero];
    }
}


void VideoStimulus::beginPause() {
    @autoreleasepool {
        VideoStimlusBase::beginPause();
        [player pause];
    }
}


void VideoStimulus::endPause() {
    @autoreleasepool {
        [player play];
        VideoStimlusBase::endPause();
    }
}


bool VideoStimulus::checkForNewPixelBuffer(MetalDisplay &_display) {
#if TARGET_OS_IPHONE
    const auto &display = dynamic_cast<IOSStimulusDisplay &>(_display);
    auto outputItemTime = [videoOutput itemTimeForHostTime:display.getCurrentTargetTimestamp()];
#else
    const auto &display = dynamic_cast<MacOSStimulusDisplay &>(_display);
    auto outputItemTime = [videoOutput itemTimeForCVTimeStamp:display.getCurrentOutputTimeStamp()];
#endif
    if (![videoOutput hasNewPixelBufferForItemTime:outputItemTime]) {
        return false;
    }
    
    auto newPixelBuffer = CVPixelBufferPtr::owned([videoOutput copyPixelBufferForItemTime:outputItemTime
                                                                       itemTimeForDisplay:nullptr]);
    if (!newPixelBuffer) {
        return false;
    }
    
    // We definitely have a new pixel buffer.  Release the old one, so that we don't draw it
    // again if we subsequently fail to get a new Metal texture.
    currentTexture = nil;
    currentMetalTexture.reset();
    currentPixelBuffer.reset();
    
    const auto newWidth = CVPixelBufferGetWidth(newPixelBuffer.get());
    const auto newHeight = CVPixelBufferGetHeight(newPixelBuffer.get());
    
    CVMetalTexturePtr newMetalTexture;
    {
        CVMetalTextureRef _newMetalTexture = nullptr;
        auto status = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                metalTextureCache.get(),
                                                                newPixelBuffer.get(),
                                                                nullptr,
                                                                MTLPixelFormatBGRA8Unorm,
                                                                newWidth,
                                                                newHeight,
                                                                0,
                                                                &_newMetalTexture);
        if (status != kCVReturnSuccess) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create Metal texture (error = %d)", status);
            return false;
        }
        newMetalTexture = CVMetalTexturePtr::created(_newMetalTexture);
    }
    
    currentPixelBuffer = std::move(newPixelBuffer);
    currentWidth = newWidth;
    currentHeight = newHeight;
    currentMetalTexture = std::move(newMetalTexture);
    lastOutputItemTime = outputItemTime;
    
    return true;
}


void VideoStimulus::handleVideoEnded() {
    //
    // This method is called from Objective-C code, so we don't need to provide an autorelease pool
    //
    
    repeatCount++;
    if (loop->getValue().getBool() || (repeatCount < repeats->getValue().getInteger())) {
        [player seekToTime:kCMTimeZero];
    } else {
        [player pause];
        videoEnded = true;
        if (ended && !(ended->getValue().getBool())) {
            ended->setValue(true);
        }
    }
}


END_NAMESPACE_MW
