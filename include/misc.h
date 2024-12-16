#pragma once

#include "drv_base.h"

namespace HFPF
{
	class DRender;
	class DMisc :
		public IDriver,
		IConfig
	{
	public:
		static inline constexpr auto ID = DRIVER_ID::MISC;

		static inline HANDLE    hProcess = NULL;
		static inline DWORD_PTR nMaxProcessMaskNG;
		static inline DWORD_PTR nMaxProcessMaskAfterLoad;
		static inline DWORD_PTR dwSystemAffinity;

		static void        SetThreadsNG();
		static void        ReturnThreadsNG();
		static inline bool LimitThreads = false;

		FN_NAMEPROC("Miscellaneous");
		FN_ESSENTIAL(false);
		FN_DRVDEF(6);

	private:
		DMisc();

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void Patch() override;
		virtual void RegisterHooks() override;
		virtual bool Prepare() override;

		static void Patch_SetThreadsNG();
		static void LimiterFunc();

		long long timing;

		struct
		{
			bool      disable_black_loading;
			bool      disable_loading_screens;
			float     post_loading_speed;
			long long fps_loading_screen;
			bool      one_thread;
			bool      fix_cpu_threads;
			bool      disable_actor_fade;
			bool      disable_player_fade;
		} m_conf;

		inline static REL::Relocation<std::uintptr_t> BlackLoadingScreensAddress{ AID::BlackLoadingScreens, Offsets::BlackLoadingScreens };
		inline static REL::Relocation<std::uintptr_t> LoadingScreensAddress{ AID::LoadingScreens, Offsets::LoadingScreens };
		inline static REL::Relocation<std::uintptr_t> PostLoadInjectAddress{ AID::PostLoadInject, Offsets::PostLoadInject };
		inline static REL::Relocation<std::uintptr_t> LoadScreenPlusLimiterAddress{ AID::LoadScreenPlusLimiterInject, Offsets::LoadScreenPlusLimiterInject };
		inline static REL::Relocation<std::uintptr_t> ActorFade_a{ AID::ActorFade, Offsets::ActorFade };
		inline static REL::Relocation<std::uintptr_t> PlayerFade_a{ AID::PlayerFade, Offsets::PlayerFade };

		DRender* m_dRender;

		static DMisc m_Instance;
	};
}
