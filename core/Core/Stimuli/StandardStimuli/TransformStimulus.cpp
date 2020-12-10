//
//  TransformStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#include "TransformStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string TransformStimulus::X_SIZE("x_size");
const std::string TransformStimulus::Y_SIZE("y_size");
const std::string TransformStimulus::X_POSITION("x_position");
const std::string TransformStimulus::Y_POSITION("y_position");
const std::string TransformStimulus::ROTATION("rotation");
const std::string TransformStimulus::FULLSCREEN("fullscreen");


void TransformStimulus::describeComponent(ComponentInfo &info) {
    MetalStimulus::describeComponent(info);
    info.addParameter(X_SIZE, false);
    info.addParameter(Y_SIZE, false);
    info.addParameter(X_POSITION, "0.0");
    info.addParameter(Y_POSITION, "0.0");
    info.addParameter(ROTATION, "0.0");
    info.addParameter(FULLSCREEN, "NO");
}


TransformStimulus::TransformStimulus(const ParameterValueMap &parameters) :
    MetalStimulus(parameters),
    xoffset(registerVariable(parameters[X_POSITION])),
    yoffset(registerVariable(parameters[Y_POSITION])),
    xscale(registerOptionalVariable(optionalVariable(parameters[X_SIZE]))),
    yscale(registerOptionalVariable(optionalVariable(parameters[Y_SIZE]))),
    rotation(registerVariable(parameters[ROTATION])),
    fullscreen(parameters[FULLSCREEN])
{
    if (!(xscale || yscale || fullscreen)) {
        throw SimpleException("Either x_size or y_size must be specified");
    }
}


void TransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    if (!fullscreen) {
        current_posx = xoffset->getValue().getFloat();
        current_posy = yoffset->getValue().getFloat();
        getCurrentSize(current_sizex, current_sizey);
        current_rot = rotation->getValue().getFloat();
    }
    
    MetalStimulus::draw(display);
    
    if (!fullscreen) {
        last_posx = current_posx;
        last_posy = current_posy;
        last_sizex = current_sizex;
        last_sizey = current_sizey;
        last_rot = current_rot;
    }
}


Datum TransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = MetalStimulus::getCurrentAnnounceDrawData();
    
    if (fullscreen) {
        announceData.addElement(STIM_FULLSCREEN, true);
    } else {
        announceData.addElement(STIM_POSX, last_posx);
        announceData.addElement(STIM_POSY, last_posy);
        announceData.addElement(STIM_SIZEX, last_sizex);
        announceData.addElement(STIM_SIZEY, last_sizey);
        announceData.addElement(STIM_ROT, last_rot);
    }
    
    return announceData;
}


void TransformStimulus::getCurrentSize(float &sizeX, float &sizeY) const {
    if (xscale) {
        sizeX = xscale->getValue().getFloat();
        if (!yscale) {
            sizeY = sizeX;
        }
    }
    if (yscale) {
        sizeY = yscale->getValue().getFloat();
        if (!xscale) {
            sizeX = sizeY;
        }
    }
}


GLKMatrix4 TransformStimulus::getCurrentMVPMatrix(const GLKMatrix4 &projectionMatrix) const {
    GLKMatrix4 currentMVPMatrix;
    
    if (fullscreen) {
        currentMVPMatrix = GLKMatrix4MakeScale(2.0, 2.0, 1.0);
    } else {
        currentMVPMatrix = GLKMatrix4Translate(projectionMatrix, current_posx, current_posy, 0.0);
        currentMVPMatrix = GLKMatrix4Rotate(currentMVPMatrix, GLKMathDegreesToRadians(current_rot), 0.0, 0.0, 1.0);
        currentMVPMatrix = GLKMatrix4Scale(currentMVPMatrix, current_sizex, current_sizey, 1.0);
    }
    
    return GLKMatrix4Translate(currentMVPMatrix, -0.5, -0.5, 0.0);
}


END_NAMESPACE_MW
