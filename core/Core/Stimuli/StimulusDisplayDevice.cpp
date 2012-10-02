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


void StimulusDisplayDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    info.setSignature("iodevice/stimulus_display");
    info.addParameter(BACKGROUND_COLOR, true, "0.5,0.5,0.5");
}


StimulusDisplayDevice::StimulusDisplayDevice(const ParameterValueMap &parameters) :
    IODevice(parameters)
{
    RGBColor bg(parameters[BACKGROUND_COLOR]);
    
    shared_ptr<StimulusDisplay> display(StimulusDisplay::getCurrentStimulusDisplay());
    display->setBackgroundColor(bg.red, bg.green, bg.blue);
    display->clearDisplay();
}


END_NAMESPACE_MW
