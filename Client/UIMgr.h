#pragma once
#include "GameObject.h"

class UIMgr
{
    DECLARE_SINGLE(UIMgr);
public:
    void Init();
    void Update();

    void CreateImageUI(const wstring& name, const wstring& texPath, const Vec2& pos, const Vec2& size, shared_ptr<class Scene> scene);
    shared_ptr<GameObject> GetUI(const wstring& name);

private:
    unordered_map<wstring, shared_ptr<GameObject>> _uiMap;
};

