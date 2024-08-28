//
//  LayerStimulus.cpp
//  LayerStimulus
//
//  Created by Christopher Stawarz on 6/19/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#include "LayerStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string LayerStimulus::MAX_SIZE_X("max_size_x");
const std::string LayerStimulus::MAX_SIZE_Y("max_size_y");


void LayerStimulus::describeComponent(ComponentInfo &info) {
    AlphaBlendedTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/layer");
    
    info.addParameter(FULLSCREEN, "YES");  // Change fullscreen's default to YES
    info.addParameter(MAX_SIZE_X, false);
    info.addParameter(MAX_SIZE_Y, false);
}


LayerStimulus::LayerStimulus(const ParameterValueMap &parameters) :
    AlphaBlendedTransformStimulus(parameters),
    maxSizeX(optionalVariable(parameters[MAX_SIZE_X])),
    maxSizeY(optionalVariable(parameters[MAX_SIZE_Y])),
    framebufferID(-1),
    framebufferTexture(nil)
{ }


LayerStimulus::~LayerStimulus() {
    @autoreleasepool {
        framebufferTexture = nil;
    }
}


void LayerStimulus::addChild(std::map<std::string, std::string> parameters,
                             ComponentRegistryPtr reg,
                             boost::shared_ptr<Component> child)
{
    auto stim = boost::dynamic_pointer_cast<Stimulus>(child);
    if (!stim) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Child component must be a stimulus");
    }
    if (getDisplay() != stim->getDisplay()) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Child stimulus must use the same display as its parent");
    }
    children.push_back(stim);
}


void LayerStimulus::freeze(bool shouldFreeze) {
    for (auto &child : children) {
        child->freeze(shouldFreeze);
    }
    AlphaBlendedTransformStimulus::freeze(shouldFreeze);
}


void LayerStimulus::load(boost::shared_ptr<StimulusDisplay> display) {
    // Always load children, even if our loaded flag is true, as they may have been
    // unloaded independently
    for (auto &child : children) {
        child->load(display);
    }
    
    AlphaBlendedTransformStimulus::load(display);
}


void LayerStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    // Always unload children, even if our loaded flag is false, as they may have been
    // loaded independently
    for (auto &child : children) {
        child->unload(display);
    }
    
    AlphaBlendedTransformStimulus::unload(display);
}


void LayerStimulus::setVisible(bool newVisible) {
    for (auto &child : children) {
        child->setVisible(newVisible);
    }
    AlphaBlendedTransformStimulus::setVisible(newVisible);
}


bool LayerStimulus::needDraw(boost::shared_ptr<StimulusDisplay> display) {
    for (auto &child : children) {
        if (child->needDraw(display)) {
            return true;
        }
    }
    return false;
}


Datum LayerStimulus::getCurrentAnnounceDrawData() {
    auto announceData = AlphaBlendedTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "layer");
    
    std::vector<Datum> childAnnounceData;
    for (auto &child : children) {
        if (child->isLoaded()) {
            childAnnounceData.emplace_back(child->getCurrentAnnounceDrawData());
        }
    }
    announceData.addElement("children", Datum(std::move(childAnnounceData)));
    
    return announceData;
}


void LayerStimulus::loadMetal(MetalDisplay &display) {
    AlphaBlendedTransformStimulus::loadMetal(display);
    
    //
    // Determine maximum size
    //
    
    if (!fullscreen) {
        float defaultMaxSizeX = 0.0;
        float defaultMaxSizeY = 0.0;
        
        // Don't evaluate x_size and y_size unless we need them
        if (!(maxSizeX && maxSizeY)) {
            getCurrentSize(defaultMaxSizeX, defaultMaxSizeY);
        }
        
        if (maxSizeX) {
            currentMaxSizeX = maxSizeX->getValue().getFloat();
        } else {
            currentMaxSizeX = defaultMaxSizeX;
        }
        if (currentMaxSizeX <= 0.0) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Maximum horizontal size must be greater than zero");
        }
        
        if (maxSizeY) {
            currentMaxSizeY = maxSizeY->getValue().getFloat();
        } else {
            currentMaxSizeY = defaultMaxSizeY;
        }
        if (currentMaxSizeY <= 0.0) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Maximum vertical size must be greater than zero");
        }
    }
    
    //
    // Create framebuffer texture
    //
    {
        std::size_t framebufferWidth, framebufferHeight;
        display.getCurrentTextureSizeForDisplayArea(fullscreen, currentMaxSizeX, currentMaxSizeY, framebufferWidth, framebufferHeight);
        framebufferID = display.createFramebuffer(framebufferWidth, framebufferHeight);
        framebufferTexture = display.getMetalFramebufferTexture(framebufferID);
    }
    
    //
    // Create render pipeline state
    //
    {
        auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
        auto vertexFunction = loadShaderFunction(library, "LayerStimulus_vertexShader");
        auto fragmentFunction = loadShaderFunction(library, "LayerStimulus_fragmentShader");
        auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
        renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
    }
}


