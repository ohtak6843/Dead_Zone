#pragma once
#include "GameObject.h"

class UIMgr
{
    DECLARE_SINGLE(UIMgr);
public:
    void Init();
    void Update();

    shared_ptr<GameObject> GetUI(const wstring& name);
    void InitTextFormats();

public:
    void CreateImageUI(const wstring& name, const wstring& texPath, const Vec2& pos, const Vec2& size, const float alpha, shared_ptr<class Scene> scene);
    void CreateRectangleUI(const wstring& name, const Vec2& pos, const Vec2& size,const Vec4 color, shared_ptr<class Scene> scene);
    void DrawTextUI(const wstring& text, const Vec2& pos, const Vec2& size, int fontSize,
        const D2D1::ColorF& textColor = D2D1::ColorF::White,
        const D2D1::ColorF& bgColor = D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

private:
    unordered_map<wstring, shared_ptr<GameObject>> _uiMap;
    unordered_map<int, ComPtr<IDWriteTextFormat>> _textFormats;
};

