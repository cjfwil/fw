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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(pop)

#include "D3D11Window.cpp"

#include "win32_hash_table.hpp"
#include "file_navigation.h"

ID3D11VertexShader *vertexShader;
ID3D11InputLayout *inputLayout;
ID3D11PixelShader *pixelShader;

ID3D11SamplerState *samplerState;

ID3D11BlendState *blendState;

ID3D11RasterizerState *rasterStateNoCull;
ID3D11RasterizerState *rasterStateBackCull;

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

aiTextureType _types[] = {aiTextureType_DIFFUSE, aiTextureType_SPECULAR};

struct mesh_buffers
{
    ID3D11Buffer *vertexBuffer;
    ID3D11Buffer *indexBuffer;

    unsigned int textureIndex[ARRAYSIZE(_types)];
    unsigned int indexCount;
    bool cullBackface = true;
};

struct texture_info
{
    ID3D11ShaderResourceView *textureView;
    unsigned int slot;
    bool hasAlpha;
    char path[MAX_PATH] = {};
};

struct model
{
    win32_expandable_list<mesh_buffers> meshList;
    bool enabled = false;
};

win32_expandable_list<model> modelList;
win32_expandable_list<path_string> pathList;

win32_expandable_list<texture_info> textures;
win32_expandable_list<key_value_pair> texturePaths;
unsigned int numNullTextures = 0;
unsigned int numLoadedTextures = 0;

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
    float aspectRatioY = aspectRatioX < (16.0f / 9.0f) ? aspectRatioX / (16.0f / 9.0f) : 1.0f;

    DirectX::XMStoreFloat4x4(
        &constantBufferData.projection,
        DirectX::XMMatrixTranspose(
            DirectX::XMMatrixPerspectiveFovRH(
                2.0f * atanf(tanf(DirectX::XMConvertToRadians(70) *
                                  0.5f) /
                             aspectRatioY),
                aspectRatioX,
                0.01f,
                3000.0f))); // make infinite view matrix?
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

