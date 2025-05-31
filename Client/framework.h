#pragma once
#include "CommandQueue.h"
#include "Mesh.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "DescriptorHeap.h"
#include "Texture.h"
#include "RenderTargetGroup.h"
#include "D3D11On12Device.h"

class Framework
{
public:
	void Init(const WindowInfo& info);
	void Update();

public:
	const WindowInfo& GetWindow() { return _window; }

	ComPtr<ID3D12Device> GetDevice() { return _device; }
	ComPtr<IDXGIFactory4> GetDXGI() { return _factory; }

	ComPtr<IDXGISwapChain3> GetSwapChain() { return _swapChain; }
	uint32 GetCurrBackBufferIndex() { return _currBackBufferIndex; }

	shared_ptr<GraphicsCommandQueue> GetGraphicsCmdQueue() { return _graphicsCmdQueue; }
	shared_ptr<ComputeCommandQueue> GetComputeCmdQueue() { return _computeCmdQueue; }
	
	ComPtr<ID3D12RootSignature>	GetGraphicsRootSignature() { return _graphicsRootSignature; }
	ComPtr<ID3D12RootSignature>	GetComputeRootSignature() { return _computeRootSignature; }

	shared_ptr<GraphicsDescriptorHeap>	GetGraphicsDescHeap() { return _graphicsDescHeap; }
	shared_ptr<ComputeDescriptorHeap>	GetComputeDescHeap() { return _computeDescHeap; }

	shared_ptr<D3D11On12Device> GetD3D11on12Device() { return _d3d11on12Device; }

	shared_ptr<ConstantBuffer> GetConstantBuffer(CONSTANT_BUFFER_TYPE type) { return _constantBuffers[static_cast<uint8>(type)]; }
	shared_ptr<RenderTargetGroup> GetRTGroup(RENDER_TARGET_GROUP_TYPE type) { return _rtGroups[static_cast<uint8>(type)]; }

public:
	void Render();
	void RenderBegin();
	void RenderEnd();

	void ResizeWindow(int32 widht, int32 height);

private:
	void ShowFps();

	void CreateSwapChain();
	void CreateGraphicsRootSignature();
	void CreateComputeRootSignature();
	void CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count);
	void CreateRenderTargetGroups();
	void CreateD3D11On12Device();

private:
	// 그려질 화면 크기 관련
	WindowInfo		_window;
	D3D12_VIEWPORT	_viewport = {};
	D3D12_RECT		_scissorRect = {};

	// Device
	ComPtr<IDXGIFactory4>		_factory;
	ComPtr<ID3D12Device>		_device;

	// CommandQueue
	shared_ptr<GraphicsCommandQueue>	_graphicsCmdQueue = make_shared<GraphicsCommandQueue>();
	shared_ptr<ComputeCommandQueue>	_computeCmdQueue = make_shared<ComputeCommandQueue>();

	// SwapChain
	ComPtr<IDXGISwapChain3>		_swapChain;
	uint32						_currBackBufferIndex = {};

	// RootSignature
	D3D12_STATIC_SAMPLER_DESC	_staticSamplerDesc = {};
	ComPtr<ID3D12RootSignature> _graphicsRootSignature;
	ComPtr<ID3D12RootSignature> _computeRootSignature;


	shared_ptr<GraphicsDescriptorHeap>	_graphicsDescHeap = make_shared<GraphicsDescriptorHeap>();
	shared_ptr<ComputeDescriptorHeap>	_computeDescHeap = make_shared<ComputeDescriptorHeap>();
	shared_ptr<D3D11On12Device>	_d3d11on12Device = make_shared<D3D11On12Device>();
	
	vector<shared_ptr<ConstantBuffer>> _constantBuffers;
	array<shared_ptr<RenderTargetGroup>, RENDER_TARGET_GROUP_COUNT> _rtGroups;
};

