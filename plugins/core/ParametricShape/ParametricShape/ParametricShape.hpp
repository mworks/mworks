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
    static const std::string SPLINE_RESOLUTION;
    static const std::string VERTEX_COORD_CENTER_X;
    static const std::string VERTEX_COORD_CENTER_Y;
    static const std::string VERTEX_COORD_RANGE;
    static const std::string MARGIN_PIXELS;
    static const std::string USE_ANTIALIASING;
    static const std::string MAX_SIZE_X;
    static const std::string MAX_SIZE_Y;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ParametricShape(const ParameterValueMap &parameters);
    ~ParametricShape();
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    static bool validateVertices(Datum &value);
    static std::vector<CGPoint> generatePoints(const Datum::list_value_type &vertexCoords, std::size_t numSamples);
    
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    void updateTexture(MetalDisplay &display);
    
    const VariablePtr vertices;
    const VariablePtr splineResolution;
    const VariablePtr vertexCoordCenterX;
    const VariablePtr vertexCoordCenterY;
    const VariablePtr vertexCoordRange;
    const VariablePtr marginPixels;
    const VariablePtr useAntialiasing;
    const VariablePtr maxSizeX;
    const VariablePtr maxSizeY;
    
    Datum currentVertices;
    int currentSplineResolution;
    
    cf::ObjectPtr<CGPathRef> path;
    double currentVertexCoordCenterX, currentVertexCoordCenterY;
    double currentVertexCoordRange;
    
    float currentMaxSizeX, currentMaxSizeY;
    
    std::size_t textureWidth, textureHeight;
    std::size_t textureBytesPerRow;
    std::unique_ptr<std::uint8_t[]> textureData;
    
    cf::ObjectPtr<CGContextRef> context;
    
    MWKTripleBufferedMTLResource<id<MTLTexture>> *texturePool;
    id<MTLTexture> currentTexture;
    
    std::size_t currentWidthPixels, currentHeightPixels;
    double currentMarginPixels;
    bool currentUseAntialiasing;
    
    std::size_t lastWidthPixels, lastHeightPixels;
    double lastMarginPixels;
    bool lastUseAntialiasing;
    
};


END_NAMESPACE_MW


#endif /* ParametricShape_hpp */
