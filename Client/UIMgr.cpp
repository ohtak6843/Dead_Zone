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
#include "Player.h"
#include "LocalPlayer.h"
#include "M4A1.h"
#include "AK47.h"
#include "Client.h"
#include "UIObject.h"

void UIMgr::Init()
{
    // 필요 시 초기 UI 생성
    InitTextFormats();
}

void UIMgr::Update()
{
    // UI 동적 업데이트
}

void UIMgr::RenderPlayerUI(const long long id, const shared_ptr<Player>& player, const int32 index)
{
	if (!player) return;

	std::wstringstream wss;
	// 플레이어 ID
	wss.str(L"");
	wss.clear();
	wss << id;
	DrawTextUI(
		wss.str(),
		Vec2(35.f, 738.f - (60.f * index)),
		Vec2(300.f, 100.f),
		16,
		D2D1::ColorF::White,
		D2D1::ColorF(0, 0, 0, 0.0f)
	);

	// 플레이어 체력
	uint32 hp = player->GetHp();
	uint32 maxHp = player->GetMaxHp();
	wss.str(L"");
	wss.clear();
	wss << hp << " / " << maxHp;
	DrawTextUI(
		wss.str(),
		Vec2(0.f, 760.f - (60.f * index)),
		Vec2(300.f, 100.f),
		16,
		D2D1::ColorF::White,
		D2D1::ColorF(0, 0, 0, 0.0f),
		HAlign::Center
	);

	// 플레이어 공격력
	uint32 AD = player->GetAttackDamage();
	wss.str(L"");
	wss.clear();
	wss << AD;
	DrawTextUI(
		wss.str(),
		Vec2(115.f, 738.f - (60.f * index)),
		Vec2(300.f, 100.f),
		16,
		D2D1::ColorF::White,
		D2D1::ColorF(0, 0, 0, 0.0f)
	);

	// 플레이어 이동속도
	uint32 Speed = player->GetWalkSpeed();
	wss.str(L"");
	wss.clear();
	wss << Speed;
	DrawTextUI(
		wss.str(),
		Vec2(195.f, 738.f - (60.f * index)),
		Vec2(300.f, 100.f),
		16,
		D2D1::ColorF::White,
		D2D1::ColorF(0, 0, 0, 0.0f)
	);
	
	auto hpber = GET_SINGLE(SceneMgr)->GetActiveScene()->FindGameObject(L"PlayerPanel_" + to_wstring(index + 1) + L"_HP");
	if (hpber)
	{
		float hpRatio = static_cast<float>(hp) / maxHp;

		const float fullWidth = 270.f; // 전체 HP바 너비
		Vec3 scale = hpber->GetTransform()->GetLocalScale();
		scale.x = fullWidth * hpRatio;
		hpber->GetTransform()->SetLocalScale(scale);


		Vec3 position = hpber->GetTransform()->GetLocalPosition();
		position.x = -495.f - (fullWidth * (1.f - hpRatio) * 0.5f);  // 왼쪽 기준 보정
		hpber->GetTransform()->SetLocalPosition(position);
	}
}

void UIMgr::RenderUI()
{
	auto scene = GET_SINGLE(SceneMgr)->GetActiveScene();
	if (!scene)
		return;

	auto sceneType = GET_SINGLE(SceneMgr)->GetSceneType();
	if (sceneType == SCENE_TYPE::LOADING)
		return;

	gameFramework->BeginD2DRender();

	auto device = gameFramework->GetD3D11on12Device();
	auto ctx = device->GetD2DDeviceContext();
	auto brush = device->GetSolidColorBrush();

	brush->SetColor(D2D1::ColorF(D2D1::ColorF::White)); // 텍스트 색 설정

	if (scene && (sceneType == SCENE_TYPE::STAGE01 || sceneType == SCENE_TYPE::STAGE02))
	{
		// 잔여탄 UI
		{
			int32 currentAmmo = 0;
			auto localPlayer = static_pointer_cast<LocalPlayer>(scene->FindGameObject(L"LocalPlayer"));
			if (localPlayer)
			{
				int32 gunType = localPlayer->getGunType();
				if (gunType == 0)
					currentAmmo = static_pointer_cast<M4A1>(scene->FindGameObject(L"M4A1"))->GetCurrentAmmo();
				else if (gunType == 1)
					currentAmmo = static_pointer_cast<AK47>(scene->FindGameObject(L"AK47"))->GetCurrentAmmo();
			}



			std::wstringstream wss;
			wss << currentAmmo;

			DrawTextUI(
				wss.str(),
				Vec2(995.f, 740.f),			// 위치
				Vec2(100.f, 100.f),			// 텍스트 상자 크기
				16,							// 폰트 크기 ( 8 ~ 128까지 짝수 사이즈만 설정 가능)
				D2D1::ColorF::White,		// 텍스트 색상
				D2D1::ColorF(0, 0, 0, 0.0f) // 배경 색
			);
		}

		// Player UI
		int32 index = 0;
		shared_ptr<Player> player = static_pointer_cast<Player>(scene->FindGameObject(L"LocalPlayer"));
		RenderPlayerUI(GWindowInfo.local, player, index++);

		for (const auto& pair : scene->GetPlayers())
		{
			RenderPlayerUI(pair.first, pair.second[0], index++);
		}
	}


	gameFramework->EndD2DRender();
}

