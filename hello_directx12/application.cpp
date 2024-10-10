// Liam Wynn, 9/26/2024, Hello DirectX 12

#include "application.h"
#include "utils.h"

bool initialize_application(
	application* app,
	HWND hwnd,
	const uint32_t screen_w,
	const uint32_t screen_h,
	const bool use_warp
) {
	bool success;

	//
	// First set up the DX12 Handler.
	//

	app->dx12 = new dx12_handler;
	success = initialize_directx_12(
		app->dx12,
		hwnd,
		screen_w,
		screen_h,
		use_warp
	);

	//
	// Now set the clear color. Sets it to cornflower blue.
	//

	app->clear_color[0] = 0.4f;
	app->clear_color[1] = 0.6f;
	app->clear_color[2] = 0.9f;
	app->clear_color[3] = 1.0f;

	//
	// Next set the viewport and scissor rect.
	//

	app->viewport = CD3DX12_VIEWPORT(
		0.0f,
		0.0f,
		(float)screen_w,
		(float)screen_h
	);

	app->scissor_rect = CD3DX12_RECT(0, 0, (long)screen_w, (long)screen_h);

	//
	// Next load all the assets needed for running the program.
	//

	load_assets(app);

	return success;
}

void load_assets(application* app) {
	ComPtr<ID3D12GraphicsCommandList> command_list;
	ComPtr<ID3D12CommandAllocator> command_allocator;
	HRESULT result;

	command_list = app->dx12->command_list;
	command_allocator = app->dx12->command_allocator;

	//
	// First initialize the shader pipeline's root signature.
	//

	app->root_signature = initialize_root_signature(app);

	//
	// Next, create the pipeline state and attach it to the
	// command list.
	//

	app->pipeline_state = initialize_pipeline_state(app);

	result = command_list->Reset(command_allocator.Get(), NULL);
	throw_if_failed(result);
	command_list->SetPipelineState(app->pipeline_state.Get());

	//
	// Initialize the vertex buffer for the textured cube.
	//

	initialize_cube(app);

	//
	// Create the texture for our cube.
	//

	create_texture(app);
}

