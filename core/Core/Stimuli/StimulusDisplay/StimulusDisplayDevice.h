//
//  StimulusDisplayDevice.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/19/12.
//  Copyright (c) 2012 MIT. All rights reserved.
//

#ifndef MWorksCore_StimulusDisplayDevice_h
#define MWorksCore_StimulusDisplayDevice_h

#include "IODevice.h"


BEGIN_NAMESPACE_MW


class StimulusDisplayDevice : public IODevice {
    
public:
    static const std::string BACKGROUND_COLOR;
    static const std::string BACKGROUND_ALPHA_MULTIPLIER;
    static const std::string REDRAW_ON_EVERY_REFRESH;
    static const std::string ANNOUNCE_STIMULI_ON_IMPLICIT_UPDATES;
    static const std::string CAPTURE_FORMAT;
    static const std::string CAPTURE_HEIGHT_PIXELS;
    static const std::string CAPTURE_ENABLED;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit StimulusDisplayDevice(const ParameterValueMap &parameters);
    
private:
    struct UniqueDeviceGuard : boost::noncopyable {
        UniqueDeviceGuard();
        ~UniqueDeviceGuard();
    private:
        static std::atomic_flag deviceExists;
    };
    
    const UniqueDeviceGuard uniqueDeviceGuard;
    const RGBColor backgroundColor;
    const double backgroundAlphaMultiplier;
    const bool redrawOnEveryRefresh;
    const bool announceStimuliOnImplicitUpdates;
    const std::string captureFormat;
    const int captureHeightPixels;
    const VariablePtr captureEnabled;
    
};


END_NAMESPACE_MW


#endif /* MWorksCore_StimulusDisplayDevice_h */
