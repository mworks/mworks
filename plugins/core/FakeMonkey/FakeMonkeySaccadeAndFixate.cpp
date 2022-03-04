/*
 *  FakeMonkeySaccadeAndFixate.cpp
 *  FakeMonkey
 *
 *  Created by labuser on 10/8/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "FakeMonkeySaccadeAndFixate.h"

mFakeMonkeySaccadeAndFixate::mFakeMonkeySaccadeAndFixate(const shared_ptr<mFakeMonkey> &_monkey, 
														 const shared_ptr<Variable> &_h_loc,
														 const shared_ptr<Variable> &_v_loc,
														 const shared_ptr<Variable> &_duration) {
	monkey = _monkey;
	h_loc = _h_loc;
	v_loc = _v_loc;
	duration=_duration;
	setName("FakeMonkeySaccadeAndFixate");
}

mFakeMonkeySaccadeAndFixate::~mFakeMonkeySaccadeAndFixate(){ /* do nothing */}
bool mFakeMonkeySaccadeAndFixate::execute(){
	
	if(monkey == NULL){
		mwarning(M_IODEVICE_MESSAGE_DOMAIN,
				 "Attempt to direct a NULL monkey");
	}
	
	monkey->saccadeTo(h_loc->getValue().getFloat(), 
					  v_loc->getValue().getFloat(), 
					  duration->getValue().getInteger());
    
    return true;
}
