#include "pch.h"
#include <DirectXTK\Src\PlatformHelpers.h>
#include "FramerateLimiter.h"
RelocAddr<uintptr_t> D3D11CreateDeviceAndSwapChainAddress(0x1D17879); //E8 7007C200          4C 8B 05 FB463804    8B 0D 29CFA104       49 8D 50 58          BF 180B0000          48 89 15 F5463804
RelocAddr<uintptr_t> ResizeTargetAddress(0x1D0AFA9); //48 8D 54 24 50       48 8B 01             FF 50 70             48 8B 4B 68          FF 15 2AD1F000
RelocAddr<uintptr_t> ResizeBuffersInjectAddress(0x1D0ADA0); //45 33 C9             48 8B 01             45 33 C0             33 D2                C7 44 24 28 02000000 C7 44 24 20 00000000 FF 50 68             8B 8C 24 88000000    E8 16CE0000
RelocAddr<uintptr_t> MovRaxRcxAddress(0x1D0ADB5); //24 20                00 00                00 00                FF 50 68             8B 8C 24 88000000    E8 16CE0000          48 8D 0D 5F4FB401    E8 AA7F0200          48 8B 05 0B5FB401
RelocAddr<uintptr_t> BethesdaFPSCap1Address(0xD423BA); //3C 00                00 00                EB 0D                33 D2                B8 3C000000          F7 F1                89 44 24 30          33 C0                48 8D 0D 2AE54905
RelocAddr<uintptr_t> BethesdaFPSCap2Address(0xD423C3); //3C 00                00 00                F7 F1                89 44 24 30          33 C0                48 8D 0D 2AE54905
RelocAddr<uintptr_t> BorderlessAddress(0xD4236C); //0FB6 05 FDD8B002     85 C9                88 44 24 35          8B 05 79D8B002       0F95 44 24 36
RelocAddr<uintptr_t> FullscreenAddress(0xD42361); //0FB6 05 F0D8B002     88 44 24 34          0FB6 05 FDD8B002     85 C9                88 44 24 35
RelocAddr<uintptr_t> FullscreenAddress1(0xCAAC70);
RelocAddr<uintptr_t> FullscreenAddress11(0xCAAC73);
RelocAddr<uintptr_t> FullscreenAddress2(0xCAACA1);
RelocAddr<uintptr_t> ResizeBuffersDisableAddress(0x1D0AC27); //0F84 45020000        48 8B 4F 68          48 8D 54 24 30       48 89 6C 24 58       4C 89 74 24 40
RelocAddr<uintptr_t> ResizeTargetDisableAddress(0x1D0AEB5); //0F84 4E010000        48 8B 4B 68          48 8D 54 24 40       48 89 B4 24 D0000000
RelocAddr<uintptr_t> CreateDXGIFactoryAddress(0x1D1748B); //E8 640BC200          48 8B 4C 24 60       48 8B 05 E44A3804    33 F6                4C 8D 85 B0010000    48 89 B5 B0010000
RelocAddr<uintptr_t> PresentAddress(0x1D0B6B8); //8B 50 40             41 FF 51 40          83 3D 66698304 08    75 38                3D 0A007A88
RelocAddr<uintptr_t> UntieAddress(0x1B1393B); //08 00                00 00                48 8B C1             48 3B CA             48 0F42 C2           0F57 C0              F3 48 0F2A C0
RelocAddr<uintptr_t> iFPSClampAddress(0x1B138E8); //08 0F                57                   C0 0F 2E             D0 74 38 0F          28 CA                F3 0F59 15 98781301
RelocAddr<uintptr_t> BlackLoadingScreensAddress(0x1297076); //75 1E                40 38 BB 78020000    75 2E                48 8B 46 20          48 89 83 E0000000    0FB6 46 28           88 83 70020000       
RelocAddr<uintptr_t> LoadingScreensAddress(0xCBFFCD); //83 7F 68 02          0F84 A9FEFFFF        33 C9                E8 3287E800          89 1C 2E             48 8B 5C 24 30
RelocAddr<uintptr_t> FrametimeAddress(0x5A66FE8);
RelocAddr<uintptr_t> PostLoadInjectAddress(0x126D75B); //F3 0F10 05 71DF8E04  48 8B 53 30          48 8B 4B 20
RelocAddr<uintptr_t> LoadScreenPlusLimiterAddress(0x1D0B67E);

