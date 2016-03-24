/*
 *  WhiteNoiseBackground.cpp
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/15/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "WhiteNoiseBackground.h"

#include <algorithm>
#include <limits>

#include <MWorksCore/StandardVariables.h>


void WhiteNoiseBackground::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.setSignature("stimulus/white_noise_background");
}


WhiteNoiseBackground::WhiteNoiseBackground(const ParameterValueMap &parameters) :
    Stimulus(parameters),
    randGen(mw::Clock::instance()->getSystemTimeUS()),
    randDist(std::numeric_limits<GLubyte>::min(), std::numeric_limits<GLubyte>::max())
{ }


void WhiteNoiseBackground::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;

    GLint maxWidth = 0;
    GLint maxHeight = 0;
    
    for (int i = 0; i < display->getNContexts(); i++) {
        OpenGLContextLock ctxLock = display->setCurrent(i);

        GLint width, height;
        display->getCurrentViewportSize(width, height);
        dims[i] = DisplayDimensions(width, height);
        
        maxWidth = std::max(width, maxWidth);
        maxHeight = std::max(height, maxHeight);
    }

    pixels.clear();
    pixels.assign(maxWidth * maxHeight, std::numeric_limits<GLuint>::max());
    
    loaded = true;
}


void WhiteNoiseBackground::unload(shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;
    
    dims.clear();
    pixels.clear();
    
    loaded = false;
}


void WhiteNoiseBackground::draw(shared_ptr<StimulusDisplay> display) {
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    DisplayDimensions &currentDims = dims[display->getCurrentContextIndex()];
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);  // Disable any previous texture binding
    
    {
        boost::mutex::scoped_lock lock(pixelsMutex);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA8,
                     currentDims.first,
                     currentDims.second,
                     0,
                     GL_BGRA,
                     GL_UNSIGNED_INT_8_8_8_8_REV,
                     &(pixels[0]));
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    GLdouble left, right, bottom, top;
    display->getDisplayBounds(left, right, bottom, top);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0.0, 0.0);
    glVertex3f(left, bottom, 0.0);
    
    glTexCoord2f(1.0, 0.0);
    glVertex3f(right, bottom, 0.0);
    
    glTexCoord2f(1.0, 1.0);
    glVertex3f(right, top, 0.0);
    
    glTexCoord2f(0.0, 1.0);
    glVertex3f(left, top, 0.0);
    
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopClientAttrib();
}


Datum WhiteNoiseBackground::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
	announceData.addElement(STIM_TYPE, "white_noise_background");
    return announceData;
}


void WhiteNoiseBackground::randomizePixels() {
    boost::mutex::scoped_lock lock(pixelsMutex);

    for (size_t i = 0; i < pixels.size(); i++) {
        GLubyte randVal = randDist(randGen);
        GLubyte *pix = (GLubyte *)(&(pixels[i]));
        for (size_t j = 0; j < componentsPerPixel - 1; j++) {
            pix[j] = randVal;
        }
    }
}























