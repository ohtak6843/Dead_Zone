#pragma once
#include "GameObject.h"

class GameObject;

class MonoBehaviour
{
public:
	MonoBehaviour();
	virtual ~MonoBehaviour();

public:
	virtual void Awake() {}
	virtual void Start() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void FinalUpdate() {}

public:
	shared_ptr<GameObject> GetGameObject() { return _gameObject.lock(); }
	shared_ptr<Transform> GetTransform() { return _gameObject.lock()->GetTransform(); }
	shared_ptr<Animator> GetAnimator() { return _gameObject.lock()->GetAnimator(); }

private:
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }
	weak_ptr<GameObject> _gameObject;

public:
	const wstring& GetName() { return _name; }

protected:
	wstring _name;
};

