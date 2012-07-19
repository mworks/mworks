/*
 *  BlockAndProtocol.cpp
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on Wed Mar 24 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "BlockAndProtocol.h"


BEGIN_NAMESPACE_MW


////////////////////////////////////////////////
// ->mBlock Functions
////////////////////////////////////////////////


Block::Block() : ListState(){
	setName("Block");
}


void Block::action(){
	ListState::action();
	if(!accessed) {
		blockAnnounce->setValue(blockAnnounce->getValue().getInteger() + 1);
		accessed = true;
	}
}

weak_ptr<State> Block::next(){
	if (!hasMoreChildrenToRun()) {
		blockAnnounce->setValue(blockAnnounce->getValue().getInteger() - 1);
	}
	return ListState::next();
}


shared_ptr<mw::Component> Block::createInstanceObject(){
    return clone<Block>();
}


		
mw::Protocol::Protocol() : ListState(){
	setName("Protocol");
}
		

shared_ptr<mw::Component> mw::Protocol::createInstanceObject(){
    return clone<Protocol>();
}


//////////////

Trial::Trial() :ListState(){
	setName("Trial");
}


void Trial::action(){
	ListState::action();
	if(!accessed) {
		trialAnnounce->setValue(trialAnnounce->getValue().getInteger() + 1);
		accessed = true;
	}
}

weak_ptr<State> Trial::next(){
	if (!hasMoreChildrenToRun()) {
		trialAnnounce->setValue(trialAnnounce->getValue().getInteger() - 1);
	}
	return ListState::next();
}


shared_ptr<mw::Component> Trial::createInstanceObject(){
    return clone<Trial>();
}


shared_ptr<mw::Component> GenericListState::createInstanceObject(){
    return clone<GenericListState>();
}


END_NAMESPACE_MW
