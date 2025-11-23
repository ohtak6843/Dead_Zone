#pragma once
#include "Scene.h"

class Stage01 : public Scene
{
public:
    Stage01();
	virtual ~Stage01();

    virtual void LoadResources() override;

    virtual void Init() override;
};