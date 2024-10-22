//
//  OrientedNoise.cpp
//  OrientedNoise
//
//  Created by Christopher Stawarz on 10/30/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#include "OrientedNoise.hpp"


BEGIN_NAMESPACE_MW


const std::string OrientedNoise::F_0("f_0");
const std::string OrientedNoise::SIGMA_F("sigma_f");
const std::string OrientedNoise::THETA_0("theta_0");
const std::string OrientedNoise::SIGMA_THETA("sigma_theta");
const std::string OrientedNoise::DIRECTORY_PATH("directory_path");
const std::string OrientedNoise::NOISE_IMAGE_SELECTION("noise_image_selection");
const std::string OrientedNoise::RAND_SEED("rand_seed");


void OrientedNoise::describeComponent(ComponentInfo &info) {
    OrientedNoiseBase::describeComponent(info);
    
    info.setSignature("stimulus/oriented_noise");
    
    info.addParameter(F_0);
    info.addParameter(SIGMA_F);
    info.addParameter(THETA_0);
    info.addParameter(SIGMA_THETA);
    info.addParameter(DIRECTORY_PATH, false);
    info.addParameter(NOISE_IMAGE_SELECTION, "random_with_replacement");
    info.addParameter(RAND_SEED, false);
}


OrientedNoise::OrientedNoise(const ParameterValueMap &parameters) :
    OrientedNoiseBase(parameters),
    f0(registerVariable(parameters[F_0])),
    sigmaF(registerVariable(parameters[SIGMA_F])),
    theta0(registerVariable(parameters[THETA_0])),
    sigmaTheta(registerVariable(parameters[SIGMA_THETA])),
    directoryPath(optionalVariableOrText(parameters[DIRECTORY_PATH])),
    noiseImageSelection(variableOrText(parameters[NOISE_IMAGE_SELECTION])),
    randSeed(optionalVariable(parameters[RAND_SEED])),
    texturePool(nil),
    currentTexture(nil)
{ }


OrientedNoise::~OrientedNoise() {
    @autoreleasepool {
        currentTexture = nil;
        texturePool = nil;
    }
}


Datum OrientedNoise::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    Datum announceData = OrientedNoiseBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "oriented_noise");
    
    announceData.addElement(F_0, current_f_0);
    announceData.addElement(SIGMA_F, current_sigma_f);
    announceData.addElement(THETA_0, current_theta_0);
    announceData.addElement(SIGMA_THETA, current_sigma_theta);
    if (directoryPath) {
        announceData.addElement(DIRECTORY_PATH, currentDirectoryPath);
        if (currentImageFileIndex >= 0 && currentImageFileIndex < imageFilePaths.size()) {
            announceData.addElement("current_" STIM_FILENAME, imageFilePaths.at(currentImageFileIndex));
            announceData.addElement("current_" STIM_FILE_HASH, imageFileHashes.at(currentImageFileIndex));
        }
        announceData.addElement(NOISE_IMAGE_SELECTION, currentNoiseImageSelectionName);
    }
    announceData.addElement(RAND_SEED, currentRandSeed);
    
    return announceData;
}


auto OrientedNoise::noiseImageSelectionFromName(const std::string &name) -> NoiseImageSelection {
    if (name == "sequential") {
        return NoiseImageSelection::Sequential;
    } else if (name == "random_with_replacement") {
        return NoiseImageSelection::RandomWithReplacement;
    } else if (name == "random_without_replacement") {
        return NoiseImageSelection::RandomWithoutReplacement;
    }
    throw SimpleException(M_PLUGIN_MESSAGE_DOMAIN,
                          boost::format("Invalid noise image selection method: \"%s\"") % name);
}


