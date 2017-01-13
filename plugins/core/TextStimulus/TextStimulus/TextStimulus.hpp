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


class TextStimulus : public ColoredTransformStimulus, boost::noncopyable {
    
public:
    static const std::string TEXT;
    static const std::string FONT_NAME;
    static const std::string FONT_SIZE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TextStimulus(const ParameterValueMap &parameters);
    
    void load(boost::shared_ptr<StimulusDisplay> display) override;
    void unload(boost::shared_ptr<StimulusDisplay> display) override;
    void drawInUnitSquare(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void bindTexture(int currentContextIndex);
    
    const VariablePtr text;
    const VariablePtr fontName;
    const VariablePtr fontSize;
    
    std::vector<double> pixelsPerDegree;
    std::vector<double> pointsPerPixel;
    std::vector<GLuint> texture;
    
    std::string currentText, lastText;
    std::string currentFontName, lastFontName;
    CGFloat currentFontSize, lastFontSize;
    
};


END_NAMESPACE_MW


#endif /* TextStimulus_hpp */



























