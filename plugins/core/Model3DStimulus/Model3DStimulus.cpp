//
//  Model3DStimulus.cpp
//  Model3DStimulus
//
//  Created by Christopher Stawarz on 7/15/24.
//

#include "Model3DStimulus.hpp"

using namespace mw::aapl_math_utilities;


BEGIN_NAMESPACE_MW


const std::string Model3DStimulus::PATH("path");
const std::string Model3DStimulus::VERTEX_COORD_CENTER_X("vertex_coord_center_x");
const std::string Model3DStimulus::VERTEX_COORD_CENTER_Y("vertex_coord_center_y");
const std::string Model3DStimulus::VERTEX_COORD_CENTER_Z("vertex_coord_center_z");
const std::string Model3DStimulus::VERTEX_COORD_RANGE("vertex_coord_range");
const std::string Model3DStimulus::FIELD_OF_VIEW("field_of_view");
const std::string Model3DStimulus::VIEWPOINT_ALTITUDE("viewpoint_altitude");
const std::string Model3DStimulus::VIEWPOINT_AZIMUTH("viewpoint_azimuth");
const std::string Model3DStimulus::LIGHT_ALTITUDE("light_altitude");
const std::string Model3DStimulus::LIGHT_AZIMUTH("light_azimuth");
const std::string Model3DStimulus::AMBIENT_LIGHT_INTENSITY("ambient_light_intensity");


void Model3DStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/model_3d");
    
    info.addParameter(PATH);
    info.addParameter(VERTEX_COORD_CENTER_X, false);
    info.addParameter(VERTEX_COORD_CENTER_Y, false);
    info.addParameter(VERTEX_COORD_CENTER_Z, false);
    info.addParameter(VERTEX_COORD_RANGE, false);
    info.addParameter(FIELD_OF_VIEW, "54.0");  // MDLCamera's default FOV
    info.addParameter(VIEWPOINT_ALTITUDE, "0.0");
    info.addParameter(VIEWPOINT_AZIMUTH, "0.0");
    info.addParameter(LIGHT_ALTITUDE, "0.0");
    info.addParameter(LIGHT_AZIMUTH, "0.0");
    info.addParameter(AMBIENT_LIGHT_INTENSITY, "0.2");
}


Model3DStimulus::Model3DStimulus(const ParameterValueMap &parameters) :
    ColoredTransformStimulus(parameters),
    path(variableOrText(parameters[PATH])),
    vertexCoordCenterX(optionalVariable(parameters[VERTEX_COORD_CENTER_X])),
    vertexCoordCenterY(optionalVariable(parameters[VERTEX_COORD_CENTER_Y])),
    vertexCoordCenterZ(optionalVariable(parameters[VERTEX_COORD_CENTER_Z])),
    vertexCoordRange(optionalVariable(parameters[VERTEX_COORD_RANGE])),
    fieldOfView(registerVariable(parameters[FIELD_OF_VIEW])),
    viewpointAltitude(registerVariable(parameters[VIEWPOINT_ALTITUDE])),
    viewpointAzimuth(registerVariable(parameters[VIEWPOINT_AZIMUTH])),
    lightAltitude(registerVariable(parameters[LIGHT_ALTITUDE])),
    lightAzimuth(registerVariable(parameters[LIGHT_AZIMUTH])),
    ambientLightIntensity(registerVariable(parameters[AMBIENT_LIGHT_INTENSITY])),
    mtkMeshes(nil),
    depthStencilState(nil)
{ }


Model3DStimulus::~Model3DStimulus() {
    @autoreleasepool {
        depthStencilState = nil;
        mtkMeshes = nil;
    }
}


