#include "pch.h"
namespace SDT
{
    DInput DInput::m_Instance;

    void DInput::RegisterHooks()
    {
        IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
    }

    bool DInput::Prepare()
    {
        return true;
    }

    void DInput::RegisterForKeyEvents(KeyEventHandler* const h)
    {
        m_Instance.m_handlers.emplace_back(h);
    }

    void DInput::MessageHandler(Event m_code, void* a_args)
    {
        auto message = static_cast<F4SEMessagingInterface::Message*>(a_args);

        switch (message->type)
        {
        case F4SEMessagingInterface::kMessage_InputLoaded:
        {
            if (m_Instance.m_handlers.empty()) {
                break;
            }
            break;
        }
        }
    }
}