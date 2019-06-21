//
//  VideoStimulus.cpp
//  VideoStimulus
//
//  Created by Christopher Stawarz on 9/22/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "VideoStimulus.hpp"

#if TARGET_OS_IPHONE
#  include <OpenGLES/EAGLIOSurface.h>
#else
#  include <OpenGL/CGLIOSurface.h>
#endif


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
    path(variableOrText(parameters[ImageStimulus::PATH])),
    volume(parameters[VOLUME]),
    loop(registerVariable(parameters[LOOP])),
    repeats(registerVariable(parameters[REPEATS])),
    player(nil),
    pixelBufferAttributes(nil),
    videoOutput(nil),
    lastVolume(0.0),
    lastOutputItemTime(kCMTimeInvalid),
    playedToEndObserver(nil),
    repeatCount(0),
    videoEnded(false),
    didDrawAfterEnding(false),
    pixelBufferWidth(0),
    pixelBufferHeight(0),
    aspectRatio(0.0),
    textureReady(false),
    colorConversionContext(nil)
{
    @autoreleasepool {
        if (!(parameters[ENDED].empty())) {
            ended = VariablePtr(parameters[ENDED]);
        }
        
        pixelBufferAttributes = @{
            (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
            (NSString *)kCVPixelBufferIOSurfacePropertiesKey: @{},  // Use IOSurface with default properties
#if TARGET_OS_IPHONE
            (NSString *)kCVPixelBufferIOSurfaceOpenGLESTextureCompatibilityKey: @(YES)
#else
            (NSString *)kCVPixelBufferIOSurfaceOpenGLTextureCompatibilityKey: @(YES)
#endif
        };
        
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
        
        colorConversionContext = [CIContext context];
    }
}


VideoStimulus::~VideoStimulus() {
    @autoreleasepool {
        colorConversionContext = nil;
        [[NSNotificationCenter defaultCenter] removeObserver:playedToEndObserver];
        videoOutput = nil;
        pixelBufferAttributes = nil;
        player = nil;
    }
}


void VideoStimulus::load(boost::shared_ptr<StimulusDisplay> display) {
    @autoreleasepool {
        boost::mutex::scoped_lock locker(stim_lock);
        
        if (loaded)
            return;
        
        //
        // Load the video file here, rather than in prepare(), so that we don't block the stimulus display
        // thread by holding the OpenGL context lock
        //
        
        filePath = pathFromParameterValue(path);
        NSURL *fileURL = [NSURL fileURLWithPath:@(filePath.string().c_str())];
        player = [[AVPlayer alloc] initWithURL:fileURL];
        player.actionAtItemEnd = AVPlayerActionAtItemEndNone;
        
        videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:pixelBufferAttributes];
        
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
        
        BasicTransformStimulus::load(display);
    }
}


void VideoStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    @autoreleasepool {
        boost::mutex::scoped_lock locker(stim_lock);
        
        if (!loaded)
            return;
        
        BasicTransformStimulus::unload(display);
        
        //
        // Unload non-OpenGL resources here, rather than in destroy(), so that we don't block the stimulus display
        // thread by holding the OpenGL context lock
        //
        
        convertedPixelBuffer.reset();
        pixelBuffer.reset();
        
        [player replaceCurrentItemWithPlayerItem:nil];
        
        // Reusing these AVFoundation objects causes problems, so destroy them and
        // create new ones when needed
        videoOutput = nil;
        player = nil;
        
        // Need to reset this to zero, otherwise we won't generate new vertex positions
        // if we load a new video with the same aspect ratio as the old one
        aspectRatio = 0.0;
        
        // Need to reset these to zero, otherwise we won't generate new texture coordinates
        // if we load a new video with the same frame dimensions as the old one
        pixelBufferWidth = 0;
        pixelBufferHeight = 0;
    }
}


bool VideoStimulus::needDraw(boost::shared_ptr<StimulusDisplay> display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    if (!VideoStimlusBase::needDraw(display)) {
        return false;
    }
    
    auto ctxLock = display->setCurrent(0);
    return (checkForNewPixelBuffer(display) || (videoEnded && !didDrawAfterEnding));
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
    
    return announceData;
}


