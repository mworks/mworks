/**
 * StandardStimuli.h
 *
 * Description:
 *
 * NOTE:  All objects that inherit from Stimulus and contain pointer data 
 * must implement a copy constructor and a destructor.  It is also advised to
 * make a private = operator.
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Fixed spacing.  Added copy constructors, 
 *                          destructors, and = operator to object that are
 *                          added to ExpandableLists.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#ifndef _STANDARD_STIMULI_H
#define _STANDARD_STIMULI_H

#include "Stimulus.h"
#include "Experiment.h"
#include "ComponentFactory.h"


BEGIN_NAMESPACE_MW


// a stimulus object with a position stored
// this is the base object from which many others should derive
class BasicTransformStimulus : public Stimulus {
	
protected:
    shared_ptr<Variable> xoffset;
    shared_ptr<Variable> yoffset;
    
    shared_ptr<Variable> xscale;
    shared_ptr<Variable> yscale;
    
    shared_ptr<Variable> rotation; // planar rotation added in for free
    shared_ptr<Variable> alpha_multiplier;
    
    bool use_screen_units;
    
    // JJD added these July 2006 to keep track of what was actually done for announcing things
    float last_posx, last_posy, last_sizex, last_sizey, last_rot;
    
public:
    static const std::string X_SIZE;
    static const std::string Y_SIZE;
    static const std::string X_POSITION;
    static const std::string Y_POSITION;
    static const std::string ROTATION;
    static const std::string ALPHA_MULTIPLIER;
    static const std::string USE_SCREEN_UNITS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit BasicTransformStimulus(const Map<ParameterValue> &parameters);
    BasicTransformStimulus(std::string _tag, 
                           shared_ptr<Variable> _xoffset, 
                           shared_ptr<Variable> _yoffset,
                           shared_ptr<Variable> _xscale, 
                           shared_ptr<Variable> _yscale, 
                           shared_ptr<Variable> _rot,
                           shared_ptr<Variable> _alpha);
    BasicTransformStimulus(const BasicTransformStimulus& tocopy);
    virtual ~BasicTransformStimulus() { }
    
    virtual void setTranslation(shared_ptr<Variable> _x, 
                                shared_ptr<Variable> _y);
    virtual void setScale(shared_ptr<Variable> _scale);
    virtual void setScale(shared_ptr<Variable> _xscale, 
                          shared_ptr<Variable> _yscale);
    virtual void setRotation(shared_ptr<Variable> rot);
    virtual void draw(shared_ptr<StimulusDisplay>  display);
    virtual void draw(shared_ptr<StimulusDisplay>  display,float x, float y,
                      float sizex, float sizey);
    
    virtual void drawInUnitSquare(shared_ptr<StimulusDisplay> display) = 0;
    
    virtual Datum getCurrentAnnounceDrawData();    
	
    virtual shared_ptr<Variable> getXScale(){ return xscale; }
    virtual shared_ptr<Variable> getYScale(){ return yscale; }
    virtual shared_ptr<Variable> getXOffset(){ return xoffset; }
    virtual shared_ptr<Variable> getYOffset(){ return yoffset; }

};


// Image file based stimulus
class ImageStimulus : public BasicTransformStimulus {
protected:
    std::string filename;
    std::string fileHash;
    vector<GLuint> texture_maps;
    int width, height;
    
public:
    static const std::string PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ImageStimulus(const Map<ParameterValue> &parameters);
    ImageStimulus(ImageStimulus& copy);
    virtual ~ImageStimulus() { }
    
    std::string getFilename();
    virtual void drawInUnitSquare(shared_ptr<StimulusDisplay> display);
    virtual void load(shared_ptr<StimulusDisplay> display);
    virtual void unload(shared_ptr<StimulusDisplay> display);
    virtual Datum getCurrentAnnounceDrawData();
};


// Simple point (e.g. for fixation)
class PointStimulus : public BasicTransformStimulus {
protected:
    shared_ptr<Variable> r;
    shared_ptr<Variable> g;
    shared_ptr<Variable> b;
    float last_r,last_g,last_b;
    
public:
    static const std::string COLOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit PointStimulus(const Map<ParameterValue> &parameters);
    PointStimulus(const PointStimulus &tocopy);
    virtual ~PointStimulus() { }
    
    virtual void drawInUnitSquare(shared_ptr<StimulusDisplay> display);
    virtual Datum getCurrentAnnounceDrawData();
    
};


class BlankScreen : public Stimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit BlankScreen(const Map<ParameterValue> &parameters);
    virtual ~BlankScreen() { }
    
    virtual void draw(shared_ptr<StimulusDisplay> display);
    virtual Datum getCurrentAnnounceDrawData();
    
protected:
    shared_ptr<Variable> r;
    shared_ptr<Variable> g;
    shared_ptr<Variable> b;
    float last_r,last_g,last_b;
    
};


END_NAMESPACE_MW


#endif























