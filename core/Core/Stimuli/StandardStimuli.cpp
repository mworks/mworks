/**
 * StandardStimuli.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Fixed spacing.  Added copy constructors, 
 *      destructors, and = operator to object that are added 
 *      to ExpandableLists.  Changed constructors to copy objects instead
 *      of just copying pointers.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "StandardStimuli.h"
#include "StandardVariables.h"
#include "ComponentRegistry.h"
//#include "ComponentFactory.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "ParsedColorTrio.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"

#include <boost/regex.hpp>
#include <boost/uuid/sha1.hpp>

#include <stdio.h> // for fopen()
#include <iostream>
#include <iomanip>

#import <Foundation/Foundation.h>


BEGIN_NAMESPACE_MW


#define VERBOSE_STANDARD_STIMULI 0


const std::string BasicTransformStimulus::X_SIZE("x_size");
const std::string BasicTransformStimulus::Y_SIZE("y_size");
const std::string BasicTransformStimulus::X_POSITION("x_position");
const std::string BasicTransformStimulus::Y_POSITION("y_position");
const std::string BasicTransformStimulus::ROTATION("rotation");
const std::string BasicTransformStimulus::ALPHA_MULTIPLIER("alpha_multiplier");


void BasicTransformStimulus::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.addParameter(X_SIZE);
    info.addParameter(Y_SIZE);
    info.addParameter(X_POSITION, "0.0");
    info.addParameter(Y_POSITION, "0.0");
    info.addParameter(ROTATION, "0.0");
    info.addParameter(ALPHA_MULTIPLIER, "1.0");
}


BasicTransformStimulus::BasicTransformStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters),
    xoffset(registerVariable(parameters[X_POSITION])),
    yoffset(registerVariable(parameters[Y_POSITION])),
    xscale(registerVariable(parameters[X_SIZE])),
    yscale(registerVariable(parameters[Y_SIZE])),
    rotation(registerVariable(parameters[ROTATION])),
    alpha_multiplier(registerVariable(parameters[ALPHA_MULTIPLIER]))
{ }


BasicTransformStimulus::BasicTransformStimulus(std::string _tag,
										shared_ptr<Variable> _xoffset, 
										shared_ptr<Variable> _yoffset, shared_ptr<Variable> _xscale,
										shared_ptr<Variable> _yscale, shared_ptr<Variable> _rot,
												 shared_ptr<Variable> _alpha)
															: Stimulus(_tag) {
    xoffset = registerVariable(_xoffset);
    yoffset = registerVariable(_yoffset);
    xscale = registerVariable(_xscale);
    yscale = registerVariable(_yscale);
    rotation = registerVariable(_rot);
	alpha_multiplier = registerVariable(_alpha);
}

							            
void BasicTransformStimulus::setTranslation(shared_ptr<Variable> _x, shared_ptr<Variable> _y) {
    xoffset = _x;
    yoffset = _y;
}
                
void BasicTransformStimulus::setScale(shared_ptr<Variable> _scale) {
    xscale = _scale;
    yscale = _scale;
}
                
void BasicTransformStimulus::setScale(shared_ptr<Variable> _xscale, shared_ptr<Variable> _yscale) {
    xscale = _xscale;
    yscale = _yscale;
}
                
void BasicTransformStimulus::setRotation(shared_ptr<Variable> rot) {
    rotation = rot;
}
                
void BasicTransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    current_posx = *xoffset;
    current_posy = *yoffset;
    current_sizex = *xscale;
    current_sizey = *yscale;
    current_rot = *rotation;
    current_alpha = *alpha_multiplier;
    
    glPushMatrix();
    
    glTranslatef(current_posx, current_posy, 0);
    glRotatef(current_rot, 0, 0, 1);
    glScalef(current_sizex, current_sizey, 1.0);
    glTranslatef(-0.5, -0.5, 0);
    
    drawInUnitSquare(display);
    
    glPopMatrix();
    
    last_posx = current_posx;
    last_posy = current_posy;
    last_sizex = current_sizex;
    last_sizey = current_sizey;
    last_rot = current_rot;
    last_alpha = current_alpha;
}


Datum BasicTransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE,STIM_TYPE_BASICTRANSFORM);
    announceData.addElement(STIM_POSX,last_posx);  
    announceData.addElement(STIM_POSY,last_posy);  
    announceData.addElement(STIM_SIZEX,last_sizex);  
    announceData.addElement(STIM_SIZEY,last_sizey);  
    announceData.addElement(STIM_ROT,last_rot);  
    announceData.addElement(STIM_ALPHA,last_alpha);
    
    return std::move(announceData);
}


void BlankScreen::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.setSignature("stimulus/blank_screen");
    info.addParameter("color", "0.5,0.5,0.5");
}


BlankScreen::BlankScreen(const ParameterValueMap &parameters) :
    Stimulus(parameters)
{
    ParsedColorTrio pct(parameters["color"]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
    
    Datum rval = *r;
    Datum gval = *g;
    Datum bval = *b;
    
    last_r = (float)rval;
    last_g = (float)gval;
    last_b = (float)bval;
}

    
void BlankScreen::draw(shared_ptr<StimulusDisplay> display) {
    
    float _r = (float)*r;
    float _g = (float)*g;
    float _b = (float)*b;
        
    glClearColor(_r,_g,_b,1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    last_r = _r;
    last_g = _g;
    last_b = _b;
    
}


// override of basde class to provide more info
Datum BlankScreen::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE,STIM_TYPE_BLANK);
    announceData.addElement(STIM_COLOR_R,last_r);  
    announceData.addElement(STIM_COLOR_G,last_g);  
    announceData.addElement(STIM_COLOR_B,last_b);
        
    return std::move(announceData);
}


class DevILImageLoader {

public:
    DevILImageLoader();
    ~DevILImageLoader();
    void load(const std::string &filename, int &width, int &height, std::string &fileHash);
    GLuint buildTexture();
    
private:
    static void initializeIL();

    static void throwILError(const std::string &message) {
        throw SimpleException(message, iluErrorString(ilGetError()));
    }

    static bool ilInitialized;
    
    NSAutoreleasePool *pool;
    ILuint ilImageName;

};


DevILImageLoader::DevILImageLoader() :
    ilImageName(0)
{
    pool = [[NSAutoreleasePool alloc] init];
}


DevILImageLoader::~DevILImageLoader() {
    if (0 != ilImageName) {
        ilDeleteImage(ilImageName);
    }
    [pool drain];
}


void DevILImageLoader::load(const std::string &filename, int &width, int &height, std::string &fileHash) {
    mprintf("Loading image %s", filename.c_str());
    
    if (0 != ilImageName) {
        throw SimpleException("Cannot load image", "Image already loaded");
    }
    
    NSError *errorPtr = nil;
    NSData *imageData = [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:(filename.c_str())]
                                               options:0
                                                 error:&errorPtr];
    if (nil != errorPtr) {
		throw SimpleException("Cannot read image file", [[errorPtr localizedDescription] UTF8String]);
    }
    
    if (!ilInitialized) {
        initializeIL();
    }

    ilImageName = ilGenImage();
    ilBindImage(ilImageName);
    
    if (IL_FALSE == ilLoadL(ilTypeFromExt(filename.c_str()), [imageData bytes], [imageData length]) ||
        IL_FALSE == ilConvertImage(IL_RGBA, IL_FLOAT))
    {
        throwILError("Cannot load image");
    }
    
    width = ilGetInteger(IL_IMAGE_WIDTH);
    height = ilGetInteger(IL_IMAGE_HEIGHT);

    //
    // Compute the SHA-1 message digest of the raw file data, convert it to a hex string, and copy it to fileHash
    //
    
    boost::uuids::detail::sha1 sha;
    sha.process_bytes([imageData bytes], [imageData length]);
    
    constexpr std::size_t digestSize = 5;
    unsigned int digest[digestSize];
    sha.get_digest(digest);
    
    std::ostringstream os;
    os.fill('0');
    os << std::hex;
    for (int i = 0; i < digestSize; i++) {
        os << std::setw(2 * sizeof(unsigned int)) << digest[i];
    }
    
    fileHash = os.str();
}


GLuint DevILImageLoader::buildTexture() {
    if (0 == ilImageName) {
        throw SimpleException("Cannot build image texture", "Image not loaded");
    }

    ilBindImage(ilImageName);

    GLuint texture = ilutGLBindMipmaps();
    if (0 == texture) {
        throwILError("Cannot build image texture");
    }
    glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture
    
    return texture;
}


void DevILImageLoader::initializeIL() {
    mprintf("initializing image loader facility");
    ilInit();
    iluInit();
    ilutInit();
    ilutRenderer(ILUT_OPENGL);
    ilutEnable(ILUT_OPENGL_CONV);
    ilInitialized = true;   
}


bool DevILImageLoader::ilInitialized = false;


const std::string ImageStimulus::PATH("path");


void ImageStimulus::describeComponent(ComponentInfo &info) {
    BasicTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/image_file");
    info.addParameter(PATH);
}


ImageStimulus::ImageStimulus(const ParameterValueMap &parameters) :
    BasicTransformStimulus(parameters),
    width(0),
    height(0)
{
    namespace bf = boost::filesystem;
    bf::path full_path(parameters[PATH]);
	
	if (bf::is_directory(full_path)) {
		throw SimpleException("Path is a directory", full_path.string());
	}
    
    filename = full_path.string();
}


std::string ImageStimulus::getFilename() {
        return filename;
}
       
void ImageStimulus::load(shared_ptr<StimulusDisplay> display) {
    if(loaded){
		return;
	}
    
    OpenGLContextLock ctxLock = display->setCurrent(0);  // Need an active OpenGL context when ilutInit() is called
    DevILImageLoader loader;
    loader.load(filename, width, height, fileHash);
	
	// TODO: this needs clean up.  We are counting on all of the contexts
	// in the stimulus display to have the exact same history.  Ideally, this
	// should be true, but we should eventually be robust in case it isn't
	texture_maps.clear();
    
    int i = 1;
    while (true) {
		GLuint texture_map = loader.buildTexture();
		
        texture_maps.push_back(texture_map);
//		fprintf(stderr, "Loaded texture map %u into context %d\n", (unsigned int)texture_map, i);fflush(stderr);
		if(texture_map){
			mprintf("Image loaded into texture_map %d", texture_map);
		}
        
        if (i >= display->getNContexts())
            break;
        
        ctxLock = display->setCurrent(i);
        i++;
	}
	
  
  loaded = true;
  
	// TODO: update to work with lists
	// TODO: this is wrong, because texture_map is unsigned...
	/*if(texture_map > 0) {
        loaded = true;
    } else {
        loaded = false;
    }*/
}

