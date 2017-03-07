//
//  VideoStimulus.cpp
//  VideoStimulus
//
//  Created by Christopher Stawarz on 9/22/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "VideoStimulus.hpp"

#if TARGET_OS_IPHONE
#  include <OpenGLES/ES3/glext.h>
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
#if TARGET_OS_IPHONE
            (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
            (NSString *)kCVPixelBufferOpenGLESCompatibilityKey: @(YES)
#else
            (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32ARGB),
            (NSString *)kCVPixelBufferOpenGLCompatibilityKey: @(YES)
#endif
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
     uniform float alpha;
     uniform sampler2D videoTexture;
     #ifndef GL_ES
     uniform sampler2DRect videoTextureRect;
     uniform bool useTextureRect;
     #endif
     
     in vec2 varyingTexCoords;
     
     out vec4 fragColor;
     
     void main() {
         vec4 videoColor;
         #ifdef GL_ES
         videoColor = texture(videoTexture, varyingTexCoords);
         #else
         if (useTextureRect) {
             videoColor = texture(videoTextureRect, varyingTexCoords);
         } else {
             videoColor = texture(videoTexture, varyingTexCoords);
         }
         #endif
         fragColor.rgb = videoColor.rgb;
         fragColor.a = alpha * videoColor.a;
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


void VideoStimulus::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    BasicTransformStimulus::prepare(display);
    
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
    
#if TARGET_OS_IPHONE
    CVOpenGLESTextureCacheRef newTextureCache = nullptr;
    auto status = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                               nullptr,
                                               EAGLContext.currentContext,
                                               nullptr,
                                               &newTextureCache);
#else
    auto context = CGLGetCurrentContext();
    CVOpenGLTextureCacheRef newTextureCache = nullptr;
    auto status = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                             nullptr,
                                             context,
                                             CGLGetPixelFormat(context),
                                             nullptr,
                                             &newTextureCache);
#endif
    if (status != kCVReturnSuccess) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              (boost::format("Cannot create OpenGL texture cache (error = %d)") % status).str());
    }
    textureCache = TextureCachePtr::owned(newTextureCache);
    
    alphaUniformLocation = glGetUniformLocation(program, "alpha");
#if MWORKS_OPENGL_ES
    glUniform1i(glGetUniformLocation(program, "videoTexture"), 0);
#else
    videoTextureUniformLocation = glGetUniformLocation(program, "videoTexture");
    videoTextureRectUniformLocation = glGetUniformLocation(program, "videoTextureRect");
    useTextureRectUniformLocation = glGetUniformLocation(program, "useTextureRect");
#endif
    
    glGenBuffers(1, &texCoordsBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(texCoordsBuffer);
    GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordsAttribLocation);
    glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void VideoStimulus::destroy(const boost::shared_ptr<StimulusDisplay> &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    glDeleteBuffers(1, &texCoordsBuffer);
    
    texture.reset();
    pixelBuffer.reset();
    textureCache.reset();
    
    [player replaceCurrentItemWithPlayerItem:nil];
    
    BasicTransformStimulus::destroy(display);
}


void VideoStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::preDraw(display);
    
    checkForNewPixelBuffer(display);
    if (!(pixelBuffer && bindTexture())) {
        return;
    }
    
    glUniform1f(alphaUniformLocation, current_alpha);
    
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void VideoStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glDisable(GL_BLEND);
    
    if (texture) {
        glBindTexture(textureTarget, 0);
    }
    
    BasicTransformStimulus::postDraw(display);
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


