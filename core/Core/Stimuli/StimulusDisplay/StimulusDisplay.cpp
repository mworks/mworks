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
#include "ComponentRegistry.h"
#include "VariableNotification.h"


// A hack for now
#define M_STIMULUS_DISPLAY_LEFT_EDGE		-26.57
#define M_STIMULUS_DISPLAY_RIGHT_EDGE		26.57
#define M_STIMULUS_DISPLAY_BOTTOM_EDGE		-17.77
#define M_STIMULUS_DISPLAY_TOP_EDGE			17.77


BEGIN_NAMESPACE_MW


StimulusDisplay::StimulusDisplay(bool announceIndividualStimuli, bool useColorManagement) :
    current_context_index(-1),
    waitingForRefresh(false),
    needDraw(false),
    redrawOnEveryRefresh(false),
    projectionMatrix(GLKMatrix4Identity),
    displayUpdatesStarted(false),
    mainDisplayRefreshRate(0.0),
    currentOutputTimeUS(-1),
    announceIndividualStimuli(announceIndividualStimuli),
    announceStimuliOnImplicitUpdates(true),
    useColorManagement(useColorManagement)
{
    // defer creation of the display chain until after the stimulus display has been created
    display_stack = shared_ptr< LinkedList<StimulusNode> >(new LinkedList<StimulusNode>());
    
	setDisplayBounds();
    setBackgroundColor(0.5, 0.5, 0.5, 1.0);

    opengl_context_manager = OpenGLContextManager::instance();
    clock = Clock::instance();
    
    auto callback = [this](const Datum &data, MWorksTime time) {
        stateSystemCallback(data, time);
    };
    stateSystemNotification = boost::make_shared<VariableCallbackNotification>(callback);
    state_system_mode->addNotification(stateSystemNotification);
}

