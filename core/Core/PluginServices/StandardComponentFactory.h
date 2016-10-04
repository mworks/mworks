/*
 *  StandardComponentFactory.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MW_STANDARD_COMPONENT_FACTORY_H
#define _MW_STANDARD_COMPONENT_FACTORY_H

#include "ComponentFactory.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


template<typename ComponentType>
class StandardComponentFactory : public ComponentFactory {
    
    //
    // ComponentType must implement the following methods:
    //
    // static void describeComponent(ComponentInfo &info);
    // explicit ComponentType(const ParameterValueMap &parameters);
    //
    
public:
    StandardComponentFactory() {
        ComponentType::describeComponent(info);
    }
    
    boost::shared_ptr<mw::Component> createObject(StdStringMap parameters, ComponentRegistryPtr reg) override {
        ParameterValueMap values;
        processParameters(parameters, reg, values);
        return boost::make_shared<ComponentType>(values);
    }
    
};


END_NAMESPACE_MW


#endif






















