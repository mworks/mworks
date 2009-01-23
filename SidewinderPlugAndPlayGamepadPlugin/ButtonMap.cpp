/*
 *  ButtonMap.cpp
 *  MSSWGamepad
 *
 *  Created by bkennedy on 10/2/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ButtonMap.h"

mButtonMap::mButtonMap(const long capability_id_,
					   const std::string &name_) : capability_id(capability_id_), name(name_) {}

long mButtonMap::getCapabilityID() const {
	return capability_id;
}

std::string mButtonMap::getName() const {
	return name;
}