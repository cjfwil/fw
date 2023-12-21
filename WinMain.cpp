#include "D3D11Window.cpp"
#include "D3D11Renderer.cpp"


INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    if (SUCCEEDED(CreateAndShowD3D11Window()))
    {
        bool recievedMessage;
        MSG msg;
        msg.message = WM_NULL;
        PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
        while (WM_QUIT != msg.message)
        {
            recievedMessage = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
            if (recievedMessage)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                // Update();
                // Render();
                const float teal[] = {0.098f, 0.439f, 0.439f, 1.000f};
                d3d11_window.context->ClearRenderTargetView(d3d11_window.renderTarget, teal);
                d3d11_window.context->ClearDepthStencilView(d3d11_window.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
                d3d11_window.swapChain->Present(1, 0);
            }
        }
    }
    return (0);
}