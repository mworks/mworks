/*
 *  WhiteNoiseBackground.h
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/15/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#ifndef WhiteNoiseBackground_H_
#define WhiteNoiseBackground_H_


BEGIN_NAMESPACE_MW


class WhiteNoiseBackground : public MetalStimulus {

public:
    static const std::string GRAYSCALE;
    static const std::string GRAIN_SIZE;
    static const std::string RAND_SEED;
    static const std::string RANDOMIZE_ON_DRAW;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit WhiteNoiseBackground(const ParameterValueMap &parameters);
    ~WhiteNoiseBackground();
    
    Datum getCurrentAnnounceDrawData() override;
    
    void randomize() { shouldRandomize = true; }
    
private:
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    const VariablePtr grayscale;
    const VariablePtr grainSize;
    const VariablePtr randSeed;
    const VariablePtr randomizeOnDraw;
    
    bool currentGrayscale;
    double currentGrainSize;
    MWTime currentRandSeed;
    bool curentRandomizeOnDraw;
    
    std::size_t textureWidth;
    std::size_t textureHeight;
    
    id<MTLComputePipelineState> computePipelineState;
    MTLSize threadgroupsPerGrid;
    MTLSize threadsPerThreadgroup;
    id<MTLRenderPipelineState> renderPipelineState;
    
    NSArray<id<MTLTexture>> *seedTextures;
    NSArray<id<MTLTexture>> *noiseTextures;
    
    std::atomic_bool shouldRandomize;
    static_assert(decltype(shouldRandomize)::is_always_lock_free);
    std::size_t randCount;
    
};


END_NAMESPACE_MW


#endif /* WhiteNoiseBackground_H_ */