using namespace Microsoft::WRL;
namespace SDT
{
    namespace Payloads
    {
        static inline constexpr std::uint8_t screen_patch[] = { //Borderless and fullscreen patch
            0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90
        };
        static inline constexpr std::uint8_t fullscreen1_patch[] = {
            0x41, 0x80, 0xFB, 0x01, 0x90, 0x90, 0x90
        };
        static inline constexpr std::uint8_t fullscreenJMP_patch[] = {
            0xEB, 0x18
        };
        static inline constexpr std::uint8_t fullscreenNOP_patch[] = {
            0x90, 0x90
        };
        static inline constexpr std::uint8_t untie_patch[] = {
            0x00
        };
        static inline constexpr std::uint8_t ifpsclamp_patch[] = {
            0x38
        };
        static inline constexpr std::uint8_t disable_blackloading_patch[] = {
            0xEB
        };
        static inline constexpr std::uint8_t disable_loading_patch[] = {
            0x90, 0x90, 0x90, 0x90
        };
        static inline constexpr std::uint8_t ResizeBuffersDisable[] = {
            0xE9, 0x46, 0x02, 0x00, 0x00, 0x90
        };
        static inline constexpr std::uint8_t ResizeTargetDisable[] = {
            0xE9, 0x4F, 0x01, 0x00, 0x00, 0x90
        };
        static inline constexpr std::uint8_t res_patch[] = {
            0xB8, 0x00, 0x00, 0x00, 0x00, 0x90
        };
    }
    static constexpr const char* SECTION_MAIN = "Main";
    static constexpr const char* CONF_UNTIE = "UntieSpeedFromFPS";
    static constexpr const char* CONF_DISABLEIFPSCLAMP = "DisableiFPSClamp";
    static constexpr const char* CONF_VSYNC = "EnableVSync";
    static constexpr const char* CONF_DISABLEVSYNC = "DisableVSyncWhileLoading";
    static constexpr const char* CONF_DISABLEBLACKLOADING = "DisableBlackLoadingScreens";
    static constexpr const char* CONF_DISABLELOADING = "DisableAnimationOnLoadingScreens";
    static constexpr const char* CONF_POSTLOADINGSPEED = "PostloadingMenuSpeed";

    static constexpr const char* CONF_ONOFFDT = "EnableDisplayTweaks";
    static constexpr const char* CONF_FULLSCREEN = "Fullscreen";
    static constexpr const char* SECTION_DISPLAY = "Display";
    static constexpr const char* CONF_BORDERLESS = "Borderless";
    static constexpr const char* CONF_BORDERLESSUPSCALE = "BorderlessUpscale";
    static constexpr const char* CONF_RESOLUTION = "Resolution";
    static constexpr const char* CONF_BUFFERSDISABLE = "ResizeBuffersDisable";
    static constexpr const char* CONF_TARGETDISABLE = "ResizeTargetDisable";
    static constexpr const char* CONF_TEARING = "AllowTearing";
    static constexpr const char* CONF_BUFFERCOUNT = "BufferCount";
    static constexpr const char* CONF_SWAPEFFECT = "SwapEffect";
    static constexpr const char* CONF_RESOLUTON = "Resolution";
    static constexpr const char* CONF_SCALINGMODE = "ScalingMode";

    static constexpr const char* SECTION_LIMIT = "Limiter";
    static constexpr const char* CONF_INGAMEFPS = "InGameFPS";
    static constexpr const char* CONF_UILOADINGFPS = "LoadingScreenFPS";
    static constexpr const char* CONF_UILOCKFPS = "LockpickingFPS";
    static constexpr const char* CONF_UIPIPFPS = "PipBoyFPS";
    static constexpr const char* CONF_FPSLIMIT_MODE = "FramerateLimitMode";

    static constexpr const char* SECTION_FIXES = "Fixes";
    static constexpr const char* CONF_FIXTHREADS = "FixCPUThreads";

    using namespace Patching;
    DRender DRender::m_Instance;

    const char* DRender::GetSwapEffectOption(DXGI_SWAP_EFFECT a_swapEffect)
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

    DRender::DRender() :
        has_swap_effect(false),
        limiter_installed(false),
        has_fl_override(false),
        fps_limit(-1),
        lslExtraTime(0),
        lslPostLoadExtraTime(0),
        gameLoadState(0),
        m_dxgiFactory(nullptr),
        m_swapchain{ 0, 0, 0 }
    {
        m_swapchain.flags = 0;
        m_swapchain.width = 0;
        m_swapchain.height = 0;
    }

    std::uint8_t DRender::GetScreenModeSetting(
        IConfigGame& a_gameConfig,
        const char* a_section,
        const char* a_key,
        const char* a_prefkey,
        bool a_default)
    {
        if (!HasConfigValue(a_section, a_key))
        {
            bool result;
            if (a_gameConfig.Get(SECTION_DISPLAY, a_prefkey, a_default, result))
            {
                _MESSAGE("[Render] %s: Using game setting (%s): %hhu", a_key, a_prefkey, result);

                return static_cast<std::uint8_t>(result);
            }

            return static_cast<std::uint8_t>(a_default);
        }

        return static_cast<std::uint8_t>(GetConfigValueBool(a_section, a_key, a_default));
    }

