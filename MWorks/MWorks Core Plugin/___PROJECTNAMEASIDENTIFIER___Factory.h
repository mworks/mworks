/*
 *  ___PROJECTNAME___Factory.h
 *  MWPlugin
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */


#ifndef ___PROJECTNAME____FACTORY_H
#define ___PROJECTNAME____FACTORY_H

#include "___PROJECTNAME___.h"

#include "MWorksCore/ComponentFactory.h"
using namespace mw;

class ___PROJECTNAME___Factory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   mw::ComponentRegistry *reg);
};

#endif
