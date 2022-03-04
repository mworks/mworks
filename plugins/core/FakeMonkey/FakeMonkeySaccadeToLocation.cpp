/*
 *  FakeMonkeySaccadeToLocation.cpp
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/8/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "FakeMonkeySaccadeToLocation.h"
using namespace mw;

mFakeMonkeySaccadeToLocation::mFakeMonkeySaccadeToLocation(const shared_ptr<mFakeMonkey> &_monkey, 
														   const shared_ptr<Variable> &_h_loc,
														   const shared_ptr<Variable> &_v_loc){
	monkey = _monkey;
	h_loc = _h_loc;
	v_loc = _v_loc;
	setName("FakeMonkeySaccadeToLocation");
}

mFakeMonkeySaccadeToLocation::~mFakeMonkeySaccadeToLocation(){ /* do nothing */}
bool mFakeMonkeySaccadeToLocation::execute(){
	
	if(monkey == NULL){
		mwarning(M_IODEVICE_MESSAGE_DOMAIN,
				 "Attempt to direct a NULL monkey");
	}
	
	monkey->saccadeTo(h_loc->getValue().getFloat(), 
					  v_loc->getValue().getFloat(), 
					  FAKE_MONKEY_MEAN_FIXATION_DURATION_US+1000000);
    
    return true;
}

