/*
 *  BlockAndProtocol.cpp
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on Wed Mar 24 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "BlockAndProtocol.h"

#include "ComponentInfo.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


void Block::describeComponent(ComponentInfo &info) {
    ListState::describeComponent(info);
    info.setSignature("block");
}


Block::Block() {
    setName("Block");
}


Block::Block(const ParameterValueMap &parameters) :
    ListState(parameters)
{ }


boost::shared_ptr<Component> Block::createInstanceObject() {
    return clone<Block>();
}


void Block::action() {
    ListState::action();
    if (!accessed) {
        blockAnnounce->setValue(blockAnnounce->getValue().getInteger() + 1);
        accessed = true;
    }
}


boost::weak_ptr<State> Block::next() {
    if (!hasMoreChildrenToRun()) {
        blockAnnounce->setValue(blockAnnounce->getValue().getInteger() - 1);
    }
    return ListState::next();
}


void Protocol::describeComponent(ComponentInfo &info) {
    ListState::describeComponent(info);
    info.setSignature("protocol");
}


Protocol::Protocol() {
    setName("Protocol");
}


Protocol::Protocol(const ParameterValueMap &parameters) :
    ListState(parameters)
{ }


boost::shared_ptr<Component> Protocol::createInstanceObject() {
    return clone<Protocol>();
}


void Trial::describeComponent(ComponentInfo &info) {
    ListState::describeComponent(info);
    info.setSignature("trial");
}


Trial::Trial() {
    setName("Trial");
}


Trial::Trial(const ParameterValueMap &parameters) :
    ListState(parameters)
{ }


boost::shared_ptr<Component> Trial::createInstanceObject() {
    return clone<Trial>();
}


void Trial::action() {
    ListState::action();
    if (!accessed) {
        trialAnnounce->setValue(trialAnnounce->getValue().getInteger() + 1);
        accessed = true;
    }
}


boost::weak_ptr<State> Trial::next() {
    if (!hasMoreChildrenToRun()) {
        trialAnnounce->setValue(trialAnnounce->getValue().getInteger() - 1);
    }
    return ListState::next();
}


void GenericListState::describeComponent(ComponentInfo &info) {
    ListState::describeComponent(info);
    info.setSignature("list");
}


GenericListState::GenericListState() {
    setName("List");
}


GenericListState::GenericListState(const ParameterValueMap &parameters) :
    ListState(parameters)
{ }


boost::shared_ptr<Component> GenericListState::createInstanceObject() {
    return clone<GenericListState>();
}


END_NAMESPACE_MW
