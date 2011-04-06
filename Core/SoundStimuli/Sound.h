#ifndef	SOUND_H_
#define SOUND_H_

/*
 *  Sound.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Utilities.h"
#include "Component.h"
#include "StandardVariables.h"
#include "Announcers.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


class Sound : public Component, public Announcable {

public:
    Sound(const ParameterValueMap &parameters) :
        Component(parameters),
        Announcable(ANNOUNCE_SOUND_TAGNAME)
    { }
	
	virtual ~Sound() { }

	virtual void play(){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				"Attempting a play an empty base-class sound");
	}
	
	virtual void stop(){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				"Attempting a stop an empty base-class sound");
	}
	
	virtual void pause(){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				"Attempting a pause an empty base-class sound");
	}
	
};


END_NAMESPACE_MW


#endif






















