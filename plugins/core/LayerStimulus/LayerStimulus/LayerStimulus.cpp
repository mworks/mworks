//
//  LayerStimulus.cpp
//  LayerStimulus
//
//  Created by Christopher Stawarz on 6/19/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#include "LayerStimulus.hpp"


BEGIN_NAMESPACE_MW


void LayerStimulus::describeComponent(ComponentInfo &info) {
    MetalStimulus::describeComponent(info);
    info.setSignature("stimulus/layer");
}


LayerStimulus::LayerStimulus(const ParameterValueMap &parameters) :
    MetalStimulus(parameters),
    renderPipelineState(nil),
    framebufferID(-1),
    framebufferTexture(nil)
{ }


LayerStimulus::~LayerStimulus() {
    @autoreleasepool {
        framebufferTexture = nil;
        renderPipelineState = nil;
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
    children.push_back(stim);
}


void LayerStimulus::freeze(bool shouldFreeze) {
    for (auto &child : children) {
        child->freeze(shouldFreeze);
    }
    MetalStimulus::freeze(shouldFreeze);
}


void LayerStimulus::load(boost::shared_ptr<StimulusDisplay> display) {
    // Always load children, even if our loaded flag is true, as they may have been
    // unloaded independently
    for (auto &child : children) {
        child->load(display);
    }
    
    MetalStimulus::load(display);
}


void LayerStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    // Always unload children, even if our loaded flag is false, as they may have been
    // loaded independently
    for (auto &child : children) {
        child->unload(display);
    }
    
    MetalStimulus::unload(display);
}


void LayerStimulus::setVisible(bool newVisible) {
    for (auto &child : children) {
        child->setVisible(newVisible);
    }
    MetalStimulus::setVisible(newVisible);
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
    auto announceData = MetalStimulus::getCurrentAnnounceDrawData();
    
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
    auto renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    
    auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
    renderPipelineDescriptor.vertexFunction = loadShaderFunction(library, "LayerStimulus_vertexShader");
    renderPipelineDescriptor.fragmentFunction = loadShaderFunction(library, "LayerStimulus_fragmentShader");
    
    auto colorAttachment = renderPipelineDescriptor.colorAttachments[0];
    colorAttachment.pixelFormat = display.getMetalFramebufferTexturePixelFormat();
    colorAttachment.blendingEnabled = YES;
    colorAttachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    
    renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
    
    {
        auto ctxLock = display.setCurrent();
        framebufferID = display.createFramebuffer();
    }
    framebufferTexture = display.getMetalFramebufferTexture(framebufferID);
}


void LayerStimulus::unloadMetal(MetalDisplay &display) {
    framebufferTexture = nil;
    {
        auto ctxLock = display.setCurrent();
        display.releaseFramebuffer(framebufferID);
        framebufferID = -1;
    }
    renderPipelineState = nil;
}


void LayerStimulus::drawMetal(MetalDisplay &display) {
    //
    // Render children to framebuffer texture
    //
    
    display.pushFramebuffer(framebufferID);
    
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
                display.setRenderingMode(child->getRenderingMode());
                child->draw(sharedDisplay);
            } else {
                merror(M_DISPLAY_MESSAGE_DOMAIN,
                       "Stimulus \"%s\" (child of stimulus \"%s\") is not loaded and will not be displayed",
                       child->getTag().c_str(),
                       getTag().c_str());
            }
        }
    }
    
    display.popFramebuffer();
    
    //
    // Render framebuffer texture to display
    //
    
    display.setRenderingMode(RenderingMode::Metal);
    
    {
        auto renderCommandEncoder = createRenderCommandEncoder(display);
        
        [renderCommandEncoder setRenderPipelineState:renderPipelineState];
        [renderCommandEncoder setFragmentTexture:framebufferTexture atIndex:0];
        
        [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
        [renderCommandEncoder endEncoding];
    }
}


END_NAMESPACE_MW
