#include "pch.h"
#include "UIMgr.h"

#include "pch.h"
#include "UIMgr.h"
#include "Resources.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "SceneMgr.h"
#include "Scene.h"
#include "framework.h"

void UIMgr::Init()
{
    // 필요 시 초기 UI 생성
    InitTextFormats();
}

void UIMgr::Update()
{
    // UI 동적 업데이트


}

void UIMgr::CreateImageUI(const wstring& name, const wstring& texPath, const Vec2& pos, const Vec2& size, shared_ptr<Scene> scene = nullptr)
{
    if (_uiMap.contains(name))
        return;

    shared_ptr<GameObject> UI = make_shared<GameObject>();
    UI->SetLayerIndex(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI"));
    UI->SetTransform(make_shared<Transform>());
    UI->GetTransform()->SetLocalPosition(Vec3(pos.x, pos.y, 100.f));
    UI->GetTransform()->SetLocalScale(Vec3(size.x, size.y, 1.f));

    shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
    meshRenderer->SetMesh(GET_SINGLE(Resources)->LoadRectangleMesh());

    shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"AlphaTexture");
    shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(name, texPath);

    shared_ptr<Material> material = make_shared<Material>();
    material->SetShader(shader);
    material->SetTexture(0, texture);
    material->SetColor(0, Vec4(1.f, 1.f, 1.f, 0.5f)); // 반투명
    meshRenderer->SetMaterial(material);

	UI->SetName(name);
    UI->SetMeshRenderer(meshRenderer);
    _uiMap.insert({ name, UI });

	if (scene == nullptr)
		scene = GET_SINGLE(SceneMgr)->GetActiveScene();
    scene->AddGameObject(UI);
}

void UIMgr::CreateRectangleUI(const wstring& name, const Vec2& pos, const Vec2& size, const Vec4 color, shared_ptr<class Scene> scene = nullptr)
{
    if (_uiMap.contains(name))
        return;

    shared_ptr<GameObject> panel= make_shared<GameObject>();
    panel->SetLayerIndex(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI"));
    panel->SetTransform(make_shared<Transform>());
    panel->GetTransform()->SetLocalPosition(Vec3(pos.x, pos.y, 100.f));
    panel->GetTransform()->SetLocalScale(Vec3(size.x, size.y, 1.f));

    shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
    meshRenderer->SetMesh(GET_SINGLE(Resources)->LoadRectangleMesh());

    shared_ptr<Material> material = make_shared<Material>();
    material->SetShader(GET_SINGLE(Resources)->Get<Shader>(L"AlphaTexture"));
    material->SetColor(0, color);
    meshRenderer->SetMaterial(material);

    panel->SetName(name);
    panel->SetMeshRenderer(meshRenderer);
    _uiMap.insert({ name, panel });

    if (scene == nullptr)
        scene = GET_SINGLE(SceneMgr)->GetActiveScene();
    scene->AddGameObject(panel);
}

void UIMgr::DrawTextUI(const wstring& text, const Vec2& pos, const Vec2& size, int fontSize = 32,
    const D2D1::ColorF& textColor, const D2D1::ColorF& bgColor)
{
    shared_ptr<D3D11On12Device> device = gameFramework->GetD3D11on12Device();
    auto ctx = device->GetD2DDeviceContext();
    auto brush = device->GetSolidColorBrush();

    D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);

    // 배경
    brush->SetColor(bgColor);
    ctx->FillRectangle(&rect, brush.Get());

    // 텍스트
    brush->SetColor(textColor);
    auto format = _textFormats.count(fontSize) ? _textFormats[fontSize].Get() : device->GetTextFormat().Get();
    ctx->DrawTextW(
        text.c_str(),
        static_cast<UINT32>(text.length()),
        format,
        &rect,
        brush.Get()
    );
}

void UIMgr::InitTextFormats()
{
    ComPtr<IDWriteFactory> writeFactory = gameFramework->GetD3D11on12Device()->GetDWriteFactory();

    for (int fontSize = 8; fontSize <= 128; fontSize += 2)
    {
        ComPtr<IDWriteTextFormat> format;
        writeFactory->CreateTextFormat(
            L"Verdana", nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<FLOAT>(fontSize),
            L"ko-kr",
            &format
        );

        _textFormats[fontSize] = format;
    }
}

shared_ptr<GameObject> UIMgr::GetUI(const wstring& name)
{
    if (_uiMap.contains(name))
        return _uiMap[name];
    return nullptr;
}
