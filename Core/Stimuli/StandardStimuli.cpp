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

#include <boost/regex.hpp>

#include <stdio.h> // for fopen()
#include <openssl/sha.h>
#include <iostream>
#include <iomanip>

#import <Foundation/Foundation.h>

using namespace mw;


#define VERBOSE_STANDARD_STIMULI 0

/*StimulusGroupReference::StimulusGroupReference(char *_tag, StimulusGroup *_group, 
															Variable *_index) : Stimulus(_tag){
	group = _group;
	index = _index;
}



void StimulusGroupReference::load(shared_ptr<StimulusDisplay> display){
    
	//(stimulus_nodes->getElement((int)(*index)))->load();
	if(index != NULL){
        mprintf("loading stimulus (%d)", GlobalCurrentExperiment->getInt(index));
        Stimulus *stim = group->getElement(GlobalCurrentExperiment->getInt(index));
        if(stim != NULL){
            //stim->load(display);
        } else {
            mprintf("Invalid referenced stimulus");
        }
    } else {
        mprintf("Invalid variable indexing stimulus group reference");
    }
}


void StimulusGroupReference::draw(shared_ptr<StimulusDisplay> display, float x, float y){
    
	//(stimulus_nodes->getElement((int)(*index)))->draw(display, x, y);
	if(index != NULL){
        mprintf("drawing stimulus (%d)", GlobalCurrentExperiment->getInt(index));
        Stimulus *stim = group->getElement(GlobalCurrentExperiment->getInt(index));
        if(stim != NULL){
            stim->draw(display,x,y);
        } else {
            mprintf("Invalid referenced stimulus");
        }
    } else {
        mprintf("Invalid variable indexing stimulus group reference");
    }
}


void StimulusGroupReference::draw(shared_ptr<StimulusDisplay> display, float x, float y, 
                                            float sizex, float sizey){
    
	//(stimulus_nodes->getElement((int)(*index)))->draw(display,x,y,sizex,sizey);
	mprintf("drawing stimulus (%d)", GlobalCurrentExperiment->getInt(index));
    if(index != NULL){
        Stimulus *stim = group->getElement(GlobalCurrentExperiment->getInt(index));
        if(stim != NULL){
            stim->draw(display,x,y,sizex,sizey);
        } else {
            mprintf("Invalid referenced stimulus");
        }
    } else {
        mprintf("Invalid variable indexing stimulus group reference");
    }
}


void StimulusGroupReference::draw(shared_ptr<StimulusDisplay> display){
	
	//(stimulus_nodes->getElement((int)(*index)))->draw(display);
	if(index != NULL){
		if (VERBOSE_STANDARD_STIMULI>0) mprintf("mStimulusGroupReference:  drawing stimulus (%d)", 
					GlobalCurrentExperiment->getInt(index));
		Stimulus *stim = 
					group->getElement(GlobalCurrentExperiment->getInt(index));
		if(stim != NULL){
			stim->draw(display);
		} else {
			mprintf("Invalid referenced stimulus");
		}
	} else {
		mprintf("Invalid variable indexing stimulus group reference");
	}
}

// JJD new method
// override of methods in base stimulus class -- announce each sub-element (using the standard base method)
void StimulusGroupReference::announceStimulusDraw()  {
  
   //(stimulus_nodes->getElement((int)(*index)))->announceStimulusDraw(); 
   if(index != NULL){
        int groupIndex = GlobalCurrentExperiment->getInt(index);
		if (VERBOSE_STANDARD_STIMULI>0) {
            mprintf("mStimulusGroupReference:  announcing stimulus draw, group index = %d", groupIndex);
        }
        Stimulus *stim = group->getElement(groupIndex);
        char *groupName = group->getGroupTagName();
		if(stim != NULL){
			stim->announceStimulusDraw(groupName,groupIndex);    // new polymophic method to give user back their group info
		} else {
			mprintf("Invalid referenced stimulus");
		}
    }
    
}

void StimulusGroupReference::announceStimulusErase() { 
    
	//(stimulus_nodes->getElement((int)(*index)))->announceStimulusErase();
    if(index != NULL){
        int groupIndex = GlobalCurrentExperiment->getInt(index);
		if (VERBOSE_STANDARD_STIMULI>0)  {
            mprintf("mStimulusGroupReference:  announcing stimulus erase, group index = %d", groupIndex);
		}
        Stimulus *stim = group->getElement(groupIndex);
        char *groupName = group->getGroupTagName();
		if(stim != NULL){
			stim->announceStimulusErase(groupName, groupIndex);    // new polymophic method to give user back their group info
		} else {
			mprintf("Invalid referenced stimulus");
		}
    }
}

	*/
        	
