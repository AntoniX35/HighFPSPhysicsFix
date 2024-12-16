#pragma once

namespace HFPF
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
	class IEvents
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

		static void PostLoadPluginINI_Hook(void* a_unk);

		std::unordered_map<Event, std::vector<_EventTriggerDescriptor>> m_events;

		decltype(&PostLoadPluginINI_Hook) LoadPluginINI_O;

		inline static REL::Relocation<std::uintptr_t> LoadPluginINI_C{ AID::FPS_Cap_Patch1, Offsets::LoadPluginINI_C };

		static IEvents m_Instance;
	};
}
