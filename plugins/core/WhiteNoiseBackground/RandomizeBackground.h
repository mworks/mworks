/*
 *  RandomizeBackground.h
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/17/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#ifndef RandomizeBackground_H_
#define RandomizeBackground_H_

#include <MWorksCore/TrialBuildingBlocks.h>

using namespace mw;


class RandomizeBackground : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit RandomizeBackground(const ParameterValueMap &parameters);
    virtual bool execute();
    
private:
    shared_ptr<StimulusNode> backgroundNode;
    
};


#endif
