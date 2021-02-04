#include "f4se/Hooks_Scaleform.cpp"
#include "f4se_common/SafeWrite.h"
#include "f4se_common/BranchTrampoline.h"
#include "xbyak/xbyak.h"
#include <config.h>
#include <shlobj.h>
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <sstream> 
#include <time.h>
#include "common/IDebugLog.h"
#include <d3d11.h>
#include <MinHook.h>
#include <chrono>
#include <algorithm>
#include <ratio>
#include <ctime>
#include <psapi.h>
#include <mutex>
#include <TlHelp32.h>
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "libMinHook-x64-v141-mdd.lib")
#pragma comment(lib, "d3d11.lib")

//Ini File
#define INI_FILE "data\\F4SE\\Plugins\\HighFPSPhysicsFix.ini"

bool firstload = true;
int isLockpicking, isLoading, NumOfThreadsWhileLoading, NumOfThreadsAfterLoad, fpslimit, isLimiting, iPresentIntervalDuringLockpicking;
unsigned int nMaxProcessorMask;
unsigned int nMaxProcessorAfterLoad;
bool FixLockpickingSound, accelerateLoading, vsync, UntieSpeedFromFPS, JumpFix, FixWorkshopRotationSpeed, FixRotationSpeed, FixStuckAnimation, limitload, limitgame, LimitCPUthreads, DisableAnimationOnLoadingScreens, LimitLockpicking, DisableBlackLoadingScreens;
float newvalue, fpsvalue, newvalue2, newvalue3, newvalue4, newvalue5, newvalue6, fpslimitgame, fpslimitload, fpslockpicking, FixMovingItems, fTime;
__int64	TimeAddress;
RelocAddr<uintptr_t> RotationSpeedAddress(0x2C16761);
RelocAddr<uintptr_t> RotationSpeedXAddress(0x12447AC);
RelocAddr<uintptr_t> RotationSpeedYAddress(0x1244850);
RelocAddr<uintptr_t> WorkshopRotationSpeedAddress(0x2C16791);
bool PluginLoadCompleted = false;
bool PluginLoadCompleted2 = false;
HANDLE f4handle = NULL;
double maxft, minft;
bool GetProcess(HWND hWnd);


//bool firstTime = true;
int iPresentInterval0 = 0;
int iPresentInterval1 = 1;
F4SEScaleformInterface* g_scaleform = NULL;
F4SEMessagingInterface* g_messaging = NULL;
PluginHandle			    g_pluginHandle = kPluginHandle_Invalid;

static F4SEPapyrusInterface* g_papyrus = NULL;

typedef void(*_ChangeThreads)(void* unk1, void* unk2, void* unk3, void* unk4);
_ChangeThreads ChangeThreads_Original = nullptr;
typedef void(*_UpdateValues)(void* unk1, void* unk2, void* unk3, void* unk4);
_UpdateValues UpdateValues_Original = nullptr;
typedef void(*_UpdateSValues)(void* unk1, void* unk2, void* unk3, void* unk4);
_UpdateSValues UpdateSValues_Original = nullptr;
typedef void(*_UpdateWValues)(void* unk1, void* unk2, void* unk3, void* unk4);
_UpdateWValues UpdateWValues_Original = nullptr;


class MenuOpenCloseHandler : public BSTEventSink<MenuOpenCloseEvent>
	{
	public:
		virtual ~MenuOpenCloseHandler() { };
		virtual	EventResult	ReceiveEvent(MenuOpenCloseEvent* evn, void* dispatcher) override;
	};

