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


using VideoStimlusBase = DynamicStimulusBase<BaseImageStimulus>;


class VideoStimulus : public VideoStimlusBase {
    
public:
    static const std::string PATH;
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
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    
    bool prepareCurrentTexture(MetalDisplay &display) override;
    
    double getCurrentAspectRatio() const override;
    id<MTLTexture> getCurrentTexture() const override;
    
    void startPlaying() override;
    void stopPlaying() override;
    void beginPause() override;
    void endPause() override;
    
    bool checkForNewPixelBuffer(MetalDisplay &display);
    void handleVideoEnded();
    
    const VariablePtr path;
    const VariablePtr volume;
    const VariablePtr loop;
    const VariablePtr repeats;
    const VariablePtr ended;
    
    id<NSObject> playedToEndObserver;
    
    boost::filesystem::path filePath;
    AVPlayer *player;
    AVPlayerItemVideoOutput *videoOutput;
    
    using CVMetalTextureCachePtr = cf::ObjectPtr<CVMetalTextureCacheRef>;
    CVMetalTextureCachePtr metalTextureCache;
    
    CIContext *colorConversionContext;
    std::size_t expectedWidth;
    std::size_t expectedHeight;
    using CGColorSpacePtr = cf::ObjectPtr<CGColorSpaceRef>;
    CGColorSpacePtr colorConvertedTextureColorSpace;
    id<MTLTexture> colorConvertedTexture;
    id<MTLTexture> colorConvertedTextureSRGBView;
    
    using CVPixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
    CVPixelBufferPtr currentPixelBuffer;
    std::size_t currentWidth;
    std::size_t currentHeight;
    
    using CVMetalTexturePtr = cf::ObjectPtr<CVMetalTextureRef>;
    CVMetalTexturePtr currentMetalTexture;
    
    id<MTLTexture> currentTexture;
    
    double lastVolume;
    CMTime lastOutputItemTime;
    ssize_t repeatCount;
    bool videoEnded;
    bool didDrawAfterEnding;
    bool didWarnAboutSizeMismatch;
    
};


END_NAMESPACE_MW


#endif /* VideoStimulus_hpp */
