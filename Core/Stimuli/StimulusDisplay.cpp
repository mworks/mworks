/**
 * StimulusDisplay.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Fixed spacing.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "Stimulus.h" 
#include "StimulusNode.h"
#include "Utilities.h"
#include "OpenGLContextManager.h"
#include "StandardVariables.h"
#include "Experiment.h"
#include "StandardVariables.h"

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
 *                  StimulusDisplayChain Methods
 **********************************************************************/
StimulusDisplayChain::StimulusDisplayChain(
										StimulusDisplay* _display) : 
										LinkedList<StimulusNode>(){
    stimulus_display = _display;
}
        
StimulusDisplayChain::~StimulusDisplayChain() { }
		
        
        
void StimulusDisplayChain::execute() {
    shared_ptr<StimulusNode> node = getBackmost(); //tail;
            
    while(node != shared_ptr<StimulusNode>()) {
        if(node->isVisible()) {
            node->draw(stimulus_display);
		}
		
		node = node->getPrevious();
    }
}

void StimulusDisplayChain::announce(MonkeyWorksTime time) {
    shared_ptr<StimulusNode> node = getBackmost(); //tail;
	
    while(node != shared_ptr< LinkedListNode<StimulusNode> >()) {
		if(node->isVisible()) {
            node->announceStimulusDraw(time); 
		}
		
		node = node->getPrevious();
    }
	
}

Data StimulusDisplayChain::getAnnounceData() {
    shared_ptr<StimulusNode> node = getBackmost(); //tail;
	
	Data stimAnnounce(M_LIST, 1);
    while(node != shared_ptr< LinkedListNode<StimulusNode> >()) {
		if(node->isVisible()) {
			Data individualAnnounce(node->getCurrentAnnounceDrawData());
			if(!individualAnnounce.isUndefined()) {
				stimAnnounce.addElement(individualAnnounce);
			}
		}
		
		node = node->getPrevious();
    }	
	return stimAnnounce;
}



/**********************************************************************
 *                  StimulusDisplay Methods
 **********************************************************************/
StimulusDisplay::StimulusDisplay() {
    stimulus_chain = 
		shared_ptr<StimulusDisplayChain>(
			new StimulusDisplayChain(this)); // TODO    
    
	setDisplayBounds();
	update_stim_chain_next_refresh = false;

    //glInit();
    //GlobalOpenGLContextManager->flush(context_id);
}

StimulusDisplay::~StimulusDisplay(){
	//delete stimulus_chain;
}

// TODO: error checking
void StimulusDisplay::setCurrent(int i){ 
	current_context = context_ids[i];
	current_context_index = i;
	GlobalOpenGLContextManager->setCurrent(current_context); 
	
}


int StimulusDisplay::getNContexts(){ 
	return context_ids.size(); 
}

/*StimulusDisplay::StimulusDisplay(StimulusDisplayChain *chain){
    StimulusDisplay(-1, chain);
}*/
   
/*StimulusDisplay::StimulusDisplay(int _context_id,
                            StimulusDisplayChain *chain) : Lockable() {
    stimulus_chain = chain;
    context_id = _context_id;
}*/

/*void StimulusDisplay::drawGLTest() {

    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
		    
		    
    glBegin(GL_TRIANGLES);
		    
    glColor3f(0.0, 0.0, 0.0);
    glVertex2f(-1.0, -1.0);
    glVertex2f(0.0, 1);
    glVertex2f(1.0, -1);
		    
    glEnd();
    glFinish();
}*/

/*void StimulusDisplay::setContextID(int _context_id) {
    context_id = _context_id;
}*/

/*void StimulusDisplay::setStimulusChain(StimulusDisplayChain *chain) {
    stimulus_chain = chain;
}*/

shared_ptr<StimulusNode> StimulusDisplay::addStimulus(shared_ptr<Stimulus> stim) {
    if(!stim) {
        mprintf("Attempt to load NULL stimulus");
        return shared_ptr<StimulusNode>();
    }

    //stim->setStimulusDisplay(this); // tag it on the way by...
    
    //setCurrent();
	/*for(int i = 0; i < context_ids->getNElements(); i++){
		int context_id = *(context_ids->getElement(i));
		GlobalOpenGLContextManager->setCurrent(context_id);
		stim->load(this);
	}*/
	
	shared_ptr<StimulusNode> stimnode(new StimulusNode(stim));
	
    stimulus_chain->addToFront(stimnode);
	
	return stimnode;
}

