#pragma once
#include "MonoBehaviour.h"


class TP_AK47 : public MonoBehaviour
{
public:
	TP_AK47();
	virtual ~TP_AK47();

	virtual void LateUpdate() override;

public:
	void SetParentObject(shared_ptr<class GameObject> parentObject)
	{
		_parentObject = parentObject;
	}

private:
	shared_ptr<class GameObject> _parentObject;
	
	Vec3 _baseScale = { 0.2f, 0.2f, 0.2f };
	Vec3 _baseRotation = { -20.0f, 90.0f, 90.0f };
	Vec3 _basePosition = { 3.0f, 5.0f, 25.0f };
};

