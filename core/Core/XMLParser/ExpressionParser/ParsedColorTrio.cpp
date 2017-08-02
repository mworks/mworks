/*
 *  ColorTrio.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 12/10/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ParsedColorTrio.h"

#include "ExpressionVariable.h"


BEGIN_NAMESPACE_MW


ParsedColorTrio::ParsedColorTrio(ComponentRegistry *reg, const std::string &color_string)
{
    auto exprList = ParsedExpressionVariable::parseExpressionList(color_string);
    if (exprList.size() != 3) {
        throw SimpleException("Color string must have three comma-separated components");
    }
    
    // first is r then g then b:
    r = boost::make_shared<ParsedExpressionVariable>(exprList.at(0));
    g = boost::make_shared<ParsedExpressionVariable>(exprList.at(1));
    b = boost::make_shared<ParsedExpressionVariable>(exprList.at(2));
}


RGBColor ParsedColorTrio::getValue() const {
    RGBColor color;
    color.red = getRedValue();
    color.green = getGreenValue();
    color.blue = getBlueValue();
    return color;
}


END_NAMESPACE_MW






















