#include "stdafx.h"
RelocAddr<uintptr_t> FullscreenAddress(0xD42361);
RelocAddr<uintptr_t> BorderlessAddress(0xD4236C);
RelocAddr<uintptr_t> CreateDXGIFactoryAddress(0x1D1748B);
RelocAddr<uintptr_t> D3D11CreateDeviceAndSwapChainAddress(0x1D17879);
RelocAddr<uintptr_t> ResizeTargetDisableAddress(0x1D0AEB5);
RelocAddr<uintptr_t> ResizeBuffersDisableAddress(0x1D0AC27);
RelocAddr<uintptr_t> ResizeTargetAddress(0x1D0AFA9);
RelocAddr<uintptr_t> PresentInjectAddress(0x1D0B699);
RelocAddr<uintptr_t> ResizeBuffersInjectAddress(0x1D0ADA0);
RelocAddr<uintptr_t> MovRaxRcxAddress(0x1D0ADB8);

PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN D3D11CreateDeviceAndSwapChain_Original = nullptr;

struct {
    uint32_t width;
    uint32_t height;
    uint32_t flags;
}swapchain;

DXGI_MODE_DESC modeDesc;

struct
{
    uint32_t caps;
} dxgi;

typedef HRESULT(WINAPI* CreateDXGIFactory_T)(REFIID riid, _COM_Outptr_ void** ppFactory);
constexpr uint32_t DXGI_CAP_FLIP_DISCARD = 0x00000001U;
constexpr uint32_t DXGI_CAP_FLIP_SEQUENTIAL = 0x00000002U;
constexpr uint32_t DXGI_CAP_TEARING = 0x00000004U;

constexpr uint32_t DXGI_CAPS_ALL = 0xFFFFFFFFU;


class D3D11CreateEvent
{
public:
    D3D11CreateEvent(
        CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) :
        m_pSwapChainDesc(pSwapChainDesc)
    {}

    CONST DXGI_SWAP_CHAIN_DESC* const m_pSwapChainDesc;
};

typedef D3D11CreateEvent D3D11CreateEventPre;

bool IsFlipOn(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) {
    return pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL ||
        pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD;
}

UINT present_flags;

IDXGIFactory* pFactory;

IDXGIFactory* DXGI_GetFactory();
CreateDXGIFactory_T CreateDXGIFactory_O;

#pragma pack(push, 1)
struct CB5Code
{
    uint8_t	op;
    int32_t	displ;
};
#pragma pack(pop)

template <uint8_t op>
bool GetDst5(uintptr_t addr, uintptr_t& out)
{
    static_assert(op == 0xE8 || op == 0xE9, "invalid opcode");

    auto ins = reinterpret_cast<CB5Code*>(addr);

    if (ins->op != op) {
        return false;
    }

    out = addr + sizeof(CB5Code) + ins->displ;

    return true;
}

template <typename T>
bool Call5(uintptr_t addr, uintptr_t dst, T& orig)
{
    uintptr_t o;
    if (!GetDst5<0xE8>(addr, o)) {
        return false;
    }

    orig = reinterpret_cast<T>(o);

    g_branchTrampoline.Write5Call(addr, dst);

    return true;
}

void safe_memset(uintptr_t addr, int val, size_t len)
{
    DWORD oldProtect;
    ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &oldProtect));
    memset(reinterpret_cast<void*>(addr), val, len);
    ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, oldProtect, &oldProtect));
}

template <typename T>
void safe_write(uintptr_t addr, T val)
{
    safe_write(addr, reinterpret_cast<const void*>(&val), sizeof(val));
}

void safe_write(uintptr_t addr, const void* data, size_t len)
{
    DWORD oldProtect;
    ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &oldProtect));
    memcpy(reinterpret_cast<void*>(addr), data, len);
    ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, oldProtect, &oldProtect));
}

const char* GetSwapEffectOption(DXGI_SWAP_EFFECT a_swapEffect)
{
    switch (a_swapEffect)
    {
    case DXGI_SWAP_EFFECT_DISCARD:
        return "discard";
    case DXGI_SWAP_EFFECT_SEQUENTIAL:
        return "sequential";
    case DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL:
        return "flip_sequential";
    case DXGI_SWAP_EFFECT_FLIP_DISCARD:
        return "flip_discard";
    default:
        return "unknown";
    }
}

