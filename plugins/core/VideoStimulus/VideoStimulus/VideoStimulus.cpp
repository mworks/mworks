//
//  VideoStimulus.cpp
//  VideoStimulus
//
//  Created by Christopher Stawarz on 9/22/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "VideoStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string VideoStimulus::VOLUME("volume");
const std::string VideoStimulus::LOOP("loop");
const std::string VideoStimulus::REPEATS("repeats");
const std::string VideoStimulus::ENDED("ended");


void VideoStimulus::describeComponent(ComponentInfo &info) {
    VideoStimlusBase::describeComponent(info);
    
    info.setSignature("stimulus/video");
    
    info.addParameter(ImageStimulus::PATH);
    info.addParameter(VOLUME, "0.0");
    info.addParameter(LOOP, "NO");
    info.addParameter(REPEATS, "0");
    info.addParameter(ENDED, false);
}


VideoStimulus::VideoStimulus(const ParameterValueMap &parameters) :
    VideoStimlusBase(parameters),
    filePath(parameters[ImageStimulus::PATH]),
    volume(parameters[VOLUME]),
    loop(registerVariable(parameters[LOOP])),
    repeats(registerVariable(parameters[REPEATS])),
    player(nil),
    videoOutput(nil),
    lastVolume(0.0),
    lastOutputItemTime(kCMTimeInvalid),
    playedToEndObserver(nil),
    repeatCount(0),
    videoEnded(false),
    didDrawAfterEnding(false),
    aspectRatio(0.0)
{
    if (!(parameters[ENDED].empty())) {
        ended = VariablePtr(parameters[ENDED]);
    }
    
    @autoreleasepool {
        player = [[AVPlayer alloc] init];
        player.actionAtItemEnd = AVPlayerActionAtItemEndNone;
        
        NSDictionary *pixelBufferAttributes = @{
            (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32ARGB),
            (NSString *)kCVPixelBufferOpenGLCompatibilityKey: @(YES)
        };
        videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:pixelBufferAttributes];
        
        auto observerCallback = [this](NSNotification *note) {
            boost::mutex::scoped_lock locker(stim_lock);
            if (note.object && note.object == player.currentItem) {
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
    [[NSNotificationCenter defaultCenter] removeObserver:playedToEndObserver];
    [videoOutput release];
    [player release];
}


void VideoStimulus::load(boost::shared_ptr<StimulusDisplay> display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    if (loaded) {
        return;
    }
    
    @autoreleasepool {
        NSURL *fileURL = [NSURL fileURLWithPath:@(filePath.string().c_str())];
        AVPlayerItem *item = [AVPlayerItem playerItemWithURL:fileURL];
        [item addOutput:videoOutput];
        [player replaceCurrentItemWithPlayerItem:item];
        
        while (item.status == AVPlayerItemStatusUnknown) {
            Clock::instance()->sleepMS(100);
        }
        if (item.status == AVPlayerItemStatusFailed) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  "Cannot load video file",
                                  item.error.localizedDescription.UTF8String);
        }
    }
    
    for (int i = 0; i < display->getNContexts(); i++) {
        OpenGLContextLock ctxLock = display->setCurrent(i);
        auto context = CGLGetCurrentContext();
        CVOpenGLTextureCacheRef newTextureCache = nullptr;
        auto status = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                                 nullptr,
                                                 context,
                                                 CGLGetPixelFormat(context),
                                                 nullptr,
                                                 &newTextureCache);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  (boost::format("Cannot create OpenGL texture cache (error = %d)") % status).str());
        }
        textureCache[i] = CVOpenGLTextureCachePtr::owned(newTextureCache);
    }
    
    loaded = true;
}


void VideoStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    if (!loaded) {
        return;
    }
    
    texture.clear();
    pixelBuffer = CVPixelBufferPtr();
    textureCache.clear();
    
    [player replaceCurrentItemWithPlayerItem:nil];
    
    loaded = false;
}


bool VideoStimulus::needDraw(boost::shared_ptr<StimulusDisplay> display) {
    boost::mutex::scoped_lock locker(stim_lock);
    return (VideoStimlusBase::needDraw(display) &&
            (checkForNewPixelBuffer(display) || (videoEnded && !didDrawAfterEnding)));
}


Datum VideoStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    Datum announceData = VideoStimlusBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "video");
    announceData.addElement(STIM_FILENAME, filePath.string());
    announceData.addElement(VOLUME, lastVolume);
    
    double currentVideoTimeSeconds = -1.0;
    if (CMTIME_IS_NUMERIC(lastOutputItemTime)) {
        currentVideoTimeSeconds = double(lastOutputItemTime.value) / double(lastOutputItemTime.timescale);
    }
    announceData.addElement("current_video_time_seconds", currentVideoTimeSeconds);
    
    return std::move(announceData);
}


void VideoStimulus::startPlaying() {
    player.volume = lastVolume = volume->getValue().getFloat();
    lastOutputItemTime = kCMTimeInvalid;
    repeatCount = 0;
    videoEnded = false;
    [player play];
    VideoStimlusBase::startPlaying();
}


