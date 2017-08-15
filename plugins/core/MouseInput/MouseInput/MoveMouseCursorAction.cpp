//
//  MoveMouseCursorAction.cpp
//  MouseInput
//
//  Created by Christopher Stawarz on 8/14/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "MoveMouseCursorAction.hpp"


BEGIN_NAMESPACE_MW


const std::string MoveMouseCursorAction::DEVICE("device");
const std::string MoveMouseCursorAction::X_POSITION("x_position");
const std::string MoveMouseCursorAction::Y_POSITION("y_position");


void MoveMouseCursorAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    
    info.setSignature("action/move_mouse_cursor");
    
    info.addParameter(DEVICE);
    info.addParameter(X_POSITION);
    info.addParameter(Y_POSITION);
}


MoveMouseCursorAction::MoveMouseCursorAction(const ParameterValueMap &parameters) :
    Action(parameters),
    weakDevice(parameters[DEVICE].getRegistry()->getObject<MouseInputDevice>(parameters[DEVICE].str())),
    xPos(parameters[X_POSITION]),
    yPos(parameters[Y_POSITION])
{
    if (weakDevice.expired()) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              "Device is not a mouse input device",
                              parameters[DEVICE].str());
    }
}


bool MoveMouseCursorAction::execute() {
    if (auto sharedDevice = weakDevice.lock()) {
        sharedDevice->moveMouseCursor(xPos->getValue().getFloat(), yPos->getValue().getFloat());
    }
    return true;
}


END_NAMESPACE_MW


