bool ValidateDisplayMode(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) {
    if (pSwapChainDesc->BufferDesc.RefreshRate.Numerator > 0 &&
        !pSwapChainDesc->BufferDesc.RefreshRate.Denominator) {
        return false;
    }

    return true;
}

UINT GetRefreshRate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) {
    if (!pSwapChainDesc->BufferDesc.RefreshRate.Denominator) {
        return 0U;
    }
    return pSwapChainDesc->BufferDesc.RefreshRate.Numerator /
        pSwapChainDesc->BufferDesc.RefreshRate.Denominator;
}

void SetFPSLimitOverride() {
    if (Vsync) {
        if (Tearing) {
            present_flags |= DXGI_PRESENT_ALLOW_TEARING;
        }
    }
    else {
        if (Tearing) {
            present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
        }
    }
}

void ResetFPSLimitOverride() {
    if (Vsync) {
        if (Tearing) {
            present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
        }
    }
}

void PatchDisplay() {
    if (Borderless) {
        SafeWriteBuf(BorderlessAddress.GetUIntPtr(), "\xB8\x01\x00\x00\x00\x90\x90", 7);
    }
    else {
        SafeWriteBuf(BorderlessAddress.GetUIntPtr(), "\xB8\x00\x00\x00\x00\x90\x90", 7);
    }

    if (Fullscreen) {
        SafeWriteBuf(FullscreenAddress.GetUIntPtr(), "\xB8\x01\x00\x00\x00\x90\x90", 7);
    }
    else {
        SafeWriteBuf(FullscreenAddress.GetUIntPtr(), "\xB8\x00\x00\x00\x00\x90\x90", 7);
    }
    if (!Fullscreen) {
        if (ResizeBuffersDisable) {
            _MESSAGE("Disabled swap chain buffer resizing");
            SafeWriteBuf(ResizeBuffersDisableAddress.GetUIntPtr(), "\xE9\x46\x02\x00\x00\x90", 6);
        }
        if (ResizeTargetDisable) {
            _MESSAGE("Disabled swap chain target resizing");
            SafeWriteBuf(ResizeTargetDisableAddress.GetUIntPtr(), "\xE9\x4F\x01\x00\x00\x90", 6);
        }
    }
    else {
        struct ResizeTargetInjectArgs : JITASM::JITASM {
            ResizeTargetInjectArgs(uintptr_t retnAddr, uintptr_t mdescAddr) : JITASM()
            {
                Xbyak::Label mdescLabel;
                Xbyak::Label retnLabel;

                mov(rdx, ptr[rip + mdescLabel]);
                mov(rax, ptr[rcx]);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(retnAddr);

                L(mdescLabel);
                dq(mdescAddr);
            }
        };
        _MESSAGE("\nIDXGISwapChain::ResizeTarget patching...");
        {
            ResizeTargetInjectArgs code(ResizeTargetAddress + 0x8, uintptr_t(&modeDesc));
            g_branchTrampoline.Write6Branch(ResizeTargetAddress, code.get());
            safe_memset(ResizeTargetAddress + 0x6, 0xCC, 0x2);
        }
        _MESSAGE("IDXGISwapChain::ResizeTarget OK");
    }

    if (!Fullscreen && Tearing && (!Vsync || limit)) {
        struct PresentFlagsInject : JITASM::JITASM {
            PresentFlagsInject(uintptr_t retnAddr, uintptr_t flagsAddr) : JITASM()
            {
                Xbyak::Label flagsLabel;
                Xbyak::Label retnLabel;

                mov(rcx, ptr[rip + flagsLabel]);
                mov(r8d, ptr[rcx]);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(retnAddr);

                L(flagsLabel);
                dq(flagsAddr);
            }
        };
        _MESSAGE("IDXGISwapChain::Present patching..");
        {
            PresentFlagsInject code(PresentInjectAddress + 0x7, uintptr_t(&present_flags));
            g_branchTrampoline.Write6Branch(PresentInjectAddress, code.get());
            safe_write<uint8_t>(PresentInjectAddress + 0x6, 0xCC);
        }
        _MESSAGE("IDXGISwapChain::Present OK");
    }
    {
        struct ResizeBuffersInjectArgs : JITASM::JITASM {
            ResizeBuffersInjectArgs(uintptr_t retnAddr, uintptr_t swdAddr) : JITASM()
            {
                Xbyak::Label retnLabel;
                Xbyak::Label bdLabel;

                mov(rdx, ptr[rip + bdLabel]);
                mov(r8d, ptr[rdx]);
                mov(r9d, ptr[rdx + 4]);
                mov(eax, ptr[rdx + 8]);
                mov(ptr[rsp + 0x28], eax);

                mov(dword[rsp + 0x20], DXGI_FORMAT_UNKNOWN);

                xor (edx, edx);

                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(retnAddr);

                L(bdLabel);
                dq(swdAddr);
            }
        };
        _MESSAGE("IDXGISwapChain::ResizeBuffers patching...");
        {
            ResizeBuffersInjectArgs code(ResizeBuffersInjectAddress + 0x18, uintptr_t(&swapchain));
            g_branchTrampoline.Write6Branch(ResizeBuffersInjectAddress, code.get());
            safe_memset(ResizeBuffersInjectAddress + 0x6, 0xCC, 0x12);
        }
        SafeWriteBuf(MovRaxRcxAddress.GetUIntPtr(), "\x48\x8B\x01", 3);
        _MESSAGE("IDXGISwapChain::ResizeBuffers OK");
    }
}

