#include "pch.h"

#include "WinAPI.h"

bool comboKeyDown = false;
F4SEScaleformInterface* g_scaleform = NULL;
F4SEMessagingInterface* g_messaging = NULL;
PluginHandle			    g_pluginHandle = kPluginHandle_Invalid;

static F4SEPapyrusInterface* g_papyrus = NULL;

class MenuOpenCloseHandler : public BSTEventSink<MenuOpenCloseEvent>
	{
	public:
		virtual ~MenuOpenCloseHandler() { };
		virtual	EventResult	ReceiveEvent(MenuOpenCloseEvent* evn, void* dispatcher) override;
};

std::chrono::high_resolution_clock::time_point tlastf = std::chrono::high_resolution_clock::now(), tcurrentf = std::chrono::high_resolution_clock::now(), hvlast = std::chrono::high_resolution_clock::now(), hvcurrent = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> time_span, tdif;

//Detect loading screen and lockpicking menu;
EventResult	MenuOpenCloseHandler::ReceiveEvent(MenuOpenCloseEvent* evn, void* dispatcher)
{
	if (!_strcmpi("LoadingMenu", evn->menuName.c_str())\
		)
	{
		if (evn->isOpen)
		{
			SDT::DRender::current_fps_max = SDT::DRender::loading_fps;
			if (SDT::DOSD::m_conf.loading_time || SDT::DOSD::m_conf.bare_loading_time) {
				SDT::DOSD::loading = true;
				SDT::DOSD::m_stats.start = clock();
			}
			if (SDT::DRender::m_conf.vsync_on && SDT::DRender::m_conf.disable_vsync) {
				SDT::DRender::m_current_vsync_present_interval = 0;
				if (SDT::DRender::tearing_enabled) {
					SDT::DRender::m_present_flags |= DXGI_PRESENT_ALLOW_TEARING;
				}
			}
		}
		else
		{
			if (SDT::DOSD::m_conf.loading_time || SDT::DOSD::m_conf.bare_loading_time) {
				SDT::DOSD::m_stats.end = clock();
				SDT::DOSD::last = true;
				SDT::DOSD::loading = false;
			}
			if (SDT::DRender::m_conf.vsync_on && SDT::DRender::m_conf.disable_vsync) {
				SDT::DRender::m_current_vsync_present_interval = SDT::DRender::m_vsync_present_interval;
				if (SDT::DRender::tearing_enabled) {
					SDT::DRender::m_present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
				}
			}
			SDT::DRender::current_fps_max = SDT::DRender::fps_max;
		}
	}
	if (SDT::DRender::lockpick_fps > 0) {
		if (!_strcmpi("LockpickingMenu", evn->menuName.c_str())\
			)
		{
			if (evn->isOpen)
			{
				SDT::DRender::current_fps_max = SDT::DRender::lockpick_fps;
			}
			else
			{
				SDT::DRender::current_fps_max = SDT::DRender::fps_max;
			}
		}
	}
	if (SDT::DRender::pipboy_fps > 0) {
		if (!_strcmpi("PipboyMenu", evn->menuName.c_str())\
			)
		{
			if (evn->isOpen)
			{
				SDT::DRender::current_fps_max = SDT::DRender::pipboy_fps;
			}
			else
			{
				SDT::DRender::current_fps_max = SDT::DRender::fps_max;
			}
		}
	}
	if (!_strcmpi("FaderMenu", evn->menuName.c_str())\
		)
	{
		if (evn->isOpen) {
		}
		else {
			SDT::DMisc::ReturnThreadsNG();
		}
	}
	return kEvent_Continue;
}

class F4SEInputHandler : public BSInputEventUser
{
public:
	F4SEInputHandler() : BSInputEventUser(true) { }
	virtual void OnButtonEvent(ButtonEvent* inputEvent)
	{
		UInt32	keyCode;
		UInt32	deviceType = inputEvent->deviceType;
		UInt32	keyMask = inputEvent->keyMask;

		if (deviceType == InputEvent::kDeviceType_Mouse)
			keyCode = InputMap::kMacro_MouseButtonOffset + keyMask;

		else if (deviceType == InputEvent::kDeviceType_Gamepad) {
			keyCode = InputMap::GamepadMaskToKeycode(keyMask);
		}
		else {
			keyCode = keyMask;
		}

		float timer = inputEvent->timer;
		bool  isDown = inputEvent->isDown == 1.0f && timer == 0.0f;
		bool  isUp = inputEvent->isDown == 0.0f && timer != 0.0f;
		BSFixedString* control = inputEvent->GetControlID();
		if (isDown) {
			if (!SDT::DOSD::m_conf.combo_key == 0) {
				if (keyCode == SDT::DOSD::m_conf.combo_key) {
					comboKeyDown = true;
				}
			}
			else {
				comboKeyDown = true;
			}
			if (keyCode == SDT::DOSD::m_conf.key && (!SDT::DOSD::m_conf.combo_key || comboKeyDown)) {
				if (!SDT::DOSD::m_stats.draw) {
					SDT::DOSD::m_stats.draw = true;
				}
				else {
					SDT::DOSD::m_stats.draw = false;
				}
			}
			if (keyCode == 36) { //Клавиша Home для 60FPS //ТЕСТ
				if (comboKeyDown) {
					_MESSAGE("Home");
					SDT::DRender::current_fps_max = SDT::DRender::fps60;
					comboKeyDown = false;
				}
				else {
					SDT::DRender::current_fps_max = SDT::DRender::fps_max;
					comboKeyDown = true;
				}
			}
		}
		if (isUp && keyCode == SDT::DOSD::m_conf.combo_key) {
				comboKeyDown = false;
		}
	}
};
F4SEInputHandler g_inputHandler;

