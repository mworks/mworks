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
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void preDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    void drawFrame(boost::shared_ptr<StimulusDisplay> display) override;
    
    double getAspectRatio() const override;
    
    const VariablePtr pixelBufferFormat;
    const VariablePtr pixelBufferWidth;
    const VariablePtr pixelBufferHeight;
    const std::string pixelBufferExpr;
    const VariablePtr elapsedTime;
    
    PythonStringEvaluator pixelBufferExprEvaluator;
    
    std::string format;
    int width;
    int height;
    std::size_t expectedBytes;
    
    GLenum textureFormat = 0;
    GLint textureInternalFormat = 0;
    GLenum textureDataType = 0;
    GLint textureDataAlignment = 0;
    
};


END_NAMESPACE_MW_PYTHON


#endif /* PythonImage_hpp */
