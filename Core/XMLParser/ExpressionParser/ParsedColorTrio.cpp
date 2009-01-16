/*
 *  ColorTrio.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 12/10/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ParsedColorTrio.h"

using namespace mw;

ParsedColorTrio::ParsedColorTrio(mwComponentRegistry *reg, 
								 const std::string &color_string) {
	using namespace boost;
	// regex for parsing the color string
	regex color_regex("^\\s*([a-zA-z]\\w*|[\\d]*\\.?[\\d]+)\\s*,\\s*([a-zA-z]\\w*|[\\d]*\\.?[\\d]+)\\s*,\\s*([a-zA-z]\\w*|[\\d]*\\.?[\\d]+)\\s*$"); 
	
	smatch colorParams;
	try{
		if(!regex_match(color_string, colorParams, color_regex)){
			// TODO: throw?
			throw SimpleException("Invalid color string", color_string);
		} 
	} catch (regex_error& e) {
		// TODO: throw a factory exception of some kind
		throw SimpleException("Regular expression error during color parsing");
	}
	
	
	if(colorParams.size() != 4) {
		throw SimpleException("Invalid color string", color_string);
	}
	
	// first is r then g then b:
	r=reg->getVariable(colorParams[1]);	
	g=reg->getVariable(colorParams[2]);	
	b=reg->getVariable(colorParams[3]);			
}

boost::shared_ptr<Variable> ParsedColorTrio::getR() const {
	return r;
}

boost::shared_ptr<Variable> ParsedColorTrio::getG() const {
	return g;
}

boost::shared_ptr<Variable> ParsedColorTrio::getB() const {
	return b;
}
