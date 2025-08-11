#include "pch.h"
#include "Framework.h"
#include "Material.h"
#include "Transform.h"
#include "InputMgr.h"
#include "Timer.h"
#include "SceneMgr.h"
#include "Light.h"

#include "Resources.h"
#include "InstancingMgr.h"
#include "GameInfo.h"
#include "UIMgr.h"
#include "FmodMgr.h"

#include "../echoserver/protocol.h"

void Framework::Init(const WindowInfo& info)
{
	_window = info;

	_viewport = { 0, 0, static_cast<FLOAT>(info.width), static_cast<FLOAT>(info.height), 0.0f, 1.0f };
	_scissorRect = CD3DX12_RECT(0, 0, info.width, info.height);

	::CreateDXGIFactory(IID_PPV_ARGS(&_factory));
	::D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device));

	_graphicsCmdQueue->Init(_device);
	_computeCmdQueue->Init(_device);

	CreateSwapChain();
	CreateGraphicsRootSignature();
	CreateComputeRootSignature();

	_graphicsDescHeap->Init(5120);
	_computeDescHeap->Init();

	CreateConstantBuffers();

	CreateRenderTargetGroups();
	CreateD3D11On12Device();

	ResizeWindow(info.width, info.height);

	GET_SINGLE(InputMgr)->Init(info.hwnd);
	GET_SINGLE(Timer)->Init();
	GET_SINGLE(Resources)->Init();
	GET_SINGLE(GameInfo)->Init();
	GET_SINGLE(UIMgr)->Init();
	GET_SINGLE(FmodMgr)->Init();

	GET_SINGLE(SceneMgr)->SetLayerName(0, L"Default");
	GET_SINGLE(SceneMgr)->SetLayerName(1, L"Gun"); // 총 UI 별도 처리
	GET_SINGLE(SceneMgr)->SetLayerName(2, L"UI");
}

void Framework::Update()
{
	GET_SINGLE(InputMgr)->Update();
	GET_SINGLE(Timer)->Update();
	GET_SINGLE(SceneMgr)->Update();
	GET_SINGLE(FmodMgr)->Update();
	GET_SINGLE(InstancingMgr)->ClearBuffer();

	Render();

	ShowFps();

	// 씬 전환 처리
	if (GET_SINGLE(SceneMgr)->GetChangeScene())
	{
		GET_SINGLE(Timer)->CancelAll();
		SCENE_TYPE nextSceneType = GET_SINGLE(SceneMgr)->GetNextSceneType();
		GET_SINGLE(SceneMgr)->SetChangeScene(false);
		GET_SINGLE(SceneMgr)->SwitchScene(nextSceneType);
		switch (nextSceneType)
		{
		case SCENE_TYPE::STAGE01:
		{
			cs_packet_generic ready{ static_cast<unsigned char>(sizeof(cs_packet_generic)),C2S_P_SCENE_LOADED };
			send(gameFramework->GetWindow().sock,
				reinterpret_cast<char*>(&ready),
				sizeof(ready), 0);
			break;
		}
		case SCENE_TYPE::STAGE02:
		{
			cs_packet_stage_loaded ready2{ sizeof(cs_packet_stage_loaded),C2S_P_STAGE_LOADED };
			send(gameFramework->GetWindow().sock,
				reinterpret_cast<char*>(&ready2),
				sizeof(ready2), 0);
			break;
		}
		case SCENE_TYPE::STAGE03:
		{
			cs_packet_stage_loaded ready3{ sizeof(cs_packet_stage_loaded),C2S_P_STAGE_LOADED };
			send(gameFramework->GetWindow().sock,
				reinterpret_cast<char*>(&ready3),
				sizeof(ready3), 0);
			break;
		}
		default:
			break;
		}
	}
}

void Framework::Render()
{
	RenderBegin();

	GET_SINGLE(SceneMgr)->Render();

	RenderEnd();
}

void Framework::RenderBegin()
{
	_graphicsCmdQueue->RenderBegin();
}

void Framework::RenderEnd()
{
	_graphicsCmdQueue->RenderEnd();

	GET_SINGLE(SceneMgr)->RenderUI();

	_swapChain->Present(1, 0);

	_graphicsCmdQueue->WaitSync();

	_currBackBufferIndex = (_currBackBufferIndex + 1) % SWAP_CHAIN_BUFFER_COUNT;
}

void Framework::ResizeWindow(int32 width, int32 height)
{
	_window.width = width;
	_window.height = height;

	RECT rect = { 0, 0, width, height };
	::AdjustWindowRect(&rect, WS_OVERLAPPED, false);
	::SetWindowPos(_window.hwnd, 0, 100, 100, width, height, 0);
}