/*
CompoundStimulus::CompoundStimulus(std::string _tag):Stimulus(_tag) {
    stimList = new ExpandableList<OffsetStimulusContainer>();
    makeSubTag();       // setup first tag just to have something
}

CompoundStimulus::~CompoundStimulus() {
    delete stimList;
}


// TODO: need to take into account case where stimulus in compound is
// somehow shared with other objects... (a strange, possibly impossible
// case currently, but needs to be considered down the road).

void CompoundStimulus::freeze(bool should_freeze){
    for(int i = 0; i < stimList->getNElements(); i++){
        (*stimList)[i]->freeze(should_freeze);
    }
    Stimulus::freeze(should_freeze);
}

//Stimulus * CompoundStimulus::frozenClone(){
//	CompoundStimulus *clone = new CompoundStimulus(tag);
//	for(int i = 0; i < stimList->getNElements(); i++){
//		shared_ptr<Stimulus> p((*stimList)[i]->frozenClone());
//		clone->addStimulus(p);
//	}
//	
//	clone->setIsFrozen(true);
//	return clone;
//}

void CompoundStimulus::addStimulus(shared_ptr<Stimulus> stim) {
    makeSubTag();
	shared_ptr<OffsetStimulusContainer> p(new OffsetStimulusContainer(currentSubTag, stim));
    stimList->addReference(p);
}

void CompoundStimulus::addStimulus(shared_ptr<Stimulus> stim, shared_ptr<Variable> _xloc, shared_ptr<Variable> _yloc) {
    makeSubTag();
	shared_ptr<OffsetStimulusContainer> p(new OffsetStimulusContainer(currentSubTag, stim, _xloc, _yloc));
    stimList->addReference(p);
}


// override of methods in base stimulus class -- announce each sub-element (using the standard base method)
void CompoundStimulus::announceStimulusDraw(MWTime now)  {
    for(int i = 0; i < stimList->getNElements(); i++) {
        (stimList->getElement(i))->announceStimulusDraw(now);
    }
}

void CompoundStimulus::makeSubTag() {

    // delete any previously used memory
   
    // make subTag    
    std::string baseTag= this->gettag();   // get base tag
    int n = stimList->getNElements();        
    char *sub = new char [5];          // make subtag
    sprintf(sub, "_%d", n );
    
	   
	currentSubTag = baseTag + sub;

    delete [] sub;
    
}

void CompoundStimulus::draw(shared_ptr<StimulusDisplay>  display, 
							 float xdeg, float ydeg) {
    for(int i = 0; i < stimList->getNElements(); i++) {
        (stimList->getElement(i))->draw(display, xdeg, ydeg);
    }
}
*/




OffsetStimulusContainer::OffsetStimulusContainer(std::string _tag, 
											shared_ptr<Stimulus> _stim):Stimulus(_tag) { 
    stim = _stim;

    xoffset = shared_ptr<Variable>(new ConstantVariable(Datum(0.0)));
    yoffset = shared_ptr<Variable>(new ConstantVariable(Datum(0.0)));
}
                
