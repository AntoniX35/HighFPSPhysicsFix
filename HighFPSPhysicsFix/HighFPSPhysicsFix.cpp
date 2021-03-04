#include "stdafx.h"
//Ini File
#define INI_FILE "data\\F4SE\\Plugins\\HighFPSPhysicsFix.ini"

float f = 0.017;
PerfCounter PerfCounter::m_Instance;
long long PerfCounter::perf_freq;
float PerfCounter::perf_freqf;

RelocAddr<uintptr_t> PostloadingMenuSpeedAddress(0x126DAEB);
RelocAddr<uintptr_t> SittingRotationSpeedXAddress(0x3804738);
RelocAddr<uintptr_t> SittingRotationSpeedYAddress(0x3804750);
RelocAddr<uintptr_t> FixCPUThreadsAddress(0xE9B7E1);
RelocAddr<uintptr_t> BethesdaVsyncAddress(0x1D17792);
RelocAddr<uintptr_t> BethesdaFPSCap1Address(0xD423BA);
RelocAddr<uintptr_t> BethesdaFPSCap2Address(0xD423C3);
RelocAddr<uintptr_t> VsyncAddress(0x61E0950);

long long CurrentFPS, FPSui, loadingFPSmax, lockpickingFPSmax, timing;
RelocAddr<uintptr_t> FPSLimiter(0x1D0B67E);

bool limit = false;
float fpslimitgame, fpslimitload, fpslockpicking, SittingRotSpeedX, SittingRotSpeedY, PostloadingMenuSpeed;
int isLockpicking, NumOfThreadsWhileLoading, NumOfThreadsWhileLoadingNewGame, fpslimit, isLimiting, SwapBufferCount, DXGISwapEffect, ScalingMode;
unsigned int nMaxProcessorMask;
unsigned int nMaxProcessorMaskNG;
unsigned int nMaxProcessorAfterLoad;
HANDLE f4handle = NULL;
double maxft, minft;
bool Fullscreen, FixLockpickingSound, accelerateLoading, UntieSpeedFromFPS, DisableiFPSClamp, FixStuttering, FixWorkshopRotationSpeed, FixRotationSpeed, FixSittingRotationSpeed, FixStuckAnimation, limitload, limitgame, LimitCPUThreadsNG, DisableAnimationOnLoadingScreens, DisableBlackLoadingScreens, FixWindSpeed, FixWhiteScreen, FixLoadingModel, ReduceAfterLoading, FixCPUThreads, WriteLoadingTime, Vsync, Tearing, ResizeBuffersDisable, ResizeTargetDisable, Borderless, FixMotionResponsive;
bool firstload = true;
F4SEScaleformInterface* g_scaleform = NULL;
F4SEMessagingInterface* g_messaging = NULL;
PluginHandle			    g_pluginHandle = kPluginHandle_Invalid;

int PresentInterval0 = 0;
int PresentInterval1 = 1;

static F4SEPapyrusInterface* g_papyrus = NULL;

float GetPrivateProfileFloat(LPCSTR lpAppName, LPCSTR lpKeyName, FLOAT flDefault, LPCSTR lpFileName)
{
	char szData[32];

	GetPrivateProfileStringA(lpAppName, lpKeyName, std::to_string(flDefault).c_str(), szData, 32, lpFileName);

	return (float)atof(szData);
}

typedef void(*_ChangeThreads)(void* unk1, void* unk2, void* unk3, void* unk4);
_ChangeThreads ChangeThreads_Original = nullptr;
typedef void(*_ChangeThreadsNewGame)(void* unk1, void* unk2, void* unk3, void* unk4);
_ChangeThreadsNewGame ChangeThreadsNewGame_Original = nullptr;
typedef void(*_ReturnThreadsNewGame)(void* unk1, void* unk2, void* unk3, void* unk4);
_ReturnThreadsNewGame ReturnThreadsNewGame_Original = nullptr;

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

