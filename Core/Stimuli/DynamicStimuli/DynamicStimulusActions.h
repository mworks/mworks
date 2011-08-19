//
//  DynamicStimulusActions.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/18/11.
//  Copyright 2011 MIT. All rights reserved.
//

#ifndef MWorksCore_DynamicStimulusActions_h
#define MWorksCore_DynamicStimulusActions_h

#include "TrialBuildingBlocks.h"
#include "StimulusNode.h"
#include "DynamicStimulusDriver.h"


BEGIN_NAMESPACE_MW


class DynamicStimulusAction : public Action {
    
public:
    static const std::string STIMULUS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit DynamicStimulusAction(const ParameterValueMap &parameters);
    
    virtual ~DynamicStimulusAction() { }
    
    virtual void finalize(std::map<std::string, std::string> parameters, ComponentRegistryPtr reg);
    
protected:
    StimulusNodePtr stimulusNode;
    DynamicStimulusDriverPtr stimulus;
    
};


class PlayDynamicStimulus : public DynamicStimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit PlayDynamicStimulus(const ParameterValueMap &parameters);
    virtual bool execute();
    
};


class StopDynamicStimulus : public DynamicStimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit StopDynamicStimulus(const ParameterValueMap &parameters);
    virtual bool execute();
    
};


END_NAMESPACE_MW


#endif



