OffsetStimulusContainer::OffsetStimulusContainer(std::string _tag, 
												   shared_ptr<Stimulus> _stim, 
												   shared_ptr<Variable> _xdeg, 
												   shared_ptr<Variable> _ydeg):
															Stimulus(_tag) {
    stim = _stim;

    xoffset = registerVariable(_xdeg);
    yoffset = registerVariable(_ydeg);
}

OffsetStimulusContainer::OffsetStimulusContainer(OffsetStimulusContainer& copy):Stimulus((Stimulus&) copy) {
//    mdebug("Offset copy constructor is %x", this);
//    mdebug("Offset copy object is %x", &copy);
//    mdebug("Offset copy stim is %x", copy.stim);
    //mdebug("More information %x", *(copy.stim));
    stim = copy.stim;
//    mdebug("Stim of offset in copy is %x", stim);
    xoffset = copy.xoffset;
    yoffset = copy.yoffset;
}

OffsetStimulusContainer::~OffsetStimulusContainer() { }

// TODO: need to write better custom "freeze" handling for the
// contained stimulus
void OffsetStimulusContainer::freeze(bool should_freeze){

    stim->freeze(should_freeze);
    Stimulus::freeze(should_freeze);
}

//Stimulus * OffsetStimulusContainer::frozenClone(){
//	shared_ptr<Stimulus> stim_clone(stim->frozenClone());
//	shared_ptr<Variable> x_clone(xoffset->frozenClone());
//	shared_ptr<Variable> y_clone(yoffset->frozenClone());
//	
//	OffsetStimulusContainer *clone = new OffsetStimulusContainer(tag, 
//												stim_clone,
//												x_clone,
//												y_clone);
//	clone->setIsFrozen(true);
//	return clone;
//}

void OffsetStimulusContainer::draw(shared_ptr<StimulusDisplay>  display,
                                                        float x, float y){ }

void OffsetStimulusContainer::draw(shared_ptr<StimulusDisplay>  display, float x, 
                                        float y, float sizex, float sizey) {
    if(stim == NULL) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
					"Attempt to draw on a NULL stimulus");
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
					"%s:%d", __FILE__, __LINE__);
    }
    stim->draw(display, (float)*xoffset+x, (float)*yoffset+y, sizex, sizey);
}

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
    

BasicTransformStimulus::~BasicTransformStimulus(){ }

//Stimulus *BasicTransformStimulus::frozenClone(){
//	
//	shared_ptr<Variable> x_clone(xoffset->frozenClone());
//	shared_ptr<Variable> y_clone(yoffset->frozenClone());
//	shared_ptr<Variable> xs_clone(xscale->frozenClone());
//	shared_ptr<Variable> ys_clone(yscale->frozenClone());
//	shared_ptr<Variable> r_clone(rotation->frozenClone());
//	shared_ptr<Variable> a_clone(alpha_multiplier->frozenClone());
//	
//	BasicTransformStimulus *clone = 
//					new BasicTransformStimulus(tag, 
//												x_clone,
//												y_clone,
//												xs_clone,
//												ys_clone,
//												r_clone,
//												a_clone);
//	clone->setIsFrozen(true);*/
//												
//	return clone;
//}

							            
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
    announceData.addElement(STIM_NAME,tag);        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_BASICTRANSFORM);  
    announceData.addElement(STIM_POSX,last_posx);  
    announceData.addElement(STIM_POSY,last_posy);  
    announceData.addElement(STIM_SIZEX,last_sizex);  
    announceData.addElement(STIM_SIZEY,last_sizey);  
    announceData.addElement(STIM_ROT,last_rot);  
//TODO	announceData.addElement(STIM_ALPHA, last_alpha);
        
    return (announceData);
}



BlankScreen::BlankScreen(std::string _tag, 
						   shared_ptr<Variable> _r,
						   shared_ptr<Variable> _g, 
						   shared_ptr<Variable> _b) : 
						   
						   Stimulus(_tag) {
    r = registerVariable(_r);
    g = registerVariable(_g);
    b = registerVariable(_b);
    
    Datum rval = *r;
    Datum gval = *g;
    Datum bval = *b;
	
    last_r = (float)rval;
    last_g = (float)gval;
    last_b = (float)bval;
    
}

