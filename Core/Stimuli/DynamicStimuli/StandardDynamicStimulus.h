/*
 *  StandardDynamicStimulus.h
 *
 *  Created by Christopher Stawarz on 8/11/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#ifndef StandardDynamicStimulus_H_
#define StandardDynamicStimulus_H_

#include "DynamicStimulusDriver.h"
#include "Stimulus.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


class StandardDynamicStimulus : public Stimulus, public DynamicStimulusDriver {

public:
    static void describeComponent(ComponentInfo &info);

    explicit StandardDynamicStimulus(const ParameterValueMap &parameters);
    
    virtual ~StandardDynamicStimulus() { }
    
    virtual bool needDraw();
    virtual Datum getCurrentAnnounceDrawData();
    
};


END_NAMESPACE_MW


#endif 























