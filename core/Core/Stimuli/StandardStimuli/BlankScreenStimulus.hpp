//
//  BlankScreenStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#ifndef BlankScreenStimulus_hpp
#define BlankScreenStimulus_hpp

#include "Stimulus.h"


BEGIN_NAMESPACE_MW


class BlankScreen : public Stimulus {
    
public:
    static const std::string COLOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit BlankScreen(const Map<ParameterValue> &parameters);
    
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    VariablePtr r;
    VariablePtr g;
    VariablePtr b;
    
    double current_r, current_g, current_b;
    double last_r, last_g, last_b;
    
};


END_NAMESPACE_MW


#endif /* BlankScreenStimulus_hpp */