BlankScreen::~BlankScreen(){ }

//Stimulus * BlankScreen::frozenClone(){
//	
//	shared_ptr<Variable> r_clone(r->frozenClone());
//	shared_ptr<Variable> g_clone(g->frozenClone());
//	shared_ptr<Variable> b_clone(b->frozenClone());
//	
//	BlankScreen *clone = 
//					new BlankScreen(tag, 
//									r_clone,
//									g_clone,
//									b_clone);
//	clone->setIsFrozen(true);
//												
//	return clone;
//}


    
void BlankScreen::drawInUnitSquare(shared_ptr<StimulusDisplay> display) {
    
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
    announceData.addElement(STIM_NAME,tag);        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_BLANK);  
    announceData.addElement(STIM_COLOR_R,last_r);  
    announceData.addElement(STIM_COLOR_G,last_g);  
    announceData.addElement(STIM_COLOR_B,last_b);
        
    return (announceData);
}

shared_ptr<mw::Component> BlankScreenFactory::createObject(std::map<std::string, std::string> parameters,
													ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, 
					   "tag", 
					   "color");
	
	std::string tagname(parameters.find("tag")->second);
	
	// find the RGB values
	// get the values
//	std::vector<std::string> colorParams;
	
	/*boost::spirit::classic::rule<> parsedItems;
	parsedItems = boost::spirit::classic::list_p[boost::spirit::classic::push_back_a(colorParams)];
	boost::spirit::classic::parse_info<> result = boost::spirit::classic::parse(parameters.find("color")->second.c_str(), parsedItems);
	if(!result.hit || colorParams.size() != 3) {
		throw InvalidAttributeException("color", parameters.find("color")->second);
	}*/

	ParsedColorTrio pct(reg, parameters.find("color")->second);

	// first is r then g then b:
	shared_ptr<Variable> r = pct.getR();	
	shared_ptr<Variable> g = pct.getG();	
	shared_ptr<Variable> b = pct.getB();	
	
	//checkAttribute(r, parameters["reference_id"], "color (r)", colorParams[1]);
//	checkAttribute(g, parameters["reference_id"], "color (g)", colorParams[2]);
//	checkAttribute(b, parameters["reference_id"], "color (b)", colorParams[3]);

	
	if(GlobalCurrentExperiment == 0) {
		throw SimpleException("no experiment currently defined");		
	}
	
	shared_ptr<StimulusDisplay> defaultDisplay = GlobalCurrentExperiment->getStimulusDisplay();
	if(defaultDisplay == 0) {
		throw SimpleException("no stimulusDisplay in current experiment");
	}
	
	
	shared_ptr <BlankScreen> newBlankScreen(new BlankScreen(tagname, r, g, b));
	
	
	
	newBlankScreen->load(defaultDisplay);
	shared_ptr <StimulusNode> thisStimNode = shared_ptr<StimulusNode>(new StimulusNode(newBlankScreen));
	reg->registerStimulusNode(tagname, thisStimNode);
	
	return newBlankScreen;
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
    unsigned char *hash = SHA1((unsigned char*)[imageData bytes], [imageData length], NULL);
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


ImageStimulus::ImageStimulus(std::string _tag, std::string _filename, 
								shared_ptr<Variable> _xoffset, 
                                shared_ptr<Variable> _yoffset, shared_ptr<Variable> _xscale, 
                                shared_ptr<Variable> _yscale, shared_ptr<Variable> _rot,
							   shared_ptr<Variable> _alpha) 
                                : BasicTransformStimulus(_tag, _xoffset, _yoffset,
                                                     _xscale ,_yscale, _rot, _alpha) {

	filename = _filename;
	
	width = 0; 
	height = 0;

}

