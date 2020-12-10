//
//  TransformStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#ifndef TransformStimulus_hpp
#define TransformStimulus_hpp

#include "MetalStimulus.hpp"


BEGIN_NAMESPACE_MW


class TransformStimulus : public MetalStimulus {
    
public:
    static const std::string X_SIZE;
    static const std::string Y_SIZE;
    static const std::string X_POSITION;
    static const std::string Y_POSITION;
    static const std::string ROTATION;
    static const std::string FULLSCREEN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TransformStimulus(const ParameterValueMap &parameters);
    
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    void getCurrentSize(float &sizeX, float &sizeY) const;
    virtual GLKMatrix4 getCurrentMVPMatrix(const GLKMatrix4 &projectionMatrix) const;
    
    const VariablePtr xoffset;
    const VariablePtr yoffset;
    const VariablePtr xscale;
    const VariablePtr yscale;
    const VariablePtr rotation;
    const bool fullscreen;
    
    float current_posx, current_posy, current_sizex, current_sizey, current_rot;
    float last_posx, last_posy, last_sizex, last_sizey, last_rot;
    
};


END_NAMESPACE_MW


#endif /* TransformStimulus_hpp */
