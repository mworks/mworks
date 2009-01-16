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
#include "ComponentRegistry_new.h"
//#include "ComponentFactory.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/lists.hpp>
#include "ParsedColorTrio.h"

#include <boost/regex.hpp>

#include <stdio.h> // for fopen()
#include <iostream>

//	#define USE_COCOA_IMAGE_LOADER	1

	#undef USE_COCOA_IMAGE_LOADER



#ifdef USE_COCOA_IMAGE_LOADER
	#import <Cocoa/Cocoa.h>
#endif
using namespace mw;


#define VERBOSE_STANDARD_STIMULI 0

/*StimulusGroupReference::StimulusGroupReference(char *_tag, StimulusGroup *_group, 
															Variable *_index) : Stimulus(_tag){
	group = _group;
	index = _index;
}



void StimulusGroupReference::load(StimulusDisplay *display){
    
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


void StimulusGroupReference::draw(StimulusDisplay *display, float x, float y){
    
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


void StimulusGroupReference::draw(StimulusDisplay *display, float x, float y, 
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


void StimulusGroupReference::draw(StimulusDisplay *display){
	
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
        	
CompoundStimulus::CompoundStimulus(std::string _tag):Stimulus(_tag) {
    stimList = new ExpandableList<OffsetStimulusContainer>();
    makeSubTag();       // setup first tag just to have something
}

CompoundStimulus::~CompoundStimulus() {
    delete stimList;
}

Stimulus * CompoundStimulus::frozenClone(){
	CompoundStimulus *clone = new CompoundStimulus(tag);
	for(int i = 0; i < stimList->getNElements(); i++){
		shared_ptr<Stimulus> p((*stimList)[i]->frozenClone());
		clone->addStimulus(p);
	}
	
	clone->setIsFrozen(true);
	return clone;
}

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
void CompoundStimulus::announceStimulusDraw(MonkeyWorksTime now)  {
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

void CompoundStimulus::draw(StimulusDisplay * display, 
							 float xdeg, float ydeg) {
    for(int i = 0; i < stimList->getNElements(); i++) {
        (stimList->getElement(i))->draw(display, xdeg, ydeg);
    }
}




OffsetStimulusContainer::OffsetStimulusContainer(std::string _tag, 
											shared_ptr<Stimulus> _stim):Stimulus(_tag) { 
    stim = _stim;

    xoffset = shared_ptr<Variable>(new ConstantVariable(Data(0.0)));
    yoffset = shared_ptr<Variable>(new ConstantVariable(Data(0.0)));
}
                
OffsetStimulusContainer::OffsetStimulusContainer(std::string _tag, 
												   shared_ptr<Stimulus> _stim, 
												   shared_ptr<Variable> _xdeg, 
												   shared_ptr<Variable> _ydeg):
															Stimulus(_tag) {
    stim = _stim;

    xoffset = _xdeg;
    yoffset = _ydeg;
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


Stimulus * OffsetStimulusContainer::frozenClone(){
	shared_ptr<Stimulus> stim_clone(stim->frozenClone());
	shared_ptr<Variable> x_clone(xoffset->frozenClone());
	shared_ptr<Variable> y_clone(yoffset->frozenClone());
	
	OffsetStimulusContainer *clone = new OffsetStimulusContainer(tag, 
												stim_clone,
												x_clone,
												y_clone);
	clone->setIsFrozen(true);
	return clone;
}

void OffsetStimulusContainer::draw(StimulusDisplay * display,
                                                        float x, float y){ }

void OffsetStimulusContainer::draw(StimulusDisplay * display, float x, 
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
    xoffset = _xoffset;
    yoffset = _yoffset;
    xscale = _xscale;
    yscale = _yscale;
    rotation = _rot;
	alpha_multiplier = _alpha;
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

Stimulus *BasicTransformStimulus::frozenClone(){
	
	shared_ptr<Variable> x_clone(xoffset->frozenClone());
	shared_ptr<Variable> y_clone(yoffset->frozenClone());
	shared_ptr<Variable> xs_clone(xscale->frozenClone());
	shared_ptr<Variable> ys_clone(yscale->frozenClone());
	shared_ptr<Variable> r_clone(rotation->frozenClone());
	shared_ptr<Variable> a_clone(alpha_multiplier->frozenClone());
	
	BasicTransformStimulus *clone = 
					new BasicTransformStimulus(tag, 
												x_clone,
												y_clone,
												xs_clone,
												ys_clone,
												r_clone,
												a_clone);
	clone->setIsFrozen(true);
												
	return clone;
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
                
void BasicTransformStimulus::draw(StimulusDisplay * display) {
    draw(display, *xoffset, *yoffset, *xscale, *yscale);
}
                
void BasicTransformStimulus::draw(StimulusDisplay * display,float x, float y, 
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
Data BasicTransformStimulus::getCurrentAnnounceDrawData() {
    
    Data announceData(M_DICTIONARY, 8);
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
    r = _r;
    g = _g;
    b = _b;
    
	Data rval = *r;
	Data gval = *g;
	Data bval = *b;
	
    last_r = (float)rval;
    last_g = (float)gval;
    last_b = (float)bval;
    
}

BlankScreen::~BlankScreen(){ }

Stimulus * BlankScreen::frozenClone(){
	
	shared_ptr<Variable> r_clone(r->frozenClone());
	shared_ptr<Variable> g_clone(g->frozenClone());
	shared_ptr<Variable> b_clone(b->frozenClone());
	
	BlankScreen *clone = 
					new BlankScreen(tag, 
									r_clone,
									g_clone,
									b_clone);
	clone->setIsFrozen(true);
												
	return clone;
}


    
void BlankScreen::drawInUnitSquare(StimulusDisplay *display) {
    
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
Data BlankScreen::getCurrentAnnounceDrawData() {
    
    Data announceData(M_DICTIONARY, 6);
    announceData.addElement(STIM_NAME,tag);        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_BLANK);  
    announceData.addElement(STIM_COLOR_R,last_r);  
    announceData.addElement(STIM_COLOR_G,last_g);  
    announceData.addElement(STIM_COLOR_B,last_b);
        
    return (announceData);
}

shared_ptr<mw::Component> BlankScreenFactory::createObject(std::map<std::string, std::string> parameters,
													mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, 
					   "tag", 
					   "color");
	
	std::string tagname(parameters.find("tag")->second);
	
	// find the RGB values
	// get the values
//	std::vector<std::string> colorParams;
	
	/*boost::spirit::rule<> parsedItems;
	parsedItems = boost::spirit::list_p[boost::spirit::push_back_a(colorParams)];
	boost::spirit::parse_info<> result = boost::spirit::parse(parameters.find("color")->second.c_str(), parsedItems);
	if(!result.hit || colorParams.size() != 3) {
		throw InvalidAttributeException("color", parameters.find("color")->second);
	}*/

	ParsedColorTrio pct(reg, parameters.find("color")->second);

	// first is r then g then b:
	shared_ptr<Variable> r = pct.getR();	
	shared_ptr<Variable> g = pct.getG();	
	shared_ptr<Variable> b = pct.getB();	
	
	//checkAttribute(r, parameters.find("reference_id")->second, "color (r)", colorParams[1]);