texture_info CreateTextureForShader(char *path, aiTextureType type)
{
    texture_info result = {};
    if (type == aiTextureType_DIFFUSE)
        result.slot = 0;
    else if (type == aiTextureType_SPECULAR)
        result.slot = 1;

    int forcedN = 4;
    unsigned char *clrData = NULL;
    unsigned int width;
    unsigned int height;
    // todo: maybe define isNullTexture based on whether stbi_load succeeds
    bool isNullTexture = (path[0] == '\0') ? true : false;
    if (isNullTexture)
    {

        // null texture
        width = 64;
        height = 64;
        clrData = (unsigned char *)malloc(width * height * forcedN);
        for (u_int x = 0; x < width; ++x)
        {
            for (u_int y = 0; y < height; ++y)
            {
                unsigned int pixelColour = ((x ^ y) % 2 == 0) ? (u_int)0xffff00ff : (u_int)0xff000000;
                if (result.slot != 0) pixelColour = 0; //non-diffuse slot
                ((unsigned int *)clrData)[x + y * width] = pixelColour;
            }
        }
        numNullTextures++;
        result.hasAlpha = true;
    }
    else
    {
        int x, y, n;
        stbi_set_flip_vertically_on_load(1);
        clrData = stbi_load(path, &x, &y, &n, forcedN);
        width = x;
        height = y;
        numLoadedTextures++;
        if (n == 4)
        {
            result.hasAlpha = true;
        }
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

    d3d11_window.context->UpdateSubresource(texture, 0, nullptr, clrData, width * forcedN, 0);

    if (isNullTexture)
    {
        free(clrData);
    }
    else
    {
        stbi_image_free(clrData);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = UINT_MAX;

    ID3D11ShaderResourceView *textureShaderResourceView;
    hr = d3d11_window.device->CreateShaderResourceView(texture, &srvDesc, &textureShaderResourceView);

    if (FAILED(hr))
    {
        // TODO:
    }

    d3d11_window.context->GenerateMips(textureShaderResourceView);
    result.textureView = textureShaderResourceView;
    strcpy(result.path, path);
    return (result);
}

mesh_buffers CreateVertexIndexBufferPair(VertexPositionUVNormal *vertices,
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

    mesh_buffers result = {};

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

void CreateTexSamplerState()
{
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
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

    // set alpha blending true/false
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    d3d11_window.device->CreateBlendState(&blendDesc, &blendState);

    D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
    rasterDesc.CullMode = D3D11_CULL_NONE;
    d3d11_window.device->CreateRasterizerState(&rasterDesc, &rasterStateNoCull);

    rasterDesc.CullMode = D3D11_CULL_BACK;
    d3d11_window.device->CreateRasterizerState(&rasterDesc, &rasterStateBackCull);
}

void CreateDeviceDependentResources()
{
    CreateTexSamplerState();

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

    build_path_list("models", ".obj", &pathList);

    // init assimp
    Assimp::Importer imp;

    for (int k = 0; k < pathList.numElements; ++k)
    {
        path_string p = pathList.data[k];
        auto scene = imp.ReadFile(p.path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_GenNormals);

        model m;
        textures.Add(CreateTextureForShader("", aiTextureType_DIFFUSE)); // adding the null texture as index 0
        textures.Add(CreateTextureForShader("", aiTextureType_SPECULAR));

        for (unsigned int j = 0; j < scene->mNumMeshes; ++j)
        {
            auto mesh = scene->mMeshes[j];

            win32_expandable_list<VertexPositionUVNormal> vertices;
            vertices.Init();

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                // float scale = 0.01f;
                float scale = 0.08f;
                if (k == 3) scale = 0.01f;
                VertexPositionUVNormal v = {};
                v.pos.x = mesh->mVertices[i].x * scale;
                v.pos.y = mesh->mVertices[i].y * scale;
                v.pos.z = mesh->mVertices[i].z * scale;

                if (mesh->mTextureCoords[0] != NULL)
                {
                    v.uv.x = (mesh->mTextureCoords[0])[i].x;
                    v.uv.y = (mesh->mTextureCoords[0])[i].y;
                }
                else
                {
                    // create own uvs based on vertices?
                    v.uv.x = 0.0f;
                    v.uv.y = 0.0f;
                }

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

            mesh_buffers vi = CreateVertexIndexBufferPair(vertices.data,
                                                          (UINT)vertices.size,
                                                          indices.data,
                                                          (UINT)indices.size,
                                                          (UINT)indices.numElements);

            // materials
            unsigned int mi = mesh->mMaterialIndex;
            char path[ARRAYSIZE(_types)][256] = {};
            unsigned int totalTexCount = 0;
            if (mi >= 0)
            {
                for (int tt = 0; tt < ARRAYSIZE(_types); ++tt)
                {
                    aiTextureType type = _types[tt];
                    aiMaterial *mat = scene->mMaterials[mi];

                    unsigned int texCount = mat->GetTextureCount(type);
                    totalTexCount += texCount;
                    for (int i = 0; i < texCount; i++)
                    {
                        aiString str;
                        mat->GetTexture(type, i, &str);
                        char parentFolder[MAX_PATH] = {};
                        get_parent_folder(parentFolder, p.path);
                        wsprintfA(path[tt], "%s/%s", parentFolder, str.C_Str());
                        key_value_pair *kvp = GetValueFromKeyLinear(texturePaths, path[tt]);
                        if (kvp == NULL)
                        {
                            texture_info ti = CreateTextureForShader(path[tt], type);
                            ti.slot = tt;
                            textures.Add(ti);

                            key_value_pair newKvp = {};
                            newKvp.value = textures.numElements - 1;
                            strcpy(newKvp.key, path[tt]);
                            texturePaths.Add(newKvp);
                        }
                    }
                }
            }

            if (totalTexCount <= 0)
            {
                for (int i = 0; i < ARRAYSIZE(_types); ++i)
                    wsprintfA(path[i], "");
            }

            for (int i = 0; i < ARRAYSIZE(_types); ++i)
            {
                key_value_pair *kvp = GetValueFromKeyLinear(texturePaths, path[i]);
                if (kvp == NULL)
                {
                    vi.textureIndex[i] = i;
                }
                else
                {
                    texture_info ti = textures.data[kvp->value];
                    vi.textureIndex[ti.slot] = kvp->value;
                }

                vi.cullBackface = (textures.data[vi.textureIndex[0]].hasAlpha) ? false : true;                
            }

            m.meshList.Add(vi);
        }
        modelList.Add(m);
    }
}

#endif