void ImageStimulus::unload(shared_ptr<StimulusDisplay> display) {
    if (!loaded) {
        return;
    }
    
    for (int i = 0; i < display->getNContexts(); i++) {
        OpenGLContextLock ctxLock = display->setCurrent(i);
        glDeleteTextures(1, &(texture_maps[i]));
        mprintf("Image unloaded from texture_map %d", texture_maps[i]);
	}
    
    texture_maps.clear();
    
    loaded = false;
}

void ImageStimulus::drawInUnitSquare(shared_ptr<StimulusDisplay> display) {
    double aspect = (double)width / (double)height;
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_maps[display->getCurrentContextIndex()]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glBegin(GL_QUADS);
    
    glColor4f(1., 1., 1., current_alpha);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    if(aspect > 1) {
        
        glTexCoord2f(0.0,0.0);
        glVertex3f(0.0,(0.5-0.5/aspect),0.0);
        
        glTexCoord2f(1.0,0.0);
        glVertex3f(1.0,(0.5-0.5/aspect),0.0);
        
        glTexCoord2f(1.0,1.0);
        glVertex3f(1.0,(0.5-0.5/aspect) + 1.0/aspect,0.0);
        
        glTexCoord2f(0.0,1.0);
        glVertex3f(0.0,(0.5-0.5/aspect) + 1.0/aspect,0.0);
    } else {
        
        glTexCoord2f(0.0,0.0);
        glVertex3f((1.0 - aspect)/2.0,0.0,0.0);
        
        glTexCoord2f(1.0,0.0);
        glVertex3f((1.0 - aspect)/2.0 + aspect,0.0,0.0);
        
        glTexCoord2f(1.0,1.0);
        glVertex3f((1.0 - aspect)/2.0 + aspect,1.0,0.0);
        
        glTexCoord2f(0.0,1.0);
        glVertex3f((1.0 - aspect)/2.0,1.0,0.0);
    }
    
    glEnd();
    
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}


