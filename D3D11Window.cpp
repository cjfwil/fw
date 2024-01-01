#ifndef D3D11_WINDOW_CPP
#define D3D11_WINDOW_CPP

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shcore.lib")

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma warning(push, 0)
#include <windows.h>
#include <shellscalingapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#pragma warning(pop)

#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 768

int dx = 0;
int dy = 0;

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
    ID3D11RenderTargetView *renderTargetView;

    UINT width;
    UINT height;

    ID3D11Texture2D *depthStencilBuffer;
    ID3D11DepthStencilView *depthStencilView;

    D3D11_VIEWPORT viewport;

    BOOL resize;
    BOOL cursorHidden;
    BOOL cursorIsBound;

    inline float CalculateAspectRatio()
    {
        return static_cast<float>(width) / static_cast<float>(height);
    }
} d3d11_window;

void CentreCursor()
{
    RECT rc;
    GetClientRect(d3d11_window.hwnd, &rc);
    POINT pt;
    pt.x = (rc.right - rc.left) / 2;
    pt.y = (rc.bottom - rc.top) / 2;
    ClientToScreen(d3d11_window.hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
}

void BoundCursorMovement()
{
    RECT rc;
    GetClientRect(d3d11_window.hwnd, &rc);
    MapWindowPoints(d3d11_window.hwnd, nullptr, (POINT *)(&rc), 2);
    ClipCursor(&rc);
    d3d11_window.cursorIsBound = TRUE;
}

void UnboundCursorMovement()
{
    ClipCursor(NULL);
    d3d11_window.cursorIsBound = FALSE;
}

void HideCursor()
{
    while (ShowCursor(FALSE) >= 0)
    {
    }
    d3d11_window.cursorHidden = TRUE;
}

void UnhideCursor()
{
    while (ShowCursor(TRUE) < 0)
    {
    }
    d3d11_window.cursorHidden = FALSE;
}

HRESULT ConfigBackBuffer(UINT width = DEFAULT_WINDOW_WIDTH, UINT height = DEFAULT_WINDOW_HEIGHT)
{
    HRESULT hr = S_OK;

    if (d3d11_window.backBuffer)
    {
        d3d11_window.backBuffer->Release();
        d3d11_window.backBuffer = NULL;
    }
    if (d3d11_window.renderTargetView)
    {
        d3d11_window.renderTargetView->Release();
        d3d11_window.renderTargetView = NULL;
    }
    if (d3d11_window.depthStencilView)
    {
        d3d11_window.depthStencilView->Release();
        d3d11_window.depthStencilView = NULL;
    }
    if (d3d11_window.depthStencilBuffer)
    {
        d3d11_window.depthStencilBuffer->Release();
        d3d11_window.depthStencilBuffer = NULL;
    }
    d3d11_window.context->Flush();

    hr = d3d11_window.swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    hr = d3d11_window.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&d3d11_window.backBuffer);

    D3D11_TEXTURE2D_DESC tdesc;
    d3d11_window.backBuffer->GetDesc(&tdesc);

    hr = d3d11_window.device->CreateRenderTargetView(d3d11_window.backBuffer, nullptr, &d3d11_window.renderTargetView);
    CD3D11_TEXTURE2D_DESC depthStencilDesc(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        width,
        height,
        1,
        1,
        D3D11_BIND_DEPTH_STENCIL);
    hr = d3d11_window.device->CreateTexture2D(&depthStencilDesc, nullptr, &d3d11_window.depthStencilBuffer);

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    hr = d3d11_window.device->CreateDepthStencilView(
        d3d11_window.depthStencilBuffer,
        &depthStencilViewDesc,
        &d3d11_window.depthStencilView);

    d3d11_window.context->OMSetRenderTargets(1, &d3d11_window.renderTargetView, d3d11_window.depthStencilView);

    ZeroMemory(&d3d11_window.viewport, sizeof(D3D11_VIEWPORT));
    d3d11_window.viewport.Height = (float)height;
    d3d11_window.viewport.Width = (float)width;
    d3d11_window.viewport.MinDepth = 0;
    d3d11_window.viewport.MaxDepth = 1;
    d3d11_window.context->RSSetViewports(1, &d3d11_window.viewport);

    d3d11_window.width = width;
    d3d11_window.height = height;

    return (hr);
}

LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    
    static BOOL isSizing = FALSE;
    switch (uMsg)
    {
    case WM_MOUSEACTIVATE:
    case WM_ACTIVATE:
    {
        BOOL clientRegionClicked = FALSE;
        if (uMsg == WM_MOUSEACTIVATE)
        {
            POINT mousePoint;
            GetCursorPos(&mousePoint);
            ScreenToClient(hWnd, &mousePoint);

            RECT clientRect = {};
            GetClientRect(hWnd, &clientRect);
            clientRegionClicked = PtInRect(&clientRect, mousePoint);
        }
        if (wParam & WA_ACTIVE || ((wParam & WA_CLICKACTIVE) && clientRegionClicked))
        {
            BoundCursorMovement();
            HideCursor();
        }
        else
        {
            UnboundCursorMovement();
            UnhideCursor();
        }
    }
    break;
    case WM_INPUT:
    {
        UINT size;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
        if (size <= sizeof(RAWINPUT))
        {
            static BYTE rawInputBuffer[sizeof(RAWINPUT)];
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawInputBuffer, &size, sizeof(RAWINPUTHEADER)) == size)
            {
                RAWINPUT *ri = (RAWINPUT *)rawInputBuffer;
                if (ri->header.dwType == RIM_TYPEMOUSE && (ri->data.mouse.lLastX != 0 || ri->data.mouse.lLastY != 0))
                {
                    dx += ri->data.mouse.lLastX;
                    dy += ri->data.mouse.lLastY;
                }
            }
        }
    }
    break;
    case WM_EXITSIZEMOVE:
    case WM_SIZE:
    {
        RECT rc = {};
        GetClientRect(hWnd, &rc);
        d3d11_window.rc = rc;
        if (uMsg == WM_EXITSIZEMOVE)
            isSizing = FALSE;
        if (!isSizing)
        {
            UINT Width = (UINT)(rc.right - rc.left);
            UINT Height = (UINT)(rc.bottom - rc.top);
            ConfigBackBuffer(Width, Height);
            d3d11_window.resize = TRUE; //call on resize function in struct?
        }
    }
    break;
    case WM_ENTERSIZEMOVE:
    {
        isSizing = TRUE;
    }
    break;
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

HRESULT CreateWin32Window(HINSTANCE arg_hInstance = NULL, char *windowTitle = "D3D11 Window")
{
    SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
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

    int defaultWidth = DEFAULT_WINDOW_WIDTH;
    int defaultHeight = DEFAULT_WINDOW_HEIGHT;
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
    IDXGIFactory *factory = NULL;

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
    dxgiDevice->Release();
    adapter->Release();
    if (factory != NULL)
        factory->Release();
    return (hr);
}

#endif