std::string ToLowerStr(std::string str)
{
	transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

std::chrono::high_resolution_clock::time_point tlastf = std::chrono::high_resolution_clock::now(), tcurrentf = std::chrono::high_resolution_clock::now(), hvlast = std::chrono::high_resolution_clock::now(), hvcurrent = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> time_span, tdif;

void GetRefreshRate() {
	HDC hDCScreen = GetDC(NULL);
	int RefreshFequency = GetDeviceCaps(hDCScreen, VREFRESH);
	ReleaseDC(NULL, hDCScreen);
	if (RefreshFequency <= 90) {
		iPresentIntervalDuringLockpicking = 1;
	}
	if (RefreshFequency >= 90 && RefreshFequency < 150) {
		iPresentIntervalDuringLockpicking = 2;
	}
	if (RefreshFequency >= 150 && RefreshFequency < 210) {
		iPresentIntervalDuringLockpicking = 3;
	}
	if (RefreshFequency >= 210) {
		iPresentIntervalDuringLockpicking = 4;
	}
}

void getinisettings() {
	char buf[256];
	std::string value;
	std::ostringstream os;

	os << "\nVSync (in Game): ";

	//VSync
	GetPrivateProfileString("Main", "EnableVSync", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		vsync = true;
		os << "enabled";
	}
	else {
		vsync = false;
		os << "disabled";
	}

	os << "\nProcessWindowGhosting: ";

	//Ghosting
	GetPrivateProfileString("Main", "DisableProcessWindowsGhosting", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		DisableProcessWindowsGhosting();
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nVSync (in Loading Screens): ";

	//Load Acceleration
	GetPrivateProfileString("Main", "DisableVSyncWhileLoading", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true" && !DisableAnimationOnLoadingScreens) {
		accelerateLoading = true;
		os << "disabled";
	}
	else {
		accelerateLoading = false;
		os << "enabled";
	}

	os << "\nLoading screens: ";

	//Loading screen with 3D model
	GetPrivateProfileString("Main", "DisableBlackLoadingScreens", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		DisableBlackLoadingScreens = true;
		os << "without black screens";
	}
	else {
		DisableBlackLoadingScreens = false;
		os << "default";
	}

	os << "\nDisable animation on loading screens: ";

	//Disable loading screens
	GetPrivateProfileString("Main", "DisableAnimationOnLoadingScreens", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		DisableAnimationOnLoadingScreens = true;
		os << "enabled";
	}
	else {
		DisableAnimationOnLoadingScreens = false;
		os << "disabled";
	}

	//Limit threads on loading screens
	NumOfThreadsWhileLoading = GetPrivateProfileIntA("Main", "NumOfThreadsWhileLoading", 3, INI_FILE);
	os << "\nNum of threads on loading screens: ";
	if (NumOfThreadsWhileLoading > 0 && !DisableAnimationOnLoadingScreens && !DisableBlackLoadingScreens) {
		os << NumOfThreadsWhileLoading;
		LimitCPUthreads = true;
	}
	else {
		LimitCPUthreads = false;
		os << "default";
	}

	//Limiters
	GetPrivateProfileString("Limiter", "FPSLimit", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	os << "\nFPS limit: ";
	if (value == "true") {
		os << "enabled";
		fpslimit = true;
	}
	else {
		os << "disabled";
	}

	int ingamefps = GetPrivateProfileIntA("Limiter", "InGameFPS", 0, INI_FILE);
	int loadscreenfps = GetPrivateProfileIntA("Limiter", "LoadingScreenFPS", 0, INI_FILE);
	os << "\nLimit(ingame): ";
	if (ingamefps > 0 && fpslimit) {
		limitgame = true;
		fpslimitgame = 1.0 / ingamefps;
		os << ingamefps << "FPS / " << fpslimitgame * 1000.0 << "ms";
		if (vsync) {
			vsync = false;
			os << " (VSync will be disabled), ";
		}
	}
	else os << "disabled, ";

	os << " Limit(loading): ";
	if (accelerateLoading) {
		if (loadscreenfps > 0 && fpslimit) {
			limitload = true;
			fpslimitload = 1.0 / loadscreenfps;
			os << loadscreenfps << "FPS / " << fpslimitload * 1000.0 << "ms";
		}
		else {
			os << "disabled";
		}
	}
	else os << "disabled";

	//Fix mouse sensitivity during lockpicking
	int LockpickingFPS = GetPrivateProfileIntA("Limiter", "LockpickingFPS", 0, INI_FILE);
	os << "\nLimit lockpicking: ";
	if (LockpickingFPS > 0) {
		FixLockpickingSound = true;
		fpslockpicking = 1.0 / LockpickingFPS;
		os << LockpickingFPS << "FPS / " << fpslockpicking * 1000.0 << "ms";
	}
	else {
		FixLockpickingSound = false;
		os << "disabled";
	}

	os << "\nUntie game speed from framerate: ";

	//Fix game speed
	GetPrivateProfileString("Fixes", "UntieSpeedFromFPS", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		UntieSpeedFromFPS = true;
		os << "enabled";
	}
	else {
		UntieSpeedFromFPS = false;
		os << "disabled";
	}

	os << "\nJump height fix: ";

	//Fix Jump
	GetPrivateProfileString("Fixes", "JumpFix", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		JumpFix = true;
		os << "enabled";
	}
	else {
		JumpFix = false;
		os << "disabled";
	}

	os << "\nFix rotation speed: ";

	//Fix rotation speed
	GetPrivateProfileString("Fixes", "FixRotationSpeed", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixRotationSpeed = true;
		os << "enabled";
	}
	else {
		FixRotationSpeed = false;
		os << "disabled";
	}

	os << "\nFix workshop rotation speed: ";

	//Fix workshop rotation speed
	GetPrivateProfileString("Fixes", "FixWorkshopRotationSpeed", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixWorkshopRotationSpeed = true;
		os << "enabled";
	}
	else {
		FixWorkshopRotationSpeed = false;
		os << "disabled";
	}

	os << "\nFix stuck animation: ";

	//Fix stuck animation
	GetPrivateProfileString("Fixes", "FixStuckAnimation", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixStuckAnimation = true;
		os << "enabled";
	}
	else {
		FixStuckAnimation = false;
		os << "disabled";
	}

	//Limit threads on loading screens
	iPresentIntervalDuringLockpicking = GetPrivateProfileIntA("Fixes", "iPresentIntervalDuringLockpicking", 3, INI_FILE);
	os << "\niPresentInterval during lockpicking: ";
	if (iPresentIntervalDuringLockpicking >= 0) {
		os << iPresentIntervalDuringLockpicking;
		LimitLockpicking = true;
		if (iPresentIntervalDuringLockpicking == 0) {
			GetRefreshRate();
		}
	}
	else {
		LimitLockpicking = false;
		os << "default";
	}
	_MESSAGE(os.str().c_str());
}

void updatemaxtime() {
	if (!PluginLoadCompleted) {
		_MESSAGE("Plugin load completed!");
		PluginLoadCompleted = true;
	}
	newvalue = tdif.count();
	if (FixRotationSpeed) {
		newvalue4 = newvalue * 3;  //RotationSpeed
		newvalue5 = newvalue / 0.1666;  //fFirstPersonSittingRotationSpeedX
		newvalue6 = newvalue5 / 2;  //fFirstPersonSittingRotationSpeedY
		SafeWriteBuf(RotationSpeedAddress.GetUIntPtr(), &newvalue4, sizeof(float));
		SafeWriteBuf(RotationSpeedXAddress.GetUIntPtr(), &newvalue5, sizeof(float));
		SafeWriteBuf(RotationSpeedYAddress.GetUIntPtr(), &newvalue6, sizeof(float));
	}
	if (FixWorkshopRotationSpeed) {
		newvalue2 = newvalue * 1.0472;
		SafeWriteBuf(WorkshopRotationSpeedAddress.GetUIntPtr(), &newvalue2, sizeof(float));
	}
}

typedef HRESULT(__stdcall* D3D11PresentHook) (IDXGISwapChain* opSwapChain, UINT SyncInterval, UINT Flags);
D3D11PresentHook phookD3D11Present = NULL;

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (isLoading == 1 && isLockpicking == 0) {
		if (limitload) {
			time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tcurrentf - tlastf);
			while (time_span.count() < fpslimitload) {
				tcurrentf = std::chrono::high_resolution_clock::now();
				time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tcurrentf - tlastf);
			}
			tlastf = std::chrono::high_resolution_clock::now();
		}
	}
	else {
		if (limitgame && isLockpicking == 0) {
			time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tcurrentf - tlastf);
			while (time_span.count() < fpslimitgame) {
				tcurrentf = std::chrono::high_resolution_clock::now();
				time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tcurrentf - tlastf);
			}
			tlastf = std::chrono::high_resolution_clock::now();
		}
	}
	updatemaxtime();
	if (isLockpicking == 1) {
		time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tcurrentf - tlastf);
		while (time_span.count() < fpslockpicking) {
			tcurrentf = std::chrono::high_resolution_clock::now();
			time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tcurrentf - tlastf);
		}
		tlastf = std::chrono::high_resolution_clock::now();
	}

	hvlast = std::chrono::high_resolution_clock::now();
	tdif = std::chrono::duration_cast<std::chrono::duration<double>>(hvlast - hvcurrent);
	hvcurrent = std::chrono::high_resolution_clock::now();

	if (vsync) {
		if (isLoading == 1 && accelerateLoading) return phookD3D11Present(pSwapChain, 0, Flags);
		return phookD3D11Present(pSwapChain, 1, Flags);
	}
	else {
		if (isLoading == 1 && !accelerateLoading) return phookD3D11Present(pSwapChain, 1, Flags);
		return phookD3D11Present(pSwapChain, 0, Flags);
	}
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
	int length = GetWindowTextLength(hWnd);
	char* buffer = new char[length + 1];
	GetWindowText(hWnd, buffer, length + 1);
	if (!strcmp(buffer, "Fallout4")) {
		GetProcess(hWnd);
	}
	return TRUE;
}

