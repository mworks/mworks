//
//  StimulusDisplayDevice.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/19/12.
//  Copyright (c) 2012 MIT. All rights reserved.
//

#include "StimulusDisplayDevice.h"

#include "ParsedColorTrio.h"
#include "StimulusDisplay.h"


BEGIN_NAMESPACE_MW


const std::string StimulusDisplayDevice::BACKGROUND_COLOR("background_color");
const std::string StimulusDisplayDevice::ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES("announce_stimuli_on_implicit_updates");


void StimulusDisplayDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    info.setSignature("iodevice/stimulus_display");
    info.addParameter(BACKGROUND_COLOR, true, "0.5,0.5,0.5");
    info.addParameter(ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES, "YES");
}


StimulusDisplayDevice::StimulusDisplayDevice(const ParameterValueMap &parameters) :
    IODevice(parameters)
{
    RGBColor bg(parameters[BACKGROUND_COLOR]);
    
    shared_ptr<StimulusDisplay> display(StimulusDisplay::getCurrentStimulusDisplay());
    
    display->setBackgroundColor(bg.red, bg.green, bg.blue);
    display->clearDisplay();
    
    display->setAnnounceStimuliOnImplicitUpdates(bool(parameters[ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES]));
}


END_NAMESPACE_MW
