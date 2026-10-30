// MipMap.cpp : Defines the entry point for the application.
//

#include "MipMap.h"
#include "Engine/Renderer/ConsoleRenderer/ConsoleRenderer.h"
#include "Engine/Input/Win32Input.h"
#include "Engine/Renderer/DirectX/DirectXHelper.h"

#include <exception>
#include <d3d12.h>
#include <WinDef.h>
#include <dxgi.h>
#include <D3DCompiler.h>

void Old();

int main()
{
	try
	{
		D3D12HelloTriangle triangle(500, 300, "Working window Creation!");
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "Fatal error: " << error.what() << '\n';
		return 1;
	}
}


void test()
{

}

D3D12HelloTriangle::D3D12HelloTriangle(int width, int height, std::string name)
	: m_width(width), m_height(height), m_useWarpDevice(true), m_aspectRatio((float)width / (float)height)
{
	// Make WIndow!
	
	HINSTANCE hInstance = GetModuleHandle(NULL);;
	int nCmdShow;

	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = &D3D12HelloTriangle::WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "DXSampleClass";
	RegisterClassEx(&windowClass);

	if (!RegisterClassExA(&windowClass))
	{
		DWORD error = GetLastError();

		// A previously registered class is acceptable.
		if (error != ERROR_CLASS_ALREADY_EXISTS)
		{
			throw std::runtime_error(
				"RegisterClassExA failed. Win32 error: " +
				std::to_string(error));
		}
	}

	RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	
	
	// Create the window and store a handle to it.
	m_hwnd = CreateWindowA(
		windowClass.lpszClassName,
		name.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		hInstance, 
		nullptr);

	if (!m_hwnd)
	{
		DWORD error = GetLastError(); // Capture immediately.

		throw std::runtime_error(
			"CreateWindowA failed. Win32 error: " +
			std::to_string(GetLastError())
		);
	}

	// Initialize the sample. OnInit is defined in each child-implementation of DXSample.
	OnInit();

	ShowWindow(m_hwnd, SW_NORMAL);

	// Main sample loop.
	MSG msg = {};
	while (1)
	{
		OnUpdate();
		OnRender();

		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	OnDestroy();

	 
}

void D3D12HelloTriangle::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void D3D12HelloTriangle::OnUpdate()
{}

void D3D12HelloTriangle::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0), "m_swapChain->Present");

	WaitForPreviousFrame();
}

void D3D12HelloTriangle::OnDestroy()
{}

void D3D12HelloTriangle::LoadPipeline()
{
#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{

		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = static_cast<float>(m_width);
	m_viewport.Height = static_cast<float>(m_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = static_cast<LONG>(m_width);
	m_scissorRect.bottom = static_cast<LONG>(m_height);


	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "CreateDXGIFactory1");

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)), "factory->EnumWarpAdapter");

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		), "D3D12CreateDevice");
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		), "D3D12CreateDevice");
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)), "m_device->CreateCommandQueue");

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.BufferDesc.Width = m_width;
	swapChainDesc.BufferDesc.Height = m_height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = m_hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;

	ComPtr<IDXGISwapChain> swapChain;
	ThrowIfFailed(factory->CreateSwapChain(
		m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		&swapChainDesc,
		&swapChain
	), "factory->CreateSwapChain");

	ThrowIfFailed(swapChain.As(&m_swapChain), "swapChain.As");

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER), "factory->MakeWindowAssociation");

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)), "m_device->CreateDescriptorHeap");

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources.
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])), "m_swapChain->GetBuffer");
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.ptr += m_rtvDescriptorSize; // Move to the next descriptor slot
		}
	}

	//ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)), "m_device->CreateCommandAllocator");
	ThrowIfFailed(
		m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_commandAllocator.GetAddressOf())),
		"CreateCommandAllocator");
}

