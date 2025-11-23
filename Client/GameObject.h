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

enum class GAMEOBJECT_TYPE
{
	DEFAULT,
	CAMERA,
	LIGHT,
	PARTICLE,
	PLAYER,
	ZOMBIE,
	UI,

	END
};

class GameObject : public Object, public enable_shared_from_this<GameObject>
{
public:
	GameObject();
	virtual ~GameObject();

public:
	GameObject& operator=(const GameObject& other);

	virtual void Awake() {}
	virtual void Start() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void FinalUpdate();

	shared_ptr<Transform> GetTransform() { return _transform; }
	shared_ptr<MeshRenderer> GetMeshRenderer() { return _meshRenderer; }
	shared_ptr<BaseCollider> GetCollider() { return _collider; }
	shared_ptr<Animator> GetAnimator() { return _animator; }

	void SetTransform(shared_ptr<Transform> transform);
	void SetMeshRenderer(shared_ptr<MeshRenderer> meshRenderer);
	void SetCollider(shared_ptr<BaseCollider> collider);
	void SetAnimator(shared_ptr<Animator> animator);

	GAMEOBJECT_TYPE GetGameObjectType() const { return _type; }

	void SetCheckFrustum(bool checkFrustum) { _checkFrustum = checkFrustum; }
	bool GetCheckFrustum() { return _checkFrustum; }

	void SetLayerIndex(uint8 layer) { _layerIndex = layer; }
	uint8 GetLayerIndex() { return _layerIndex; }

	void SetStatic(bool flag) { _static = flag; }
	bool IsStatic() { return _static; }

	// 업데이트 및 랜더링 여부
	void SetActive(bool active) { _isActive = active; }
	bool IsActive() const { return _isActive; }

protected:
	GAMEOBJECT_TYPE _type;

	shared_ptr<Transform> _transform;
	shared_ptr<MeshRenderer> _meshRenderer;
	shared_ptr<BaseCollider> _collider;
	shared_ptr<Animator> _animator;

	bool _isActive = true;
	bool _checkFrustum = true;
	uint8 _layerIndex = 0;
	bool _static = false;
};