//	checkAttribute(g, parameters.find("reference_id")->second, "color (g)", colorParams[2]);
//	checkAttribute(b, parameters.find("reference_id")->second, "color (b)", colorParams[3]);

	
	if(GlobalCurrentExperiment == 0) {
		throw SimpleException("no experiment currently defined");		
	}
	
	shared_ptr<StimulusDisplay> defaultDisplay = GlobalCurrentExperiment->getStimulusDisplay();
	if(defaultDisplay == 0) {
		throw SimpleException("no stimulusDisplay in current experiment");
	}
	
	
	shared_ptr <BlankScreen> newBlankScreen(new BlankScreen(tagname, r, g, b));
	
	
	
	newBlankScreen->load(defaultDisplay.get());
	shared_ptr <StimulusNode> thisStimNode = shared_ptr<StimulusNode>(new StimulusNode(newBlankScreen));
	reg->registerStimulusNode(tagname, thisStimNode);
	
	return newBlankScreen;
}


bool OpenGLImageLoader::initialized = false;
Lockable *OpenGLImageLoader::lock = NULL;

GLuint OpenGLImageLoader::load(std::string filename, StimulusDisplay *display,
                                                    int *width, int *height) {
    
	
	#ifdef	USE_COCOA_IMAGE_LOADER
	
	NSImage * image;
	GLuint texName = 0;
	
	
	image = [[NSImage alloc] initWithContentsOfFile:
								[NSString stringWithCString:filename.c_str() 
										  encoding:NSASCIIStringEncoding]];

	
	NSSize orig_size = [image size];
	double aspect_ratio = (double)orig_size.width / (double)orig_size.height;
	
	double resizeWidth, resizeHeight, padWidth, padHeight;
	
	if(orig_size.width > orig_size.height){
		
		int power = 1;
		while( power < orig_size.width ) {
			power <<= 1;
		}
	
		resizeWidth = power;
		resizeHeight = (1.0 / aspect_ratio) * resizeWidth;
		padWidth = resizeWidth;
		padHeight = resizeWidth;

	} else {
		int power = 1;
		while( power < orig_size.width ) {
			power <<= 1;
		}
	
		resizeHeight = power;
		resizeWidth = (aspect_ratio) * resizeHeight;
		padWidth = resizeHeight;
		padHeight = resizeHeight;
	}

	double height_diff = padHeight - resizeHeight;
	double width_diff = padWidth - resizeWidth;
	
	NSImage *resizedImage = [[NSImage alloc] 
								initWithSize: NSMakeSize(padWidth, padHeight)]; // TODO: off center

	[resizedImage lockFocus];
	[resizedImage setBackgroundColor:[NSColor colorWithDeviceRed:1.0
											  green:1.0
											  blue:1.0
											  alpha:0.0]];
	 [[NSColor clearColor] set];
	 NSRectFill(NSMakeRect(0,0,[resizedImage size].width, [resizedImage size].height));

	
	[image drawInRect: NSMakeRect(width_diff/2, height_diff/2, 
								  resizeWidth - width_diff/2 , 
								  resizeHeight - height_diff/2) 
		   fromRect: NSMakeRect(0, 0, orig_size.width, orig_size.height) 
		   operation: NSCompositeSourceOver fraction: 1.0];
//	[image drawInRect: NSMakeRect(0, 0, resizeWidth, resizeHeight) 
//		   fromRect: NSMakeRect(0, 0, orig_size.width, orig_size.height) 
//		   operation: NSCompositeSourceOver fraction: 1.0];
	[resizedImage unlockFocus];
	
	
	NSBitmapImageRep* bitmap;// = [NSBitmapImageRep alloc];
    int samplesPerPixel = 0;
    NSSize imgSize = [image size];
	*width = (int)imgSize.width;
    *height = (int)imgSize.height;
 
    /*[image lockFocus];
    [bitmap initWithFocusedViewRect:
                    NSMakeRect(0.0, 0.0, imgSize.width, imgSize.height)]; 
    [image unlockFocus];*/
	
	bitmap = [NSBitmapImageRep imageRepWithData:[resizedImage TIFFRepresentation]];
 
 
	
 
	//glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
 
	 // Generate a new texture name 
	glGenTextures (1, &texName);
//	fprintf(stderr, "Loading texName = %u\n", (unsigned int)texName);fflush(stderr);
	glBindTexture (GL_TEXTURE_2D, texName);
 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
 
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, IL_FALSE);
 
	//glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, [bitmap bytesPerRow]/([bitmap bitsPerPixel] >> 3)); //imgWidth
	
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	//glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	//glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
 
	// Set proper unpacking row length for bitmap.
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, ([bitmap bytesPerRow]/([bitmap bitsPerPixel]  >> 3)));
 
	// Set byte aligned unpacking (needed for 3 byte per pixel bitmaps).
	//glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
 

 
	samplesPerPixel = [bitmap samplesPerPixel];
 
	// Nonplanar, RGB 24 bit bitmap, or RGBA 32 bit bitmap.
	if(![bitmap isPlanar] && 
		(samplesPerPixel == 3 || samplesPerPixel == 4)) 
	{ 
		
	//	glBindTexture (GL_TEXTURE_2D, texName);
	//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		
		gluBuild2DMipmaps(GL_TEXTURE_2D, 
						  samplesPerPixel == 4 ? GL_RGBA8 : GL_RGBA8, 
						  [bitmap pixelsWide], [bitmap pixelsHigh],
						  samplesPerPixel == 4 ? GL_RGBA : GL_RGB, 
						  GL_UNSIGNED_BYTE, 
						  [bitmap bitmapData]);
		
		
		/*glTexImage2D(GL_TEXTURE_2D, 0, 
			//GL_RGBA,
			samplesPerPixel == 4 ? GL_RGBA8 : GL_RGB8,
			[bitmap pixelsWide], 
			[bitmap pixelsHigh], 
			0, 
			samplesPerPixel == 4 ? GL_RGBA : GL_RGB,
			GL_UNSIGNED_BYTE, 
			[bitmap bitmapData]);*/
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
	} else if(samplesPerPixel == 1){
	//	glBindTexture (GL_TEXTURE_2D, texName);
//		glEnable(GL_TEXTURE_2D);
//		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, [bitmap pixelsWide], [bitmap pixelsHigh],
                  GL_LUMINANCE, GL_UNSIGNED_BYTE, [bitmap bitmapData]);
				  