    void DRender::LoadConfig()
    {
        IConfigGame gameConfig(FALLOUT4_PREFS_INI_FILE);
        
        m_conf.untie = GetConfigValueBool("Main", "UntieSpeedFromFPS", true);
        m_conf.disable_clamp = GetConfigValueBool(SECTION_MAIN, CONF_DISABLEIFPSCLAMP, true);
        m_conf.vsync_on = GetConfigValueBool(SECTION_MAIN, CONF_VSYNC, true);
        if (m_conf.vsync_on) {
            m_vsync_present_interval = 1;
            m_current_vsync_present_interval = 1;
        }
        m_conf.enable_tearing = GetConfigValueBool(SECTION_MAIN, CONF_TEARING, false);
        m_conf.disable_vsync = GetConfigValueBool(SECTION_MAIN, CONF_DISABLEVSYNC, true);
        m_conf.disable_black_loading = GetConfigValueBool(SECTION_MAIN, CONF_DISABLEBLACKLOADING, false);
        m_conf.disable_loading_screens = GetConfigValueBool(SECTION_MAIN, CONF_DISABLELOADING, false);
        m_conf.postloading_speed = GetConfigValueFloat(SECTION_MAIN, CONF_POSTLOADINGSPEED, 1.0f);
        m_conf.fullscreen = GetScreenModeSetting(gameConfig, SECTION_DISPLAY, CONF_FULLSCREEN, "bFull Screen", false);
        m_conf.borderless = GetScreenModeSetting(gameConfig, SECTION_DISPLAY, CONF_BORDERLESS, "bBorderless", true);
        m_conf.upscale = GetConfigValueBool(SECTION_DISPLAY, CONF_BORDERLESSUPSCALE, false);
        m_conf.swap_effect = GetConfigValueInteger(SECTION_DISPLAY, CONF_SWAPEFFECT, 0);
        m_conf.upscale_select_primary_monitor = true;
        m_conf.disablebufferresize = GetConfigValueBool(SECTION_DISPLAY, CONF_BUFFERSDISABLE, false);
        m_conf.disabletargetresize = GetConfigValueBool(SECTION_DISPLAY, CONF_TARGETDISABLE, false);
        m_conf.scale_mode = GetConfigValueInteger(SECTION_DISPLAY, CONF_SCALINGMODE, 1);
        if (m_conf.scale_mode == 1) {
            m_conf.scaling_mode = DXGI_MODE_SCALING_UNSPECIFIED;
        }
        if (m_conf.scale_mode == 2) {
            m_conf.scaling_mode = DXGI_MODE_SCALING_CENTERED;
        }
        if (m_conf.scale_mode == 3) {
            m_conf.scaling_mode = DXGI_MODE_SCALING_STRETCHED;
        }
        m_conf.buffer_count = GetConfigValueInteger(SECTION_DISPLAY, CONF_BUFFERCOUNT, 0);
        if (m_conf.buffer_count > -1) {
            m_conf.buffer_count = std::clamp<std::int32_t>(m_conf.buffer_count, 0, 8);
        }
        m_conf.limit_mode = std::clamp<std::uint8_t>(GetConfigValue<std::uint8_t>(SECTION_LIMIT, CONF_FPSLIMIT_MODE, 0), 0, 1);
        m_conf.limits.game = GetConfigValueFloat (SECTION_LIMIT, CONF_INGAMEFPS, -1.0f);
        m_conf.limits.ui_loadscreen = GetConfigValueFloat(SECTION_LIMIT, CONF_UILOADINGFPS, -1.0f);
        m_conf.limits.ui_lockpick = GetConfigValueFloat(SECTION_LIMIT, CONF_UILOCKFPS, -1.0f);
        m_conf.limits.ui_pipboy = GetConfigValueFloat(SECTION_LIMIT, CONF_UIPIPFPS, -1.0f);
        if (!ConfigParseResolution(GetConfigValue(SECTION_DISPLAY, CONF_RESOLUTON, "-1 -1"), m_conf.resolution)) {
            m_conf.resolution[0] = -1;
            m_conf.resolution[1] = -1;
        }
        m_conf.fixcputhreads = GetConfigValueBool(SECTION_FIXES, CONF_FIXTHREADS, true);
    }

    bool DRender::ConfigParseResolution(const std::string& in, std::int32_t(&a_out)[2])
    {
        std::vector<std::int32_t> v2;
        StrHelpers::SplitString<std::int32_t>(in, 'x', v2);

        if (v2.size() < 2)
            return false;

        a_out[0] = v2[0];
        a_out[1] = v2[1];
        return true;
    }

