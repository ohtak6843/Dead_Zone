#include "pch.h"
#include "UIMgr.h"

#include "pch.h"
#include "UIMgr.h"
#include "Resources.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "SceneMgr.h"
#include "Scene.h"

void UIMgr::Init()
{
    // 필요 시 초기 UI 생성
}

void UIMgr::Update()
{
    // UI 동적 업데이트 (예: 체력 수치 표시 등)
}

void UIMgr::CreateImageUI(const wstring& name, const wstring& texPath, const Vec2& pos, const Vec2& size, shared_ptr<Scene> scene = nullptr)
{
    if (_uiMap.contains(name))
        return;

    shared_ptr<GameObject> UI = make_shared<GameObject>();
    UI->SetLayerIndex(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI"));
    UI->SetTransform(make_shared<Transform>());
    UI->GetTransform()->SetLocalPosition(Vec3(pos.x, pos.y, 500.f)); // UI는 z를 충분히 앞으로
    UI->GetTransform()->SetLocalScale(Vec3(size.x, size.y, 1.f));

    shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
    meshRenderer->SetMesh(GET_SINGLE(Resources)->LoadRectangleMesh());

    shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"AlphaTexture");
    shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(name, texPath);

    shared_ptr<Material> material = make_shared<Material>();
    material->SetShader(shader);
    material->SetTexture(0, texture);
    meshRenderer->SetMaterial(material);

	UI->SetName(name);
    UI->SetMeshRenderer(meshRenderer);
    _uiMap.insert({ name, UI });

	if (scene == nullptr)
		scene = GET_SINGLE(SceneMgr)->GetActiveScene();
    scene->AddGameObject(UI);
}

shared_ptr<GameObject> UIMgr::GetUI(const wstring& name)
{
    if (_uiMap.contains(name))
        return _uiMap[name];
    return nullptr;
}