/*		glTexImage2D(GL_TEXTURE_2D, 0, 
			GL_RGBA,
			[bitmap pixelsWide], 
			[bitmap pixelsHigh], 
			0, 
			GL_LUMINANCE,
			GL_UNSIGNED_BYTE, 
			[bitmap bitmapData]);*/
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
	} else {
		// Handle other bitmap formats.
		mwarning(M_DISPLAY_MESSAGE_DOMAIN, "Unknown bitmap format");
	}
	
    // Clean up.
    [bitmap release];
	[image release];
	
	
	return texName;
	
	/*[image lockFocus];
	bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect (0.0f, 0.0f, size.width, size.height)];
	[image unlockFocus];
	NSSize texSize;
	texSize.width = [bitmap size].width;
	texSize.height = [bitmap size].height;
	
	*width = (int)texSize.width;
    *height = (int)texSize.height;
	
	display->setCurrent(); // if we successfully retrieve a current context (required)
	glGenTextures (1, &texName);
	glBindTexture (GL_TEXTURE_RECTANGLE_EXT, texName);
	
	GLuint colorspace;
	if([bitmap hasAlpha]){
		colorspace = GL_RGBA;
	} else {
		colorspace = GL_RGB;
	}
	
	GLuint data_format;
	
	if([bitmap bitsPerPixel] == 16){
		data_format = GL_UNSIGNED_SHORT;
	} else if([bitmap bitsPerPixel] == 32){
		data_format = GL_UNSIGNED_INT;
	} else {
		data_format = GL_UNSIGNED_BYTE;
	}
	glTexImage2D (GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, texSize.width, texSize.height, 0, colorspace, data_format, [bitmap bitmapData]);
	
	glBindTexture (GL_TEXTURE_RECTANGLE_EXT, 0); // unbind
	
	[bitmap release];
	[image release];
	
	
	
	return texName;*/
	
	#else
	
	GLuint texture_map;
    ILuint il_image_name;
	ILenum il_error;
	
	Lockable *lock;
	
	//clock->sleepMS(1);
	
	if(filename == ""){
		
		throw SimpleException("Cannot load image (NULL filename).");
		return 0;
	}
	
    // TODO:
    // SHOULD CHECK TO BE SURE THAT THE FILE REALLY EXISTS!
    // IL WILL CHOKE IF IT DOESN'T!
	FILE *test = fopen(filename.c_str(),"r");
	if(!test){
		throw SimpleException("Image file does not exist", filename);
	}
	fclose(test);
	
    if(1 || !OpenGLImageLoader::initialized) {
		mprintf("initializing image loader facility");
		OpenGLImageLoader::lock = new Lockable();
        lock = OpenGLImageLoader::lock;
		
		//lock->lock();
		ilInit();
        ilutInit();
        ilutRenderer(ILUT_OPENGL);
		ilutEnable(ILUT_OPENGL_CONV);
        //lock->unlock();
		OpenGLImageLoader::initialized = true;
    }
	
	
	//lock->lock();
            
	
    ilGenImages(1,&il_image_name);
    ilBindImage(il_image_name);
    	   
	// TODO: check validity of the image file
	// Can't trust DevIL library not to crash here
	
	mprintf("Loading image %s", filename.c_str());
	
    //this variable is unused as of oct 21 2004 TODO            
    bool image_loaded = ilLoadImage(filename.c_str());
    
	ilConvertImage(IL_RGBA, IL_FLOAT);
	
	//ilConvertPal(IL_PAL_RGBA32);
	
	if((il_error = ilGetError()) != IL_NO_ERROR) {
        // TODO HANDLE ERROR
        merror(M_DISPLAY_MESSAGE_DOMAIN,
				"IL Image Library Error: %x", 
				il_error);
		
		throw SimpleException("Cannot load image", filename);

		lock->unlock();
        return -1;
    }
            
    *width = (int)ilGetInteger(IL_IMAGE_WIDTH);
    *height = (int)ilGetInteger(IL_IMAGE_HEIGHT);
	
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	//int depth = (int)ilGetInteger(IL_IMAGE_DEPTH);
	
    for(int i = 0; i < display->getNContexts(); i++){
		display->setCurrent(i);        
		//texture_map = ilutGLBindTexImage();
		texture_map = ilutGLBindMipmaps();
	}
    //texture_map = ilutGLBindMipmaps();
            
    //texture_map = ilutGLLoadImage(filename);
            
    if((il_error = ilGetError()) != IL_NO_ERROR) {
        throw SimpleException(
			(boost::format("Cannot bind image texture. IL Image Library Error") % il_error).str());
        image_loaded = false;
		lock->unlock();
        return -1;
    }
	
	ilDeleteImage(il_image_name);
	if((il_error = ilGetError()) != IL_NO_ERROR) {
         throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
			(boost::format("Cannot delete image. IL Image Library Error: %s") % il_error).str());
        image_loaded = false;
		lock->unlock();
        return -1;
    }	
    /*if(texture_map >= 0){
        return true;
    } else {
        return false;
    }*/
	
	//lock->unlock();
	
	 return texture_map;
	
	#endif
	
   
}

