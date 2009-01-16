/*
 *  StimulusDisplay.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 6/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef STIMULUS_DISPLAY_H_
#define STIMULUS_DISPLAY_H_

#ifdef	__APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif	linux
// TODO: where are these under linux?
#endif

#include "LinkedList.h"
#include "ExpandableList.h"
#include <vector>
#include "boost/enable_shared_from_this.hpp"

#include <boost/shared_ptr.hpp>
namespace mw {
	using namespace boost;
	

	
	
	
	class Stimulus;
	class StimulusNode;
	class StimulusDisplay;
	
	/**
	 * StimulusDisplay represents a single abstracted stimulus display.
	 * The idea is that after initialization, you can use routines in here
	 * to set up a display chain, configure high-level changes in the 
	 * OpenGL context, control the drawing to the display.
	 *
	 * All of the details of actually creating OpenGL contexts (i.e. platform
	 * specific stuff) is handled under the hood by the OpenGLContextManager.
	 * This includes actual Cocoa or AGL calls to create, attach, and configure
	 * the contexts.  OpenGLContextManager will handle the details of doing its
	 * best in terms of producing an optimally accelerated and meeting the color
	 * demands of the user (also full-screen AA).  Functions will be provided for 
	 * reporting back exactly what StimulusDisplay is getting.  Eventually, 
	 * a ContextViewer utility will display the status and stats for every
	 * context, and possibly allow users to create and change them. 
	 *
	 * May want to move the actual context itself into this class, at the risk of 
	 * losing perfect portability, for performance's sake.  Right now, context
	 * changes and flushes go through the ContextManager, which involves several
	 * function calls and pointer dereferences.
	 
	 * The display chain is a simple mechanisms for allowing OpenGL calls to 
	 * be bundled up into Stimulus and StimulusTransformation objects while 
	 * still allowing for some control over what order everything is drawn in.
	 * This helps when we're drawing stuff into a 2D plane and we don't
	 * necessarily want to worry about depth and stencil buffers.
	 *
	 * Programmatically, the display chain is a linked list.  Items are by 
	 * default added to the head (draw in front), but send (to) back/front 
	 * functions are also provided.  Elements can be removed from the list
	 * or simply inactivated as necessary.  Background optimization of OpenGL 
	 * calls into display lists can occur with or without the users knowledge.
	 * Grouped stimuli should be automatically optimized.  Mechanisms for 
	 * dynamically monitoring groupings of stimuli and determining efficient 
	 * displaylist optimization is also possible.  In particular, as long as the
	 * display chain has unchanging membership (but members are turned on
	 * and off), it is possible to compile all possible combinations of frames
	 * and then set up a finite state machine to determine which compiled frame 
	 * to show at any given moment.  Transitions in the FSM are determined by 
	 * the activate/inactive calls in the chain.
	 *
	 * NOTE: StimulusNode objects CANNOT be added to ExpandableList objects
	 * because the copy constructor is declared private to prevent such  action.
	 */
	class StimulusDisplayChain : public LinkedList<StimulusNode> {
    protected:
		
		StimulusDisplay *stimulus_display;
		
    public:
		StimulusDisplayChain(StimulusDisplay *display);
        virtual ~StimulusDisplayChain();
		
        void execute();
		void announce(MonkeyWorksTime time);
		Data getAnnounceData();
		
    private:
        StimulusDisplayChain(const StimulusDisplayChain& s) { }
        void operator=(const StimulusDisplayChain& l) { }
	};
	
	
	
	class StimulusDisplay : public enable_shared_from_this<StimulusDisplay> {
    protected:
        std::vector<int> context_ids;
		shared_ptr<StimulusDisplayChain> stimulus_chain;
		
		GLuint current_context;
		int current_context_index;
		
		boost::mutex display_lock;
		
		double left, right, top, bottom; // display bounds
		
		bool update_stim_chain_next_refresh;
		
        void glInit();
		
    public:
		
		StimulusDisplay();
		~StimulusDisplay();
		
		void addContext(int _context_id);
		
		// TODO: error checking on these!
		int getNContexts();
		void setCurrent(int i);	
		GLuint getCurrentContext();	
		int getCurrentContextIndex();
		
        shared_ptr<StimulusNode> addStimulus(shared_ptr<Stimulus> stim);
		void addStimulusNode(shared_ptr<StimulusNode> stimnode);
		
		void asynchronousUpdateDisplay(); // should not be called externally
		void updateDisplay();
		void clearDisplay();
		void setDisplayBounds();
		
	private:
        StimulusDisplay(const StimulusDisplay& s) { }
        void operator=(const StimulusDisplay& l) { }
	};
}
#endif


