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
    
protected:
    DynamicStimulusDriverPtr getDynamicStimulus() {
        return boost::dynamic_pointer_cast<DynamicStimulusDriver>(stimulusNode->getStimulus());
    }
    
    StimulusNodePtr stimulusNode;
    
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


class PauseDynamicStimulus : public DynamicStimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit PauseDynamicStimulus(const ParameterValueMap &parameters);
    virtual bool execute();
    
};


class UnpauseDynamicStimulus : public DynamicStimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit UnpauseDynamicStimulus(const ParameterValueMap &parameters);
    virtual bool execute();
    
};


END_NAMESPACE_MW


#endif



























