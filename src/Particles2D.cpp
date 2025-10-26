

#include "Particles2D.h"
#include <cstdlib>

void Particle2DSystem::drawParticles(SpriteBatcher& pics,
                                     int picIndex, Vector3D shift)
{

    for (unsigned long i = 0; i < particles.count(); i++)
    {
        Particle* p = 0;
        p = &particles[i];

        if (!p->isdead)
        {
            float _x = p->pos.x + shift.x;
            float _y = p->pos.z + shift.z;

            pics.draw(picIndex,
                       _x,
                       _y,
                       0,
                       true,
                       p->size,
                       p->size,
                       0,
                       p->c,
                       p->c);
        }
    }

}
//----------------------------------------------

void Particle2DSystem::setParticleLifetime(int lifetime)
{
    particleLifetime = lifetime;
}

//----------------------------------------------
void Particle2DSystem::setPos(float x, float y, float z)
{
    centerx=x;
    centery=y;
    centerz=z;
}
//----------------------------------------------
void Particle2DSystem::setDirIntervals(Vector3D vel, int hangle)
{
    mainVelocity = vel;
    halfangle = hangle;
    Matrix mat;
    MatrixRotationY(-(hangle/2.0f)*0.0175f, mat);
    begin = vel;
    begin.transform(mat);
}

//----------------------------------------------
void Particle2DSystem::updateSystem()
{

    for (unsigned long i = 0; i < particles.count(); i++)
    {
        Particle* p=0;
        p = &particles[i];
    
        if (p)
        {
            if (!p->isdead)
            {
                p->age++;
                p->pos = Vector3D(p->pos.x + p->velocity.x,
                                  p->pos.y + p->velocity.y,
                                  p->pos.z + p->velocity.z );

                float colorchange = 1.0f / particleLifetime;

                float sizeChange = (fabs(endSize - startSize))/particleLifetime;

                if (p->size < endSize)
                    p->size += sizeChange;
                if (p->size > endSize)
                    p->size -= sizeChange;

                if (endColor.c[3] < p->c.c[3])
                    p->c.c[3] -= colorchange;
                else
                    p->c.c[3] += colorchange;

                if (endColor.c[0] < p->c.c[0])
                    p->c.c[0] -= colorchange;
                else
                    p->c.c[0] += colorchange;

                if (endColor.c[1] < p->c.c[1])
                    p->c.c[1] -= colorchange;
                else
                    p->c.c[1] += colorchange;

                if (endColor.c[2] < p->c.c[2])
                    p->c.c[2] -= colorchange;
                else
                    p->c.c[2] += colorchange;

                if (p->age >= particleLifetime)
                    p->isdead=true;
            }
        }
    }

    if (particles.count())
    {
    for (long i = particles.count(); i >= 0; --i)
    {
        if (particles[i].isdead)
        {
            particles.remove(i);
        }
    }
    }


    if (maxSystemLifetime > 0)
    {
        ++defSystemLifetime;

        if (defSystemLifetime >= maxSystemLifetime)
        {
            _isDead = true;
        }
    }

    if (!_isDead)
    {
        Particle np;
        np.pos = Vector3D(centerx, centery, centerz);
        np.c = startColor;
        np.size = startSize;
        int ang = rand() % halfangle + 1;

        float angle = 0.175f * ang;
        //printf("%f\n",angle);
        float a = cosf(angle);
        float b = sinf(angle);
        np.velocity = Vector3D(begin.v[0] * a + begin.v[2] * b,
                               0,
                               begin.v[0] * -b + begin.v[2] * a);
        np.velocity.normalize();

        particles.add(np);
    }

}

//------------------------------------
void Particle2DSystem::destroy(){
    particles.destroy();
    _isDead = true;
}
//---------------------------------------
void Particle2DSystem::setColors(COLOR _start,
                                 COLOR _end){
    startColor = _start;
    endColor = _end;
}
//-----------------------------
void Particle2DSystem::setSizes(float _start, float _end){
    startSize = _start;
    endSize = _end;
}

//----------------------------------------
void Particle2DSystem::setSystemLifetime(int lifetime)
{
    maxSystemLifetime = lifetime;
    defSystemLifetime = 0;
}

