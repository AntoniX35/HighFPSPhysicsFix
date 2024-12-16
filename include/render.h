#pragma once

namespace HFPF
{
	using EventResult = RE::BSEventNotifyControl;
	static constexpr std::uint32_t DXGI_CAP_FLIP_DISCARD = 0x00000001U;
	static constexpr std::uint32_t DXGI_CAP_FLIP_SEQUENTIAL = 0x00000002U;
	static constexpr std::uint32_t DXGI_CAP_TEARING = 0x00000004U;

	static constexpr std::uint32_t DXGI_CAPS_ALL = 0xFFFFFFFFU;

	class FramerateLimiter;

	class DRender :
		public IDriver,
		public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
		IConfig
	{
		typedef stl::iunordered_map<std::string, int> SEMap;
		static SEMap                                  cfgSwapEffectMap;

		typedef stl::iunordered_map<std::string, DXGI_MODE_SCALING> SMMap;
		static SMMap                                                cfgScalingModeMap;

		typedef HRESULT(WINAPI* CreateDXGIFactory_T)(REFIID riid, _COM_Outptr_ void** ppFactory);

		typedef void (*presentCallback_t)(IDXGISwapChain* pSwapChain);

	public:
		static inline constexpr auto ID = DRIVER_ID::RENDER;
		static void                  Register();

		struct
		{
			bool disable_clamp;
			bool disable_vsync_loading;

			std::uint8_t      fullscreen;
			std::uint8_t      borderless;
			bool              upscale;
			bool              upscale_select_primary_monitor;
			bool              disablebufferresize;
			bool              disabletargetresize;
			bool              vsync_on;
			std::uint32_t     vsync_present_interval;
			int               swap_effect;
			std::int32_t      scale_mode;
			DXGI_MODE_SCALING scaling_mode;
			std::int32_t      max_rr;
			std::int32_t      buffer_count;
			bool              enable_tearing;
			std::int32_t      resolution[2];
			float             resolution_scale;
			std::uint8_t      limit_mode;

			bool   loading_time;
			bool   bare_loading_time;
			double delay;

			struct
			{
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
			} limits;
		} m_conf;

		struct
		{
			long long ext_fps;
			long long int_fps;
			long long loading_fps;
			long long lockpick_fps;
			long long pipboy_fps;
			bool      comboKeyDown;
		} m_limits;

		[[nodiscard]] float GetMaxFramerate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
		[[nodiscard]] bool  IsLimiterInstalled() { return limiter_installed; }

		SKMP_FORCEINLINE void AddPresentCallbackPre(presentCallback_t f)
		{
			m_presentCallbacksPre.emplace_back(f);
		}

		SKMP_FORCEINLINE void AddPresentCallbackPost(presentCallback_t f)
		{
			m_presentCallbacksPost.emplace_back(f);
		}

		FN_NAMEPROC("Render");
		FN_ESSENTIAL(false);
		FN_DRVDEF(2);

	private:
		DRender();

		void SetLimit(float limit, bool disable_vsync);
		bool HasLimits() const;

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void Patch() override;
		virtual void RegisterHooks() override;
		virtual bool Prepare() override;
		virtual void PostInit() override;
		virtual void OnGameConfigLoaded() override;

		std::uint8_t GetScreenModeSetting(IConfigGame& a_gameConfig, const char* a_section, const char* a_key, const char* a_prefkey, bool a_default);

		static DXGI_SWAP_EFFECT GetSwapEffect(int a_code);
		static const char*      GetSwapEffectOption(DXGI_SWAP_EFFECT a_swapEffect);

		void UISetLimitLoading(float limit, bool disable_vsync);
		void UISetLimitLockpicking(float limit);
		void UISetLimitPipboy(float limit);

		static const wchar_t* StatsRendererCallback_LoadTime();
		static void           OnD3D11PostCreate_LoadTime(Event code, void* data);

		bool loading;
		bool last;

		struct SKMP_ALIGN(16)
		{
			bool    loading_screen = true;
			clock_t start, end, timeout;

		} m_stats;

		bool        ConfigTranslateSwapEffect(const std::string& param, int& out) const;
		bool        ConfigTranslateScalingMode(const std::string& param, DXGI_MODE_SCALING& out) const;
		static bool ConfigParseResolution(const std::string& in, std::int32_t (&a_out)[2]);

		bool             ValidateDisplayMode(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
		UINT             GetRefreshRate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
		DXGI_SWAP_EFFECT AutoGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
		DXGI_SWAP_EFFECT ManualGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
		void             ApplyD3DSettings(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);

		SKMP_FORCEINLINE static long long GetCurrentFramerateLimit();
		static void                       Throttle(IDXGISwapChain*);

		void OnD3D11PreCreate(IDXGIAdapter* pAdapter, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);

		static HRESULT WINAPI D3D11CreateDeviceAndSwapChain_Hook(_In_opt_ IDXGIAdapter* pAdapter,
			D3D_DRIVER_TYPE                                                             DriverType,
			HMODULE                                                                     Software,
			UINT                                                                        Flags,
			_In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL*                      pFeatureLevels,
			UINT                                                                        FeatureLevels,
			UINT                                                                        SDKVersion,
			_In_opt_ CONST DXGI_SWAP_CHAIN_DESC*                                        pSwapChainDesc,
			_COM_Outptr_opt_ IDXGISwapChain**                                           ppSwapChain,
			_COM_Outptr_opt_ ID3D11Device**                                             ppDevice,
			_Out_opt_ D3D_FEATURE_LEVEL*                                                pFeatureLevel,
			_COM_Outptr_opt_ ID3D11DeviceContext**                                      ppImmediateContext);

		static HRESULT WINAPI            CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory);
		static HRESULT STDMETHODCALLTYPE Present_Hook(
			IDXGISwapChain4* pSwapChain,
			UINT             SyncInterval,
			UINT             PresentFlags);

		IDXGIFactory* DXGI_GetFactory() const;
		void          DXGI_GetCapabilities();
		bool          HasWindowedHWCompositionSupport(IDXGIAdapter* adapter) const;

		static void OnConfigLoad(Event m_code, void* args);

		EventResult ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		bool intextlimits;

		bool m_hasLimits;

		void SetFPSLimitOverride(long long max, bool disable_vsync);
		void SetFPSLimitPost(long long a_max, long long a_expire);
		void ResetFPSLimitOverride();
		void IntExt();

		void QueueFPSLimitOverride(long long max, bool disable_vsync);
		void QueueFPSLimitPost(long long a_max, long long a_expire);
		void QueueFPSLimitOverrideReset();
		void QueueIntExt();

		bool      OSD_Load_Time;
		long long tts;
		int       fps_limit;
		bool      has_scaling_mode;
		float     fmt_max;
		float     fmt_min;
		long long fps_max;
		bool      tearing_enabled;
		long long current_fps_max, oo_current_fps_max, oo_expire_time;
		bool      has_fl_override;
		bool      limiter_installed;

		struct m_fl
		{
			m_fl() :
				enabled(false){};
			m_fl(
				bool      a_disable_vsync,
				long long a_limit) :
				enabled(true),
				disable_vsync(a_disable_vsync),
				limit(a_limit){};

			bool      enabled;
			bool      disable_vsync;
			long long limit;
		};

		SKMP_FORCEINLINE bool IsFlipOn(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
		{
			return pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL ||
			       pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD;
		}

		UINT m_vsync_present_interval;
		UINT m_current_vsync_present_interval;
		UINT m_present_flags;

		std::int64_t m_originalResW{ 0 };
		std::int64_t m_originalResH{ 0 };

		struct
		{
			std::int32_t* iFPSClamp{ nullptr };
		} m_gv;

		IDXGIFactory* m_dxgiFactory;

		PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN m_D3D11CreateDeviceAndSwapChain_O;
		CreateDXGIFactory_T                    m_createDXGIFactory_O;

		inline static REL::Relocation<std::uintptr_t> FPS_Cap_Patch1{ AID::FPS_Cap_Patch1, (REL::Module::get().version() >= F4SE::RUNTIME_LATEST) ? Offsets::FPS_Cap_Patch1 : 0xBC7 };
		inline static REL::Relocation<std::uintptr_t> FPS_Cap_Patch2{ AID::FPS_Cap_Patch1, (REL::Module::get().version() >= F4SE::RUNTIME_LATEST) ? Offsets::FPS_Cap_Patch2 : 0xBD0 };
		inline static REL::Relocation<std::uintptr_t> Borderless_Patch{ AID::FPS_Cap_Patch1, (REL::Module::get().version() >= F4SE::RUNTIME_LATEST) ? Offsets::Borderless_Patch : 0xB87 };
		inline static REL::Relocation<std::uintptr_t> FullScreen_Patch1{ AID::FullScreen_Patch1, Offsets::FullScreen_Patch1 };
		inline static REL::Relocation<std::uintptr_t> FullScreen_Patch3{ AID::FullScreen_Patch1, Offsets::FullScreen_Patch3 };
		inline static REL::Relocation<std::uintptr_t> Screen_Patch{ AID::FPS_Cap_Patch1, (REL::Module::get().version() >= F4SE::RUNTIME_LATEST) ? Offsets::Screen_Patch : 0xB4A };
		inline static REL::Relocation<std::uintptr_t> ResizeBuffersDisable{ AID::ResizeBuffers, Offsets::ResizeBuffersDisable };
		inline static REL::Relocation<std::uintptr_t> MovRaxRcx{ AID::ResizeBuffers, Offsets::MovRaxRcx };
		inline static REL::Relocation<std::uintptr_t> ResizeBuffersInject{ AID::ResizeBuffers, Offsets::ResizeBuffers };
		inline static REL::Relocation<std::uintptr_t> ResizeTargetDisable{ AID::ResizeTarget, Offsets::ResizeTargetDisable };
		inline static REL::Relocation<std::uintptr_t> ResizeTarget{ AID::ResizeTarget, Offsets::ResizeTarget };
		inline static REL::Relocation<std::uintptr_t> CreateDXGIFactory{ AID::D3D11Create, Offsets::CreateDXGIFactory };
		inline static REL::Relocation<std::uintptr_t> D3D11CreateDeviceAndSwapChain{ AID::D3D11Create, Offsets::D3D11CreateDeviceAndSwapChain };
		inline static REL::Relocation<std::uintptr_t> BethesdaVsync{ AID::D3D11Create, Offsets::BethesdaVsync };
		inline static REL::Relocation<std::uintptr_t> Present{ AID::LoadScreenPlusLimiterInject, Offsets::PresentInject };
		inline static REL::Relocation<std::uintptr_t> iSizeW_Patch{ AID::FPS_Cap_Patch1, (REL::Module::get().version() >= F4SE::RUNTIME_LATEST) ? Offsets::iSizeW : 0xB93 };
		inline static REL::Relocation<std::uintptr_t> iSizeH_Patch{ AID::FPS_Cap_Patch1, (REL::Module::get().version() >= F4SE::RUNTIME_LATEST) ? Offsets::iSizeH : 0xBA1 };
		inline static REL::Relocation<int*>           g_extInt{ AID::ExtInt, 0x20 };

		std::vector<presentCallback_t> m_presentCallbacksPre;
		std::vector<presentCallback_t> m_presentCallbacksPost;

		struct
		{
			std::uint32_t width;
			std::uint32_t height;
			std::uint32_t flags;
		} m_swapchain;

		DXGI_MODE_DESC modeDesc;

		struct
		{
			std::uint32_t caps;
		} m_dxgi;

		std::unique_ptr<FramerateLimiter> m_limiter;

		DOSD*   m_OSDDriver{ nullptr };
		wchar_t bufStats[64];

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
			ID3D11Device*               pDevice,
			ID3D11DeviceContext*        pImmediateContext,
			IDXGISwapChain*             pSwapChain,
			IDXGIAdapter*               pAdapter) :
			D3D11CreateEvent(pSwapChainDesc),
			m_pDevice(pDevice),
			m_pImmediateContext(pImmediateContext),
			m_pSwapChain(pSwapChain),
			m_pAdapter(pAdapter)
		{}

		ID3D11Device* const        m_pDevice;
		ID3D11DeviceContext* const m_pImmediateContext;
		IDXGISwapChain* const      m_pSwapChain;
		IDXGIAdapter* const        m_pAdapter;
	};
}
