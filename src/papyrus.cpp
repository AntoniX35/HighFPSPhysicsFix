#include "PCH.h"
namespace HFPF
{
	static constexpr const char* SECTION_PAPYRUS = "Papyrus";
	static constexpr const char* CKEY_DYNBUDGETENABLED = "DynamicUpdateBudget";
	static constexpr const char* CKEY_UPDATEBUDGET = "UpdateBudgetBase";
	static constexpr const char* CKEY_MAXTIME = "BudgetMaxFPS";
	static constexpr const char* CKEY_STATSON = "OSDStatsEnabled";

	DPapyrus DPapyrus::m_Instance;

	DPapyrus::DPapyrus() :
		m_lastInterval(1.0f / 60.0f)
	{
		m_bufStats1[0] = 0x0;
	}

	void DPapyrus::LoadConfig()
	{
		m_conf.dynbudget_enabled = GetConfigValue(SECTION_PAPYRUS, CKEY_DYNBUDGETENABLED, false);
		m_conf.dynbudget_fps_min = 60.0f;
		m_conf.dynbudget_fps_max = std::clamp(GetConfigValue(SECTION_PAPYRUS, CKEY_MAXTIME, 144.0f), m_conf.dynbudget_fps_min, 300.0f);
		m_conf.dynbudget_base = std::clamp(GetConfigValue(SECTION_PAPYRUS, CKEY_UPDATEBUDGET, 1.2f), 0.1f, 4.0f);
		m_conf.stats_enabled = GetConfigValue(SECTION_PAPYRUS, CKEY_STATSON, false);
	}

	void DPapyrus::PostLoadConfig()
	{
		m_OSDDriver = IDDispatcher::GetDriver<DOSD>();
		if (!m_OSDDriver || !m_OSDDriver->IsOK()) {
			m_conf.stats_enabled = false;
		}

		if (m_conf.dynbudget_enabled) {
			if (m_conf.dynbudget_fps_max == m_conf.dynbudget_fps_min) {
				logger::warn("[Papyrus] dynbudget_fps_max == dynbudget_fps_min, disabling..");
				m_conf.dynbudget_enabled = false;
			} else {
				m_bmult = m_conf.dynbudget_base / (1.0f / 60.0f * 1000.0f) * 1000.0f;
				m_t_max = 1.0f / m_conf.dynbudget_fps_min;
				m_t_min = 1.0f / m_conf.dynbudget_fps_max;

				logger::info("[Papyrus] UpdateBudgetBase: {} ms ({} - {})",
					m_conf.dynbudget_base, m_t_min * m_bmult, m_t_max * m_bmult);
			}
		}
	}

	void DPapyrus::Patch()
	{
		if (m_conf.dynbudget_enabled) {
			struct UpdateBudgetInject : Xbyak::CodeGenerator
			{
				UpdateBudgetInject(std::uintptr_t targetAddr, bool enable_stats)
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
					} else {
						dq(std::uintptr_t(DPapyrus::CalculateUpdateBudget));
					}
				}
			};

			logger::info("[Papyrus] [Patch] [UpdateBudget (game)] patching...");
			{
				UpdateBudgetInject code(UpdateBudgetGame.address(), m_conf.stats_enabled);
				code.ready();

				auto& trampoline = F4SE::GetTrampoline();
				trampoline.write_branch<6>(
					UpdateBudgetGame.address(),
					trampoline.allocate(code));

				REL::safe_write(UpdateBudgetGame.address() + 0x6, &Payloads::NOP2, 2);
			}
			logger::info("[Papyrus] [Patch] [UpdateBudget (game)] OK");

			logger::info("[Papyrus] [Patch] [UpdateBudget (UI)] patching...");
			{
				UpdateBudgetInject code(UpdateBudgetUI.address(), m_conf.stats_enabled);

				auto& trampoline = F4SE::GetTrampoline();
				trampoline.write_branch<6>(
					UpdateBudgetUI.address(),
					trampoline.allocate(code));

				REL::safe_write(UpdateBudgetUI.address() + 0x6, &Payloads::NOP2, 2);
			}
			logger::info("[Papyrus] [Patch] [UpdateBudget (UI)] OK");
			{
				UpdateBudgetInject code(UpdateBudget.address(), m_conf.stats_enabled);

				auto& trampoline = F4SE::GetTrampoline();
				trampoline.write_branch<6>(
					UpdateBudget.address(),
					trampoline.allocate(code));

				REL::safe_write(UpdateBudget.address() + 0x6, &Payloads::NOP2, 2);
			}
		}
	}

	void DPapyrus::RegisterHooks()
	{
		if (m_conf.dynbudget_enabled && m_conf.stats_enabled) {
			IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_Papyrus);
		}
	}

	void DPapyrus::OnGameConfigLoaded()
	{
		if (m_conf.dynbudget_enabled) {
			m_gv.fUpdateBudgetMS = RE::GetINISettingAddr<float>("fUpdateBudgetMS:Papyrus");
			ASSERT(m_gv.fUpdateBudgetMS);
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
		} else {
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