ImageStimulus::ImageStimulus(std::string _tag, std::string _filename, 
								shared_ptr<Variable> _xoffset, 
                                shared_ptr<Variable> _yoffset, shared_ptr<Variable> _xscale, 
                                shared_ptr<Variable> _yscale, shared_ptr<Variable> _rot,
							   shared_ptr<Variable> _alpha) 
                                : BasicTransformStimulus(_tag, _xoffset, _yoffset,
                                                     _xscale ,_yscale, _rot, _alpha) {
//    fprintf(stderr,"Creating image stimulus (filename = %s)\n",_filename.c_str());
//	fflush(stderr);
	
	filename = _filename;
	
	texture_maps = new ExpandableList<GLuint>();
	//image_loaded = false;
	//il_image_name = 0; 
	width = 0; 
	height = 0;

    //	filename = _filename;
    //image_loader = new OpenGLImageLoader();
}

// for cloning
ImageStimulus::ImageStimulus( std::string _tag, 
								std::string _filename,
								ExpandableList<GLuint> *_texture_maps, 
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
	
	texture_maps = _texture_maps;
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
	
	delete texture_maps;
}


Stimulus * ImageStimulus::frozenClone(){

	shared_ptr<Variable> x_clone(xoffset->frozenClone());
	shared_ptr<Variable> y_clone(yoffset->frozenClone());
	shared_ptr<Variable> xs_clone(xscale->frozenClone());
	shared_ptr<Variable> ys_clone(yscale->frozenClone());
	shared_ptr<Variable> r_clone(rotation->frozenClone());
	shared_ptr<Variable> a_clone(alpha_multiplier->frozenClone());

	ImageStimulus *clone = 
					new ImageStimulus(tag, 
									filename,
									new ExpandableList<GLuint>(*texture_maps),
									width,
									height,
									x_clone,
									y_clone,
									xs_clone,
									ys_clone,
									   r_clone,
	a_clone);
									
	
	clone->setIsFrozen(true);
									
	return clone;
}