void OrientedNoise::selectImageFileIndices(std::deque<std::size_t> &imageFileIndices,
                                           std::size_t numImages,
                                           NoiseImageSelection selectionType,
                                           int previousIndex,
                                           std::mt19937 &auxRandGen)
{
    imageFileIndices.resize(numImages);
    
    if (numImages == 1) {
        // If there's just one image, then there's only one possible sequence
        imageFileIndices.front() = 0;
        return;
    }
    
    if (selectionType == NoiseImageSelection::RandomWithReplacement) {
        // Select indices at random, ensuring that the same index isn't used twice in a row
        std::uniform_int_distribution<std::size_t> randDist(0, numImages - 1);
        for (auto &currentIndex : imageFileIndices) {
            do {
                currentIndex = randDist(auxRandGen);
            } while (currentIndex == previousIndex);
            previousIndex = currentIndex;
        }
        return;
    }
    
    // Select indices sequentially
    std::iota(imageFileIndices.begin(), imageFileIndices.end(), 0);
    
    if (selectionType == NoiseImageSelection::RandomWithoutReplacement) {
        // Shuffle the indices, ensuring that the first index in the new sequence is
        // different from the last index in the previous sequence
        do {
            std::shuffle(imageFileIndices.begin(), imageFileIndices.end(), auxRandGen);
        } while (imageFileIndices.front() == previousIndex);
    }
}


void OrientedNoise::loadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    ColoredTransformStimulus::loadMetal(display);
    
    if (randSeed) {
        currentRandSeed = randSeed->getValue().getInteger();
    } else {
        currentRandSeed = Clock::instance()->getSystemTimeNS();
    }
    auxRandGen.seed(currentRandSeed);
    
    if (directoryPath) {
        
        //
        // Load base noise frames from image files
        //
        
        currentDirectoryPath = directoryPath->getValue().getString();
        currentNoiseImageSelectionName = noiseImageSelection->getValue().getString();
        currentNoiseImageSelection = noiseImageSelectionFromName(currentNoiseImageSelectionName);
        
        std::string workingPath;
        if (auto experiment = GlobalCurrentExperiment) {
            workingPath = experiment->getWorkingPath();
        }
        
        std::vector<std::string> filePaths;
        getFilePaths(workingPath, currentDirectoryPath, filePaths);
        std::sort(filePaths.begin(), filePaths.end());
        
        for (auto &filePath : filePaths) {
            loadImageFile(filePath);
        }
        
        imageFileIndices.clear();
        currentImageFileIndex = -1;
        
    } else {
        
        //
        // Determine noise texture size
        //
        
        float sizeX, sizeY;
        getCurrentSize(sizeX, sizeY);
        
        std::size_t textureWidth, textureHeight;
        display.getCurrentTextureSizeForDisplayArea(sizeX, sizeY, textureWidth, textureHeight);
        
        currentLog2Cols = pixelsToLog2Pixels(textureWidth);
        currentLog2Rows = pixelsToLog2Pixels(textureHeight);
        
        //
        // We need to generate base noise frames dynamically, so seed the random number generator
        //
        
        std::uniform_int_distribution<randGenType::scalar_type> seedDist(randGenType::min(), randGenType::max());
        randGenType::vector_type seed(0);
        for (std::size_t i = 0; i < randGenType::vector_width; i++) {
            seed[i] = seedDist(auxRandGen);
        }
        randGen.seed(seed);
        
    }
    
    fftSetup.reset(vDSP_create_fftsetup(currentLog2Rows + currentLog2Cols, kFFTRadix2));
    if (!fftSetup) {
        throw std::bad_alloc();
    }
    
    currentRows = 1 << currentLog2Rows;
    currentCols = 1 << currentLog2Cols;
    
    //
    // Allocate buffers
    //
    {
        const auto numTexels = currentRows * currentCols;
        noiseTexture.resize(numTexels);
        imaginaryComponents.resize(numTexels);
        fftMask.resize(numTexels);
        temp.resize(2 * numTexels);  // Twice as large as others
    }
    
    //
    // Create texture pool
    //
    {
        auto textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Float
                                                                                    width:currentCols
                                                                                   height:currentRows
                                                                                mipmapped:NO];
        // Keep the default value of storageMode: MTLStorageModeManaged on macOS, MTLStorageModeShared on iOS
        
        texturePool = [MWKTripleBufferedMTLResource textureWithDevice:display.getMetalDevice()
                                                           descriptor:textureDescriptor];
    }
    
    //
    // Create render pipeline state
    //
    {
        auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
        auto vertexFunction = loadShaderFunction(library, "vertexShader");
        auto fragmentFunction = loadShaderFunction(library, "fragmentShader");
        auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
        renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
    }
}


