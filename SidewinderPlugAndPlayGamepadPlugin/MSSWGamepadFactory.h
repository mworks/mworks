/*
 *  MSSWGamepadFactory.h
 *  MSSWGamepadPlugin
 *
 *  Created by labuser on 8/18/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef MSSW_GAMEPAD_FACTORY_H
#define MSSW_GAMEPAD_FACTORY_H

#include "MonkeyWorksCore/ComponentFactory.h"

class mMSSWGamepadFactory : public mw::ComponentFactory {
	virtual boost::shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
														  mw::mwComponentRegistry *reg);
};

#endif

