#include "PCH.h"

namespace HFPF
{
	IEvents IEvents::m_Instance;

	bool IEvents::Initialize()
	{
		if (!Hook::Call5(
				F4SE::GetTrampoline(),
				LoadPluginINI_C.address(),
				reinterpret_cast<std::uintptr_t>(PostLoadPluginINI_Hook),
				m_Instance.LoadPluginINI_O)) {
			logger::critical("[Events] Could not install event hooks");
			return false;
		}

		logger::info("[Events] Installed event hooks");

		return true;
	}

	void IEvents::RegisterForEvent(Event a_code, EventCallback a_fn)
	{
		m_Instance.m_events.try_emplace(a_code).first->second.emplace_back(a_code, a_fn);
	}

	void IEvents::TriggerEvent(Event a_code, void* a_args)
	{
		const auto it = m_Instance.m_events.find(a_code);
		if (it == m_Instance.m_events.end())
			return;

		for (const auto& evtd : it->second)
			evtd.m_callback(a_code, a_args);
	}

	void IEvents::PostLoadPluginINI_Hook(void* a_unk)
	{
		m_Instance.LoadPluginINI_O(a_unk);
		IDDispatcher::DriversOnGameConfigLoaded();
		m_Instance.TriggerEvent(Event::OnConfigLoad);
	}
}
