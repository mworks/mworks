//
//  MetalStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#ifndef MetalStimulus_hpp
#define MetalStimulus_hpp

#include "AppleStimulusDisplay.hpp"
#include "ParameterValue.h"
#include "Stimulus.h"


BEGIN_NAMESPACE_MW


class MetalStimulus : public Stimulus {
    
public:
    explicit MetalStimulus(const ParameterValueMap &parameters);
    
    RenderingMode getRenderingMode() const override { return RenderingMode::Metal; }
    
    void load(boost::shared_ptr<StimulusDisplay> display) override;
    void unload(boost::shared_ptr<StimulusDisplay> display) override;
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    
protected:
    using MetalDisplay = AppleStimulusDisplay;
    
    static id<MTLLibrary> loadDefaultLibrary(MetalDisplay &display, NSBundle *bundle);
    static id<MTLFunction> loadShaderFunction(id<MTLLibrary> library, const std::string &name);
    static id<MTLFunction> loadShaderFunction(id<MTLLibrary> library,
                                              const std::string &name,
                                              MTLFunctionConstantValues *constantValues);
    static id<MTLRenderPipelineState> createRenderPipelineState(MetalDisplay &display,
                                                                MTLRenderPipelineDescriptor *renderPipelineDescriptor);
    
    static id<MTLRenderCommandEncoder> createRenderCommandEncoder(MetalDisplay &display);
    static id<MTLRenderCommandEncoder> createRenderCommandEncoder(MetalDisplay &display,
                                                                  MTLRenderPassDescriptor *renderPassDescriptor);
    
    template <typename T>
    static void setVertexBytes(id<MTLRenderCommandEncoder> renderCommandEncoder,
                               const T &bufferData,
                               NSUInteger bufferIndex);
    
    template <typename T>
    static void setFragmentBytes(id<MTLRenderCommandEncoder> renderCommandEncoder,
                                 const T &bufferData,
                                 NSUInteger bufferIndex);
    
    virtual void loadMetal(MetalDisplay &display) { }
    virtual void unloadMetal(MetalDisplay &display) { }
    virtual void drawMetal(MetalDisplay &display) { }
    
private:
    static MetalDisplay & getMetalDisplay(const boost::shared_ptr<StimulusDisplay> &display) {
        return *(boost::dynamic_pointer_cast<AppleStimulusDisplay>(display));
    }
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    
};


inline id<MTLRenderCommandEncoder> MetalStimulus::createRenderCommandEncoder(MetalDisplay &display) {
    return createRenderCommandEncoder(display, display.createMetalRenderPassDescriptor());
}


inline id<MTLRenderCommandEncoder>
MetalStimulus::createRenderCommandEncoder(MetalDisplay &display, MTLRenderPassDescriptor *renderPassDescriptor) {
    return [display.getCurrentMetalCommandBuffer() renderCommandEncoderWithDescriptor:renderPassDescriptor];
}


template <typename T>
inline void MetalStimulus::setVertexBytes(id<MTLRenderCommandEncoder> renderCommandEncoder,
                                          const T &bufferData,
                                          NSUInteger bufferIndex)
{
    static_assert(std::is_standard_layout_v<T> && !std::is_pointer_v<T>);
    static_assert(sizeof(bufferData) < 4096);  // setVertexBytes is for single-use data smaller than 4 KB
    [renderCommandEncoder setVertexBytes:&bufferData length:sizeof(bufferData) atIndex:bufferIndex];
}


template <typename T>
inline void MetalStimulus::setFragmentBytes(id<MTLRenderCommandEncoder> renderCommandEncoder,
                                            const T &bufferData,
                                            NSUInteger bufferIndex)
{
    static_assert(std::is_standard_layout_v<T> && !std::is_pointer_v<T>);
    static_assert(sizeof(bufferData) < 4096);  // setFragmentBytes is for single-use data smaller than 4 KB
    [renderCommandEncoder setFragmentBytes:&bufferData length:sizeof(bufferData) atIndex:bufferIndex];
}


END_NAMESPACE_MW


#endif /* MetalStimulus_hpp */