DXGI_SWAP_EFFECT AutoGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) {
    if (pSwapChainDesc->Windowed == TRUE) {
        if (dxgi.caps & DXGI_CAP_FLIP_DISCARD) {
            return DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }
        else if (dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL) {
            return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        }
    }
    return DXGI_SWAP_EFFECT_DISCARD;
}

DXGI_SWAP_EFFECT ManualGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) {
    auto se = DXGI_SWAP_EFFECT_DISCARD;
    if (DXGISwapEffect == 1) {
       se = DXGI_SWAP_EFFECT_DISCARD;
    }
    if (DXGISwapEffect == 2) {
        se = DXGI_SWAP_EFFECT_SEQUENTIAL;
    }
    if (DXGISwapEffect == 3) {
        se = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    }
    if (DXGISwapEffect == 4) {
        se = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    }
    if (pSwapChainDesc->Windowed == TRUE) {
        auto nse = se;
        if (nse == DXGI_SWAP_EFFECT_FLIP_DISCARD &&
            !(dxgi.caps & DXGI_CAP_FLIP_DISCARD)) {
            nse = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        }
        if (nse == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL &&
            !(dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL)) {
            nse = DXGI_SWAP_EFFECT_DISCARD;
        }
        if (nse != se) {
            _MESSAGE("%s not supported, using %s", GetSwapEffectOption(se), GetSwapEffectOption(nse));
            se = nse;
        }
    }
    return se;
}

void DXGI_GetCapabilities() {
    bool release;
    IDXGIFactory* factory;

    if (!pFactory)
    {
        _MESSAGE("IDXGIFactory not set, attempting to create..");

        factory = DXGI_GetFactory();
        if (!factory) {
            _MESSAGE("Couldn't create IDXGIFactory, assuming DXGI_CAPS_ALL");
            dxgi.caps = DXGI_CAPS_ALL;
            return;
        }

        release = true;
    }
    else {
        factory = pFactory;
        release = false;
    }

    dxgi.caps = 0;

    do
    {
        {
            Microsoft::WRL::ComPtr<IDXGIFactory5> tmp;
            if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                dxgi.caps = (
                    DXGI_CAP_FLIP_SEQUENTIAL |
                    DXGI_CAP_FLIP_DISCARD);

                BOOL allowTearing;
                HRESULT hr = tmp->CheckFeatureSupport(
                    DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                    &allowTearing, sizeof(allowTearing));

                if (SUCCEEDED(hr) && allowTearing) {
                    dxgi.caps |= DXGI_CAP_TEARING;
                }

                break;
            }
        }

        {
            Microsoft::WRL::ComPtr<IDXGIFactory4> tmp;
            if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                dxgi.caps = (
                    DXGI_CAP_FLIP_SEQUENTIAL |
                    DXGI_CAP_FLIP_DISCARD);

                break;
            }
        }

        {
            Microsoft::WRL::ComPtr<IDXGIFactory3> tmp;
            if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                dxgi.caps = DXGI_CAP_FLIP_SEQUENTIAL;
            }
        }
    } while (0);

    if (release)
        factory->Release();
}

