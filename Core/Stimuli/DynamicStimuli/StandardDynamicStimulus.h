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
#include "MWorksCore/Stimulus.h"

using namespace mw;


class StandardDynamicStimulus : public Stimulus, public DynamicStimulusDriver {

public:
    StandardDynamicStimulus(const std::string &tag, shared_ptr<Variable> framesPerSecond);
    
    virtual ~StandardDynamicStimulus() { }

    virtual void didStop();
    
    virtual bool needDraw();
    virtual void draw(shared_ptr<StimulusDisplay> display);
    virtual void drawFrame(shared_ptr<StimulusDisplay> display, int frameNumber) { }
    
    virtual Datum getCurrentAnnounceDrawData();
    
protected:
    int lastFrameDrawn;
    
};


#endif 