    void DRender::PostLoadConfig()
    {
        tts = IPerfCounter::Query();

        if (m_conf.upscale) {
            if (!(!m_conf.fullscreen && m_conf.borderless))
            {
                m_conf.upscale = false;
            }
            else {
                _MESSAGE("[Render] Borderless upscaling enabled");
            }
        }

        if (m_conf.limits.game > 0.0f) {
            current_fps_max = fps_max = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.game)) * 1000000.0L);
            //fps60 = static_cast<long long>((1.0L / static_cast<long double>(60)) * 1000000.0L); //Home клавиша!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            fps_limit = 1;
        }
        else if (m_conf.limits.game == 0.0f) {
            fps_limit = 0;
        }
        else {
            fps_limit = -1;
        }
        fps60 = static_cast<long long>((1.0L / 60L) * 1000000.0L); //ТЕСТ
        if (m_conf.limits.ui_loadscreen > 0.0f) {
                loading_fps = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.ui_loadscreen)) * 1000000.0L);
        }
        if (m_conf.limits.ui_lockpick > 0.0f) {
            lockpick_fps = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.ui_lockpick)) * 1000000.0L);
        }
        if (m_conf.limits.ui_pipboy > 0.0f) {
            pipboy_fps = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.ui_pipboy)) * 1000000.0L);
        }
    }

    void DRender::Patch()
    {
        std::uint32_t maxrefreshrate = 10000;
        safe_write(
            BethesdaFPSCap1Address, 
            static_cast<std::uint32_t>(maxrefreshrate)
        );
        safe_write(
            BethesdaFPSCap2Address,
            static_cast<std::uint32_t>(maxrefreshrate)
        );
        safe_write(
            BorderlessAddress,
            reinterpret_cast<const void*>(Payloads::screen_patch),
            sizeof(Payloads::screen_patch)
        );
        safe_write(
            BorderlessAddress + 0x1,
            static_cast<std::uint32_t>(m_Instance.m_conf.borderless)
        );
        safe_write(
            FullscreenAddress1,
            reinterpret_cast<const void*>(Payloads::fullscreen1_patch),
            sizeof(Payloads::fullscreen1_patch)
        );
        safe_write(
            FullscreenAddress1 + 0x3,
            static_cast<std::uint32_t>(m_Instance.m_conf.fullscreen)
        );
        safe_write(
            FullscreenAddress1 + 0x4,
            reinterpret_cast<const void*>(Payloads::fullscreenNOP_patch),
            sizeof(Payloads::fullscreenNOP_patch)
        );
        if (m_conf.fullscreen) {
            safe_write(
                FullscreenAddress2,
                reinterpret_cast<const void*>(Payloads::fullscreenJMP_patch),
                sizeof(Payloads::fullscreenJMP_patch)
            );
        }
        else {
            safe_write(
                FullscreenAddress2,
                reinterpret_cast<const void*>(Payloads::fullscreenNOP_patch),
                sizeof(Payloads::fullscreenNOP_patch)
            );
        }
        safe_write(
            FullscreenAddress,
            reinterpret_cast<const void*>(Payloads::screen_patch),
            sizeof(Payloads::screen_patch)
        );
        safe_write(
            FullscreenAddress + 0x1,
            static_cast<std::uint32_t>(m_Instance.m_conf.fullscreen)
        );
        if (m_conf.untie) {
            safe_write(
                UntieAddress,
                reinterpret_cast<const void*>(Payloads::untie_patch),
                sizeof(Payloads::untie_patch)
            );
        }
        if (m_conf.disable_clamp) {
            safe_write(
                iFPSClampAddress,
                reinterpret_cast<const void*>(Payloads::ifpsclamp_patch),
                sizeof(Payloads::ifpsclamp_patch)
            );
        }
        if (m_conf.disable_black_loading) {
            safe_write(
                BlackLoadingScreensAddress,
                reinterpret_cast<const void*>(Payloads::disable_blackloading_patch),
                sizeof(Payloads::disable_blackloading_patch)
            );
        }
        if (m_conf.disable_loading_screens) {
            safe_write(
                LoadingScreensAddress,
                reinterpret_cast<const void*>(Payloads::disable_loading_patch),
                sizeof(Payloads::disable_loading_patch)
            );
        }
        safe_write(
            BethesdaFPSCap1Address,
            static_cast<std::uint32_t>(10000)
        );
        safe_write(
            BethesdaFPSCap2Address,
            static_cast<std::uint32_t>(10000)
        );
        if (m_conf.postloading_speed != 1.0f) {
            {
                struct PostLoadPatch : JITASM::JITASM {
                    PostLoadPatch(std::uintptr_t a_hookTarget, float* a_value)
                        : JITASM(IF4SE::GetLocalTrampoline())
                    {
                        Xbyak::Label retnLabel;
                        Xbyak::Label timerLabel;
                        Xbyak::Label valueLabel;

                        movss(xmm0, dword[rip + valueLabel]);
                        mov(rcx, ptr[rip + timerLabel]);
                        mulss(xmm0, dword[rcx]);

                        jmp(ptr[rip + retnLabel]);

                        L(retnLabel);
                        dq(a_hookTarget + 0x8);

                        L(timerLabel);
                        dq(std::uintptr_t(Game::g_frameTimer));

                        L(valueLabel);
                        db(reinterpret_cast<Xbyak::uint8*>(a_value), sizeof(float));
                    }
                };
                _MESSAGE("[Postloading menu speed] patching...");
                {
                    PostLoadPatch code(PostLoadInjectAddress, &m_conf.postloading_speed);
                    IF4SE::GetBranchTrampoline().Write5Branch(PostLoadInjectAddress, code.get());
                }
                _MESSAGE("[Postloading menu speed] OK");
                safe_memset(PostLoadInjectAddress + 0x5, 0x90, 0x3);
            }
        }
        if (m_conf.limits.ui_loadscreen > 0.0f || m_conf.limits.ui_loadscreen > 0.0f || m_conf.limits.ui_lockpick > 0.0f || m_conf.limits.ui_pipboy > 0.0f)
        {
            limiter_installed = true;

            m_limiter = std::make_unique<FramerateLimiter>();

            if (m_conf.limit_mode == 0)
            {
                AddPresentCallbackPre(Throttle);
            }
            else
            {
                AddPresentCallbackPost(Throttle);
            }

            _MESSAGE("[Render] Framerate limiter installed, mode: %s", m_conf.limit_mode == 0 ? "pre" : "post");
        }

        if (!m_conf.fullscreen)
        {
            if (m_conf.disablebufferresize) {
                safe_write(
                    ResizeBuffersDisableAddress,
                    reinterpret_cast<const void*>(Payloads::ResizeBuffersDisable),
                    sizeof(Payloads::ResizeBuffersDisable)
                );
                _MESSAGE("[Render] Disabled swap chain buffer resizing");
            }

            if (m_conf.disabletargetresize) {
                safe_write(
                    ResizeTargetDisableAddress,
                    reinterpret_cast<const void*>(Payloads::ResizeTargetDisable),
                    sizeof(Payloads::ResizeTargetDisable)
                );

                _MESSAGE("[Render] Disabled swap chain target resizing");
            }
        }
        else {
            struct ResizeTargetInjectArgs : JITASM::JITASM {
                ResizeTargetInjectArgs(std::uintptr_t retnAddr, std::uintptr_t mdescAddr
                ) : JITASM(IF4SE::GetLocalTrampoline())
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

            _MESSAGE("[IDXGISwapChain::ResizeTarget] patching...");
            {
                ResizeTargetInjectArgs code(
                    ResizeTargetAddress + 0x8,
                    std::uintptr_t(&modeDesc));
                IF4SE::GetBranchTrampoline().Write6Branch(
                    ResizeTargetAddress, code.get());
                safe_memset(ResizeTargetAddress + 0x6, 0xCC, 0x2);
            }
            _MESSAGE("[IDXGISwapChain::ResizeTarget] OK");
        }

        {
            struct ResizeBuffersInjectArgs : JITASM::JITASM {
                ResizeBuffersInjectArgs(std::uintptr_t retnAddr, std::uintptr_t swdAddr
                ) : JITASM(IF4SE::GetLocalTrampoline())
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
            _MESSAGE("[IDXGISwapChain::ResizeBuffers] patching...");
            {
                ResizeBuffersInjectArgs code(
                    ResizeBuffersInjectAddress + 0x15,
                    std::uintptr_t(&m_swapchain));

                IF4SE::GetBranchTrampoline().Write6Branch(
                    ResizeBuffersInjectAddress, code.get());

                safe_memset(ResizeBuffersInjectAddress + 0x6, 0xCC, 0x12);
            }
        }
        {
            struct PatchRax : JITASM::JITASM {
                PatchRax(std::uintptr_t retnAddr
                ) : JITASM(IF4SE::GetLocalTrampoline())
                {
                    Xbyak::Label retnLabel;

                    mov(rax, ptr[rcx]);

                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(retnAddr);
                }
            };
            {
                PatchRax code(MovRaxRcxAddress + 0x6);
                IF4SE::GetBranchTrampoline().Write6Branch(
                    MovRaxRcxAddress, code.get());
            }
            _MESSAGE("[IDXGISwapChain::ResizeBuffers] OK");
        }
    }

    void DRender::RegisterHooks()
    {
        if (!Hook::Call5(
            IF4SE::GetBranchTrampoline(),
            CreateDXGIFactoryAddress,
            reinterpret_cast<std::uintptr_t>(CreateDXGIFactory_Hook),
            m_createDXGIFactory_O))
        {
            _MESSAGE("[Render] CreateDXGIFactory hook failed");
        }

        if (!Hook::Call5(
            IF4SE::GetBranchTrampoline(),
            D3D11CreateDeviceAndSwapChainAddress,
            reinterpret_cast<std::uintptr_t>(D3D11CreateDeviceAndSwapChain_Hook),
            m_D3D11CreateDeviceAndSwapChain_O))
        {
            _MESSAGE("[Render] D3D11CreateDeviceAndSwapChain hook failed");
            SetOK(false);
            return;
        }
    }
    void DRender::LimiterFunc() {
        while (IPerfCounter::delta_us(timing, IPerfCounter::Query()) < loading_fps)
        {
            ::Sleep(0);
        }
        timing = IPerfCounter::Query();
    }

    void DRender::PostInit()
    {
        if (m_conf.limits.ui_loadscreen > 0.0f && m_conf.fixcputhreads) {
            timing = IPerfCounter::Query();
            IF4SE::GetBranchTrampoline().Write5Call(LoadScreenPlusLimiterAddress, std::uintptr_t(LimiterFunc));
        }

        struct PresentHook : JITASM::JITASM {
            PresentHook(std::uintptr_t targetAddr
            ) : JITASM(IF4SE::GetLocalTrampoline())
            {
                Xbyak::Label callLabel;
                Xbyak::Label retnLabel;

                mov(edx, dword[rax + 0x40]);
                call(ptr[rip + callLabel]);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(targetAddr + 0x7);

                L(callLabel);
                dq(std::uintptr_t(Present_Hook));
            }
        };

        _MESSAGE("[IDXGISwapChain::Present] patching...");
        {
            PresentHook code(PresentAddress);
            IF4SE::GetBranchTrampoline().Write6Branch(PresentAddress, code.get());
        }
        _MESSAGE("[IDXGISwapChain::Present] OK");

        auto numPre = m_Instance.m_presentCallbacksPre.size();
        auto numPost = m_Instance.m_presentCallbacksPost.size();

        _MESSAGE("[Render] Installed present hook (pre:%zu post:%zu)", numPre, numPost);
    }

    bool DRender::Prepare()
    {  
        return true;
    }

    void DRender::SetFPSLimitOverride(long long max, bool disable_vsync)
    {
        if (m_conf.vsync_on) {
            if (disable_vsync) {
                m_current_vsync_present_interval = 0;
                if (tearing_enabled) {
                    m_present_flags |= DXGI_PRESENT_ALLOW_TEARING;
                }
            }
            else {
                if (tearing_enabled) {
                    m_present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
                }
            }
        }

        current_fps_max = max;
        has_fl_override = true;
    }

    void DRender::SetFPSLimitPost(long long a_max, long long a_expire)
    {
        m_Instance.oo_current_fps_max = a_max;
        m_Instance.oo_expire_time = a_expire;
    }

    void DRender::ResetFPSLimitOverride()
    {
        if (!has_fl_override) {
            return;
        }

        if (m_conf.vsync_on) {
            m_current_vsync_present_interval = m_vsync_present_interval;
            if (tearing_enabled) {
                m_present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
            }
        }

        current_fps_max = fps_max;
        has_fl_override = false;
    }

    void DRender::QueueFPSLimitOverride(long long max, bool disable_vsync)
    {
        m_afTasks.AddTask([=]() { SetFPSLimitOverride(max, disable_vsync); });
    }

    void DRender::QueueFPSLimitPost(long long a_max, long long a_expire)
    {
        m_afTasks.AddTask([=]() { SetFPSLimitPost(a_max, a_expire); });
    }

    long long DRender::GetCurrentFramerateLimit()
    {
        if (m_Instance.oo_expire_time > 0)
        {
            if (IPerfCounter::Query() < m_Instance.oo_expire_time) {
                return m_Instance.oo_current_fps_max;
            }
            else {
                m_Instance.oo_expire_time = 0;
            }
        }

        return m_Instance.current_fps_max;

    }

    void DRender::Throttle(IDXGISwapChain*)
    {
        m_Instance.m_afTasks.ProcessTasks();

        auto limit = GetCurrentFramerateLimit();
        if (limit > 0) {
            m_Instance.m_limiter->Wait(limit);
        }
    }

    bool DRender::ValidateDisplayMode(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        if (pSwapChainDesc->BufferDesc.RefreshRate.Numerator > 0 &&
            !pSwapChainDesc->BufferDesc.RefreshRate.Denominator) {
            return false;
        }

        return true;
    }

    UINT DRender::GetRefreshRate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        if (!pSwapChainDesc->BufferDesc.RefreshRate.Denominator) {
            return 0U;
        }
        return pSwapChainDesc->BufferDesc.RefreshRate.Numerator /
            pSwapChainDesc->BufferDesc.RefreshRate.Denominator;
    }

    float DRender::GetMaxFramerate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        float maxt = 0.0f;

        if (m_conf.vsync_on && pSwapChainDesc->Windowed == FALSE) {
            maxt = static_cast<float>(GetRefreshRate(pSwapChainDesc));
        }

        if (fps_limit == 1) {
            if (!(m_conf.vsync_on && pSwapChainDesc->Windowed == FALSE) ||
                m_conf.limits.game < maxt)
            {
                maxt = m_conf.limits.game;
            }
        }

        return maxt;
    }

    DXGI_SWAP_EFFECT DRender::AutoGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        if (pSwapChainDesc->Windowed == TRUE) {
            if (m_dxgi.caps & DXGI_CAP_FLIP_DISCARD) {
                return DXGI_SWAP_EFFECT_FLIP_DISCARD;
            }
            else if (m_dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL) {
                return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            }
        }
        return DXGI_SWAP_EFFECT_DISCARD;
    }

    DXGI_SWAP_EFFECT DRender::ManualGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    {
        auto se = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
        if (m_conf.swap_effect == 2) {
            se = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_SEQUENTIAL;
        }
        if (m_conf.swap_effect == 3) {
            se = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        }
        if (m_conf.swap_effect == 4) {
            se = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }
        if (pSwapChainDesc->Windowed == TRUE) {
            auto nse = se;

            if (nse == DXGI_SWAP_EFFECT_FLIP_DISCARD &&
                !(m_dxgi.caps & DXGI_CAP_FLIP_DISCARD))
            {
                nse = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            }

            if (nse == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL &&
                !(m_dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL))
            {
                nse = DXGI_SWAP_EFFECT_DISCARD;
            }

            if (nse != se) {
                _MESSAGE("[Render] %s not supported, using %s",
                    GetSwapEffectOption(se),
                    GetSwapEffectOption(nse)
                );
                se = nse;
            }
        }

        return se;
    }

    void DRender::ApplyD3DSettings(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    {
        if (m_conf.fullscreen) {
            pSwapChainDesc->Windowed = FALSE;
        }
        if (pSwapChainDesc->Windowed == TRUE) {
            DXGI_GetCapabilities();
        }

        if (m_conf.swap_effect == 0) {
            pSwapChainDesc->SwapEffect = AutoGetSwapEffect(pSwapChainDesc);
        }
        else {
            pSwapChainDesc->SwapEffect = ManualGetSwapEffect(pSwapChainDesc);
        }

        bool flip_model = IsFlipOn(pSwapChainDesc);

        if (pSwapChainDesc->Windowed == TRUE)
        {
            if (m_conf.enable_tearing && flip_model &&
                (!m_conf.vsync_on || limiter_installed))
            {
                if (m_dxgi.caps & DXGI_CAP_TEARING) {
                    pSwapChainDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

                    if (!m_conf.vsync_on) {
                        m_present_flags |= DXGI_PRESENT_ALLOW_TEARING;
                    }

                    tearing_enabled = true;
                }
                else {
                    _MESSAGE("[Render] DXGI_FEATURE_PRESENT_ALLOW_TEARING not supported");
                }
            }
        }
        else {
            pSwapChainDesc->BufferDesc.Scaling = m_conf.scaling_mode;
        }

        if (m_conf.buffer_count == 0) {
            if (flip_model) {
                pSwapChainDesc->BufferCount = 3;
            }
        }
        else if (m_conf.buffer_count > 0) {
            pSwapChainDesc->BufferCount = static_cast<UINT>(m_conf.buffer_count);
        }

        if (flip_model) {
            pSwapChainDesc->SampleDesc.Count = 1;
            pSwapChainDesc->SampleDesc.Quality = 0;

            if (pSwapChainDesc->BufferCount < 2) {
                pSwapChainDesc->BufferCount = 2;
                _MESSAGE("[Render] Buffer count below the minimum required for flip model, increasing to 2");
            }
        }

        if (m_conf.upscale) {
            m_swapchain.width = pSwapChainDesc->BufferDesc.Width;
            m_swapchain.height = pSwapChainDesc->BufferDesc.Height;
        }

        modeDesc = pSwapChainDesc->BufferDesc;

        if (pSwapChainDesc->Windowed == FALSE) {
            modeDesc.Format = DXGI_FORMAT_UNKNOWN;
        }

        m_swapchain.flags = pSwapChainDesc->Flags;
    }

    void DRender::OnD3D11PreCreate(IDXGIAdapter* pAdapter, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    {
        if (!m_Instance.ValidateDisplayMode(pSwapChainDesc)) {
            _MESSAGE("[Render] Invalid refresh rate: (%u/%u)",
                pSwapChainDesc->BufferDesc.RefreshRate.Numerator,
                pSwapChainDesc->BufferDesc.RefreshRate.Denominator);
        }

        m_Instance.ApplyD3DSettings(const_cast<DXGI_SWAP_CHAIN_DESC*>(pSwapChainDesc));

        _MESSAGE(
            "[Render] [D3D] Requesting mode: %ux%u@%u | VSync: %u | Windowed: %d",
            pSwapChainDesc->BufferDesc.Width, pSwapChainDesc->BufferDesc.Height,
            m_Instance.GetRefreshRate(pSwapChainDesc),
            m_Instance.m_vsync_present_interval, pSwapChainDesc->Windowed);

        _MESSAGE("[Render] [D3D] SwapEffect: %s | SwapBufferCount: %u | Tearing: %d | Flags: 0x%.8X",
            GetSwapEffectOption(pSwapChainDesc->SwapEffect), pSwapChainDesc->BufferCount,
            m_Instance.tearing_enabled, pSwapChainDesc->Flags);

        _MESSAGE("[Render] [D3D] Windowed hardware composition support: %s",
            m_Instance.HasWindowedHWCompositionSupport(pAdapter) ? "yes" : "no");

        if (pSwapChainDesc->Windowed == TRUE) {
            if (!m_Instance.IsFlipOn(pSwapChainDesc)) {
                if (!(m_Instance.m_dxgi.caps & (DXGI_CAP_FLIP_DISCARD | DXGI_CAP_FLIP_SEQUENTIAL))) {
                    _MESSAGE("[Render] Flip not supported on your system, switch to exclusive fullscreen for better peformance");
                }
                else {
                    _MESSAGE("[Render] Switch to exclusive fullscreen or set SwapEffect to flip_discard or flip_sequential for better peformance");
                }
            }
        }
        else {
            if (m_Instance.IsFlipOn(pSwapChainDesc)) {
                _MESSAGE("[Render] Using flip in exclusive fullscreen may cause issues");
            }
        }
    }

    HRESULT WINAPI DRender::D3D11CreateDeviceAndSwapChain_Hook(
        _In_opt_ IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
        _COM_Outptr_opt_ IDXGISwapChain** ppSwapChain,
        _COM_Outptr_opt_ ID3D11Device** ppDevice,
        _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
        _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
    {
        m_Instance.OnD3D11PreCreate(pAdapter, pSwapChainDesc);

        D3D11CreateEventPre evd_pre(pSwapChainDesc);
        IEvents::TriggerEvent(Event::OnD3D11PreCreate, reinterpret_cast<void*>(&evd_pre));

        HRESULT hr = m_Instance.m_D3D11CreateDeviceAndSwapChain_O(
            pAdapter, DriverType, Software, Flags,
            pFeatureLevels, FeatureLevels, SDKVersion,
            pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel,
            ppImmediateContext);

        if (hr == S_OK) {
            D3D11CreateEventPost evd_post(pSwapChainDesc, *ppDevice, *ppImmediateContext, *ppSwapChain, pAdapter);
            IEvents::TriggerEvent(Event::OnD3D11PostCreate, reinterpret_cast<void*>(&evd_post));
            IEvents::TriggerEvent(Event::OnD3D11PostPostCreate, reinterpret_cast<void*>(&evd_post));
        }
        else {
            _MESSAGE("[Render] D3D11CreateDeviceAndSwapChain failed: 0x%lX", hr);
        }
        return hr;
    }

    HRESULT WINAPI DRender::CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory)
    {
        HRESULT hr = m_Instance.m_createDXGIFactory_O(riid, ppFactory);
        if (SUCCEEDED(hr)) {
            m_Instance.m_dxgiFactory = static_cast<IDXGIFactory*>(*ppFactory);
        }
        else {
            _MESSAGE("[Render] CreateDXGIFactory failed (%lX)", hr);
            ::abort();
        }
        return hr;
    }


    HRESULT STDMETHODCALLTYPE DRender::Present_Hook(
        IDXGISwapChain4* pSwapChain,
        UINT SyncInterval,
        UINT PresentFlags)
    {
        for (const auto& f : m_Instance.m_presentCallbacksPre) {
            f(pSwapChain);
        }

        HRESULT hr = pSwapChain->Present(m_Instance.m_current_vsync_present_interval, m_Instance.m_present_flags);

        for (const auto& f : m_Instance.m_presentCallbacksPost) {
            f(pSwapChain);
        }

        return hr;
    };


    IDXGIFactory* DRender::DXGI_GetFactory() const
    {
        HMODULE hModule = ::LoadLibraryA("dxgi.dll");
        if (!hModule)
            return nullptr;

        auto func = reinterpret_cast<CreateDXGIFactory_T>(::GetProcAddress(hModule, "CreateDXGIFactory"));
        if (!func)
            return nullptr;

        IDXGIFactory* pFactory;
        if (!SUCCEEDED(func(IID_PPV_ARGS(&pFactory))))
            return nullptr;

        return pFactory;
    }

    void DRender::DXGI_GetCapabilities()
    {
        bool release;
        IDXGIFactory* factory;

        if (!m_dxgiFactory)
        {
            _MESSAGE("[Render] IDXGIFactory not set, attempting to create..");

            factory = DXGI_GetFactory();
            if (!factory) {
                _MESSAGE("[Render] Couldn't create IDXGIFactory, assuming DXGI_CAPS_ALL");
                m_dxgi.caps = DXGI_CAPS_ALL;
                return;
            }

            release = true;
        }
        else {
            factory = m_dxgiFactory;
            release = false;
        }

        m_dxgi.caps = 0;

        do
        {
            {
                ComPtr<IDXGIFactory5> tmp;
                if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                    m_dxgi.caps = (
                        DXGI_CAP_FLIP_SEQUENTIAL |
                        DXGI_CAP_FLIP_DISCARD);

                    BOOL allowTearing;
                    HRESULT hr = tmp->CheckFeatureSupport(
                        DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                        &allowTearing, sizeof(allowTearing));

                    if (SUCCEEDED(hr) && allowTearing) {
                        m_dxgi.caps |= DXGI_CAP_TEARING;
                    }

                    break;
                }
            }

            {
                ComPtr<IDXGIFactory4> tmp;
                if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                    m_dxgi.caps = (
                        DXGI_CAP_FLIP_SEQUENTIAL |
                        DXGI_CAP_FLIP_DISCARD);

                    break;
                }
            }

            {
                ComPtr<IDXGIFactory3> tmp;
                if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                    m_dxgi.caps = DXGI_CAP_FLIP_SEQUENTIAL;
                }
            }
        } while (0);

        if (release)
            factory->Release();
    }

    bool DRender::HasWindowedHWCompositionSupport(IDXGIAdapter* adapter) const
    {
        if (adapter == nullptr) {
            return false;
        }

        for (UINT i = 0;; ++i)
        {
            ComPtr<IDXGIOutput> output;
            if (FAILED(adapter->EnumOutputs(i, &output))) {
                break;
            }

            ComPtr<IDXGIOutput6> output6;
            if (SUCCEEDED(output.As(&output6)))
            {
                UINT flags;
                HRESULT hr = output6->CheckHardwareCompositionSupport(&flags);

                if (SUCCEEDED(hr) && (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED)) {
                    return true;
                }
            }
        }

        return false;
    }


    bool DRender::QueryVideoMemoryInfo(
        IDXGISwapChain* a_swapChain,
        DXGI_QUERY_VIDEO_MEMORY_INFO& a_out) const
    {
        try
        {
            ComPtr<IDXGIDevice> pDXGIDevice;
            DirectX::ThrowIfFailed(a_swapChain->GetDevice(IID_PPV_ARGS(pDXGIDevice.GetAddressOf())));

            ComPtr<IDXGIAdapter> pDXGIAdapter;
            DirectX::ThrowIfFailed(pDXGIDevice->GetAdapter(pDXGIAdapter.GetAddressOf()));

            ComPtr<IDXGIAdapter3> adapter;
            DirectX::ThrowIfFailed(pDXGIAdapter.As(&adapter));

            DirectX::ThrowIfFailed(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, std::addressof(a_out)));

            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

}