void OrientedNoise::unloadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    currentTexture = nil;
    texturePool = nil;
    
    temp = decltype(temp)();
    fftMask = decltype(fftMask)();
    imaginaryComponents = decltype(imaginaryComponents)();
    noiseTexture = decltype(noiseTexture)();
    
    fftSetup.reset();
    
    imageFileData = decltype(imageFileData)();
    imageFileHashes = decltype(imageFileHashes)();
    imageFilePaths = decltype(imageFilePaths)();
    
    ColoredTransformStimulus::unloadMetal(display);
}


void OrientedNoise::drawMetal(MetalDisplay &display) {
    ColoredTransformStimulus::drawMetal(display);
    
    current_f_0 = f0->getValue().getFloat();
    current_sigma_f = sigmaF->getValue().getFloat();
    current_theta_0 = theta0->getValue().getFloat();
    current_sigma_theta = sigmaTheta->getValue().getFloat();
    
    if (!currentTexture ||
        current_f_0 != last_f_0 ||
        current_sigma_f != last_sigma_f ||
        current_theta_0 != last_theta_0 ||
        current_sigma_theta != last_sigma_theta)
    {
        // This is the first draw call, or parameters of the FFT mask have changed, so we need
        // to regenerate the mask
        computeFFTMask();
    }
    
    if (directoryPath) {
        if (imageFileIndices.empty()) {
            // Re-generate the list of image file indices
            selectImageFileIndices(imageFileIndices,
                                   imageFileData.size(),
                                   currentNoiseImageSelection,
                                   currentImageFileIndex,
                                   auxRandGen);
        }
        
        // Copy next base noise frame to noiseTexture
        currentImageFileIndex = imageFileIndices.front();
        noiseTexture = imageFileData.at(currentImageFileIndex);
        imageFileIndices.pop_front();
    } else {
        // Regenerate base noise
        if (noiseTexture.size() % randGenType::vector_width != 0) {
            throw std::logic_error("Noise texture size is not a multiple of random number generator's vector width");
        }
        for (std::size_t offset = 0; offset < noiseTexture.size(); offset += randGenType::vector_width) {
            // The heap memory allocated by noiseTexture may not meet the alignment requirements of
            // float_vector_type, so we need to interpret it as packed_float_vector_type in order to
            // safely assign texels to it
            auto texels = randGenType::asNormalizedFloat(randGen());
            *reinterpret_cast<randGenType::packed_float_vector_type *>(noiseTexture.data() + offset) = texels;
        }
    }
    
    filterNoise();
    
    // Copy the noise data to the next-available texture from the pool
    currentTexture = [texturePool acquireWithCommandBuffer:display.getCurrentMetalCommandBuffer()];
    [currentTexture replaceRegion:MTLRegionMake2D(0, 0, currentCols, currentRows)
                      mipmapLevel:0
                        withBytes:noiseTexture.data()
                      bytesPerRow:currentCols * 4];
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    setCurrentMVPMatrix(display, renderCommandEncoder, 0);
    
    [renderCommandEncoder setFragmentTexture:currentTexture atIndex:0];
    setCurrentColor(renderCommandEncoder, 0);
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
    
    last_f_0 = current_f_0;
    last_sigma_f = current_sigma_f;
    last_theta_0 = current_theta_0;
    last_sigma_theta = current_sigma_theta;
}


