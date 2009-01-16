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
#include "Experiment.h"
#include "ComponentFactory.h"
namespace mw {
// a stimulus object with a position stored
// may still be an error here TODO because no destructor and copy 
// constructor does nothing
class OffsetStimulusContainer : public Stimulus {
	protected:
		shared_ptr<Stimulus> stim;
		shared_ptr<Variable> xoffset;
		shared_ptr<Variable> yoffset;

	public:
        /**
         * Constructor
         */
		OffsetStimulusContainer(std::string _tag, shared_ptr<Stimulus> _stim);
		OffsetStimulusContainer(std::string _tag, shared_ptr<Stimulus> _stim, 
								 shared_ptr<Variable> _xdeg, 
								 shared_ptr<Variable> _ydeg);
								 
        OffsetStimulusContainer(OffsetStimulusContainer& copy);
		~OffsetStimulusContainer();
        virtual Stimulus * frozenClone();
		
		virtual void draw(StimulusDisplay * display, float x, float y);
		virtual void draw(StimulusDisplay * display, float x, float y, 
                                                    float sizex, float sizey);
    private:
        // do not use the assignment operator, it does nothing
        // useful.  I made it private to hide it from users and to
        // prevent a default version from being used.
        void operator=(const OffsetStimulusContainer& l) { }
};

// a stimulus object with a position stored
// this is the base object from which many others should derive
class BasicTransformStimulus : public Stimulus {
	
	protected:
		shared_ptr<Variable> xoffset;
		shared_ptr<Variable> yoffset;
		
		shared_ptr<Variable> xscale;
		shared_ptr<Variable> yscale;
                
		shared_ptr<Variable> rotation; // planar rotation added in for free
		shared_ptr<Variable> alpha_multiplier;
        
        // JJD added these July 2006 to keep track of what was actually done for announcing things
        float last_posx, last_posy, last_sizex, last_sizey, last_rot;
        
	public:
        BasicTransformStimulus(std::string _tag, 
								shared_ptr<Variable> _xoffset, 
								shared_ptr<Variable> _yoffset,
								shared_ptr<Variable> _xscale, 
								shared_ptr<Variable> _yscale, 
								shared_ptr<Variable> _rot,
								shared_ptr<Variable> _alpha);
		BasicTransformStimulus(const BasicTransformStimulus& tocopy);
		~BasicTransformStimulus();
		
        virtual Stimulus * frozenClone();
		
		virtual void setTranslation(shared_ptr<Variable> _x, 
									shared_ptr<Variable> _y);
        virtual void setScale(shared_ptr<Variable> _scale);
        virtual void setScale(shared_ptr<Variable> _xscale, 
							  shared_ptr<Variable> _yscale);
        virtual void setRotation(shared_ptr<Variable> rot);
        virtual void draw(StimulusDisplay * display);
        virtual void draw(StimulusDisplay * display,float x, float y,
                                                    float sizex, float sizey);
        virtual Data getCurrentAnnounceDrawData();    
};

class OpenGLImageLoader {
	protected:
		

	public:
		static bool initialized;
		static Lockable *lock;
		
		static GLuint load(std::string filename, StimulusDisplay *display,
                                                   int *width, int *height);
};

// Image file based stimulus
class ImageStimulus : public BasicTransformStimulus {
    protected:
        std::string filename;
        ExpandableList<GLuint> *texture_maps;
		int width, height;
		
		// for debugging
		ConstantVariable *const0, *const2;
		
    public:
        ImageStimulus(std::string _tag, std::string filename, 
								shared_ptr<Variable> _xoffset, 
								shared_ptr<Variable> _yoffset, 
								shared_ptr<Variable> _xscale,
								shared_ptr<Variable> _yscale,
					   shared_ptr<Variable> _rot,
									shared_ptr<Variable> _alpha);
		// constructor for cloning an existing image stimulus
		ImageStimulus(std::string _tag, std::string _filename,
								ExpandableList<GLuint> *_texture_maps, 
								int _width,
								int _height,
								shared_ptr<Variable> _xoffset, 
                                shared_ptr<Variable> _yoffset, 
								shared_ptr<Variable> _xscale, 
                                shared_ptr<Variable> _yscale, 
					   shared_ptr<Variable> _rot,
								shared_ptr<Variable> _alpha);
								
        ImageStimulus(ImageStimulus& copy);
        virtual ~ImageStimulus();
		virtual Stimulus * frozenClone();
		
