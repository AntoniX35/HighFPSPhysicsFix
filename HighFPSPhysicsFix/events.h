#pragma once
namespace SDT
{
    enum class Event : std::uint32_t
    {
        OnConfigLoad,
        OnD3D11PreCreate,
        OnD3D11PostCreate,
        OnD3D11PostPostCreate,
        OnCreateWindowEx,
        OnMenuEvent,
        OnMessage,
        OnGameSave,
        OnGameLoad,
        OnFormDelete,
        OnRevert,
        OnLogMessage,
        OnGameShutdown,
        OnExit
    };

    typedef void (*EventCallback)(Event, void*);

    template <typename E, typename C>
    class EventTriggerDescriptor
    {
    public:
        EventTriggerDescriptor(E m_code, C callback) :
            m_code(m_code), m_callback(callback)
        {}

        E m_code;
        C m_callback;
    };

    class IEvents :
        protected IHook
    {
    public:

        static inline constexpr auto ID = DRIVER_ID::EVENTS;

        typedef EventTriggerDescriptor<Event, EventCallback> _EventTriggerDescriptor;

        static bool Initialize();
        static void RegisterForEvent(Event a_code, EventCallback a_fn);
        static void TriggerEvent(Event a_code, void* a_args = nullptr);

        FN_NAMEPROC("Events");
        FN_ESSENTIAL(true);
        FN_DRVDEF(0);
    private:
        IEvents() = default;

        static void MessageHandler(F4SEMessagingInterface::Message* a_message);

        std::unordered_map<Event, std::vector<_EventTriggerDescriptor>> m_events;

        static IEvents m_Instance;
    };
}