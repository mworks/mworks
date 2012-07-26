/*
 *  ButtonMap.h
 *  MSSWGamepad
 *
 *  Created by bkennedy on 10/2/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef BUTTON_MAP_H
#define BUTTON_MAP_H

#include <string>


class ButtonMap {
    
private:
    long capabilityID;
    std::string name;
    long minValue, maxValue, meanValue;
    
public:
    ButtonMap(long capabilityID, const std::string &name, long minValue, long maxValue) :
        capabilityID(capabilityID),
        name(name),
        minValue(minValue),
        maxValue(maxValue),
        meanValue((maxValue - minValue) / 2L)
    { }
    
    long getCapabilityID() const { return capabilityID; }
    const std::string& getName() const { return name; }
    long getMinValue() const { return minValue; }
    long getMaxValue() const { return maxValue; }
    long getMeanValue() const { return meanValue; }
    
};


#endif
