#pragma once
#include "Particle.h"

class MuzzleFlashParticle : public Particle
{
public:
	MuzzleFlashParticle();
	virtual ~MuzzleFlashParticle() = default;

	virtual void FinalUpdate() override;

private:
};