void Framework::ToggleFullScreen(bool flag)
{
	if (flag)
	{
		// 창모드 -> 전체화면
		// 스타일 변경: 테두리 제거하고 팝업 스타일로
		LONG currentStyle = ::GetWindowLong(_window.hwnd, GWL_STYLE);
		currentStyle &= ~WS_OVERLAPPEDWINDOW;
		currentStyle |= WS_POPUP;
		::SetWindowLong(_window.hwnd, GWL_STYLE, currentStyle);

		// 모니터 정보 가져와서 전체화면 크기로 설정
		MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
		if (::GetMonitorInfo(MonitorFromWindow(_window.hwnd, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
		{
			const RECT& rc = monitorInfo.rcMonitor;
			::SetWindowPos(_window.hwnd, HWND_TOP,
				rc.left, rc.top,
				rc.right - rc.left,
				rc.bottom - rc.top,
				SWP_NOZORDER | SWP_FRAMECHANGED);
		}

		SET_FULL_SCREEN(flag);
	}
	else
	{
		// 전체화면 -> 창모드
		// 스타일을 원래대로 복원
		LONG currentStyle = ::GetWindowLong(_window.hwnd, GWL_STYLE);
		currentStyle &= ~WS_POPUP;
		currentStyle |= WS_OVERLAPPEDWINDOW;
		::SetWindowLong(_window.hwnd, GWL_STYLE, currentStyle);

		// 저장된 창 위치와 크기로 복원
		::SetWindowPos(_window.hwnd, 0, 100, 100, _window.width, _window.height,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		SET_FULL_SCREEN(flag);
	}
}

void Framework::ShowFps()
{
	uint32 fps = GET_SINGLE(Timer)->GetFps();

	WCHAR text[100] = L"";
	::wsprintf(text, L"FPS : %d", fps);

	::SetWindowText(_window.hwnd, text);
}

void Framework::CreateSwapChain()
{
	_swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = _window.width;
	swapChainDesc.Height = _window.height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDesc = {};
	swapChainFullScreenDesc.RefreshRate.Numerator = 60;
	swapChainFullScreenDesc.RefreshRate.Denominator = 1;
	swapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainFullScreenDesc.Windowed = _window.windowed;

	_factory->CreateSwapChainForHwnd(
		_graphicsCmdQueue->GetCmdQueue().Get(),
		_window.hwnd,
		&swapChainDesc,
		&swapChainFullScreenDesc,
		nullptr,
		(IDXGISwapChain1**)_swapChain.GetAddressOf()
	);
}

void Framework::CreateGraphicsRootSignature()
{
	D3D12_STATIC_SAMPLER_DESC samplerDesc;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias = 0.f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	_staticSamplerDesc = samplerDesc;

	CD3DX12_DESCRIPTOR_RANGE ranges[] =
	{
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT - 1, 1),
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0),
	};

	CD3DX12_ROOT_PARAMETER param[2];
	param[0].InitAsConstantBufferView(static_cast<uint32>(CBV_REGISTER::b0));
	param[1].InitAsDescriptorTable(_countof(ranges), ranges);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param, 1, &_staticSamplerDesc);
	rootSignatureDesc.Flags = rootSignatureFlags;

	ComPtr<ID3DBlob> blobSignature;
	ComPtr<ID3DBlob> blobError;
	::D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blobSignature, &blobError);
	DEVICE->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(), IID_PPV_ARGS(&_graphicsRootSignature));
}

void Framework::CreateComputeRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE ranges[] =
	{
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT, 0),
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0),
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, UAV_REGISTER_COUNT, 0),
	};

	CD3DX12_ROOT_PARAMETER param[1];
	param[0].InitAsDescriptorTable(_countof(ranges), ranges);

	D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param);
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ComPtr<ID3DBlob> blobSignature;
	ComPtr<ID3DBlob> blobError;
	::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blobSignature, &blobError);
	DEVICE->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(), IID_PPV_ARGS(&_computeRootSignature));

	COMPUTE_CMD_LIST->SetComputeRootSignature(_computeRootSignature.Get());
}

void Framework::CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count)
{
	uint8 typeInt = static_cast<uint8>(reg);
	assert(_constantBuffers.size() == typeInt);

	shared_ptr<ConstantBuffer> buffer = make_shared<ConstantBuffer>();
	buffer->Init(reg, bufferSize, count);
	_constantBuffers.push_back(buffer);
}

