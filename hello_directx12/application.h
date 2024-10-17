// Liam Wynn, 9/26/2024, Hello DirectX 12

//
// The application is what manages any game specific logic
// and below. So the application manages all the DX12 logic
// (via the dx12_handler). But it also contains all the game
// specific stuff.
//

#pragma once

#include "dx12_handler.h"

using namespace DirectX;
using namespace std;

const UINT TEXTURE_W = 256;
const UINT TEXTURE_H = 256;
// Num bytes per pixel.
const UINT TEXTURE_PIXEL_SIZE = 4;

// One of the things I am taking issue with this example is that the
// input to our vertex shader here is a FLOAT3 and a FLOAT2. However,
// in the shader, it takes two float4's. I need to figure out why
// this is the case, and how to structure it to match.
//
// I am keeping the above comment for posterity's sake. However, my
// code *DOES NOT* reflect this anymore. But basically I guess it boils
// down to HLSL can handle the type differences with padding. So if it
// expects a float4 but gets a float3, no problem we'll fix that up.
struct vertex {
	XMFLOAT3 position;
	XMFLOAT2 uv;
};

struct application {
	uint32_t screen_w;
	uint32_t screen_h;

	// Contains all DX12 objects.
	dx12_handler* dx12;

	float clear_color[4];

	// Resources to render the cube.
	ComPtr<ID3D12Resource> vertex_buffer;
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
	ComPtr<ID3D12Resource> index_buffer;
	D3D12_INDEX_BUFFER_VIEW index_buffer_view;
	ComPtr<ID3D12Resource> texture;

	// Camera resources
	XMMATRIX model_matrix;
	XMMATRIX view_matrix;
	XMMATRIX projection_matrix;
	float field_of_view;

	// This describes the various parameters passed to the
	// different stages of the shader pipeline.
	ComPtr<ID3D12RootSignature> root_signature;

	// This describes all the state information for the rendering
	// pipeline. This would be things like the root signature, the
	// vertex shader, and the pixel shader.
	ComPtr<ID3D12PipelineState> pipeline_state;

	// Needed for rasterizer state
	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissor_rect;

	// Needed for the depth buffer.
	ComPtr<ID3D12Resource> depth_buffer;
	ComPtr<ID3D12DescriptorHeap> depth_stencil_view;
};

bool initialize_application(
	application* app,
	HWND hwnd,
	const uint32_t screen_w,
	const uint32_t screen_h,
	const bool use_warp
);

void load_assets(application* app);
ComPtr<ID3D12RootSignature> initialize_root_signature(application* app);
ComPtr<ID3D12PipelineState> initialize_pipeline_state(application* app);
// Initializes the buffers needed for the cube we draw.
void initialize_cube(application* app);
void upload_buffer_data(
	ComPtr<ID3D12Device> dev,
	void* buffer_data,
	const UINT buffer_size,
	ID3D12Resource** resource_buffer
);
void create_texture(application* app);
vector<UINT8> generate_texture_data();
void initialize_depth_buffer(application* app);

void frame(application* app);

void update(application* app);

void render(application* app);
// Sets up the dx12 handler's command list for rendering.
void populate_command_list(application* app);

void shutdown_application(application* app);
