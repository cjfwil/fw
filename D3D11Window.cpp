#ifndef D3D11_WINDOW_CPP
#define D3D11_WINDOW_CPP

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>

struct D3D11_Window
{
    HINSTANCE hInstance;
    char *windowClassName = "D3D11WindowClass";
    HWND hwnd;
    RECT rc;
    HMENU hMenu;

    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Device *device;
    ID3D11DeviceContext *context;
    IDXGISwapChain *swapChain;

    ID3D11Texture2D *backBuffer;
    ID3D11RenderTargetView *renderTarget;

    D3D11_TEXTURE2D_DESC backBufferDesc;

    ID3D11Texture2D *depthStencilBuffer;
    ID3D11DepthStencilView *depthStencilView;

    D3D11_VIEWPORT viewport;

    float AspectRatio()
    {
        return static_cast<float>(backBufferDesc.Width) / static_cast<float>(backBufferDesc.Height);
    }
} d3d11_window;

LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
    {
        HMENU hMenu;
        hMenu = GetMenu(hWnd);
        if (hMenu != NULL)
        {
            DestroyMenu(hMenu);
        }
        DestroyWindow(hWnd);
        UnregisterClassA(d3d11_window.windowClassName, d3d11_window.hInstance);
        return 0;
    }
    break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    }

    return (DefWindowProcA(hWnd, uMsg, wParam, lParam));
}

HRESULT CreateWin32FullscreenBorderlessWindow(HINSTANCE arg_hInstance = NULL, char *windowTitle = "D3D11 Window")
{
    d3d11_window.hInstance = arg_hInstance;
    if (d3d11_window.hInstance == NULL)
    {
        d3d11_window.hInstance = (HINSTANCE)GetModuleHandle(NULL);
    }

    HICON hIcon = NULL;
    char szExePath[MAX_PATH];
    GetModuleFileNameA(NULL, szExePath, MAX_PATH);

    if (hIcon)
    {
        hIcon = ExtractIconA(d3d11_window.hInstance, szExePath, 0);
    }

    WNDCLASSA wndClass;
    wndClass.style = CS_DBLCLKS;
    wndClass.lpfnWndProc = StaticWindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = d3d11_window.hInstance;
    wndClass.hIcon = hIcon;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = "D3D11WindowClass";

    if (!RegisterClassA(&wndClass))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return HRESULT_FROM_WIN32(dwError);
    }

    int defaultWidth = 640;
    int defaultHeight = 480;
    SetRect(&d3d11_window.rc, 0, 0, defaultWidth, defaultHeight);
    AdjustWindowRect(&d3d11_window.rc, WS_OVERLAPPEDWINDOW, (d3d11_window.hMenu != NULL) ? true : false);

    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    d3d11_window.hwnd = CreateWindowA(d3d11_window.windowClassName, windowTitle, WS_OVERLAPPEDWINDOW, x, y, (d3d11_window.rc.right - d3d11_window.rc.left), (d3d11_window.rc.bottom - d3d11_window.rc.top), 0, d3d11_window.hMenu, d3d11_window.hInstance, 0);

    if (d3d11_window.hwnd == NULL)
    {
        DWORD dwError = GetLastError();
        return HRESULT_FROM_WIN32(dwError);
    }

    return (S_OK);
}

HRESULT CreateD3D11DeviceResources()
{
    HRESULT hr = S_OK;

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
        };

    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device *device;
    ID3D11DeviceContext *context;

    hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        deviceFlags,
        levels,
        ARRAYSIZE(levels),
        D3D11_SDK_VERSION,
        &device,
        &d3d11_window.featureLevel,
        &context);

    if (FAILED(hr))
    {
        // TODO: handle failure
    }

    d3d11_window.device = device;
    d3d11_window.context = context;

    return (hr);
}

HRESULT ConfigBackBuffer()
{
    HRESULT hr = S_OK;
    hr = d3d11_window.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&d3d11_window.backBuffer);
    if (SUCCEEDED(hr))
    {
        hr = d3d11_window.device->CreateRenderTargetView(d3d11_window.backBuffer, nullptr, &d3d11_window.renderTarget);
        d3d11_window.backBuffer->GetDesc(&d3d11_window.backBufferDesc);

        CD3D11_TEXTURE2D_DESC depthStencilDesc(
            DXGI_FORMAT_D24_UNORM_S8_UINT,
            static_cast<UINT>(d3d11_window.backBufferDesc.Width),
            static_cast<UINT>(d3d11_window.backBufferDesc.Height),
            1,
            1,
            D3D11_BIND_DEPTH_STENCIL);

        d3d11_window.device->CreateTexture2D(&depthStencilDesc, nullptr, &d3d11_window.depthStencilBuffer);

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
        d3d11_window.device->CreateDepthStencilView(
            d3d11_window.depthStencilBuffer,
            &depthStencilViewDesc,
            &d3d11_window.depthStencilView);

        ZeroMemory(&d3d11_window.viewport, sizeof(D3D11_VIEWPORT));
        d3d11_window.viewport.Height = (float)d3d11_window.backBufferDesc.Height;
        d3d11_window.viewport.Width = (float)d3d11_window.backBufferDesc.Width;
        d3d11_window.viewport.MinDepth = 0;
        d3d11_window.viewport.MaxDepth = 1;
        d3d11_window.context->RSSetViewports(1, &d3d11_window.viewport);
    }

    return (hr);
}

HRESULT CreateD3D11WindowResource()
{
    HRESULT hr = S_OK;

    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.Windowed = TRUE;
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.OutputWindow = d3d11_window.hwnd;

    IDXGIDevice3 *dxgiDevice;
    d3d11_window.device->QueryInterface(__uuidof(IDXGIDevice3), reinterpret_cast<void **>(&dxgiDevice));
    IDXGIAdapter *adapter;
    IDXGIFactory *factory;

    hr = dxgiDevice->GetAdapter(&adapter);
    if (SUCCEEDED(hr))
    {
        adapter->GetParent(IID_PPV_ARGS(&factory));
        hr = factory->CreateSwapChain(d3d11_window.device, &desc, &d3d11_window.swapChain);
    }
    if (SUCCEEDED(hr))
    {
        hr = ConfigBackBuffer();
    }
    return (hr);
}

#endif