void OrientedNoise::loadImageFile(const std::string &filePath) {
    imageFilePaths.emplace_back(pathFromParameterValue(filePath).string());
    imageFileHashes.emplace_back();
    auto imageSource = ImageFileStimulus::loadImageFile(imageFilePaths.back(), imageFileHashes.back());
    
    auto image = cf::ObjectPtr<CGImageRef>::created(CGImageSourceCreateImageAtIndex(imageSource.get(), 0, nullptr));
    auto width = CGImageGetWidth(image.get());
    auto height = CGImageGetHeight(image.get());
    
    if (imageFileData.empty()) {
        auto log2Width = std::log2(width);
        auto log2Height = std::log2(height);
        if (std::round(log2Width) != log2Width || std::round(log2Height) != log2Height) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Image width and height must be powers of two");
        }
        if (log2Width < minLog2RowsCols || log2Height < minLog2RowsCols) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Images must have at least %1% pixels in each dimension") %
                                  (1 << minLog2RowsCols));
        }
        currentLog2Cols = log2Width;
        currentLog2Rows = log2Height;
    } else if (width != 1 << currentLog2Cols || height != 1 << currentLog2Rows) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "All images must have the same dimensions");
    }
    
    imageFileData.emplace_back(width * height);
    auto colorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(CGColorSpaceCreateWithName(kCGColorSpaceLinearGray));
    
    vImage_Buffer buf = {
        .data = imageFileData.back().data(),
        .height = height,
        .width = width,
        .rowBytes = width * 4
    };
    
    vImage_CGImageFormat format = {
        .bitsPerComponent = 32,
        .bitsPerPixel = 32,
        .colorSpace = colorSpace.get(),
        .bitmapInfo = CGBitmapInfo(kCGImageAlphaNone) | kCGBitmapFloatComponents | kCGBitmapByteOrder32Host,
        .version = 0,
        .decode = nullptr,
        .renderingIntent = kCGRenderingIntentPerceptual
    };
    
    auto error = vImageBuffer_InitWithCGImage(&buf,
                                              &format,
                                              nullptr,
                                              image.get(),
                                              kvImageNoAllocate);
    if (kvImageNoError != error) {
        throw SimpleException(boost::format("Cannot extract image file data (error = %d)") % error);
    }
}


static float degreesToRadians(float degrees) {
    return degrees / 180.0f * std::numbers::pi_v<float>;
}


static void wrapTheta(std::vector<float> &theta) {
    auto data = theta.data();
    const auto size = theta.size();
    const float two_pi = 2.0f * std::numbers::pi_v<float>;
    const float neg_pi = -std::numbers::pi_v<float>;
    
    // Divide by two pi, take the fractional part, multiply by two pi,
    // then add two pi to make negative values positive
    vDSP_vsdiv(data, 1, &two_pi, data, 1, size);
    vDSP_vfrac(data, 1, data, 1, size);
    vDSP_vsmsa(data, 1, &two_pi, &two_pi, data, 1, size);
    
    // Since we added two pi, we need to "re-wrap".  Again, divide by
    // two pi, take the fractional part, and multiply by two pi.
    // Then, subtract pi, so all values are in the range [-pi, pi).
    vDSP_vsdiv(data, 1, &two_pi, data, 1, size);
    vDSP_vfrac(data, 1, data, 1, size);
    vDSP_vsmsa(data, 1, &two_pi, &neg_pi, data, 1, size);
}


static void fftShift(std::vector<float> &matrix, std::vector<float> &temp, std::size_t rows, std::size_t cols) {
    // Row and column counts are always powers of 2
    auto tempRows = rows / 2;
    auto tempCols = cols / 2;
    
    //
    // Swap quadrants 2 and 4
    //
    auto quad2 = matrix.data();
    auto quad4 = matrix.data() + cols * tempRows + tempCols;
    vDSP_mmov(quad4, temp.data(), tempCols, tempRows, cols, tempCols);
    vDSP_mmov(quad2, quad4, tempCols, tempRows, cols, cols);
    vDSP_mmov(temp.data(), quad2, tempCols, tempRows, tempCols, cols);
    
    //
    // Swap quadrants 1 and 3
    //
    auto quad1 = matrix.data() + tempCols;
    auto quad3 = matrix.data() + cols * tempRows;
    vDSP_mmov(quad3, temp.data(), tempCols, tempRows, cols, tempCols);
    vDSP_mmov(quad1, quad3, tempCols, tempRows, cols, cols);
    vDSP_mmov(temp.data(), quad1, tempCols, tempRows, tempCols, cols);
}


