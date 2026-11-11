#include "DX12Renderer.h"
#include "Engine/Renderer/DirectX/DirectXHelper.h"

#include <stdexcept>
#include <chrono>
#include <wincodec.h>
#include <cmath>
#include <algorithm>

#pragma comment(lib, "windowscodecs.lib")

namespace Renderer
{
    const Color Color::Red(1.0f, 0.0f, 0.0f, 1.0f);
    const Color Color::Green(0.0f, 1.0f, 0.0f, 1.0f);
    const Color Color::Blue(0.0f, 0.0f, 1.0f, 1.0f);
    const Color Color::White(1.0f, 1.0f, 1.0f, 1.0f);
    const Color Color::Black(0.0f, 0.0f, 0.0f, 1.0f);
    const Color Color::Yellow(1.0f, 1.0f, 0.0f, 1.0f);
    const Color Color::Cyan(0.0f, 1.0f, 1.0f, 1.0f);
    const Color Color::Magenta(1.0f, 0.0f, 1.0f, 1.0f);

    // 8x8 Bitmap font data for printable ASCII 32 through 126 (95 characters, 8 bytes each)
    static const unsigned char Font8x8Data[95][8] =
    {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // ' '
        { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00 }, // '!'
        { 0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00 }, // '"'
        { 0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00 }, // '#'
        { 0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00 }, // '$'
        { 0x00, 0x63, 0x66, 0x0C, 0x18, 0x33, 0x63, 0x00 }, // '%'
        { 0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00 }, // '&'
        { 0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 }, // '''
        { 0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00 }, // '('
        { 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00 }, // ')'
        { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00 }, // '*'
        { 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00 }, // '+'
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 }, // ','
        { 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00 }, // '-'
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 }, // '.'
        { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x00 }, // '/'
        { 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00 }, // '0'
        { 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00 }, // '1'
        { 0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00 }, // '2'
        { 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00 }, // '3'
        { 0x0C, 0x1C, 0x34, 0x64, 0x7E, 0x04, 0x0E, 0x00 }, // '4'
        { 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00 }, // '5'
        { 0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00 }, // '6'
        { 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00 }, // '7'
        { 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00 }, // '8'
        { 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00 }, // '9'
        { 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00 }, // ':'
        { 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x00 }, // ';'
        { 0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00 }, // '<'
        { 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00 }, // '='
        { 0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00 }, // '>'
        { 0x3C, 0x66, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00 }, // '?'
        { 0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x62, 0x3C, 0x00 }, // '@'
        { 0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00 }, // 'A'
        { 0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00 }, // 'B'
        { 0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00 }, // 'C'
        { 0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00 }, // 'D'
        { 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00 }, // 'E'
        { 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00 }, // 'F'
        { 0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3E, 0x00 }, // 'G'
        { 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00 }, // 'H'
        { 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00 }, // 'I'
        { 0x0E, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00 }, // 'J'
        { 0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00 }, // 'K'
        { 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00 }, // 'L'
        { 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00 }, // 'M'
        { 0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00 }, // 'N'
        { 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00 }, // 'O'
        { 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00 }, // 'P'
        { 0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00 }, // 'Q'
        { 0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00 }, // 'R'
        { 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00 }, // 'S'
        { 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 }, // 'T'
        { 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00 }, // 'U'
        { 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00 }, // 'V'
        { 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x63, 0x00 }, // 'W'
        { 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00 }, // 'X'
        { 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00 }, // 'Y'
        { 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00 }, // 'Z'
        { 0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00 }, // '['
        { 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x00 }, // '\'
        { 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00 }, // ']'
        { 0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00 }, // '^'
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00 }, // '_'
        { 0x30, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00 }, // '`'
        { 0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00 }, // 'a'
        { 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00 }, // 'b'
        { 0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C, 0x00 }, // 'c'
        { 0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00 }, // 'd'
        { 0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00 }, // 'e'
        { 0x0E, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00 }, // 'f'
        { 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x7C }, // 'g'
        { 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00 }, // 'h'
        { 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00 }, // 'i'
        { 0x06, 0x00, 0x0E, 0x06, 0x06, 0x66, 0x3C, 0x00 }, // 'j'
        { 0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00 }, // 'k'
        { 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00 }, // 'l'
        { 0x00, 0x00, 0x66, 0x7F, 0x7F, 0x6B, 0x63, 0x00 }, // 'm'
        { 0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00 }, // 'n'
        { 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00 }, // 'o'
        { 0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60 }, // 'p'
        { 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06 }, // 'q'
        { 0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x00 }, // 'r'
        { 0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00 }, // 's'
        { 0x18, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0E, 0x00 }, // 't'
        { 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00 }, // 'u'
        { 0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00 }, // 'v'
        { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00 }, // 'w'
        { 0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00 }, // 'x'
        { 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x7C }, // 'y'
        { 0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00 }, // 'z'
        { 0x0E, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0E, 0x00 }, // '{'
        { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 }, // '|'
        { 0x70, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x70, 0x00 }, // '}'
        { 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // '~'
    };

    DX12Renderer::DX12Renderer(int windowWidth, int windowHeight, std::function<void()> Start, std::function<void(float)> Update, std::string windowName)
        : width(windowWidth), height(windowHeight), OnStart(Start), OnUpdate(Update), isRunning(true),
          currentlyBoundTexture(nullptr), srvDescriptorAllocatedCount(0), mappedVertexData(nullptr),
          currentVertexOffset(0), frameIndex(0), fenceValue(0), fenceEvent(nullptr)
    {
        currentClearColor = Color(0.0f, 0.2f, 0.4f, 1.0f);

        InitializeWindow(windowName);
        InitializePipeline();
        InitializeAssets();
        InitializeDefaultTextures();
    }

    DX12Renderer::~DX12Renderer()
    {
        // 1. Ensure GPU finishes before cleanup
        if (device && commandQueue && fence)
        {
            WaitForPreviousFrame();
        }

        // 2. Unmap and release vertex buffer
        if (vertexBuffer)
        {
            vertexBuffer->Unmap(0, nullptr);
            mappedVertexData = nullptr;
            vertexBuffer.Reset();
        }

        // 3. Destroy all texture resources
        userTextures.clear();
        whiteTexture.reset();
        fontTexture.reset();

        // 4. Release render targets before swap chain!
        for (UINT n = 0; n < FrameCount; n++)
        {
            renderTargets[n].Reset();
        }

        // 5. Release swap chain before device
        swapChain.Reset();

        // 6. Release pipeline state, signature, and heaps
        pipelineState.Reset();
        rootSignature.Reset();
        rtvHeap.Reset();
        srvHeap.Reset();

        // 7. Release command list and allocator
        commandList.Reset();
        commandAllocator.Reset();

        // 8. Close synchronization handles
        if (fenceEvent)
        {
            CloseHandle(fenceEvent);
            fenceEvent = nullptr;
        }
        fence.Reset();
        commandQueue.Reset();

        // 9. Release device last
        device.Reset();

        // 10. Destroy window handle if still around
        if (windowHandle)
        {
            DestroyWindow(windowHandle);
            windowHandle = nullptr;
        }
    }

    void DX12Renderer::InitializeWindow(const std::string& windowName)
    {
        HINSTANCE hInstance = GetModuleHandle(NULL);

        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = &DX12Renderer::WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = "DX12RendererWindow";

        RegisterClassEx(&windowClass);

        RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        windowHandle = CreateWindowA(
            windowClass.lpszClassName,
            windowName.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            hInstance,
            nullptr);

        if (!windowHandle)
        {
            throw std::runtime_error("CreateWindowA failed. Error: " + std::to_string(GetLastError()));
        }

        ShowWindow(windowHandle, SW_NORMAL);
    }

    void DX12Renderer::InitializePipeline()
    {
#if defined(_DEBUG)
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
#endif

        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        scissorRect.left = 0;
        scissorRect.top = 0;
        scissorRect.right = static_cast<LONG>(width);
        scissorRect.bottom = static_cast<LONG>(height);

        Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "CreateDXGIFactory1");

        Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        if (hardwareAdapter)
        {
            ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)), "D3D12CreateDevice");
        }
        else
        {
            Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
            ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)), "factory->EnumWarpAdapter");
            ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)), "D3D12CreateDevice WARP");
        }

        // Create Command Queue
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)), "CreateCommandQueue");

        // Create Swap Chain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.OutputWindow = windowHandle;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;

        Microsoft::WRL::ComPtr<IDXGISwapChain> tempSwapChain;
        ThrowIfFailed(factory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, &tempSwapChain), "CreateSwapChain");
        ThrowIfFailed(tempSwapChain.As(&swapChain), "swapChain.As");
        ThrowIfFailed(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER), "MakeWindowAssociation");

        frameIndex = swapChain->GetCurrentBackBufferIndex();

        // Create RTV Descriptor Heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)), "CreateDescriptorHeap RTV");
        rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create SRV Descriptor Heap
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = MaxDescriptors;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)), "CreateDescriptorHeap SRV");
        srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Create Frame Resources
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])), "swapChain->GetBuffer");
            device->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.ptr += rtvDescriptorSize;
        }

        ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)), "CreateCommandAllocator");
    }

    void DX12Renderer::InitializeAssets()
    {
        // Root Signature: 1 Descriptor Table for Texture SRV at register t0, 1 Static Sampler at register s0
        D3D12_DESCRIPTOR_RANGE descriptorRange = {};
        descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descriptorRange.NumDescriptors = 1;
        descriptorRange.BaseShaderRegister = 0;
        descriptorRange.RegisterSpace = 0;
        descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER rootParameters[1] = {};
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
        rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange;

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = 1;
        rootSignatureDesc.pParameters = rootParameters;
        rootSignatureDesc.NumStaticSamplers = 1;
        rootSignatureDesc.pStaticSamplers = &sampler;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error), "D3D12SerializeRootSignature");
        ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "CreateRootSignature");

        // Compile 2D Shaders
        Microsoft::WRL::ComPtr<ID3DBlob> vertexShader = CompileShader(L"Shaders/2d_shaders.hlsl", "VSMain", "vs_5_0");
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShader = CompileShader(L"Shaders/2d_shaders.hlsl", "PSMain", "ps_5_0");

        // Input Layout
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Pipeline State with Alpha Blending
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
        psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
        psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

        D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
        blendDesc.BlendEnable = TRUE;
        blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
        psoDesc.BlendState.IndependentBlendEnable = FALSE;
        psoDesc.BlendState.RenderTarget[0] = blendDesc;

        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)), "CreateGraphicsPipelineState");

        // Create Command List
        ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)), "CreateCommandList");
        ThrowIfFailed(commandList->Close(), "CommandList->Close");

        // Create Dynamic Vertex Buffer (Upload Heap)
        const UINT vertexBufferSize = MaxBatchVertices * sizeof(Vertex2D);
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = vertexBufferSize;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexBuffer)), "CreateCommittedResource VertexBuffer");

        D3D12_RANGE readRange = { 0, 0 };
        ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedVertexData)), "vertexBuffer->Map");

        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = sizeof(Vertex2D);
        vertexBufferView.SizeInBytes = vertexBufferSize;

        // Synchronization Fence & Event
        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "CreateFence");
        fenceValue = 1;
        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!fenceEvent)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "CreateEvent");
        }
    }

    void DX12Renderer::InitializeDefaultTextures()
    {
        // 1. Create Default 1x1 White Texture
        const unsigned char whitePixel[4] = { 255, 255, 255, 255 };
        whiteTexture.reset(CreateTexture(1, 1, whitePixel));

        // 2. Create Custom 8x8 Font Atlas Texture (128x64 RGBA: 16 cols x 6 rows = 96 glyphs)
        const int atlasWidth = 128;
        const int atlasHeight = 64;
        std::vector<unsigned char> fontPixels(atlasWidth * atlasHeight * 4, 0);

        for (int charIndex = 0; charIndex < 95; charIndex++)
        {
            int col = charIndex % 16;
            int row = charIndex / 16;
            int startX = col * 8;
            int startY = row * 8;

            for (int y = 0; y < 8; y++)
            {
                unsigned char rowByte = Font8x8Data[charIndex][y];
                for (int x = 0; x < 8; x++)
                {
                    bool pixelOn = (rowByte & (0x80 >> x)) != 0;
                    int pixelIndex = ((startY + y) * atlasWidth + (startX + x)) * 4;

                    if (pixelOn)
                    {
                        fontPixels[pixelIndex + 0] = 255;
                        fontPixels[pixelIndex + 1] = 255;
                        fontPixels[pixelIndex + 2] = 255;
                        fontPixels[pixelIndex + 3] = 255;
                    }
                    else
                    {
                        fontPixels[pixelIndex + 0] = 0;
                        fontPixels[pixelIndex + 1] = 0;
                        fontPixels[pixelIndex + 2] = 0;
                        fontPixels[pixelIndex + 3] = 0;
                    }
                }
            }
        }

        fontTexture.reset(CreateTexture(atlasWidth, atlasHeight, fontPixels.data()));
        currentlyBoundTexture = whiteTexture.get();
    }

    float DX12Renderer::ToNdcX(int x) const
    {
        return (static_cast<float>(x) / static_cast<float>(width)) * 2.0f - 1.0f;
    }

    float DX12Renderer::ToNdcY(int y) const
    {
        return 1.0f - (static_cast<float>(y) / static_cast<float>(height)) * 2.0f;
    }

    Texture* DX12Renderer::CreateTexture(const int textureWidth, const int textureHeight, const unsigned char* rgbaPixels)
    {
        if (srvDescriptorAllocatedCount >= MaxDescriptors)
        {
            throw std::runtime_error("Maximum number of texture descriptors reached");
        }

        auto newTexture = std::make_unique<Texture>();
        newTexture->width = textureWidth;
        newTexture->height = textureHeight;

        // Texture Resource in Default Heap
        D3D12_HEAP_PROPERTIES defaultHeapProps = {};
        defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Width = textureWidth;
        textureDesc.Height = textureHeight;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

        ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&newTexture->resource)), "CreateCommittedResource Texture");

        // Upload Heap for copy
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
        UINT numRows;
        UINT64 rowSizeInBytes;
        UINT64 totalBytes;
        device->GetCopyableFootprints(&textureDesc, 0, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

        D3D12_HEAP_PROPERTIES uploadHeapProps = {};
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC uploadBufferDesc = {};
        uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadBufferDesc.Width = totalBytes;
        uploadBufferDesc.Height = 1;
        uploadBufferDesc.DepthOrArraySize = 1;
        uploadBufferDesc.MipLevels = 1;
        uploadBufferDesc.SampleDesc.Count = 1;
        uploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
        ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer)), "CreateCommittedResource TextureUpload");

        BYTE* mappedUploadData = nullptr;
        ThrowIfFailed(uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedUploadData)), "uploadBuffer->Map");

        for (UINT y = 0; y < numRows; y++)
        {
            memcpy(mappedUploadData + layout.Offset + (y * layout.Footprint.RowPitch),
                   rgbaPixels + (y * textureWidth * 4),
                   textureWidth * 4);
        }
        uploadBuffer->Unmap(0, nullptr);

        // Execute Copy on Command List
        ThrowIfFailed(commandAllocator->Reset(), "commandAllocator->Reset (Texture)");
        ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr), "commandList->Reset (Texture)");

        D3D12_TEXTURE_COPY_LOCATION dst = {};
        dst.pResource = newTexture->resource.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = uploadBuffer.Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint = layout;

        commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = newTexture->resource.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);

        ThrowIfFailed(commandList->Close(), "commandList->Close (Texture)");
        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(1, ppCommandLists);

        WaitForPreviousFrame();

        // Create SRV in Descriptor Heap
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
        cpuHandle.ptr += static_cast<SIZE_T>(srvDescriptorAllocatedCount) * srvDescriptorSize;

        newTexture->gpuDescriptorHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
        newTexture->gpuDescriptorHandle.ptr += static_cast<UINT64>(srvDescriptorAllocatedCount) * srvDescriptorSize;

        device->CreateShaderResourceView(newTexture->resource.Get(), &srvDesc, cpuHandle);
        srvDescriptorAllocatedCount++;

        Texture* resultPointer = newTexture.get();
        userTextures.push_back(std::move(newTexture));
        return resultPointer;
    }

    Texture* DX12Renderer::LoadTexture(const std::string& filePath)
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to initialize WIC Imaging Factory");
        }

        std::wstring wideFilePath(filePath.begin(), filePath.end());

        Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
        hr = wicFactory->CreateDecoderFromFilename(wideFilePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to load texture file: " + filePath);
        }

        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        ThrowIfFailed(decoder->GetFrame(0, &frame), "decoder->GetFrame");

        Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
        ThrowIfFailed(wicFactory->CreateFormatConverter(&converter), "CreateFormatConverter");
        ThrowIfFailed(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom), "converter->Initialize");

        UINT texWidth = 0;
        UINT texHeight = 0;
        ThrowIfFailed(converter->GetSize(&texWidth, &texHeight), "converter->GetSize");

        std::vector<unsigned char> pixels(texWidth * texHeight * 4);
        ThrowIfFailed(converter->CopyPixels(nullptr, texWidth * 4, static_cast<UINT>(pixels.size()), pixels.data()), "converter->CopyPixels");

        return CreateTexture(static_cast<int>(texWidth), static_cast<int>(texHeight), pixels.data());
    }

    void DX12Renderer::DestroyTexture(Texture* texture)
    {
        if (!texture) return;
        for (auto it = userTextures.begin(); it != userTextures.end(); ++it)
        {
            if (it->get() == texture)
            {
                userTextures.erase(it);
                break;
            }
        }
    }

    void DX12Renderer::Start()
    {
        OnStart();

        auto lastTime = std::chrono::high_resolution_clock::now();

        while (isRunning)
        {
            MSG msg = {};
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    isRunning = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (!isRunning)
            {
                break;
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            BeginFrame();
            OnUpdate(deltaTime);
            EndFrame();
        }

        // Clean sync when closing
        if (device && commandQueue && fence)
        {
            WaitForPreviousFrame();
        }
    }

    void DX12Renderer::Clear(Color clearColor)
    {
        currentClearColor = clearColor;

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += static_cast<SIZE_T>(frameIndex) * rtvDescriptorSize;

        const float clearColorArray[] = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
        commandList->ClearRenderTargetView(rtvHandle, clearColorArray, 0, nullptr);
    }

    void DX12Renderer::SetActiveTexture(const Texture* texture)
    {
        const Texture* target = texture ? texture : whiteTexture.get();
        if (currentlyBoundTexture != target)
        {
            FlushBatch();
            currentlyBoundTexture = target;
        }
    }

    void DX12Renderer::FlushBatch()
    {
        if (batchedVertices.empty())
        {
            return;
        }

        if (!currentlyBoundTexture)
        {
            currentlyBoundTexture = whiteTexture.get();
        }

        UINT batchVertexCount = static_cast<UINT>(batchedVertices.size());

        if (currentVertexOffset + batchVertexCount > MaxBatchVertices)
        {
            batchedVertices.clear();
            return;
        }

        // Copy vertices to mapped upload heap buffer at current slice offset
        memcpy(mappedVertexData + currentVertexOffset, batchedVertices.data(), batchVertexCount * sizeof(Vertex2D));

        // Bind active texture SRV descriptor table to root parameter 0
        commandList->SetGraphicsRootDescriptorTable(0, currentlyBoundTexture->gpuDescriptorHandle);

        // Draw instanced using currentVertexOffset as the start vertex location
        commandList->DrawInstanced(batchVertexCount, 1, currentVertexOffset, 0);

        currentVertexOffset += batchVertexCount;
        batchedVertices.clear();
    }

    void DX12Renderer::BeginFrame()
    {
        ThrowIfFailed(commandAllocator->Reset(), "commandAllocator->Reset");
        ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()), "commandList->Reset");

        // Transition backbuffer to RENDER_TARGET
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = renderTargets[frameIndex].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += static_cast<SIZE_T>(frameIndex) * rtvDescriptorSize;

        const float clearColorArray[] = { currentClearColor.r, currentClearColor.g, currentClearColor.b, currentClearColor.a };
        commandList->ClearRenderTargetView(rtvHandle, clearColorArray, 0, nullptr);
        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        commandList->SetGraphicsRootSignature(rootSignature.Get());

        ID3D12DescriptorHeap* descriptorHeaps[] = { srvHeap.Get() };
        commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

        currentVertexOffset = 0;
        currentlyBoundTexture = nullptr;
    }

    void DX12Renderer::EndFrame()
    {
        // Flush any remaining batched primitives
        FlushBatch();

        // Transition backbuffer to PRESENT
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = renderTargets[frameIndex].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);

        ThrowIfFailed(commandList->Close(), "commandList->Close");

        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(1, ppCommandLists);

        ThrowIfFailed(swapChain->Present(1, 0), "swapChain->Present");
        WaitForPreviousFrame();
    }

    void DX12Renderer::WaitForPreviousFrame()
    {
        const UINT64 fenceToWaitFor = fenceValue;
        ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceToWaitFor), "commandQueue->Signal");
        fenceValue++;

        if (fence->GetCompletedValue() < fenceToWaitFor)
        {
            ThrowIfFailed(fence->SetEventOnCompletion(fenceToWaitFor, fenceEvent), "fence->SetEventOnCompletion");
            WaitForSingleObject(fenceEvent, INFINITE);
        }

        frameIndex = swapChain->GetCurrentBackBufferIndex();
    }

    void DX12Renderer::DrawFilledRectangle(const int startXPosition, const int startYPosition, const int endXPosition, const int endYPosition, Color color)
    {
        SetActiveTexture(whiteTexture.get());

        if (batchedVertices.size() + 6 > MaxBatchVertices)
        {
            FlushBatch();
        }

        float x1 = ToNdcX(startXPosition);
        float y1 = ToNdcY(startYPosition);
        float x2 = ToNdcX(endXPosition);
        float y2 = ToNdcY(endYPosition);

        DirectX::XMFLOAT4 c(color.r, color.g, color.b, color.a);
        DirectX::XMFLOAT2 uv(0.0f, 0.0f);

        batchedVertices.push_back({ DirectX::XMFLOAT2(x1, y1), c, uv });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x2, y1), c, uv });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x1, y2), c, uv });

        batchedVertices.push_back({ DirectX::XMFLOAT2(x1, y2), c, uv });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x2, y1), c, uv });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x2, y2), c, uv });
    }

    void DX12Renderer::DrawRectangle(const int startXPosition, const int startYPosition, const int endXPosition, const int endYPosition, Color color)
    {
        DrawFilledRectangle(startXPosition, startYPosition, endXPosition, startYPosition + 1, color); // Top
        DrawFilledRectangle(startXPosition, endYPosition - 1, endXPosition, endYPosition, color);     // Bottom
        DrawFilledRectangle(startXPosition, startYPosition, startXPosition + 1, endYPosition, color); // Left
        DrawFilledRectangle(endXPosition - 1, startYPosition, endXPosition, endYPosition, color);     // Right
    }

    void DX12Renderer::DrawLine(const int startXPosition, const int startYPosition, const int endXPosition, const int endYPosition, Color color)
  
        
       int dx = std::abs(endXPosition - startXPosition);
    {
        int dy = std::abs(endYPosition - startYPosition);
        if (dx >= dy)
        {
            int minX = (std::min)(startXPosition, endXPosition);
            int maxX = (std::max)(startXPosition, endXPosition);
            DrawFilledRectangle(minX, startYPosition, maxX + 1, startYPosition + 1, color);
        }
        else
        {
            int minY = (std::min)(startYPosition, endYPosition);
            int maxY = (std::max)(startYPosition, endYPosition);
            DrawFilledRectangle(startXPosition, minY, startXPosition + 1, maxY + 1, color);
        }
    }

    void DX12Renderer::DrawPixel(const int xPosition, const int yPosition, Color color)
    {
        DrawFilledRectangle(xPosition, yPosition, xPosition + 1, yPosition + 1, color);
    }

    void DX12Renderer::DrawCharacter(const int xPosition, const int yPosition, const char character, Color color, float scale)
    {
        if (character < 32 || character > 126)
        {
            return;
        }

        SetActiveTexture(fontTexture.get());

        if (batchedVertices.size() + 6 > MaxBatchVertices)
        {
            FlushBatch();
        }

        int charIndex = character - 32;
        int col = charIndex % 16;
        int row = charIndex / 16

      
        float u0 = (col * 8.0f) / 128.0f;
        float v0 = (row * 8.0f) / 64.0f;
        float u1 = ((col + 1) * 8.0f) / 128.0f;
        float v1 = ((row + 1) * 8.0f) / 64.0f;

        float x1 = ToNdcX(xPosition);
        float y1 = ToNdcY(yPosition);
        float x2 = ToNdcX(xPosition + static_cast<int>(8.0f * scale));
        float y2 = ToNdcY(yPosition + static_cast<int>(8.0f * scale));

        DirectX::XMFLOAT4 c(color.r, color.g, color.b, color.a);

        batchedVertices.push_back({ DirectX::XMFLOAT2(x1, y1), c, DirectX::XMFLOAT2(u0, v0) });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x2, y1), c, DirectX::XMFLOAT2(u1, v0) });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x1, y2), c, DirectX::XMFLOAT2(u0, v1) });

        batchedVertices.push_back({ DirectX::XMFLOAT2(x1, y2), c, DirectX::XMFLOAT2(u0, v1) });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x2, y1), c, DirectX::XMFLOAT2(u1, v0) });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x2, y2), c, DirectX::XMFLOAT2(u1, v1) });
    }

    void DX12Renderer::DrawString(const int xPosition, const int yPosition, const std::string& text, Color color, float scale)
    {
        int currentX = xPosition;
        int currentY = yPosition;
        const int charWidth = static_cast<int>(8.0f * scale);
        const int charHeight = static_cast<int>(10.0f * scale);

        for (char ch : text)
        {
            if (ch == '\n')
            {
                currentX = xPosition;
                currentY += charHeight;
                continue;
            }

            DrawCharacter(currentX, currentY, ch, color, scale);


           currentX += charWidth;
        }
    }

    void DX12Renderer::DrawTexture(const int xPosition, const int yPosition, const Texture* texture, Color tintColor)
    {
        if (!texture) return;
        DrawTexture(xPosition, yPosition, texture->width, texture->height, texture, tintColor);
    }

    void DX12Renderer::DrawTexture(const int xPosition, const int yPosition, const int destinationWidth, const int destinationHeight, const Texture* texture, Color tintColor)
    {
        if (!texture) return;
        DrawSprite(xPosition, yPosition, destinationWidth, destinationHeight, 0, 0, texture->width, texture->height, texture, tintColor);
    }

    void DX12Renderer::DrawSprite(const int xPosition, const int yPosition, const int destinationWidth, const int destinationHeight, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const Texture* texture, Color tintColor)
    {
        if (!texture) return;

        SetActiveTexture(texture);

        if (batchedVertices.size() + 6 > MaxBatchVertices)
        {
            FlushBatch();
        }

        float x1 = ToNdcX(xPosition);
        float y1 = ToNdcY(yPosition);
        float x2 = ToNdcX(xPosition + destinationWidth);
        float y2 = ToNdcY(yPosition + destinationHeight);

        float u0 = static_cast<float>(sourceX) / static_cast<float>(texture->width);
        float v0 = static_cast<float>(sourceY) / static_cast<float>(texture->height);
        flot u1 = static_cast<float>(sourceX + sourceWidth) / static_cast<float>(texture->width);
        float v1 = static_cast<float>(sourceY + sourceHeight) / static_cast<float>(texture->height);

        DirectX::XMFLOAT4 c(tintColor.r, tintColor.g, tintColor.b, tintColor.a);

        batched
            
           Vertices.push_back({ DirectX::XMFLOAT2(x1, y1), c, DirectX::XMFLOAT2(u0, v0) });
        btchedVertices.push_back({ DirectX::XMFLOAT2(x2, y1), c, DirectX::XMFLOAT2(u1, v0) });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x1, y2), c, DirectX::XMFLOAT2(u0, v1) });

        batchedVertices.push_back({ DirectX::XMFLOAT2(x1, y2), c, DirectX::XMFLOAT2(u0, v1) });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x2, y1), c, DirectX::XMFLOAT2(u1, v0) });
        batchedVertices.push_back({ DirectX::XMFLOAT2(x2, y2), c, DirectX::XMFLOAT2(u1, v1) });
    }

    const HWND* DX12Renderer::GetWindowHandlePointer() const
    {
        return &windowHandle;
    }

    int DX12Renderer::GetWidth() const
    {
        return width;
    }

    int DX12Renderer::GetHeight() const
    {
        return height;
    }

    LRESULT CALLBACK DX12Renderer::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcA(hWnd, message, wParam, lParam);
        }
    }

}