ComPtr<ID3D12RootSignature> initialize_root_signature(application* app) {
	ComPtr<ID3D12RootSignature> root_signature;
	ComPtr<ID3D12Device> dev;
	D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data;
	HRESULT result;
	CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
	CD3DX12_ROOT_PARAMETER1 root_parameters[1];
	D3D12_STATIC_SAMPLER_DESC sampler;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_sig_desc;
	ComPtr<ID3DBlob> sig_blob;
	ComPtr<ID3DBlob> err_blob;

	dev = app->dx12->device;

	//
	// V1.1 is the highest version that this program supports. See if the
	// machine supports it. If not, fall back to V1.0.
	//

	feature_data = {};
	feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	result = dev->CheckFeatureSupport(
		D3D12_FEATURE_ROOT_SIGNATURE,
		&feature_data,
		sizeof(feature_data)
	);

	if (FAILED(result)) {
		feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	//
	// Now set up the root signature parameters.
	//

	// First we set up our ranges. In this case, we only
	// have one to deal with - SRVs. We only have one texture
	// we sample from, starting from the 0th register.
	ranges[0] = {};
	ranges[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,
		0,
		0,
		D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC
	);

	root_parameters[0] = {};
	root_parameters[0].InitAsDescriptorTable(
		1,
		&ranges[0],
		D3D12_SHADER_VISIBILITY_PIXEL
	);

	// Here we set up what kind of texture filter we want.
	// At some point I'd like to go back and learn what exactly
	// all these parameters are doing.
	sampler = {};
	// The filtering method used when sampling the texture.
	sampler.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	// What should the sampler do if the u coordinate is outside
	// [0, 1]?
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	// What should the sampler do if the v coordinate is outside
	// [0, 1]?
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	// What should the sampler do if the w coordinate is outside
	// [0, 1]? 
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	// If we're doing mipmapping, this is a bias we add to the calculated
	// mip-level. Say we calculated mip level 3, but then our bias was 2.
	// Then our resulting mip level is 5.
	sampler.MipLODBias = 0;
	// Something to do with anisotropy. Not sure. TODO: Read up on this
	sampler.MaxAnisotropy = 0;
	// Compares sampled data to existing sampled data. For what reason
	// I am not sure. This is needed for fancy sampling in something like
	// a shadow mapping, depth testing, or any kind of fancy filtering.
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	// If our Address members are D3D12_TEXTURE_ADDRESS_MODE_BORDER, then
	// this is the border color we'd want for it.
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	// These two have to do with mipmap ranges. Im not too sure what
	// they are used for.
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	// Corresponds to the shader register in HLSL. In our shader
	// (texture_shader.hlsl), we specified this was located at register
	// 0. So if we had multiple samplers, we would put them in the
	// corresponding shader regsiter.
	sampler.ShaderRegister = 0;
	// Similar to above, but we are dealing with reigster spaces. I don't
	// know how many register spaces there are. By default the value is 0
	// in HLSL, so that's why we set this to 0 here.
	sampler.RegisterSpace = 0;
	// Basically says: who in the pipeline sees this sampler? In our case,
	// we want the pixel shader to see it.
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Use the root params + sampler to define the root signature's
	// description.
	root_sig_desc.Init_1_1(
		_countof(root_parameters),
		root_parameters,
		1,
		&sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	//
	// Use the root signature description to create the root signature
	// itself.
	//

	result = D3DX12SerializeVersionedRootSignature(
		&root_sig_desc,
		feature_data.HighestVersion,
		&sig_blob,
		&err_blob
	);

	throw_if_failed(result);

	result = dev->CreateRootSignature(
		0,
		sig_blob->GetBufferPointer(),
		sig_blob->GetBufferSize(),
		IID_PPV_ARGS(&root_signature)
	);

	throw_if_failed(result);

	return root_signature;
}

ComPtr<ID3D12PipelineState> initialize_pipeline_state(application* app) {
	ComPtr<ID3D12Device> dev;
	ComPtr<ID3D12PipelineState> pipeline_state;
	ComPtr<ID3DBlob> vertex_blob;
	ComPtr<ID3DBlob> pixel_blob;
	UINT compile_flags;
	HRESULT result;
	D3D12_INPUT_ELEMENT_DESC input_element_desc[2];
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;

	dev = app->dx12->device;

	//
	// Load the shader (which is a single file with both the vertex and pixel
	// shaders), and compile it.
	//

	// If we're in debug mode, we want to add these flags.
	compile_flags = 0;
#if defined(_DEBUG)
	compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// Compile the vertex shader.
	result = D3DCompileFromFile(
		L"./texture_shader.hlsl",
		NULL,
		NULL,
		"vs_main",
		"vs_5_0",
		compile_flags,
		0,
		&vertex_blob,
		NULL
	);

	throw_if_failed(result);

	// Compile the pixel shader.
	result = D3DCompileFromFile(
		L"./texture_shader.hlsl",
		NULL,
		NULL,
		"ps_main",
		"ps_5_0",
		compile_flags,
		0,
		&pixel_blob,
		NULL
	);

	throw_if_failed(result);

	//
	// Define the vertex input layout. So what I think is happening
	// is we give it a float3 + float2 (position + uv). However, the
	// shader must automatically convert both to a float4. This is
	// gross in my opinion, and it should be consistent all around.
	//

	input_element_desc[0] = {
		"POSITION",
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,
		0,
		0,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
	};

	input_element_desc[1] = {
		"TEXCOORD",
		0,
		DXGI_FORMAT_R32G32_FLOAT,
		0,
		12,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
	};

	//
	// Create the description for the PSO
	//

	pso_desc = {};
	pso_desc.InputLayout = { input_element_desc, _countof(input_element_desc) };
	pso_desc.pRootSignature = app->root_signature.Get();
	pso_desc.VS = CD3DX12_SHADER_BYTECODE(vertex_blob.Get());
	pso_desc.PS = CD3DX12_SHADER_BYTECODE(pixel_blob.Get());
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pso_desc.DepthStencilState.DepthEnable = FALSE;
	pso_desc.DepthStencilState.StencilEnable = FALSE;
	pso_desc.SampleMask = UINT_MAX;
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pso_desc.SampleDesc.Count = 1;

	//
	// Finally, create the pipeline state object.
	//

	result = dev->CreateGraphicsPipelineState(
		&pso_desc,
		IID_PPV_ARGS(&pipeline_state)
	);

	throw_if_failed(result);

	return pipeline_state;
}

void initialize_cube(application* app) {
	vertex cube_verts[4];
	WORD cube_indices[6];
	UINT vertex_buffer_size;
	UINT index_buffer_size;
	ComPtr<ID3D12Resource> vertex_buffer;
	ComPtr<ID3D12Resource> index_buffer;
	ComPtr<ID3D12Device> dev;
	D3D12_VERTEX_BUFFER_VIEW vbv;
	D3D12_INDEX_BUFFER_VIEW ibv;

	dev = app->dx12->device;

	//
	// Define the input data we will send to the shader.
	//

	cube_verts[0] = { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f } }; 
	cube_verts[1] = { {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }; 
	cube_verts[2] = { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } }; 
	cube_verts[3] = { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f } };

	cube_indices[0] = 0;
	cube_indices[1] = 1;
	cube_indices[2] = 2;
	cube_indices[3] = 0;
	cube_indices[4] = 3;
	cube_indices[5] = 1;

	vertex_buffer_size = sizeof(cube_verts);
	index_buffer_size = sizeof(cube_indices);

	upload_buffer_data(
		dev,
		cube_verts,
		vertex_buffer_size,
		&vertex_buffer
	);

	app->vertex_buffer = vertex_buffer;

	//
	// Finally, initialize the vertex buffer view.
	//

	vbv = {};
	vbv.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
	vbv.StrideInBytes = sizeof(vertex);
	vbv.SizeInBytes = vertex_buffer_size;

	app->vertex_buffer_view = vbv;

	//
	// Now we will repeat the process for the index buffer.
	//

	upload_buffer_data(
		dev,
		cube_indices,
		index_buffer_size,
		&index_buffer
	);

	app->index_buffer = index_buffer;

	//
	// Finally, initialize the index buffer view.
	//

	ibv = {};
	ibv.BufferLocation = index_buffer->GetGPUVirtualAddress();
	ibv.SizeInBytes = index_buffer_size;
	ibv.Format = DXGI_FORMAT_R16_UINT;

	app->index_buffer_view = ibv;
}

