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
#include "StimulusNode.h"
#include "Utilities.h"
#include "StandardVariables.h"
#include "Experiment.h"
#include "StandardVariables.h"
#include "ComponentRegistry.h"
#include "VariableNotification.h"

#include <CoreAudio/HostTime.h>

#include "boost/bind.hpp"


// A hack for now
#define M_STIMULUS_DISPLAY_LEFT_EDGE		-26.57
#define M_STIMULUS_DISPLAY_RIGHT_EDGE		26.57
#define M_STIMULUS_DISPLAY_BOTTOM_EDGE		-17.77
#define M_STIMULUS_DISPLAY_TOP_EDGE			17.77


BEGIN_NAMESPACE_MW


StimulusDisplay::StimulusDisplay(bool announceIndividualStimuli) :
    current_context_index(-1),
    projectionMatrix(GLKMatrix4Identity),
    displayLinksRunning(false),
    mainDisplayRefreshRate(0.0),
    currentOutputTimeUS(-1),
    announceIndividualStimuli(announceIndividualStimuli),
    announceStimuliOnImplicitUpdates(true)
{
    // defer creation of the display chain until after the stimulus display has been created
    display_stack = shared_ptr< LinkedList<StimulusNode> >(new LinkedList<StimulusNode>());
    
	setDisplayBounds();
    setBackgroundColor(0.5, 0.5, 0.5);

    opengl_context_manager = OpenGLContextManager::instance();
    clock = Clock::instance();

    waitingForRefresh = false;
    needDraw = false;
    
    std::memset(&currentOutputTimeStamp, 0, sizeof(currentOutputTimeStamp));
    
    stateSystemNotification = shared_ptr<VariableCallbackNotification>(new VariableCallbackNotification(boost::bind(&StimulusDisplay::stateSystemCallback, this, _1, _2)));
    state_system_mode->addNotification(stateSystemNotification);
}

StimulusDisplay::~StimulusDisplay(){
    stateSystemNotification->remove();
    
    for (auto dl : displayLinks) {
        CVDisplayLinkRelease(dl);
    }
}

OpenGLContextLock StimulusDisplay::setCurrent(int i){
    if ((i >= getNContexts()) || (i < 0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "invalid context index (%d)", i);
        return OpenGLContextLock();
    }

	return opengl_context_manager->setCurrent(context_ids[i]);
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

void StimulusDisplay::getDisplayBounds(const Datum &display_info,
                                       GLdouble &left,
                                       GLdouble &right,
                                       GLdouble &bottom,
                                       GLdouble &top)
{
	if(display_info.getDataType() == M_DICTIONARY &&
	   display_info.hasKey(M_DISPLAY_WIDTH_KEY) &&
	   display_info.hasKey(M_DISPLAY_HEIGHT_KEY) &&
	   display_info.hasKey(M_DISPLAY_DISTANCE_KEY)){
	
		GLdouble width_unknown_units = display_info.getElement(M_DISPLAY_WIDTH_KEY);
		GLdouble height_unknown_units = display_info.getElement(M_DISPLAY_HEIGHT_KEY);
		GLdouble distance_unknown_units = display_info.getElement(M_DISPLAY_DISTANCE_KEY);
	
		GLdouble half_width_deg = (180. / M_PI) * atan((width_unknown_units/2.)/distance_unknown_units);
		GLdouble half_height_deg = half_width_deg * height_unknown_units / width_unknown_units;
		//GLdouble half_height_deg = (180. / M_PI) * atan((height_unknown_units/2.)/distance_unknown_units);
		
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
}

void StimulusDisplay::setDisplayBounds(){
    shared_ptr<mw::ComponentRegistry> reg = mw::ComponentRegistry::getSharedRegistry();
    shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
    
    Datum display_info = *main_screen_info; // from standard variables
    getDisplayBounds(display_info, left, right, bottom, top);
    
    projectionMatrix = GLKMatrix4MakeOrtho(left, right, bottom, top, -1.0, 1.0);
	
	mprintf("Display bounds set to (%g left, %g right, %g top, %g bottom)",
			left, right, top, bottom);
}

void StimulusDisplay::getDisplayBounds(GLdouble &left, GLdouble &right, GLdouble &bottom, GLdouble &top) {
    left = this->left;
    right = this->right;
    bottom = this->bottom;
    top = this->top;
}

void StimulusDisplay::setBackgroundColor(GLclampf red, GLclampf green, GLclampf blue) {
    backgroundRed = red;
    backgroundGreen = green;
    backgroundBlue = blue;
}

void StimulusDisplay::setAnnounceStimuliOnImplicitUpdates(bool announceStimuliOnImplicitUpdates) {
    this->announceStimuliOnImplicitUpdates = announceStimuliOnImplicitUpdates;
}

void StimulusDisplay::setMainDisplayRefreshRate() {
    CVTime refreshPeriod = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(displayLinks.at(0));
    double refreshRate = 60.0;
    
    if (refreshPeriod.flags & kCVTimeIsIndefinite) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Could not determine main display refresh rate.  Assuming %g Hz.",
                 refreshRate);
    } else {
        refreshRate = double(refreshPeriod.timeScale) / double(refreshPeriod.timeValue);
    }
    
    mainDisplayRefreshRate = refreshRate;
}

