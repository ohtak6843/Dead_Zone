#pragma once
#include "Object.h"

class Transform;
class MeshRenderer;
class Camera;
class Light;
class MonoBehaviour;
class Particle;
class Terrain;
class BaseCollider;
class Animator;

class GameObject : public Object, public enable_shared_from_this<GameObject>
{
public:
	GameObject();
	virtual ~GameObject();

	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void FinalUpdate();

	shared_ptr<Transform> GetTransform() { return _transform; }
	shared_ptr<MeshRenderer> GetMeshRenderer() { return _meshRenderer; }
	shared_ptr<Camera> GetCamera() { return _camera; }
	shared_ptr<Light> GetLight() { return _light; }
	shared_ptr<Particle> GetParticle() { return _particle; }
	shared_ptr<BaseCollider> GetCollider() { return _collider; }
	shared_ptr<Animator> GetAnimator() { return _animator; }
	shared_ptr<MonoBehaviour> GetMonoBehaviour(const wstring& name);

	void SetTransform(shared_ptr<Transform> transform);
	void SetMeshRenderer(shared_ptr<MeshRenderer> meshRenderer);
	void SetCamera(shared_ptr<Camera> camera);
	void SetLight(shared_ptr<Light> light);
	void SetParticle(shared_ptr<Particle> particle);
	void SetCollider(shared_ptr<BaseCollider> collider);
	void SetAnimator(shared_ptr<Animator> animator);

	void AddScript(shared_ptr<MonoBehaviour> script);

	void SetCheckFrustum(bool checkFrustum) { _checkFrustum = checkFrustum; }
	bool GetCheckFrustum() { return _checkFrustum; }

	void SetLayerIndex(uint8 layer) { _layerIndex = layer; }
	uint8 GetLayerIndex() { return _layerIndex; }

	void SetStatic(bool flag) { _static = flag; }
	bool IsStatic() { return _static; }

	// 업데이트 및 랜더링 여부
	void SetActive(bool active) { _isActive = active; }
	bool IsActive() const { return _isActive; }

private:
	shared_ptr<Transform> _transform;
	shared_ptr<MeshRenderer> _meshRenderer;
	shared_ptr<Camera> _camera;
	shared_ptr<Light> _light;
	shared_ptr<Particle> _particle;
	shared_ptr<BaseCollider> _collider;
	shared_ptr<Animator> _animator;
	vector<shared_ptr<MonoBehaviour>> _scripts;

	bool _isActive = true;
	bool _checkFrustum = true;
	uint8 _layerIndex = 0;
	bool _static = false;
};

