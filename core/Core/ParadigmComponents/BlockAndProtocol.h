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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <boost/lexical_cast.hpp>
#pragma clang diagnostic pop


BEGIN_NAMESPACE_MW


class Block : public ListState {
	
public:
    static void describeComponent(ComponentInfo &info);
	
	Block();
    explicit Block(const Map<ParameterValue> &parameters);
	
    shared_ptr<mw::Component> createInstanceObject() override;
	
    void action() override;
    weak_ptr<State> next() override;
    
};


class Protocol : public ListState {
	
public:
    static void describeComponent(ComponentInfo &info);
	
	Protocol();
    explicit Protocol(const Map<ParameterValue> &parameters);
	
    shared_ptr<mw::Component> createInstanceObject() override;
    
};


class Trial : public ListState {
	
public:
    static void describeComponent(ComponentInfo &info);
	
	Trial();
    explicit Trial(const Map<ParameterValue> &parameters);
	
    void action() override;
    weak_ptr<State> next() override;
    shared_ptr<mw::Component> createInstanceObject() override;
	
};


class GenericListState : public ListState {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    GenericListState();
    explicit GenericListState(const Map<ParameterValue> &parameters);
    
    shared_ptr<mw::Component> createInstanceObject() override;
    
};


END_NAMESPACE_MW


#endif