void StimulusDisplay::addContext(int _context_id){
	context_ids.push_back(_context_id);
    int contextIndex = context_ids.size() - 1;
    
    CVDisplayLinkRef dl;
    
    if (kCVReturnSuccess != CVDisplayLinkCreateWithActiveCGDisplays(&dl)) {
        throw SimpleException("Unable to create display link");
    }
    
    displayLinks.push_back(dl);
    displayLinkContexts.emplace_back(new DisplayLinkContext(this, contextIndex));
    
    if (kCVReturnSuccess != CVDisplayLinkSetOutputCallback(dl,
                                                           &StimulusDisplay::displayLinkCallback,
                                                           displayLinkContexts.back().get()))
    {
        throw SimpleException("Unable to set display link output callback");
    }
    
    if (kCVReturnSuccess != opengl_context_manager->prepareDisplayLinkForContext(dl, _context_id)) {
        throw SimpleException("Unable to associate display link with OpenGL context");
    }
    
    auto ctxLock = opengl_context_manager->setCurrent(_context_id);
    prepareContext(contextIndex);
    
    if (0 == contextIndex) {
        setMainDisplayRefreshRate();
        allocateFramebufferStorage();
    }
}


namespace {
    const std::string vertexShaderSource
    (R"(
     in vec4 vertexPosition;
     in vec2 texCoords;
     
     out vec2 varyingTexCoords;
     
     void main() {
         gl_Position = vertexPosition;
         varyingTexCoords = texCoords;
     }
     )");
    
    
    const std::string fragmentShaderSource
    (R"(
     uniform sampler2D framebufferTexture;
     
     in vec2 varyingTexCoords;
     
     out vec4 fragColor;
     
     void main() {
         fragColor = texture(framebufferTexture, varyingTexCoords);
     }
     )");
    
    
    constexpr GLint numVertices = 4;
    constexpr GLint componentsPerVertex = 2;
    
    
    const std::array<GLfloat, numVertices*componentsPerVertex> vertexPositions
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };
    
    
    const std::array<GLfloat, numVertices*componentsPerVertex> texCoords
    {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };
}


