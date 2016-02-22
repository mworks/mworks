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


BEGIN_NAMESPACE_MW


class Block : public ListState {
	
public:
	
	Block();
	
    shared_ptr<mw::Component> createInstanceObject() override;
	
    void action() override;
    weak_ptr<State> next() override;
};


class Protocol : public ListState {
	
	
public:
	
	Protocol();
	
    shared_ptr<mw::Component> createInstanceObject() override;
};


class Trial : public ListState {
	
public:
	
	Trial();
	
	
    void action() override;
    weak_ptr<State> next() override;
    shared_ptr<mw::Component> createInstanceObject() override;
	
};


class GenericListState : public ListState {
    
public:
    shared_ptr<mw::Component> createInstanceObject() override;
    
};


class BlockFactory : public ListStateFactory<Block>{};
class TrialFactory : public ListStateFactory<Trial>{};
class ProtocolFactory : public ListStateFactory<mw::Protocol>{};
class GenericListStateFactory : public ListStateFactory<GenericListState>{}; 


END_NAMESPACE_MW


#endif
