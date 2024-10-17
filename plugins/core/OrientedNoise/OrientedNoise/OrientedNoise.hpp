//
//  OrientedNoise.hpp
//  OrientedNoise
//
//  Created by Christopher Stawarz on 10/30/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#ifndef OrientedNoise_hpp
#define OrientedNoise_hpp


BEGIN_NAMESPACE_MW


using OrientedNoiseBase = DynamicStimulusBase<ColoredTransformStimulus>;


class OrientedNoise : public OrientedNoiseBase {
    
public:
    static const std::string F_0;
    static const std::string SIGMA_F;
    static const std::string THETA_0;
    static const std::string SIGMA_THETA;
    static const std::string DIRECTORY_PATH;
    static const std::string NOISE_IMAGE_SELECTION;
    static const std::string RAND_SEED;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit OrientedNoise(const ParameterValueMap &parameters);
    ~OrientedNoise();
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    enum class NoiseImageSelection {
        Sequential,
        RandomWithReplacement,
        RandomWithoutReplacement
    };
    
    static NoiseImageSelection noiseImageSelectionFromName(const std::string &name);
    static void selectImageFileIndices(std::deque<std::size_t> &imageFileIndices,
                                       std::size_t numImages,
                                       NoiseImageSelection selectionType,
                                       int previousIndex,
                                       std::mt19937 &auxRandGen);
    
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    vDSP_Length pixelsToLog2Pixels(std::size_t pixels) {
        // Round down, so we don't compute more pixels than we can display, and
        // ensure that we use at least the minimum number of pixels
        return std::max(double(minLog2RowsCols), std::floor(std::log2(double(pixels))));
    }
    
    void loadImageFile(const std::string &filePath);
    void computeFFTMask();
    void filterNoise();
    
    const boost::shared_ptr<Variable> f0;
    const boost::shared_ptr<Variable> sigmaF;
    const boost::shared_ptr<Variable> theta0;
    const boost::shared_ptr<Variable> sigmaTheta;
    const boost::shared_ptr<Variable> directoryPath;
    const boost::shared_ptr<Variable> noiseImageSelection;
    const boost::shared_ptr<Variable> randSeed;
    
    MWTime currentRandSeed;
    std::mt19937 auxRandGen;
    
    std::string currentDirectoryPath;
    std::string currentNoiseImageSelectionName;
    NoiseImageSelection currentNoiseImageSelection;
    
    std::vector<std::string> imageFilePaths;
    std::vector<std::string> imageFileHashes;
    std::vector<std::vector<float>> imageFileData;
    std::deque<std::size_t> imageFileIndices;
    int currentImageFileIndex;
    
    // Require at least 4 pixels in each dimension, so that we can divide the
    // noise texture into chunks of 16 for use with simd_minstd_rand
    static constexpr vDSP_Length minLog2RowsCols = 2;
    vDSP_Length currentLog2Rows, currentLog2Cols;
    
    class simd_minstd_rand {
    public:
        static constexpr std::size_t vector_width = 16;
        
        using scalar_type = std::uint32_t;
        using vector_type = simd::uint16;
        using packed_vector_type = simd::packed::uint16;
        static_assert(sizeof(vector_type) == vector_width * sizeof(scalar_type),
                      "Vector size, vector width, and scalar size are not compatible");
        
        using float_scalar_type = float;
        using float_vector_type = simd::float16;
        using packed_float_vector_type = simd::packed::float16;
        static_assert(sizeof(float_vector_type) == vector_width * sizeof(float_scalar_type),
                      "Float vector size, vector width, and float scalar size are not compatible");
        
        static constexpr scalar_type min() { return c == 0u; }
        static constexpr scalar_type max() { return m - 1u; }
        
        constexpr simd_minstd_rand() :
            // This will make all vector elements identical.  Must call seed() before using!
            state(min())
        { }
        
        void seed(vector_type s) {
            state = s + min() * vector_type(s == 0u);  // Replace zeros with minimum value
        }
        
        vector_type operator()() {
            auto result = (a * state + c) % m;
            state = result;
            return result;
        }
        
        static float_vector_type asNormalizedFloat(vector_type value) {
            return ((simd_float(value) - float_scalar_type(min())) /
                    (float_scalar_type(max()) - float_scalar_type(min())));
        }
        
    private:
        static constexpr scalar_type a = 48271u;
        static constexpr scalar_type c = 0u;
        static constexpr scalar_type m = 2147483647u;
        
        // Instances of this class will be allocated on the heap, so we need to store the state
        // in packed form to ensure that we don't exceed the alignment guarantees of std::malloc
        packed_vector_type state;
    };
    
    using randGenType = simd_minstd_rand;
    randGenType randGen;
    
    struct FFTSetupDeleter {
        void operator()(FFTSetup fftSetup) {
            vDSP_destroy_fftsetup(fftSetup);
        }
    };
    std::unique_ptr<std::remove_pointer<FFTSetup>::type, FFTSetupDeleter> fftSetup;
    
    std::size_t currentRows, currentCols;
    std::vector<float> noiseTexture, imaginaryComponents, fftMask, temp;
    MWKTripleBufferedMTLResource<id<MTLTexture>> *texturePool;
    id<MTLTexture> currentTexture;
    
    float current_f_0, current_sigma_f, current_theta_0, current_sigma_theta;
    float last_f_0, last_sigma_f, last_theta_0, last_sigma_theta;
    
};


END_NAMESPACE_MW


#endif /* OrientedNoise_hpp */