void StimulusDisplay::prepareContext(int contextIndex) {
    auto vertexShader = gl::createShader(GL_VERTEX_SHADER, vertexShaderSource);
    auto fragmentShader = gl::createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    auto &program = programs[contextIndex];
    program = gl::createProgram({ vertexShader.get(), fragmentShader.get() }).release();
    gl::ProgramUsage programUsage(program);
    
    glUniform1i(glGetUniformLocation(program, "framebufferTexture"), 0);
    
    auto &vertexArray = vertexArrays[contextIndex];
    glGenVertexArrays(1, &vertexArray);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    GLuint vertexPositionBuffer = 0;
    glGenBuffers(1, &vertexPositionBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(vertexPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.data(), GL_STATIC_DRAW);
    GLint vertexPositionAttribLocation = glGetAttribLocation(program, "vertexPosition");
    glEnableVertexAttribArray(vertexPositionAttribLocation);
    glVertexAttribPointer(vertexPositionAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    GLuint texCoordsBuffer = 0;
    glGenBuffers(1, &texCoordsBuffer);
    arrayBufferBinding.bind(texCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STATIC_DRAW);
    GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordsAttribLocation);
    glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void StimulusDisplay::allocateFramebufferStorage() {
    glGenFramebuffers(1, &framebuffer);
    glGenTextures(1, &framebufferTexture);
    
    gl::FramebufferBinding<GL_DRAW_FRAMEBUFFER> framebufferBinding(framebuffer);
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(framebufferTexture);
    
    GLint viewportWidth, viewportHeight;
    getCurrentViewportSize(viewportWidth, viewportHeight);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 viewportWidth,
                 viewportHeight,
                 0,
                 GL_BGRA,
                 GL_UNSIGNED_INT_8_8_8_8_REV,
                 nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
    
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
        throw SimpleException("OpenGL framebuffer setup failed");
    }
}


void StimulusDisplay::drawStoredFramebuffer(int contextIndex) const {
    gl::ProgramUsage programUsage(programs.at(contextIndex));
    gl::VertexArrayBinding vertexArrayBinding(vertexArrays.at(contextIndex));
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(framebufferTexture);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void StimulusDisplay::getCurrentViewportSize(GLint &width, GLint &height) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];
}


void StimulusDisplay::stateSystemCallback(const Datum &data, MWorksTime time) {
    unique_lock lock(display_lock);
    
    int newState = data.getInteger();
    
    if ((IDLE == newState) && displayLinksRunning) {
        
        // If another thread is waiting for a display refresh, allow it to complete before stopping
        // the display link
        while (waitingForRefresh) {
            refreshCond.wait(lock);
        }
        
        displayLinksRunning = false;
        std::memset(&currentOutputTimeStamp, 0, sizeof(currentOutputTimeStamp));
        currentOutputTimeUS = -1;
        
        // We need to release the lock before calling CVDisplayLinkStop, because
        // StimulusDisplay::displayLinkCallback could be blocked waiting for the lock, and
        // CVDisplayLinkStop won't return until displayLinkCallback exits, leading to deadlock.
        lock.unlock();
        
        // NOTE: As of OS X 10.11, stopping the display links from a non-main thread causes issues
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (auto dl : displayLinks) {
                if (kCVReturnSuccess != CVDisplayLinkStop(dl)) {
                    merror(M_DISPLAY_MESSAGE_DOMAIN,
                           "Unable to stop updates on display %d",
                           CVDisplayLinkGetCurrentCGDisplay(dl));
                }
            }
        });
        
        mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates stopped");
        
    } else if ((RUNNING == newState) && !displayLinksRunning) {
        
        lastFrameTime = 0;
        
        // NOTE: As of OS X 10.11, starting the display links from a non-main thread causes issues
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (auto dl : displayLinks) {
                if (kCVReturnSuccess != CVDisplayLinkStart(dl)) {
                    merror(M_DISPLAY_MESSAGE_DOMAIN,
                           "Unable to start updates on display %d",
                           CVDisplayLinkGetCurrentCGDisplay(dl));
                }
            }
        });
        
        displayLinksRunning = true;
        
        // Wait for a refresh to complete, so we know that getCurrentOutputTimeUS() will return a valid time
        ensureRefresh(lock);
        
        mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates started (refresh rate: %g Hz)", getMainDisplayRefreshRate());
        
    }
}