Datum Model3DStimulus::getCurrentAnnounceDrawData() {
    auto announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "model_3d");
    
    announceData.addElement(STIM_FILENAME, filePath.string());
    
    if (vertexCoordCenterX) {
        announceData.addElement(VERTEX_COORD_CENTER_X, currentVertexCoordCenterX);
    }
    if (vertexCoordCenterY) {
        announceData.addElement(VERTEX_COORD_CENTER_Y, currentVertexCoordCenterY);
    }
    if (vertexCoordCenterZ) {
        announceData.addElement(VERTEX_COORD_CENTER_Z, currentVertexCoordCenterZ);
    }
    if (vertexCoordRange) {
        announceData.addElement(VERTEX_COORD_RANGE, currentVertexCoordRange);
    }
    
    announceData.addElement(FIELD_OF_VIEW, currentFieldOfView);
    announceData.addElement(VIEWPOINT_ALTITUDE, currentViewpointAltitude);
    announceData.addElement(VIEWPOINT_AZIMUTH, currentViewpointAzimuth);
    announceData.addElement(LIGHT_ALTITUDE, currentLightAltitude);
    announceData.addElement(LIGHT_AZIMUTH, currentLightAzimuth);
    announceData.addElement(AMBIENT_LIGHT_INTENSITY, currentAmbientLightIntensity);
    
    return announceData;
}


