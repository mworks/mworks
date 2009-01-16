/**
 * Stimulus.h
 *
 * Description: An Stimulus object is an encapsulation of a piece of OpenGL 
 * code, to be executed within the context of a display chain which controls
 * the order of execution and which allows rapid switching on and off 
 * of objects.
 *
 * All Stimulus objects have the following methods:
 *
 * draw: executes the opengl calls to draw the stimulus, assuming units 
 * are in degrees (must call load first!)
 *
 * setVisible: sets the visible flag
 *
 * isVisible: returns the visible flag value
 *
 * precache: cache a stimulus for faster drawing
 *
 * load: prepare for a draw call
 *
 * drawThumbnail: draw a thumnail version of the stimulus for 
 * host display purposes
 *
 * getBounds: returns an array defining the bounds of the stimulus
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Added Copy constructor and destructor to 
 *                              Stimulus.  Fixed spacing.  Hiding copy
 *                          constructors and = operators for classes that
 *                          should not be copied.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */
 
#ifndef STIMULUS_H
#define STIMULUS_H

#include "StimulusDisplay.h"
#include "StimulusNode.h"

#include "GenericData.h"
#include "GenericVariable.h"
#include "ExpandableList.h"
#include "Lockable.h"
#include "Utilities.h"
#include "Announcers.h"
#include <vector>

#ifdef	__APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#elif	linux
	// TODO: where are these under linux?
#endif

#define ILUT_USE_OPENGL
#include <IL/ilut.h>
namespace mw {
typedef int StimID;

// A hack for now
#define M_STIMULUS_DISPLAY_LEFT_EDGE		-26.57
#define M_STIMULUS_DISPLAY_RIGHT_EDGE		26.57
#define M_STIMULUS_DISPLAY_BOTTOM_EDGE		-17.77
#define M_STIMULUS_DISPLAY_TOP_EDGE			17.77


class StimulusDisplay; // later in this file...


class Stimulus : public Announcable, public mw::Component {
    private:
        // prevent the use of the = operator.
        //mStimulus& operator=(const Stimulus& lval) { }

	protected:
        bool loaded, visible, cached, has_thumbnail;
        Stimulus *thumbnail;

        
		
		bool frozen;
		
		/*CollectionPoint *collection_point;*/
        
    public:
        /**
         * Default Constructor.  Sets all members to false.
         */
        Stimulus(std::string _tag);
        
        /**
         * Copy Constructor.  Deeply copies object
         */
        Stimulus(const Stimulus& copy);
        
		
		/**
		 * "Frozen" clone method:
		 *
		 *  Make a new stimulus object with all of the variables converted
		 *  into constants (effectively "freezing" it in time)
		 */
		 virtual Stimulus *frozenClone(); 
		
        /**
         * Destructor.
         */
        virtual ~Stimulus();
        
        /**
         * Overload this function.  This is a shell that does nothing.  Used
         * to load a stimulus into memory.
         */
        virtual void load(StimulusDisplay *display);
        
        /**
         * Sets whether this stimulus is visible.
         */
        virtual void setVisible(bool newvis);

        /**
         * This is the method that most will want to override
         * this specifies how to draw the stimulus in the 
         * interval [(0,1), (0,1)].  This default version does nothing.
         */
        virtual void drawInUnitSquare(StimulusDisplay *display);

        /**
         * Draws a stimulus in display 'display' at 0,0 with size 1,1.
         * Calls drawInUnitSquare() member function
         */
        virtual void draw(StimulusDisplay *display);

        /**
         * Draws a stimulus in display 'display' at x,y with size 1,1.
         * Calls drawInUnitSquare() member function
         */
        virtual void draw(StimulusDisplay *display, float x, float y);

        /**
         * Draws a stimulus in display 'display' at x,y with size sizex,sizey.
         * Calls drawInUnitSquare() member function
         */    
        virtual void draw(StimulusDisplay *display, float x, float y, 
                                                    float sizex, float sizey);

        /**
         * Option pre-caching optimization. Overload it if you can do it
         * This version does nothing.
         */
        virtual void precache();
        
        /**
         * Draws a thumbnail version.  This default version calls the
         * draw() function with the same arguments.
         */
        virtual void drawThumbnail(StimulusDisplay *display, 
														float x, float y);

        /**
         * Draws a thumbnail version.  This default version calls the
         * draw() function with the same arguments.
         */
        virtual void drawThumbnail(StimulusDisplay *display, 
                                                float x, float y, 
												float sizex, float sizey);

        /**
         * Returns the bounds of the stimulus.  This version returns NULL.
         */
        virtual int* getBounds();
        
        /**
         * Inspection methods.
         */
        bool isLoaded();
        bool isVisible();
        bool isCached();
        bool hasThumbnail();
        Stimulus * getThumbnail();
        std::string gettag();
        
        /** 
         *  announcement methods // JJD
         *  two methods are used to do different behavior for draw vs. erase
        */
        virtual void announceStimulusDraw(MonkeyWorksTime now); 

        
        // these method should be overriden to provide more announcement detail       
        virtual Data getCurrentAnnounceDrawData();  
        
		
		void setIsFrozen(bool _isit){  frozen = _isit; }
        
};


/**
 * StimulusGroup provides a simple mechanism for creating groups of stimuli
 * as an organizational convenience.  Its use is not necessary
 */
class StimulusGroup : public ExpandableList<Stimulus>, public mw::Component, public ComponentFactory {
private:
	std::vector <shared_ptr<StimulusGroup> > sub_groups;
	
	weak_ptr<StimulusGroup> parent;

	void addSubGroup(const shared_ptr <StimulusGroup> stim_group);
	void addStimToParents(const shared_ptr <Stimulus> stim);
	
public:
	StimulusGroup();
	StimulusGroup(const std::string &_tag);     
	virtual ~StimulusGroup();
	
	unsigned int dimensionSize(const unsigned int dimension);
	unsigned int getNDimensions() const;

	shared_ptr<StimulusGroup> getSubGroup(const unsigned int sub_group_index) const;
	shared_ptr<StimulusGroup> getParent() const;
	
	void setParent(weak_ptr <StimulusGroup> _parent);
	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
	
	virtual void addChild(std::map<std::string, std::string> parameters,
						  mwComponentRegistry *reg,
						  shared_ptr<mw::Component> child);
	
};
}
#endif