gl::Shader VideoStimulus::getVertexShader() const {
    static const std::string source
    (R"(
     uniform mat4 mvpMatrix;
     in vec4 vertexPosition;
     in vec2 texCoords;
     out vec2 varyingTexCoords;
     
     void main() {
         gl_Position = mvpMatrix * vertexPosition;
         varyingTexCoords = texCoords;
     }
     )");
    
    return gl::createShader(GL_VERTEX_SHADER, source);
}


gl::Shader VideoStimulus::getFragmentShader() const {
    static const std::string source
    (R"(
     #ifdef GL_ES
     uniform sampler2D videoTexture;
     #else
     uniform sampler2DRect videoTexture;
     #endif
     uniform float alpha;
     
     in vec2 varyingTexCoords;
     
     out vec4 fragColor;
     
     void main() {
         vec4 videoColor = texture(videoTexture, varyingTexCoords);
         fragColor.rgb = videoColor.rgb;
         fragColor.a = alpha * videoColor.a;
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


void VideoStimulus::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::prepare(display);
    
    glGenTextures(1, &texture);
    
    glUniform1i(glGetUniformLocation(program, "videoTexture"), 0);
    alphaUniformLocation = glGetUniformLocation(program, "alpha");
    
    glGenBuffers(1, &texCoordsBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(texCoordsBuffer);
#if TARGET_OS_IPHONE
    VertexPositionArray texCoords {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STATIC_DRAW);
#endif
    GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordsAttribLocation);
    glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void VideoStimulus::destroy(const boost::shared_ptr<StimulusDisplay> &display) {
    glDeleteBuffers(1, &texCoordsBuffer);
    glDeleteTextures(1, &texture);
    
    BasicTransformStimulus::destroy(display);
}


void VideoStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::preDraw(display);
    
    checkForNewPixelBuffer(display);
    if (!(pixelBuffer && bindTexture(display))) {
        return;
    }
    
    glUniform1f(alphaUniformLocation, current_alpha);
}


void VideoStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glBindTexture(textureTarget, 0);
    BasicTransformStimulus::postDraw(display);
}


void VideoStimulus::startPlaying() {
    @autoreleasepool {
        player.volume = lastVolume = volume->getValue().getFloat();
        lastOutputItemTime = kCMTimeInvalid;
        repeatCount = 0;
        videoEnded = false;
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


void VideoStimulus::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    if (videoEnded) {
        didDrawAfterEnding = true;
        return;
    }
    BasicTransformStimulus::draw(display);
}


bool VideoStimulus::checkForNewPixelBuffer(const boost::shared_ptr<StimulusDisplay> &_display) {
    @autoreleasepool {
#if TARGET_OS_IPHONE
        const auto display = boost::dynamic_pointer_cast<IOSStimulusDisplay>(_display);
        auto outputItemTime = [videoOutput itemTimeForHostTime:display->getCurrentTargetTimestamp()];
#else
        const auto display = boost::dynamic_pointer_cast<MacOSStimulusDisplay>(_display);
        auto outputItemTime = [videoOutput itemTimeForCVTimeStamp:display->getCurrentOutputTimeStamp()];
#endif
        if (![videoOutput hasNewPixelBufferForItemTime:outputItemTime]) {
            return false;
        }
        
        auto newPixelBuffer = PixelBufferPtr::owned([videoOutput copyPixelBufferForItemTime:outputItemTime
                                                                         itemTimeForDisplay:nullptr]);
        if (!newPixelBuffer) {
            return false;
        }
        
        const auto newWidth = CVPixelBufferGetWidth(newPixelBuffer.get());
        const auto newHeight = CVPixelBufferGetHeight(newPixelBuffer.get());
        
        //
        // If the stimulus display is color managed, use CoreImage to convert the new pixel buffer to sRGB
        //
        if (display->getUseColorManagement()) {
            if (newWidth != pixelBufferWidth || newHeight != pixelBufferHeight) {
                CVPixelBufferRef _convertedPixelBuffer = nullptr;
                auto status = CVPixelBufferCreate(kCFAllocatorDefault,
                                                  newWidth,
                                                  newHeight,
                                                  kCVPixelFormatType_32BGRA,
                                                  (__bridge CFDictionaryRef)pixelBufferAttributes,
                                                  &_convertedPixelBuffer);
                if (status != kCVReturnSuccess) {
                    merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create pixel buffer (error = %d)", status);
                    return false;
                }
                convertedPixelBuffer = PixelBufferPtr::owned(_convertedPixelBuffer);
            }
            
            CIImage *image = [CIImage imageWithCVPixelBuffer:newPixelBuffer.get()];
            auto colorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(CGColorSpaceCreateWithName(kCGColorSpaceSRGB));
            [colorConversionContext render:image
                           toCVPixelBuffer:convertedPixelBuffer.get()
                                    bounds:CGRectMake(0, 0, newWidth, newHeight)
                                colorSpace:colorSpace.get()];
            
            newPixelBuffer = convertedPixelBuffer;
        }
        
        pixelBuffer = newPixelBuffer;
        lastOutputItemTime = outputItemTime;
        textureReady = false;
        
        const auto newAspectRatio = double(newWidth) / double(newHeight);
        if (newAspectRatio != aspectRatio) {
            aspectRatio = newAspectRatio;
            VertexPositionArray vertexPositions;
            if (fullscreen) {
                vertexPositions = BasicTransformStimulus::getVertexPositions();
            } else {
                vertexPositions = ImageStimulus::getVertexPositions(aspectRatio);
            }
            gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(vertexPositionBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.data(), GL_STATIC_DRAW);
        }
        
        if (newWidth != pixelBufferWidth || newHeight != pixelBufferHeight) {
            pixelBufferWidth = newWidth;
            pixelBufferHeight = newHeight;
#if !TARGET_OS_IPHONE
            VertexPositionArray texCoords {
                0.0f, GLfloat(pixelBufferHeight),
                GLfloat(pixelBufferWidth), GLfloat(pixelBufferHeight),
                0.0f, 0.0f,
                GLfloat(pixelBufferWidth), 0.0f
            };
            gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(texCoordsBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STREAM_DRAW);
#endif
        }
        
        return true;
    }
}


bool VideoStimulus::bindTexture(const boost::shared_ptr<StimulusDisplay> &display) {
    @autoreleasepool {
        glBindTexture(textureTarget, texture);
        
        if (textureReady) {
            return true;
        }
        
#if TARGET_OS_IPHONE
        //
        // On iOS, we need to use GL_BGRA_EXT as the texture format, which is defined by the
        // APPLE_texture_format_BGRA8888 extension:
        //
        // https://www.khronos.org/registry/OpenGL/extensions/APPLE/APPLE_texture_format_BGRA8888.txt
        //
        if (![EAGLContext.currentContext texImageIOSurface:CVPixelBufferGetIOSurface(pixelBuffer.get())
                                                    target:textureTarget
                                            internalFormat:(display->getUseColorManagement() ? GL_SRGB8_ALPHA8 : GL_RGBA8)
                                                     width:pixelBufferWidth
                                                    height:pixelBufferHeight
                                                    format:GL_BGRA_EXT
                                                      type:GL_UNSIGNED_BYTE
                                                     plane:0])
        {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL ES texture");
            return false;
        }
#else
        auto status = CGLTexImageIOSurface2D(CGLGetCurrentContext(),
                                             textureTarget,
                                             (display->getUseColorManagement() ? GL_SRGB8_ALPHA8 : GL_RGBA8),
                                             pixelBufferWidth,
                                             pixelBufferHeight,
                                             GL_BGRA,
                                             GL_UNSIGNED_INT_8_8_8_8_REV,
                                             CVPixelBufferGetIOSurface(pixelBuffer.get()),
                                             0);
        if (status != kCGLNoError) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL texture (error = %d)", status);
            return false;
        }
#endif
        
        glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        textureReady = true;
        
        return true;
    }
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
        didDrawAfterEnding = false;
        if (ended && !(ended->getValue().getBool())) {
            ended->setValue(true);
        }
    }
}


END_NAMESPACE_MW



