StimulusDisplay::~StimulusDisplay(){
    stateSystemNotification->remove();
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
                                       double &left,
                                       double &right,
                                       double &bottom,
                                       double &top)
{
	if(display_info.getDataType() == M_DICTIONARY &&
	   display_info.hasKey(M_DISPLAY_WIDTH_KEY) &&
	   display_info.hasKey(M_DISPLAY_HEIGHT_KEY) &&
	   display_info.hasKey(M_DISPLAY_DISTANCE_KEY)){
	
		double width_unknown_units = display_info.getElement(M_DISPLAY_WIDTH_KEY);
		double height_unknown_units = display_info.getElement(M_DISPLAY_HEIGHT_KEY);
		double distance_unknown_units = display_info.getElement(M_DISPLAY_DISTANCE_KEY);
	
		double half_width_deg = (180. / M_PI) * atan((width_unknown_units/2.)/distance_unknown_units);
		double half_height_deg = half_width_deg * height_unknown_units / width_unknown_units;
		//double half_height_deg = (180. / M_PI) * atan((height_unknown_units/2.)/distance_unknown_units);
		
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

void StimulusDisplay::getDisplayBounds(double &left, double &right, double &bottom, double &top) {
    left = this->left;
    right = this->right;
    bottom = this->bottom;
    top = this->top;
}

void StimulusDisplay::setBackgroundColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    backgroundRed = red;
    backgroundGreen = green;
    backgroundBlue = blue;
    backgroundAlpha = alpha;
}

void StimulusDisplay::setRedrawOnEveryRefresh(bool redrawOnEveryRefresh) {
    this->redrawOnEveryRefresh = redrawOnEveryRefresh;
}

void StimulusDisplay::setAnnounceStimuliOnImplicitUpdates(bool announceStimuliOnImplicitUpdates) {
    this->announceStimuliOnImplicitUpdates = announceStimuliOnImplicitUpdates;
}

void StimulusDisplay::addContext(int _context_id){
	context_ids.push_back(_context_id);
    int contextIndex = context_ids.size() - 1;
    
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
    
    
    const std::string basicFragmentShaderSource
    (R"(
     uniform sampler2D framebufferTexture;
     
     in vec2 varyingTexCoords;
     
     out vec4 fragColor;
     
     void main() {
         fragColor = texture(framebufferTexture, varyingTexCoords);
     }
     )");
    
    
    //
    // The technique we use for color conversion is adapted from Chapter 24, "Using Lookup Tables to Accelerate
    // Color Transformations", in "GPU Gems 2":
    //
    // https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter24.html
    //
    
    const std::string colorManagedFragmentShaderSource
    (R"(
     uniform sampler2D framebufferTexture;
     
     uniform highp sampler3D colorLUT;
     uniform float colorScale;
     uniform float colorOffset;
     uniform bool shouldConvertColors;
     
     in vec2 varyingTexCoords;
     
     out vec4 fragColor;
     
     vec3 linearToSRGB(vec3 linear) {
         vec3 srgb = mix(12.92 * linear,
                         1.055 * pow(linear, vec3(0.41666)) - 0.055,
                         vec3(greaterThanEqual(linear, vec3(0.0031308))));
         // To fully reverse the sRGB-to-linear conversion performed by the GPU, we must first map the
         // floating-point sRGB color components we just computed back to integers in the range [0, 255],
         // and then re-normalize them to [0.0, 1.0].  Counterintuitively, this rounding process
         // results in better-looking output, with less visible banding in grayscale gradients, than using
         // non-rounded values.  Also, it matches the output we get when we turn off the sRGB-to-linear
         // conversion using the GL_EXT_texture_sRGB_decode extension.
         return round(srgb * 255.0) / 255.0;
     }
     
     void main() {
         vec4 rawColor = texture(framebufferTexture, varyingTexCoords);
         if (shouldConvertColors) {
             fragColor.rgb = texture(colorLUT, linearToSRGB(rawColor.rgb) * colorScale + colorOffset).rgb;
         } else {
             fragColor.rgb = linearToSRGB(rawColor.rgb);
         }
         fragColor.a = rawColor.a;
     }
     )");
    
    
    constexpr GLint numVertices = 4;
    constexpr GLint componentsPerVertex = 2;
    constexpr GLint numGridPoints = 32;
    
    
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
    auto fragmentShader = gl::createShader(GL_FRAGMENT_SHADER, (useColorManagement ?
                                                                colorManagedFragmentShaderSource :
                                                                basicFragmentShaderSource));
    
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
    
    if (useColorManagement) {
        glUniform1i(glGetUniformLocation(program, "colorLUT"), 1);
        glUniform1f(glGetUniformLocation(program, "colorScale"), (GLfloat(numGridPoints) - 1.0f) / GLfloat(numGridPoints));
        glUniform1f(glGetUniformLocation(program, "colorOffset"), 1.0f / (2.0f * GLfloat(numGridPoints)));
        glUniform1i(glGetUniformLocation(program, "shouldConvertColors"), createColorConversionLUT(contextIndex));
    }
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
                 (useColorManagement ? GL_SRGB8_ALPHA8 : GL_RGBA8),
                 viewportWidth,
                 viewportHeight,
                 0,
#if MWORKS_OPENGL_ES
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
#else
                 GL_BGRA,
                 GL_UNSIGNED_INT_8_8_8_8_REV,
#endif
                 nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
    
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
        throw SimpleException("OpenGL framebuffer setup failed");
    }
}


bool StimulusDisplay::createColorConversionLUT(int contextIndex) {
    auto lutData = opengl_context_manager->getColorConversionLUTData(context_ids.at(contextIndex), numGridPoints);
    auto &colorConversionLUT = colorConversionLUTs[contextIndex];

    if (lutData.empty()) {
        // No LUT data means no conversion is required
        colorConversionLUT = 0;
        return false;
    }
    
    if (lutData.size() != (numGridPoints * numGridPoints * numGridPoints * 3)) {
        throw SimpleException("Wrong number of elements in color conversion lookup table data");
    }
    
    glGenTextures(1, &colorConversionLUT);
    gl::TextureBinding<GL_TEXTURE_3D> colorConversionLUTBinding(colorConversionLUT);
    
    gl::resetPixelStorageUnpackParameters(alignof(decltype(lutData)::value_type));
    
    glTexImage3D(GL_TEXTURE_3D,
                 0,
                 GL_RGB16F,
                 numGridPoints,
                 numGridPoints,
                 numGridPoints,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 lutData.data());
    
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return true;
}


void StimulusDisplay::drawStoredFramebuffer(int contextIndex) const {
    gl::ProgramUsage programUsage(programs.at(contextIndex));
    gl::VertexArrayBinding vertexArrayBinding(vertexArrays.at(contextIndex));
    
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    if (useColorManagement) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, colorConversionLUTs.at(contextIndex));
    }
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
    
    if (useColorManagement) {
        glBindTexture(GL_TEXTURE_3D, 0);
        glActiveTexture(GL_TEXTURE0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}


void StimulusDisplay::getCurrentViewportSize(GLint &width, GLint &height) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];
}