void getinisettings() {
	char buf[256];
	std::string value;
	std::ostringstream os;

	os << "\nFull screen: ";

	GetPrivateProfileString("Display", "FullScreen", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		Fullscreen = true;
		os << "enabled";
	}
	else {
		Fullscreen = false;
		os << "disabled";
	}

	os << "\nBorderless screen: ";

	GetPrivateProfileString("Display", "Borderless", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		Borderless = true;
		os << "enabled";
	}
	else {
		Borderless = false;
		os << "disabled";
	}

	os << "\nVSync (in Game): ";

	//VSync
	GetPrivateProfileString("Display", "EnableVSync", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		Vsync = true;
		os << "enabled";
	}
	else {
		Vsync = false;
		os << "disabled";
	}

	os << "\nVSync (in Loading Screens): ";

	//Load Acceleration
	GetPrivateProfileString("Display", "DisableVSyncWhileLoading", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true" && !DisableAnimationOnLoadingScreens) {
		accelerateLoading = true;
		os << "disabled";
	}
	else {
		accelerateLoading = false;
		os << "enabled";
	}

	os << "\nNumber of buffers in the swap chain: ";

	SwapBufferCount = GetPrivateProfileIntA("Display", "BufferCount", 0, INI_FILE);
	if (SwapBufferCount == 0) {
		os << "auto";
	}
	else {
		os << SwapBufferCount;
	}

	os << "\nResize buffers: ";

	GetPrivateProfileString("Display", "ResizeBuffersDisable", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		ResizeBuffersDisable = true;
		os << "enabled";
	}
	else {
		ResizeBuffersDisable = false;
		os << "disabled";
	}

	os << "\nResize target: ";

	GetPrivateProfileString("Display", "ResizeTargetDisable", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		ResizeTargetDisable = true;
		os << "enabled";
	}
	else {
		ResizeTargetDisable = false;
		os << "disabled";
	}
	
	os << "\nDXGI swap effect: ";

	DXGISwapEffect = GetPrivateProfileIntA("Display", "SwapEffect", 0, INI_FILE);
	if (DXGISwapEffect == 0) {
		os << "auto";
	}
	if (DXGISwapEffect == 1) {
		DXGI_SWAP_EFFECT nse = DXGI_SWAP_EFFECT_DISCARD;
		DXGI_SWAP_EFFECT se = DXGI_SWAP_EFFECT_DISCARD;
		os << "discard";
	}
	if (DXGISwapEffect == 2) {
		DXGI_SWAP_EFFECT nse = DXGI_SWAP_EFFECT_SEQUENTIAL;
		DXGI_SWAP_EFFECT se = DXGI_SWAP_EFFECT_SEQUENTIAL;
		os << "sequential";
	}
	if (DXGISwapEffect == 3) {
		DXGI_SWAP_EFFECT nse = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		DXGI_SWAP_EFFECT se = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		os << "flip_sequential";
	}
	if (DXGISwapEffect == 4) {
		DXGI_SWAP_EFFECT nse = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		DXGI_SWAP_EFFECT se = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		os << "flip_discard";
	}

	os << "\nAllow tearing: ";

	GetPrivateProfileString("Display", "AllowTearing", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		Tearing = true;
		os << "enabled";
	}
	else {
		Tearing = false;
		os << "disabled";
	}

	os << "\nDXGI mode scalling: ";

	ScalingMode = GetPrivateProfileIntA("Display", "ScalingMode", 0, INI_FILE);
	if (ScalingMode == 1) {
		os << "unspecified";
	}
	if (ScalingMode == 2) {
		os << "centered";
	}
	if (ScalingMode == 3) {
		os << "stretched";
	}

	os << "\nProcessWindowGhosting: ";

	//Ghosting
	GetPrivateProfileString("Display", "DisableProcessWindowsGhosting", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		DisableProcessWindowsGhosting();
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nUntie game speed from framerate: ";

	//Fix game speed
	GetPrivateProfileString("Main", "UntieSpeedFromFPS", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		UntieSpeedFromFPS = true;
		os << "enabled";
	}
	else {
		UntieSpeedFromFPS = false;
		os << "disabled";
	}

	os << "\nDisable iFPSClamp: ";

	//Disable iFPSClamp
	GetPrivateProfileString("Main", "DisableiFPSClamp", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		DisableiFPSClamp = true;
		os << "enabled";
	}
	else {
		DisableiFPSClamp = false;
		os << "disabled";
	}

	//Limiters

	float ingamefps = GetPrivateProfileFloat("Limiter", "InGameFPS", 0, INI_FILE);
	os << "\nLimit(ingame): ";
	if (ingamefps > 0) {
		limitgame = true;
		limit = true;
		fpslimitgame = 1.0 / ingamefps;
		os << ingamefps << "FPS / " << fpslimitgame * 1000.0 << "ms";
	}
	else {
		os << "disabled, ";
	}

	float loadscreenfps = GetPrivateProfileFloat("Limiter", "LoadingScreenFPS", 0, INI_FILE);
	os << " Limit(loading): ";
	if (accelerateLoading) {
		if (loadscreenfps > 0) {
			limitload = true;
			limit = true;
			fpslimitload = 1.0 / loadscreenfps;
			os << loadscreenfps << "FPS / " << fpslimitload * 1000.0 << "ms";
		}
		else {
			os << "disabled";
		}
	}
	else os << "disabled";

	//Fix mouse sensitivity during lockpicking
	float LockpickingFPS = GetPrivateProfileFloat("Limiter", "LockpickingFPS", 0, INI_FILE);
	os << "\nLimit lockpicking: ";
	if (LockpickingFPS > 0) {
		FixLockpickingSound = true;
		limit = true;
		fpslockpicking = 1.0 / LockpickingFPS;
		os << LockpickingFPS << "FPS / " << fpslockpicking * 1000.0 << "ms";
	}
	else {
		FixLockpickingSound = false;
		os << "disabled";
	}

	//Limit threads on loading screens (starting a new game)
	NumOfThreadsWhileLoadingNewGame = GetPrivateProfileIntA("Limiter", "NumOfThreadsWhileLoadingNewGame", 1, INI_FILE);
	os << "\nNum of threads on loading screens (new game): ";
	if (NumOfThreadsWhileLoadingNewGame > 0) {
		os << NumOfThreadsWhileLoadingNewGame;
		LimitCPUThreadsNG = true;
	}
	else {
		LimitCPUThreadsNG = false;
		os << "default";
	}

	os << "\nLoading screens: ";

	//Loading screen with 3D model
	GetPrivateProfileString("Limiter", "DisableBlackLoadingScreens", "false", buf, sizeof(buf), INI_FILE);
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
	GetPrivateProfileString("Limiter", "DisableAnimationOnLoadingScreens", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		DisableAnimationOnLoadingScreens = true;
		os << "enabled";
	}
	else {
		DisableAnimationOnLoadingScreens = false;
		os << "disabled";
	}

	os << "\nPostloading menu speed: ";

	//Postloading menu speed
	PostloadingMenuSpeed = GetPrivateProfileFloat("Limiter", "PostloadingMenuSpeed", 1, INI_FILE);
	if (PostloadingMenuSpeed != 1) {
		os << PostloadingMenuSpeed;
		ReduceAfterLoading = true;
	}
	else {
		ReduceAfterLoading = false;
		os << "default";
	}

	os << "\nFix CPU threads: ";

	//Fix CPU threads
	GetPrivateProfileString("Fixes", "FixCPUThreads", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixCPUThreads = true;
		os << "enabled";
	}
	else {
		FixCPUThreads = false;
		os << "disabled";
	}

	os << "\nWrite loading time to log: ";

	//Write loading time to log
	GetPrivateProfileString("Fixes", "WriteLoadingTime", "false", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		os << "enabled";
		WriteLoadingTime = true;
	}
	else {
		WriteLoadingTime = false;
		os << "disabled";
	}

	os << "\nFix stuttering: ";

	//Fix game stuttering
	GetPrivateProfileString("Fixes", "FixStuttering", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixStuttering = true;
		os << "enabled";
	}
	else {
		FixStuttering = false;
		os << "disabled";
	}

	os << "\nFix white screen: ";

	//Fix white screen
	GetPrivateProfileString("Fixes", "FixWhiteScreen", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixWhiteScreen = true;
		os << "enabled";
	}
	else {
		FixWhiteScreen = false;
		os << "disabled";
	}

	os << "\nFix wind speed: ";

	//Fix wind speed
	GetPrivateProfileString("Fixes", "FixWindSpeed", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixWindSpeed = true;
		os << "enabled";
	}
	else {
		FixWindSpeed = false;
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

	os << "\nFix sitting rotation speed: ";

	//Fix rotation speed
	GetPrivateProfileString("Fixes", "FixSittingRotationSpeed", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixSittingRotationSpeed = true;
		os << "enabled";
	}
	else {
		FixSittingRotationSpeed = false;
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

	os << "\nFix loading model: ";

	//Fix workshop rotation speed
	GetPrivateProfileString("Fixes", "FixLoadingModel", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixLoadingModel = true;
		os << "enabled";
	}
	else {
		FixLoadingModel = false;
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

	os << "\nFix motion responsive: ";

	GetPrivateProfileString("Fixes", "FixMotionResponsive", "true", buf, sizeof(buf), INI_FILE);
	value = ToLowerStr(buf);
	if (value == "true") {
		FixMotionResponsive = true;
		os << "enabled\n";
	}
	else {
		FixMotionResponsive = false;
		os << "disabled\n";
	}
	_MESSAGE(os.str().c_str());
}

void GetNumberOfThreads() {
	nMaxProcessorMaskNG = (1 << NumOfThreadsWhileLoadingNewGame) - 1;
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	nMaxProcessorAfterLoad = (1 << SystemInfo.dwNumberOfProcessors) - 1;
	f4handle = GetCurrentProcess();
}

void SetThreadsNewGame(void* unk1, void* unk2, void* unk3, void* unk4) {
	SetProcessAffinityMask(f4handle, nMaxProcessorMaskNG);
	ChangeThreadsNewGame_Original(unk1, unk2, unk3, unk4);
}

void ReturnThreadsNewGame(void* unk1, void* unk2, void* unk3, void* unk4) {
	SetProcessAffinityMask(f4handle, nMaxProcessorAfterLoad);
	ReturnThreadsNewGame_Original(unk1, unk2, unk3, unk4);
}

void HookFPS() {
	if (DisableiFPSClamp) {
		float clamp = 0;
		SafeWriteBuf(RelocAddr<uintptr_t>(0x5B5B6C8).GetUIntPtr(), &clamp, sizeof(float));
	}
	if (FixSittingRotationSpeed) { 
		//fix sitting rotation
		SittingRotSpeedX = GetINISetting("fFirstPersonSittingRotationSpeedX:Camera")->data.f32 / f;
		SittingRotSpeedY = GetINISetting("fFirstPersonSittingRotationSpeedY:Camera")->data.f32 / f;
		SafeWriteBuf(SittingRotationSpeedXAddress.GetUIntPtr(), &SittingRotSpeedX, sizeof(float));
		SafeWriteBuf(SittingRotationSpeedYAddress.GetUIntPtr(), &SittingRotSpeedY, sizeof(float));
		//x
		SafeWriteBuf(RelocAddr<uintptr_t>(0x124603D).GetUIntPtr(), "\xF3\x0F\x59\x05\x8F\x56\x91\x04\xF3\x0F\x59\x40\x4C\xE9\x86\xE6\xFF\xFF", 18);
		SafeWriteBuf(RelocAddr<uintptr_t>(0x12446D0).GetUIntPtr(), "\xE9\x68\x19\x00\x00", 5);
		//y
		SafeWriteBuf(RelocAddr<uintptr_t>(0x124764C).GetUIntPtr(), "\xF3\x0F\x59\x0D\x80\x40\x91\x04\xF3\x0F\x10\x43\x64\xE9\x8E\xD0\xFF\xFF", 18);
		SafeWriteBuf(RelocAddr<uintptr_t>(0x12446E7).GetUIntPtr(), "\xE9\x60\x2F\x00\x00", 5);
	}
}

clock_t start, end;

//Detect loading screen and lockpicking menu
EventResult	MenuOpenCloseHandler::ReceiveEvent(MenuOpenCloseEvent* evn, void* dispatcher)
{
	if (!_strcmpi("LoadingMenu", evn->menuName.c_str())\
		)
	{
		if (evn->isOpen)
		{
			if (WriteLoadingTime) {
				start = clock();
			}
			CurrentFPS = loadingFPSmax;
			if (accelerateLoading) {
				if (Vsync) {
					SafeWriteBuf(VsyncAddress.GetUIntPtr(), &PresentInterval0, sizeof(PresentInterval0));
					SetFPSLimitOverride();
				}
			}
		}
		else
		{
			CurrentFPS = FPSui;
			if (accelerateLoading) {
				if (Vsync) {
					SafeWriteBuf(VsyncAddress.GetUIntPtr(), &PresentInterval1, sizeof(PresentInterval1));
					ResetFPSLimitOverride();
				}
			}
			if (WriteLoadingTime) {
				end = clock();
				_MESSAGE("Loading time: %.2f second(s)", ((double)end - start) / ((double)CLOCKS_PER_SEC));
			}
			if (isLimiting == 1) {
				SetProcessAffinityMask(f4handle, nMaxProcessorAfterLoad);
				isLimiting = 0;
			}
		}
	}
	if (!_strcmpi("LockpickingMenu", evn->menuName.c_str())\
		)
	{
		if (evn->isOpen)
		{
			if (FixLockpickingSound) {
				CurrentFPS = lockpickingFPSmax;
				isLockpicking = 1;
			}
		}
		else
		{
			if (FixLockpickingSound) {
				CurrentFPS = FPSui;
				isLockpicking = 0;
			}
		}
	}
	return kEvent_Continue;
}

void LimitCPUNewGame() {
		struct ChangeThreadsNewGame_Code : Xbyak::CodeGenerator {
			ChangeThreadsNewGame_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				lea(r8,ptr[rsp + 0x40]);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(RelocAddr<uintptr_t>(0x12A23F5).GetUIntPtr() + 5);
			}
		};
		_MESSAGE("Limit CPU threads patching...");
		void* codeBuf = g_localTrampoline.StartAlloc();
		ChangeThreadsNewGame_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		ChangeThreadsNewGame_Original = (_ChangeThreadsNewGame)codeBuf;

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x12A23F5).GetUIntPtr(), (uintptr_t)SetThreadsNewGame);
		_MESSAGE("Limit CPU threads OK");
}

void ReturnThreads() {
		struct ReturnThreadsNewGame_Code : Xbyak::CodeGenerator {
			ReturnThreadsNewGame_Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				lea(r8, ptr[rsp + 0x40]);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(RelocAddr<uintptr_t>(0xD1D4A0).GetUIntPtr() + 6);
			}
		};
		_MESSAGE("Return CPU threads patching...");
		void* codeBuf = g_localTrampoline.StartAlloc();
		ReturnThreadsNewGame_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		ReturnThreadsNewGame_Original = (_ReturnThreadsNewGame)codeBuf;

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0xD1D4A0).GetUIntPtr(), (uintptr_t)ReturnThreadsNewGame);
		_MESSAGE("Return CPU threads OK");
}

