#pragma once
#include "GameObject.h"

class CameraObject : public GameObject
{
public:
	CameraObject();
	virtual ~CameraObject();

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void FinalUpdate() override;

public:
	shared_ptr<Camera> GetCamera() { return _camera; }
	void SetCamera(shared_ptr<Camera> camera);

protected:
	shared_ptr<Camera> _camera;
};