CVReturn StimulusDisplay::displayLinkCallback(CVDisplayLinkRef _displayLink,
                                              const CVTimeStamp *now,
                                              const CVTimeStamp *outputTime,
                                              CVOptionFlags flagsIn,
                                              CVOptionFlags *flagsOut,
                                              void *_context)
{
    DisplayLinkContext &context = *static_cast<DisplayLinkContext *>(_context);
    StimulusDisplay &display = *(context.first);
    int contextIndex = context.second;
    
    if (contextIndex != 0) {
        
        display.refreshMirrorDisplay(contextIndex);
        
    } else {
        
        {
            unique_lock lock(display.display_lock);
            
            if (bool(warnOnSkippedRefresh->getValue())) {
                if (display.lastFrameTime) {
                    int64_t delta = (outputTime->videoTime - display.lastFrameTime) - outputTime->videoRefreshPeriod;
                    if (delta) {
                        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                                 "Skipped %g display refresh cycles",
                                 (double)delta / (double)(outputTime->videoRefreshPeriod));
                    }
                }
            }
            
            display.lastFrameTime = outputTime->videoTime;
            display.currentOutputTimeStamp = *outputTime;
            
            //
            // Here's how the time calculation works:
            //
            // outputTime->hostTime is the (estimated) time that the frame we're currently drawing will be displayed.
            // The value is in units of the "host time base", which appears to mean that it's directly comparable to
            // the value returned by mach_absolute_time().  However, the relationship to mach_absolute_time() is not
            // explicitly stated in the docs, so presumably we can't depend on it.
            //
            // What the CoreVideo docs *do* say is "the host time base for Core Video and the one for CoreAudio are
            // identical, and the values returned from either API can be used interchangeably".  Hence, we can use the
            // CoreAudio function AudioConvertHostTimeToNanos() to convert from the host time base to nanoseconds.
            //
            // Once we have a system time in nanoseconds, we substract the system base time and convert to
            // microseconds, which leaves us with a value that can be compared to clock->getCurrentTimeUS().
            //
            display.currentOutputTimeUS = (MWTime(AudioConvertHostTimeToNanos(outputTime->hostTime)) -
                                           display.clock->getSystemBaseTimeNS()) / 1000LL;
            
            display.refreshMainDisplay();
            display.waitingForRefresh = false;
        }
        
        // Signal waiting threads that refresh is complete
        display.refreshCond.notify_all();
        
    }
    
    return kCVReturnSuccess;
}


void StimulusDisplay::refreshMainDisplay() {
    //
    // Determine whether we need to draw
    //
    
    const bool updateIsExplicit = needDraw;
    
    if (!needDraw) {
        shared_ptr<StimulusNode> node = display_stack->getFrontmost();
        while (node) {
            if (node->isVisible()) {
                needDraw = node->needDraw(shared_from_this());
                if (needDraw)
                    break;
            }
            node = node->getNext();
        }
    }

    //
    // Draw stimuli
    //
    
    OpenGLContextLock ctxLock = opengl_context_manager->setCurrent(context_ids.at(0));
    
    if (needDraw) {
        gl::FramebufferBinding<GL_DRAW_FRAMEBUFFER> framebufferBinding(framebuffer);
        const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, &drawBuffer);
        
        current_context_index = 0;
        drawDisplayStack();
        current_context_index = -1;
    }
    
    drawStoredFramebuffer(0);
    opengl_context_manager->flush(0);
    
    if (needDraw) {
        announceDisplayUpdate(updateIsExplicit);
    }
    
    needDraw = false;
}


void StimulusDisplay::refreshMirrorDisplay(int contextIndex) const {
    OpenGLContextLock ctxLock = opengl_context_manager->setCurrent(context_ids[contextIndex]);
    
    drawStoredFramebuffer(contextIndex);
    opengl_context_manager->flush(contextIndex);
}


void StimulusDisplay::clearDisplay() {
    unique_lock lock(display_lock);
    
    shared_ptr<StimulusNode> node = display_stack->getFrontmost();
    while(node) {
        node->setVisible(false);
        node = node->getNext();
    }
	
    needDraw = true;
    ensureRefresh(lock);
}


