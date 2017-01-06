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
    static void describeComponent(ComponentInfo &info);
    
    explicit WhiteNoiseBackground(const ParameterValueMap &parameters);
    
    void load(shared_ptr<StimulusDisplay> display) override;
    void unload(shared_ptr<StimulusDisplay> display) override;
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
    void randomizePixels();
    
private:
    static constexpr GLint numVertices = 4;
    static constexpr GLint componentsPerVertex = 2;
    static constexpr GLint componentsPerPixel = 4;
    using VertexPositionArray = std::array<GLfloat, numVertices*componentsPerVertex>;
    
    void updateTexture();
    
    static const std::string vertexShaderSource;
    static const std::string fragmentShaderSource;
    static const VertexPositionArray vertexPositions;
    static const VertexPositionArray texCoords;
    
    GLint width, height;

    std::vector<GLuint> pixels;
    boost::mutex pixelsMutex;

    boost::rand48 randGen;
    boost::uniform_int<GLubyte> randDist;
    
    GLuint program = 0;
    GLuint vertexArray = 0;
    GLuint vertexPositionBuffer = 0;
    GLuint texCoordsBuffer = 0;
    GLuint texture = 0;

};


END_NAMESPACE_MW


#endif






















