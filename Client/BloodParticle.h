#pragma once
#include "Particle.h"

class BloodParticle : public Particle
{
public:
	BloodParticle();
	virtual ~BloodParticle() = default;

	virtual void FinalUpdate() override;
};

