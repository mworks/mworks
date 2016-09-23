/*
 *  StandardDynamicStimulus.h
 *
 *  Created by Christopher Stawarz on 8/11/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#ifndef StandardDynamicStimulus_H_
#define StandardDynamicStimulus_H_

#include "Stimulus.h"
#include "DynamicStimulusDriver.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


class StandardDynamicStimulus : public Stimulus, public DynamicStimulusDriver {
    
public:
    static const std::string AUTOPLAY;
    
    static void describeComponent(ComponentInfo &info);

    explicit StandardDynamicStimulus(const ParameterValueMap &parameters);
    
    void setVisible(bool newvis) override;
    bool needDraw() override;
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    void beginPause() override;
    
private:
    virtual void drawFrame(shared_ptr<StimulusDisplay> display) = 0;
    
    const VariablePtr autoplay;
    bool didDrawWhilePaused;
    
};


END_NAMESPACE_MW


#endif 