void OrientedNoise::computeFFTMask() {
    //
    // The FFT mask is computed via the following equations:
    //
    // f_r = sqrt(f_x ** 2 + f_y ** 2)
    // G_r = exp(-0.5 * ((f_r - f_0) / sigma_f) ** 2)
    //
    // theta = atan(f_y / f_x)
    // G_theta_1 = exp(-0.5 * ((theta - theta_0) / sigma_theta) ** 2)
    // G_theta_2 = exp(-0.5 * ((theta - (theta_0 + pi)) / sigma_theta) ** 2)
    // G_theta = G_theta_1 + G_theta_2
    //
    // fftMask = G_r * G_theta
    //
    
    bool badParams = false;
    
    if (current_f_0 < 0.0f) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "%s must be greater than or equal to zero (currently %g)",
               F_0.c_str(),
               current_f_0);
        badParams = true;
    }
    if (current_sigma_f <= 0.0f) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "%s must be greater than zero (currently %g)",
               SIGMA_F.c_str(),
               current_sigma_f);
        badParams = true;
    }
    if (current_theta_0 < 0.0f || current_theta_0 > 180.0f) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "%s must be in the range [0, 180] (currently %g)",
               THETA_0.c_str(),
               current_theta_0);
        badParams = true;
    }
    if (current_sigma_theta <= 0.0f || current_sigma_theta > 90.0f) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "%s must be in the range (0, 90] (currently %g)",
               SIGMA_THETA.c_str(),
               current_sigma_theta);
        badParams = true;
    }
    
    if (badParams) {
        // Set all mask values to zero
        vDSP_vclr(fftMask.data(), 1, fftMask.size());
        return;
    }
    
    //
    // Create (f_x, f_y) matrix.
    //
    // The values for f_x and f_y are determined by the Nyquist critical
    // frequency (f_c), which is equal to half of the sampling rate in a given
    // dimension.  Both f_x and f_y lie in the half-open interval [-f_c, f_c).
    // The interval is half open, because the FFT has the same value at -f_c and
    // f_c, so the latter is omitted.
    //
    // For more information, see _Numerical Recipes in C_, section 12.1,
    // "Fourier Transform of Discretely Sampled Data".
    //
    for (std::size_t i = 0; i < currentRows; i++) {
        // f_x
        {
            const float samplingRate = float(currentCols) / current_sizex;
            const float start = -0.5f * samplingRate;
            const float step = samplingRate / float(currentCols);
            vDSP_vramp(&start, &step, temp.data() + 2*i*currentCols, 2, currentCols);
        }
        
        // f_y
        {
            const float samplingRate = float(currentRows) / current_sizey;
            const float value = -0.5f * samplingRate + i * samplingRate / float(currentRows);
            vDSP_vfill(&value, temp.data() + 2*i*currentCols + 1, 2, currentCols);
        }
    }
    
    // Convert (f_x, f_y) to (f_r, theta)
    vDSP_polar(temp.data(), 2, temp.data(), 2, temp.size() / 2);
    
    //
    // Compute G_r (stored in fftMask)
    //
    {
        auto G_r = fftMask.data();
        const int G_r_size = fftMask.size();
        
        // (f_r - current_f_0) / current_sigma_f -> G_r
        {
            const float arg1 = 1.0f / current_sigma_f;
            const float arg2 = -current_f_0 / current_sigma_f;
            vDSP_vsmsa(temp.data(), 2, &arg1, &arg2, G_r, 1, G_r_size);
        }
        
        // G_r * G_r -> G_r
        vDSP_vsq(G_r, 1, G_r, 1, G_r_size);
        
        // -0.5 * G_r -> G_r
        {
            const float arg = -0.5f;
            vDSP_vsmul(G_r, 1, &arg, G_r, 1, G_r_size);
        }
        
        // exp(G_r) -> G_r
        vvexpf(G_r, G_r, &G_r_size);
    }
    
    //
    // Compute G_theta (multiplied in to fftMask)
    //
    {
        auto G_theta_1 = temp.data();
        auto G_theta_2 = temp.data() + 1;
        const vDSP_Stride G_theta_stride = 2;
        const int G_theta_size = temp.size() / 2;
        
        // theta - current_theta_0 -> G_theta_1 (overwrites f_r)
        {
            const float arg = -degreesToRadians(current_theta_0);
            vDSP_vsadd(temp.data() + 1, 2, &arg, G_theta_1, G_theta_stride, G_theta_size);
        }
        
        // theta - (current_theta_0 + M_PI) -> G_theta_2 (overwrites theta)
        {
            const float arg = -(degreesToRadians(current_theta_0) + std::numbers::pi_v<float>);
            vDSP_vsadd(temp.data() + 1, 2, &arg, G_theta_2, G_theta_stride, G_theta_size);
        }
        
        // wrapTheta(G_theta_12) -> G_theta_12
        wrapTheta(temp);
        
        // G_theta_12 / current_sigma_theta -> G_theta_12
        {
            const float arg = degreesToRadians(current_sigma_theta);
            vDSP_vsdiv(temp.data(), 1, &arg, temp.data(), 1, temp.size());
        }
        
        // G_theta_12 * G_theta_12 -> G_theta_12
        vDSP_vsq(temp.data(), 1, temp.data(), 1, temp.size());
        
        // -0.5 * G_theta_12 -> G_theta_12
        {
            const float arg = -0.5f;
            vDSP_vsmul(temp.data(), 1, &arg, temp.data(), 1, temp.size());
        }
        
        // exp(G_theta_12) -> G_theta_12
        {
            const int size = temp.size();
            vvexpf(temp.data(), temp.data(), &size);
        }
        
        // G_theta_1 + G_theta_2 -> G_theta_1
        vDSP_vadd(G_theta_1, G_theta_stride, G_theta_2, G_theta_stride, G_theta_1, G_theta_stride, G_theta_size);
        
        // fftMask * G_theta_1 -> fftMask
        vDSP_vmul(fftMask.data(), 1, G_theta_1, G_theta_stride, fftMask.data(), 1, fftMask.size());
    }
    
    // Shift zero-frequency component from center to start
    fftShift(fftMask, temp, currentRows, currentCols);
}


