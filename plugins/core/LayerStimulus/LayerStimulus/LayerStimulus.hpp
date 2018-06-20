//
//  LayerStimulus.hpp
//  LayerStimulus
//
//  Created by Christopher Stawarz on 6/19/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#ifndef LayerStimulus_hpp
#define LayerStimulus_hpp


BEGIN_NAMESPACE_MW


class LayerStimulus : public Stimulus, boost::noncopyable {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit LayerStimulus(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  boost::shared_ptr<Component> child) override;
    
    void freeze(bool shouldFreeze = true) override;
    
    void load(boost::shared_ptr<StimulusDisplay> display) override;
    void unload(boost::shared_ptr<StimulusDisplay> display) override;
    bool needDraw(boost::shared_ptr<StimulusDisplay> display) override;
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    std::vector<boost::shared_ptr<Stimulus>> children;
    
    GLuint program = 0;
    GLuint vertexPositionBuffer = 0;
    GLuint texCoordsBuffer = 0;
    GLuint vertexArray = 0;
    GLuint framebufferTexture = 0;
    GLuint framebuffer = 0;
    
};


END_NAMESPACE_MW


#endif /* LayerStimulus_hpp */





















