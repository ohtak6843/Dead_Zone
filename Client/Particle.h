#pragma once
#include "GameObject.h"
#include "Material.h"

class GameObject;
class Transform;
class Material;
class Mesh;
class UploadBuffer;
class Texture;

struct ParticleInfo
{
	Vec3	worldPos;
	float	curTime;
	Vec3	worldDir;
	float	lifeTime;
	int32	alive;
	int32   type;
	int32	padding[3];
};

enum PARTICLE_TYPE
{
	DEFAULT = 0,
	MUZZLE_FLASH,    // 머즐 플래시
	BLOOD,          // 피
};


struct ComputeSharedInfo
{
	int32 addCount;
	int32 padding[3];
};

class Particle
{
public:
	Particle();
	virtual ~Particle();

public:
	virtual void FinalUpdate();
	void Render();

public:

	void SetMaxParticle(uint32 max) { _maxParticle = max; }
	void SetLifeTime(float min, float max) { _minLifeTime = min; _maxLifeTime = max; }
	void SetSpeed(float min, float max) { _minSpeed = min; _maxSpeed = max; }
	void SetScale(float start, float end) { _startScale = start; _endScale = end; }
	void SetCreateInterval(float interval) { _createInterval = interval; }
	void SetTexture(int32 index, shared_ptr<Texture> tex) { _material->SetTexture(index, tex); }
	void SetlifeTime(float lifeTime) { _lifeTime = lifeTime; }
	void SetParticleType(int32 type) { _type = type; }

	void SetActive(bool active) { _isActive = active; _elapsedTime = 0.0f; _accTime = 0.f; }

public:
	shared_ptr<GameObject> GetGameObject() { return _gameObject.lock(); }
	shared_ptr<Transform> GetTransform() { return GetGameObject()->GetTransform(); }

private:
	friend class ParticleObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }
	weak_ptr<GameObject> _gameObject;

protected:
	shared_ptr<UploadBuffer>	_particleBuffer;
	shared_ptr<UploadBuffer>	_computeSharedBuffer;
	uint32							_maxParticle = 100;

	shared_ptr<Material>		_computeMaterial;
	shared_ptr<Material>		_material;
	shared_ptr<Mesh>			_mesh;

	// 타입
	int32				_type = PARTICLE_TYPE::DEFAULT;

	// 파티클 활성 상태
	bool            _isActive = false;

	// 누적 시간
	float				_accTime = 0.f;
	// 지속 시간 
	float				_lifeTime = 1.0f;
	// 경과 시간
	float				_elapsedTime = 0.f;

	// 파티클 생성 간격
	float				_createInterval = 0.005f;

	// 수명
	float				_minLifeTime = 0.5f;
	float				_maxLifeTime = 1.f;

	// 속도
	float				_minSpeed = 100;
	float				_maxSpeed = 50;

	// 크기
	float				_startScale = 10.f;
	float				_endScale = 5.f;
};
