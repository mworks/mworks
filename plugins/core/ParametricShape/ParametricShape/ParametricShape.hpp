//
//  ParametricShape.hpp
//  ParametricShape
//
//  Created by Christopher Stawarz on 10/6/21.
//

#ifndef ParametricShape_hpp
#define ParametricShape_hpp


BEGIN_NAMESPACE_MW


class ParametricShape : public ColoredTransformStimulus {
    
public:
    static const std::string VERTICES;
    static const std::string MAX_SIZE_X;
    static const std::string MAX_SIZE_Y;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ParametricShape(const ParameterValueMap &parameters);
    ~ParametricShape();
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    static constexpr double vertexCoordMin = -2.0;
    static constexpr double vertexCoordMax = 2.0;
    static constexpr double vertexCoordScale = (vertexCoordMax - vertexCoordMin);
    
    static bool validateVertices(const Datum &value);
    static std::vector<CGPoint> generatePoints(const Datum::list_value_type &vertexCoords);
    
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    void computeTextureDimensions(double widthDegrees,
                                  double heightDegrees,
                                  std::size_t &widthPixels,
                                  std::size_t &heightPixels) const;
    void updateTexture(MetalDisplay &display);
    
    const VariablePtr vertices;
    const VariablePtr maxSizeX;
    const VariablePtr maxSizeY;
    
    Datum currentVertices;
    float currentMaxSizeX, currentMaxSizeY;
    
    std::size_t viewportWidth, viewportHeight;
    double pixelsPerDegree;
    
    std::size_t textureWidth, textureHeight;
    std::size_t textureBytesPerRow;
    std::unique_ptr<std::uint8_t[]> textureData;
    
    cf::ObjectPtr<CGContextRef> context;
    cf::ObjectPtr<CGPathRef> path;
    
    MWKTripleBufferedMTLResource<id<MTLTexture>> *texturePool;
    id<MTLTexture> currentTexture;
    
    std::size_t currentWidthPixels, currentHeightPixels;
    std::size_t lastWidthPixels, lastHeightPixels;
    
};


END_NAMESPACE_MW


#endif /* ParametricShape_hpp */
