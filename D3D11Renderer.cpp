#ifndef D3D11_RENDERER_CPP
#define D3D11_RENDERER_CPP

#pragma warning(push, 0)
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>

#include <stdio.h>
#include <memory.h>
#include <directxmath.h>
#pragma warning(pop)

#include "D3D11Window.cpp"

ID3D11VertexShader *vertexShader;
ID3D11InputLayout *inputLayout;
ID3D11PixelShader *pixelShader;

ID3D11ShaderResourceView *textureShaderResourceView;
ID3D11SamplerState *samplerState;

typedef struct _constantBufferStruct
{
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
} ConstantBufferStruct;
static_assert((sizeof(ConstantBufferStruct) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

ConstantBufferStruct constantBufferData;
unsigned int indexCount;
unsigned int frameCount;

ID3D11Buffer *constantBuffer;
ID3D11Buffer *vertexBuffer;
ID3D11Buffer *indexBuffer;

typedef struct _vertexPositionColor
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 color;
} VertexPositionColor;
static_assert((sizeof(VertexPositionColor) % 8) == 0, "VertexPositionColor size must be 8-byte aligned");

typedef struct _vertexPositionUV
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    float padding;
} VertexPositionUV;
static_assert((sizeof(VertexPositionUV) % 8) == 0, "VertexPositionUV size must be 8-byte aligned");

DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.7f, 1.5f, 0.f);
DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.7f, 0.0f, 0.f);
DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.f);

void CreateViewAndPerspective()
{
    DirectX::XMStoreFloat4x4(&constantBufferData.world, DirectX::XMMatrixIdentity());

    DirectX::XMStoreFloat4x4(
        &constantBufferData.view,
        DirectX::XMMatrixTranspose(
            DirectX::XMMatrixLookAtRH(
                eye,
                at,
                up)));

    float aspectRatioX = d3d11_window.CalculateAspectRatio();
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
    CreateViewAndPerspective();
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

    if (FAILED(hr))
    {
        // TODO:
    }

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

HRESULT CreateShadersTex()
{
    HRESULT hr = S_OK;
    ID3D11Device *device = d3d11_window.device;

    // TODO: replace std lib
    FILE *vShader, *pShader;
    BYTE *bytes;
    size_t destSize = 4096;
    size_t bytesRead = 0;
    bytes = new BYTE[destSize];
    fopen_s(&vShader, "CubeVertexShaderTex.cso", "rb");
    bytesRead = fread_s(bytes, destSize, 1, 4096, vShader);
    hr = device->CreateVertexShader(
        bytes,
        bytesRead,
        nullptr,
        &vertexShader);

    if (FAILED(hr))
    {
        // TODO:
    }

    D3D11_INPUT_ELEMENT_DESC iaDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    hr = device->CreateInputLayout(iaDesc, ARRAYSIZE(iaDesc), bytes, bytesRead, &inputLayout);
    delete bytes;

    bytes = new BYTE[destSize];
    bytesRead = 0;
    fopen_s(&pShader, "CubePixelShaderTex.cso", "rb");
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

HRESULT CreateTexturedCube()
{
    HRESULT hr = S_OK;
    ID3D11Device *device = d3d11_window.device;
    VertexPositionUV CubeVertices[] =
        {
            {DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f)}, // 0
            {DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f)},  // 1
            {DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f)},   // 2
            {DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f)},  // 3
            {DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), DirectX::XMFLOAT2(0.0f, 1.0f)},  // 4
            {DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), DirectX::XMFLOAT2(0.0f, 0.0f)},   // 5
            {DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), DirectX::XMFLOAT2(1.0f, 0.0f)},    // 6
            {DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), DirectX::XMFLOAT2(1.0f, 1.0f)}    // 7
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
            0, 2, 1, 0, 3, 2, // Front face
            4, 5, 6, 4, 6, 7, // Back face
            4, 1, 5, 4, 0, 1, // Left face
            3, 6, 2, 3, 7, 6, // Right face
            1, 6, 5, 1, 2, 6, // Top face
            4, 3, 0, 4, 7, 3  // Bottom face
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

void CreateTexture()
{
    unsigned int width = 64;
    unsigned int height = 64;
    unsigned int *clrData = (unsigned int *)malloc(width * height * sizeof(unsigned int));
    for (u_int x = 0; x < width; ++x)
    {
        for (u_int y = 0; y < height; ++y)
        {
            clrData[x+y*width] = ((x^y) % 2 == 0) ? 0xffffffff : 0x00000000;
        }
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = (void *)clrData;
    sd.SysMemPitch = width * sizeof(unsigned int);

    ID3D11Texture2D *texture;
    HRESULT hr = d3d11_window.device->CreateTexture2D(&texDesc, &sd, &texture);
    if (FAILED(hr))
    {
        // TODO:
    }

    free(clrData);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = d3d11_window.device->CreateShaderResourceView(texture, &srvDesc, &textureShaderResourceView);

    if (FAILED(hr))
    {
        // TODO:
    }
}

void CreateSamplerState()
{
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    HRESULT hr = d3d11_window.device->CreateSamplerState(&sd, &samplerState);
    if (FAILED(hr))
    {
        // TODO:
    }
}

void CreateDeviceDependentResources()
{
    // todo run creation of meshes and shaders asynchronously
    // CreateShaders();
    // CreateCube();

    CreateShadersTex();
    CreateTexturedCube();
    CreateTexture();
}

#endif