void Model3DStimulus::loadMetal(MetalDisplay &display) {
    ColoredTransformStimulus::loadMetal(display);
    
    //
    // Get file path
    //
    
    filePath = pathFromParameterValue(path);
    auto filePathString = @(filePath.string().c_str());
    
    //
    // Validate file extension
    //
    {
        auto fileExtension = filePathString.pathExtension;
        if (![MDLAsset canImportFileExtension:fileExtension]) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Unsupported model file extension: \"%1%\"")
                                  % fileExtension.UTF8String);
        }
    }
    
    //
    // Load model file
    //
    
    MDLAsset *mdlAsset = nil;
    {
        auto mdlVertexDescriptor = [[MDLVertexDescriptor alloc] init];
        mdlVertexDescriptor.attributes[0] = [[MDLVertexAttribute alloc] initWithName:MDLVertexAttributePosition
                                                                              format:MDLVertexFormatFloat3
                                                                              offset:0
                                                                         bufferIndex:0];
        mdlVertexDescriptor.attributes[1] = [[MDLVertexAttribute alloc] initWithName:MDLVertexAttributeNormal
                                                                              format:MDLVertexFormatFloat3
                                                                              offset:sizeof(float) * 3
                                                                         bufferIndex:0];
        mdlVertexDescriptor.layouts[0] = [[MDLVertexBufferLayout alloc] initWithStride:sizeof(float) * 6];
        
        auto mdlBufferAllocator = [[MTKMeshBufferAllocator alloc] initWithDevice:display.getMetalDevice()];
        
        NSError *error = nil;
        mdlAsset = [[MDLAsset alloc] initWithURL:[NSURL fileURLWithPath:filePathString]
                                vertexDescriptor:mdlVertexDescriptor
                                 bufferAllocator:mdlBufferAllocator
                                preserveTopology:NO
                                           error:&error];
        if (!mdlAsset || error) {
            auto message = std::string("Cannot load model file");
            if (error) {
                message += std::string(": ") + error.localizedDescription.UTF8String;
            }
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, message);
        }
        if (!mdlAsset.count) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Model file contains no objects");
        }
    }
    
    //
    // Compute viewing frustum parameters
    //
    // We want to frame the asset's contents as tightly as possible, given the current field of view,
    // while also ensuring that everything remains visible under arbitrary rotation.  To accomplish this,
    // we use the asset's bounding box to determine its bounding cube and the sphere that circumscribes
    // the cube (i.e. the "bounding sphere").  We then construct the viewing frustum such that each face
    // of the frustum touches the bounding sphere at exactly one point.
    //
    {
        auto boundingBox = mdlAsset.boundingBox;
        auto boundingBoxCenter = (boundingBox.minBounds + boundingBox.maxBounds) / 2.0;
        
        if (vertexCoordCenterX) {
            currentVertexCoordCenterX = vertexCoordCenterX->getValue().getFloat();
        } else {
            currentVertexCoordCenterX = boundingBoxCenter.x;
        }
        
        if (vertexCoordCenterY) {
            currentVertexCoordCenterY = vertexCoordCenterY->getValue().getFloat();
        } else {
            currentVertexCoordCenterY = boundingBoxCenter.y;
        }
        
        if (vertexCoordCenterZ) {
            currentVertexCoordCenterZ = vertexCoordCenterZ->getValue().getFloat();
        } else {
            currentVertexCoordCenterZ = boundingBoxCenter.z;
        }
        
        if (vertexCoordRange) {
            currentVertexCoordRange = vertexCoordRange->getValue().getFloat();
        } else {
            auto halfWidth = std::max(std::abs(currentVertexCoordCenterX - boundingBox.minBounds.x),
                                      std::abs(currentVertexCoordCenterX - boundingBox.maxBounds.x));
            auto halfHeight = std::max(std::abs(currentVertexCoordCenterY - boundingBox.minBounds.y),
                                       std::abs(currentVertexCoordCenterY - boundingBox.maxBounds.y));
            auto halfDepth = std::max(std::abs(currentVertexCoordCenterZ - boundingBox.minBounds.z),
                                      std::abs(currentVertexCoordCenterZ - boundingBox.maxBounds.z));
            currentVertexCoordRange = 2.0 * std::max(std::max(halfWidth, halfHeight), halfDepth);
        }
        if (currentVertexCoordRange <= 0.0) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Vertex coordinate range must be greater than zero");
        }
        
        boundingSphereCenter = simd::make_float3(currentVertexCoordCenterX,
                                                 currentVertexCoordCenterY,
                                                 currentVertexCoordCenterZ);
        boundingSphereRadius = currentVertexCoordRange * std::sqrt(3.0) / 2.0;
        
        // If the user specified the vertex coordinate range, warn if the vertex coordinate grid
        // doesn't contain all the vertices
        if (vertexCoordRange) {
            auto vertexCoordMin = boundingSphereCenter - 0.5 * currentVertexCoordRange;
            auto vertexCoordMax = boundingSphereCenter + 0.5 * currentVertexCoordRange;
            if (simd::any(boundingBox.minBounds < vertexCoordMin) ||
                simd::any(boundingBox.maxBounds > vertexCoordMax))
            {
                mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                         "Vertex coordinate grid for model 3D stimulus \"%s\" does not contain all vertices in "
                         "the model: coordinate grid covers (%g, %g, %g) to (%g, %g, %g), but vertices range from "
                         "(%g, %g, %g) to (%g, %g, %g)",
                         getTag().c_str(),
                         vertexCoordMin.x,
                         vertexCoordMin.y,
                         vertexCoordMin.z,
                         vertexCoordMax.x,
                         vertexCoordMax.y,
                         vertexCoordMax.z,
                         boundingBox.minBounds.x,
                         boundingBox.minBounds.y,
                         boundingBox.minBounds.z,
                         boundingBox.maxBounds.x,
                         boundingBox.maxBounds.y,
                         boundingBox.maxBounds.z);
            }
        }
    }
    
    //
    // Create meshes
    //
    {
        NSError *error = nil;
        mtkMeshes = [MTKMesh newMeshesFromAsset:mdlAsset device:display.getMetalDevice() sourceMeshes:nil error:&error];
        if (!mtkMeshes) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  "Cannot extract mesh data from model file",
                                  error.localizedDescription.UTF8String);
        }
        if (!mtkMeshes.count) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Model file contains no meshes");
        }
    }
    
    //
    // Create render pipeline state
    //
    {
        NSError *error = nil;
        auto vertexDescriptor = MTKMetalVertexDescriptorFromModelIOWithError(mdlAsset.vertexDescriptor, &error);
        if (error) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  "Cannot create vertex descriptor for model file",
                                  error.localizedDescription.UTF8String);
        }
        
        auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
        auto vertexFunction = loadShaderFunction(library, "vertexShader");
        auto fragmentFunction = loadShaderFunction(library, "fragmentShader");
        auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
        renderPipelineDescriptor.vertexDescriptor = vertexDescriptor;
        renderPipelineDescriptor.depthAttachmentPixelFormat = display.getMetalDepthTexturePixelFormat();
        renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
    }
    
    //
    // Create depth stencil state
    //
    {
        auto depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLessEqual;
        depthStencilDescriptor.depthWriteEnabled = YES;
        depthStencilState = [display.getMetalDevice() newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    }
}