void upload_buffer_data(
	ComPtr<ID3D12Device> dev,
	void* buffer_data,
	const UINT buffer_size,
	ID3D12Resource** resource_buffer
) {
	CD3DX12_HEAP_PROPERTIES heap_properties;
	CD3DX12_RESOURCE_DESC buffer_resource_desc;
	HRESULT result;
	UINT8* buffer_data_begin;
	CD3DX12_RANGE read_range(0, 0);

	heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size);

	// Note: using upload heaps to transfer static data like vert
	// buffers is not recommended. Every time the GPU needs it, the
	// upload heap will be marshalled over. Please read up on Default
	// Heap Usage. An upload heap is used here for code simplicity,
	// and because there are very few verts to actually transfer.
	result = dev->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&buffer_resource_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(resource_buffer)
	);

	throw_if_failed(result);

	//
	// Copy the cube vertex data over to the vertex buffer.
	//

	buffer_data_begin = NULL;
	result = (*resource_buffer)->Map(0, &read_range, (void**)(&buffer_data_begin));
	throw_if_failed(result);
	memcpy(buffer_data_begin, buffer_data, buffer_size);
	(*resource_buffer)->Unmap(0, NULL);
}

void create_texture(application* app) {
	D3D12_RESOURCE_DESC texture_desc;
	ComPtr<ID3D12Resource> texture;
	ComPtr<ID3D12Device> dev;
	ComPtr<ID3D12GraphicsCommandList> command_list;
	ComPtr<ID3D12CommandQueue> command_queue;
	ComPtr<ID3D12DescriptorHeap> srv_heap;
	CD3DX12_HEAP_PROPERTIES default_heap;
	HRESULT result;
	UINT64 upload_buffer_size;
	CD3DX12_HEAP_PROPERTIES upload_heap;
	CD3DX12_RESOURCE_DESC upload_buff;
	ComPtr<ID3D12Resource> texture_upload_heap;
	vector<UINT8> texture_data;
	D3D12_SUBRESOURCE_DATA texture_subresource;
	CD3DX12_RESOURCE_BARRIER texture_upload_barrier;
	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;

	dev = app->dx12->device;
	command_list = app->dx12->command_list;
	command_queue = app->dx12->command_queue;
	srv_heap = app->dx12->srv_heap;

	//
	// First describe the texture for DX12.
	//

	texture_desc = {};
	texture_desc.MipLevels = 1;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.Width = TEXTURE_W;
	texture_desc.Height = TEXTURE_H;
	texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	texture_desc.DepthOrArraySize = 1;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	//
	// Now create the texture.
	//

	default_heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	result = dev->CreateCommittedResource(
		&default_heap,
		D3D12_HEAP_FLAG_NONE,
		&texture_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		NULL,
		IID_PPV_ARGS(&texture)
	);

	throw_if_failed(result);

	upload_buffer_size = GetRequiredIntermediateSize(
		texture.Get(),
		0,
		1
	);

	//
	// Now create the GPU upload buffer
	//

	upload_heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	upload_buff = CD3DX12_RESOURCE_DESC::Buffer(upload_buffer_size);
	result = dev->CreateCommittedResource(
		&upload_heap,
		D3D12_HEAP_FLAG_NONE,
		&upload_buff,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&texture_upload_heap)
	);

	throw_if_failed(result);

	//
	// Procedurally generate a texture.
	//

	texture_data = generate_texture_data();

	//
	// Upload the actual texture data using the upload heap.
	// 
	// One thing - what exactly are RowPitch and SlicePitch? They aren't
	// just the width and height of the texture. But basically it goes like
	// this: for a 2D texture, RowPitch is the size in bytes of a single row
	// of pixels. So consider a texture where each pixel has an RGBA component,
	// and each of these is a byte. Then a single pixel is 4 bytes in size.
	// Now if the width of a texture is 64 pixels, then its 64 * 4 = 256
	// bytes.
	// 
	// The slice is how many total bytes are needed for a single 2D texture.
	// Thus if our texture is 64 x 64 pixels, then the SlicePitch is 
	// 64 * 64 * 4 = 16384 bytes.
	//

	texture_subresource = {};
	texture_subresource.pData = texture_data.data();
	texture_subresource.RowPitch = TEXTURE_W * TEXTURE_PIXEL_SIZE;
	texture_subresource.SlicePitch = texture_subresource.RowPitch * TEXTURE_H;

	UpdateSubresources(
		command_list.Get(),
		texture.Get(),
		texture_upload_heap.Get(),
		0,
		0,
		1,
		&texture_subresource
	);

	texture_upload_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		texture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	command_list->ResourceBarrier(1, &texture_upload_barrier);

	//
	// Lastly, create the SRV for the texture.
	//

	srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = texture_desc.Format;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;
	dev->CreateShaderResourceView(
		texture.Get(),
		&srv_desc,
		srv_heap->GetCPUDescriptorHandleForHeapStart()
	);

	//
	// Finally, close the command list.
	//

	result = command_list->Close();
	throw_if_failed(result);
	ID3D12CommandList* commands[] = { command_list.Get() };
	command_queue->ExecuteCommandLists(
		_countof(commands),
		commands
	);

	wait_for_previous_frame(app->dx12);

	app->texture = texture;
}

