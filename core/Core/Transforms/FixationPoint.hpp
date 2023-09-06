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
    static const std::string ACTIVE_WHEN_HIDDEN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FixationPoint(const ParameterValueMap &parameters);
    
    void setVisible(bool newvis) override;
    Datum getCurrentAnnounceDrawData() override;
    ExpandableList<Datum> * getGoldStandardValues() override;
    
private:
    const bool activeWhenHidden;
    
};


class CircularFixationPoint : public EllipseStimulus, public CircularRegionTrigger, public GoldStandard {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit CircularFixationPoint(const ParameterValueMap &parameters);
    
    void setVisible(bool newvis) override;
    Datum getCurrentAnnounceDrawData() override;
    ExpandableList<Datum> * getGoldStandardValues() override;
    
private:
    const bool activeWhenHidden;
    
};


END_NAMESPACE_MW


#endif /* FixationPoint_hpp */
