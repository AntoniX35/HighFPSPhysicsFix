#include "PCH.h"
#include "Render/FramerateLimiter.h"
#include "ext/Utility.h"

using namespace Microsoft::WRL;

namespace HFPF
{
	static constexpr const char* SECTION_MAIN = "Main";
	static constexpr const char* CKEY_DISABLEIFPSCLAMP = "DisableiFPSClamp";
	static constexpr const char* CKEY_VSYNC = "EnableVSync";
	static constexpr const char* CKEY_VSYNCPRESENTINT = "VSyncPresentInterval";
	static constexpr const char* CKEY_DISABLEVSYNC = "DisableVSyncWhileLoading";

	static constexpr const char* SECTION_DISPLAY = "Display";
	static constexpr const char* CKEY_FULLSCREEN = "Fullscreen";
	static constexpr const char* CKEY_BORDERLESS = "Borderless";
	static constexpr const char* CKEY_BORDERLESSUPSCALE = "BorderlessUpscale";
	static constexpr const char* CKEY_UPSCALE_PRIMARY_MON = "BorderlessUpscaleRelativeToPrimaryMonitor";
	static constexpr const char* CKEY_BUFFERSDISABLE = "ResizeBuffersDisable";
	static constexpr const char* CKEY_TARGETDISABLE = "ResizeTargetDisable";
	static constexpr const char* CKEY_BUFFERCOUNT = "BufferCount";
	static constexpr const char* CKEY_SWAPEFFECT = "SwapEffect";
	static constexpr const char* CKEY_SCALINGMODE = "ScalingMode";
	static constexpr const char* CKEY_TEARING = "AllowTearing";

	static constexpr const char* SECTION_LIMIT = "Limiter";
	static constexpr const char* CKEY_INGAMEFPS = "InGameFPS";
	static constexpr const char* CKEY_EXTERIORFPS = "ExteriorFPS";
	static constexpr const char* CKEY_INTERIORFPS = "InteriorFPS";
	static constexpr const char* CKEY_UILOADINGFPS = "LoadingScreenFPS";
	static constexpr const char* CKEY_UILOCKFPS = "LockpickingFPS";
	static constexpr const char* CKEY_UIPIPFPS = "PipBoyFPS";
	static constexpr const char* CKEY_FPSLIMIT_MODE = "FramerateLimitMode";

	static constexpr const char* CKEY_RESOLUTON = "Resolution";
	static constexpr const char* CKEY_RESSCALE = "ResolutionScale";

	static constexpr const char* SECTION_OSD = "OSD";

	static constexpr const char* CKEY_LOADINGTIME = "LoadingTime";
	static constexpr const char* CKEY_BARELOADTIME = "Bare_LoadingTime";
	static constexpr const char* CKEY_LOADTIMEDELAY = "LoadingTimeDelay";

	DRender DRender::m_Instance;

