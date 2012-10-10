/*
 *  ColorTrio.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 12/10/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ParsedColorTrio.h"


BEGIN_NAMESPACE_MW


const boost::regex ParsedColorTrio::color_regex("^\\s*([a-zA-z]\\w*|[\\d]*\\.?[\\d]+)\\s*,\\s*([a-zA-z]\\w*|[\\d]*"
                                         "\\.?[\\d]+)\\s*,\\s*([a-zA-z]\\w*|[\\d]*\\.?[\\d]+)\\s*$");


ParsedColorTrio::ParsedColorTrio(ComponentRegistry *reg, const std::string &color_string)
{
	boost::smatch colorParams;
	try{
		if(!regex_match(color_string, colorParams, color_regex)){
			throw SimpleException("Invalid color string", color_string);
		} 
	} catch (boost::regex_error& e) {
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


RGBColor ParsedColorTrio::getValue() const {
    RGBColor color;
    color.red = getRedValue();
    color.green = getGreenValue();
    color.blue = getBlueValue();
    return color;
}


END_NAMESPACE_MW






















