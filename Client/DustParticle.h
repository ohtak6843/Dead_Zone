#pragma once
#include "Particle.h"

class DustParticle : public Particle
{
public:
	DustParticle();
	virtual ~DustParticle() = default;

	virtual void FinalUpdate() override;
};

