#pragma once
#include "Scene.h"

class Stage02 : public Scene
{
public:
	Stage02();
	virtual ~Stage02();

	virtual void LoadResources() override;

	virtual void Init() override;
};