// for cloning
ImageStimulus::ImageStimulus( std::string _tag, 
								std::string _filename,
								const vector<GLuint>& _texture_maps, 
								int _width, int _height,
								shared_ptr<Variable> _xoffset, 
                                shared_ptr<Variable> _yoffset, shared_ptr<Variable> _xscale, 
                                shared_ptr<Variable> _yscale, shared_ptr<Variable> _rot,
								shared_ptr<Variable> _alpha) 
                                : BasicTransformStimulus(_tag, _xoffset, _yoffset,
                                                     _xscale ,_yscale, _rot, _alpha) {
//    fprintf(stderr,"Creating image stimulus (filename = %s)\n",_filename.c_str());
//	fflush(stderr);
	
	// actually copy the string...
	filename = _filename;
	
    vector<GLuint>::const_iterator i;
    for(i = _texture_maps.begin(); i != _texture_maps.end(); ++i){
        texture_maps.push_back(*i);
    }

    //image_loaded = false;
	//il_image_name = 0; 
	width = _width; 
	height = _height;

	
    //	filename = _filename;
    //image_loader = new OpenGLImageLoader();
	
	
}
     
ImageStimulus::ImageStimulus(ImageStimulus& copy):
					BasicTransformStimulus((BasicTransformStimulus&) copy) { 
    
	filename = copy.getFilename();
}       
        
ImageStimulus::~ImageStimulus() {
	
	if(frozen){
		//delete xoffset;
//		delete yoffset;
//		delete xscale;
//		delete yscale;
//		delete rotation;
		
	}
	
}


//Stimulus * ImageStimulus::frozenClone(){
//
//	shared_ptr<Variable> x_clone(xoffset->frozenClone());
//	shared_ptr<Variable> y_clone(yoffset->frozenClone());
//	shared_ptr<Variable> xs_clone(xscale->frozenClone());
//	shared_ptr<Variable> ys_clone(yscale->frozenClone());
//	shared_ptr<Variable> r_clone(rotation->frozenClone());
//	shared_ptr<Variable> a_clone(alpha_multiplier->frozenClone());
//
//	ImageStimulus *clone = 
//					new ImageStimulus(tag, 
//									filename,
//									texture_maps,
//									width,
//									height,
//									x_clone,
//									y_clone,
//									xs_clone,
//									ys_clone,
//                                    r_clone,
//                                    a_clone);
//									
//	
//	clone->setIsFrozen(true);
//									
//	return clone;
//}



std::string ImageStimulus::getFilename() {
        return filename;
}
       
void ImageStimulus::load(shared_ptr<StimulusDisplay> display) {
    if(loaded){
		return;
	}
    
    DevILImageLoader loader;
    display->setCurrent(0);  // Need an active OpenGL context when ilutInit() is called
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
        
        display->setCurrent(i);
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
        display->setCurrent(i);
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
		
		
		//glActiveTexture(0);
		
    } else {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Stimulus image is not loaded.  Displaying nothing.");
    }
}

// override of basde class to provide more info
Datum ImageStimulus::getCurrentAnnounceDrawData() {
    
    //mprintf("getting announce DRAW data for image stimulus %s",tag );
    
    Datum announceData(M_DICTIONARY, 10);
    announceData.addElement(STIM_NAME,tag);        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_IMAGE);
    announceData.addElement(STIM_FILENAME,filename);  
    announceData.addElement(STIM_FILE_HASH,fileHash);  
    announceData.addElement(STIM_POSX,last_posx);  
    announceData.addElement(STIM_POSY,last_posy);  
    announceData.addElement(STIM_SIZEX,last_sizex);  
    announceData.addElement(STIM_SIZEY,last_sizey);  
    announceData.addElement(STIM_ROT,last_rot);  
//TODO    announceData.addElement(STIM_ALPHA,last_alpha);  
    
    return (announceData);
}


