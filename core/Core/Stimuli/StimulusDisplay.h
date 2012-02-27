/*
 *  StimulusDisplay.h
 *  MWorksCore
 *
 *  Created by David Cox on 6/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef STIMULUS_DISPLAY_H_
#define STIMULUS_DISPLAY_H_

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include <CoreVideo/CVDisplayLink.h>


#include "Clock.h"
#include "LinkedList.h"
#include <vector>
#include "boost/enable_shared_from_this.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition_variable.hpp>

namespace mw {
	using namespace boost;
	
	// forward declarations
    class Datum;
	class Stimulus;
	class StimulusNode;
	class StimulusDisplay;
    class OpenGLContextManager;
    class VariableCallbackNotification;
	
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
	 */
		
  
	
	class StimulusDisplay : public enable_shared_from_this<StimulusDisplay> {
      protected:
        std::vector<int> context_ids;
		int current_context_index;
		shared_ptr< LinkedList<StimulusNode> > display_stack;
        
        shared_ptr<Clock> clock;
        shared_ptr<OpenGLContextManager> opengl_context_manager;
		
		boost::shared_mutex display_lock;
        typedef boost::shared_lock<boost::shared_mutex> shared_lock;
        typedef boost::upgrade_lock<boost::shared_mutex> upgrade_lock;
        typedef boost::upgrade_to_unique_lock<boost::shared_mutex> upgrade_to_unique_lock;
        boost::barrier refreshSync;
        boost::condition_variable_any refreshCond;
        bool waitingForRefresh;

        bool needDraw;
		
		GLdouble left, right, top, bottom; // display bounds
        
        shared_ptr<VariableCallbackNotification> stateSystemNotification;
        CVDisplayLinkRef displayLink;
        int64_t lastFrameTime;
        MWTime currentOutputTimeUS;
        
        std::vector< shared_ptr<StimulusNode> > stimsToAnnounce;
        std::vector<Datum> stimAnnouncements;
		
        virtual void glInit();
		virtual void setDisplayBounds();
        void refreshDisplay();
        void drawDisplayStack(bool doStimAnnouncements);
        void ensureRefresh(upgrade_lock &lock);

        static void announceDisplayUpdate(void *_display);
        void announceDisplayStack(MWTime time);
        Datum getAnnounceData();

        void stateSystemCallback(const Datum &data, MWorksTime time);
        static CVReturn displayLinkCallback(CVDisplayLinkRef displayLink,
                                            const CVTimeStamp *now,
                                            const CVTimeStamp *outputTime,
                                            CVOptionFlags flagsIn,
                                            CVOptionFlags *flagsOut,
                                            void *_display);
		
      public:
		
		StimulusDisplay();
		~StimulusDisplay();
        
        virtual void initialize();
		
		void addContext(int _context_id);
		
		int getNContexts() { return context_ids.size(); }
		void setCurrent(int i);	
		int getCurrentContextIndex() { return current_context_index; }
        void getCurrentViewportSize(GLint &width, GLint &height);
		
        shared_ptr<StimulusNode> addStimulus(shared_ptr<Stimulus> stim);
		void addStimulusNode(shared_ptr<StimulusNode> stimnode);
		
		void updateDisplay();
		void clearDisplay();
        void getDisplayBounds(GLdouble &left, GLdouble &right, GLdouble &bottom, GLdouble &top);
        double getMainDisplayRefreshRate();
        MWTime getCurrentOutputTimeUS() const { return currentOutputTimeUS; }
        
        static shared_ptr<StimulusDisplay> getCurrentStimulusDisplay();
		
		
        // Delegated methods for transformations
        // Default implementations are for orthographic display
        virtual void translate2D(double x_deg, double y_deg);	
        virtual void rotateInPlane2D(double rot_angle_deg);
        virtual void scale2D(double x_size_deg, double y_size_deg);  
        
        // Stimulus transformation in 
        virtual void translate2D_screenUnits(double x, double y);
        virtual void scale2D_screenUnits(double x_size, double y_size);
                
	  private:
        GLdouble degrees_per_screen_unit;
        StimulusDisplay(const StimulusDisplay& s) : refreshSync(2) { }
        void operator=(const StimulusDisplay& l) { }
    };


    class VirtualTangentScreenDisplay : public StimulusDisplay {

      protected:
        GLdouble screen_width, screen_height, screen_distance, screen_radius,
                 screen_aspect_ratio;
        GLdouble x_offset_screen_units, y_offset_screen_units;
        GLdouble fov_y_deg, fov_x_deg;
        GLdouble near_clip_distance, far_clip_distance;

        virtual void glInit();
		virtual void setDisplayBounds();

      public:
    
        VirtualTangentScreenDisplay();

        virtual void translate2D(double x_deg, double y_deg);	
        virtual void rotateInPlane2D(double rot_angle_deg);
        virtual void scale2D(double x_size_deg, double y_size_deg);        
        
        virtual void translate2D_screenUnits(double x, double y);
        virtual void scale2D_screenUnits(double x_size, double y_size);
        
	  private:
        VirtualTangentScreenDisplay(const VirtualTangentScreenDisplay& s);
        void operator=(const VirtualTangentScreenDisplay& l) { }
    };
    
}
#endif


