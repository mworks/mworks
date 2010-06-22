/**
 * StimulusDisplay.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Fixed spacing.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "OpenGLContextManager.h"
#include "Stimulus.h" 
#include "StimulusNode.h"
#include "Utilities.h"
#include "StandardVariables.h"
#include "Experiment.h"
#include "StandardVariables.h"
#include "ComponentRegistry.h"

#include "boost/bind.hpp"


#ifdef	__APPLE__
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#elif	linux
	// TODO: where are these in linux?
#endif
using namespace mw;



/**********************************************************************
 *                  StimulusDisplay Methods
 **********************************************************************/
StimulusDisplay::StimulusDisplay() {

    // defer creation of the display chain until after the stimulus display has been created
    display_stack = shared_ptr< LinkedList<StimulusNode> >(new LinkedList<StimulusNode>());
    
	setDisplayBounds();
	update_stim_chain_next_refresh = false;

    opengl_context_manager = OpenGLContextManager::instance();
    clock = Clock::instance();
    
}

StimulusDisplay::~StimulusDisplay(){
	// nothing to do
}

// TODO: error checking
void StimulusDisplay::setCurrent(int i){ 
	current_context = context_ids[i];
	current_context_index = i;
    
    
	opengl_context_manager->setCurrent(current_context); 
	
}


int StimulusDisplay::getNContexts(){ 
	return context_ids.size(); 
}



shared_ptr<StimulusNode> StimulusDisplay::addStimulus(shared_ptr<Stimulus> stim) {
    if(!stim) {
        mprintf("Attempt to load NULL stimulus");
        return shared_ptr<StimulusNode>();
    }

	shared_ptr<StimulusNode> stimnode(new StimulusNode(stim));
	
    display_stack->addToFront(stimnode);
	
	return stimnode;
}

void StimulusDisplay::addStimulusNode(shared_ptr<StimulusNode> stimnode) {
    if(!stimnode) {
        mprintf("Attempt to load NULL stimulus");
        return;
    }
    
	// remove it, in case it already belongs to a list
	stimnode->remove();
	
	display_stack->addToFront(stimnode);  // TODO
}



/*int StimulusDisplay::getContextID() {
    return context_id;
}
        
void StimulusDisplay::setCurrent() {
    GlobalOpenGLContextManager->setCurrent(context_id);
}*/


void *checkFence(void *arg){
	GLuint fence = *((GLuint *)arg);
	
	glFinishFenceAPPLE(fence);
	
	// JJD added code for stimulus timing test June 2006
	//mprintf("mStimulusDisplay::updateDisplay:  Updated display (supposedly).  Setting stimDisplayUpdate variable");
    stimDisplayUpdate->setValue(true);
	
	return 0;
}

void StimulusDisplay::setDisplayBounds(){
  shared_ptr<mw::ComponentRegistry> reg = mw::ComponentRegistry::getSharedRegistry();
  shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
  
 Datum display_info = *main_screen_info; // from standard variables
	if(display_info.getDataType() == M_DICTIONARY &&
	   display_info.hasKey(M_DISPLAY_WIDTH_KEY) &&
	   display_info.hasKey(M_DISPLAY_HEIGHT_KEY) &&
	   display_info.hasKey(M_DISPLAY_DISTANCE_KEY)){
	
		float width_unknown_units = display_info.getElement(M_DISPLAY_WIDTH_KEY);
		float height_unknown_units = display_info.getElement(M_DISPLAY_HEIGHT_KEY);
		float distance_unknown_units = display_info.getElement(M_DISPLAY_DISTANCE_KEY);
	
		float half_width_deg = (180. / M_PI) * atan((width_unknown_units/2.)/distance_unknown_units);
		float half_height_deg = half_width_deg * height_unknown_units / width_unknown_units;
		//float half_height_deg = (180. / M_PI) * atan((height_unknown_units/2.)/distance_unknown_units);
		
		left = -half_width_deg;
		right = half_width_deg;
		top = half_height_deg;
		bottom = -half_height_deg;
	} else {
		left = M_STIMULUS_DISPLAY_LEFT_EDGE;
		right = M_STIMULUS_DISPLAY_RIGHT_EDGE;
		top = M_STIMULUS_DISPLAY_TOP_EDGE;
		bottom = M_STIMULUS_DISPLAY_BOTTOM_EDGE;
	}
	
	mprintf("Display bounds set to (%g left, %g right, %g top, %g bottom)",
			left, right, top, bottom);
}

void StimulusDisplay::addContext(int _context_id){
	context_ids.push_back(_context_id);
	current_context_index = context_ids.size();
	current_context = _context_id;
    opengl_context_manager->setCurrent(_context_id);
	glInit();
    glFinish();
    opengl_context_manager->updateAndFlush(_context_id);
}

GLuint StimulusDisplay::getCurrentContext(){
	return current_context;
}

int StimulusDisplay::getCurrentContextIndex(){
	return current_context_index;
}

