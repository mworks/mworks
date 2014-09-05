//
//  FrameListStimulus.h
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#ifndef __MovieStimulusPlugin__FrameListStimulus__
#define __MovieStimulusPlugin__FrameListStimulus__

#include "BaseFrameListStimulus.h"


BEGIN_NAMESPACE_MW


class FrameListStimulus : public BaseFrameListStimulus {
    
public:
    static const std::string STIMULUS_GROUP;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FrameListStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() MW_OVERRIDE;
    
protected:
    void startPlaying() MW_OVERRIDE;
    
    int getNumFrames() MW_OVERRIDE {
        return stimulusGroup->getNElements();
    }
    
    int getNominalFrameNumber() MW_OVERRIDE;
    
    shared_ptr<Stimulus> getStimulusForFrame(int frameNumber) MW_OVERRIDE {
        return stimulusGroup->getElement(frameNumber);
    }
    
private:
    shared_ptr<StimulusGroup> stimulusGroup;
    
    MWTime currentFrameTime;
    int currentNominalFrameNumber;
    
};


END_NAMESPACE_MW


#endif /* !defined(__MovieStimulusPlugin__FrameListStimulus__) */



























