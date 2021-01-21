//
//  FixationPoint.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/21/21.
//

#ifndef FixationPoint_hpp
#define FixationPoint_hpp

#include "EyeCalibrators.h"
#include "RectangleStimulus.hpp"
#include "EllipseStimulus.hpp"
#include "Trigger.h"


BEGIN_NAMESPACE_MW


class FixationPoint : public RectangleStimulus, public SquareRegionTrigger, public GoldStandard {
    
public:
    static const std::string TRIGGER_WIDTH;
    static const std::string TRIGGER_WATCH_X;
    static const std::string TRIGGER_WATCH_Y;
    static const std::string TRIGGER_FLAG;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FixationPoint(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    ExpandableList<Datum> * getGoldStandardValues() override;
    
};


class CircularFixationPoint : public EllipseStimulus, public CircularRegionTrigger, public GoldStandard {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit CircularFixationPoint(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    ExpandableList<Datum> * getGoldStandardValues() override;
    
};


END_NAMESPACE_MW


#endif /* FixationPoint_hpp */
