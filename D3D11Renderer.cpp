#ifndef D3D11_RENDERER_CPP
#define D3D11_RENDERER_CPP

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>

#include <stdio.h>
#include <directxmath.h>

#include "D3D11Window.cpp"

ID3D11VertexShader *vertexShader;
ID3D11InputLayout *inputLayout;
ID3D11PixelShader *pixelShader;

typedef struct _constantBufferStruct
{
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
} ConstantBufferStruct;

ConstantBufferStruct constantBufferData;
unsigned int indexCount;

ID3D11Buffer *constantBuffer;
ID3D11Buffer *vertexBuffer;
ID3D11Buffer *indexBuffer;

typedef struct _vertexPositionColor
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 color;
} VertexPositionColor;

void CreateViewAndPerspective()
{
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.7f, 1.5f, 0.f);
    DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, -0.1f, 0.0f, 0.f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.f);

    DirectX::XMStoreFloat4x4(
        &constantBufferData.view,
        DirectX::XMMatrixTranspose(
            DirectX::XMMatrixLookAtRH(
                eye,
                at,
                up)));

    float aspectRatioX = d3d11_window.AspectRatio();
    float aspectRatioY = aspectRatioX < (16.0f / 9.0f) ? aspectRatioX /
                                                             (16.0f / 9.0f)
                                                       : 1.0f;

    DirectX::XMStoreFloat4x4(
        &constantBufferData.projection,
        DirectX::XMMatrixTranspose(
            DirectX::XMMatrixPerspectiveFovRH(
                2.0f * atanf(tanf(DirectX::XMConvertToRadians(70) *
                                          0.5f) /
                                 aspectRatioY),
                aspectRatioX,
                0.01f,
                100.0f)));
}

void CreateWindowSizeDependentResources()
{
    // CreateViewAndPerspective();
}

HRESULT CreateShaders()
{
    HRESULT hr = S_OK;
    ID3D11Device *device = d3d11_window.device;

    // TODO: replace std lib
    FILE *vShader, *pShader;
    BYTE *bytes;
    size_t destSize = 4096;
    size_t bytesRead = 0;
    bytes = new BYTE[destSize];
    fopen_s(&vShader, "CubeVertexShader.cso", "rb");
    bytesRead = fread_s(bytes, destSize, 1, 4096, vShader);
    hr = device->CreateVertexShader(
        bytes,
        bytesRead,
        nullptr,
        &vertexShader);

    D3D11_INPUT_ELEMENT_DESC iaDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    hr = device->CreateInputLayout(iaDesc, ARRAYSIZE(iaDesc), bytes, bytesRead, &inputLayout);
    delete bytes;

    bytes = new BYTE[destSize];
    bytesRead = 0;
    fopen_s(&pShader, "CubePixelShader.cso", "rb");
    bytesRead = fread_s(bytes, destSize, 1, 4096, pShader);
    hr = device->CreatePixelShader(bytes, bytesRead, nullptr, &pixelShader);
    delete bytes;

    CD3D11_BUFFER_DESC cbDesc(sizeof(ConstantBufferStruct), D3D11_BIND_CONSTANT_BUFFER);
    hr = device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);

    fclose(vShader);
    fclose(pShader);
    return (hr);
}

HRESULT CreateCube()
{
    HRESULT hr = S_OK;
    ID3D11Device *device = d3d11_window.device;
    VertexPositionColor CubeVertices[] =
        {
            {
                DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f),
                DirectX::XMFLOAT3(0, 0, 0),
            },
            {
                DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f),
                DirectX::XMFLOAT3(0, 0, 1),
            },
            {
                DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f),
                DirectX::XMFLOAT3(0, 1, 0),
            },
            {
                DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f),
                DirectX::XMFLOAT3(0, 1, 1),
            },
            {
                DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f),
                DirectX::XMFLOAT3(1, 0, 0),
            },
            {
                DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f),
                DirectX::XMFLOAT3(1, 0, 1),
            },
            {
                DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f),
                DirectX::XMFLOAT3(1, 1, 0),
            },
            {
                DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
                DirectX::XMFLOAT3(1, 1, 1),
            },
        };

    CD3D11_BUFFER_DESC vDesc(sizeof(CubeVertices), D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA vData;
    ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
    vData.pSysMem = CubeVertices;
    vData.SysMemPitch = 0;
    vData.SysMemSlicePitch = 0;

    hr = device->CreateBuffer(&vDesc, &vData, &vertexBuffer);

    // Create index buffer:
    unsigned short CubeIndices[] =
        {
            0,
            2,
            1, // -x
            1,
            2,
            3,
            4,
            5,
            6, // +x
            5,
            7,
            6,
            0,
            1,
            5, // -y
            0,
            5,
            4,
            2,
            6,
            7, // +y
            2,
            7,
            3,
            0,
            4,
            6, // -z
            0,
            6,
            2,
            1,
            3,
            7, // +z
            1,
            7,
            5,
        };
    indexCount = ARRAYSIZE(CubeIndices);
    CD3D11_BUFFER_DESC indexDesc(sizeof(CubeIndices), D3D11_BIND_INDEX_BUFFER);
    D3D11_SUBRESOURCE_DATA indexData;
    ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
    indexData.pSysMem = CubeIndices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);
    return (hr);
}

void CreateDeviceDependentResources()
{
    // todo run creation of meshes and shaders asynchronously
    CreateShaders();
    CreateCube();
}

#endif