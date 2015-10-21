/*
 *  MovingDots.h
 *  MovingDots
 *
 *  Created by Christopher Stawarz on 8/6/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#ifndef MovingDots_H_
#define MovingDots_H_

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>


BEGIN_NAMESPACE_MW


class MovingDots : public StandardDynamicStimulus {

public:
    static const std::string FIELD_RADIUS;
    static const std::string FIELD_CENTER_X;
    static const std::string FIELD_CENTER_Y;
    static const std::string DOT_DENSITY;
    static const std::string DOT_SIZE;
    static const std::string COLOR;
    static const std::string ALPHA_MULTIPLIER;
    static const std::string DIRECTION;
    static const std::string SPEED;
    static const std::string COHERENCE;
    static const std::string LIFETIME;
    static const std::string ANNOUNCE_DOTS;
    
    static void describeComponent(ComponentInfo &info);

    explicit MovingDots(const ParameterValueMap &parameters);
    ~MovingDots() { }
    
    void load(shared_ptr<StimulusDisplay> display) MW_OVERRIDE;
    void drawFrame(shared_ptr<StimulusDisplay> display) MW_OVERRIDE;
    Datum getCurrentAnnounceDrawData() MW_OVERRIDE;
   
protected:
    void stopPlaying() MW_OVERRIDE;
    
private:
    void updateParameters();
    void updateDots();
    void advanceDot(GLint i, GLfloat dt, GLfloat dr);
    void replaceDot(GLint i, GLfloat direction, GLfloat age);
    
    GLfloat rand(GLfloat min, GLfloat max) {
        const boost::uniform_real<GLfloat> randDist(min, max);
        boost::variate_generator< boost::mt19937&, boost::uniform_real<GLfloat> > randVar(randGen, randDist);
        return randVar();
    }
    
    GLfloat& getX(GLint i) { return dotPositions[i*verticesPerDot]; }
    GLfloat& getY(GLint i) { return dotPositions[i*verticesPerDot + 1]; }
    GLfloat& getDirection(GLint i) { return dotDirections[i]; }
    GLfloat& getAge(GLint i) { return dotAges[i]; }
    
    GLfloat newDirection(GLfloat coherence) {
        if ((coherence == 0.0f) || ((coherence != 1.0f) && (rand(0.0f, 1.0f) > coherence))) {
            return rand(0.0f, 360.0f);
        }
        return 0.0f;
    }
    
    GLfloat newAge(GLfloat lifetime) {
        if (lifetime != 0.0f) {
            return rand(0.0f, lifetime);
        }
        return 0.0f;
    }
    
    shared_ptr<Variable> fieldRadius;
    shared_ptr<Variable> fieldCenterX;
    shared_ptr<Variable> fieldCenterY;
    shared_ptr<Variable> dotDensity;
    shared_ptr<Variable> dotSize;
    shared_ptr<Variable> red;
    shared_ptr<Variable> green;
    shared_ptr<Variable> blue;
    shared_ptr<Variable> alpha;
    shared_ptr<Variable> direction;
    shared_ptr<Variable> speed;
    shared_ptr<Variable> coherence;
    shared_ptr<Variable> lifetime;
    shared_ptr<Variable> announceDots;
    
    GLfloat previousFieldRadius, currentFieldRadius;
    GLint previousNumDots, currentNumDots;
    GLfloat previousSpeed, currentSpeed;
    GLfloat previousCoherence, currentCoherence;
    GLfloat previousLifetime, currentLifetime;
    
    std::vector<GLfloat> dotSizeToPixels;
    static const GLint verticesPerDot = 2;
    std::vector<GLfloat> dotPositions;
    std::vector<GLfloat> dotDirections;
    std::vector<GLfloat> dotAges;
    
    boost::mt19937 randGen;
    
    MWTime previousTime, currentTime;
    
};


END_NAMESPACE_MW


#endif 























