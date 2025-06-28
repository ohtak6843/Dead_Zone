#include "pch.h"
#include "ParticleObject.h"
#include "Transform.h"
#include "Particle.h"
#include "BaseCollider.h"
#include "Animator.h"

ParticleObject::ParticleObject()
{
	_type = GAMEOBJECT_TYPE::PARTICLE;
}

ParticleObject::~ParticleObject()
{
}

void ParticleObject::FinalUpdate()
{
	if (_transform != nullptr)
		_transform->FinalUpdate();

	if (_particle != nullptr)
		_particle->FinalUpdate();

	if (_collider != nullptr)
		_collider->FinalUpdate();

	if (_animator != nullptr)
		_animator->FinalUpdate();
}

void ParticleObject::SetParticle(shared_ptr<Particle> particle)
{
	particle->SetGameObject(shared_from_this());
	_particle = particle;
}