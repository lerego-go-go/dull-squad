#include "imgui.h"
#include "imgui_impl_dx11.h"

#include <d3d11.h>
#include <cstdint>

static void* im_memcpy(void* destination, const void* source, size_t size)
{
    auto data_source = static_cast<const uint8_t*>(source);
    auto data_destination = static_cast<uint8_t*>(destination);

    __movsb(data_destination, data_source, size);
    return static_cast<void*>(data_destination);
}

static void* im_memset(void* destination, int value, size_t size)
{
    auto data = static_cast<uint8_t*>(destination);

    __stosb(data, static_cast<uint8_t>(value), size);
    return static_cast<void*>(data);
}

struct ImGui_ImplDX11_Data
{
    ID3D11Device*               pd3dDevice;
    ID3D11DeviceContext*        pd3dDeviceContext;
    IDXGIFactory*               pFactory;
    ID3D11Buffer*               pVB;
    ID3D11Buffer*               pIB;
    ID3D11VertexShader*         pVertexShader;
    ID3D11InputLayout*          pInputLayout;
    ID3D11Buffer*               pVertexConstantBuffer;
    ID3D11PixelShader*          pPixelShader;
    ID3D11SamplerState*         pFontSampler;
    ID3D11ShaderResourceView*   pFontTextureView;
    ID3D11RasterizerState*      pRasterizerState;
    ID3D11BlendState*           pBlendState;
    ID3D11DepthStencilState*    pDepthStencilState;
    int                         VertexBufferSize;
    int                         IndexBufferSize;

    ImGui_ImplDX11_Data()       { im_memset(this, 0, sizeof(*this)); VertexBufferSize = 5000; IndexBufferSize = 10000; }
};

struct VERTEX_CONSTANT_BUFFER
{
    float   mvp[4][4];
};

static ImGui_ImplDX11_Data* ImGui_ImplDX11_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplDX11_Data*)ImGui::GetIO().BackendRendererUserData : NULL;
}

static void ImGui_ImplDX11_SetupRenderState(ImDrawData* draw_data, ID3D11DeviceContext* ctx)
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();

    D3D11_VIEWPORT vp;
    im_memset(&vp, 0, sizeof(D3D11_VIEWPORT));

    vp.Width = draw_data->DisplaySize.x;
    vp.Height = draw_data->DisplaySize.y;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;

    ctx->RSSetViewports(1, &vp);

    unsigned int stride = sizeof(ImDrawVert);
    unsigned int offset = 0;

    ctx->IASetInputLayout(bd->pInputLayout);
    ctx->IASetVertexBuffers(0, 1, &bd->pVB, &stride, &offset);
    ctx->IASetIndexBuffer(bd->pIB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->VSSetShader(bd->pVertexShader, NULL, 0);
    ctx->VSSetConstantBuffers(0, 1, &bd->pVertexConstantBuffer);

    ctx->PSSetShader(bd->pPixelShader, NULL, 0);
    ctx->PSSetSamplers(0, 1, &bd->pFontSampler);

    ctx->GSSetShader(NULL, NULL, 0);
    ctx->HSSetShader(NULL, NULL, 0);
    ctx->DSSetShader(NULL, NULL, 0);
    ctx->CSSetShader(NULL, NULL, 0);

    const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };

    ctx->OMSetBlendState(bd->pBlendState, blend_factor, 0xffffffff);
    ctx->OMSetDepthStencilState(bd->pDepthStencilState, 0);
    ctx->RSSetState(bd->pRasterizerState);
}

constexpr std::uintptr_t MinimumUserAddress = 0x0000000000010000;
constexpr std::uintptr_t MaximumUserAddress = 0x00007FFFFFFFFFFF;

constexpr std::uintptr_t MinimumSystemAddress = 0x8000000000000000;

inline std::uintptr_t ToAddress(const void* pointer)
{
    return reinterpret_cast<std::uintptr_t>(pointer);
}

inline bool IsUserAddress(std::uintptr_t address)
{
    return (address >= MinimumUserAddress &&
        address <= MaximumUserAddress);
}

inline bool IsUserAddress(const void* pointer)
{
    const auto address = ToAddress(pointer);
    return IsUserAddress(address);
}

