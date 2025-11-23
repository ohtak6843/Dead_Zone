#pragma once
#include "CameraObject.h"

class TitleCamera : public CameraObject
{
public:
	TitleCamera();
	virtual ~TitleCamera();

	virtual void LateUpdate() override;

private:
	POINT _mousePos;
};

