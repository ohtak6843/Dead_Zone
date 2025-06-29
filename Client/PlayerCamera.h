#pragma once
#include "CameraObject.h"

class PlayerCamera : public CameraObject
{
public:
	PlayerCamera();
	virtual ~PlayerCamera();

public:
	virtual void LateUpdate() override;
};