inline bool IsAddressValid(std::uintptr_t address)
{
    return IsUserAddress(address);
}

inline bool IsAddressValid(const void* pointer)
{
    const auto address = ToAddress(pointer);
    return IsAddressValid(address);
}

template< typename Type >
inline void SafeDeleteArray(Type& object)
{
    if (IsAddressValid(object))
    {
        delete[] object;
        object = nullptr;
    }
}

template< typename Type >
inline void SafeRelease(Type& object)
{
    if (IsAddressValid(object))
    {
        object->Release();
        object = nullptr;
    }
}

void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data)
{
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    ID3D11DeviceContext* ctx = bd->pd3dDeviceContext;

    if (!bd->pVB || bd->VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (bd->pVB) 
            SafeRelease(bd->pVB);

        bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;

        D3D11_BUFFER_DESC desc;
        im_memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));

        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = bd->VertexBufferSize * sizeof(ImDrawVert);
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        if (bd->pd3dDevice->CreateBuffer(&desc, NULL, &bd->pVB) < 0)
            return;
    }

    if (!bd->pIB || bd->IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (bd->pIB)
            SafeRelease(bd->pIB);

        bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;
        D3D11_BUFFER_DESC desc;
        im_memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = bd->IndexBufferSize * sizeof(ImDrawIdx);
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (bd->pd3dDevice->CreateBuffer(&desc, NULL, &bd->pIB) < 0)
            return;
    }

    D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;

    if (ctx->Map(bd->pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
        return;

    if (ctx->Map(bd->pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
        return;

    ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
    ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        im_memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        im_memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

        vtx_dst += cmd_list->VtxBuffer.Size;
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    ctx->Unmap(bd->pVB, 0);
    ctx->Unmap(bd->pIB, 0);

    {
        D3D11_MAPPED_SUBRESOURCE mapped_resource;
        
        if (ctx->Map(bd->pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
            return;

        VERTEX_CONSTANT_BUFFER* constant_buffer = (VERTEX_CONSTANT_BUFFER*)mapped_resource.pData;
       
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        
        float mvp[4][4] =
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };

        im_memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
        ctx->Unmap(bd->pVertexConstantBuffer, 0);
    }

    struct BACKUP_DX11_STATE
    {
        UINT                        ScissorRectsCount, ViewportsCount;
        D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11RasterizerState*      RS;
        ID3D11BlendState*           BlendState;
        FLOAT                       BlendFactor[4];
        UINT                        SampleMask;
        UINT                        StencilRef;
        ID3D11DepthStencilState*    DepthStencilState;
        ID3D11ShaderResourceView*   PSShaderResource;
        ID3D11SamplerState*         PSSampler;
        ID3D11PixelShader*          PS;
        ID3D11VertexShader*         VS;
        ID3D11GeometryShader*       GS;
        UINT                        PSInstancesCount, VSInstancesCount, GSInstancesCount;
        ID3D11ClassInstance         *PSInstances[256], *VSInstances[256], *GSInstances[256];
        D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
        ID3D11Buffer*               IndexBuffer, *VertexBuffer, *VSConstantBuffer;
        UINT                        IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
        DXGI_FORMAT                 IndexBufferFormat;
        ID3D11InputLayout*          InputLayout;
    };

    BACKUP_DX11_STATE old = {};

    old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;

    ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
    ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
    ctx->RSGetState(&old.RS);

    ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
    ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);

    ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
    ctx->PSGetSamplers(0, 1, &old.PSSampler);

    old.PSInstancesCount = old.VSInstancesCount = old.GSInstancesCount = 256;

    ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
    ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);

    ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
    ctx->GSGetShader(&old.GS, old.GSInstances, &old.GSInstancesCount);

    ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
    ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
    ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
    ctx->IAGetInputLayout(&old.InputLayout);

    ImGui_ImplDX11_SetupRenderState(draw_data, ctx);

    int global_idx_offset = 0;
    int global_vtx_offset = 0;

    ImVec2 clip_off = draw_data->DisplayPos;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (pcmd->UserCallback != NULL)
            {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplDX11_SetupRenderState(draw_data, ctx);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                const D3D11_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
                
                ctx->RSSetScissorRects(1, &r);

                ID3D11ShaderResourceView* texture_srv = (ID3D11ShaderResourceView*)pcmd->GetTexID();

                ctx->PSSetShaderResources(0, 1, &texture_srv);
                ctx->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);
            }
        }

        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
    ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
    ctx->RSSetState(old.RS); if (old.RS) SafeRelease(old.RS);
   
    ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) SafeRelease(old.BlendState);
    ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef); if (old.DepthStencilState) SafeRelease(old.DepthStencilState);
    
    ctx->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) SafeRelease(old.PSShaderResource);
    ctx->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) SafeRelease(old.PSSampler);
    ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) SafeRelease(old.PS);
   
    for (UINT i = 0; i < old.PSInstancesCount; i++) 
        if (old.PSInstances[i]) 
            SafeRelease(old.PSInstances[i]);

    ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) SafeRelease(old.VS);
    ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) SafeRelease(old.VSConstantBuffer);
    
    ctx->GSSetShader(old.GS, old.GSInstances, old.GSInstancesCount); if (old.GS) SafeRelease(old.GS);
   
    for (UINT i = 0; i < old.VSInstancesCount; i++) 
        if (old.VSInstances[i]) 
            SafeRelease(old.VSInstances[i]);

    ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
    ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) SafeRelease(old.IndexBuffer);
    ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) SafeRelease(old.VertexBuffer);
    ctx->IASetInputLayout(old.InputLayout); if (old.InputLayout) SafeRelease(old.InputLayout);
}

