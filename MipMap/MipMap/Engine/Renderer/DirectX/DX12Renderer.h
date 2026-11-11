#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <directxmath.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace Renderer
{
    struct Color
    {
        float r;
        float g;
        float b;
        float a;

        Color()
            : r(0.0f), g(0.0f), b(0.0f), a(1.0f)
        {
        }

        Color(float r, float g, float b, float a = 1.0f)
            : r(r), g(g), b(b), a(a)
        {
        }

        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color White;
        static const Color Black;
        static const Color Yellow;
        static const Color Cyan;
        static const Color Magenta;
    };

    struct Vertex2D
    {
        DirectX::XMFLOAT2 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 uv;
    };

    struct Texture
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
        int width;
        int height;
    };

    class DX12Renderer
    {
    public:
        DX12Renderer(int windowWidth, int windowHeight, std::function<void()> Start, std::function<void(float)> Update, std::string windowName = "2D DirectX 12 Engine");
        ~DX12Renderer();

        void Start();

        // Screen Clear
        void Clear(Color clearColor = Color(0.0f, 0.2f, 0.4f, 1.0f));

        // 2D Primitive Drawing Commands
        void DrawPixel(const int xPosition, const int yPosition, Color color = Color::White);
        void DrawLine(const int startXPosition, const int startYPosition, const int endXPosition, const int endYPosition, Color color = Color::White);
        void DrawRectangle(const int startXPosition, const int startYPosition, const int endXPosition, const int endYPosition, Color color = Color::White);
        void DrawFilledRectangle(const int startXPosition, const int startYPosition, const int endXPosition, const int endYPosition, Color color = Color::White);

        // Custom 8x8 Bitmap Font Rendering
        void DrawCharacter(const int xPosition, const int yPosition, const char character, Color color = Color::White, float scale = 1.0f);
        void DrawString(const int xPosition, const int yPosition, const std::string& text, Color color = Color::White, float scale = 1.0f);

        // Texture Management & Drawing
        Texture* LoadTexture(const std::string& filePath);
        Texture* CreateTexture(const int textureWidth, const int textureHeight, const unsigned char* rgbaPixels);
        void DestroyTexture(Texture* texture);

        void DrawTexture(const int xPosition, const int yPosition, const Texture* texture, Color tintColor = Color::White);
        void DrawTexture(const int xPosition, const int yPosition, const int destinationWidth, const int destinationHeight, const Texture* texture, Color tintColor = Color::White);
        void DrawSprite(const int xPosition, const int yPosition, const int destinationWidth, const int destinationHeight, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const Texture* texture, Color tintColor = Color::White);

        // Window & Handle Access (for InputManager integration)
        const HWND* GetWindowHandlePointer() const;
        int GetWidth() const;
        int GetHeight() const;

    private:
        static const UINT FrameCount = 2;
        static const UINT MaxBatchVertices = 65536;
        static const UINT MaxDescriptors = 256;

        HWND windowHandle;
        int width;
        int height;
        bool isRunning;

        std::function<void()> OnStart;
        std::function<void(float)> OnUpdate;

        // DX12 Pipeline Objects
        D3D12_VIEWPORT viewport;
        D3D12_RECT scissorRect;
        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[FrameCount];
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
        UINT rtvDescriptorSize;
        UINT srvDescriptorSize;
        UINT srvDescriptorAllocatedCount;

        // Dynamic 2D Vertex Buffer (Upload Heap)
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        Vertex2D* mappedVertexData;
        UINT currentVertexOffset;

        // Built-in Default Textures
        std::unique_ptr<Texture> whiteTexture;
        std::unique_ptr<Texture> fontTexture;

        // User Loaded Textures
        std::vector<std::unique_ptr<Texture>> userTextures;

        // Batch Accumulator
        std::vector<Vertex2D> batchedVertices;
        const Texture* currentlyBoundTexture;
        Color currentClearColor;

        // Synchronization
        UINT frameIndex;
        HANDLE fenceEvent;
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        UINT64 fenceValue;

        // Internal Helpers
        void InitializeWindow(const std::string& windowName);
        void InitializePipeline();
        void InitializeAssets();
        void InitializeDefaultTextures();

        float ToNdcX(int x) const;
        float ToNdcY(int y) const;

        void BeginFrame();
        void EndFrame();
        void SetActiveTexture(const Texture* texture);
        void FlushBatch();
        void WaitForPreviousFrame();

        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    };
}