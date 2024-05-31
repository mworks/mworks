//
//  OpticFlowField.hpp
//  OpticFlowField
//
//  Created by Christopher Stawarz on 5/31/23.
//

#ifndef OpticFlowField_hpp
#define OpticFlowField_hpp


BEGIN_NAMESPACE_MW


using OpticFlowFieldBase = DynamicStimulusBase<ColoredTransformStimulus>;


class OpticFlowField : public OpticFlowFieldBase {
    
public:
    static const std::string Z_NEAR;
    static const std::string Z_FAR;
    static const std::string DOT_DENSITY;
    static const std::string DOT_SIZE;
    static const std::string DIRECTION_ALTITUDE;
    static const std::string DIRECTION_AZIMUTH;
    static const std::string SPEED;
    static const std::string COHERENCE;
    static const std::string LIFETIME;
    static const std::string MAX_NUM_DOTS;
    static const std::string RAND_SEED;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit OpticFlowField(const ParameterValueMap &parameters);
    ~OpticFlowField();
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    struct Direction {
        float altitude;
        float azimuth;
    };
    
    static constexpr double altitudeMin = -90.0;
    static constexpr double altitudeMax = 90.0;
    static constexpr double azimuthMin = -180.0;
    static constexpr double azimuthMax = 180.0;
    
    static constexpr std::size_t getDotPositionsSize(std::size_t numDots) {
        return numDots * sizeof(decltype(dotPositions)::element_type);
    }
    
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    simd::float4x4 getCurrentMVPMatrix(const simd::float4x4 &projectionMatrix) const override;
    
    void startPlaying() override;
    
    bool computeNumDots(double newWidth, double newHeight, std::size_t &newNumDots) const;
    bool computeNumDots(double newWidth,
                        double newHeight,
                        double &newZNear,
                        double &newZFar,
                        double &newFieldRadius,
                        simd::float4x4 &newPerspectiveMatrix,
                        double &newDotDensity,
                        std::size_t &newNumDots) const;
    bool updateParameters();
    void updateDots();
    void updateDot(std::size_t i, float dt, float dr);
    void replaceDot(std::size_t i, float age);
    
    float rand(float min, float max) {
        return randDist(randGen, decltype(randDist)::param_type(min, max));
    }
    
    Direction newDirection() {
        if ((currentCoherence == 0.0f) || ((currentCoherence != 1.0f) && (rand(0.0f, 1.0f) > currentCoherence))) {
            return { rand(altitudeMin, altitudeMax), rand(azimuthMin, azimuthMax) };
        }
        return { 0.0f, 0.0f };
    }
    
    float newAge() {
        if (currentLifetime != 0.0f) {
            return rand(0.0f, currentLifetime);
        }
        return 0.0f;
    }
    
    const VariablePtr zNear;
    const VariablePtr zFar;
    const VariablePtr dotDensity;
    const VariablePtr dotSize;
    const VariablePtr directionAltitude;
    const VariablePtr directionAzimuth;
    const VariablePtr speed;
    const VariablePtr coherence;
    const VariablePtr lifetime;
    const VariablePtr maxNumDots;
    const VariablePtr randSeed;
    
    float displayWidth, displayHeight;
    
    std::size_t currentMaxNumDots;
    std::unique_ptr<simd::float4[]> dotPositions;
    std::unique_ptr<Direction[]> dotDirections;
    std::unique_ptr<float[]> dotAges;
    
    MWTime currentRandSeed;
    std::mt19937 randGen;
    std::uniform_real_distribution<float> randDist;
    
    MWKTripleBufferedMTLResource<id<MTLBuffer>> *bufferPool;
    
    MWTime currentTime;
    float currentZNear, currentZFar;
    float currentDotDensity, currentDotSize;
    float currentDirectionAltitude, currentDirectionAzimuth;
    float currentSpeed, currentCoherence, currentLifetime;
    float currentFieldRadius;
    simd::float4x4 currentPerspectiveMatrix;
    std::size_t currentNumDots;
    
    MWTime previousTime;
    float previousSpeed, previousCoherence, previousLifetime;
    float previousFieldRadius;
    std::size_t previousNumDots;
    
};


END_NAMESPACE_MW


#endif /* OpticFlowField_hpp */
