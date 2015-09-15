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
#include "ConstantVariable.h"
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

#include <stdio.h> // for fopen()
#include <openssl/sha.h>
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


BasicTransformStimulus::BasicTransformStimulus(
								const BasicTransformStimulus& tocopy) :
										Stimulus((const Stimulus &)tocopy){
	xoffset = tocopy.xoffset;
	yoffset = tocopy.yoffset;
	xscale = tocopy.xscale;
	yscale = tocopy.yscale;
	rotation = tocopy.rotation;
	alpha_multiplier = tocopy.alpha_multiplier;
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
                
void BasicTransformStimulus::draw(shared_ptr<StimulusDisplay>  display) {
    draw(display, *xoffset, *yoffset, *xscale, *yscale);
}
                
void BasicTransformStimulus::draw(shared_ptr<StimulusDisplay>  display,float x, float y, 
                                                    float sizex, float sizey) {
    
    float rot = (float)(*rotation);
    
    glPushMatrix();
    
	glTranslatef(x, y,0);
	glRotatef(rot, 0,0,1);
	glScalef(sizex, sizey, 1.0); // scale it up
    glTranslatef(-0.5, -0.5, 0);
    					
    drawInUnitSquare(display);
                    
    glPopMatrix();
    
    // save these as last drawn values
    last_posx = x;
    last_posy = y;
    last_sizex = sizex;
    last_sizey = sizey;
    last_rot = rot;
    
}


// override of basde class to provide more info
Datum BasicTransformStimulus::getCurrentAnnounceDrawData() {
    
    Datum announceData(M_DICTIONARY, 8);
    announceData.addElement(STIM_NAME,getTag());        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_BASICTRANSFORM);  
    announceData.addElement(STIM_POSX,last_posx);  
    announceData.addElement(STIM_POSY,last_posy);  
    announceData.addElement(STIM_SIZEX,last_sizex);  
    announceData.addElement(STIM_SIZEY,last_sizey);  
    announceData.addElement(STIM_ROT,last_rot);  
        
    return (announceData);
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
    
    Datum announceData(M_DICTIONARY, 6);
    announceData.addElement(STIM_NAME,getTag());        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_BLANK);  
    announceData.addElement(STIM_COLOR_R,last_r);  
    announceData.addElement(STIM_COLOR_G,last_g);  
    announceData.addElement(STIM_COLOR_B,last_b);
        
    return (announceData);
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

    // Compute the SHA-1 message digest of the raw file data, convert it to a hex string, and copy it to fileHash
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    unsigned char *hash = SHA1((unsigned char*)[imageData bytes], [imageData length], NULL);
#pragma clang diagnostic pop
    std::ostringstream os;
    os.fill('0');
    os << std::hex;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        os << std::setw(2) << (unsigned int)(hash[i]);
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


ImageStimulus::ImageStimulus(ImageStimulus& copy):
					BasicTransformStimulus((BasicTransformStimulus&) copy) { 
    
	filename = copy.getFilename();
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
    if (loaded) {
        
        glActiveTextureARB(GL_TEXTURE0_ARB); 
        glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture_maps[display->getCurrentContextIndex()]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glEnable (GL_BLEND); 
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
								
        glBegin(GL_QUADS);
		
		float a = alpha_multiplier->getValue().getFloat();
		glColor4f(1., 1., 1., a);

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
		
		
		glBindTexture(GL_TEXTURE_2D, 0); // unbind that fucker
        
        last_alpha = a;
		
		
		//glActiveTexture(0);
		
    } else {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Stimulus image is not loaded.  Displaying nothing.");
    }
}

// override of basde class to provide more info
Datum ImageStimulus::getCurrentAnnounceDrawData() {
    
    //mprintf("getting announce DRAW data for image stimulus %s",tag );
    
    Datum announceData = BasicTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE,STIM_TYPE_IMAGE);
    announceData.addElement(STIM_FILENAME,filename);  
    announceData.addElement(STIM_FILE_HASH,fileHash);  
    announceData.addElement(STIM_ALPHA,last_alpha);
    
    return (announceData);
}


const std::string RectangleStimulus::COLOR("color");


void RectangleStimulus::describeComponent(ComponentInfo &info) {
    BasicTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/rectangle");
    info.addParameter(COLOR, "1.0,1.0,1.0");
}


RectangleStimulus::RectangleStimulus(const ParameterValueMap &parameters) :
    BasicTransformStimulus(parameters)
{
    ParsedColorTrio pct(parameters[COLOR]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
}


RectangleStimulus::RectangleStimulus(const RectangleStimulus &tocopy) : 
				BasicTransformStimulus((const BasicTransformStimulus&)tocopy){
	r = tocopy.r;
	g = tocopy.g;
	b = tocopy.b;
}


void RectangleStimulus::drawInUnitSquare(shared_ptr<StimulusDisplay> display) {
    
     // draw point at desired location with desired color
     // fill a (0,0) (1,1) box with the right color
    if(r == NULL || g == NULL || b == NULL ){
		merror(M_DISPLAY_MESSAGE_DOMAIN,
				"NULL color variable in RectangleStimulus.");
	}
	
	
    // get current values in these variables.
	GLfloat _r = (float)(*r);
	GLfloat _g = (float)(*g);
	GLfloat _b = (float)(*b);
	GLfloat _a = (float)(*alpha_multiplier);
	   
	glBindTexture(GL_TEXTURE_2D, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	
    glBegin(GL_QUADS);
		
		//mprintf("fixpoint r: %g, g: %g, b: %g", red, green, blue);
	
        //glColor3f(_r,_g,_b);
		glColor4f(_r, _g, _b, _a);
        glVertex3f(0.0,0.0,0.0);
		glVertex3f(1.0,0.0,0.0);
        glVertex3f(1.0,1.0,0.0);
        glVertex3f(0.0,1.0,0.0);

    glEnd();
    
	glDisable(GL_BLEND);
    
    last_r = _r;
    last_g = _g;
    last_b = _b;
    last_alpha = _a;
    

}

// override of base class to provide more info
Datum RectangleStimulus::getCurrentAnnounceDrawData() {
    
    //mprintf("getting announce DRAW data for rectangle stimulus %s",tag );
    
    Datum announceData = BasicTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "rectangle");
    announceData.addElement(STIM_COLOR_R,last_r);
    announceData.addElement(STIM_COLOR_G,last_g);  
    announceData.addElement(STIM_COLOR_B,last_b);  
    announceData.addElement(STIM_ALPHA, last_alpha);
    
    return (announceData);
}


END_NAMESPACE_MW























