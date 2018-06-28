//
//  MaskStimulus.hpp
//  LayerStimulus
//
//  Created by Christopher Stawarz on 6/19/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#ifndef MaskStimulus_hpp
#define MaskStimulus_hpp


BEGIN_NAMESPACE_MW


class MaskStimulus : public BasicTransformStimulus {
    
public:
    static const std::string MASK;
    static const std::string INVERTED;
    static const std::string STD_DEV;
    static const std::string MEAN;
    static const std::string NORMALIZED;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit MaskStimulus(const ParameterValueMap &parameters);
    
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    enum class MaskType {
        rectangle = 1,
        ellipse = 2,
        gaussian = 3
    };
    
    static MaskType maskTypeFromName(const std::string &name);
    
    gl::Shader getVertexShader() const override;
    gl::Shader getFragmentShader() const override;
    
    void setBlendEquation() override;
    
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void preDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    const boost::shared_ptr<Variable> maskTypeName;
    const boost::shared_ptr<Variable> inverted;
    const boost::shared_ptr<Variable> std_dev;
    const boost::shared_ptr<Variable> mean;
    const boost::shared_ptr<Variable> normalized;
    
    std::string current_mask_type_name;
    MaskType current_mask_type;
    bool current_inverted;
    double current_std_dev, current_mean;
    bool current_normalized;
    
    std::string last_mask_type_name;
    MaskType last_mask_type;
    bool last_inverted;
    double last_std_dev, last_mean;
    bool last_normalized;
    
    GLint alphaUniformLocation = -1;
    GLint maskTypeUniformLocation = -1;
    GLint invertedUniformLocation = -1;
    GLint stdDevUniformLocation = -1;
    GLint meanUniformLocation = -1;
    GLint normalizedUniformLocation = -1;
    
};


END_NAMESPACE_MW


#endif /* MaskStimulus_hpp */




















