#pragma once

namespace SDT
{
    static constexpr std::uint32_t DXGI_CAP_FLIP_DISCARD = 0x00000001U;
    static constexpr std::uint32_t DXGI_CAP_FLIP_SEQUENTIAL = 0x00000002U;
    static constexpr std::uint32_t DXGI_CAP_TEARING = 0x00000004U;

    static constexpr std::uint32_t DXGI_CAPS_ALL = 0xFFFFFFFFU;
    struct MenuFramerateLimitDescriptor
    {
        MenuFramerateLimitDescriptor() : enabled(false) {};
        MenuFramerateLimitDescriptor(
            bool a_disable_vsync,
            long long a_limit)
            :
            enabled(true),
            limit(a_limit)
        {};

        bool enabled;
        long long limit;
    };

    class FramerateLimiter;

    class DRender :
        public IDriver,
        IConfig
    {
        typedef stl::iunordered_map<std::string, int, std::allocator<std::pair<const std::string, int>>> SEMap;
        static SEMap cfgSwapEffectMap;

        typedef stl::iunordered_map<std::string, DXGI_MODE_SCALING, std::allocator<std::pair<const std::string, DXGI_MODE_SCALING>>> SMMap;
        static SMMap cfgScalingModeMap;

        typedef HRESULT(WINAPI* CreateDXGIFactory_T)(REFIID riid, _COM_Outptr_ void** ppFactory);

        typedef void (*presentCallback_t)(IDXGISwapChain* pSwapChain);

    public:
        static inline bool ReadMemory(uintptr_t addr, void* data, size_t len) {
            UInt32 oldProtect;
            if (VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(data, (void*)addr, len);
                if (VirtualProtect((void*)addr, len, oldProtect, &oldProtect))
                    return true;
            }
            return false;
        }
        static inline DWORD_PTR ft4handle;
        static inline uintptr_t ExteriorInteriorAddress, FrametimeAddress, ScreenAddress, FullScreenAddress, ResizeBuffersDisableAddress, CreateDXGIFactoryAddress, FPSAddress, BlackLoadingScreensAddress, LoadingScreensAddress, PostLoadInjectAddress, DetectLoadAddress;
        static inline constexpr auto ID = DRIVER_ID::RENDER;

        static inline bool tearing_enabled = false;
        static inline UINT m_current_vsync_present_interval = 0;
        static inline UINT m_vsync_present_interval = 0;
        static inline UINT m_present_flags = 0;

        typedef void(*RTProcR) (void);
        typedef void(*PhysCalcR) (void*, std::int32_t);

        static inline long long current_fps_max, oo_current_fps_max, oo_expire_time, fps_max, loading_fps, lockpick_fps, pipboy_fps, ext_fps, int_fps = 0; //fps60 - TEST

        static inline bool once, lockpicking, intextlimits, pipboy;

        struct
        {
            bool untie;
            bool disable_clamp;
            static inline bool vsync_on;
            static inline bool disable_vsync;
            bool disable_black_loading;
            bool disable_loading_screens;
            float postloading_speed;
            static inline std::uint8_t fullscreen;
            static inline std::uint8_t borderless;
            static inline bool upscale;
            bool upscale_select_primary_monitor;
            bool disablebufferresize;
            bool disabletargetresize;
            std::uint32_t vsync_present_interval;
            int swap_effect;
            std::int32_t scale_mode;
            DXGI_MODE_SCALING scaling_mode;
            std::int32_t max_rr;
            std::int32_t buffer_count;
            std::int32_t max_frame_latency;
            bool enable_tearing;
            std::int32_t resolution[2];
            float resolution_scale;
            std::uint8_t limit_mode;

            struct {
                float game;
                float exterior;
                float interior;
                float ui;
                float ui_loadscreen;
                float ui_pipboy;
                float ui_map;
                float ui_inventory;
                float ui_journal;
                float ui_custom;
                float ui_main;
                float ui_race;
                float ui_perk;
                float ui_book;
                float ui_lockpick;
                float ui_console;
                float ui_tween;
                float ui_sw;

                bool ui_dv;
                bool ui_loadscreen_dv;
                bool ui_map_dv;
                bool ui_inventory_dv;
                bool ui_journal_dv;
                bool ui_custom_dv;
                bool ui_main_dv;
                bool ui_race_dv;
                bool ui_perk_dv;
                bool ui_book_dv;
                bool ui_lockpick_dv;
                bool ui_console_dv;
                bool ui_tween_dv;
                bool ui_sw_dv;

                std::int32_t ui_loadscreenex;
                std::int32_t ui_initialloadex;
            } limits;
            bool fixcputhreads;
        } static inline m_conf;

        //[[nodiscard]] float GetMaxFramerate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
        [[nodiscard]] bool IsLimiterInstalled() { return limiter_installed; }

        [[nodiscard]] bool QueryVideoMemoryInfo(
            IDXGISwapChain* a_swapChain,
            DXGI_QUERY_VIDEO_MEMORY_INFO& a_out) const;

        __forceinline void AddPresentCallbackPre(presentCallback_t f) {
            m_presentCallbacksPre.emplace_back(f);
        }

        __forceinline void AddPresentCallbackPost(presentCallback_t f) {
            m_presentCallbacksPost.emplace_back(f);
        }

        FN_NAMEPROC("Render");
        FN_ESSENTIAL(false);
        FN_DRVDEF(2);
    private:
        DRender();
        static void LimiterFunc();
        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;
        virtual void PostInit() override;

        std::uint8_t GetScreenModeSetting(IConfigGame& a_gameConfig, const char* a_section, const char* a_key, const char* a_prefkey, bool a_default);

        static const char* GetSwapEffectOption(DXGI_SWAP_EFFECT a_swapEffect);
        static bool ConfigParseResolution(const std::string& in, std::int32_t(&a_out)[2]);

        bool ValidateDisplayMode(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
        UINT GetRefreshRate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
        DXGI_SWAP_EFFECT AutoGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
        DXGI_SWAP_EFFECT ManualGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
        void ApplyD3DSettings(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);

        __forceinline static long long GetCurrentFramerateLimit();
        static void Throttle(IDXGISwapChain*);

        static void OnD3D11PreCreate(IDXGIAdapter* pAdapter, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
        static HRESULT WINAPI D3D11CreateDeviceAndSwapChain_Hook(_In_opt_ IDXGIAdapter* pAdapter,
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
            _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);

        static HRESULT WINAPI CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory);
        static HRESULT STDMETHODCALLTYPE Present_Hook(
            IDXGISwapChain4* pSwapChain,
            UINT SyncInterval,
            UINT PresentFlags);

        IDXGIFactory *DXGI_GetFactory() const;
        void DXGI_GetCapabilities();
        bool HasWindowedHWCompositionSupport(IDXGIAdapter* adapter) const;

        void SetFPSLimitOverride(long long max, bool disable_vsync);
        void SetFPSLimitPost(long long a_max, long long a_expire);
        void ResetFPSLimitOverride();

        void QueueFPSLimitOverride(long long max, bool disable_vsync);
        void QueueFPSLimitPost(long long a_max, long long a_expire);

        long long tts;
        static inline long long timing;
        int fps_limit;
        bool has_swap_effect;
        bool has_scaling_mode;
        float fmt_max;
        float fmt_min;
        long long lslExtraTime, lslPostLoadExtraTime;
        std::uint8_t gameLoadState;
        bool has_fl_override;
        bool limiter_installed;

        __forceinline bool IsFlipOn(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) {
            return pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL ||
                pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }

        struct
        {
            std::int32_t* iFPSClamp;
            std::int32_t* iSizeW;
            std::int32_t* iSizeH;
        } m_gv;

        IDXGIFactory* m_dxgiFactory;

        PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN m_D3D11CreateDeviceAndSwapChain_O;
        CreateDXGIFactory_T m_createDXGIFactory_O;

        std::vector<presentCallback_t> m_presentCallbacksPre;
        std::vector<presentCallback_t> m_presentCallbacksPost;

        struct {
            std::uint32_t width;
            std::uint32_t height;
            std::uint32_t flags;
        }m_swapchain;

        DXGI_MODE_DESC modeDesc;

        struct
        {
            std::uint32_t caps;
        } m_dxgi;

        TaskQueue m_afTasks;
        std::unique_ptr<FramerateLimiter> m_limiter;

        static DRender m_Instance;
    };

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

    class D3D11CreateEventPost :
        public D3D11CreateEvent
    {
    public:
        D3D11CreateEventPost(
            CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
            ID3D11Device* pDevice,
            ID3D11DeviceContext* pImmediateContext,
            IDXGISwapChain* pSwapChain,
            IDXGIAdapter* pAdapter
        ) :
            D3D11CreateEvent(pSwapChainDesc),
            m_pDevice(pDevice),
            m_pImmediateContext(pImmediateContext),
            m_pSwapChain(pSwapChain),
            m_pAdapter(pAdapter)
        {}

        ID3D11Device* const m_pDevice;
        ID3D11DeviceContext* const m_pImmediateContext;
        IDXGISwapChain* const m_pSwapChain;
        IDXGIAdapter* const m_pAdapter;
    };

}