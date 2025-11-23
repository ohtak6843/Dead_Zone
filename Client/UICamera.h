#pragma once
#include "CameraObject.h"

class UICamera : public CameraObject
{
public:
	UICamera();
	virtual ~UICamera();

	virtual void LateUpdate() override;
};

