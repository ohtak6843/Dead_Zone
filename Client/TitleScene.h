#pragma once
#include "Scene.h"


class TitleScene : public Scene
{
public:
	TitleScene();
	virtual ~TitleScene();

	virtual void Init() override;
};

