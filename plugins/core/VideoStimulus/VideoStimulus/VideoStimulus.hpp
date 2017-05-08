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
    bool bindTexture();
    void handleVideoEnded();
    
    const VariablePtr path;
    const VariablePtr volume;
    const VariablePtr loop;
    const VariablePtr repeats;
    VariablePtr ended;
    
    boost::filesystem::path filePath;
    AVPlayer *player;
    AVPlayerItemVideoOutput *videoOutput;
    double lastVolume;
    CMTime lastOutputItemTime;
    id<NSObject> playedToEndObserver;
    ssize_t repeatCount;
    bool videoEnded;
    bool didDrawAfterEnding;
    
#if TARGET_OS_IPHONE
    using TextureCachePtr = cf::ObjectPtr<CVOpenGLESTextureCacheRef>;
#else
    using TextureCachePtr = cf::ObjectPtr<CVOpenGLTextureCacheRef>;
#endif
    TextureCachePtr textureCache;
    
    using PixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
    PixelBufferPtr pixelBuffer;
    double aspectRatio;
    
#if TARGET_OS_IPHONE
    using TexturePtr = cf::ObjectPtr<CVOpenGLESTextureRef>;
#else
    using TexturePtr = cf::ObjectPtr<CVOpenGLTextureRef>;
#endif
    TexturePtr texture;
    GLenum textureTarget;
    GLuint textureName;
    
    GLint alphaUniformLocation = -1;
#if !MWORKS_OPENGL_ES
    GLint videoTextureUniformLocation = -1;
    GLint videoTextureRectUniformLocation = -1;
    GLint useTextureRectUniformLocation = -1;
#endif
    GLuint texCoordsBuffer = 0;
    
};


END_NAMESPACE_MW


#endif /* VideoStimulus_hpp */



