void StimulusDisplay::addStimulusNode(shared_ptr<StimulusNode> stimnode) {
    if(!stimnode) {
        mprintf("Attempt to load NULL stimulus");
        return;
    }
	
	
	// remove it, in case it already belongs to a list
	stimnode->remove();
	
	stimulus_chain->addToFront(stimnode);  // TODO
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
	Data display_info = *mainDisplayInfo; // from standard variables
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
	GlobalOpenGLContextManager->setCurrent(_context_id);
	glInit();
}

GLuint StimulusDisplay::getCurrentContext(){
	return current_context;
}

int StimulusDisplay::getCurrentContextIndex(){
	return current_context_index;
}

void *performAsynchronousUpdateDisplay(const shared_ptr<StimulusDisplay> &sd){
	sd->updateDisplay();
	return 0;
}

void StimulusDisplay::asynchronousUpdateDisplay() {
	boost::mutex::scoped_lock d_lock(display_lock);
	if(!update_stim_chain_next_refresh) {
		update_stim_chain_next_refresh = true;
		
		shared_ptr<Scheduler> scheduler = Scheduler::instance();
		
		shared_ptr<StimulusDisplay> this_one = shared_from_this();
		scheduler->fork(boost::bind(performAsynchronousUpdateDisplay,
									this_one),
						M_DEFAULT_STIMULUS_PRIORITY);
	}
}


void StimulusDisplay::updateDisplay() {
	boost::mutex::scoped_lock lock(display_lock);
	
	shared_ptr <Clock> clock = Clock::instance();
	MonkeyWorksTime before_draw = clock->getCurrentTimeUS();
	update_stim_chain_next_refresh = false;
	
	for(unsigned int i = 0; i < context_ids.size(); i++){
		setCurrent(i);
		//int context_id = *(context_ids->getElement(i));
		//GlobalOpenGLContextManager->setCurrent(context_id);
		
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
		
		stimulus_chain->execute();
		
		if(i == 0){ // only for the main display
			glSetFenceAPPLE(GlobalOpenGLContextManager->getFence());
		}
		GlobalOpenGLContextManager->flush(current_context);
		
		/*	GLuint *fence_copy = new GLuint[1];
		 fence_copy[0] = GlobalOpenGLContextManager->getFence();
		 GlobalScheduler->fork(&checkFence,
		 (void *)(fence_copy),				// leaks
		 60);*/
		
		
		if(i == 0){  // only for the first (main) display
			glFinishFenceAPPLE(GlobalOpenGLContextManager->getFence());
			
			MonkeyWorksTime now = clock->getCurrentTimeUS();
			
			
			stimDisplayUpdate->setValue(stimulus_chain->getAnnounceData(), now);
			stimulus_chain->announce(now);
			MonkeyWorksTime slop = 2*(1000000/GlobalOpenGLContextManager->getDisplayRefreshRate(GlobalOpenGLContextManager->getMainDisplayIndex()));
			if(now-before_draw > slop) {
				merror(M_DISPLAY_MESSAGE_DOMAIN,
					   "updating main window display is taking longer than two frames (%lld > %lld) to update", 
					   now-before_draw, 
					   slop);		
			}
		}
	}	
}

void StimulusDisplay::clearDisplay(){

	shared_ptr<StimulusNode> node = stimulus_chain->getFrontmost();
	while(node != shared_ptr<StimulusNode>()){
		
		node->setVisible(false);
		
		node = node->getNext();
	}
	
	//glInit();
	updateDisplay();

}

