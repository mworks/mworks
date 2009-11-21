/*
 *  ColorTrio.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 12/10/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef PARSED_COLOR_TRIO_H
#define PARSED_COLOR_TRIO_H

#include "boost/shared_ptr.hpp"
#include <string>
#include "GenericVariable.h"
#include "ComponentRegistry.h"
//#include "ComponentFactory.h"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <boost/regex.hpp>

namespace mw {
	class ParsedColorTrio {
	private:
		boost::shared_ptr<Variable> r;
		boost::shared_ptr<Variable> g;
		boost::shared_ptr<Variable> b;
	public:
		ParsedColorTrio(ComponentRegistry *reg, 
						const std::string &color_string);
		boost::shared_ptr<Variable> getR() const;
		boost::shared_ptr<Variable> getG() const;
		boost::shared_ptr<Variable> getB() const;
	};
}
#endif