DWORD_PTR GetProcessBaseAddress(DWORD processID)
{
	DWORD_PTR   baseAddress = 0;
	HANDLE      processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	HMODULE* moduleArray;
	LPBYTE      moduleArrayBytes;
	DWORD       bytesRequired;

	if (processHandle)
	{
		if (EnumProcessModules(processHandle, NULL, 0, &bytesRequired))
		{
			if (bytesRequired)
			{
				moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

				if (moduleArrayBytes)
				{
					unsigned int moduleCount;

					moduleCount = bytesRequired / sizeof(HMODULE);
					moduleArray = (HMODULE*)moduleArrayBytes;

					if (EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired))
					{
						baseAddress = (DWORD_PTR)moduleArray[0];
					}

					LocalFree(moduleArrayBytes);
				}
			}
		}

		CloseHandle(processHandle);
	}

	return baseAddress;
}

bool GetProcess(HWND hWnd) {
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	if (!pid) return false;
	DWORD_PTR baseadd = GetProcessBaseAddress(pid);
	if (!baseadd) return false;
	f4handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!f4handle) return false;
	TimeAddress = baseadd + 0x5B5B6D0;
	return true;
}

void GetProcId() {
	EnumWindows(enumWindowCallback, NULL);
	nMaxProcessorMask = (1 << NumOfThreadsWhileLoading) - 1;
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	nMaxProcessorAfterLoad = (1 << SystemInfo.dwNumberOfProcessors) - 1;
}
//Create DeviceAndSwapChain
bool CreateDeviceAndSwapChain() {
	while (FindWindowA("Fallout4", NULL) == NULL) {
		Sleep(10);
	}
	_MESSAGE("\nPreparing D3D Hook");
	GetProcId();
	IDXGISwapChain* pSwapChain;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GetDesktopWindow();
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;//((GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? FALSE : TRUE;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &sd, &pSwapChain, NULL, NULL, NULL)))
	{
		_MESSAGE("Failed to CreateDeviceAndSwapChain");
		return false;
	}
	DWORD_PTR* pVTable = (DWORD_PTR*)pSwapChain;
	pVTable = (DWORD_PTR*)pVTable[0];
	if (MH_Initialize() == MH_OK) {
		//Present Hook
		if (MH_CreateHook((DWORD_PTR*)pVTable[8], hookD3D11Present, reinterpret_cast<void**>(&phookD3D11Present)) == MH_OK) {
			if (MH_EnableHook((DWORD_PTR*)pVTable[8]) == MH_OK) {
				_MESSAGE("Function Hooked!");
			}
			else {
				_MESSAGE("Error: Failed to hook Present!");
				return false;
			}
		}
		else {
			_MESSAGE("Error: Failed to hook Present!");
			return false;
		}
	}
	else {
		_MESSAGE("Error: Could not initialize MinHook!");
		return false;
	}
	return true;
}