void StimulusDisplay::glInit() {
    // This Will Clear The Background Color To Black
    //glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    // Enables Clearing Of The Depth Buffer
    //glClearDepth(1.0);	
    // The Type Of Depth Test To Do
    //glDepthFunc(GL_LESS);			
    // Enables Depth Testing
    //glEnable(GL_DEPTH_TEST);

    				
	
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_TEXTURE_RECTANGLE_EXT);
	//glShadeModel(GL_SMOOTH);	
	
	//for(int i = 0; i < getNContexts(); i++){

		//setCurrent(i);
		// Conservatism:
		glShadeModel(GL_FLAT);
		glDisable(GL_BLEND);
		glDisable(GL_DITHER);
		glDisable(GL_FOG);
		glDisable(GL_LIGHTING);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // DDC added
		
		//glDisable(GL_TEXTURE_2D);
		
		//glEnable (GL_BLEND); 
		//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		//glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		//glClearAccum(0.0, 0.0, 0.0, 0.0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); // Reset The Projection Matrix
		
		//glOrtho(-10.0,10.0,-10.0,10.0, 10,-10);
		gluOrtho2D(-16.0,16.0,-12.0,12.0);
		glMatrixMode(GL_MODELVIEW);
		
		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
	//}
}



/*StimulusNodeGroup::StimulusNodeGroup(){
	group = new ExpandableList<StimulusNode>();
}

void StimulusNodeGroup::addStimulusNode(StimulusNode *node){
	group->addElement(node);
}

StimulusNode *StimulusNodeGroup::getStimulusNode(int index){
	return group->getElement(index);
}
*/

StimulusNodeGroup::StimulusNodeGroup(shared_ptr<StimulusGroup> group){
	for(int i = 0; i < group->getNElements(); i++){
		shared_ptr<StimulusNode> p(new StimulusNode(group->getElement(i)));
		addReference(p);
	} // TODO leaks
}

StimulusGroupReferenceNode::StimulusGroupReferenceNode(
							shared_ptr<StimulusNodeGroup> _group,
							shared_ptr<Variable> _index) : StimulusNode(){

	stimulus_nodes = _group;
	index = _index;
}


int StimulusGroupReferenceNode::getIndexValue(){
	int the_index = (*index);
	int nelements = stimulus_nodes->getNElements();
	if(the_index >= nelements ||
	   the_index < 0){
		merror(M_DISPLAY_MESSAGE_DOMAIN, 
			   "Attempt to access illegal index (%d) in stimulus group (nelements = %d)", 
			   the_index, 
			   nelements);
		return -1;
	}
	
	return the_index;
	
}

void StimulusGroupReferenceNode::addToDisplay(shared_ptr<StimulusNode> stimnode, shared_ptr<StimulusDisplay> display){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		display->addStimulusNode(stimulus_nodes->getElement(index_value));
	}
}
							
// set the "frozen" state of the node
void StimulusGroupReferenceNode::setFrozen(bool _frozen){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->setFrozen(_frozen);
	}
}
	
bool StimulusGroupReferenceNode::isFrozen(){

	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->isFrozen();
	} else {
		return false;
	}
}

// Passthrough to the referenced node
void StimulusGroupReferenceNode::draw(StimulusDisplay * display){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->draw(display);
	}
}


void StimulusGroupReferenceNode::setVisible(bool _vis){ // JJD add
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->setVisible(_vis);
	}
}

bool StimulusGroupReferenceNode::isVisible(){          // JJD add
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->isVisible();
	} else {
		return false;
	}
}	
	
/*void StimulusGroupReferenceNode::setVisibleOnLastUpdate(bool _vis){ // JJD add
	(stimulus_nodes->getElement((int)(*index)))->setVisibleOnLastUpdate(_vis);
}

bool StimulusGroupReferenceNode::wasVisibleOnLastUpdate(){          // JJD add
	return (stimulus_nodes->getElement((int)(*index)))->wasVisibleOnLastUpdate();
}*/

void StimulusGroupReferenceNode::announceStimulusDraw(MonkeyWorksTime time){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->announceStimulusDraw(time);
	}
}

Data StimulusGroupReferenceNode::getCurrentAnnounceDrawData(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->getCurrentAnnounceDrawData();
	} else {
		Data undef;
		return undef;
	}
}


void StimulusGroupReferenceNode::remove(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->remove();
	}
}

void StimulusGroupReferenceNode::bringToFront(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->bringToFront();
	}
}

void StimulusGroupReferenceNode::sendToBack(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->sendToBack();
	}
}


void StimulusGroupReferenceNode::bringForward(){
	
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->moveForward();
	}
}

void StimulusGroupReferenceNode::sendBackward(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->moveBackward();
	}
}

