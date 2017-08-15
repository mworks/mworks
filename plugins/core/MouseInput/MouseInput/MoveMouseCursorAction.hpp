//
//  MoveMouseCursorAction.hpp
//  MouseInput
//
//  Created by Christopher Stawarz on 8/14/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef MoveMouseCursorAction_hpp
#define MoveMouseCursorAction_hpp

#include "MouseInputDevice.h"


BEGIN_NAMESPACE_MW


class MoveMouseCursorAction : public Action {
    
public:
    static const std::string DEVICE;
    static const std::string X_POSITION;
    static const std::string Y_POSITION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit MoveMouseCursorAction(const ParameterValueMap &parameters);
    bool execute() override;
    
private:
    const boost::weak_ptr<MouseInputDevice> weakDevice;
    const VariablePtr xPos;
    const VariablePtr yPos;
    
};


END_NAMESPACE_MW


#endif /* MoveMouseCursorAction_hpp */