void SetThreads(void* unk1, void* unk2, void* unk3, void* unk4) {
	if (isLoading == 1) {
		SetProcessAffinityMask(f4handle, nMaxProcessorMask);
		isLimiting = 1;
	}
	ChangeThreads_Original(unk1, unk2, unk3, unk4);
}

void UpdateRotationValue(void* unk1, void* unk2, void* unk3, void* unk4) {
	ReadProcessMemory(f4handle, (PVOID*)TimeAddress, &fTime, sizeof(float), 0);
	newvalue4 = fTime * 3;       //RotationSpeed
	SafeWriteBuf(RotationSpeedAddress.GetUIntPtr(), &newvalue4, sizeof(float));
	UpdateValues_Original(unk1, unk2, unk3, unk4);
}

void UpdateSitValues(void* unk1, void* unk2, void* unk3, void* unk4) {
	ReadProcessMemory(f4handle, (PVOID*)TimeAddress, &fTime, sizeof(float), 0);
	newvalue5 = fTime / 0.1666;  //fFirstPersonSittingRotationSpeedX
	newvalue6 = fTime / 0.3332;   //fFirstPersonSittingRotationSpeedY
	SafeWriteBuf(RotationSpeedXAddress.GetUIntPtr(), &newvalue5, sizeof(float));
	SafeWriteBuf(RotationSpeedYAddress.GetUIntPtr(), &newvalue6, sizeof(float));
	UpdateSValues_Original(unk1, unk2, unk3, unk4);
}