static void ImGui_ImplDX11_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    
    unsigned char* pixels;
    int width, height;
    
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    
    {
        D3D11_TEXTURE2D_DESC desc;
        ImZeroMemory(&desc, sizeof(desc));

        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D* pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;

        subResource.pSysMem = pixels;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;

        bd->pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        IM_ASSERT(pTexture != NULL);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ImZeroMemory(&srvDesc, sizeof(srvDesc));

        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;

        bd->pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &bd->pFontTextureView);

        SafeRelease(pTexture);
    }

    io.Fonts->SetTexID((ImTextureID)bd->pFontTextureView);

    {
        D3D11_SAMPLER_DESC desc;
        ImZeroMemory(&desc, sizeof(desc));
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MipLODBias = 0.f;
        desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        desc.MinLOD = 0.f;
        desc.MaxLOD = 0.f;
        bd->pd3dDevice->CreateSamplerState(&desc, &bd->pFontSampler);
    }
}

struct Shader
{
    Shader(uint8_t key, const uint8_t* const code, size_t size)
        : m_key(key)
        , m_code(code)
        , m_data(nullptr)
        , m_size(size)
    {
        m_data = new uint8_t[m_size];
    }

    ~Shader()
    {
        SafeDeleteArray(m_data);
    }

    bool empty() const
    {
        return (m_size == 0);
    }

    size_t size() const
    {
        return m_size;
    }

    const uint8_t* decrypt() const
    {
        for (size_t index = 0; index < m_size; index++)
        {
            m_data[index] = m_code[index] ^ m_key;
        }

        return m_data;
    }

    uint8_t m_key = 0;
    const uint8_t* m_code = nullptr;
    uint8_t* m_data = nullptr;
    size_t m_size = 0;
};

#include "ImShader.h" // TestMemory VS Studio project for encrypt

constexpr std::uint8_t g_vs_key = 0xEDLL;
constexpr std::uint8_t g_ps_key = 0xEDLL;