vector<UINT8> generate_texture_data() {

	//
	// Creates a simple black and white checker-board
	// texture.
	//

	vector<UINT8> result;
	UINT row_pitch;
	// Width and height of a cell in the checkerboard.
	UINT cell_pitch;
	UINT cell_height;
	UINT texture_size;
	UINT n;
	UINT x;
	UINT y;
	UINT i;
	UINT j;
	UINT8* data;

	row_pitch = TEXTURE_W * TEXTURE_PIXEL_SIZE;
	cell_pitch = row_pitch >> 3;
	cell_height = TEXTURE_W >> 3;
	texture_size = row_pitch * TEXTURE_H;

	result.resize(texture_size);
	data = result.data();

	for (n = 0; n < texture_size; n += TEXTURE_PIXEL_SIZE) {
		x = n % row_pitch;
		y = n / row_pitch;
		i = x / cell_pitch;
		j = y / cell_height;

		if (i % 2 == j % 2) {
			data[n + 0] = 0x00;
			data[n + 1] = 0x00;
			data[n + 2] = 0x00;
			data[n + 3] = 0xff;
		} else {
			data[n + 0] = 0xff;
			data[n + 1] = 0xff;
			data[n + 2] = 0xff;
			data[n + 3] = 0xff;
		}
	}

	return result;
}

void frame(application* app) {
	update(app);
	render(app);
}

void update(application* app) {

}

