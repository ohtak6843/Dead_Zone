#pragma once

class SwapChain;
class DescriptorHeap;

class BaseCommandQueue
{
public:
	virtual ~BaseCommandQueue();

	void Init(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type);
	
	void CreateCmdQueueAndList(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type);
	void CreateFence(ComPtr<ID3D12Device> device);

	void WaitSync();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() const { return _cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetCmdList() const { return _cmdList; }

protected:
	ComPtr<ID3D12CommandQueue> _cmdQueue;
	ComPtr<ID3D12CommandAllocator> _cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList> _cmdList;

	ComPtr<ID3D12Fence> _fence;
	HANDLE _fenceEvent = nullptr;
	uint64 _fenceValue = 0;
};

class GraphicsCommandQueue : public BaseCommandQueue
{
public:
	virtual ~GraphicsCommandQueue();

	void Init(ComPtr<ID3D12Device> device);

	void CreateResCmdQueueAndList(ComPtr<ID3D12Device> device);

	void RenderBegin();
	void RenderEnd();
	void FlushResourceCommandQueue();

	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCmdList() { return _cmdList; }
	ComPtr<ID3D12GraphicsCommandList> GetResourceCmdList() { return _resCmdList; }

private:
	ComPtr<ID3D12CommandAllocator> _resCmdAlloc;
	ComPtr<ID3D12GraphicsCommandList> _resCmdList;
};

class ComputeCommandQueue : public BaseCommandQueue
{
public:
	virtual ~ComputeCommandQueue() = default;

	void Init(ComPtr<ID3D12Device> device);
	void FlushComputeCommandQueue();
};