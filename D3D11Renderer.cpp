#ifndef D3D11_RENDERER_CPP
#define D3D11_RENDERER_CPP

#pragma warning(push, 0)
#pragma warning(disable : 4365)
#pragma warning(disable : 4668)
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

typedef struct _ModelViewProjectionConstantBuffer
{
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
} ModelViewProjectionConstantBuffer;
static_assert((sizeof(ModelViewProjectionConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

ModelViewProjectionConstantBuffer constantBufferData;
unsigned int frameCount;

ID3D11Buffer *constantBuffer;

struct vertex_index_buffer_pair
{
    ID3D11Buffer *vertexBuffer;
    ID3D11Buffer *indexBuffer;
    unsigned int indexCount;
};

win32_expandable_list<vertex_index_buffer_pair> mainModel;

typedef struct _vertexPositionColor
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 color;
    float padding[2];
} VertexPositionColor;
static_assert((sizeof(VertexPositionColor) % 16) == 0, "VertexPositionColor size must be 16-byte aligned");

typedef struct _vertexPositionUV
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    float padding[3];
} VertexPositionUV;
static_assert((sizeof(VertexPositionUV) % 16) == 0, "VertexPositionUV size must be 16-byte aligned");

typedef struct _vertexPositionUVNormal
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT3 normal;
} VertexPositionUVNormal;
static_assert((sizeof(VertexPositionUVNormal) % 16) == 0, "VertexPositionUVNormal size must be 16-byte aligned");

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
                3000.0f))); //make infinite view matrix?
}

void CreateWindowSizeDependentResources()
{
    CreateViewAndPerspective();
}

HRESULT CreateShaderPair(char *vertexShaderPath, char *pixelShaderPath, D3D11_INPUT_ELEMENT_DESC *iaDesc, UINT iaDescSize)
{
    HRESULT hr = S_OK;
    ID3D11Device *device = d3d11_window.device;

    // TODO: replace std lib
    FILE *vShader, *pShader;
    BYTE *bytes;
    size_t destSize = 4096;
    size_t bytesRead = 0;
    bytes = new BYTE[destSize];
    fopen_s(&vShader, vertexShaderPath, "rb");
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

    hr = device->CreateInputLayout(iaDesc, iaDescSize, bytes, bytesRead, &inputLayout);
    delete bytes;

    bytes = new BYTE[destSize];
    bytesRead = 0;
    fopen_s(&pShader, pixelShaderPath, "rb");
    bytesRead = fread_s(bytes, destSize, 1, 4096, pShader);
    hr = device->CreatePixelShader(bytes, bytesRead, nullptr, &pixelShader);
    delete bytes;

    CD3D11_BUFFER_DESC cbDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
    hr = device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);

    fclose(vShader);
    fclose(pShader);
    return (hr);
}

