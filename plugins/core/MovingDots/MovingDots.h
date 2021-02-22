/*
 *  MovingDots.h
 *  MovingDots
 *
 *  Created by Christopher Stawarz on 8/6/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#ifndef MovingDots_h
#define MovingDots_h


BEGIN_NAMESPACE_MW


using MovingDotsBase = DynamicStimulusBase<MetalStimulus>;


class MovingDots : public MovingDotsBase {
    
public:
    static const std::string FIELD_RADIUS;
    static const std::string FIELD_CENTER_X;
    static const std::string FIELD_CENTER_Y;
    static const std::string DOT_DENSITY;
    static const std::string DOT_SIZE;
    static const std::string COLOR;
    static const std::string ALPHA_MULTIPLIER;
    static const std::string DIRECTION;
    static const std::string SPEED;
    static const std::string COHERENCE;
    static const std::string LIFETIME;
    static const std::string RAND_SEED;
    static const std::string MAX_NUM_DOTS;
    static const std::string ANNOUNCE_DOTS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit MovingDots(const ParameterValueMap &parameters);
    ~MovingDots();
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    static constexpr std::size_t getDotPositionsSize(std::size_t numDots) {
        return numDots * sizeof(decltype(dotPositions)::element_type);
    }
    
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    void drawFrame(boost::shared_ptr<StimulusDisplay> display) override;
    
    void startPlaying() override;
    
    bool computeNumDots(double &newFieldRadius, double &newDotDensity, std::size_t &newNumDots) const;
    bool updateParameters();
    void updateDots();
    void advanceDot(std::size_t i, float dr);
    void replaceDot(std::size_t i, float direction, float age);
    
    float rand(float min, float max) {
        return randDist(randGen, decltype(randDist)::param_type(min, max));
    }
    
    float newDirection(float coherence) {
        if ((coherence == 0.0f) || ((coherence != 1.0f) && (rand(0.0f, 1.0f) > coherence))) {
            return rand(0.0f, 2.0f * M_PI);
        }
        return 0.0f;
    }
    
    float newAge(float lifetime) {
        if (lifetime != 0.0f) {
            return rand(0.0f, lifetime);
        }
        return 0.0f;
    }
    
    const VariablePtr fieldRadius;
    const VariablePtr fieldCenterX;
    const VariablePtr fieldCenterY;
    const VariablePtr dotDensity;
    const VariablePtr dotSize;
    VariablePtr red;
    VariablePtr green;
    VariablePtr blue;
    const VariablePtr alpha;
    const VariablePtr direction;
    const VariablePtr speed;
    const VariablePtr coherence;
    const VariablePtr lifetime;
    const VariablePtr randSeed;
    const VariablePtr maxNumDots;
    const VariablePtr announceDots;
    
    std::size_t currentMaxNumDots;
    std::unique_ptr<simd::float2[]> dotPositions;
    std::unique_ptr<float[]> dotDirections;
    std::unique_ptr<float[]> dotAges;
    
    MWTime currentRandSeed;
    std::mt19937 randGen;
    std::uniform_real_distribution<float> randDist;
    
    float dotSizeToPixels;
    
    id<MTLRenderPipelineState> renderPipelineState;
    MWKTripleBufferedMTLResource<id<MTLBuffer>> *bufferPool;
    
    MWTime currentTime;
    float currentFieldRadius, currentFieldCenterX, currentFieldCenterY;
    float currentDotDensity, currentDotSize;
    float currentRed, currentGreen, currentBlue, currentAlpha;
    float currentDirection, currentSpeed, currentCoherence, currentLifetime;
    std::size_t currentNumDots;
    
    MWTime previousTime;
    float previousFieldRadius;
    float previousSpeed, previousCoherence, previousLifetime;
    std::size_t previousNumDots;
    
};


END_NAMESPACE_MW


#endif /* MovingDots_h */