void Framework::CreateConstantBuffers()
{
	CreateConstantBuffer(CBV_REGISTER::b0, sizeof(LightParams), 1);
	CreateConstantBuffer(CBV_REGISTER::b1, sizeof(TransformParams), 5120);
	CreateConstantBuffer(CBV_REGISTER::b2, sizeof(MaterialParams), 5120);
}

void Framework::CreateRenderTargetGroups()
{
	// DepthStencil
	shared_ptr<Texture> dsTexture = GET_SINGLE(Resources)->CreateTexture(L"DepthStencil",
		DXGI_FORMAT_D32_FLOAT, _window.width, _window.height,
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	// SwapChain Group
	{
		vector<RenderTarget> rtVec(SWAP_CHAIN_BUFFER_COUNT);

		for (uint32 i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		{
			wstring name = L"SwapChainTarget_" + std::to_wstring(i);

			ComPtr<ID3D12Resource> resource;
			_swapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
			rtVec[i].target = GET_SINGLE(Resources)->CreateTextureFromResource(name, resource);
		}

		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)] = make_shared<RenderTargetGroup>();
		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)]->Create(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN, rtVec, dsTexture);
	}
	
	// Shadow Group
	{
		vector<RenderTarget> rtVec(RENDER_TARGET_SHADOW_GROUP_MEMBER_COUNT);

		rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"ShadowTarget",
			DXGI_FORMAT_R32_FLOAT, 8192, 8192,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		shared_ptr<Texture> shadowDepthTexture = GET_SINGLE(Resources)->CreateTexture(L"ShadowDepthStencil",
			DXGI_FORMAT_D32_FLOAT, 8192, 8192,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SHADOW)] = make_shared<RenderTargetGroup>();
		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SHADOW)]->Create(RENDER_TARGET_GROUP_TYPE::SHADOW, rtVec, shadowDepthTexture);
	}

	// Deferred Group
	{
		vector<RenderTarget> rtVec(RENDER_TARGET_G_BUFFER_GROUP_MEMBER_COUNT);

		rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"PositionTarget",
			DXGI_FORMAT_R32G32B32A32_FLOAT, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		rtVec[1].target = GET_SINGLE(Resources)->CreateTexture(L"NormalTarget",
			DXGI_FORMAT_R32G32B32A32_FLOAT, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		rtVec[2].target = GET_SINGLE(Resources)->CreateTexture(L"DiffuseTarget",
			DXGI_FORMAT_R8G8B8A8_UNORM, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::G_BUFFER)] = make_shared<RenderTargetGroup>();
		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::G_BUFFER)]->Create(RENDER_TARGET_GROUP_TYPE::G_BUFFER, rtVec, dsTexture);
	}

	// Lighting Group
	{
		vector<RenderTarget> rtVec(RENDER_TARGET_LIGHTING_GROUP_MEMBER_COUNT);

		rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"DiffuseLightTarget",
			DXGI_FORMAT_R8G8B8A8_UNORM, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		rtVec[1].target = GET_SINGLE(Resources)->CreateTexture(L"SpecularLightTarget",
			DXGI_FORMAT_R8G8B8A8_UNORM, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::LIGHTING)] = make_shared<RenderTargetGroup>();
		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::LIGHTING)]->Create(RENDER_TARGET_GROUP_TYPE::LIGHTING, rtVec, dsTexture);
	}
}

void Framework::CreateD3D11On12Device()
{
	vector<ComPtr<ID3D12Resource>> rtVec = {
	_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)]->GetRTTexture(0)->GetTex2D(),
	_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)]->GetRTTexture(1)->GetTex2D()
	};
	_d3d11on12Device->Init(_device, _factory, rtVec, _graphicsCmdQueue->GetCmdQueue());
}

void Framework::BeginD2DRender()
{
	uint8 idx = GetCurrBackBufferIndex();
	auto device = GetD3D11on12Device();

	device->GetD3D11on12Device()->AcquireWrappedResources(device->GetWrappedBackBuffer(idx).GetAddressOf(), 1);
	device->GetD2DDeviceContext()->SetTarget(device->GetD3D11On12RT(idx).Get());
	device->GetD2DDeviceContext()->BeginDraw();
}

void Framework::EndD2DRender()
{
	uint8 idx = GetCurrBackBufferIndex();
	auto device = GetD3D11on12Device();

	device->GetD2DDeviceContext()->EndDraw();
	device->GetD3D11on12Device()->ReleaseWrappedResources(device->GetWrappedBackBuffer(idx).GetAddressOf(), 1);
	device->GetD3D11DeviceContext()->Flush();
}