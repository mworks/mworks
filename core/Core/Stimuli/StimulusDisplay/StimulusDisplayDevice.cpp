//
//  StimulusDisplayDevice.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/19/12.
//  Copyright (c) 2012 MIT. All rights reserved.
//

#include "StimulusDisplayDevice.h"


BEGIN_NAMESPACE_MW


const std::string StimulusDisplayDevice::DISPLAY_INFO("display_info");
const std::string StimulusDisplayDevice::BACKGROUND_COLOR("background_color");
const std::string StimulusDisplayDevice::BACKGROUND_ALPHA_MULTIPLIER("background_alpha_multiplier");
const std::string StimulusDisplayDevice::REDRAW_ON_EVERY_REFRESH("redraw_on_every_refresh");
const std::string StimulusDisplayDevice::ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES("announce_stimuli_on_implicit_updates");
const std::string StimulusDisplayDevice::UPDATE_VARIABLE("update_variable");
const std::string StimulusDisplayDevice::CAPTURE_FORMAT("capture_format");
const std::string StimulusDisplayDevice::CAPTURE_HEIGHT_PIXELS("capture_height_pixels");
const std::string StimulusDisplayDevice::CAPTURE_ENABLED("capture_enabled");
const std::string StimulusDisplayDevice::CAPTURE_VARIABLE("capture_variable");


void StimulusDisplayDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/stimulus_display");
    
    info.addParameter(DISPLAY_INFO, false);
    info.addParameter(BACKGROUND_COLOR, "0.5,0.5,0.5");
    info.addParameter(BACKGROUND_ALPHA_MULTIPLIER, "1.0");
    info.addParameter(REDRAW_ON_EVERY_REFRESH, "NO");
    info.addParameter(ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES, "YES");
    info.addParameter(UPDATE_VARIABLE, false);
    info.addParameter(CAPTURE_FORMAT, "none");
    info.addParameter(CAPTURE_HEIGHT_PIXELS, "0");
    info.addParameter(CAPTURE_ENABLED, "YES");
    info.addParameter(CAPTURE_VARIABLE, false);
}


StimulusDisplayDevice::StimulusDisplayDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    displayInfo(optionalVariable(parameters[DISPLAY_INFO])),
    backgroundColor(parameters[BACKGROUND_COLOR]),
    backgroundAlphaMultiplier(parameters[BACKGROUND_ALPHA_MULTIPLIER]),
    redrawOnEveryRefresh(parameters[REDRAW_ON_EVERY_REFRESH]),
    announceStimuliOnImplicitUpdates(parameters[ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES]),
    updateVar(optionalVariable(parameters[UPDATE_VARIABLE])),
    captureFormat(variableOrText(parameters[CAPTURE_FORMAT])->getValue().getString()),
    captureHeightPixels(parameters[CAPTURE_HEIGHT_PIXELS]),
    captureEnabled(parameters[CAPTURE_ENABLED]),
    captureVar(optionalVariable(parameters[CAPTURE_VARIABLE]))
{
    // Make a copy to ensure the experiment stays alive until we're done with it
    auto experiment = GlobalCurrentExperiment;
    if (!experiment) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "No experiment currently defined");
    }
    
    if (displayInfo) {
        // Create a new display using the provided display info
        const auto config = StimulusDisplay::getDisplayConfiguration(displayInfo->getValue());
        display = StimulusDisplay::prepareStimulusDisplay(config);
    } else if (experiment->getShouldCreateDefaultStimulusDisplay()) {
        // The experiment has a default display, so just configure that
        isDefaultDisplay.set();
        display = experiment->getDefaultStimulusDisplay();
    } else {
        // Create a new display using the contents of #mainScreenInfo
        auto mainScreenInfo = parameters[DISPLAY_INFO].getRegistry()->getVariable(MAIN_SCREEN_INFO_TAGNAME);
        const auto config = StimulusDisplay::getDisplayConfiguration(mainScreenInfo->getValue());
        display = StimulusDisplay::prepareStimulusDisplay(config);
    }
    
    if (!isDefaultDisplay.get()) {
        // Register the new display with the experiment
        experiment->addStimulusDisplay(display);
    }
    
    display->setBackgroundColor(backgroundColor.red,
                                backgroundColor.green,
                                backgroundColor.blue,
                                backgroundAlphaMultiplier);
    display->clearDisplay();
    
    display->setRedrawOnEveryRefresh(redrawOnEveryRefresh);
    display->setAnnounceStimuliOnImplicitUpdates(announceStimuliOnImplicitUpdates);
    if (updateVar) {
        display->setUpdateVar(updateVar);
    }
    
    if (captureFormat != "none") {
        display->configureCapture(captureFormat, captureHeightPixels, captureEnabled);
        if (captureVar) {
            display->setCaptureVar(captureVar);
        }
    }
}


StimulusDisplayDevice::IsDefaultDisplay::~IsDefaultDisplay() {
    if (isDefaultDisplay) {
        defaultDisplayInUse.clear();
    }
}


void StimulusDisplayDevice::IsDefaultDisplay::set() {
    if (defaultDisplayInUse.test_and_set()) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              "At most one stimulus display device can configure the default display");
    }
    isDefaultDisplay = true;
}


std::atomic_flag StimulusDisplayDevice::IsDefaultDisplay::defaultDisplayInUse = ATOMIC_FLAG_INIT;


END_NAMESPACE_MW
