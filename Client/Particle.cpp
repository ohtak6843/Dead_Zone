#include "pch.h"
#include "Particle.h"
#include "GameObject.h"
#include "UploadBuffer.h"
#include "Mesh.h"
#include "Resources.h"
#include "Transform.h"
#include "Timer.h"

Particle::Particle()
{
	_particleBuffer = make_shared<UploadBuffer>();
	_particleBuffer->Init(sizeof(ParticleInfo), _maxParticle);

	_computeSharedBuffer = make_shared<UploadBuffer>();
	_computeSharedBuffer->Init(sizeof(ComputeSharedInfo), 1);

	_mesh = GET_SINGLE(Resources)->LoadPointMesh();
	_material = GET_SINGLE(Resources)->Get<Material>(L"Particle");
	shared_ptr<Texture> tex = GET_SINGLE(Resources)->Load<Texture>(
		L"Bubbles", L"..\\Resources\\Texture\\Particle\\bubble.png");

	_material->SetTexture(_type, tex);

	_computeMaterial = GET_SINGLE(Resources)->Get<Material>(L"ComputeParticle");
}

Particle::~Particle()
{
}

void Particle::FinalUpdate()
{
	_elapsedTime += DELTA_TIME;

	int32 add = 0;
	if (_isActive && _lifeTime > 0.0f && _elapsedTime >= _lifeTime)
	{
		_isActive = false;
	}

	if (_isActive)
	{
		_accTime += DELTA_TIME;

		if (_createInterval < _accTime)
		{
			_accTime -= _createInterval;
			add = 1;
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

void Particle::Render()
{
	if (_accTime >= (_lifeTime + _maxLifeTime))
		return;

	GetTransform()->PushData();

	_particleBuffer->PushGraphicsData(SRV_REGISTER::t9);
	_material->SetFloat(0, _startScale);
	_material->SetFloat(1, _endScale);
	_material->PushGraphicsData();

	_mesh->Render(_maxParticle);
}

void Particle::Reset()
{
	_isActive = false;
	_elapsedTime = 0.0f;
	_accTime = 0.f;

	if (_computeSharedBuffer)
	{
		ComputeSharedInfo init{};              // 모두 0
		_computeSharedBuffer->CopyData(0, 1, &init);
	}
	// 3) 파티클 배열 초기화 (alive=0)
	if (_particleBuffer)
	{
		_particleBuffer->Clear(); // 전체 0으로: worldPos/dir/curTime/lifeTime/alive 모두 0
	}
}
