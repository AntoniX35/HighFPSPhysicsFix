#pragma once

namespace SDT
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

        struct {
            static inline float* fUpdateBudgetMS;
        }static inline m_gv;

        struct {
            bool dynbudget_enabled;
            float dynbudget_fps_min;
            float dynbudget_fps_max;
            float dynbudget_base;
            static inline bool stats_enabled;
            bool warn_overstressed;
        }static inline m_conf;

        static inline uintptr_t UpdateBudgetGame;

    private:
        DPapyrus();

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual bool Prepare() override;
        virtual void RegisterHooks() override;

        static float __forceinline CalculateUpdateBudget();
        static float CalculateUpdateBudgetStats();

        static const wchar_t* StatsRendererCallback1();

        static void OnD3D11PostCreate_Papyrus(Event code, void* data);

        float m_lastInterval;

        float m_bmult;
        float m_t_max;
        float m_t_min;

        wchar_t m_bufStats1[64];
        static inline uintptr_t UpdateBudgetUI;
        DOSD* m_OSDDriver;

        StatsCounter m_stats_counter;

        static DPapyrus m_Instance;
    };
}