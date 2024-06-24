//
//  CompoundStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/13/24.
//

#ifndef CompoundStimulus_hpp
#define CompoundStimulus_hpp

#include "ParameterValue.h"
#include "Stimulus.h"


BEGIN_NAMESPACE_MW


class CompoundStimulus : public Stimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit CompoundStimulus(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  ComponentPtr child) override;
    
    void freeze(bool shouldFreeze = true) override;
    
    void load(StimulusDisplayPtr display) override;
    void unload(StimulusDisplayPtr display) override;
    void setVisible(bool newVisible) override;
    bool needDraw(StimulusDisplayPtr display) override;
    void draw(StimulusDisplayPtr display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    std::vector<StimulusPtr> children;
    
};


END_NAMESPACE_MW


#endif /* CompoundStimulus_hpp */
