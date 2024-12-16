#pragma once
namespace HFPF
{
	class DPapyrus :
		public IDriver,
		IConfig
	{
	public:
		static inline constexpr auto ID = DRIVER_ID::PAPYRUS;

		FN_NAMEPROC("Papyrus");
		FN_ESSENTIAL(false);
		FN_DRVDEF(6);

	private:
		DPapyrus();

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void Patch() override;
		virtual bool Prepare() override;
		virtual void RegisterHooks() override;
		virtual void OnGameConfigLoaded() override;

		static float SKMP_FORCEINLINE CalculateUpdateBudget();
		static float                  CalculateUpdateBudgetStats();

		static const wchar_t* StatsRendererCallback1();

		static void OnD3D11PostCreate_Papyrus(Event code, void* data);

		struct
		{
			bool  dynbudget_enabled;
			float dynbudget_fps_min;
			float dynbudget_fps_max;
			float dynbudget_base;
			bool  stats_enabled;
		} m_conf;

		inline static REL::Relocation<std::uintptr_t> UntieAddress{ AID::Untie, Offsets::Untie };
		inline static REL::Relocation<std::uintptr_t> UpdateBudgetGame{ AID::BudgetGame, Offsets::BudgetGame };
		inline static REL::Relocation<std::uintptr_t> UpdateBudgetUI{ AID::BudgetUI, Offsets::BudgetUI };
		inline static REL::Relocation<std::uintptr_t> UpdateBudget{ AID::Budget, Offsets::Budget };

		float m_lastInterval;

		struct
		{
			float* fUpdateBudgetMS{ nullptr };
		} m_gv;

		float m_bmult;
		float m_t_max;
		float m_t_min;

		wchar_t m_bufStats1[64];

		DOSD* m_OSDDriver;

		StatsCounter m_stats_counter;

		static DPapyrus m_Instance;
	};
}
