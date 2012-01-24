/*
 *  ColorTrio.h
 *  MWorksCore
 *
 *  Created by bkennedy on 12/10/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef PARSED_COLOR_TRIO_H
#define PARSED_COLOR_TRIO_H

#include <string>

#include <boost/regex.hpp>

#include "ComponentRegistry.h"
#include "GenericVariable.h"


BEGIN_NAMESPACE_MW


struct RGBColor {
    double red;
    double green;
    double blue;
};


class ParsedColorTrio {

public:
    ParsedColorTrio(ComponentRegistry *reg, const std::string &color_string);

    VariablePtr getR() const { return r; }
    VariablePtr getG() const { return g; }
    VariablePtr getB() const { return b; }
    
    RGBColor getValue() const;
    
    double getRedValue() const { return r->getValue().getFloat(); }
    double getGreenValue() const { return g->getValue().getFloat(); }
    double getBlueValue() const { return b->getValue().getFloat(); }

private:
    static const boost::regex color_regex;

    VariablePtr r;
    VariablePtr g;
    VariablePtr b;

};


END_NAMESPACE_MW


#endif






















