//
//  VideoStimulus.hpp
//  VideoStimulus
//
//  Created by Christopher Stawarz on 9/22/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef VideoStimulus_hpp
#define VideoStimulus_hpp


BEGIN_NAMESPACE_MW


using VideoStimlusBase = DynamicStimulusBase<BasicTransformStimulus>;


class VideoStimulus : public VideoStimlusBase {
    
public:
    static const std::string VOLUME;
    static const std::string LOOP;
    static const std::string REPEATS;
    static const std::string ENDED;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit VideoStimulus(const ParameterValueMap &parameters);
    ~VideoStimulus();
    
    void load(boost::shared_ptr<StimulusDisplay> display) override;
    void unload(boost::shared_ptr<StimulusDisplay> display) override;
    bool needDraw(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    gl::Shader getVertexShader() const override;
    gl::Shader getFragmentShader() const override;
    
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void destroy(const boost::shared_ptr<StimulusDisplay> &display) override;
    void preDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    void postDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    void startPlaying() override;
    void stopPlaying() override;
    void beginPause() override;
    void endPause() override;
    
    void drawFrame(boost::shared_ptr<StimulusDisplay> display) override;
    
    bool checkForNewPixelBuffer(const boost::shared_ptr<StimulusDisplay> &display);
    bool bindTexture(const boost::shared_ptr<StimulusDisplay> &display);
    void handleVideoEnded();
    
    const VariablePtr path;
    const VariablePtr volume;
    const VariablePtr loop;
    const VariablePtr repeats;
    VariablePtr ended;
    
    boost::filesystem::path filePath;
    AVPlayer *player;
    NSDictionary *pixelBufferAttributes;
    AVPlayerItemVideoOutput *videoOutput;
    double lastVolume;
    CMTime lastOutputItemTime;
    id<NSObject> playedToEndObserver;
    ssize_t repeatCount;
    bool videoEnded;
    bool didDrawAfterEnding;
    
    using PixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
    PixelBufferPtr pixelBuffer;
    PixelBufferPtr convertedPixelBuffer;
    std::size_t pixelBufferWidth;
    std::size_t pixelBufferHeight;
    double aspectRatio;
    bool textureReady;
    
    CIContext *colorConversionContext;
    
#if TARGET_OS_IPHONE
    static constexpr GLenum textureTarget = GL_TEXTURE_2D;
#else
    static constexpr GLenum textureTarget = GL_TEXTURE_RECTANGLE;
#endif
    GLuint texture = 0;
    GLint alphaUniformLocation = -1;
    GLuint texCoordsBuffer = 0;
    
};


END_NAMESPACE_MW


#endif /* VideoStimulus_hpp */



























