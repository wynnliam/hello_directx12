// Liam Wynn, 9/26/2024, Hello DirectX 12

//
// The DX12 Handler is a struct to hold all the needed
// DirectX12 stuff like swap chains, command lists, etc.
// I think for a more robust engine you may want a better
// organization than a "pile of data" so to speak, but
// for our purposes this works just fine.
//

#pragma once

#include "stdafx.h"

const UINT NUM_RENDER_TARGETS = 3;

struct dx12_handler {
	dx12_handler();

	//
	// Core DX12 objects.
	//

	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12CommandQueue> command_queue;
	ComPtr<IDXGISwapChain3> swap_chain;
	ComPtr<ID3D12Resource> render_targets[NUM_RENDER_TARGETS];
	ComPtr<ID3D12DescriptorHeap> rtv_heap;
	ComPtr<ID3D12DescriptorHeap> srv_heap;
	UINT rtv_descriptor_size;
	ComPtr<ID3D12CommandAllocator> command_allocator;
	ComPtr<ID3D12GraphicsCommandList> command_list;

	//
	// Synchronization fence objects needed for rendering.
	//

	UINT frame_index;
	HANDLE fence_event;
	ComPtr<ID3D12Fence> fence;
	UINT64 fence_value;
};

bool initialize_directx_12(
	dx12_handler* dx12,
	HWND hwnd,
	const uint32_t screen_w,
	const uint32_t screen_h,
	const bool use_warp
);

void enable_dx12_debug_layer();

ComPtr<IDXGIFactory4> create_dx12_factory();

// Finds a suitable adapter for DX12. An adapter is the hardware
// that generates an image. So this includes the GPU, integrated
// grpahics, and/or CPU.
ComPtr<IDXGIAdapter4> get_valid_adapter(
	const bool use_warp,
	ComPtr<IDXGIFactory4> factory
);

ComPtr<ID3D12Device> create_dx12_device(ComPtr<IDXGIAdapter4> adapter);

ComPtr<ID3D12CommandQueue> create_command_queue(ComPtr<ID3D12Device> dev);

ComPtr<IDXGISwapChain3> create_swap_chain(
	HWND hwnd,
	const uint32_t screen_w,
	const uint32_t screen_h,
	ComPtr<IDXGIFactory4> factory,
	ComPtr<ID3D12CommandQueue> command_queue
);

ComPtr<ID3D12DescriptorHeap> create_descriptor_heap(
	ComPtr<ID3D12Device> dev,
	const UINT num_descriptors,
	D3D12_DESCRIPTOR_HEAP_TYPE heap_type,
	D3D12_DESCRIPTOR_HEAP_FLAGS descriptor_flags
);

void update_render_target_views(dx12_handler* dx12);

ComPtr<ID3D12CommandAllocator> create_command_allocator(
	ComPtr<ID3D12Device> dev,
	D3D12_COMMAND_LIST_TYPE command_list_type
);

ComPtr<ID3D12GraphicsCommandList> create_command_list(
	ComPtr<ID3D12Device> device,
	ComPtr<ID3D12CommandAllocator> allocator,
	D3D12_COMMAND_LIST_TYPE command_list_type
);

ComPtr<ID3D12Fence> create_fence(ComPtr<ID3D12Device> device);

HANDLE create_fence_event();

void wait_for_previous_frame(dx12_handler* dx12);

void shutdown_directx_12(dx12_handler* dx12);