Datum ImageStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = BasicTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE,STIM_TYPE_IMAGE);
    announceData.addElement(STIM_FILENAME,filename);  
    announceData.addElement(STIM_FILE_HASH,fileHash);  
    
    return std::move(announceData);
}


const std::string ColoredTransformStimulus::COLOR("color");


void ColoredTransformStimulus::describeComponent(ComponentInfo &info) {
    BasicTransformStimulus::describeComponent(info);
    info.addParameter(COLOR, "1.0,1.0,1.0");
}


ColoredTransformStimulus::ColoredTransformStimulus(const ParameterValueMap &parameters) :
    BasicTransformStimulus(parameters)
{
    ParsedColorTrio pct(parameters[COLOR]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
}


void ColoredTransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    current_r = *r;
    current_g = *g;
    current_b = *b;
    
    BasicTransformStimulus::draw(display);
    
    last_r = current_r;
    last_g = current_g;
    last_b = current_b;
}


Datum ColoredTransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = BasicTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_COLOR_R,last_r);
    announceData.addElement(STIM_COLOR_G,last_g);
    announceData.addElement(STIM_COLOR_B,last_b);
    
    return std::move(announceData);
}


void RectangleStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/rectangle");
}


void RectangleStimulus::drawInUnitSquare(shared_ptr<StimulusDisplay> display) {
    glColor4f(current_r, current_g, current_b, current_alpha);
	  
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    glBegin(GL_QUADS);
		
        glVertex3f(0.0,0.0,0.0);
		glVertex3f(1.0,0.0,0.0);
        glVertex3f(1.0,1.0,0.0);
        glVertex3f(0.0,1.0,0.0);

    glEnd();
    
	glDisable(GL_BLEND);
}


