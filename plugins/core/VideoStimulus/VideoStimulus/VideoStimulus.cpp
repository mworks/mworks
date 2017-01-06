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
    
    return std::move(announceData);
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
     uniform sampler2DRect videoTextureRect;
     uniform bool useTextureRect;
     
     in vec2 varyingTexCoords;
     
     out vec4 fragColor;
     
     void main() {
         vec4 videoColor;
         if (useTextureRect) {
             videoColor = texture(videoTextureRect, varyingTexCoords);
         } else {
             videoColor = texture(videoTexture, varyingTexCoords);
         }
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
    textureCache = CVOpenGLTextureCachePtr::owned(newTextureCache);
    
    alphaUniformLocation = glGetUniformLocation(program, "alpha");
    videoTextureUniformLocation = glGetUniformLocation(program, "videoTexture");
    videoTextureRectUniformLocation = glGetUniformLocation(program, "videoTextureRect");
    useTextureRectUniformLocation = glGetUniformLocation(program, "useTextureRect");
    
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
    
    CVOpenGLTextureCacheFlush(textureCache.get(), 0);
    
    {
        CVOpenGLTextureRef newTexture = nullptr;
        auto status = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                 textureCache.get(),
                                                                 pixelBuffer.get(),
                                                                 nullptr,
                                                                 &newTexture);
        if (status != kCVReturnSuccess) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL texture (error = %d)", status);
            return false;
        }
        
        texture = CVOpenGLTexturePtr::owned(newTexture);
        textureTarget = CVOpenGLTextureGetTarget(newTexture);
        textureName = CVOpenGLTextureGetName(newTexture);
    }
    
    glBindTexture(textureTarget, textureName);
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    if (GL_TEXTURE_RECTANGLE == textureTarget) {
        glUniform1i(videoTextureUniformLocation, 1);
        glUniform1i(videoTextureRectUniformLocation, 0);
        glUniform1i(useTextureRectUniformLocation, true);
    } else {
        glUniform1i(videoTextureUniformLocation, 0);
        glUniform1i(videoTextureRectUniformLocation, 1);
        glUniform1i(useTextureRectUniformLocation, false);
    }
    
    VertexPositionArray texCoords;
    CVOpenGLTextureGetCleanTexCoords(texture.get(),
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



























