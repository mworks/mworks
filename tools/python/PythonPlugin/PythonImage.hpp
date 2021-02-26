//
//  PythonImage.hpp
//  PythonTools
//
//  Created by Christopher Stawarz on 10/1/20.
//  Copyright © 2020 MWorks Project. All rights reserved.
//

#ifndef PythonImage_hpp
#define PythonImage_hpp

#include "PythonEvaluator.hpp"


BEGIN_NAMESPACE_MW_PYTHON


using PythonImageBase = DynamicStimulusBase<BaseImageStimulus>;


class PythonImage : public PythonImageBase {
    
public:
    static const std::string PIXEL_BUFFER_FORMAT;
    static const std::string PIXEL_BUFFER_WIDTH;
    static const std::string PIXEL_BUFFER_HEIGHT;
    static const std::string PIXEL_BUFFER_EXPR;
    static const std::string ELAPSED_TIME;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit PythonImage(const ParameterValueMap &parameters);
    ~PythonImage();
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    enum class Format {
        RGB8,
        RGBA8,
        RGBA16F
    };
    
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    
    bool prepareCurrentTexture(MetalDisplay &display) override;
    
    double getCurrentAspectRatio() const override;
    id<MTLTexture> getCurrentTexture() const override;
    
    void drawFrame(boost::shared_ptr<StimulusDisplay> display) override;
    
    const VariablePtr pixelBufferFormat;
    const VariablePtr pixelBufferWidth;
    const VariablePtr pixelBufferHeight;
    const std::string pixelBufferExpr;
    const VariablePtr elapsedTime;
    
    PythonStringEvaluator pixelBufferExprEvaluator;
    
    std::string formatName;
    Format format;
    int width;
    int height;
    std::size_t expectedBytes;
    
    NSUInteger textureBytesPerRow;
    MWKTripleBufferedMTLResource<id<MTLTexture>> *texturePool;
    std::unique_ptr<std::uint8_t[]> rgbaData;
    id<MTLTexture> currentTexture;
    
};


END_NAMESPACE_MW_PYTHON


#endif /* PythonImage_hpp */
