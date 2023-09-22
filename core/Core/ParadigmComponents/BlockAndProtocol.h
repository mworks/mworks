/*
 *  BlockAndProtocol.h
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on Wed Mar 24 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef BLOCK_AND_PROTOCOL_H
#define BLOCK_AND_PROTOCOL_H

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
    
    boost::shared_ptr<Component> createInstanceObject() override;
    
    void action() override;
    boost::weak_ptr<State> next() override;
    
};


class Protocol : public ListState {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    Protocol();
    explicit Protocol(const Map<ParameterValue> &parameters);
    
    boost::shared_ptr<Component> createInstanceObject() override;
    
};


class Trial : public ListState {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    Trial();
    explicit Trial(const Map<ParameterValue> &parameters);
    
    boost::shared_ptr<Component> createInstanceObject() override;
    
    void action() override;
    boost::weak_ptr<State> next() override;
    
};


class GenericListState : public ListState {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    GenericListState();
    explicit GenericListState(const Map<ParameterValue> &parameters);
    
    boost::shared_ptr<Component> createInstanceObject() override;
    
};


END_NAMESPACE_MW


#endif
