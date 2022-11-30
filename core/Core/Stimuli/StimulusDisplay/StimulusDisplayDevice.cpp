//
//  StimulusDisplayDevice.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/19/12.
//  Copyright (c) 2012 MIT. All rights reserved.
//

#include "StimulusDisplayDevice.h"

#include "StimulusDisplay.h"


BEGIN_NAMESPACE_MW


const std::string StimulusDisplayDevice::BACKGROUND_COLOR("background_color");
const std::string StimulusDisplayDevice::BACKGROUND_ALPHA_MULTIPLIER("background_alpha_multiplier");
const std::string StimulusDisplayDevice::REDRAW_ON_EVERY_REFRESH("redraw_on_every_refresh");
const std::string StimulusDisplayDevice::ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES("announce_stimuli_on_implicit_updates");
const std::string StimulusDisplayDevice::CAPTURE_FORMAT("capture_format");
const std::string StimulusDisplayDevice::CAPTURE_HEIGHT_PIXELS("capture_height_pixels");
const std::string StimulusDisplayDevice::CAPTURE_ENABLED("capture_enabled");


void StimulusDisplayDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/stimulus_display");
    
    info.addParameter(BACKGROUND_COLOR, "0.5,0.5,0.5");
    info.addParameter(BACKGROUND_ALPHA_MULTIPLIER, "1.0");
    info.addParameter(REDRAW_ON_EVERY_REFRESH, "NO");
    info.addParameter(ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES, "YES");
    info.addParameter(CAPTURE_FORMAT, "none");
    info.addParameter(CAPTURE_HEIGHT_PIXELS, "0");
    info.addParameter(CAPTURE_ENABLED, "YES");
}


StimulusDisplayDevice::StimulusDisplayDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    backgroundColor(parameters[BACKGROUND_COLOR]),
    backgroundAlphaMultiplier(parameters[BACKGROUND_ALPHA_MULTIPLIER]),
    redrawOnEveryRefresh(parameters[REDRAW_ON_EVERY_REFRESH]),
    announceStimuliOnImplicitUpdates(parameters[ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES]),
    captureFormat(variableOrText(parameters[CAPTURE_FORMAT])->getValue().getString()),
    captureHeightPixels(parameters[CAPTURE_HEIGHT_PIXELS]),
    captureEnabled(parameters[CAPTURE_ENABLED])
{
    auto display = StimulusDisplay::getDefaultStimulusDisplay();
    
    display->setBackgroundColor(backgroundColor.red,
                                backgroundColor.green,
                                backgroundColor.blue,
                                backgroundAlphaMultiplier);
    display->clearDisplay();
    
    display->setRedrawOnEveryRefresh(redrawOnEveryRefresh);
    display->setAnnounceStimuliOnImplicitUpdates(announceStimuliOnImplicitUpdates);
    
    if (captureFormat != "none") {
        display->configureCapture(captureFormat, captureHeightPixels, captureEnabled);
    }
}


StimulusDisplayDevice::UniqueDeviceGuard::UniqueDeviceGuard() {
    if (deviceExists.test_and_set()) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Experiment can contain at most one stimulus display device");
    }
}


StimulusDisplayDevice::UniqueDeviceGuard::~UniqueDeviceGuard() {
    deviceExists.clear();
}


std::atomic_flag StimulusDisplayDevice::UniqueDeviceGuard::deviceExists = ATOMIC_FLAG_INIT;


END_NAMESPACE_MW