vertex_index_buffer_pair CreateVertexIndexBufferPair(VertexPositionUVNormal *vertices,
                                                     unsigned int verticesSize,
                                                     unsigned short *indices,
                                                     unsigned int indicesSize,
                                                     UINT indexCount)
{
    HRESULT hr = S_OK;
    ID3D11Device *device = d3d11_window.device;

    // Create Vertex buffer:
    CD3D11_BUFFER_DESC vDesc(verticesSize, D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA vData;
    ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
    vData.pSysMem = vertices;
    vData.SysMemPitch = 0;
    vData.SysMemSlicePitch = 0;

    vertex_index_buffer_pair result = {};

    hr = device->CreateBuffer(&vDesc, &vData, &result.vertexBuffer);

    // Create index buffer:
    result.indexCount = indexCount;
    CD3D11_BUFFER_DESC indexDesc(indicesSize, D3D11_BIND_INDEX_BUFFER);
    D3D11_SUBRESOURCE_DATA indexData;
    ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    hr = device->CreateBuffer(&indexDesc, &indexData, &result.indexBuffer);
    // TODO: hresult error checking

    return (result);
}

void CreateTexture()
{
    unsigned int width = 512;
    unsigned int height = width;
    unsigned int *clrData = (unsigned int *)malloc(width * height * sizeof(unsigned int));
    for (u_int x = 0; x < width; ++x)
    {
        for (u_int y = 0; y < height; ++y)
        {
            clrData[x + y * width] = ((x ^ y) % 2 == 0) ? (u_int)0x00ff00ff : (u_int)0x00000000;
        }
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    // ---- this is commented out because it cant be used with generating mipmaps -----
    // ---- if we dont need mips we can use this ----
    // D3D11_SUBRESOURCE_DATA sd = {};
    // sd.pSysMem = (void *)clrData;
    // sd.SysMemPitch = width * sizeof(unsigned int);

    ID3D11Texture2D *texture;
    HRESULT hr = d3d11_window.device->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr))
    {
        // TODO:
    }

    d3d11_window.context->UpdateSubresource(texture, 0, nullptr, clrData, width * sizeof(unsigned int), 0);

    free(clrData);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = UINT_MAX;

    hr = d3d11_window.device->CreateShaderResourceView(texture, &srvDesc, &textureShaderResourceView);

    if (FAILED(hr))
    {
        // TODO:
    }

    d3d11_window.context->GenerateMips(textureShaderResourceView);
}

void CreateSamplerState()
{
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MipLODBias = 0.0f;
    sd.MinLOD = 0.0f;
    sd.MaxLOD = D3D11_FLOAT32_MAX;

    BOOL af16 = TRUE; // anisotropic filtering 16x
    if (af16)
    {
        sd.Filter = D3D11_FILTER_ANISOTROPIC;
        sd.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY; // equals 16
    }

    HRESULT hr = d3d11_window.device->CreateSamplerState(&sd, &samplerState);
    if (FAILED(hr))
    {
        // TODO:
    }
}

void CreateDeviceDependentResources()
{
    // todo run creation of meshes and shaders asynchronously
    D3D11_INPUT_ELEMENT_DESC iaDescNormals[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
         0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    CreateShaderPair("CubeVertexShaderLighting.cso", "CubePixelShaderLighting.cso", iaDescNormals, ARRAYSIZE(iaDescNormals));

    // init assimp
    Assimp::Importer imp;
    auto model = imp.ReadFile("models/sponza.obj", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_GenNormals);

    mainModel.Init();
    for (unsigned int j = 0; j < model->mNumMeshes; ++j)
    {
        auto mesh = model->mMeshes[j];

        win32_expandable_list<VertexPositionUVNormal> vertices;
        vertices.Init();

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            VertexPositionUVNormal v = {};
            v.pos.x = mesh->mVertices[i].x;
            v.pos.y = mesh->mVertices[i].y;
            v.pos.z = mesh->mVertices[i].z;

            v.uv.x = (mesh->mTextureCoords[0])[i].x;
            v.uv.y = (mesh->mTextureCoords[0])[i].y;

            v.normal.x = mesh->mNormals[i].x;
            v.normal.y = mesh->mNormals[i].y;
            v.normal.z = mesh->mNormals[i].z;

            vertices.Add(v);
        }

        win32_expandable_list<unsigned short> indices;
        indices.Init();
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            const auto &f = mesh->mFaces[i];
            assert(f.mNumIndices == 3);
            indices.Add((unsigned short)f.mIndices[0]);
            indices.Add((unsigned short)f.mIndices[2]);
            indices.Add((unsigned short)f.mIndices[1]);
        }
        vertex_index_buffer_pair vi = CreateVertexIndexBufferPair(vertices.data,
                                    (UINT)vertices.size,
                                    indices.data,
                                    (UINT)indices.size,
                                    (UINT)indices.numElements);
        
        mainModel.Add(vi);
    }

    CreateTexture();
}

#endif