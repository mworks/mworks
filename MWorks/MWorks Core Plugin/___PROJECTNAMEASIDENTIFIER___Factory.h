/*
 *  ___PROJECTNAMEASIDENTIFIER___Factory.h
 *  ___PROJECTNAME___
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */

#ifndef ___PROJECTNAMEASIDENTIFIER____FACTORY_H
#define ___PROJECTNAMEASIDENTIFIER____FACTORY_H

#include <MWorksCore/ComponentFactory.h>

using namespace mw;


class ___PROJECTNAMEASIDENTIFIER___Factory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   ComponentRegistry *reg);
};


#endif