bool VideoStimulus::checkForNewPixelBuffer(const boost::shared_ptr<StimulusDisplay> &_display) {
#if TARGET_OS_IPHONE
    const auto &display = boost::dynamic_pointer_cast<IOSStimulusDisplay>(_display);
    auto outputItemTime = [videoOutput itemTimeForHostTime:display->getCurrentTargetTimestamp()];
#else
    const auto &display = boost::dynamic_pointer_cast<MacOSStimulusDisplay>(_display);
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
    
    pixelBuffer = newPixelBuffer;
    lastOutputItemTime = outputItemTime;
    texture.reset();
    
    const auto newAspectRatio = (double(CVPixelBufferGetWidth(pixelBuffer.get())) /
                                 double(CVPixelBufferGetHeight(pixelBuffer.get())));
    if (newAspectRatio != aspectRatio) {
        aspectRatio = newAspectRatio;
        auto vertexPositions = ImageStimulus::getVertexPositions(aspectRatio);
        gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(vertexPositionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.data(), GL_STATIC_DRAW);
    }
    
    return true;
}


bool VideoStimulus::bindTexture() {
    if (texture) {
        glBindTexture(textureTarget, textureName);
        return true;
    }
    
#if TARGET_OS_IPHONE
    CVOpenGLESTextureCacheFlush(textureCache.get(), 0);
#else
    CVOpenGLTextureCacheFlush(textureCache.get(), 0);
#endif
    
    {
#if TARGET_OS_IPHONE
        CVOpenGLESTextureRef newTexture = nullptr;
        //
        // On iOS, we need to use GL_BGRA_EXT as the texture format, which is defined by the
        // APPLE_texture_format_BGRA8888 extension.  Confusingly, this requires the internal format
        // to be GL_RGBA (or GL_RGBA8):
        //
        // https://www.khronos.org/registry/OpenGL/extensions/APPLE/APPLE_texture_format_BGRA8888.txt
        //
        //
        auto status = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                   textureCache.get(),
                                                                   pixelBuffer.get(),
                                                                   nullptr,
                                                                   GL_TEXTURE_2D,
                                                                   GL_RGBA8,
                                                                   CVPixelBufferGetWidth(pixelBuffer.get()),
                                                                   CVPixelBufferGetHeight(pixelBuffer.get()),
                                                                   GL_BGRA_EXT,
                                                                   GL_UNSIGNED_BYTE,
                                                                   0,
                                                                   &newTexture);
#else
        CVOpenGLTextureRef newTexture = nullptr;
        auto status = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                 textureCache.get(),
                                                                 pixelBuffer.get(),
                                                                 nullptr,
                                                                 &newTexture);
#endif
        if (status != kCVReturnSuccess) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL texture (error = %d)", status);
            return false;
        }
        
        texture = TexturePtr::owned(newTexture);
#if TARGET_OS_IPHONE
        textureTarget = CVOpenGLESTextureGetTarget(newTexture);
        textureName = CVOpenGLESTextureGetName(newTexture);
#else
        textureTarget = CVOpenGLTextureGetTarget(newTexture);
        textureName = CVOpenGLTextureGetName(newTexture);
#endif
    }
    
    glBindTexture(textureTarget, textureName);
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
#if !MWORKS_OPENGL_ES
    if (GL_TEXTURE_RECTANGLE == textureTarget) {
        glUniform1i(videoTextureUniformLocation, 1);
        glUniform1i(videoTextureRectUniformLocation, 0);
        glUniform1i(useTextureRectUniformLocation, true);
    } else {
        glUniform1i(videoTextureUniformLocation, 0);
        glUniform1i(videoTextureRectUniformLocation, 1);
        glUniform1i(useTextureRectUniformLocation, false);
    }
#endif
    
    VertexPositionArray texCoords;
#if TARGET_OS_IPHONE
    CVOpenGLESTextureGetCleanTexCoords(texture.get(),
#else
    CVOpenGLTextureGetCleanTexCoords(texture.get(),
#endif
                                     &(texCoords.at(0 * componentsPerVertex)),
                                     &(texCoords.at(1 * componentsPerVertex)),
                                     &(texCoords.at(3 * componentsPerVertex)),
                                     &(texCoords.at(2 * componentsPerVertex)));
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(texCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STREAM_DRAW);
    
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



