std::string ImageStimulus::getFilename() {
        return filename;
}
       
void ImageStimulus::load(StimulusDisplay *display) {
    //GlobalOpenGLContextManager->setGlobalContextCurrent();
    if(loaded){
		return;
	}
	
	// TODO: this needs clean up.  We are counting on all of the contexts
	// in the stimulus display to have the exact same history.  Ideally, this
	// should be true, but we should eventually be robust in case it isn't
	for(int i = 0; i < display->getNContexts(); i++){
		display->setCurrent(i);
		GLuint texture_map = OpenGLImageLoader::load(filename, display, &width, &height);
		texture_maps->addElement(i, texture_map);
//		fprintf(stderr, "Loaded texture map %u into context %d\n", (unsigned int)texture_map, i);fflush(stderr);
		if(texture_map){
			mprintf("Image loaded into texture_map %d", texture_map);
		}
	}
	
	// TODO: update to work with lists
	// TODO: this is wrong, because texture_map is unsigned...
	/*if(texture_map > 0) {
        loaded = true;
    } else {
        loaded = false;
    }*/
}

void ImageStimulus::drawInUnitSquare(StimulusDisplay *display) {
    double aspect = (double)width / (double)height;
    if(1 || loaded) {
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        
		
		
		//glActiveTexture(GL_TEXTURE0);

//		fprintf(stderr, "Binding texture %lu on context index %u (context %u)",
//						*(texture_maps->getElement(display->getCurrentContextIndex())),
//						display->getCurrentContextIndex(),
//						(unsigned int)display->getCurrentContext());fflush(stderr);
		glBindTexture(GL_TEXTURE_2D, *(texture_maps->getElement(display->getCurrentContextIndex())));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glEnable (GL_BLEND); 
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
								
        glBegin(GL_QUADS);
		
		float a = alpha_multiplier->getValue().getFloat();
		glColor4f(0.5, 0.5, 0.5, 1-a);
//		glColor4f(0.5, 0.5, 0.5, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
	
        if(aspect > 1) {
			
            glTexCoord2f(0.0,0.0); 
            //glMultiTexCoord2f(GL_TEXTURE0,0.0,0.0);
			glVertex3f(0.0,(0.5-0.5/aspect),0.0);
            
			
            glTexCoord2f(1.0,0.0); 
			//glMultiTexCoord2f(GL_TEXTURE0,1.0,0.0); 
            glVertex3f(1.0,(0.5-0.5/aspect),0.0);
            
			
            glTexCoord2f(1.0,1.0); 
            //glMultiTexCoord2f(GL_TEXTURE0, 1.0,1.0);
			glVertex3f(1.0,(0.5-0.5/aspect) + 1.0/aspect,0.0);
            
			
            glTexCoord2f(0.0,1.0); 
            //glMultiTexCoord2f(GL_TEXTURE0, 0.0,1.0);
			glVertex3f(0.0,(0.5-0.5/aspect) + 1.0/aspect,0.0);
        } else {
			
            glTexCoord2f(0.0,0.0); 
            //glMultiTexCoord2f(GL_TEXTURE0, 0.0,0.0);
			glVertex3f((1.0 - aspect)/2.0,0.0,0.0);
            
			
            glTexCoord2f(1.0,0.0);
			//glMultiTexCoord2f(GL_TEXTURE0, 1.0,0.0); 
            glVertex3f((1.0 - aspect)/2.0 + aspect,0.0,0.0);
            
			
            glTexCoord2f(1.0,1.0); 
            //glMultiTexCoord2f(GL_TEXTURE0, 1.0,1.0);
			glVertex3f((1.0 - aspect)/2.0 + aspect,1.0,0.0);
            
			
            glTexCoord2f(0.0,1.0); 
            //glMultiTexCoord2f(GL_TEXTURE0, 0.0,1.0);
			glVertex3f((1.0 - aspect)/2.0,1.0,0.0);
        }
		
        glEnd();
		
		
		glBindTexture(GL_TEXTURE_2D, 0); // unbind that fucker
		
		
		//glActiveTexture(0);
		
    } else {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
					"Stimulus image did not properly load. Showing nothing");
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
					"%s:%d", __FILE__, __LINE__);
    }
}