	const char* DRender::GetSwapEffectOption(DXGI_SWAP_EFFECT a_swapEffect)
	{
		switch (a_swapEffect) {
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

	bool DRender::HasLimits() const
	{
		return m_hasLimits;
	}

	DRender::DRender() :
		fps_max(0),
		current_fps_max(0),
		limiter_installed(false),
		tearing_enabled(false),
		has_fl_override(false),
		fps_limit(-1),
		m_present_flags(0),
		oo_expire_time(0),
		oo_current_fps_max(0),
		m_dxgiFactory(nullptr),
		m_swapchain{ 0, 0, 0 },
		m_vsync_present_interval(0),
		m_current_vsync_present_interval(0)
	{
		m_swapchain.flags = 0;
		m_swapchain.width = 0;
		m_swapchain.height = 0;

		bufStats[0] = 0x0;
	}

	std::uint8_t DRender::GetScreenModeSetting(
		IConfigGame& a_gameConfig,
		const char*  a_section,
		const char*  a_key,
		const char*  a_prefkey,
		bool         a_default)
	{
		if (!HasConfigValue(a_section, a_key)) {
			bool result;
			if (a_gameConfig.Get(SECTION_DISPLAY, a_prefkey, a_default, result)) {
				logger::info("[Render] {}: Using game setting ({}): {}", a_key, a_prefkey, result);

				return static_cast<std::uint8_t>(result);
			}

			return static_cast<std::uint8_t>(a_default);
		}

		return static_cast<std::uint8_t>(GetConfigValue(a_section, a_key, a_default));
	}

	void DRender::LoadConfig()
	{
		IConfigGame gameConfig(FALLOUT4_PREFS_INI_FILE);

		m_conf.disable_clamp = GetConfigValue(SECTION_MAIN, CKEY_DISABLEIFPSCLAMP, true);
		m_conf.disable_vsync_loading = GetConfigValue(SECTION_MAIN, CKEY_DISABLEVSYNC, true);

		gameConfig.Get(SECTION_DISPLAY, "iSize W", 0i64, m_originalResW);
		gameConfig.Get(SECTION_DISPLAY, "iSize H", 0i64, m_originalResH);

		m_conf.fullscreen = GetScreenModeSetting(gameConfig, SECTION_DISPLAY, CKEY_FULLSCREEN, "bFull Screen", false);
		m_conf.borderless = GetScreenModeSetting(gameConfig, SECTION_DISPLAY, CKEY_BORDERLESS, "bBorderless", true);
		m_conf.upscale = GetConfigValue(SECTION_DISPLAY, CKEY_BORDERLESSUPSCALE, false);
		m_conf.upscale_select_primary_monitor = GetConfigValue(SECTION_DISPLAY, CKEY_UPSCALE_PRIMARY_MON, true);
		m_conf.disablebufferresize = GetConfigValue(SECTION_DISPLAY, CKEY_BUFFERSDISABLE, false);
		m_conf.disabletargetresize = GetConfigValue(SECTION_DISPLAY, CKEY_TARGETDISABLE, false);

		m_conf.vsync_on = GetConfigValue(SECTION_MAIN, CKEY_VSYNC, true);
		m_conf.vsync_present_interval = std::clamp<std::uint32_t>(GetConfigValue<std::uint32_t>(SECTION_DISPLAY, CKEY_VSYNCPRESENTINT, 1), 1, 4);
		m_conf.swap_effect = GetConfigValue(SECTION_DISPLAY, CKEY_SWAPEFFECT, 0);
		m_conf.scale_mode = GetConfigValue(SECTION_DISPLAY, CKEY_SCALINGMODE, 1);
		if (m_conf.scale_mode == 1) {
			m_conf.scaling_mode = DXGI_MODE_SCALING_UNSPECIFIED;
		}
		if (m_conf.scale_mode == 2) {
			m_conf.scaling_mode = DXGI_MODE_SCALING_CENTERED;
		}
		if (m_conf.scale_mode == 3) {
			m_conf.scaling_mode = DXGI_MODE_SCALING_STRETCHED;
		}
		m_conf.buffer_count = GetConfigValue(SECTION_DISPLAY, CKEY_BUFFERCOUNT, 0);
		if (m_conf.buffer_count > -1) {
			m_conf.buffer_count = std::clamp<std::int32_t>(m_conf.buffer_count, 0, 8);
		}
		m_conf.enable_tearing = GetConfigValue(SECTION_MAIN, CKEY_TEARING, true);

		m_conf.limit_mode = std::clamp<std::uint8_t>(GetConfigValue<std::uint8_t>(SECTION_LIMIT, CKEY_FPSLIMIT_MODE, 0), 0, 1);
		m_conf.limits.game = GetConfigValue(SECTION_LIMIT, CKEY_INGAMEFPS, -1.0f);
		m_conf.limits.exterior = GetConfigValue(SECTION_LIMIT, CKEY_EXTERIORFPS, -1.0f);
		m_conf.limits.interior = GetConfigValue(SECTION_LIMIT, CKEY_INTERIORFPS, -1.0f);
		m_conf.limits.ui_loadscreen = GetConfigValue(SECTION_LIMIT, CKEY_UILOADINGFPS, 350.0f);
		m_conf.limits.ui_lockpick = GetConfigValue(SECTION_LIMIT, CKEY_UILOCKFPS, 60.0f);
		m_conf.limits.ui_pipboy = GetConfigValue(SECTION_LIMIT, CKEY_UIPIPFPS, 60.0f);

		if (!ConfigParseResolution(GetConfigValue(SECTION_DISPLAY, CKEY_RESOLUTON, "-1 -1"), m_conf.resolution)) {
			m_conf.resolution[0] = -1;
			m_conf.resolution[1] = -1;
		}
		m_conf.resolution_scale = GetConfigValue(SECTION_DISPLAY, CKEY_RESSCALE, -1.0f);
		m_conf.loading_time = GetConfigValue(SECTION_OSD, CKEY_LOADINGTIME, false);
		m_conf.bare_loading_time = GetConfigValue(SECTION_OSD, CKEY_BARELOADTIME, false);
		m_conf.delay = GetConfigValue(SECTION_OSD, CKEY_LOADTIMEDELAY, 5.0f);
	}

	bool DRender::ConfigParseResolution(const std::string& in, std::int32_t (&a_out)[2])
	{
		std::vector<std::int32_t> v2;
		StrHelpers::SplitString(in, 'x', v2);

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
			if (!(!m_conf.fullscreen && m_conf.borderless)) {
				m_conf.upscale = false;
			} else {
				logger::info("[Render] Borderless upscaling enabled");
			}
		}

		m_current_vsync_present_interval = m_vsync_present_interval = m_conf.vsync_on ? m_conf.vsync_present_interval : 0;

		if (m_conf.limits.game > 0.0f) {
			fps_limit = 1;
			m_hasLimits = true;
			current_fps_max = fps_max = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.game)) * 1000000.0L);
			logger::info("[Render] Framerate limit (game): {}", m_conf.limits.game);
		} else if (m_conf.limits.game == 0.0f) {
			fps_limit = 0;
		} else {
			fps_limit = -1;
		}

		if (m_conf.limits.exterior > 0.0f) {
			fps_limit = 1;
			intextlimits = true;
			m_hasLimits = true;
			m_limits.ext_fps = current_fps_max = fps_max = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.exterior)) * 1000000.0L);
			logger::info("[Render] Framerate limit (exterior): {}", m_conf.limits.exterior);
		}
		if (m_conf.limits.interior > 0.0f) {
			fps_limit = 1;
			intextlimits = true;
			m_hasLimits = true;
			m_limits.int_fps = current_fps_max = fps_max = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.interior)) * 1000000.0L);
			logger::info("[Render] Framerate limit (interior): {}", m_conf.limits.interior);
		}

		if (m_conf.limits.ui_loadscreen > 0.0f) {
			m_limits.loading_fps = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.ui_loadscreen)) * 1000000.0L);
			logger::info("[Render] Framerate limit (loading): {}", m_conf.limits.ui_loadscreen);
		}
		if (m_conf.limits.ui_lockpick > 0.0f) {
			m_limits.lockpick_fps = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.ui_lockpick)) * 1000000.0L);
			logger::info("[Render] Framerate limit (lockpicking): {}", m_conf.limits.ui_lockpick);
		}
		if (m_conf.limits.ui_pipboy > 0.0f) {
			m_limits.pipboy_fps = static_cast<long long>((1.0L / static_cast<long double>(m_conf.limits.ui_pipboy)) * 1000000.0L);
			logger::info("[Render] Framerate limit (pipboy): {}", m_conf.limits.ui_pipboy);
		}
		if (m_conf.limits.game > 0.0f || m_conf.limits.ui_loadscreen > 0.0f || m_conf.limits.ui_lockpick > 0.0f || m_conf.limits.ui_pipboy > 0.0f) {
			m_hasLimits = true;
			fps_limit = 1;
		}
	}

	void DRender::Patch()
	{
		REL::safe_write(BethesdaVsync.address(), &Payloads::NOP4, 4);
		std::uint32_t maximumfps = 10000;

		REL::safe_write(
			FPS_Cap_Patch1.address(),
			&maximumfps,
			sizeof(maximumfps));
		REL::safe_write(
			FPS_Cap_Patch2.address(),
			&maximumfps,
			sizeof(maximumfps));
		REL::safe_write(
			Borderless_Patch.address(),
			reinterpret_cast<const void*>(Payloads::screen_patch),
			sizeof(Payloads::screen_patch));
		REL::safe_write(
			Borderless_Patch.address() + 0x1,
			static_cast<std::uint32_t>(m_Instance.m_conf.borderless));
		REL::safe_write(
			FullScreen_Patch1.address(),
			reinterpret_cast<const void*>(Payloads::fullscreen1_patch),
			sizeof(Payloads::fullscreen1_patch));
		if (m_conf.fullscreen) {
			REL::safe_write(
				FullScreen_Patch3.address(),
				reinterpret_cast<const void*>(Payloads::fullscreenJMP_patch),
				sizeof(Payloads::fullscreenJMP_patch));
		} else {
			REL::safe_write(
				FullScreen_Patch3.address(),
				reinterpret_cast<const void*>(Payloads::fullscreenNOP_patch),
				sizeof(Payloads::fullscreenNOP_patch));
		}
		REL::safe_write(
			Screen_Patch.address(),
			reinterpret_cast<const void*>(Payloads::screen_patch),
			sizeof(Payloads::screen_patch));
		REL::safe_write(
			Screen_Patch.address() + 0x1,
			static_cast<std::uint32_t>(m_Instance.m_conf.fullscreen));
		if (HasLimits()) {
			limiter_installed = true;

			m_limiter = std::make_unique<FramerateLimiter>();

			if (m_conf.limit_mode == 0) {
				AddPresentCallbackPre(Throttle);
			} else {
				AddPresentCallbackPost(Throttle);
			}

			logger::info("[Render] Framerate limiter installed, mode: {}", m_conf.limit_mode == 0 ? "pre" : "post");
		}
		if (!m_conf.fullscreen) {
			if (m_conf.disablebufferresize) {
				REL::safe_write(
					ResizeBuffersDisable.address(),
					reinterpret_cast<const void*>(Payloads::ResizeBuffersDisable),
					sizeof(Payloads::ResizeBuffersDisable));
				logger::info("[Render] Disabled swap chain buffer resizing");
			}

			if (m_conf.disabletargetresize) {
				REL::safe_write(
					ResizeTargetDisable.address(),
					reinterpret_cast<const void*>(Payloads::ResizeTargetDisable),
					sizeof(Payloads::ResizeTargetDisable));

				logger::info("[Render] [Patch] Disabled swap chain target resizing");
			}

			bool         doPatch = false;
			std::int32_t w, h;

			if (m_conf.resolution[0] > 0 && m_conf.resolution[1] > 0) {
				w = m_conf.resolution[0];
				h = m_conf.resolution[1];

				doPatch = true;
			} else {
				w = static_cast<std::int32_t>(m_originalResW);
				h = static_cast<std::int32_t>(m_originalResH);
			}

			if (m_conf.resolution_scale > 0.0f) {
				w = static_cast<std::int32_t>(static_cast<float>(w) * m_conf.resolution_scale);
				h = static_cast<std::int32_t>(static_cast<float>(h) * m_conf.resolution_scale);

				doPatch = true;
			}
			if (doPatch) {
				w = std::max<std::int32_t>(w, 32);
				h = std::max<std::int32_t>(h, 32);

				REL::safe_write(
					iSizeW_Patch.address(),
					reinterpret_cast<const void*>(Payloads::res_patch),
					sizeof(Payloads::res_patch));
				REL::safe_write(iSizeW_Patch.address() + 0x1, w);

				REL::safe_write(
					iSizeH_Patch.address(),
					reinterpret_cast<const void*>(Payloads::res_patch),
					sizeof(Payloads::res_patch));
				REL::safe_write(iSizeH_Patch.address() + 0x1, h);

				logger::info("[Render] [Patch] Resolution override: ({}x{})", w, h);
			}
		} else {
			struct ResizeTargetInjectArgs : Xbyak::CodeGenerator
			{
				ResizeTargetInjectArgs(std::uintptr_t retnAddr, std::uintptr_t mdescAddr)
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
			logger::info("[Render] [Patch] [IDXGISwapChain::ResizeTarget] patching...");
			{
				ResizeTargetInjectArgs code(
					ResizeTarget.address() + 0x8,
					std::uintptr_t(&modeDesc));
				code.ready();

				auto& trampoline = F4SE::GetTrampoline();
				trampoline.write_branch<6>(
					ResizeTarget.address(),
					trampoline.allocate(code));
			}
			REL::safe_fill(ResizeTarget.address() + 0x6, Payloads::INT3, 0x2);
			logger::info("[Render] [Patch] [IDXGISwapChain::ResizeTarget] OK");
		}
		{
			struct ResizeBuffersInjectArgs : Xbyak::CodeGenerator
			{
				ResizeBuffersInjectArgs(std::uintptr_t retnAddr, std::uintptr_t swdAddr)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label bdLabel;

					mov(rdx, ptr[rip + bdLabel]);
					mov(r8d, ptr[rdx]);
					mov(r9d, ptr[rdx + 4]);
					mov(eax, ptr[rdx + 8]);
					mov(ptr[rsp + 0x28], eax);
					mov(dword[rsp + 0x20], DXGI_FORMAT_UNKNOWN);
					xor_(edx, edx);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(retnAddr);

					L(bdLabel);
					dq(swdAddr);
				}
			};
			logger::info("[Render] [Patch] [IDXGISwapChain::ResizeBuffers] patching...");
			{
				ResizeBuffersInjectArgs code(
					ResizeBuffersInject.address() + 0x15,
					std::uintptr_t(&m_swapchain));
				code.ready();

				auto& trampoline = F4SE::GetTrampoline();
				trampoline.write_branch<6>(
					ResizeBuffersInject.address(),
					trampoline.allocate(code));
			}
			REL::safe_fill(ResizeBuffersInject.address() + 0x6, Payloads::INT3, 0x12);
		}
		{
			struct PatchRax : Xbyak::CodeGenerator
			{
				PatchRax(std::uintptr_t retnAddr)
				{
					Xbyak::Label retnLabel;

					mov(rax, ptr[rcx]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(retnAddr);
				}
			};
			{
				PatchRax code(MovRaxRcx.address() + 0x6);
				code.ready();

				auto& trampoline = F4SE::GetTrampoline();
				trampoline.write_branch<6>(
					MovRaxRcx.address(),
					trampoline.allocate(code));
			}
			logger::info("[Render] [Patch] [IDXGISwapChain::ResizeBuffers] OK");
		}
	}

	void DRender::RegisterHooks()
	{
		if (!Hook::Call5(
				F4SE::GetTrampoline(),
				CreateDXGIFactory.address(),
				reinterpret_cast<std::uintptr_t>(CreateDXGIFactory_Hook),
				m_createDXGIFactory_O)) {
			logger::warn("[Render] CreateDXGIFactory hook failed");
		}

		if (!Hook::Call5(
				F4SE::GetTrampoline(),
				D3D11CreateDeviceAndSwapChain.address(),
				reinterpret_cast<std::uintptr_t>(D3D11CreateDeviceAndSwapChain_Hook),
				m_D3D11CreateDeviceAndSwapChain_O)) {
			logger::error("[Render] D3D11CreateDeviceAndSwapChain hook failed");
			SetOK(false);
			return;
		}
		IEvents::RegisterForEvent(Event::OnConfigLoad, OnConfigLoad);
	}

	void DRender::PostInit()
	{
		struct PresentHook : Xbyak::CodeGenerator
		{
			PresentHook(std::uintptr_t targetAddr)
			{
				Xbyak::Label callLabel;
				Xbyak::Label retnLabel;

				mov(edx, dword[rax + 0x40]);
				call(ptr[rip + callLabel]);
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(targetAddr + 0x6);

				L(callLabel);
				dq(std::uintptr_t(Present_Hook));
			}
		};

		logger::info("[Render] [Patch] [IDXGISwapChain::Present] patching...");
		{
			PresentHook code(Present.address());
			code.ready();

			auto& trampoline = F4SE::GetTrampoline();
			trampoline.write_branch<6>(
				Present.address(),
				trampoline.allocate(code));
		}
		logger::info("[Render] [Patch] [IDXGISwapChain::Present] OK");

		logger::info("[Render] Installed present hook (pre:{} post:{})", m_Instance.m_presentCallbacksPre.size(), m_Instance.m_presentCallbacksPost.size());

		m_OSDDriver = IDDispatcher::GetDriver<DOSD>();
		if (m_OSDDriver && m_OSDDriver->IsOK() && m_OSDDriver->m_conf.enabled) {
			if (m_conf.loading_time || m_conf.bare_loading_time) {
				OSD_Load_Time = true;
			}
		}
		if (OSD_Load_Time) {
			IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_LoadTime);
		}
	}

	bool DRender::Prepare()
	{
		return true;
	}

	void DRender::OnGameConfigLoaded()
	{
		if (m_Instance.m_conf.disable_clamp) {
			m_gv.iFPSClamp = RE::GetINISettingAddr<std::int32_t>("iFPSClamp:General");
			ASSERT(m_gv.iFPSClamp);
		}
	}

	void DRender::OnConfigLoad(Event m_code, void* args)
	{
		if (m_Instance.m_conf.disable_clamp) {
			if (*m_Instance.m_gv.iFPSClamp != 0) {
				logger::info("[Render] Setting iFPSClamp=0");
				*m_Instance.m_gv.iFPSClamp = 0;
			}
		}
	}

	void DRender::IntExt()
	{
		if (*g_extInt == 0) {
			current_fps_max = fps_max = m_limits.ext_fps;
		} else {
			m_Instance.current_fps_max = m_Instance.fps_max = m_limits.int_fps;
		}
	}

	void DRender::SetFPSLimitOverride(long long max, bool disable_vsync)
	{
		if (m_conf.vsync_on) {
			if (disable_vsync) {
				m_current_vsync_present_interval = 0;
				if (tearing_enabled) {
					m_present_flags |= DXGI_PRESENT_ALLOW_TEARING;
				}
			} else {
				m_current_vsync_present_interval = m_vsync_present_interval;
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
		m_Instance.SetFPSLimitOverride(max, disable_vsync);
	}

	void DRender::QueueFPSLimitPost(long long a_max, long long a_expire)
	{
		F4SE::GetTaskInterface()->AddTask([=, this]() {
			m_Instance.SetFPSLimitPost(a_max, a_expire);
		});
	}

	void DRender::QueueFPSLimitOverrideReset()
	{
		F4SE::GetTaskInterface()->AddTask([this]() {
			m_Instance.ResetFPSLimitOverride();
		});
	}

	void DRender::QueueIntExt()
	{
		F4SE::GetTaskInterface()->AddTask([this]() {
			m_Instance.IntExt();
		});
	}

	void DRender::Register()
	{
		DRender* sink = new DRender();
		RE::UI::GetSingleton()->RegisterSink<RE::MenuOpenCloseEvent>(sink);
	}
	EventResult DRender::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (a_event.menuName == RE::LoadingMenu::MENU_NAME) {
			if (a_event.opening) {
				QueueFPSLimitOverride(m_Instance.m_limits.loading_fps, m_Instance.m_conf.disable_vsync_loading);
				if (m_Instance.OSD_Load_Time) {
					m_Instance.loading = true;
					m_Instance.m_stats.start = clock();
				}
			} else {
				if (m_Instance.OSD_Load_Time) {
					m_Instance.last = true;
					m_Instance.m_stats.end = clock();
					m_Instance.loading = false;
				}
				if (m_Instance.intextlimits) {
					QueueIntExt();
				}
				QueueFPSLimitOverrideReset();
			}
		}
		if (a_event.menuName == RE::LockpickingMenu::MENU_NAME) {
			if (a_event.opening) {
				QueueFPSLimitOverride(m_Instance.m_limits.lockpick_fps, false);
			} else {
				QueueFPSLimitOverrideReset();
			}
		}
		if (a_event.menuName == RE::PipboyMenu::MENU_NAME) {
			if (a_event.opening) {
				QueueFPSLimitOverride(m_Instance.m_limits.pipboy_fps, false);
			} else {
				QueueFPSLimitOverrideReset();
			}
		}
		if (DMisc::LimitThreads) {
			if (a_event.menuName == RE::BSFixedString("FaderMenu")) {
				if (a_event.opening) {
				} else {
					DMisc::ReturnThreadsNG();
				}
			}
		}
		return EventResult::kContinue;
	}

	long long DRender::GetCurrentFramerateLimit()
	{
		if (m_Instance.oo_expire_time > 0) {
			if (IPerfCounter::Query() < m_Instance.oo_expire_time) {
				return m_Instance.oo_current_fps_max;
			} else {
				m_Instance.oo_expire_time = 0;
			}
		}

		return m_Instance.current_fps_max;
	}

	void DRender::Throttle(IDXGISwapChain*)
	{
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
				m_conf.limits.game < maxt) {
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
			} else if (m_dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL) {
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
				!(m_dxgi.caps & DXGI_CAP_FLIP_DISCARD)) {
				nse = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			}

			if (nse == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL &&
				!(m_dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL)) {
				nse = DXGI_SWAP_EFFECT_DISCARD;
			}

			if (nse != se) {
				logger::warn("[Render] {} not supported, using {}", GetSwapEffectOption(se), GetSwapEffectOption(nse));
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
		} else {
			pSwapChainDesc->SwapEffect = ManualGetSwapEffect(pSwapChainDesc);
		}
		bool flip_model = IsFlipOn(pSwapChainDesc);

		if (pSwapChainDesc->Windowed == TRUE) {
			if (m_conf.enable_tearing && flip_model && (!m_conf.vsync_on || limiter_installed)) {
				if (m_dxgi.caps & DXGI_CAP_TEARING) {
					pSwapChainDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

					if (!m_conf.vsync_on) {
						m_present_flags |= DXGI_PRESENT_ALLOW_TEARING;
					}

					tearing_enabled = true;
				} else {
					logger::warn("[Render] DXGI_FEATURE_PRESENT_ALLOW_TEARING not supported");
				}
			}
		} else {
			if (has_scaling_mode) {
				pSwapChainDesc->BufferDesc.Scaling = m_conf.scaling_mode;
			}
		}

		if (m_conf.buffer_count == 0) {
			if (flip_model) {
				pSwapChainDesc->BufferCount = 3;
			}
		} else if (m_conf.buffer_count > 0) {
			pSwapChainDesc->BufferCount = static_cast<UINT>(m_conf.buffer_count);
		}

		if (flip_model) {
			pSwapChainDesc->SampleDesc.Count = 1;
			pSwapChainDesc->SampleDesc.Quality = 0;

			if (pSwapChainDesc->BufferCount < 2) {
				pSwapChainDesc->BufferCount = 2;
				logger::warn("[Render] Buffer count below the minimum required for flip model, increasing to 2");
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

	static std::string GetAdapterName(IDXGIAdapter* pAdapter)
	{
		DXGI_ADAPTER_DESC ad;
		if (SUCCEEDED(pAdapter->GetDesc(&ad))) {
			return to_native(ad.Description);
		} else {
			return {};
		}
	}

	void DRender::OnD3D11PreCreate(IDXGIAdapter* pAdapter, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
	{
		if (!ValidateDisplayMode(pSwapChainDesc)) {
			logger::warn(
				"[Render] Invalid refresh rate: ({}/{})",
				pSwapChainDesc->BufferDesc.RefreshRate.Numerator,
				pSwapChainDesc->BufferDesc.RefreshRate.Denominator);
		}

		logger::info("[Render] Adapter: {}", GetAdapterName(pAdapter).c_str());

		ApplyD3DSettings(const_cast<DXGI_SWAP_CHAIN_DESC*>(pSwapChainDesc));

		logger::info(
			"[Render] [D3D] Requesting mode: {}x{}@{} | VSync: {} | Windowed: {}",
			pSwapChainDesc->BufferDesc.Width,
			pSwapChainDesc->BufferDesc.Height,
			GetRefreshRate(pSwapChainDesc),
			m_vsync_present_interval,
			pSwapChainDesc->Windowed);

		logger::info(
			"[Render] [D3D] SwapEffect: {} | SwapBufferCount: {} | Tearing: {} | Flags: {}",
			GetSwapEffectOption(pSwapChainDesc->SwapEffect),
			pSwapChainDesc->BufferCount,
			tearing_enabled,
			pSwapChainDesc->Flags);

		logger::info(
			"[Render] [D3D] Windowed hardware composition support: {}",
			HasWindowedHWCompositionSupport(pAdapter) ? "yes" : "no");

		if (pSwapChainDesc->Windowed == TRUE) {
			if (!IsFlipOn(pSwapChainDesc)) {
				if (!(m_dxgi.caps & (DXGI_CAP_FLIP_DISCARD | DXGI_CAP_FLIP_SEQUENTIAL))) {
					logger::warn("[Render] Flip not supported on your system, switch to exclusive fullscreen for better peformance");
				} else {
					logger::warn("[Render] Switch to exclusive fullscreen or set SwapEffect to flip_discard or flip_sequential for better peformance");
				}
			}
		} else {
			if (IsFlipOn(pSwapChainDesc)) {
				logger::warn("[Render] Using flip in exclusive fullscreen may cause issues");
			}
		}
	}

	HRESULT WINAPI DRender::D3D11CreateDeviceAndSwapChain_Hook(
		_In_opt_ IDXGIAdapter*                                 pAdapter,
		D3D_DRIVER_TYPE                                        DriverType,
		HMODULE                                                Software,
		UINT                                                   Flags,
		_In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
		UINT                                                   FeatureLevels,
		UINT                                                   SDKVersion,
		_In_opt_ CONST DXGI_SWAP_CHAIN_DESC*                   pSwapChainDesc,
		_COM_Outptr_opt_ IDXGISwapChain**                      ppSwapChain,
		_COM_Outptr_opt_ ID3D11Device**                        ppDevice,
		_Out_opt_ D3D_FEATURE_LEVEL*                           pFeatureLevel,
		_COM_Outptr_opt_ ID3D11DeviceContext**                 ppImmediateContext)
	{
		m_Instance.OnD3D11PreCreate(pAdapter, pSwapChainDesc);

		D3D11CreateEventPre evd_pre(pSwapChainDesc);
		IEvents::TriggerEvent(Event::OnD3D11PreCreate, reinterpret_cast<void*>(&evd_pre));

		auto hr = m_Instance.m_D3D11CreateDeviceAndSwapChain_O(
			pAdapter,
			DriverType,
			Software,
			Flags,
			pFeatureLevels,
			FeatureLevels,
			SDKVersion,
			pSwapChainDesc,
			ppSwapChain,
			ppDevice,
			pFeatureLevel,
			ppImmediateContext);

		if (hr > 0) {
			logger::warn("[Render] D3D11CreateDeviceAndSwapChain returned status: {}", hr);
		}

		if (SUCCEEDED(hr)) {
			if (ppDevice && ppSwapChain) {
				D3D11CreateEventPost evd_post(
					pSwapChainDesc,
					*ppDevice,
					*ppImmediateContext,
					*ppSwapChain,
					pAdapter);

				if (evd_post.m_pDevice && evd_post.m_pImmediateContext) {
					IEvents::TriggerEvent(Event::OnD3D11PostCreate, reinterpret_cast<void*>(&evd_post));
					IEvents::TriggerEvent(Event::OnD3D11PostPostCreate, reinterpret_cast<void*>(&evd_post));
				}
			}
		} else {
			logger::error(
				PLUGIN_NAME_LONG,
				"[Render] D3D11CreateDeviceAndSwapChain failed: {}",
				hr);
		}

		return hr;
	}

	HRESULT WINAPI DRender::CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory)
	{
		HRESULT hr = m_Instance.m_createDXGIFactory_O(riid, ppFactory);
		if (SUCCEEDED(hr)) {
			m_Instance.m_dxgiFactory = static_cast<IDXGIFactory*>(*ppFactory);
		} else {
			logger::critical("[Render] CreateDXGIFactory failed {}", hr);
		}
		return hr;
	}

	HRESULT STDMETHODCALLTYPE DRender::Present_Hook(
		IDXGISwapChain4* pSwapChain,
		UINT             SyncInterval,
		UINT             PresentFlags)
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
		bool          release;
		IDXGIFactory* factory;

		if (!m_dxgiFactory) {
			logger::warn("[Render] IDXGIFactory not set, attempting to create..");

			factory = DXGI_GetFactory();
			if (!factory) {
				logger::warn("[Render] Couldn't create IDXGIFactory, assuming DXGI_CAPS_ALL");
				m_dxgi.caps = DXGI_CAPS_ALL;
				return;
			}

			release = true;
		} else {
			factory = m_dxgiFactory;
			release = false;
		}

		m_dxgi.caps = 0;

		do {
			{
				ComPtr<IDXGIFactory5> tmp;
				if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
					m_dxgi.caps = (DXGI_CAP_FLIP_SEQUENTIAL |
								   DXGI_CAP_FLIP_DISCARD);

					BOOL    allowTearing;
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
					m_dxgi.caps = (DXGI_CAP_FLIP_SEQUENTIAL |
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

		for (UINT i = 0;; ++i) {
			ComPtr<IDXGIOutput> output;
			if (FAILED(adapter->EnumOutputs(i, &output))) {
				break;
			}

			ComPtr<IDXGIOutput6> output6;
			if (SUCCEEDED(output.As(&output6))) {
				UINT    flags;
				HRESULT hr = output6->CheckHardwareCompositionSupport(&flags);

				if (SUCCEEDED(hr) && (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED)) {
					return true;
				}
			}
		}

		return false;
	}

	const wchar_t* DRender::StatsRendererCallback_LoadTime()
	{
		if (m_Instance.loading) {
			m_Instance.m_stats.end = clock();
			if (m_Instance.m_conf.loading_time) {
				::_snwprintf_s(m_Instance.bufStats, _TRUNCATE, L"\nLoading time: %.2f sec", ((double)m_Instance.m_stats.end - m_Instance.m_stats.start) / ((double)CLOCKS_PER_SEC));
			}
			if (m_Instance.m_conf.bare_loading_time) {
				::_snwprintf_s(m_Instance.bufStats, _TRUNCATE, L"\n%.2f", ((double)m_Instance.m_stats.end - m_Instance.m_stats.start) / ((double)CLOCKS_PER_SEC));
			}
		} else if (m_Instance.last) {
			if (m_Instance.m_conf.loading_time) {
				::_snwprintf_s(m_Instance.bufStats, _TRUNCATE, L"\nLoading time: %.2f sec", ((double)m_Instance.m_stats.end - m_Instance.m_stats.start) / ((double)CLOCKS_PER_SEC));
			}
			if (m_Instance.m_conf.bare_loading_time) {
				::_snwprintf_s(m_Instance.bufStats, _TRUNCATE, L"\n%.2f", ((double)m_Instance.m_stats.end - m_Instance.m_stats.start) / ((double)CLOCKS_PER_SEC));
			}
			m_Instance.m_stats.timeout = clock();
			if (m_Instance.m_conf.delay <= ((double)m_Instance.m_stats.timeout - m_Instance.m_stats.end) / ((double)CLOCKS_PER_SEC)) {
				::_snwprintf_s(m_Instance.bufStats, _TRUNCATE, L"");
				m_Instance.last = false;
			}
		}

		return m_Instance.bufStats;
	}

	void DRender::OnD3D11PostCreate_LoadTime(Event code, void* data)
	{
		m_Instance.m_OSDDriver->AddStatsCallback(StatsRendererCallback_LoadTime);
	}
}