void render(application* app) {
	dx12_handler* dx12;
	ComPtr<ID3D12GraphicsCommandList> command_list;
	ComPtr<ID3D12CommandQueue> command_queue;
	ComPtr<IDXGISwapChain3> swap_chain;
	HRESULT result;

	dx12 = app->dx12;
	command_list = dx12->command_list;
	command_queue = dx12->command_queue;
	swap_chain = dx12->swap_chain;

	//
	// Record all the commands we need to render the scene into
	// the command list.
	//

	populate_command_list(app);

	//
	// Execute the command list.
	//

	ID3D12CommandList* command_lists[] = { command_list.Get() };
	command_queue->ExecuteCommandLists(
		_countof(command_lists),
		command_lists
	);

	//
	// Lastly, draw the frame.
	//

	result = swap_chain->Present(1, 0);
	throw_if_failed(result);

	//
	// Do synchronization step.
	//

	wait_for_previous_frame(dx12);
}

void populate_command_list(application* app) {
	dx12_handler* dx12;
	HRESULT result;
	ComPtr<ID3D12CommandAllocator> command_allocator;
	ComPtr<ID3D12GraphicsCommandList> command_list;
	UINT frame_index;
	ComPtr<ID3D12Resource> back_buffer;
	ComPtr<ID3D12DescriptorHeap> rtv_heap;
	UINT rtv_descriptor_size;
	ComPtr<ID3D12DescriptorHeap> srv_heap;
	ComPtr<ID3D12PipelineState> pipeline_state;
	CD3DX12_RESOURCE_BARRIER barrier_render_target;
	CD3DX12_RESOURCE_BARRIER barrier_present;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle;

	dx12 = app->dx12;
	command_allocator = dx12->command_allocator;
	command_list = dx12->command_list;
	frame_index = dx12->frame_index;
	back_buffer = dx12->render_targets[frame_index];
	rtv_heap = dx12->rtv_heap;
	rtv_descriptor_size = dx12->rtv_descriptor_size;
	srv_heap = dx12->srv_heap;
	pipeline_state = app->pipeline_state;

	//
	// First step is to reset the command allocator. The command
	// allocator's job is to be a memory manager for a command list.
	// So if we want to use the command list, we need to make sure
	// its memory is reset. However, we need to ensure it is reset
	// ONLY when no commands are in-flight. We ensure this is so
	// with our fence as we will see shortly.
	//

	result = command_allocator->Reset();
	throw_if_failed(result);

	//
	// Next we need to reset the command list itself as if the command
	// list is brand new. This way we can record new commands. It's
	// kind of like fixing pointers: until we do this step, the command
	// list is likely pointing to commands in the allocator that no
	// longer exist!
	//

	result = command_list->Reset(command_allocator.Get(), pipeline_state.Get());
	throw_if_failed(result);

	//
	// Next, set all the pipeline state
	//

	command_list->SetGraphicsRootSignature(app->root_signature.Get());

	ID3D12DescriptorHeap* heaps[] = { srv_heap.Get() };
	command_list->SetDescriptorHeaps(
		_countof(heaps),
		heaps
	);

	command_list->SetGraphicsRootDescriptorTable(
		0,
		srv_heap->GetGPUDescriptorHandleForHeapStart()
	);

	command_list->RSSetViewports(
		1,
		&(app->viewport)
	);

	command_list->RSSetScissorRects(
		1,
		&(app->scissor_rect)
	);

	//
	// Now we can begin to clear the render target. To do so,
	// we first need to make sure the current render target is
	// transitioned from the PRESENT state to RENDER_TARGET.
	//

	barrier_render_target = CD3DX12_RESOURCE_BARRIER::Transition(
		back_buffer.Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	command_list->ResourceBarrier(1, &barrier_render_target);

	//
	// Get the RTV for the current back buffer.
	//

	rtv_handle.InitOffsetted(
		rtv_heap->GetCPUDescriptorHandleForHeapStart(),
		frame_index,
		rtv_descriptor_size
	);

	command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, NULL);

	//
	// Now actually record the commands.
	//

	// Set the clear color.
	command_list->ClearRenderTargetView(
		rtv_handle,
		app->clear_color,
		0,
		NULL
	);

	// Draw the cube.
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command_list->IASetVertexBuffers(0, 1, &(app->vertex_buffer_view));
	command_list->IASetIndexBuffer(&(app->index_buffer_view));
	command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);

	//
	// Once our commands are done, close the command list.
	//

	barrier_present = CD3DX12_RESOURCE_BARRIER::Transition(
		back_buffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	command_list->ResourceBarrier(1, &barrier_present);

	result = command_list->Close();
	throw_if_failed(result);
}

void shutdown_application(application* app) {
	if (app->dx12) {
		shutdown_directx_12(app->dx12);
		delete app->dx12;
		app->dx12 = NULL;
	}
}