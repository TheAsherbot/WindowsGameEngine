#include <windows.h>
#include <exception>
#include <stdexcept>
#include <comdef.h>
#include <string>
#include <d3dcompiler.h>


inline void ThrowIfFailed(HRESULT hr, const char* operation)
{
    if (FAILED(hr))
    {
        _com_error error(hr);

        throw std::runtime_error(
            std::string(operation) +
            " failed. HRESULT=" +
            std::to_string(static_cast<unsigned long>(hr)) +
            ", message=" +
            error.ErrorMessage());
    }
}

void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter)
{
    *ppAdapter = nullptr;
    for (UINT adapterIndex = 0; ; ++adapterIndex)
    {
        IDXGIAdapter1* pAdapter = nullptr;
        if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
        {
            // No more adapters to enumerate.
            break;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
            *ppAdapter = pAdapter;
            return;
        }
        pAdapter->Release();
    }
}

static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
    const wchar_t* file,
    const char* entryPoint,
    const char* profile)
{
    Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;

    HRESULT hr = D3DCompileFromFile(
        file,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        profile,
        0,
        0,
        bytecode.GetAddressOf(),
        errors.GetAddressOf());

    if (errors)
    {
        OutputDebugStringA(
            static_cast<const char*>(errors->GetBufferPointer()));
    }

    if (FAILED(hr))
    {
        _com_error error(hr);
        throw std::runtime_error(
            std::string("Shader compile failed for ") +
            entryPoint + ": " +
            error.ErrorMessage());
    }

    if (!bytecode)
    {
        throw std::runtime_error(
            std::string("Shader compiler returned no bytecode for ") +
            entryPoint);
    }

    return bytecode;
}