// override of basde class to provide more info
Data ImageStimulus::getCurrentAnnounceDrawData() {
    
    //mprintf("getting announce DRAW data for image stimulus %s",tag );
    
    Data announceData(M_DICTIONARY, 9);
    announceData.addElement(STIM_NAME,tag);        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_IMAGE);
    announceData.addElement(STIM_FILENAME,filename);  
    announceData.addElement(STIM_POSX,last_posx);  
    announceData.addElement(STIM_POSY,last_posy);  
    announceData.addElement(STIM_SIZEX,last_sizex);  
    announceData.addElement(STIM_SIZEY,last_sizey);  
    announceData.addElement(STIM_ROT,last_rot);  
//TODO    announceData.addElement(STIM_ALPHA,last_alpha);  
    
    return (announceData);
}


shared_ptr<mw::Component> ImageStimulusFactory::createObject(std::map<std::string, std::string> parameters,
													mwComponentRegistry *reg) {
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
			reg->getVariable(parameters["alpha_multiplier"], std::string("1"));	
	
	bf::path full_path = reg->getPath(parameters["working_path"], parameters["path"]);
		
	checkAttribute(x_size, parameters.find("reference_id")->second, "x_size", parameters["x_size"]);
	checkAttribute(y_size, parameters.find("reference_id")->second, "y_size", parameters.find("y_size")->second);
	checkAttribute(x_position, parameters.find("reference_id")->second, "x_position", parameters.find("x_position")->second);
	checkAttribute(y_position, parameters.find("reference_id")->second, "y_position", parameters.find("y_position")->second);
	checkAttribute(rotation, parameters.find("reference_id")->second, "rotation", parameters.find("rotation")->second);
	checkAttribute(alpha_multiplier,parameters.find("reference_id")->second, "alpha_multiplier", parameters.find("alpha_multiplier")->second);
	if(!bf::exists(full_path)) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "path", parameters.find("path")->second);
	}
	
	if(bf::is_directory(full_path)) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "path", parameters.find("path")->second);
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
	
	newImageStimulus->load(defaultDisplay.get());
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
    r = _r;     // should be 0-1    (1 is full saturation)
    g = _g; 
    b = _b;                                    
                         
}

