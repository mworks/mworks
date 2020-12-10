//
//  ColoredTransformStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#ifndef ColoredTransformStimulus_hpp
#define ColoredTransformStimulus_hpp

#include "AlphaBlendedTransformStimulus.hpp"


BEGIN_NAMESPACE_MW


class ColoredTransformStimulus : public AlphaBlendedTransformStimulus {
    
public:
    static const std::string COLOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ColoredTransformStimulus(const ParameterValueMap &parameters);
    
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    VariablePtr r;
    VariablePtr g;
    VariablePtr b;
    
    float current_r, current_g, current_b;
    float last_r, last_g, last_b;
    
};


END_NAMESPACE_MW


#endif /* ColoredTransformStimulus_hpp */
