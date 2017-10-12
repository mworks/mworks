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


class WhiteNoiseBackground : public Stimulus, boost::noncopyable {

public:
    static const std::string RAND_SEED;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit WhiteNoiseBackground(const ParameterValueMap &parameters);
    
    void load(shared_ptr<StimulusDisplay> display) override;
    void unload(shared_ptr<StimulusDisplay> display) override;
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
    void randomize();
    
private:
    static constexpr GLint numVertices = 4;
    static constexpr GLint componentsPerVertex = 2;
    using VertexPositionArray = std::array<GLfloat, numVertices*componentsPerVertex>;
    
    void createProgram(GLuint &program, const gl::Shader &vertexShader, const std::string &fragmentShaderSource);
    void createTexture(GLuint &texture, const std::vector<GLuint> &data);
    void runProgram(GLuint program, GLuint texture);
    
    static const std::string vertexShaderSource;
    static const std::string sharedFragmentShaderSource;
    static const std::string noiseGenFragmentShaderSource;
    static const std::string noiseRenderFragmentShaderSource;
    static const VertexPositionArray vertexPositions;
    static const VertexPositionArray texCoords;
    
    MWTime randSeed;
    std::size_t randCount;
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool shouldRandomize;
    
    GLint width, height;

    GLuint vertexPositionBuffer = 0;
    GLuint texCoordsBuffer = 0;
    GLuint noiseGenProgram = 0;
    GLuint noiseRenderProgram = 0;
    std::map<GLuint, GLuint> vertexArrays;  // Maps program to vertex array
    GLuint seedTexture = 0;
    GLuint noiseTexture = 0;
    std::map<GLuint, GLuint> framebuffers;  // Maps texture to framebuffer
    
};


END_NAMESPACE_MW


#endif






