void onF4SEMessage(F4SEMessagingInterface::Message* msg) {
	switch (msg->type)
	{
	case F4SEMessagingInterface::kMessage_GameLoaded:
	{
		(*g_menuControls)->inputEvents.Push(&g_inputHandler);
	}
	break;
	case F4SEMessagingInterface::kMessage_NewGame:
	{
			SDT::DMisc::SetThreadsNG();
	}
	break;
	case F4SEMessagingInterface::kMessage_GameDataReady:
	{
		static auto pMenuOpenCloseHandler = new MenuOpenCloseHandler();
		(*g_ui)->menuOpenCloseEventSource.AddEventSink(pMenuOpenCloseHandler);
		int* LocationX = SDT::IF4SE::GetINISettingAddr<int>("iLocation X:Display");
		int* LocationY = SDT::IF4SE::GetINISettingAddr<int>("iLocation Y:Display");
		SDT::DPapyrus::m_gv.fUpdateBudgetMS = SDT::IF4SE::GetINISettingAddr<float>("fUpdateBudgetMS:Papyrus");
		if (SDT::DWindow::m_conf.center_window) {
			LocationX = 0;
			LocationY = 0;
		}
	}
	break;
	}
}

namespace SDT
{
	static constexpr const char* CONF_SECT_MAIN = "Main";
	static constexpr const char* CONF_MAIN_KEY_LOGGING = "LogLevel";

	class Initializer :
		IConfig
	{
	public:
		int Run(const F4SEInterface* a_f4se)
		{
			int result = LoadConfiguration();
			if (result != 0) {
				_MESSAGE("Unable to load HighFPSPhysicsFix.ini file. Line: (%d)", result);
			}

			if (IsCustomLoaded()) {
				_MESSAGE("Custom configuration loaded");
			}

			auto& f4se = IF4SE::GetSingleton();

			if (!f4se.QueryInterfaces(a_f4se)) {
				return false;
			}
			if (!f4se.CreateTrampolines(a_f4se)) {
				return false;
			}

			result = Initialize(a_f4se);
			if (result == 0)
			{
				auto usageBranch = f4se.GetTrampolineUsage(TrampolineID::kBranch);
				auto usageLocal = f4se.GetTrampolineUsage(TrampolineID::kLocal);

				_MESSAGE("[Trampoline] branch: %zu/%zu, codegen: %zu/%u",
					usageBranch.used, usageBranch.total, usageLocal.used, usageLocal.total);
			}
			ClearConfiguration();
			return result;
		}

	private:

		int Initialize(const F4SEInterface* a_f4se)
		{
			if (!IEvents::Initialize()) {
				return 1;
			}

			if (!IDDispatcher::InitializeDrivers()) {
				return -1;
			}

			return 0;
		}
	};
}
extern "C"
{
	bool F4SEPlugin_Query(const F4SEInterface* a_f4se, PluginInfo* a_info)
	{
		g_pluginHandle = a_f4se->GetPluginHandle();

		//Get messaging interface
		g_messaging = (F4SEMessagingInterface*)a_f4se->QueryInterface(kInterface_Messaging);
		if (!g_messaging) {
			_MESSAGE("couldn't get messaging interface");
			return false;
		}

		return SDT::IF4SE::GetSingleton().Query(a_f4se, a_info);
	}

	bool F4SEPlugin_Load(const F4SEInterface* f4se) {

		RelocAddr<uintptr_t> BethesdaVsyncAddress(0x1D17792);
		RelocAddr<uintptr_t> BethesdaFPSCap1Address(0xD423BA);
		RelocAddr<uintptr_t> BethesdaFPSCap2Address(0xD423C3);
		SafeWriteBuf(BethesdaVsyncAddress.GetUIntPtr(), "\x90\x90\x90\x90", 4);
		//SafeWriteBuf(RelocAddr<uintptr_t>(0x17704DB).GetUIntPtr(), "\xC3\x90\x90\x90\x90", 5);
		uint32_t maxrefreshrate = 10000;
		SafeWriteBuf(BethesdaFPSCap1Address.GetUIntPtr(), &maxrefreshrate, sizeof(maxrefreshrate));
		SafeWriteBuf(BethesdaFPSCap2Address.GetUIntPtr(), &maxrefreshrate, sizeof(maxrefreshrate));
		ASSERT(SDT::IF4SE::GetSingleton().ModuleHandle() != nullptr);

		g_messaging->RegisterListener(g_pluginHandle, "F4SE", onF4SEMessage);

		bool ok(false);

		int result = SDT::Initializer().Run(f4se);
		if (result == 0) {
			ok = true;
		}
		else if (result == -1)
		{
			WinApi::MessageBoxError(
				"An unrecoverable error has occured during plugin initialization.\n\n"
				"Some patches have already been applied before this error occured."
				"The game process will be terminated to avoid issues.\n\n"
				"See the HighFPSPhysics.log for more info."
			);
			std::_Exit(1);
		}
		return ok;
	}
};

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		SDT::IF4SE::GetSingleton().SetModuleHandle(hModule);
		break;
	}
	return TRUE;
}