void Model3DStimulus::unloadMetal(MetalDisplay &display) {
    depthStencilState = nil;
    mtkMeshes = nil;
    
    ColoredTransformStimulus::unloadMetal(display);
}


void Model3DStimulus::drawMetal(MetalDisplay &display) {
    ColoredTransformStimulus::drawMetal(display);
    
    if (!updateParameters()) {
        return;
    }
    
    auto renderPassDescriptor = display.createMetalRenderPassDescriptor();
    renderPassDescriptor.depthAttachment.texture = display.getMetalDepthTexture();
    renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
    renderPassDescriptor.depthAttachment.clearDepth = 1.0;
    
    auto renderCommandEncoder = createRenderCommandEncoder(display, renderPassDescriptor);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    [renderCommandEncoder setDepthStencilState:depthStencilState];
    
    // "Model" MVP matrix
    {
        auto modelMVPMatrix = getModelMVPMatrix(display);
        setVertexBytes(renderCommandEncoder, modelMVPMatrix, 1);
    }
    // "Display" MVP matrix
    setCurrentMVPMatrix(display, renderCommandEncoder, 2);
    
    {
        auto lightRotationMatrix = simd::float3x3(matrix3x3_rotation(radians_from_degrees(currentLightAltitude), -1.0, 0.0, 0.0));
        lightRotationMatrix *= matrix3x3_rotation(radians_from_degrees(currentLightAzimuth), 0.0, 1.0, 0.0);
        auto lightDirection = lightRotationMatrix * simd::make_float3(0.0, 0.0, 1.0);
        setFragmentBytes(renderCommandEncoder, lightDirection, 0);
    }
    {
        auto ambientIntensity = float(currentAmbientLightIntensity);
        setFragmentBytes(renderCommandEncoder, ambientIntensity, 1);
    }
    setCurrentColor(renderCommandEncoder, 2);
    
    for (MTKMesh *mesh in mtkMeshes) {
        // We know the vertexBuffers array contains exactly one element, because every MDLVertexAttribute
        // we create specifies the same value (0) for the bufferIndex property
        auto vertexBuffer = mesh.vertexBuffers.firstObject;
        [renderCommandEncoder setVertexBuffer:vertexBuffer.buffer offset:vertexBuffer.offset atIndex:0];
        for (MTKSubmesh *submesh in mesh.submeshes) {
            auto indexBuffer = submesh.indexBuffer;
            [renderCommandEncoder drawIndexedPrimitives:submesh.primitiveType
                                             indexCount:submesh.indexCount
                                              indexType:submesh.indexType
                                            indexBuffer:indexBuffer.buffer
                                      indexBufferOffset:indexBuffer.offset];
        }
    }
    
    [renderCommandEncoder endEncoding];
}


simd::float4x4 Model3DStimulus::getCurrentMVPMatrix(const simd::float4x4 &projectionMatrix) const {
    auto currentMVPMatrix = ColoredTransformStimulus::getCurrentMVPMatrix(projectionMatrix);
    
    // The "model" MVP matrix produces Metal normalized device coordinates (-1 to 1 in x and y, 0 to 1
    // in z), whereas the MVP matrix returned by ColoredTransformStimulus::getCurrentMVPMatrix assumes
    // x and y are in the range [0,1] and z is in the range [-1,1].  Therefore, we need to translate
    // and scale all coordinates.  Also, since the "model" projection matrix flips from right-handed to
    // left-handed coordinates, we need to negate z to flip back.
    currentMVPMatrix *= matrix4x4_scale(0.5, 0.5, -2.0);
    currentMVPMatrix *= matrix4x4_translation(1.0, 1.0, -0.5);
    
    return currentMVPMatrix;
}