shared_ptr<mw::Component> ImageStimulusFactory::createObject(std::map<std::string, std::string> parameters,
													ComponentRegistry *reg) {
	namespace bf = boost::filesystem;
	
	REQUIRE_ATTRIBUTES(parameters, 
					   "tag", 
					   "x_size", 
					   "y_size", 
					   "x_position", 
					   "y_position", 
					   "rotation", 
					   "path");
	
	std::string tagname(parameters.find("tag")->second);
	shared_ptr<Variable> x_size = reg->getVariable(parameters.find("x_size")->second);	
	shared_ptr<Variable> y_size = reg->getVariable(parameters.find("y_size")->second);	
	shared_ptr<Variable> x_position = reg->getVariable(parameters.find("x_position")->second);	
	shared_ptr<Variable> y_position = reg->getVariable(parameters.find("y_position")->second);	
	shared_ptr<Variable> rotation = reg->getVariable(parameters.find("rotation")->second);	
	
	shared_ptr<Variable> alpha_multiplier = 
			reg->getVariable(parameters["alpha_multiplier"], std::string("1.0"));	
	
	bf::path full_path = reg->getPath(parameters["working_path"], parameters["path"]);
		
	checkAttribute(x_size, parameters["reference_id"], "x_size", parameters["x_size"]);
	checkAttribute(y_size, parameters["reference_id"], "y_size", parameters.find("y_size")->second);
	checkAttribute(x_position, parameters["reference_id"], "x_position", parameters.find("x_position")->second);
	checkAttribute(y_position, parameters["reference_id"], "y_position", parameters.find("y_position")->second);
	checkAttribute(rotation, parameters["reference_id"], "rotation", parameters.find("rotation")->second);
	checkAttribute(alpha_multiplier,parameters["reference_id"], "alpha_multiplier", parameters.find("alpha_multiplier")->second);
	if(!bf::exists(full_path)) {
		throw InvalidReferenceException(parameters["reference_id"], "path", parameters.find("path")->second);
	}
	
	if(bf::is_directory(full_path)) {
		throw InvalidReferenceException(parameters["reference_id"], "path", parameters.find("path")->second);
	}
	
	if(GlobalCurrentExperiment == NULL) {
		throw SimpleException("no experiment currently defined");		
	}
	
	shared_ptr<StimulusDisplay> defaultDisplay = GlobalCurrentExperiment->getStimulusDisplay();
	if(defaultDisplay == 0) {
		throw SimpleException("no stimulusDisplay in current experiment");
	}
	

	shared_ptr <ImageStimulus> newImageStimulus = shared_ptr<ImageStimulus>(new ImageStimulus(tagname, 
																						 full_path.string(), 
																						 x_position,
																						 y_position,
																						 x_size,
																						 y_size,
																						 rotation,
																						 alpha_multiplier));
	
	
  Stimulus::load_style deferred = Stimulus::nondeferred_load;
  if(!parameters["deferred"].empty()){
    string deferred_value = parameters["deferred"];
    boost::algorithm::to_lower(deferred_value);
    if(deferred_value == "yes" || deferred_value == "1" || deferred_value == "true"){
      deferred = Stimulus::deferred_load;
    } else if(deferred_value == "explicit"){
      deferred = Stimulus::explicit_load;
    }
  }

  newImageStimulus->setDeferred(deferred);
  
  // TODO: deferred load?
  if(deferred != Stimulus::deferred_load && deferred != Stimulus::explicit_load){
    newImageStimulus->load(defaultDisplay);
  }
  
	shared_ptr <StimulusNode> thisStimNode = shared_ptr<StimulusNode>(new StimulusNode(newImageStimulus));
	reg->registerStimulusNode(tagname, thisStimNode);
	
	return newImageStimulus;
}

