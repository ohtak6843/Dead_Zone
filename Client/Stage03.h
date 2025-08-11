#pragma once
#include "Scene.h"


class Stage03 : public Scene
{
public:
	Stage03();
	virtual ~Stage03();
	
	virtual void LoadResources() override;

	virtual void Init() override;
};

