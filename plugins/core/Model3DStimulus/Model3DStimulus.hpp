//
//  Model3DStimulus.hpp
//  Model3DStimulus
//
//  Created by Christopher Stawarz on 7/15/24.
//

#ifndef Model3DStimulus_hpp
#define Model3DStimulus_hpp


BEGIN_NAMESPACE_MW


class Model3DStimulus : public ColoredTransformStimulus {
    
public:
    static const std::string PATH;
    static const std::string VERTEX_COORD_CENTER_X;
    static const std::string VERTEX_COORD_CENTER_Y;
    static const std::string VERTEX_COORD_CENTER_Z;
    static const std::string VERTEX_COORD_RANGE;
    static const std::string FIELD_OF_VIEW;
    static const std::string VIEWPOINT_ALTITUDE;
    static const std::string VIEWPOINT_AZIMUTH;
    static const std::string LIGHT_ALTITUDE;
    static const std::string LIGHT_AZIMUTH;
    static const std::string AMBIENT_LIGHT_INTENSITY;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit Model3DStimulus(const ParameterValueMap &parameters);
    ~Model3DStimulus();
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    simd::float4x4 getCurrentMVPMatrix(const simd::float4x4 &projectionMatrix) const override;
    
    bool updateParameters();
    simd::float4x4 getModelMVPMatrix(MetalDisplay &display) const;
    
    const VariablePtr path;
    const VariablePtr vertexCoordCenterX;
    const VariablePtr vertexCoordCenterY;
    const VariablePtr vertexCoordCenterZ;
    const VariablePtr vertexCoordRange;
    const VariablePtr fieldOfView;
    const VariablePtr viewpointAltitude;
    const VariablePtr viewpointAzimuth;
    const VariablePtr lightAltitude;
    const VariablePtr lightAzimuth;
    const VariablePtr ambientLightIntensity;
    
    std::string filePath;
    std::string fileHash;
    
    double currentVertexCoordCenterX, currentVertexCoordCenterY, currentVertexCoordCenterZ;
    double currentVertexCoordRange;
    
    simd::float3 boundingSphereCenter;
    float boundingSphereRadius;
    
    NSArray<MTKMesh *> *mtkMeshes;
    id<MTLDepthStencilState> depthStencilState;
    
    float currentFieldOfView;
    float currentViewpointAltitude, currentViewpointAzimuth;
    float currentLightAltitude, currentLightAzimuth;
    float currentAmbientLightIntensity;
    
};


END_NAMESPACE_MW


#endif /* Model3DStimulus_hpp */