void LimiterFunc() {
	if (CurrentFPS > 0) {
		while (PerfCounter::deltal(timing, ::_Query_perf_counter()) < CurrentFPS)
		{
			Sleep(0);
		}
	}
	timing = _Query_perf_counter();
}

void RegisterHooks() {
	g_branchTrampoline.Write5Call(FPSLimiter.GetUIntPtr(), (uintptr_t)LimiterFunc);
	CurrentFPS = FPSui = static_cast<long long>(fpslimitgame * 1000000.0);
	lockpickingFPSmax = static_cast<long long>(fpslockpicking * 1000000.0);
	loadingFPSmax = static_cast<long long>(fpslimitload * 1000000.0);
}

void PatchGame() {
	if (FixCPUThreads) {
		SafeWriteBuf(FixCPUThreadsAddress.GetUIntPtr(), "\xEB\x14", 2); //74 14
	}
	if (UntieSpeedFromFPS) {
		unsigned char data1[] = { 0xBA, 0x00, 0x00, 0x00, 0x00 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1B1393A).GetUIntPtr(), &data1, sizeof(data1));
	}
	if (ReduceAfterLoading) {
		SafeWriteBuf(PostloadingMenuSpeedAddress.GetUIntPtr(), &PostloadingMenuSpeed, sizeof(float));
		SafeWriteBuf(RelocAddr<uintptr_t>(0x126D2DB).GetUIntPtr(), "\xF3\x0F\x10\x05\x08\x08\x00\x00\xF3\x0F\x59\x05\xE9\xE3\x8E\x04\xE9\x73\x04\x00\x00", 21);
		SafeWriteBuf(RelocAddr<uintptr_t>(0x126D75B).GetUIntPtr(), "\xE9\x7B\xFB\xFF\xFF\x90\x90\x90", 8);
	}
	if (FixRotationSpeed) {
		//objects
		SafeWriteBuf(RelocAddr<uintptr_t>(0xF49645).GetUIntPtr(), "\xF3\x0F\x59\x15\x83\x20\xC1\x04", 8);
		float ForwardRotationSpeed = 2.941176470588235;
		float ReverseRotationSpeed = -2.941176470588235;
		SafeWriteBuf(RelocAddr<uintptr_t>(0xF49664).GetUIntPtr(), &ForwardRotationSpeed, sizeof(float));
		SafeWriteBuf(RelocAddr<uintptr_t>(0xF49668).GetUIntPtr(), &ReverseRotationSpeed, sizeof(float));
		SafeWriteBuf(RelocAddr<uintptr_t>(0xF49623).GetUIntPtr(), "\xF3\x0F\x10\x15\x39\x00\x00\x00", 8);
		SafeWriteBuf(RelocAddr<uintptr_t>(0xF4962D).GetUIntPtr(), "\xF3\x0F\x10\x15\x33\x00\x00\x00", 8);
	}
	if (FixWorkshopRotationSpeed) {
		SafeWriteBuf(RelocAddr<uintptr_t>(0x2182B2).GetUIntPtr(), "\xF3\x0F\x59\x0D\x1A\x34\x94\x05", 8);
	}
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
	if (FixStuttering) {
		//cvttss2si rcx,xmm3 fix
		//jmp
		unsigned char data6[] = { 0xE9, 0x92, 0x9D, 0xFF, 0xFF };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EB96).GetUIntPtr(), &data6, sizeof(data6));
		//= 1
		float value1 = 1;
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6D470).GetUIntPtr(), &value1, sizeof(float));
		//movss xmm3
		unsigned char data7[] = { 0xF3, 0x0F, 0x10, 0x1D, 0x3B, 0x4B, 0x00, 0x00, 0xF3, 0x48, 0x0F, 0x2C, 0xCB, 0xE9, 0x5C, 0x62, 0x00, 0x00 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6892D).GetUIntPtr(), &data7, sizeof(data7));
		//C24 = 0
		unsigned char data9[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EDA1).GetUIntPtr(), &data9, sizeof(data9));
		//C20 = FPS
		unsigned char data10[] = { 0xF3, 0x0F, 0x11, 0x35, 0xCD, 0x40, 0x7D, 0x04 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EB4B).GetUIntPtr(), &data10, sizeof(data10));
		//C1C = FPS
		unsigned char data11[] = { 0xF3, 0x0F, 0x11, 0x35, 0x9F, 0x40, 0x7D, 0x04 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EB75).GetUIntPtr(), &data11, sizeof(data11));
		//fix moving objects
		unsigned char data14[] = { 0xF3, 0x0F, 0x10, 0x0D, 0xC8, 0x1A, 0x7C, 0x01, 0xE9, 0xD3, 0xFD, 0xFF, 0xFF };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x14D58A0).GetUIntPtr(), &data14, sizeof(data14));
		unsigned char data22[] = { 0xF3, 0x0F, 0x11, 0x15, 0xB6, 0x42, 0x7D, 0x04, 0x48, 0x83, 0xC4, 0x40, 0x5F, 0xC3 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EBF2).GetUIntPtr(), &data22, sizeof(data22));
		//fix hair/clothes
		if (!FixWindSpeed) {
			unsigned char data18[] = { 0xF3, 0x44, 0x0F, 0x10, 0x0D, 0x59, 0x36, 0x7D, 0x04 };
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6F84E).GetUIntPtr(), &data18, sizeof(data18));
			unsigned char data19[] = { 0xF3, 0x44, 0x0F, 0x10, 0x0D, 0xB0, 0x35, 0x7D, 0x04 };
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6F8F7).GetUIntPtr(), &data19, sizeof(data19));
			unsigned char data21[] = { 0xF3, 0x0F, 0x10, 0x05, 0xCD, 0x33, 0x7D, 0x04 };
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6FADB).GetUIntPtr(), &data21, sizeof(data21));
		}
	}
	if (FixWhiteScreen) {
		unsigned char data23[] = { 0x90, 0x90 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x172A893).GetUIntPtr(), &data23, sizeof(data23));
	}
	if (FixWindSpeed) {
		unsigned char data17[] = { 0x0F, 0x29, 0x74, 0x24, 0x30, 0xF3, 0x0F, 0x10, 0x35, 0x97, 0x7A, 0xEC, 0x00, 0xE9, 0x2B, 0xFE, 0xFF, 0xFF };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1DCF8CC).GetUIntPtr(), &data17, sizeof(data17));
		unsigned char data8[] = { 0xE9, 0xC6, 0x01, 0x00, 0x00 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1DCF701).GetUIntPtr(), &data8, sizeof(data8));
	}
	if (FixRotationSpeed) {
		//fix mouse rotation speed
		unsigned char data12[] = { 0xF3, 0x0F, 0x59, 0x0D, 0x10, 0x9D, 0x97, 0x01 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x129CA32).GetUIntPtr(), &data12, sizeof(data12));
		unsigned char fPickMouseRotationSpeed[] = { 0x96, 0x43, 0x8B, 0x3C, 0x00 }; //0.017
		SafeWriteBuf(RelocAddr<uintptr_t>(0x2C1674A).GetUIntPtr(), fPickMouseRotationSpeed, sizeof(fPickMouseRotationSpeed));
	}
	//Disable animation on loading screens
	if (DisableAnimationOnLoadingScreens) {
		unsigned char data20[] = { 0x90, 0x90, 0x90, 0x90 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0xCBFFCD).GetUIntPtr(), &data20, sizeof(data20));
	}
	if (DisableBlackLoadingScreens) {
		unsigned char data15[] = { 0xEB, 0x1E };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1297076).GetUIntPtr(), &data15, sizeof(data15));
	}
	if (FixLoadingModel) {
		//fix rotation on loading screen
		unsigned char data24[] = { 0x8B, 0x8B, 0x6C, 0x02, 0x00, 0x00, 0xF3, 0x44, 0x0F, 0x10, 0x05, 0x25, 0xFC, 0x9F, 0x01, 0xE9, 0x2A, 0xFD, 0xFF, 0xFF };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x129773C).GetUIntPtr(), &data24, sizeof(data24));
		unsigned char data25[] = { 0xE9, 0xC3, 0x02, 0x00, 0x00, 0x90 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1297474).GetUIntPtr(), &data25, sizeof(data25));
		unsigned char data13[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1298EA3).GetUIntPtr(), &data13, sizeof(data13));
	}
	if (FixMotionResponsive) {
		float MotionFeedback = 294.1176470588235;
		SafeWriteBuf(RelocAddr<uintptr_t>(0x2845F5).GetUIntPtr(), &MotionFeedback, sizeof(float));
		SafeWriteBuf(RelocAddr<uintptr_t>(0x2844EB).GetUIntPtr(), "\xE9\xAC\x00\x00\x00\x0F\x2F\xC1\x90\x90\x90\x90", 12);
		SafeWriteBuf(RelocAddr<uintptr_t>(0x28459C).GetUIntPtr(), "\xF3\x0F\x10\x45\x40\xF3\x0F\x10\x0D\x4C\x00\x00\x00\xF3\x0F\x59\xCE\xEB\x05\xCC\xC2\x00\x00\xCC\xE9\x37\xFF\xFF\xFF", 29);
	}
}