bool Model3DStimulus::updateParameters() {
    auto newFieldOfView = fieldOfView->getValue().getFloat();
    if ((newFieldOfView <= 0.0) || (newFieldOfView >= 180.0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Field of view must be greater than 0 and less than 180");
        return false;
    }
    
    auto newAmbientLightIntensity = ambientLightIntensity->getValue().getFloat();
    if ((newAmbientLightIntensity < 0.0) || (newAmbientLightIntensity > 1.0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Ambient light intensity must be between 0 and 1");
        return false;
    }
    
    currentFieldOfView = newFieldOfView;
    currentViewpointAltitude = viewpointAltitude->getValue().getFloat();
    currentViewpointAzimuth = viewpointAzimuth->getValue().getFloat();
    currentLightAltitude = lightAltitude->getValue().getFloat();
    currentLightAzimuth = lightAzimuth->getValue().getFloat();
    currentAmbientLightIntensity = newAmbientLightIntensity;
    
    return true;
}


simd::float4x4 Model3DStimulus::getModelMVPMatrix(MetalDisplay &display) const {
    double aspectRatio;
    if (!fullscreen) {
        aspectRatio = current_sizex / current_sizey;
    } else {
        auto &viewport = display.getCurrentMetalViewport();
        aspectRatio = viewport.width / viewport.height;
    }
    
    const auto distanceToBoundingSphereCenter = boundingSphereRadius / std::sin(radians_from_degrees(currentFieldOfView / 2.0));
    const auto nearZ = distanceToBoundingSphereCenter - boundingSphereRadius;
    const auto farZ = distanceToBoundingSphereCenter + boundingSphereRadius;
    
    //
    // We construct the viewing frustum so that the vertical field of view exactly
    // frames the bounding sphere.  When the aspect ratio is less than one, in order
    // for things to work correctly, we use the inverse of the actual aspect ratio
    // to create the perspective projection matrix.  Then, we rotate the eye
    // coordinates by 90 degrees before applying the projection matrix, and rotate
    // them back afterward.
    //
    const auto shouldRotateFrustum = (aspectRatio < 1.0);
    auto modelMVPMatrix = simd::float4x4(matrix_perspective_right_hand(radians_from_degrees(currentFieldOfView),
                                                                       (shouldRotateFrustum ? 1.0 / aspectRatio : aspectRatio),
                                                                       nearZ,
                                                                       farZ));
    if (shouldRotateFrustum) {
        modelMVPMatrix = matrix4x4_rotation(-std::numbers::pi / 2.0, 0.0, 0.0, 1.0) * modelMVPMatrix;
        modelMVPMatrix *= matrix4x4_rotation(std::numbers::pi / 2.0, 0.0, 0.0, 1.0);
    }
    
    modelMVPMatrix *= matrix_look_at_right_hand(simd::make_float3(0.0, 0.0, distanceToBoundingSphereCenter),
                                                simd::make_float3(0.0, 0.0, 0.0),
                                                simd::make_float3(0.0, 1.0, 0.0));
    
    modelMVPMatrix *= matrix4x4_rotation(radians_from_degrees(currentViewpointAltitude), 1.0, 0.0, 0.0);
    modelMVPMatrix *= matrix4x4_rotation(radians_from_degrees(currentViewpointAzimuth), 0.0, -1.0, 0.0);
    modelMVPMatrix *= matrix4x4_translation(-boundingSphereCenter);
    
    return modelMVPMatrix;
}


END_NAMESPACE_MW