void UpdateWValues(void* unk1, void* unk2, void* unk3, void* unk4) {
	ReadProcessMemory(f4handle, (PVOID*)TimeAddress, &fTime, sizeof(float), 0);
	newvalue2 = fTime * 1.0472;
	SafeWriteBuf(WorkshopRotationSpeedAddress.GetUIntPtr(), &newvalue2, sizeof(float));
	UpdateWValues_Original(unk1, unk2, unk3, unk4);
}

void HookFPS() {
	if (!vsync) {
		SafeWriteBuf(RelocAddr<uintptr_t>(0x61E0950).GetUIntPtr(), &iPresentInterval0, sizeof(iPresentInterval0));
	}
	if (FixRotationSpeed) {
		struct UpdateValues_Code : Xbyak::CodeGenerator {
			UpdateValues_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				mov(ptr[rsp + 0x08], rbx);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(RelocAddr<uintptr_t>(0xF49540).GetUIntPtr() + 5);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		UpdateValues_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		UpdateValues_Original = (_UpdateValues)codeBuf;

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0xF49540).GetUIntPtr(), (uintptr_t)UpdateRotationValue);
	}
	if (FixRotationSpeed) { //sitting
		struct UpdateSValues_Code : Xbyak::CodeGenerator {
			UpdateSValues_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				push(rsi);
				push(rdi);
				sub(rsp, 0x28);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(RelocAddr<uintptr_t>(0xF443B0).GetUIntPtr() + 7);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		UpdateSValues_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		UpdateSValues_Original = (_UpdateSValues)codeBuf;

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0xF443B0).GetUIntPtr(), (uintptr_t)UpdateSitValues);
	}
	if (FixWorkshopRotationSpeed) {
		struct UpdateWValues_Code : Xbyak::CodeGenerator {
			UpdateWValues_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				mov(rax, rsp);
				push(rbp);
				push(r14);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(RelocAddr<uintptr_t>(0xBEFE80).GetUIntPtr() + 6);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		UpdateWValues_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		UpdateWValues_Original = (_UpdateWValues)codeBuf;

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0xBEFE80).GetUIntPtr(), (uintptr_t)UpdateWValues);
	}
	_MESSAGE("\nPlugin load completed!");
	PluginLoadCompleted2 = true;
}
//Detect loading screen and lockpicking menu
//clock_t start, end;
EventResult	MenuOpenCloseHandler::ReceiveEvent(MenuOpenCloseEvent* evn, void* dispatcher)
{
	if (!_strcmpi("LoadingMenu", evn->menuName.c_str())\
		)
	{
		if (evn->isOpen)
		{
			//start = clock();
			isLoading = 1;
			if (accelerateLoading) {
				SafeWriteBuf(RelocAddr<uintptr_t>(0x61E0950).GetUIntPtr(), &iPresentInterval0, sizeof(iPresentInterval0));
			}
		}
		else
		{
			isLoading = 0;
			//end = clock();
			//_MESSAGE("Loading time: %.0f second(s)", ((double)end - start) / ((double)CLOCKS_PER_SEC));
			if (isLimiting == 1) {
				SetProcessAffinityMask(f4handle, nMaxProcessorAfterLoad);
				isLimiting = 0;
			}
			if (!vsync) {
				SafeWriteBuf(RelocAddr<uintptr_t>(0x61E0950).GetUIntPtr(), &iPresentInterval0, sizeof(iPresentInterval0));
			}
			else {
				SafeWriteBuf(RelocAddr<uintptr_t>(0x61E0950).GetUIntPtr(), &iPresentInterval1, sizeof(iPresentInterval1));
			}
		}
	}
	if (!_strcmpi("LockpickingMenu", evn->menuName.c_str())\
		)
	{
		if (evn->isOpen)
		{
			if (FixLockpickingSound) {
				isLockpicking = 1;
			}
			if (LimitLockpicking) {
				SafeWriteBuf(RelocAddr<uintptr_t>(0x61E0950).GetUIntPtr(), &iPresentIntervalDuringLockpicking, sizeof(iPresentIntervalDuringLockpicking));
			}
		}
		else
		{
			isLockpicking = 0;
			if (!vsync) {
				SafeWriteBuf(RelocAddr<uintptr_t>(0x61E0950).GetUIntPtr(), &iPresentInterval0, sizeof(iPresentInterval0));
			}
			else {
				SafeWriteBuf(RelocAddr<uintptr_t>(0x61E0950).GetUIntPtr(), &iPresentInterval1, sizeof(iPresentInterval1));
			}
		}
	}
	return kEvent_Continue;
}

