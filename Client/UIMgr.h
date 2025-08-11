#pragma once
#include "GameObject.h"
#include "UIObject.h"

enum class HAlign { Left, Center, Right };
static DWRITE_TEXT_ALIGNMENT ToDW(HAlign a) {
    switch (a) {
    case HAlign::Center: return DWRITE_TEXT_ALIGNMENT_CENTER;
    case HAlign::Right:  return DWRITE_TEXT_ALIGNMENT_TRAILING;
    default:             return DWRITE_TEXT_ALIGNMENT_LEADING; // Left
    }
}

class UIMgr
{
    DECLARE_SINGLE(UIMgr);
public:
    void Init();
    void Update();
    void InitTextFormats();

public:
    void CreateImageUI(const wstring& name, const wstring& texPath, const Vec2& pos, const Vec2& size, const float alpha,
        bool active = true, int32 uiType = static_cast<int32>(UI_TYPE::DEFAULT), shared_ptr<class Scene> scene = nullptr);

    void CreateRectangleUI(const wstring& name, const Vec2& pos, const Vec2& size,const Vec4 color,
        bool active = true, int32 uiType = static_cast<int32>(UI_TYPE::DEFAULT), shared_ptr<class Scene> scene = nullptr);

    void DrawTextUI(const wstring& text, const Vec2& pos, const Vec2& size,
        int fontSize = 32,
        const D2D1::ColorF& textColor = D2D1::ColorF::White,
        const D2D1::ColorF& bgColor = D2D1::ColorF(0.f, 0.f, 0.f, 0.f),
        HAlign hAlign = HAlign::Left);

	void ClearUI() { _uiMap.clear(); }

public:
	void LoadUIImage(shared_ptr<class Scene> scene);
    void RenderUI();
    void RenderPlayerUI(const long long id, const shared_ptr<class Player>& player, const int32 index);

public:
    shared_ptr<UIObject> GetUI(const wstring& name);
	shared_ptr<UIObject> SelectUI(POINT pos);

private:
    unordered_map<wstring, shared_ptr<UIObject>> _uiMap;
    unordered_map<int, ComPtr<IDWriteTextFormat>> _textFormats;
};