void StimulusDisplay::glInit() {
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    
    glClearColor(backgroundRed, backgroundGreen, backgroundBlue, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}


void StimulusDisplay::drawDisplayStack() {
    // OpenGL setup

    glInit();
    
    // Draw all of the stimuli in the chain, back to front

    shared_ptr<StimulusNode> node = display_stack->getBackmost();
    while (node) {
        if (node->isVisible()) {
            if (!(node->isLoaded())) {
                merror(M_DISPLAY_MESSAGE_DOMAIN,
                       "Stimulus \"%s\" is not loaded and will not be displayed",
                       node->getStimulus()->getTag().c_str());
            } else {
                node->draw(shared_from_this());
                
                Datum individualAnnounce(node->getCurrentAnnounceDrawData());
                if (!individualAnnounce.isUndefined()) {
                    stimsToAnnounce.push_back(node);
                    stimAnnouncements.push_back(individualAnnounce);
                }
            }
        }
        node = node->getPrevious();
    }
}


MWTime StimulusDisplay::updateDisplay() {
    unique_lock lock(display_lock);
    
    shared_ptr<StimulusNode> node = display_stack->getFrontmost();
    while (node) {
        if (node->isPending()) {
            // we're taking care of the pending state, so
            // clear this flag
            node->clearPending();
            
            // convert "pending visible" stimuli to "visible" ones
            node->setVisible(node->isPendingVisible());
            
            if (node->isPendingRemoval()) {
                node->clearPendingRemoval();
                auto currentNode = node;
                node = currentNode->getNext();
                currentNode->remove();
                continue;
            }
        }
        
        node = node->getNext();
    }
    
#define ERROR_ON_LATE_FRAMES
#ifdef ERROR_ON_LATE_FRAMES
    MWTime before_draw = clock->getCurrentTimeUS();
#endif

    needDraw = true;
    ensureRefresh(lock);
    
#ifdef ERROR_ON_LATE_FRAMES
    MWTime now = clock->getCurrentTimeUS();
    MWTime slop = 2 * (MWTime)(1000000.0 / getMainDisplayRefreshRate());
    
    if(now-before_draw > slop) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "updating main window display is taking longer than two frames (%lld > %lld) to update", 
               now-before_draw, 
               slop);		
    }
#endif
    
    // Return the predicted output time of the just-submitted frame
    return currentOutputTimeUS;
}


void StimulusDisplay::ensureRefresh(unique_lock &lock) {
    if (!displayLinksRunning) {
        // Need to do the refresh here
        refreshMainDisplay();
        for (int i = 1; i < context_ids.size(); i++) {
            refreshMirrorDisplay(context_ids[i]);
        }
    } else {
        // Wait for next display refresh to complete
        waitingForRefresh = true;
        do {
            refreshCond.wait(lock);
        } while (waitingForRefresh);
    }
}


void StimulusDisplay::announceDisplayUpdate(bool updateIsExplicit) {
    MWTime now = getCurrentOutputTimeUS();
    if (-1 == now) {
        now = clock->getCurrentTimeUS();
    }
    
    stimDisplayUpdate->setValue(getAnnounceData(updateIsExplicit), now);
    
    if (announceIndividualStimuli && shouldAnnounceStimuli(updateIsExplicit)) {
        announceDisplayStack(now);
    }

    stimsToAnnounce.clear();
    stimAnnouncements.clear();
}


void StimulusDisplay::announceDisplayStack(MWTime time) {
    for (size_t i = 0; i < stimsToAnnounce.size(); i++) {
        stimsToAnnounce[i]->announce(stimAnnouncements[i], time);
    }
}


Datum StimulusDisplay::getAnnounceData(bool updateIsExplicit) {
    Datum stimAnnounce;
    
    if (!shouldAnnounceStimuli(updateIsExplicit)) {
        // No stim announcements, so just report the number of stimuli drawn
        stimAnnounce = Datum(long(stimAnnouncements.size()));
    } else {
        stimAnnounce = Datum(M_LIST, int(stimAnnouncements.size()));
        for (size_t i = 0; i < stimAnnouncements.size(); i++) {
            stimAnnounce.addElement(stimAnnouncements[i]);
        }
    }
    
	return stimAnnounce;
}


shared_ptr<StimulusDisplay> StimulusDisplay::getCurrentStimulusDisplay() {
    if (!GlobalCurrentExperiment) {
        throw SimpleException("no experiment currently defined");		
    }
    
    shared_ptr<StimulusDisplay> currentDisplay = GlobalCurrentExperiment->getStimulusDisplay();
    if (!currentDisplay) {
        throw SimpleException("no stimulus display in current experiment");
    }
    
    return currentDisplay;
}


END_NAMESPACE_MW





