//
//  RenderActions.hpp
//  RenderActions
//
//  Created by Christopher Stawarz on 3/19/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef RenderActions_hpp
#define RenderActions_hpp


BEGIN_NAMESPACE_MW


class RenderActions : public StandardDynamicStimulus, boost::noncopyable {
    
public:
    static const std::string ELAPSED_TIME;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RenderActions(const ParameterValueMap &parameters);
    
    RenderingMode getRenderingMode() const override { return RenderingMode::None; }
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  boost::shared_ptr<Component> child) override;
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void drawFrame(boost::shared_ptr<StimulusDisplay> display) override;
    
    const boost::shared_ptr<Variable> elapsedTime;
    std::vector<boost::shared_ptr<Action>> actions;
    
};


END_NAMESPACE_MW


#endif /* RenderActions_hpp */
