//
//  RectangleStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#include "RectangleStimulus.hpp"


BEGIN_NAMESPACE_MW


void RectangleStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/rectangle");
}


Datum RectangleStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "rectangle");
    return announceData;
}


END_NAMESPACE_MW