void LayerStimulus::unloadMetal(MetalDisplay &display) {
    framebufferTexture = nil;
    display.releaseFramebuffer(framebufferID);
    framebufferID = -1;
    
    AlphaBlendedTransformStimulus::unloadMetal(display);
}


void LayerStimulus::drawMetal(MetalDisplay &display) {
    AlphaBlendedTransformStimulus::drawMetal(display);
    
    //
    // Determine current size in pixels
    //
    
    if (!fullscreen) {
        if (current_sizex > currentMaxSizeX) {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Current horizontal size of stimulus \"%s\" (%g) exceeds maximum (%g).  To resolve this issue, "
                   "set %s to an appropriate value.",
                   getTag().c_str(),
                   current_sizex,
                   currentMaxSizeX,
                   MAX_SIZE_X.c_str());
            return;
        }
        if (current_sizey > currentMaxSizeY) {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Current vertical size of stimulus \"%s\" (%g) exceeds maximum (%g).  To resolve this issue, "
                   "set %s to an appropriate value.",
                   getTag().c_str(),
                   current_sizey,
                   currentMaxSizeY,
                   MAX_SIZE_Y.c_str());
            return;
        }
    }
    
    std::size_t currentWidthPixels, currentHeightPixels;
    display.getCurrentTextureSizeForDisplayArea(fullscreen, current_sizex, current_sizey, currentWidthPixels, currentHeightPixels);
    
    //
    // Render children to framebuffer texture
    //
    
    display.pushFramebuffer(framebufferID);
    display.pushMetalViewport(0, 0, currentWidthPixels, currentHeightPixels);
    
    if (!fullscreen) {
        const auto halfWidthDegrees = current_sizex / 2.0;
        const auto halfHeightDegrees = current_sizey / 2.0;
        display.pushMetalProjectionMatrix(-halfWidthDegrees, halfWidthDegrees, -halfHeightDegrees, halfHeightDegrees);
    }
    
    // Make background transparent
    {
        auto renderPassDescriptor = display.createMetalRenderPassDescriptor(MTLLoadActionClear);
        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 0.0);
        auto renderCommandEncoder = createRenderCommandEncoder(display, renderPassDescriptor);
        [renderCommandEncoder endEncoding];
    }
    
    {
        auto sharedDisplay = display.shared_from_this();
        for (auto &child : children) {
            if (child->isLoaded()) {
                child->draw(sharedDisplay);
            } else {
                merror(M_DISPLAY_MESSAGE_DOMAIN,
                       "Stimulus \"%s\" (child of stimulus \"%s\") is not loaded and will not be displayed",
                       child->getTag().c_str(),
                       getTag().c_str());
            }
        }
    }
    
    if (display.getUseAntialiasing()) {
        // Resolve the multisample framebuffer texture to a single-sample texture for display
        auto renderPassDescriptor = display.createMetalRenderPassDescriptor(MTLLoadActionLoad,
                                                                            MTLStoreActionMultisampleResolve);
        renderPassDescriptor.colorAttachments[0].resolveTexture = display.getMetalMultisampleResolveTexture();
        auto renderCommandEncoder = createRenderCommandEncoder(display, renderPassDescriptor);
        [renderCommandEncoder endEncoding];
    }
    
    if (!fullscreen) {
        display.popMetalProjectionMatrix();
    }
    
    display.popMetalViewport();
    display.popFramebuffer();
    
    //
    // Render framebuffer texture to display
    //
    {
        auto renderCommandEncoder = createRenderCommandEncoder(display);
        [renderCommandEncoder setRenderPipelineState:renderPipelineState];
        
        setCurrentMVPMatrix(display, renderCommandEncoder, 0);
        {
            auto texCoordScale = simd::make_float2(currentWidthPixels, currentHeightPixels);
            setVertexBytes(renderCommandEncoder, texCoordScale, 1);
        }
        
        {
            auto fragmentTexture = (display.getUseAntialiasing() ? display.getMetalMultisampleResolveTexture() : framebufferTexture);
            [renderCommandEncoder setFragmentTexture:fragmentTexture atIndex:0];
        }
        {
            auto currentAlpha = float(current_alpha);
            setFragmentBytes(renderCommandEncoder, currentAlpha, 0);
        }
        
        [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
        [renderCommandEncoder endEncoding];
    }
}


END_NAMESPACE_MW