void D3D12HelloTriangle::LoadAssets()
{

	// Create an empty root signature.
	{
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.NumParameters = 0;
		rootSignatureDesc.pParameters = nullptr;
		rootSignatureDesc.NumStaticSamplers = 0;
		rootSignatureDesc.pStaticSamplers = nullptr;
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error), "D3D12SerializeRootSignature");
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)), "m_device->CreateRootSignature");
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader = CompileShader(
			L"Shaders/shaders.hlsl",
			"VSMain",
			"vs_5_0");

		ComPtr<ID3DBlob> pixelShader = CompileShader(
			L"Shaders/shaders.hlsl",
			"PSMain",
			"ps_5_0");

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
		//psoDesc.RasterizerState = D3D12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
		psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		psoDesc.RasterizerState.DepthClipEnable = TRUE;
		psoDesc.RasterizerState.MultisampleEnable = FALSE;
		psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
		psoDesc.RasterizerState.ForcedSampleCount = 0;
		psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		//psoDesc.BlendState = D3D12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
		psoDesc.BlendState.IndependentBlendEnable = FALSE;

		// RenderTarget[0] default (opaque / no blending)
		psoDesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
		psoDesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// Remaining render targets (1 through 7)
		for (UINT i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			psoDesc.BlendState.RenderTarget[i] = psoDesc.BlendState.RenderTarget[0];
		}

		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)), "m_device->CreateGraphicsPipelineState");
	}

	// Create the command list.

	if (!m_device)
	{
		throw std::runtime_error("Device was not created");
	}

	if (!m_commandAllocator)
	{
		throw std::runtime_error("Command allocator was not created");
	}

	ThrowIfFailed(
		m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_commandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(m_commandList.GetAddressOf())),
		"CreateCommandList");

	if (!m_commandList)
	{
		throw std::runtime_error(
			"CreateCommandList returned successfully but produced no command list");
	}

	ThrowIfFailed(
		m_commandList->Close(),
		"CommandList::Close");

	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		Vertex triangleVertices[] =
		{
			{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		D3D12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;	
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		 
		//auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = vertexBufferSize;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)), "m_device->CreateCommittedResource");

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		D3D12_RANGE readRange;
		readRange.Begin = 0;
		readRange.End = 0;
		ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)), "m_vertexBuffer->Map");
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "m_device->CreateFence");
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "HRESULT_FROM_WIN32(GetLastError())");
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}
}

void D3D12HelloTriangle::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocator->Reset(), "m_commandAllocator->Reset()");

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()), "m_commandList->Reset");

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.


	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &barrier);
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	rtvHandle.ptr = SIZE_T(INT64(m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr) + (INT64(m_frameIndex) * INT64(m_rtvDescriptorSize)));
	
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_commandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_commandList->Close(), "m_commandList->Close");
}

void D3D12HelloTriangle::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. More advanced samples 
	// illustrate how to use fences for efficient resource usage.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence), "m_commandQueue->Signal");
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent), "m_fence->SetEventOnCompletion");
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

LRESULT CALLBACK D3D12HelloTriangle::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProcA(hWnd, message, wParam, lParam);
	}
}

























void Start();
void Update(float deltaTime);

Renderer::ConsoleRenderer* renderer;
Input::InputManager* inputManager;

const POINT* mousePosition;



void Start()
{
	inputManager->SetMouseTracker((Input::MouseTracker)3);

	
	mousePosition = inputManager->GetMousePositionPointer();
}

int i = 0;
void Update(float deltaTime)
{

	inputManager->Update();
	
	renderer->HideCurser(true);

	if (inputManager->GetMouseButtonState(Input::Left) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Left Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Left) == Input::Held)
	{
		renderer->DrawString(4, 1, "Left Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Left) == Input::Released)
	{
		renderer->DrawString(4, 1, "Left Released");
	}
	else if (inputManager->GetMouseButtonState(Input::Middle) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Middle Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Middle) == Input::Held)
	{
		renderer->DrawString(4, 1, "Middle Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Middle) == Input::Released)
	{
		renderer->DrawString(4, 1, "Middle Released");
	}
	else if (inputManager->GetMouseButtonState(Input::Right) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Right Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Right) == Input::Held)
	{
		renderer->DrawString(4, 1, "Right Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Right) == Input::Released)
	{
		renderer->DrawString(4, 1, "Right Released");
	}
	else if (inputManager->GetMouseButtonState(Input::Side1) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Side1 Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Side1) == Input::Held)
	{
		renderer->DrawString(4, 1, "Side1 Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Side1) == Input::Released)
	{
		renderer->DrawString(4, 1, "Side1 Released");
	}
	else if (inputManager->GetMouseButtonState(Input::Side2) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Side2 Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Side2) == Input::Held)
	{
		renderer->DrawString(4, 1, "Side2 Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Side2) == Input::Released)
	{
		renderer->DrawString(4, 1, "Side2 Released");
	}
	else
	{
		i++;
		if (i > 10000)
		{
			renderer->DrawString(4, 1, "                          ");
			renderer->DrawString(4, 5, "        ");
			renderer->DrawString(4, 5, std::to_string(i));
			renderer->DrawString(1, 2, "X:     Y:                             ");
			renderer->DrawString(4, 2, std::to_string(mousePosition->x));
			renderer->DrawString(11, 2, std::to_string(mousePosition->y));
		}
		return;
	}


	i = 0;

	renderer->DrawString(4, 5, "        ");
	renderer->DrawString(4, 5, std::to_string(i));
	renderer->DrawString(1, 2, "X:     Y:                             ");
	renderer->DrawString(4, 2, std::to_string(mousePosition->x));
	renderer->DrawString(11, 2, std::to_string(mousePosition->y));
	
}


void Old()
{
	HANDLE consoleInputHandle = nullptr;

	renderer = new Renderer::ConsoleRenderer((short)20, (short)8, (short)16, (short)18, &Start, &Update);
	inputManager = new Input::InputManager(renderer->GetWindowHandlePointer());

	inputManager->RegisterKey(Input::A);

	renderer->Start();

	delete renderer;
	delete inputManager;

}