Datum RectangleStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "rectangle");
    return std::move(announceData);
}


void CircleStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/circle");
}


void CircleStimulus::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
    pixelDensity.clear();
    
    GLdouble xMin, xMax, yMin, yMax;
    GLint width, height;
    
    display->getDisplayBounds(xMin, xMax, yMin, yMax);
    
    for (int i = 0; i < display->getNContexts(); i++) {
        OpenGLContextLock ctxLock = display->setCurrent(i);
        display->getCurrentViewportSize(width, height);
        pixelDensity.push_back(double(width) / (xMax - xMin));
    }
    
    loaded = true;
}


#define TWO_PI (2.0 * M_PI)

void CircleStimulus::drawInUnitSquare(shared_ptr<StimulusDisplay> display) {
    glColor4f(current_r, current_g, current_b, current_alpha);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // The formula for the number of sections is borrowed from http://slabode.exofire.net/circle_draw.shtml
    double radius = std::max(current_sizex, current_sizey) / 2.0;
    int sections = 10 * std::sqrt(radius * pixelDensity.at(display->getCurrentContextIndex()));
    
    glBegin(GL_TRIANGLE_FAN);
    
    glVertex2f(0.5, 0.5);
    
    for (int i=0; i<=sections; ++i)
    {
        glVertex2f(0.5 + (0.5 * std::cos(i * TWO_PI / sections)),
                   0.5 + (0.5 * std::sin(i * TWO_PI / sections)));
    }
    
    glEnd();
    
    glDisable(GL_BLEND);
}


Datum CircleStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "circle");
    return std::move(announceData);
}


END_NAMESPACE_MW