PointStimulus::PointStimulus(std::string _tag, shared_ptr<Variable> _xoffset, shared_ptr<Variable> _yoffset, 
						shared_ptr<Variable> _xscale, shared_ptr<Variable> _yscale,
							   shared_ptr<Variable> _rot,
							   shared_ptr<Variable> _alpha,
							   shared_ptr<Variable> _r, shared_ptr<Variable> _g, shared_ptr<Variable> _b
							   ) :
							BasicTransformStimulus
							(_tag, _xoffset, _yoffset,_xscale ,_yscale, _rot, _alpha) {    
    r = registerVariable(_r);     // should be 0-1    (1 is full saturation)
    g = registerVariable(_g); 
    b = registerVariable(_b);                                    
                         
}

PointStimulus::PointStimulus(const PointStimulus &tocopy) : 
				BasicTransformStimulus((const BasicTransformStimulus&)tocopy){
	r = tocopy.r;
	g = tocopy.g;
	b = tocopy.b;
}

PointStimulus::~PointStimulus(){ }

//Stimulus * PointStimulus::frozenClone(){
//	shared_ptr<Variable> x_clone(xoffset->frozenClone());
//	shared_ptr<Variable> y_clone(yoffset->frozenClone());
//	shared_ptr<Variable> xs_clone(xscale->frozenClone());
//	shared_ptr<Variable> ys_clone(yscale->frozenClone());
//	shared_ptr<Variable> rot_clone(rotation->frozenClone());
//	shared_ptr<Variable> alpha_clone(alpha_multiplier->frozenClone());	
//	shared_ptr<Variable> r_clone(r->frozenClone());
//	shared_ptr<Variable> g_clone(g->frozenClone());
//	shared_ptr<Variable> b_clone(b->frozenClone());
//
//	PointStimulus *clone = new PointStimulus(tag,
//											   x_clone,
//											   y_clone,
//											   xs_clone,
//											   ys_clone,
//											   rot_clone,
//											   alpha_clone,
//											   r_clone,
//											   g_clone,
//											   b_clone
//											   );
//	clone->setIsFrozen(true);
//		
//	return clone;
//}


void PointStimulus::drawInUnitSquare(shared_ptr<StimulusDisplay> display) {
    
     // draw point at desired location with desired color
     // fill a (0,0) (1,1) box with the right color
    if(r == NULL || g == NULL || b == NULL ){
		merror(M_DISPLAY_MESSAGE_DOMAIN,
				"NULL color variable in PointStimulus.");
	}
	
	
    // get current values in these variables.
	GLfloat _r = (float)(*r);
	GLfloat _g = (float)(*g);
	GLfloat _b = (float)(*b);
	   
	glBindTexture(GL_TEXTURE_2D, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	
    glBegin(GL_QUADS);
		
		//mprintf("fixpoint r: %g, g: %g, b: %g", red, green, blue);
	
        //glColor3f(_r,_g,_b);
		glColor4f(_r, _g, _b, *alpha_multiplier);
        glVertex3f(0.0,0.0,0.0);
		glVertex3f(1.0,0.0,0.0);
        glVertex3f(1.0,1.0,0.0);
        glVertex3f(0.0,1.0,0.0);

    glEnd();
    
	glDisable(GL_BLEND);
    
    last_r = _r;
    last_g = _g;
    last_b = _b;
    

}

// override of base class to provide more info
Datum PointStimulus::getCurrentAnnounceDrawData() {
    
    //mprintf("getting announce DRAW data for point stimulus %s",tag );
    
    Datum announceData(M_DICTIONARY, 11);
    announceData.addElement(STIM_NAME,tag);        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_POINT);
    announceData.addElement(STIM_POSX,last_posx);  
    announceData.addElement(STIM_POSY,last_posy);  
    announceData.addElement(STIM_SIZEX,last_sizex);  
    announceData.addElement(STIM_SIZEY,last_sizey);  
    announceData.addElement(STIM_ROT,last_rot);  
    announceData.addElement(STIM_COLOR_R,last_r);  
    announceData.addElement(STIM_COLOR_G,last_g);  
    announceData.addElement(STIM_COLOR_B,last_b);  
    
    return (announceData);
}


