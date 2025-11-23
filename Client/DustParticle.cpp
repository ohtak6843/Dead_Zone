#include "pch.h"
#include "DustParticle.h"
#include "Resources.h"
#include "Timer.h"
#include "UploadBuffer.h"

DustParticle::DustParticle()
{
	_particleBuffer = make_shared<UploadBuffer>();
	_particleBuffer->Init(sizeof(ParticleInfo), _maxParticle);

	_computeSharedBuffer = make_shared<UploadBuffer>();
	_computeSharedBuffer->Init(sizeof(ComputeSharedInfo), 1);

	// 파라미터 설정
	SetMaxParticle(100);
	SetLifeTime(2.0f, 2.0f);
	SetSpeed(100.f, 150.f);
	SetScale(100.f, 50.f);
	SetEmitterLifeTime(1.5f);
	SetCreateInterval(0.005f);
	SetParticleType(PARTICLE_TYPE::DUST);


	// 텍스처 설정
	shared_ptr<Texture> tex = GET_SINGLE(Resources)->Load<Texture>(
		L"Dust", L"..\\Resources\\Texture\\Particle\\Dust.png");
	SetTexture(_type, tex);
}

void DustParticle::FinalUpdate()
{
	_elapsedTime += DELTA_TIME;

	int32 add = 0;
	if (_isActive && _lifeTime > 0.0f && _elapsedTime >= _lifeTime)
	{
		Reset();
	}

	if (_isActive)
	{
		_accTime += DELTA_TIME;

		if (_createInterval < _accTime)
		{
			_accTime -= _createInterval;
			add = 3;
		}
	}

	_particleBuffer->PushComputeUAVData(UAV_REGISTER::u0);
	_computeSharedBuffer->PushComputeUAVData(UAV_REGISTER::u1);

	_computeMaterial->SetInt(0, _maxParticle);
	_computeMaterial->SetInt(1, add);
	_computeMaterial->SetInt(2, _type);

	_computeMaterial->SetVec2(1, Vec2(DELTA_TIME, _accTime));
	_computeMaterial->SetVec4(0, Vec4(_minLifeTime, _maxLifeTime, _minSpeed, _maxSpeed));


	_computeMaterial->Dispatch(1, 1, 1);
}