void onF4SEMessage(F4SEMessagingInterface::Message* msg) {
	switch (msg->type) {
	case F4SEMessagingInterface::kMessage_GameDataReady:
		static auto pMenuOpenCloseHandler = new MenuOpenCloseHandler();
		(*g_ui)->menuOpenCloseEventSource.AddEventSink(pMenuOpenCloseHandler);
		if (firstload) {
			PatchGame();
			HookFPS();
			if (LimitCPUThreadsNG) {
				_MESSAGE("Getting the number of processor threads...");
				GetNumberOfThreads();
				LimitCPUNewGame();
				ReturnThreads();
			}
			if (limit) {
				timing = _Query_perf_counter();
				RegisterHooks();
			}
			_MESSAGE("\nPlugin load completed!\n");
			firstload = false;
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

		if (f4se->runtimeVersion != SUPPORTED_RUNTIME_VERSION) {
			char str[512];
			sprintf_s(str, sizeof(str), "Your game version: v%d.%d.%d.%d\nExpected version: v%d.%d.%d.%d\n%s will be disabled.",
				GET_EXE_VERSION_MAJOR(f4se->runtimeVersion),
				GET_EXE_VERSION_MINOR(f4se->runtimeVersion),
				GET_EXE_VERSION_BUILD(f4se->runtimeVersion),
				GET_EXE_VERSION_SUB(f4se->runtimeVersion),
				GET_EXE_VERSION_MAJOR(SUPPORTED_RUNTIME_VERSION),
				GET_EXE_VERSION_MINOR(SUPPORTED_RUNTIME_VERSION),
				GET_EXE_VERSION_BUILD(SUPPORTED_RUNTIME_VERSION),
				GET_EXE_VERSION_SUB(SUPPORTED_RUNTIME_VERSION),
				PLUGIN_NAME_LONG
			);
			MessageBox(NULL, str, PLUGIN_NAME_LONG, MB_OK | MB_ICONEXCLAMATION);
			return false;
		}
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
		getinisettings();
		PatchDisplay();
		Hook();
		//Disable bethesda auto Vsync and FPS cap
		SafeWriteBuf(BethesdaVsyncAddress.GetUIntPtr(), "\x90\x90\x90\x90", 4);
		uint32_t maxrefreshrate = 10000;
		SafeWriteBuf(BethesdaFPSCap1Address.GetUIntPtr(), &maxrefreshrate, sizeof(maxrefreshrate));
		SafeWriteBuf(BethesdaFPSCap2Address.GetUIntPtr(), &maxrefreshrate, sizeof(maxrefreshrate));
		if (Vsync) {
			SafeWriteBuf(VsyncAddress.GetUIntPtr(), &PresentInterval1, sizeof(PresentInterval1));
		}
		g_messaging->RegisterListener(g_pluginHandle, "F4SE", onF4SEMessage);
		return true;
	}
};