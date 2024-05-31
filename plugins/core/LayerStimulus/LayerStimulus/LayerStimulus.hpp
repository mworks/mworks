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


class LayerStimulus : public AlphaBlendedTransformStimulus {
    
public:
    static const std::string MAX_SIZE_X;
    static const std::string MAX_SIZE_Y;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit LayerStimulus(const ParameterValueMap &parameters);
    ~LayerStimulus();
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  boost::shared_ptr<Component> child) override;
    
    void freeze(bool shouldFreeze = true) override;
    
    void load(boost::shared_ptr<StimulusDisplay> display) override;
    void unload(boost::shared_ptr<StimulusDisplay> display) override;
    void setVisible(bool newVisible) override;
    bool needDraw(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    const VariablePtr maxSizeX;
    const VariablePtr maxSizeY;
    
    std::vector<boost::shared_ptr<Stimulus>> children;
    
    float currentMaxSizeX, currentMaxSizeY;
    
    int framebufferID;
    id<MTLTexture> framebufferTexture;
    
};


END_NAMESPACE_MW


#endif /* LayerStimulus_hpp */