void OrientedNoise::filterNoise() {
    // Set imaginary components to zero
    vDSP_vclr(imaginaryComponents.data(), 1, imaginaryComponents.size());
    
    // Compute forward FFT
    DSPSplitComplex fftData = { noiseTexture.data(), imaginaryComponents.data() };
    DSPSplitComplex fftBuffer = { temp.data(), temp.data() + temp.size() / 2 };
    vDSP_fft2d_zipt(fftSetup.get(), &fftData, 1, 0, &fftBuffer, currentLog2Cols, currentLog2Rows, kFFTDirection_Forward);
    
    // Apply FFT mask
    vDSP_zrvmul(&fftData, 1, fftMask.data(), 1, &fftData, 1, noiseTexture.size());
    
    // Compute inverse FFT
    vDSP_fft2d_zipt(fftSetup.get(), &fftData, 1, 0, &fftBuffer, currentLog2Cols, currentLog2Rows, kFFTDirection_Inverse);
    
    // Translate values so mean is zero
    {
        float mean;
        vDSP_meanv(noiseTexture.data(), 1, &mean, noiseTexture.size());
        const float offset = -mean;
        vDSP_vsadd(noiseTexture.data(), 1, &offset, noiseTexture.data(), 1, noiseTexture.size());
    }
    
    // Scale values to lie in range [-0.5, 0.5], then translate so mean is 0.5
    {
        float maxMag;
        vDSP_maxmgv(noiseTexture.data(), 1, &maxMag, noiseTexture.size());
        const float scale = 0.5f / maxMag;
        const float offset = 0.5f;
        vDSP_vsmsa(noiseTexture.data(), 1, &scale, &offset, noiseTexture.data(), 1, noiseTexture.size());
    }
}


END_NAMESPACE_MW
