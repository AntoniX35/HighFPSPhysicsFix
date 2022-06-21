#include "pch.h"

namespace SDT
{
    IEvents IEvents::m_Instance;

    bool IEvents::Initialize()
    {
        auto& f4se = IF4SE::GetSingleton();

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

    void IEvents::MessageHandler(F4SEMessagingInterface::Message* a_message)
    {
        m_Instance.TriggerEvent(Event::OnMessage, static_cast<void*>(a_message));
    }
}

