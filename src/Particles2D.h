
#ifndef PARTICLES2D_H
#define PARTICLES2D_H

#include "SpriteBatcher.h"
#include "Vectors.h"
#include "Matrix.h"
#include "DArray.h"


struct Particle
{
    Vector3D pos;
    COLOR c;
    float size;
    Vector3D velocity;
    int age;
    bool isdead;

    Particle()
    {
        pos = Vector3D(0,0,0);
        c = COLOR(1.0f, 0.0f, 0.0f, 0.8f);
        velocity=Vector3D(0,0,0);
        age=0;
        size = 1.0f;
        isdead=false;
    }
};


class Particle2DSystem{

    DArray<Particle> particles;
    float centerx;
    float centery;
    float centerz;
    int particleLifetime;

    Vector3D mainVelocity;
    int halfangle;
    Vector3D begin;
    //float velmat[16];

    COLOR startColor;
    COLOR endColor;
    float startSize;
    float endSize;

    int maxSystemLifetime;
    int defSystemLifetime;
    bool _isDead;

public:
    Particle2DSystem(){
        _isDead = true;
        halfangle = 180;
        maxSystemLifetime = 20;
    }

    void setPos(float x, float y, float z);
    void setParticleLifetime(int lifetime);
    void setSystemLifetime(int lifetime);
    void setDirIntervals(Vector3D  vel, int hangle);
    void setColors(COLOR _start,
                   COLOR _end);
    void setSizes(float _start, float _end);
    void updateSystem();
    void drawParticles(SpriteBatcher& pics,
                       int picIndex, Vector3D shift = Vector3D(0,0,0));
    void destroy();
    bool isDead(){return _isDead;}
    void start(){_isDead = false;}
    void stop(){_isDead = true;}

};


#endif //PARTICLES2D_H
