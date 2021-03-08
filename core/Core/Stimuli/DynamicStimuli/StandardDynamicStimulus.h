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


template<typename BaseStimulus>
class DynamicStimulusBase : public BaseStimulus, public DynamicStimulusDriver {
    
public:
    static const std::string AUTOPLAY;
    
    static void describeComponent(ComponentInfo &info);

    explicit DynamicStimulusBase(const ParameterValueMap &parameters);
    
    void setVisible(bool newvis) override;
    bool needDraw(boost::shared_ptr<StimulusDisplay> display) override;
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    void beginPause() override;
    
    virtual void drawFrame(boost::shared_ptr<StimulusDisplay> display);
    
private:
    const VariablePtr autoplay;
    bool didDrawWhilePaused;
    
};


template<typename BaseStimulus>
const std::string DynamicStimulusBase<BaseStimulus>::AUTOPLAY("autoplay");


template<typename BaseStimulus>
void DynamicStimulusBase<BaseStimulus>::describeComponent(ComponentInfo &info) {
    BaseStimulus::describeComponent(info);
    info.addParameter(AUTOPLAY, "NO");
}


template<typename BaseStimulus>
DynamicStimulusBase<BaseStimulus>::DynamicStimulusBase(const ParameterValueMap &parameters) :
    BaseStimulus(parameters),
    autoplay(parameters[AUTOPLAY]),
    didDrawWhilePaused(false)
{ }


template<typename BaseStimulus>
void DynamicStimulusBase<BaseStimulus>::setVisible(bool newvis) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    BaseStimulus::setVisible(newvis);
    
    if (!newvis && isPlaying() && autoplay->getValue().getBool()) {
        stopPlaying();
    }
}


template<typename BaseStimulus>
bool DynamicStimulusBase<BaseStimulus>::needDraw(boost::shared_ptr<StimulusDisplay> display) {
    return isPlaying() && !(isPaused() && didDrawWhilePaused);
}


template<typename BaseStimulus>
void DynamicStimulusBase<BaseStimulus>::draw(boost::shared_ptr<StimulusDisplay> display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    if (!isPlaying()) {
        if (autoplay->getValue().getBool()) {
            startPlaying();
        } else {
            return;
        }
    }
    
    drawFrame(display);
    
    if (isPaused()) {
        didDrawWhilePaused = true;
    }
}


template<typename BaseStimulus>
Datum DynamicStimulusBase<BaseStimulus>::getCurrentAnnounceDrawData() {
    auto announceData = BaseStimulus::getCurrentAnnounceDrawData();
    announceData.addElement("start_time", getStartTime());
    return announceData;
}


template<typename BaseStimulus>
void DynamicStimulusBase<BaseStimulus>::beginPause() {
    DynamicStimulusDriver::beginPause();
    didDrawWhilePaused = false;
}


template<typename BaseStimulus>
void DynamicStimulusBase<BaseStimulus>::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    BaseStimulus::draw(display);
}


using StandardDynamicStimulus = DynamicStimulusBase<Stimulus>;


END_NAMESPACE_MW


#endif /* StandardDynamicStimulus_H_ */