void StimulusDisplay::pushFramebuffer(GLuint framebuffer, const std::vector<GLenum> &drawBuffers) {
    framebufferStack.emplace_back(framebuffer, drawBuffers);
    bindCurrentFramebuffer();
}


void StimulusDisplay::popFramebuffer() {
    if (framebufferStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: No framebuffer to pop");
        return;
    }
    framebufferStack.pop_back();
    bindCurrentFramebuffer();
}


void StimulusDisplay::bindCurrentFramebuffer() {
    if (framebufferStack.empty()) {
        opengl_context_manager->bindDefaultFramebuffer(context_ids.at(0));
        return;
    }
    auto &framebufferInfo = framebufferStack.back();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferInfo.first);
    glDrawBuffers(framebufferInfo.second.size(), framebufferInfo.second.data());
}


void StimulusDisplay::stateSystemCallback(const Datum &data, MWorksTime time) {
    unique_lock lock(display_lock);
    
    const int newStateSystemMode = data.getInteger();
    
    if ((RUNNING == newStateSystemMode) && !displayUpdatesStarted) {
        
        displayUpdatesStarted = true;  // Need to set this *before* calling startDisplayUpdates
        startDisplayUpdates();
        
        // Wait for a refresh to complete, so subclass methods that report the current
        // output time will return a valid value
        ensureRefresh(lock);
        
        mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates started (refresh rate: %g Hz)", getMainDisplayRefreshRate());
    
    } else if ((IDLE == newStateSystemMode) && displayUpdatesStarted) {
        
        // If another thread is waiting for a display refresh, allow it to complete before stopping
        // the display link
        while (waitingForRefresh) {
            refreshCond.wait(lock);
        }
        
        displayUpdatesStarted = false;  // Need to clear this *before* calling stopDisplayUpdates
        
        // We need to release the lock before calling stopDisplayUpdates, because
        // a display update callback could be blocked waiting for the lock, and
        // attempting to stop a blocked update loop could lead to deadlock
        lock.unlock();
        
        stopDisplayUpdates();
        currentOutputTimeUS = -1;
        
        mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates stopped");
        
    }
}


void StimulusDisplay::refreshMainDisplay() {
    //
    // Determine whether we need to draw
    //
    
    const bool updateIsExplicit = needDraw;
    needDraw = needDraw || redrawOnEveryRefresh;
    
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
    
    constexpr int contextIndex = 0;
    const int context_id = context_ids.at(contextIndex);
    OpenGLContextLock ctxLock = opengl_context_manager->setCurrent(context_id);
    
    if (needDraw) {
        pushFramebuffer(framebuffer);
        
        current_context_index = contextIndex;
        drawDisplayStack();
        current_context_index = -1;
        
        popFramebuffer();
    }
    
    drawStoredFramebuffer(contextIndex);
    opengl_context_manager->flush(context_id);
    
    if (needDraw) {
        announceDisplayUpdate(updateIsExplicit);
    }
    
    needDraw = false;
}


void StimulusDisplay::refreshMirrorDisplay(int contextIndex) const {
    OpenGLContextLock ctxLock = opengl_context_manager->setCurrent(context_ids.at(contextIndex));
    
    drawStoredFramebuffer(contextIndex);
    opengl_context_manager->flush(contextIndex);
}


void StimulusDisplay::clearDisplay() {
    unique_lock lock(display_lock);
    
    // Remove all stimuli from the display stack
    while (auto node = display_stack->getFrontmost()) {
        node->remove();
    }
	
    needDraw = true;
    ensureRefresh(lock);
}


void StimulusDisplay::drawDisplayStack() {
#if !MWORKS_OPENGL_ES
    // This has no effect on non-sRGB framebuffers, so we can enable it unconditionally
    gl::Enabled<GL_FRAMEBUFFER_SRGB> framebufferSRGBEnabled;
#endif
    
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    
    glClearColor(backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //
    // Draw all of the stimuli in the chain, back to front
    //
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

    needDraw = true;
    ensureRefresh(lock);
    
    // Return the predicted output time of the just-submitted frame
    return currentOutputTimeUS;
}


void StimulusDisplay::ensureRefresh(unique_lock &lock) {
    if (!displayUpdatesStarted) {
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


void StimulusDisplay::reportSkippedFrames(double numSkippedFrames) const {
    if (warnOnSkippedRefresh->getValue().getBool()) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN, "Skipped %g display refresh cycles", numSkippedFrames);
    }
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



























