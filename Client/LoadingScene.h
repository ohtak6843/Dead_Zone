#pragma once
#include "Scene.h"
#include "SceneMgr.h"

class LoadingScene : public Scene
{
public:
    LoadingScene();
    virtual ~LoadingScene();

    virtual void Init() override;
};