PointStimulus::PointStimulus(const PointStimulus &tocopy) : 
				BasicTransformStimulus((const BasicTransformStimulus&)tocopy){
	r = tocopy.r;
	g = tocopy.g;
	b = tocopy.b;
}

PointStimulus::~PointStimulus(){ }

Stimulus * PointStimulus::frozenClone(){
	shared_ptr<Variable> x_clone(xoffset->frozenClone());
	shared_ptr<Variable> y_clone(yoffset->frozenClone());
	shared_ptr<Variable> xs_clone(xscale->frozenClone());
	shared_ptr<Variable> ys_clone(yscale->frozenClone());
	shared_ptr<Variable> rot_clone(rotation->frozenClone());
	shared_ptr<Variable> alpha_clone(alpha_multiplier->frozenClone());	
	shared_ptr<Variable> r_clone(r->frozenClone());
	shared_ptr<Variable> g_clone(g->frozenClone());
	shared_ptr<Variable> b_clone(b->frozenClone());

	PointStimulus *clone = new PointStimulus(tag,
											   x_clone,
											   y_clone,
											   xs_clone,
											   ys_clone,
											   rot_clone,
											   alpha_clone,
											   r_clone,
											   g_clone,
											   b_clone
											   );
	clone->setIsFrozen(true);
		
	return clone;
}


void PointStimulus::drawInUnitSquare(StimulusDisplay *display) {
    
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
Data PointStimulus::getCurrentAnnounceDrawData() {
    
    //mprintf("getting announce DRAW data for point stimulus %s",tag );
    
    Data announceData(M_DICTIONARY, 11);
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
/*
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
        
void FreeRunningMovieStimulus::draw(StimulusDisplay *display) {
    if(!running && GlobalCurrentExperiment->getInt(task_mode) != IDLE) {
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
    if(GlobalCurrentExperiment->getInt(task_mode) == IDLE && running) {
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
        
void FreeRunningMovieStimulus::draw(StimulusDisplay *display,float x, 
                                        float y, float sizex, float sizey) {
    BasicTransformStimulus::draw(display,x,y,sizex,sizey);
}
        
void FreeRunningMovieStimulus::drawInUnitSquare(StimulusDisplay *display) { }

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
 
