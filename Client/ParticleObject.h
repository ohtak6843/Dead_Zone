#pragma once
#include "GameObject.h"

class Particle;

class ParticleObject : public GameObject
{
public:
	ParticleObject();
	virtual ~ParticleObject();

public:
	virtual void FinalUpdate() override;

public:
	shared_ptr<Particle> GetParticle() { return _particle; }
	void SetParticle(shared_ptr<Particle> particle);

protected:
	shared_ptr<Particle> _particle;

};

