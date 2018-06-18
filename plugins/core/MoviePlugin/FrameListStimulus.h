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
    static void describeComponent(ComponentInfo &info);
    
    explicit FrameListStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() MW_OVERRIDE;
    
private:
    void startPlaying() MW_OVERRIDE;
    
    int getNominalFrameNumber() MW_OVERRIDE;
    
    MWTime currentFrameTime;
    int currentNominalFrameNumber;
    
};


END_NAMESPACE_MW


#endif /* !defined(__MovieStimulusPlugin__FrameListStimulus__) */



