/*
void * forceDisplayUpdate(void *arg) {
    //mprintf("displaying...");
    shared_ptr<StimulusDisplay> display = *((shared_ptr<StimulusDisplay> *)arg);
    //mprintf("Called force display...");
    if(display) {
        display->updateDisplay();
    }
    // just return NULL because noone is probably checking this anyway
    return NULL;
}

FreeRunningMovieStimulus::FreeRunningMovieStimulus(char *_tag, long _nframes,
                        long _frame_interval, float _xoffset, float _yoffset,
                        float _xscale, float _yscale, float _rot) 
                        : BasicTransformStimulus(_tag,
                                            new ConstantVariable(_xoffset), 
                                            new ConstantVariable(_yoffset),
                                            new ConstantVariable(_xscale), 
                                            new ConstantVariable(_yscale), 
                                            new ConstantVariable(_rot)) { 
    // TODO: this is a leak for sure
    nframes = _nframes; 
    frame_interval_ms = _frame_interval;
    start_time = clock->getSystemTime();
    running = false;
}

void FreeRunningMovieStimulus::setCurrentFrame() { };
        
void FreeRunningMovieStimulus::draw(shared_ptr<StimulusDisplay> display) {
    if(!running && GlobalCurrentExperiment->getInt(state_system_mode) != IDLE) {
        if(display) {
            if(0 && nframes) {
                schedule_node = GlobalScheduler->schedule(0, frame_interval_ms,
                                        abs(nframes), &forceDisplayUpdate,
										(void *)display, 
										M_DEFAULT_STIMULUS_PRIORITY, 
										M_DEFAULT_WARN_SLOP_MS, 
										M_DEFAULT_FAIL_SLOP_MS);
            } else {
                schedule_node = GlobalScheduler->schedule(0, frame_interval_ms,
                                                    M_REPEAT_INDEFINITELY,
                                                    &forceDisplayUpdate,
													(void *)display,
													M_DEFAULT_STIMULUS_PRIORITY, 
													M_DEFAULT_WARN_SLOP_MS, 
													M_DEFAULT_FAIL_SLOP_MS);
            }
        }
        running = true;
    }
    if(GlobalCurrentExperiment->getInt(state_system_mode) == IDLE && running) {
        if(schedule_node) {
            running = false;
            schedule_node->cancel();
            schedule_node = NULL;
        }
        return;
    }
            
    int current_time = clock->getSystemTime();
    int elapsed = current_time - start_time;
    current_frame = (int)((double)elapsed / (double)frame_interval_ms);
        
    if(current_frame > abs(nframes)) {
        if(nframes > 0) {
            running = false;
            return; 	// do something more interesting here...
        } else {
            //running = true;
            current_frame = current_frame % abs(nframes);
        }
    }
    //mprintf("current_frame: %d", current_frame);
    draw(display, *xoffset, *yoffset, *xscale, *yscale);
}
        
void FreeRunningMovieStimulus::draw(shared_ptr<StimulusDisplay> display,float x, 
                                        float y, float sizex, float sizey) {
    BasicTransformStimulus::draw(display,x,y,sizex,sizey);
}
        
void FreeRunningMovieStimulus::drawInUnitSquare(shared_ptr<StimulusDisplay> display) { }

void FreeRunningMovieStimulus::setVisible(bool vis) {
    BasicTransformStimulus::setVisible(vis);
    if(!vis && running && schedule_node) { 
        schedule_node->cancel();
        running = false;
        // kludge/needs addition: someone must clean this up
        schedule_node = NULL;
    }
}

void VideoRequest::service() {
	//E->drawStimulusID(stimtodraw, xloc, yloc);
}

void videoInterruptService(void) {	
    //ITC_DigitalBitsOn(0x02);
	//VRtime = currentTime;
	//if (E->getBool(IS_RUNNING)) 
		//putEvent5("VR", NULL, NULL); 
	
	//E->drawPendingStimuli();
}*/
 