void ApplyD3DSettings(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
{
    if (pSwapChainDesc->Windowed == TRUE) {
        DXGI_GetCapabilities();
    }
    if (DXGISwapEffect == 0) {
        pSwapChainDesc->SwapEffect = AutoGetSwapEffect(pSwapChainDesc);
    }
    else {
        pSwapChainDesc->SwapEffect = ManualGetSwapEffect(pSwapChainDesc);
    }

    bool flip_model = IsFlipOn(pSwapChainDesc);
    if (pSwapChainDesc->Windowed == TRUE) {
        if (Tearing && flip_model &&
            (!Vsync || limit))
        {
            if (dxgi.caps & DXGI_CAP_TEARING) {
                pSwapChainDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

                if (!Vsync) {
                    present_flags |= DXGI_PRESENT_ALLOW_TEARING;
                }

                Tearing = true;
            }
            else {
                _MESSAGE("DXGI_FEATURE_PRESENT_ALLOW_TEARING not supported");
            }
        }
    }
    else {
        if (ScalingMode == 1) {
            pSwapChainDesc->BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        }
        if (ScalingMode == 2) {
            pSwapChainDesc->BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
        }
        if (ScalingMode == 3) {
            pSwapChainDesc->BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
        }
    }

    if (SwapBufferCount == 0) {
        if (flip_model) {
            pSwapChainDesc->BufferCount = 3;
        }
        else {
            pSwapChainDesc->BufferCount = 2;
        }
    }
    else if (SwapBufferCount > 0) {
        pSwapChainDesc->BufferCount = static_cast<UINT>(SwapBufferCount);
    }

    if (flip_model) {
        pSwapChainDesc->SampleDesc.Count = 1;
        pSwapChainDesc->SampleDesc.Quality = 0;

        if (pSwapChainDesc->BufferCount < 2) {
            pSwapChainDesc->BufferCount = 2;
            _MESSAGE("Buffer count below the minimum required for flip model, increasing to 2");
        }
    }

    modeDesc = pSwapChainDesc->BufferDesc;

    if (pSwapChainDesc->Windowed == FALSE) {
        modeDesc.Format = DXGI_FORMAT_UNKNOWN;
    }

    swapchain.flags = pSwapChainDesc->Flags;
}

bool HasWindowedHWCompositionSupport(IDXGIAdapter* adapter) {
    if (adapter == nullptr) {
        return false;
    }

    for (UINT i = 0;; ++i) {
        Microsoft::WRL::ComPtr<IDXGIOutput> output;
        if (FAILED(adapter->EnumOutputs(i, &output))) {
            break;
        }

        Microsoft::WRL::ComPtr<IDXGIOutput6> output6;
        if (SUCCEEDED(output.As(&output6))) {
            UINT flags;
            HRESULT hr = output6->CheckHardwareCompositionSupport(&flags);

            if (SUCCEEDED(hr) && (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED)) {
                return true;
            }
        }
    }

    return false;
}

void OnD3D11PreCreate(IDXGIAdapter* pAdapter, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) {
    if (!ValidateDisplayMode(pSwapChainDesc)) {
        _MESSAGE("Invalid refresh rate: (%u/%u)",
            pSwapChainDesc->BufferDesc.RefreshRate.Numerator,
            pSwapChainDesc->BufferDesc.RefreshRate.Denominator);
    }

    ApplyD3DSettings(const_cast<DXGI_SWAP_CHAIN_DESC*>(pSwapChainDesc));

    _MESSAGE("[D3D11] Requesting mode: %ux%u@%u | VSync: %u | Windowed: %d",
        pSwapChainDesc->BufferDesc.Width, pSwapChainDesc->BufferDesc.Height,
        GetRefreshRate(pSwapChainDesc), Vsync, pSwapChainDesc->Windowed);

    _MESSAGE("[D3D11] Swap effect: %s | Buffer count: %u | Flags: 0x%.8X",
        GetSwapEffectOption(pSwapChainDesc->SwapEffect), pSwapChainDesc->BufferCount, pSwapChainDesc->Flags);

    _MESSAGE("[D3D11] Windowed hardware composition support: %s",
        HasWindowedHWCompositionSupport(pAdapter) ? "yes" : "no");

    if (pSwapChainDesc->Windowed == TRUE) {
        if (!IsFlipOn(pSwapChainDesc)) {
            if (!(dxgi.caps & (DXGI_CAP_FLIP_DISCARD | DXGI_CAP_FLIP_SEQUENTIAL))) {
                _MESSAGE("Flip not supported on your system, switch to exclusive fullscreen for better peformance");
            }
            else {
                _MESSAGE("Switch to exclusive fullscreen or set SwapEffect to flip_discard or flip_sequential for better peformance");
            }
        }
    }
    else {
        if (IsFlipOn(pSwapChainDesc)) {
            _MESSAGE("Using flip in exclusive fullscreen may cause issues");
        }
    }
}

HRESULT WINAPI D3D11CreateDeviceAndSwapChain_Hook(
    _In_opt_ IDXGIAdapter* pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    _In_reads_opt_(FeatureLevels) D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    _In_opt_ DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
    _COM_Outptr_opt_ IDXGISwapChain** ppSwapChain,
    _COM_Outptr_opt_ ID3D11Device** ppDevice,
    _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
    _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext) {
    if (Fullscreen) {
        pSwapChainDesc->Windowed = FALSE;
    }
    OnD3D11PreCreate(pAdapter, pSwapChainDesc);
    auto evd_pre = D3D11CreateEventPre(pSwapChainDesc);

    HRESULT hr = D3D11CreateDeviceAndSwapChain_Original(
        pAdapter, DriverType, Software, Flags,
        pFeatureLevels, FeatureLevels, SDKVersion,
        pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel,
        ppImmediateContext);
    if (hr == S_OK) {
        //OnD3D11PostCreate(pSwapChainDesc, ppDevice);
    }
    else {
        _MESSAGE("D3D11CreateDeviceAndSwapChain failed: 0x%lX", hr);
    }
    return hr;
}

HRESULT WINAPI CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory) {
    HRESULT hr = CreateDXGIFactory_O(riid, ppFactory);
    if (SUCCEEDED(hr)) {
        pFactory = static_cast<IDXGIFactory*>(*ppFactory);
    }
    else {
        _MESSAGE("CreateDXGIFactory failed (%lX)", hr);
        abort();
    }
    return hr;
}

IDXGIFactory* DXGI_GetFactory() {
    HMODULE hModule = LoadLibraryA("dxgi.dll");
    if (!hModule)
        return nullptr;

    auto func = reinterpret_cast<CreateDXGIFactory_T>(GetProcAddress(hModule, "CreateDXGIFactory"));
    if (!func)
        return nullptr;

    IDXGIFactory* pFactory;
    if (!SUCCEEDED(func(IID_PPV_ARGS(&pFactory))))
        return nullptr;

    return pFactory;
}

void Hook() {
    Call5(CreateDXGIFactoryAddress.GetUIntPtr(), reinterpret_cast<uintptr_t>(CreateDXGIFactory_Hook), CreateDXGIFactory_O);
    Call5(D3D11CreateDeviceAndSwapChainAddress.GetUIntPtr(), reinterpret_cast<uintptr_t>(D3D11CreateDeviceAndSwapChain_Hook), D3D11CreateDeviceAndSwapChain_Original);
}