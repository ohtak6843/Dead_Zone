#pragma once
#include "GameObject.h"

class Light;

class LightObject : public GameObject
{
public:
	LightObject();
	virtual ~LightObject();

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void FinalUpdate() override;

public:
	shared_ptr<Light> GetLight() { return _light; }
	void SetLight(shared_ptr<Light> light);

protected:
	shared_ptr<Light> _light;
};

