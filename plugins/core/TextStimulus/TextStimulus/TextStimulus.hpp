//
//  TextStimulus.hpp
//  TextStimulus
//
//  Created by Christopher Stawarz on 9/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef TextStimulus_hpp
#define TextStimulus_hpp


BEGIN_NAMESPACE_MW


class TextStimulus : public ColoredTransformStimulus {
    
public:
    static const std::string TEXT;
    static const std::string FONT_NAME;
    static const std::string FONT_SIZE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TextStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    gl::Shader getVertexShader() const override;
    gl::Shader getFragmentShader() const override;
    
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void destroy(const boost::shared_ptr<StimulusDisplay> &display) override;
    void preDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    void postDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    void bindTexture(const boost::shared_ptr<StimulusDisplay> &display);
    
    static const VertexPositionArray texCoords;
    
    const VariablePtr text;
    const VariablePtr fontName;
    const VariablePtr fontSize;
    
    GLint viewportWidth, viewportHeight;
    double pixelsPerDegree;
    double pointsPerPixel;
    GLuint texture = 0;
    GLuint texCoordsBuffer = 0;
    
    std::string currentText, lastText;
    std::string currentFontName, lastFontName;
    CGFloat currentFontSize, lastFontSize;
    
};


END_NAMESPACE_MW


#endif /* TextStimulus_hpp */



