void VideoStimulus::stopPlaying() {
    VideoStimlusBase::stopPlaying();
    [player pause];
    [player seekToTime:kCMTimeZero];
}


void VideoStimulus::beginPause() {
    VideoStimlusBase::beginPause();
    [player pause];
}


void VideoStimulus::endPause() {
    [player play];
    VideoStimlusBase::endPause();
}


void VideoStimulus::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    if (videoEnded) {
        didDrawAfterEnding = true;
        return;
    }
    BasicTransformStimulus::draw(display);
}


void VideoStimulus::drawInUnitSquare(boost::shared_ptr<StimulusDisplay> display) {
    const auto currentContextIndex = display->getCurrentContextIndex();
    
    if (currentContextIndex == 0) {
        checkForNewPixelBuffer(display);
    }
    if (!(pixelBuffer && prepareTexture(currentContextIndex))) {
        return;
    }
    
    auto &currentTexture = texture.at(currentContextIndex);
    auto currentTextureTarget = CVOpenGLTextureGetTarget(currentTexture.get());
    
    glEnable(currentTextureTarget);
    glBindTexture(currentTextureTarget, CVOpenGLTextureGetName(currentTexture.get()));
    glTexParameteri(currentTextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(currentTextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(currentTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(currentTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0, 1.0, 1.0, current_alpha);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glVertexPointer(numCoordsPerVertex, GL_DOUBLE, 0, vertexCoords.data());
    glTexCoordPointer(numCoordsPerVertex, GL_FLOAT, 0, textureCoords.at(currentContextIndex).data());
    glDrawArrays(GL_QUADS, 0, numVertices);
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    
    glDisable(GL_BLEND);
    
    glBindTexture(currentTextureTarget, 0);
    glDisable(currentTextureTarget);
}


bool VideoStimulus::checkForNewPixelBuffer(const boost::shared_ptr<StimulusDisplay> &display) {
    auto outputItemTime = [videoOutput itemTimeForCVTimeStamp:display->getCurrentOutputTimeStamp()];
    if (![videoOutput hasNewPixelBufferForItemTime:outputItemTime]) {
        return false;
    }
    
    auto newPixelBuffer = CVPixelBufferPtr::owned([videoOutput copyPixelBufferForItemTime:outputItemTime
                                                                       itemTimeForDisplay:nullptr]);
    if (!newPixelBuffer) {
        return false;
    }
    
    pixelBuffer = newPixelBuffer;
    lastOutputItemTime = outputItemTime;
    texture.clear();
    
    const auto newAspectRatio = (double(CVPixelBufferGetWidth(pixelBuffer.get())) /
                                 double(CVPixelBufferGetHeight(pixelBuffer.get())));
    if (newAspectRatio != aspectRatio) {
        aspectRatio = newAspectRatio;
        if (aspectRatio > 1.0) {
            vertexCoords = {
                0.0, (0.5 - 0.5/aspectRatio),
                1.0, (0.5 - 0.5/aspectRatio),
                1.0, (0.5 - 0.5/aspectRatio) + 1.0/aspectRatio,
                0.0, (0.5 - 0.5/aspectRatio) + 1.0/aspectRatio
            };
        } else {
            vertexCoords = {
                (1.0 - aspectRatio)/2.0, 0.0,
                (1.0 - aspectRatio)/2.0 + aspectRatio, 0.0,
                (1.0 - aspectRatio)/2.0 + aspectRatio, 1.0,
                (1.0 - aspectRatio)/2.0, 1.0
            };
        }
    }
    
    return true;
}


bool VideoStimulus::prepareTexture(int currentContextIndex) {
    if (texture.find(currentContextIndex) != texture.end()) {
        return true;
    }
    
    auto &currentTextureCache = textureCache.at(currentContextIndex);
    CVOpenGLTextureCacheFlush(currentTextureCache.get(), 0);
    
    CVOpenGLTextureRef newTexture = nullptr;
    auto status = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                             currentTextureCache.get(),
                                                             pixelBuffer.get(),
                                                             nullptr,
                                                             &newTexture);
    if (status != kCVReturnSuccess) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL texture (error = %d)", status);
        return false;
    }
    
    texture[currentContextIndex] = CVOpenGLTexturePtr::owned(newTexture);
    
    auto &texCoords = textureCoords[currentContextIndex];
    CVOpenGLTextureGetCleanTexCoords(newTexture,
                                     &(texCoords.at(0 * numCoordsPerVertex)),
                                     &(texCoords.at(1 * numCoordsPerVertex)),
                                     &(texCoords.at(2 * numCoordsPerVertex)),
                                     &(texCoords.at(3 * numCoordsPerVertex)));
    
    return true;
}


void VideoStimulus::handleVideoEnded() {
    repeatCount++;
    if (loop->getValue().getBool() || (repeatCount < repeats->getValue().getInteger())) {
        [player seekToTime:kCMTimeZero];
    } else {
        [player pause];
        videoEnded = true;
        didDrawAfterEnding = false;
        if (ended && !(ended->getValue().getBool())) {
            ended->setValue(true);
        }
    }
}


END_NAMESPACE_MW



























