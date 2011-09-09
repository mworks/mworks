#ifndef BLOCK_AND_PROTOCOL_H
#define BLOCK_AND_PROTOCOL_H

/*
 *  BlockAndProtocol.h
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on Wed Mar 24 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "States.h"
#include <boost/lexical_cast.hpp>

namespace mw {

class Block : public ListState{
	
public:
	
	Block();
	
	virtual shared_ptr<mw::Component> createInstanceObject();
	
	virtual void action();
	virtual weak_ptr<State> next();
};


class Protocol : public ListState {
	
	
public:
	
	Protocol();
	
	virtual shared_ptr<mw::Component> createInstanceObject();
};


class GenericListState : public ListState {
	
public:
	
	GenericListState();
	
	virtual shared_ptr<mw::Component> createInstanceObject();
	

};


class Trial : public ListState {
	
public:
	
	Trial();
	
	
	virtual void action();
	virtual weak_ptr<State> next();
	virtual shared_ptr<mw::Component> createInstanceObject();
	
};


class BlockFactory : public ListStateFactory<Block>{};
class TrialFactory : public ListStateFactory<Trial>{};
class ProtocolFactory : public ListStateFactory<mw::Protocol>{};
class GenericListStateFactory : public ListStateFactory<GenericListState>{}; 
}
#endif