void LimitCPU() {
	if (LimitCPUthreads) {
		struct ChangeThreads_Code : Xbyak::CodeGenerator {
			ChangeThreads_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				mov(ptr[rsp + 0x10], rbx);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(RelocAddr<uintptr_t>(0x7FA780).GetUIntPtr() + 5);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		ChangeThreads_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		ChangeThreads_Original = (_ChangeThreads)codeBuf;

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x7FA780).GetUIntPtr(), (uintptr_t)SetThreads);
	}
}

void onF4SEMessage(F4SEMessagingInterface::Message* msg) {
	switch (msg->type) {
	case F4SEMessagingInterface::kMessage_GameDataReady:
		static auto pMenuOpenCloseHandler = new MenuOpenCloseHandler();
		(*g_ui)->menuOpenCloseEventSource.AddEventSink(pMenuOpenCloseHandler);
		if (firstload) {
			firstload = false;
			if (FixRotationSpeed) {
				//fix rotation speed
				unsigned char data2[] = { 0xF3, 0x0F, 0x59, 0x15, 0x14, 0xD1, 0xCC, 0x01 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0xF49645).GetUIntPtr(), &data2, sizeof(data2));
				unsigned char data4[] = { 0xF3, 0x0F, 0x10, 0x05, 0xDC, 0x00, 0x00, 0x00 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x12446C8).GetUIntPtr(), &data4, sizeof(data4));
				unsigned char data5[] = { 0xF3, 0x0F, 0x10, 0x0D, 0x69, 0x01, 0x00, 0x00 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x12446DF).GetUIntPtr(), &data5, sizeof(data5));
				//fix mouse rotation speed
				unsigned char data12[] = { 0xF3, 0x0F, 0x59, 0x0D, 0x10, 0x9D, 0x97, 0x01 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x129CA32).GetUIntPtr(), &data12, sizeof(data12));
				unsigned char fPickMouseRotationSpeed[] = { 0x96, 0x43, 0x8B, 0x3C, 0x00 }; //0.017
				SafeWriteBuf(RelocAddr<uintptr_t>(0x2C1674A).GetUIntPtr(), fPickMouseRotationSpeed, sizeof(fPickMouseRotationSpeed));
				//fix rotation on loading screen
				unsigned char data13[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x1298EA3).GetUIntPtr(), &data13, sizeof(data13));
			}
			if (FixWorkshopRotationSpeed) {
				unsigned char data3[] = { 0xF3, 0x0F, 0x59, 0x0D, 0xD7, 0xE4, 0x9F, 0x02 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x2182B2).GetUIntPtr(), &data3, sizeof(data3));
			}
			//fix loading rotation
			if (FixStuckAnimation) {
				unsigned char s1[] = { 0x53 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x24EC8BF).GetUIntPtr(), &s1, sizeof(s1));
				unsigned char s2[] = { 0xF3, 0x0F, 0x10, 0x05, 0x82, 0x9E, 0x72, 0x00 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x24EC8C0).GetUIntPtr(), &s2, sizeof(s2));
				unsigned char s3[] = { 0xE8, 0xF3, 0x43, 0x00, 0x00 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x24EC8C8).GetUIntPtr(), &s3, sizeof(s3));
				unsigned char s4[] = { 0x5B };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x24EC8CD).GetUIntPtr(), &s4, sizeof(s4));
				unsigned char s5[] = { 0xC3 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x24EC8CE).GetUIntPtr(), &s5, sizeof(s5));
				unsigned char s6[] = { 0xE8, 0xE4, 0x02, 0x00, 0x00 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x24EC5D6).GetUIntPtr(), &s6, sizeof(s6));
				unsigned char s7[] = { 0xF3, 0x0F, 0x11, 0x41, 0xF0 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x24F0CDC).GetUIntPtr(), &s7, sizeof(s7));
				//fix moving objects
				unsigned char data16[] = { 0x0F, 0x2E, 0x05, 0x5B, 0x98, 0x36, 0x02 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x14BEC1E).GetUIntPtr(), &data16, sizeof(data16));
			}
			if (JumpFix) {
				//fix jumping
				unsigned char data17[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
				SafeWriteBuf(RelocAddr<uintptr_t>(0x1E15375).GetUIntPtr(), &data17, sizeof(data17));
			}
			if (vsync) {
				SafeWriteBuf(RelocAddr<uintptr_t>(0x61E0950).GetUIntPtr(), &iPresentInterval1, sizeof(iPresentInterval1));
			}
			// Set the max processor mask
			if (!fpslimit) {
				GetProcId();
				HookFPS();
			}
			else {
				if (!CreateDeviceAndSwapChain()) {
					HookFPS();
				}
			}
		}
		break;
	}
}

