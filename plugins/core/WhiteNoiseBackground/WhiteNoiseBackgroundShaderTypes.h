//
//  WhiteNoiseBackgroundShaderTypes.h
//  WhiteNoiseBackground
//
//  Created by Christopher Stawarz on 3/4/21.
//

#ifndef WhiteNoiseBackgroundShaderTypes_h
#define WhiteNoiseBackgroundShaderTypes_h


#ifndef __METAL_VERSION__
BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(white_noise_background)
#endif


struct MinStdRand {
    
    using value_type = uint32_t;
    
    static constexpr value_type a() { return 48271u; }
    static constexpr value_type c() { return 0u; }
    static constexpr value_type m() { return 2147483647u; }
    
    static constexpr value_type min() { return c() == 0u; }
    static constexpr value_type max() { return m() - 1u; }
    
    static constexpr value_type next(value_type previous) { return (a() * previous + c()) % m(); }
    
    static constexpr float normalize(value_type value) { return float(value - min()) / float(max() - min()); }
    
};

#ifndef __METAL_VERSION__
// Sanity checks
static_assert(MinStdRand::a() == std::minstd_rand::multiplier);
static_assert(MinStdRand::c() == std::minstd_rand::increment);
static_assert(MinStdRand::m() == std::minstd_rand::modulus);
static_assert(MinStdRand::min() == std::minstd_rand::min());
static_assert(MinStdRand::max() == std::minstd_rand::max());
#endif


enum {
    seedTextureIndex,
    noiseTextureIndex
};


enum {
    rgbNoiseFunctionConstantIndex
};


enum {
    redNoiseTextureIndex,
    greenNoiseTextureIndex,
    blueNoiseTextureIndex
};


#ifndef __METAL_VERSION__
END_NAMESPACE(white_noise_background)
END_NAMESPACE_MW
#endif


#endif /* WhiteNoiseBackgroundShaderTypes_h */
