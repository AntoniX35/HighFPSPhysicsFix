#include "pch.h"
#include "f4se\PapyrusVM.h"
namespace SDT
{
    static constexpr const char* SECTION_PAPYRUS = "Papyrus";
    static constexpr const char* CONF_SEOFIX = "SetExpressionOverridePatch";
    static constexpr const char* CONF_DYNBUDGETENABLED = "DynamicUpdateBudget";
    static constexpr const char* CONF_UPDATEBUDGET = "UpdateBudgetBase";
    static constexpr const char* CONF_MAXTIME = "BudgetMaxFPS";
    static constexpr const char* CONF_STATSON = "OSDStatsEnabled";
    
    using namespace Patching;

    DPapyrus DPapyrus::m_Instance;

    DPapyrus::DPapyrus() :
        m_lastInterval(1.0f / 60.0f)
    {
        m_bufStats1[0] = 0x0;
    }

    void DPapyrus::LoadConfig()
    {
        m_conf.dynbudget_enabled = GetConfigValueBool(SECTION_PAPYRUS, CONF_DYNBUDGETENABLED, false);
        m_conf.dynbudget_fps_min = 60.0f;
        m_conf.dynbudget_fps_max = std::clamp(GetConfigValueFloat(SECTION_PAPYRUS, CONF_MAXTIME, 144.0f), m_conf.dynbudget_fps_min, 300.0f);
        m_conf.dynbudget_base = std::clamp(GetConfigValueFloat(SECTION_PAPYRUS, CONF_UPDATEBUDGET, 1.2f), 0.1f, 4.0f);
        m_conf.stats_enabled = GetConfigValueBool(SECTION_PAPYRUS, CONF_STATSON, false);
    }

    void DPapyrus::PostLoadConfig()
    {
        m_OSDDriver = IDDispatcher::GetDriver<DOSD>();
        if (!m_OSDDriver || !m_OSDDriver->IsOK())
        {
            m_conf.stats_enabled = false;
            m_conf.warn_overstressed = false;
        }

        if (m_conf.dynbudget_enabled)
        {
            if (m_conf.dynbudget_fps_max == m_conf.dynbudget_fps_min) {
                _MESSAGE("dynbudget_fps_max == dynbudget_fps_min, disabling..");
                m_conf.dynbudget_enabled = false;
            }
            else
            {
                m_bmult = m_conf.dynbudget_base / (1.0f / 60.0f * 1000.0f) * 1000.0f;
                m_t_max = 1.0f / m_conf.dynbudget_fps_min;
                m_t_min = 1.0f / m_conf.dynbudget_fps_max;

                _MESSAGE("[Papyrus] UpdateBudgetBase: %.6g ms (%.6g - %.6g)",
                    m_conf.dynbudget_base, m_t_min * m_bmult, m_t_max * m_bmult);

            }
        }
    }

    void DPapyrus::Patch() {
        if (m_conf.dynbudget_enabled) {
            UpdateBudgetUI = UpdateBudgetGame + 0x110;
            struct UpdateBudgetInject : JITASM::JITASM {
                UpdateBudgetInject(std::uintptr_t targetAddr, bool enable_stats)
                    : JITASM(IF4SE::GetLocalTrampoline())
                {
                    Xbyak::Label callLabel;
                    Xbyak::Label retnLabel;

                    call(ptr[rip + callLabel]);
                    movss(xmm6, xmm0);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(targetAddr + 0x8);

                    L(callLabel);
                    if (enable_stats) {
                        dq(std::uintptr_t(DPapyrus::CalculateUpdateBudgetStats));
                    }
                    else {
                        dq(std::uintptr_t(DPapyrus::CalculateUpdateBudget));
                    }
                }
            };

            _MESSAGE("[UpdateBudget (game)] patching...");
            {
                UpdateBudgetInject code(UpdateBudgetGame, m_conf.stats_enabled);
                IF4SE::GetBranchTrampoline().Write6Branch(UpdateBudgetGame, code.get());

                safe_memset(UpdateBudgetGame + 0x6, 0x90, 2);
            }
            _MESSAGE("[UpdateBudget (game)] OK");

            _MESSAGE("[UpdateBudget (UI)] patching...");
            {
                UpdateBudgetInject code(UpdateBudgetUI, m_conf.stats_enabled);
                IF4SE::GetBranchTrampoline().Write6Branch(UpdateBudgetUI, code.get());

                safe_memset(UpdateBudgetUI + 0x6, 0x90, 2);
            }
            _MESSAGE("[UpdateBudget (UI)] OK");
        }
    }

    void DPapyrus::RegisterHooks()
    {
        if ((m_conf.dynbudget_enabled && m_conf.stats_enabled) || m_conf.warn_overstressed) {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_Papyrus);
        }
    }

    bool DPapyrus::Prepare()
    {
        
        return true;
    }

    float DPapyrus::CalculateUpdateBudget()
    {
        float interval = std::clamp(*Game::g_frameTimer, m_Instance.m_t_min, m_Instance.m_t_max);

        if (interval <= m_Instance.m_lastInterval) {
            m_Instance.m_lastInterval = interval;
        }
        else {
            m_Instance.m_lastInterval = std::min(m_Instance.m_lastInterval + interval * 0.0075f, interval);
        }

        interval = m_Instance.m_lastInterval * m_Instance.m_bmult;

        *m_Instance.m_gv.fUpdateBudgetMS = interval;
        return interval;
    }

    float DPapyrus::CalculateUpdateBudgetStats()
    {
        float cft = CalculateUpdateBudget();

        m_Instance.m_stats_counter.accum(static_cast<double>(cft));

        return cft;
    }

    const wchar_t* DPapyrus::StatsRendererCallback1()
    {
        double val;
        if (m_Instance.m_stats_counter.get(val)) {
            ::_snwprintf_s(m_Instance.m_bufStats1,
                _TRUNCATE, L"fUpdateBudgetMS: %.4g", val);
        }

        return m_Instance.m_bufStats1;
    }

    void DPapyrus::OnD3D11PostCreate_Papyrus(Event, void*)
    {
        if (m_Instance.m_conf.stats_enabled)
            m_Instance.m_OSDDriver->AddStatsCallback(StatsRendererCallback1);
    }

}