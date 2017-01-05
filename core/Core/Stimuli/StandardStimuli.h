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


BEGIN_NAMESPACE_MW


// a stimulus object with a position stored
// this is the base object from which many others should derive
class BasicTransformStimulus : public Stimulus {
	
protected:
    const shared_ptr<Variable> xoffset;
    const shared_ptr<Variable> yoffset;
    
    const shared_ptr<Variable> xscale;
    const shared_ptr<Variable> yscale;
    
    const shared_ptr<Variable> rotation; // planar rotation added in for free
    const shared_ptr<Variable> alpha_multiplier;
    
    float current_posx, current_posy, current_sizex, current_sizey, current_rot, current_alpha;
    float last_posx, last_posy, last_sizex, last_sizey, last_rot, last_alpha;
    
public:
    static const std::string X_SIZE;
    static const std::string Y_SIZE;
    static const std::string X_POSITION;
    static const std::string Y_POSITION;
    static const std::string ROTATION;
    static const std::string ALPHA_MULTIPLIER;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit BasicTransformStimulus(const Map<ParameterValue> &parameters);
    BasicTransformStimulus(const std::string &_tag,
                           const shared_ptr<Variable> &_xoffset,
                           const shared_ptr<Variable> &_yoffset,
                           const shared_ptr<Variable> &_xscale,
                           const shared_ptr<Variable> &_yscale,
                           const shared_ptr<Variable> &_rot,
                           const shared_ptr<Variable> &_alpha);
    
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
    virtual void drawInUnitSquare(shared_ptr<StimulusDisplay> display) = 0;

};


class ImageStimulus : public BasicTransformStimulus, boost::noncopyable {
protected:
    std::string filename;
    std::string fileHash;
    vector<GLuint> texture_maps;
    int width, height;
    
public:
    static const std::string PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ImageStimulus(const Map<ParameterValue> &parameters);
    
    const std::string& getFilename() { return filename; }
    
    void load(shared_ptr<StimulusDisplay> display) override;
    void unload(shared_ptr<StimulusDisplay> display) override;
    void drawInUnitSquare(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
};


class ColoredTransformStimulus : public BasicTransformStimulus {
    
protected:
    shared_ptr<Variable> r;
    shared_ptr<Variable> g;
    shared_ptr<Variable> b;
    
    float current_r, current_g, current_b;
    float last_r, last_g, last_b;
    
public:
    static const std::string COLOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ColoredTransformStimulus(const Map<ParameterValue> &parameters);
    
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
};


class RectangleStimulus : public ColoredTransformStimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using ColoredTransformStimulus::ColoredTransformStimulus;
    
    void drawInUnitSquare(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
};


class CircleStimulus : public ColoredTransformStimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using ColoredTransformStimulus::ColoredTransformStimulus;
    
    void load(shared_ptr<StimulusDisplay> display) override;
    void drawInUnitSquare(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    std::vector<double> pixelDensity;
    
};


class BlankScreen : public Stimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit BlankScreen(const Map<ParameterValue> &parameters);
    
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    shared_ptr<Variable> r;
    shared_ptr<Variable> g;
    shared_ptr<Variable> b;
    float last_r,last_g,last_b;
    
};


END_NAMESPACE_MW


#endif























