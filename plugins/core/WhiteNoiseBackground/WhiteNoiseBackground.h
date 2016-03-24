/*
 *  WhiteNoiseBackground.h
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/15/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#ifndef WhiteNoiseBackground_H_
#define WhiteNoiseBackground_H_

#include <boost/noncopyable.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>

#include <MWorksCore/Stimulus.h>
#include <MWorksCore/ComponentInfo.h>
#include <MWorksCore/ParameterValue.h>

using namespace mw;


class WhiteNoiseBackground : public Stimulus, boost::noncopyable {

public:
    static void describeComponent(ComponentInfo &info);
    
    explicit WhiteNoiseBackground(const ParameterValueMap &parameters);

    virtual ~WhiteNoiseBackground() { }
    
    virtual void load(shared_ptr<StimulusDisplay> display);
    virtual void unload(shared_ptr<StimulusDisplay> display);
    virtual void draw(shared_ptr<StimulusDisplay> display);
    virtual Datum getCurrentAnnounceDrawData();
    
    void randomizePixels();
    
private:
    typedef std::pair<GLint, GLint> DisplayDimensions;
    std::map<int, DisplayDimensions> dims;

    static const GLint componentsPerPixel = 4;
    std::vector<GLuint> pixels;
    boost::mutex pixelsMutex;

    boost::rand48 randGen;
    boost::uniform_int<GLubyte> randDist;

};


#endif






