        std::string getFilename();
        virtual void drawInUnitSquare(StimulusDisplay *display);
        virtual void load(StimulusDisplay *display);
        virtual Data getCurrentAnnounceDrawData();
};

class ImageStimulusFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

// Simple point (e.g. for fixation) - JJD started, but not clear how to implement JJD TODO
// the x and y units are in the ambient open GL space
// how to specify size  ???
class PointStimulus : public BasicTransformStimulus {
    protected:
        shared_ptr<Variable> r;
		shared_ptr<Variable> g;
		shared_ptr<Variable> b;
        float last_r,last_g,last_b;
        
    public:
        PointStimulus(std::string _tag, shared_ptr<Variable> _xoffset, 
										shared_ptr<Variable> _yoffset, 
                                        shared_ptr<Variable> _xscale,
                                        shared_ptr<Variable> _yscale,
					   shared_ptr<Variable> _rot,
					   shared_ptr<Variable> _alpha,
                                        shared_ptr<Variable> _r, 
                                        shared_ptr<Variable> _g, 
                                        shared_ptr<Variable> _b);
		PointStimulus(const PointStimulus &tocopy);
		~PointStimulus();
		virtual Stimulus * frozenClone();
		
        virtual void drawInUnitSquare(StimulusDisplay *display);
        virtual Data getCurrentAnnounceDrawData();

};



       

// default copy constructor is ok here even though we are managing 
// a pointer to an Experiment.  It is not needed to copy the entire
// experiment because there should only be one copy in memory
/*class FreeRunningMovieStimulus : public BasicTransformStimulus {
    protected:
        // needs to know what experiment it belongs to (kludgey but true!)
        Experiment *experiment;
        long nframes;			// how many frames? negative numbers loop
        long frame_interval_ms;		// how long between frames?
        int current_frame;		// what frame are we on now?
        long start_time;
        // called internally to set the frame number before 
        // other draw functions are called
        void setCurrentFrame();
        bool running;
        ScheduleTask *schedule_node;

    public:    
        FreeRunningMovieStimulus(char *_tag, long nframes=1, long frame_interval=16, 
                                        float _xoffset=0, float _yoffset=0,
                                        float _xscale=1.0, float _yscale = 1.0,
                                        float _rot = 0.0);
        // set the frame and then go on as normal
        virtual void draw(StimulusDisplay * display);
        // overload this to provide teeth
        virtual void drawInUnitSquare(shared_ptr<StimulusDisplay> display);
        virtual void draw(StimulusDisplay * display,float x, float y, 
                                                    float sizex, float sizey);
        // start/stop rolling when this is called
        virtual void setVisible(bool vis);
        
};*/

// CompoundStimulus: a stimulus made up of several registered sub-stimuli
// ExpandableList being private means that you cannot call any of the
// expandable list methods outside of the implementation.
class CompoundStimulus : public Stimulus {
 
    protected:
        ExpandableList<OffsetStimulusContainer> *stimList;
        std::string currentSubTag;
        
	public:
        CompoundStimulus(std::string _tag);
        ~CompoundStimulus();
		
		virtual Stimulus * frozenClone();
		
        void addStimulus(shared_ptr<Stimulus> stim);
        void addStimulus(shared_ptr<Stimulus> stim, 
						 shared_ptr<Variable> _xloc, 
						 shared_ptr<Variable> _yloc);
						 
        virtual void draw(StimulusDisplay * display, 
						  float xdeg, float ydeg);
        void makeSubTag();
        virtual void announceStimulusDraw(MonkeyWorksTime now);  // override of base class method
        
};

class BlankScreen : public Stimulus{
// the default copy constructor is ok for this object.
// Variable(const Param& that) : r(that.r), g(that.g), b(that.b) { }
    protected:
        shared_ptr<Variable> r;
		shared_ptr<Variable> g;
		shared_ptr<Variable> b;
        float last_r,last_g,last_b;
        
    public:
        BlankScreen(std::string _tag, 
					 shared_ptr<Variable> r, 
					 shared_ptr<Variable> g, 
					 shared_ptr<Variable> b);
		virtual ~BlankScreen();
		virtual Stimulus * frozenClone();
		
        virtual void drawInUnitSquare(StimulusDisplay *display);
        virtual Data getCurrentAnnounceDrawData();
};

class BlankScreenFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

/*class VideoRequest {
	protected:
		StimID stimtodraw;
		Data xloc;
		Data yloc;
		
	public:
		VideoRequest(StimID _stim, Data _x, Data _y) {
			stimtodraw = _stim;
			xloc = _x;
			yloc = _y;
		}
        virtual ~VideoRequest() { }
		virtual void service();
};

void videoInterruptService();*/
}
#endif
