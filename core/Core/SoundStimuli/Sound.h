/*
 *  Sound.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SOUND_H_
#define SOUND_H_

#include "Announcers.h"
#include "Component.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


class Sound : public Component, public Announcable {
    
public:
    static const std::string AUTOLOAD;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit Sound(const ParameterValueMap &parameters);
    explicit Sound(const std::string &tag = "", bool autoload = true);
    
    bool shouldAutoload() const { return autoload; }
    virtual void load() { }
    virtual void unload() { }
    
    virtual void play() = 0;
    virtual void play(MWTime startTime);
    virtual void pause() = 0;
    virtual void stop() = 0;
    
private:
    const bool autoload;
    
};


using SoundPtr = boost::shared_ptr<Sound>;


END_NAMESPACE_MW


#endif /* SOUND_H_ */