bool    ImGui_ImplDX11_CreateDeviceObjects()
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    
    if (!bd->pd3dDevice)
        return false;

    if (bd->pFontSampler)
        ImGui_ImplDX11_InvalidateDeviceObjects();

    {
        {
            Shader vs(g_vs_key, g_vs_code, g_vs_size);

            if (bd->pd3dDevice->CreateVertexShader(vs.decrypt(), vs.size(), NULL, &bd->pVertexShader) != S_OK)
                return false;

            auto psn = xorstr ("POSITION");
            auto trd = xorstr ("TEXCOORD");
            auto cor = xorstr ("COLOR");

            D3D11_INPUT_ELEMENT_DESC local_layout[] =
            {
                { psn.crypt_get(), 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF(ImDrawVert, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { trd.crypt_get(), 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF(ImDrawVert, uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { cor.crypt_get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)IM_OFFSETOF(ImDrawVert, col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            if (bd->pd3dDevice->CreateInputLayout(local_layout, 3, vs.decrypt(), vs.size(), &bd->pInputLayout) != S_OK)
                return false;

            psn.crypt();
            trd.crypt();
            cor.crypt();

            vs.~Shader();
        }

        {
            D3D11_BUFFER_DESC desc;

            desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;

            bd->pd3dDevice->CreateBuffer(&desc, NULL, &bd->pVertexConstantBuffer);
        }
    }

    {
        Shader ps(g_ps_key, g_ps_code, g_ps_size);

        if (bd->pd3dDevice->CreatePixelShader(ps.decrypt(), ps.size(), NULL, &bd->pPixelShader) != S_OK)
            return false;

        ps.~Shader();
    }

    {
        D3D11_BLEND_DESC desc;
        ImZeroMemory(&desc, sizeof(desc));
        desc.AlphaToCoverageEnable = false;
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        bd->pd3dDevice->CreateBlendState(&desc, &bd->pBlendState);
    }

    {
        D3D11_RASTERIZER_DESC desc;
        ImZeroMemory(&desc, sizeof(desc));
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.ScissorEnable = true;
        desc.DepthClipEnable = true;
        bd->pd3dDevice->CreateRasterizerState(&desc, &bd->pRasterizerState);
    }

    {
        D3D11_DEPTH_STENCIL_DESC desc;
        ImZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = false;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        desc.StencilEnable = false;
        desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        desc.BackFace = desc.FrontFace;
        bd->pd3dDevice->CreateDepthStencilState(&desc, &bd->pDepthStencilState);
    }

    ImGui_ImplDX11_CreateFontsTexture();

    return true;
}

void    ImGui_ImplDX11_InvalidateDeviceObjects()
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
   
    if (!bd->pd3dDevice)
        return;

    SafeRelease(bd->pFontSampler);
    SafeRelease(bd->pFontTextureView);

    ImGui::GetIO().Fonts->SetTexID(NULL);

    SafeRelease(bd->pIB);
    SafeRelease(bd->pVB);
    SafeRelease(bd->pBlendState);
    SafeRelease(bd->pDepthStencilState);
    SafeRelease(bd->pRasterizerState);
    SafeRelease(bd->pPixelShader);
    SafeRelease(bd->pVertexConstantBuffer);
    SafeRelease(bd->pInputLayout);
    SafeRelease(bd->pVertexShader);
}

bool    ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == NULL);

    ImGui_ImplDX11_Data* bd = IM_NEW(ImGui_ImplDX11_Data)();

    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = xor ("gqcz");
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    IDXGIDevice* pDXGIDevice = NULL;
    IDXGIAdapter* pDXGIAdapter = NULL;
    IDXGIFactory* pFactory = NULL;

    if (device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK)
        if (pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK)
            if (pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK)
            {
                bd->pd3dDevice = device;
                bd->pd3dDeviceContext = device_context;
                bd->pFactory = pFactory;
            }

    SafeRelease(pDXGIDevice);
    SafeRelease(pDXGIAdapter);

    bd->pd3dDevice->AddRef();
    bd->pd3dDeviceContext->AddRef();

    return true;
}

void ImGui_ImplDX11_Shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();

    ImGui_ImplDX11_InvalidateDeviceObjects();

    SafeRelease(bd->pFactory);
    SafeRelease(bd->pd3dDevice);
    SafeRelease(bd->pd3dDeviceContext);

    io.BackendRendererName = NULL;
    io.BackendRendererUserData = NULL;
    
    IM_DELETE(bd);
}

void ImGui_ImplDX11_NewFrame()
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    IM_ASSERT(bd != NULL);

    if (bd->pd3dDeviceContext)
    {
        UINT viewport_count = 1;
        D3D11_VIEWPORT viewport = { };

        bd->pd3dDeviceContext->RSGetViewports(&viewport_count, &viewport);

        auto& io = ImGui::GetIO();

        io.DisplaySize = { viewport.Width, viewport.Height };
    }

    if (!bd->pFontSampler)
        ImGui_ImplDX11_CreateDeviceObjects();
}