void StimulusDisplay::updateDisplay(bool explicit_update) {
    
	boost::mutex::scoped_lock lock(display_lock);
	

    int refresh_rate = opengl_context_manager->getDisplayRefreshRate(opengl_context_manager->getMainDisplayIndex());
    if(refresh_rate <= 0){
        refresh_rate = 60;
    }
    
    
	MWTime before_draw = clock->getCurrentTimeUS();
	update_stim_chain_next_refresh = false;
	
	for(unsigned int i = 0; i < context_ids.size(); i++){
        
        setCurrent(i); // Set the current OpenGL context
		
        // OpenGL setup
		glShadeModel(GL_FLAT);
		glDisable(GL_BLEND);
		glDisable(GL_DITHER);
		glDisable(GL_FOG);
		glDisable(GL_LIGHTING);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); // Reset The Projection Matrix
		
		
		gluOrtho2D(left, right, bottom, top);
		glMatrixMode(GL_MODELVIEW);
		
        
        // Actually draw all of the stimuli in the chain
		//MWTime chain_start = clock->getCurrentTimeUS();
        drawDisplayStack(explicit_update);
        //MWTime chain_end = clock->getCurrentTimeUS();
        
#define USE_GL_FENCE
#define ERROR_ON_LATE_FRAMES

        
		if(i == 0){ // only for the main display
            #ifdef USE_GL_FENCE
            if(opengl_context_manager->hasFence()){
                glSetFenceAPPLE(opengl_context_manager->getFence());
            }
            #endif
            
            opengl_context_manager->flush(i);
		} else {
            opengl_context_manager->updateAndFlush(i);
        }

        //MWTime flush_start = clock->getCurrentTimeUS();
		//MWTime flush_end = clock->getCurrentTimeUS();
		
		if(i == 0){  // only for the first (main) display

#ifdef USE_GL_FENCE
            if(opengl_context_manager->hasFence()){
                glFinishFenceAPPLE(opengl_context_manager->getFence());
			}
#endif
            
#ifdef ERROR_ON_LATE_FRAMES
            MWTime now = clock->getCurrentTimeUS();
			
			
			stimDisplayUpdate->setValue(getAnnounceData(), now);
			announceDisplayStack(now);
            
			MWTime slop = 2*(1000000/refresh_rate);
            
//          std::cerr << "now - before_draw: " << now - before_draw << 
//                       " |  chain end - chain start: " << chain_end - chain_start <<
//                       " |  flush end - flush start: " << flush_end - flush_start << std::endl;
			
            if(now-before_draw > slop) {
				merror(M_DISPLAY_MESSAGE_DOMAIN,
					   "updating main window display is taking longer than two frames (%lld > %lld) to update", 
					   now-before_draw, 
					   slop);		
			}
#endif
            
		}

        
	}	
}

void StimulusDisplay::clearDisplay(){

	shared_ptr<StimulusNode> node = display_stack->getFrontmost();
	while(node != shared_ptr<StimulusNode>()){
		
		node->setVisible(false);
		
		node = node->getNext();
	}
	
	//glInit();
	updateDisplay();

}

void StimulusDisplay::glInit() {

    glShadeModel(GL_FLAT);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // DDC added
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reset The Projection Matrix
    
    gluOrtho2D(-16.0,16.0,-12.0,12.0);
    glMatrixMode(GL_MODELVIEW);
    
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

}


void StimulusDisplay::drawDisplayStack(bool explicit_update) {
    
    shared_ptr<StimulusNode> node = display_stack->getBackmost(); //tail;
    
    
    while(node != shared_ptr<StimulusNode>()) {
        
        if(explicit_update && node->isPending()){
            
            // we're taking care of the pending state, so
            // clear this flag
            node->clearPending();
            
            // on "explicit" updates, we'll convert "pending
            // visible" stimuli to "visible" ones
            node->setVisible(node->isPendingVisible());
            
            if(node->isPendingRemoval()){
                node->clearPendingRemoval();
                node->remove();
                continue;
            }
            
            
        }
        
        if(node->isVisible()) {
            node->draw(shared_from_this());
        }
        
        node = node->getPrevious();
        
    }
}

void StimulusDisplay::announceDisplayStack(MWTime time) {
    shared_ptr<StimulusNode> node = display_stack->getBackmost(); //tail;
	
    while(node != shared_ptr< LinkedListNode<StimulusNode> >()) {
		if(node->isVisible()) {
            node->announceStimulusDraw(time); 
		}
		
		node = node->getPrevious();
    }
	
}

Datum StimulusDisplay::getAnnounceData() {
    shared_ptr<StimulusNode> node = display_stack->getBackmost(); //tail;
	
    Datum stimAnnounce(M_LIST, 1);
    while(node != shared_ptr< LinkedListNode<StimulusNode> >()) {
		if(node->isVisible()) {
            Datum individualAnnounce(node->getCurrentAnnounceDrawData());
			if(!individualAnnounce.isUndefined()) {
				stimAnnounce.addElement(individualAnnounce);
			}
		}
		
		node = node->getPrevious();
    }	
	return stimAnnounce;
}





