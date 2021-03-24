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
    
    static void describeComponent(ComponentInfo &info);
    
    explicit StimulusDisplayDevice(const ParameterValueMap &parameters);
    
};


END_NAMESPACE_MW


#endif
