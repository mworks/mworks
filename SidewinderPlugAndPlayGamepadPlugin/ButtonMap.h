/*
 *  ButtonMap.h
 *  MSSWGamepad
 *
 *  Created by bkennedy on 10/2/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef BUTTON_MAP_H
#define BUTTON_MAP_H

#include "MonkeyWorksCore/GenericVariable.h"
using namespace mw;


class mButtonMap {
protected:
	long capability_id;
	std::string name;
public:
	mButtonMap(const long capability_id_,
			   const std::string &name);
	long getCapabilityID() const;	
	std::string getName() const;
};

#endif