void UIMgr::CreateImageUI(const wstring& name, const wstring& texPath, const Vec2& pos, const Vec2& size, const float alpha, bool active, int32 uiType, shared_ptr<Scene> scene)
{
    if (_uiMap.contains(name))
        return;

    shared_ptr<UIObject> UI = make_shared<UIObject>();
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
    material->SetColor(0, Vec4(1.f, 1.f, 1.f, alpha)); // 투명도
    meshRenderer->SetMaterial(material);

	UI->SetName(name);
    UI->SetMeshRenderer(meshRenderer);
    UI->SetActive(active);
    _uiMap.insert({ name, UI });

	if (scene == nullptr)
		scene = GET_SINGLE(SceneMgr)->GetActiveScene();
    scene->AddGameObject(UI);
}

void UIMgr::CreateRectangleUI(const wstring& name, const Vec2& pos, const Vec2& size, const Vec4 color, bool active, int32 uiType, shared_ptr<class Scene> scene)
{
    if (_uiMap.contains(name))
        return;

    shared_ptr<UIObject> UI= make_shared<UIObject>();
	UI->SetLayerIndex(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI"));
	UI->SetTransform(make_shared<Transform>());
	UI->GetTransform()->SetLocalPosition(Vec3(pos.x, pos.y, 100.f));
	UI->GetTransform()->SetLocalScale(Vec3(size.x, size.y, 1.f));

    shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
    meshRenderer->SetMesh(GET_SINGLE(Resources)->LoadRectangleMesh());

    shared_ptr<Material> material = make_shared<Material>();
    material->SetShader(GET_SINGLE(Resources)->Get<Shader>(L"AlphaTexture"));
    material->SetColor(0, color);
    meshRenderer->SetMaterial(material);

	UI->SetName(name);
	UI->SetMeshRenderer(meshRenderer);
	UI->SetActive(active);
    _uiMap.insert({ name, UI });

    if (scene == nullptr)
        scene = GET_SINGLE(SceneMgr)->GetActiveScene();
    scene->AddGameObject(UI);
}

void UIMgr::DrawTextUI(const wstring& text, const Vec2& pos, const Vec2& size, int fontSize,
    const D2D1::ColorF& textColor, const D2D1::ColorF& bgColor, HAlign hAlign)
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
    format->SetTextAlignment(ToDW(hAlign));

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

shared_ptr<UIObject> UIMgr::GetUI(const wstring& name)
{
    if (_uiMap.contains(name))
        return _uiMap[name];
    return nullptr;
}

shared_ptr<UIObject> UIMgr::SelectUI(POINT pos)
{
	// 화면 중심(0,0) 기준 좌표로 변환
	const auto& win = gameFramework->GetWindow();
	float cx = win.width * 0.5f;
	float cy = win.height * 0.5f;

	// 윈도우 좌표(좌상단 원점, 아래+Y) → UI 좌표(중심 원점, 위+Y)
	Vec2 uiPos = {
		pos.x - cx,
		(win.height - pos.y) - cy
	};

	shared_ptr<UIObject> selected = nullptr;

	for (auto& [name, ui] : _uiMap)
	{
		if (!ui || !ui->IsActive())
			continue;

		auto t = ui->GetTransform();
		Vec3 pos = t->GetLocalPosition();
		Vec3 scale = t->GetLocalScale();

		float halfW = scale.x * 0.5f;
		float halfH = scale.y * 0.5f;

		if (uiPos.x >= pos.x - halfW && uiPos.x <= pos.x + halfW &&
			uiPos.y >= pos.y - halfH && uiPos.y <= pos.y + halfH)
		{
			selected = ui;
			break;
		}
	}

	return selected;
}

void UIMgr::LoadUIImage(shared_ptr<Scene> scene)
{
	if (scene == nullptr)
		scene = GET_SINGLE(SceneMgr)->GetActiveScene();

	// UI 초기화
	//ClearUI(); 현재 다른곳에서 처리

	// 조준선 UI 생성
	CreateImageUI(
		L"Crosshair",
		L"..\\Resources\\Texture\\Crosshair\\crosshair01.png",
		Vec2(0.f, 0.f), // 화면 중앙
		Vec2(50.f, 50.f), // 크기
		1.f, // 투명도 0 ~ 1
		true, // 활성화 여부
		static_cast<int32>(UI_TYPE::CROSSHAIR), // UI 타입
		scene
	);


	// 총 패널 UI 생성
	CreateRectangleUI(
		L"GunPanel_1",
		Vec2(490.f, -360.f),
		Vec2(300.f, 50.f),
		Vec4(0.5f, 0.5f, 0.5f, 0.5f), // 반투명 검정색
		true, // 활성화 여부
		static_cast<int32>(UI_TYPE::PANEL), // UI 타입
		scene
	);

	CreateImageUI(
		L"AK47",
		L"..\\Resources\\Texture\\Icon\\Gun\\AK47 실루엣(흰색).png",
		Vec2(520.f, -360.f),
		Vec2(165.f, 50.f),
		0.5f, // 투명도 0 ~ 1
		true, // 활성화 여부
		static_cast<int32>(UI_TYPE::IMAGE), // UI 타입
		scene
	);
	CreateImageUI(
		L"소총탄",
		L"..\\Resources\\Texture\\Icon\\Bullet\\소총탄.png",
		Vec2(410.f, -360.f),  // 위치
		Vec2(40.f, 40.f),	// 크기
		0.5f, // 투명도 0 ~ 1
		true, // 활성화 여부
		static_cast<int32>(UI_TYPE::IMAGE), // UI 타입
		scene
	);

	// 플레이어 정보 출력
	for (int i = 0; i < 3; i++)
	{
		wstring playerName = L"PlayerPanel_" + to_wstring(i + 1);

		// 패널
		CreateRectangleUI(
			playerName,					// 이름
			Vec2(-500.f, -360.f + (60 * i)),	// 위치
			Vec2(300.f, 50.f),					// 크기
			Vec4(0.5f, 0.5f, 0.5f, 0.5f),		// 색상
			i == 0,								// 활성화 여부 ( 플레이어 1 빼고 전부 비활성화 )
			static_cast<int32>(UI_TYPE::PANEL),	// UI 타입
			scene
		);

		// HP바 배경
		CreateRectangleUI(
			playerName + L"_Max_HP",			// 이름
			Vec2(-495.f, -370.f + (60 * i)),	// 위치
			Vec2(270.f, 20.f),					// 크기
			Vec4(0.0f, 0.0f, 0.0f, 1.f),		// 색상
			i == 0,								// 활성화 여부 ( 플레이어 1 빼고 전부 비활성화 )
			static_cast<int32>(UI_TYPE::BAR),	// UI 타입
			scene
		);

		// HP바
		CreateRectangleUI(
			playerName + L"_HP",				// 이름
			Vec2(-495.f, -370.f + +(60 * i)),	// 위치
			Vec2(270.f, 20.f),					// 크기
			Vec4(1.f, 0.0f, 0.0f, 1.f),			// 색상
			i == 0,								// 활성화 여부 ( 플레이어 1 빼고 전부 비활성화 )
			static_cast<int32>(UI_TYPE::BAR),	// UI 타입
			scene
		);

		// 플레이어 아이콘
		CreateImageUI(
			playerName + L"_Player_ID",									// 이름
			L"..\\Resources\\Texture\\Icon\\UI\\플레이어 아이콘.png",	// 경로
			Vec2(-620.f, -348.f + (60 * i)),							// 위치
			Vec2(20.f, 20.f),											// 크기
			0.8f,														// 투명도 0 ~ 1
			i == 0,														// 활성화 여부 ( 플레이어 1 빼고 전부 비활성화 )
			static_cast<int32>(UI_TYPE::ICON),							// UI 타입
			scene
		);

		// 공격력 아이콘
		CreateImageUI(
			playerName + L"_Attack_LV",								// 이름
			L"..\\Resources\\Texture\\Icon\\UI\\공격 아이콘.png",	// 경로
			Vec2(-540.f, -348.f + (60 * i)),						// 위치
			Vec2(20.f, 20.f),										// 크기
			0.8f,													// 투명도 0 ~ 1
			i == 0,													// 활성화 여부 ( 플레이어 1 빼고 전부 비활성화 )
			static_cast<int32>(UI_TYPE::ICON),						// UI 타입
			scene
		);

		// 이동속도 아이콘
		CreateImageUI(
			playerName + L"_Speed_LV",									// 이름
			L"..\\Resources\\Texture\\Icon\\UI\\이동속도 아이콘.png",	// 경로
			Vec2(-460.f, -348.f + (60 * i)),							// 위치
			Vec2(20.f, 20.f),											// 크기
			0.8f,														// 투명도 0 ~ 1
			i == 0,														// 활성화 여부 ( 플레이어 1 빼고 전부 비활성화 )
			static_cast<int32>(UI_TYPE::ICON),							// UI 타입
			scene
		);
	}
}