extern "C"
{
	bool F4SEPlugin_Query(const F4SEInterface* f4se, PluginInfo* info)
	{
		std::string logPath = "\\My Games\\Fallout4\\F4SE\\";
		logPath += PLUGIN_NAME_SHORT;
		logPath += ".log";
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, logPath.c_str());

		_MESSAGE("%s v%s", PLUGIN_NAME_SHORT, PLUGIN_VERSION_STRING);

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = PLUGIN_NAME_SHORT;
		info->version = PLUGIN_VERSION;

		g_pluginHandle = f4se->GetPluginHandle();

		if (f4se->runtimeVersion > SUPPORTED_RUNTIME_VERSION) {
			_MESSAGE("INFO: Newer game version (%08X) than target (%08X).", f4se->runtimeVersion, SUPPORTED_RUNTIME_VERSION);
		}

		//Get messaging interface
		g_messaging = (F4SEMessagingInterface*)f4se->QueryInterface(kInterface_Messaging);
		if (!g_messaging) {
			_MESSAGE("couldn't get messaging interface");
			return false;
		}

		return true;
	}
	bool F4SEPlugin_Load(const F4SEInterface* f4se) {
		//Get Ini Settings
		getinisettings();
		//Game speed fix
		if (UntieSpeedFromFPS) {
			unsigned char data1[] = { 0xBA, 0x00, 0x00, 0x00, 0x00 };
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1B1393A).GetUIntPtr(), &data1, sizeof(data1));
		}
		//Disable animation on loading screens
		if (DisableAnimationOnLoadingScreens) {
			unsigned char data20[] = { 0x90, 0x90, 0x90, 0x90 };
			SafeWriteBuf(RelocAddr<uintptr_t>(0xCBFFCD).GetUIntPtr(), &data20, sizeof(data20));
		}
		if (!g_branchTrampoline.Create(1024 * 64))
		{
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!g_localTrampoline.Create(1024 * 64, nullptr))
		{
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}
		LimitCPU();
		if (DisableBlackLoadingScreens) {
			unsigned char data15[] = { 0xEB, 0x1E };
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1297076).GetUIntPtr(), &data15, sizeof(data15));
		}
		g_messaging->RegisterListener(g_pluginHandle, "F4SE", onF4SEMessage);
		return true;
	}
};
