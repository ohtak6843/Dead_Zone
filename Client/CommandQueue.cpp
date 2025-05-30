#include "pch.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "Framework.h"

BaseCommandQueue::~BaseCommandQueue()
{
	::CloseHandle(_fenceEvent);
}

void BaseCommandQueue::Init(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_cmdQueue));

	device->CreateCommandAllocator(type, IID_PPV_ARGS(&_cmdAlloc));
	device->CreateCommandList(0, type, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList));
	_cmdList->Close();

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void BaseCommandQueue::WaitSync()
{
	_fenceValue++;
	_cmdQueue->Signal(_fence.Get(), _fenceValue);

	if (_fence->GetCompletedValue() < _fenceValue)
	{
		_fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

GraphicsCommandQueue::~GraphicsCommandQueue()
{
	::CloseHandle(_fenceEvent);
}

void GraphicsCommandQueue::Init(ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain3> swapChain)
{
	_swapChain = swapChain;
	BaseCommandQueue::Init(device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_resCmdAlloc));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _resCmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_resCmdList));
	// _resCmdList->Close(); // 열려 있어도 무방함
}

void GraphicsCommandQueue::RenderBegin()
{
	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	int8 backIndex = gameFramework->GetCurrBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	_cmdList->SetGraphicsRootSignature(GRAPHICS_ROOT_SIGNATURE.Get());
	gameFramework->GetConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM)->Clear();
	gameFramework->GetConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL)->Clear();
	gameFramework->GetGraphicsDescHeap()->Clear();

	ID3D12DescriptorHeap* descHeap = gameFramework->GetGraphicsDescHeap()->GetDescriptorHeap().Get();
	_cmdList->SetDescriptorHeaps(1, &descHeap);
	_cmdList->ResourceBarrier(1, &barrier);
}

void GraphicsCommandQueue::RenderEnd()
{
	int8 backIndex = gameFramework->GetCurrBackBufferIndex();
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	_cmdList->ResourceBarrier(1, &barrier);
	_cmdList->Close();

	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);
}

void GraphicsCommandQueue::FlushResourceCommandQueue()
{
	_resCmdList->Close();

	ID3D12CommandList* cmdListArr[] = { _resCmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	_resCmdAlloc->Reset();
	_resCmdList->Reset(_resCmdAlloc.Get(), nullptr);

	GRAPHICS_CMD_LIST->SetGraphicsRootSignature(GRAPHICS_ROOT_SIGNATURE.Get());
}

void ComputeCommandQueue::Init(ComPtr<ID3D12Device> device)
{
	BaseCommandQueue::Init(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
}

void ComputeCommandQueue::FlushComputeCommandQueue()
{
	_cmdList->Close();
	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	COMPUTE_CMD_LIST->SetComputeRootSignature(COMPUTE_ROOT_SIGNATURE.Get());
}
