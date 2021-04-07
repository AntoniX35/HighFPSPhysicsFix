#include "stdafx.h"
#include "INIReader.h"

float f = 0.017;
PerfCounter PerfCounter::m_Instance;
long long PerfCounter::perf_freq;
float PerfCounter::perf_freqf;

RelocAddr<uintptr_t> FPSLimiter(0x1D0B67E);
RelocAddr<uintptr_t> PostloadingMenuSpeedAddress(0x126DAEB);
RelocAddr<uintptr_t> SittingRotationSpeedXAddress(0x3804738);
RelocAddr<uintptr_t> SittingRotationSpeedYAddress(0x3804750);
RelocAddr<uintptr_t> BethesdaVsyncAddress(0x1D17792);
RelocAddr<uintptr_t> BethesdaFPSCap1Address(0xD423BA);
RelocAddr<uintptr_t> BethesdaFPSCap2Address(0xD423C3);
RelocAddr<uintptr_t> VsyncAddress(0x61E0950);
RelocAddr<uintptr_t> FrametimeAddress(0x5A66FE8);    
RelocAddr<uintptr_t> MotionFeedBackAddress(0x2845F5);
RelocAddr<uintptr_t> DefaultValueAddress(0x2C97370); //0.0166666666666666
RelocAddr<uintptr_t> FPS60ValueAddress(0x2C1674A);   //0.017
RelocAddr<uintptr_t> PanSpeedAddress(0x3808270);

long long CurrentFPS, FPSui, loadingFPSmax, lockpickingFPSmax, timing;
bool limit = false;
float fpslimitgame, fpslimitload, fpslockpicking, SittingRotSpeedX, SittingRotSpeedY, PostloadingMenuSpeed;
int isLockpicking, NumOfThreadsWhileLoading, NumOfThreadsWhileLoadingNewGame, fpslimit, SwapBufferCount, DXGISwapEffect, ScalingMode;
unsigned int nMaxProcessorMask;
unsigned int nMaxProcessorMaskNG;
unsigned int nMaxProcessorAfterLoad;
HANDLE f4handle = NULL;
double maxft, minft;
bool EnableDisplayTweaks, Fullscreen, FixLockpickingSound, accelerateLoading, UntieSpeedFromFPS, DisableiFPSClamp, FixStuttering, FixWorkshopRotationSpeed, FixRotationSpeed, FixSittingRotationSpeed, FixStuckAnimation, limitload, limitgame, LimitCPUThreadsNG, DisableAnimationOnLoadingScreens, DisableBlackLoadingScreens, FixWindSpeed, FixWhiteScreen, FixLoadingModel, ReduceAfterLoading, FixCPUThreads, WriteLoadingTime, Vsync, Tearing, ResizeBuffersDisable, ResizeTargetDisable, Borderless, FixMotionResponsive;
bool firstload = true;
F4SEScaleformInterface* g_scaleform = NULL;
F4SEMessagingInterface* g_messaging = NULL;
PluginHandle			    g_pluginHandle = kPluginHandle_Invalid;

int PresentInterval0 = 0;
int PresentInterval1 = 1;

static F4SEPapyrusInterface* g_papyrus = NULL;

typedef void(*_ChangeThreads)(void* unk1, void* unk2, void* unk3, void* unk4);
_ChangeThreads ChangeThreads_Original = nullptr;
typedef void(*_ChangeThreadsNewGame)(void* unk1, void* unk2, void* unk3, void* unk4);
_ChangeThreadsNewGame ChangeThreadsNewGame_Original = nullptr;
typedef void(*_ReturnThreadsNewGame)(void* unk1, void* unk2, void* unk3, void* unk4);
_ReturnThreadsNewGame ReturnThreadsNewGame_Original = nullptr;

clock_t start, end;

class MenuOpenCloseHandler : public BSTEventSink<MenuOpenCloseEvent>
	{
	public:
		virtual ~MenuOpenCloseHandler() { };
		virtual	EventResult	ReceiveEvent(MenuOpenCloseEvent* evn, void* dispatcher) override;
};

std::chrono::high_resolution_clock::time_point tlastf = std::chrono::high_resolution_clock::now(), tcurrentf = std::chrono::high_resolution_clock::now(), hvlast = std::chrono::high_resolution_clock::now(), hvcurrent = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> time_span, tdif;

void getinisettings() {
	std::string value;
	std::ostringstream os;

	INIReader reader(PLUGIN_INI_FILE);
	if (reader.ParseError() < 0) {
		_MESSAGE("Can't load 'HighFPSPhysics.ini'\n");
	}

	//Main

	os << "\nUntie game speed from framerate: ";

	UntieSpeedFromFPS = reader.GetBoolean("Main", "UntieSpeedFromFPS", true);
	if (UntieSpeedFromFPS) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nDisable iFPSClamp: ";

	DisableiFPSClamp = reader.GetBoolean("Main", "DisableiFPSClamp", true);
	if (DisableiFPSClamp) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}
	
	os << "\nVSync (in Game): ";

	Vsync = reader.GetBoolean("Main", "EnableVSync", true);
	if (Vsync) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nVSync (in Loading Screens): ";

	accelerateLoading = reader.GetBoolean("Main", "DisableVSyncWhileLoading", true);	
	if (accelerateLoading && !DisableAnimationOnLoadingScreens) {
		accelerateLoading = true;
		os << "disabled";
	}
	else {
		accelerateLoading = false;
		os << "enabled";
	}

	os << "\nLoading screens: ";

	DisableBlackLoadingScreens = reader.GetBoolean("Main", "DisableBlackLoadingScreens", false);
	if (DisableBlackLoadingScreens) {
		os << "without black screens";
	}
	else {
		os << "default";
	}

	os << "\nDisable animation on loading screens: ";

	DisableAnimationOnLoadingScreens = reader.GetBoolean("Main", "DisableAnimationOnLoadingScreens", false);
	if (DisableAnimationOnLoadingScreens) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nWrite loading time to log: ";

	//Write loading time to log
	WriteLoadingTime = reader.GetBoolean("Main", "WriteLoadingTime", false);
	if (WriteLoadingTime) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nPostloading menu speed: ";

	PostloadingMenuSpeed = reader.GetReal("Main", "PostloadingMenuSpeed", 1);
	if (PostloadingMenuSpeed != 1) {
		os << PostloadingMenuSpeed;
		ReduceAfterLoading = true;
	}
	else {
		ReduceAfterLoading = false;
		os << "default";
	}

	os << "\nProcessWindowGhosting: ";

	bool DisableGhosting = reader.GetBoolean("Main", "DisableProcessWindowsGhosting", true);
	if (DisableGhosting) {
		DisableProcessWindowsGhosting();
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	//Limiters

	float ingamefps = reader.GetReal("Limiter", "InGameFPS", 0);
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

	float loadscreenfps = reader.GetReal("Limiter", "LoadingScreenFPS", 0);
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

	float LockpickingFPS = reader.GetReal("Limiter", "LockpickingFPS", 0);
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

	NumOfThreadsWhileLoadingNewGame = reader.GetInteger("Limiter", "NumOfThreadsWhileLoadingNewGame", 1);
	os << "\nNum of threads on loading screens (new game): ";
	if (NumOfThreadsWhileLoadingNewGame > 0) {
		os << NumOfThreadsWhileLoadingNewGame;
		LimitCPUThreadsNG = true;
	}
	else {
		LimitCPUThreadsNG = false;
		os << "default";
	}

	os << "\nEnable display tweaks: ";

	EnableDisplayTweaks = reader.GetBoolean("DisplayTweaks", "EnableDisplayTweaks", false);
	if (EnableDisplayTweaks) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFull screen: ";

	Fullscreen = reader.GetBoolean("DisplayTweaks", "FullScreen", false);
	if (Fullscreen) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nBorderless screen: ";

	Borderless = reader.GetBoolean("DisplayTweaks", "Borderless", true);
	if (Borderless) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}
	os << "\nNumber of buffers in the swap chain: ";

	SwapBufferCount = reader.GetInteger("DisplayTweaks", "BufferCount", 0);
	if (SwapBufferCount == 0) {
		os << "auto";
	}
	else {
		os << SwapBufferCount;
	}

	os << "\nResize buffers: ";

	ResizeBuffersDisable = reader.GetBoolean("DisplayTweaks", "ResizeBuffersDisable", false);
	if (ResizeBuffersDisable) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nResize target: ";

	ResizeTargetDisable = reader.GetBoolean("DisplayTweaks", "ResizeTargetDisable", false);
	if (ResizeTargetDisable) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}
	
	os << "\nDXGI swap effect: ";

	DXGISwapEffect = reader.GetInteger("DisplayTweaks", "SwapEffect", 0);
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

	Tearing = reader.GetBoolean("DisplayTweaks", "AllowTearing", true);
	if (Tearing) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nDXGI mode scalling: ";

	ScalingMode = reader.GetInteger("DisplayTweaks", "ScalingMode", 0);
	if (ScalingMode == 1) {
		os << "unspecified";
	}
	if (ScalingMode == 2) {
		os << "centered";
	}
	if (ScalingMode == 3) {
		os << "stretched";
	}

	os << "\nFix CPU threads: ";

	//Fix CPU threads
	FixCPUThreads = reader.GetBoolean("Fixes", "FixCPUThreads", true);
	
	if (FixCPUThreads) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix stuttering: ";

	//Fix game stuttering
	FixStuttering = reader.GetBoolean("Fixes", "FixStuttering", true);
	if (FixStuttering) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix white screen: ";

	//Fix white screen
	FixWhiteScreen = reader.GetBoolean("Fixes", "FixWhiteScreen", true);
	if (FixWhiteScreen) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix wind speed: ";

	//Fix wind speed
	FixWindSpeed = reader.GetBoolean("Fixes", "FixWindSpeed", true);	
	if (FixWindSpeed) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix rotation speed: ";

	FixRotationSpeed = reader.GetBoolean("Fixes", "FixRotationSpeed", true);
	
	if (FixRotationSpeed) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix sitting rotation speed: ";

	FixSittingRotationSpeed = reader.GetBoolean("Fixes", "FixSittingRotationSpeed", true);	
	if (FixSittingRotationSpeed) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix workshop rotation speed: ";

	FixWorkshopRotationSpeed = reader.GetBoolean("Fixes", "FixWorkshopRotationSpeed", true);
	if (FixWorkshopRotationSpeed) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix loading model: ";

	FixLoadingModel = reader.GetBoolean("Fixes", "FixLoadingModel", true);	
	if (FixLoadingModel) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix stuck animation: ";

	FixStuckAnimation = reader.GetBoolean("Fixes", "FixStuckAnimation", true);	
	if (FixStuckAnimation) {
		os << "enabled";
	}
	else {
		os << "disabled";
	}

	os << "\nFix motion responsive: ";

	FixMotionResponsive = reader.GetBoolean("Fixes", "FixMotionResponsive", true);	
	if (FixMotionResponsive) {
		os << "enabled\n";
	}
	else {
		os << "disabled\n";
	}
	_MESSAGE(os.str().c_str());
}

void GetNumberOfThreads() {
	nMaxProcessorMaskNG = (1 << NumOfThreadsWhileLoadingNewGame) - 1;
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	nMaxProcessorAfterLoad = (1 << SystemInfo.dwNumberOfProcessors) - 1;
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
		{
			struct Patch1_Code : Xbyak::CodeGenerator {
				Patch1_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label timerLabel;

					mov(r9, ptr[rip + timerLabel]);
					mulss(xmm0, dword[r9]);
					mulss(xmm0, ptr[rax + 0x4C]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x5);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf1 = g_localTrampoline.StartAlloc();
			Patch1_Code code1(codeBuf1, RelocAddr<uintptr_t>(0x12446D0).GetUIntPtr(), FrametimeAddress);
			g_localTrampoline.EndAlloc(code1.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x12446D0).GetUIntPtr(), uintptr_t(code1.getCode()));
		}
		//y
		{
			struct Patch2_Code : Xbyak::CodeGenerator {
				Patch2_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label timerLabel;

					mov(r9, ptr[rip + timerLabel]);
					mulss(xmm1, dword[r9]);
					movss(xmm0, ptr[rbx + 0x64]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x5);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf2 = g_localTrampoline.StartAlloc();
			Patch2_Code code2(codeBuf2, RelocAddr<uintptr_t>(0x12446E7).GetUIntPtr(), FrametimeAddress);
			g_localTrampoline.EndAlloc(code2.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x12446E7).GetUIntPtr(), uintptr_t(code2.getCode()));
		}
	}
}

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

void GetProcess() {
	DWORD procId = GetCurrentProcessId();
	f4handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
}

void PatchGame() {
	if (DisableAnimationOnLoadingScreens) {
		SafeWriteBuf(RelocAddr<uintptr_t>(0xCBFFCD).GetUIntPtr(), "\x90\x90\x90\x90", 4);
	}
	if (DisableBlackLoadingScreens) {
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1297076).GetUIntPtr(), "\xEB", 1);
	}
	float DefaultFPSValue = 0.017;
	SafeWriteBuf(FPS60ValueAddress.GetUIntPtr(), &DefaultFPSValue, sizeof(float));
	if (FixCPUThreads) {
		SafeWriteBuf(RelocAddr<uintptr_t>(0xE9B7E1).GetUIntPtr(), "\xEB", 1); //je -> jmp
		struct Patch_Code : Xbyak::CodeGenerator {
			Patch_Code(void* buf, uintptr_t a_hookTarget) : Xbyak::CodeGenerator(4096, buf) {
				Xbyak::Label retnLabel;

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_hookTarget + 0x89);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		Patch_Code code(codeBuf, RelocAddr<uintptr_t>(0xD69DCF).GetUIntPtr());
		g_localTrampoline.EndAlloc(code.getCurr());
		g_branchTrampoline.Write6Branch(RelocAddr<uintptr_t>(0xD69DCF).GetUIntPtr(), uintptr_t(code.getCode()));
		SafeWriteBuf(RelocAddr<uintptr_t>(0xD69DCF + 0x5).GetUIntPtr(), "\x90", 1);
	}
	if (UntieSpeedFromFPS) {
		SafeWriteBuf(RelocAddr<uintptr_t>(0x1B1393B).GetUIntPtr(), "\x00", 1);
	}
	if (ReduceAfterLoading) {
		SafeWriteBuf(PostloadingMenuSpeedAddress.GetUIntPtr(), &PostloadingMenuSpeed, sizeof(float));
		struct Patch_Code : Xbyak::CodeGenerator {
			Patch_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_frameTimer, uintptr_t a_value) : Xbyak::CodeGenerator(4096, buf) {
				Xbyak::Label retnLabel;
				Xbyak::Label timerLabel;
				Xbyak::Label valueLabel;

				mov(r14, ptr[rip + valueLabel]);
				movss(xmm0, dword[r14]);
				mov(r14, ptr[rip + timerLabel]);
				mulss(xmm0, dword[r14]);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_hookTarget + 0x8);

				L(valueLabel);
				dq(a_value);

				L(timerLabel);
				dq(a_frameTimer);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		Patch_Code code(codeBuf, RelocAddr<uintptr_t>(0x126D75B).GetUIntPtr(), FrametimeAddress, PostloadingMenuSpeedAddress);
		g_localTrampoline.EndAlloc(code.getCurr());
		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x126D75B).GetUIntPtr(), uintptr_t(code.getCode()));
		SafeWriteBuf(RelocAddr<uintptr_t>(0x126D75B + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
	}
	if (FixRotationSpeed) {
		float ForwardRotationSpeed = 2.941176470588235;
		float ReverseRotationSpeed = -2.941176470588235;
		SafeWriteBuf(RelocAddr<uintptr_t>(0xF49664).GetUIntPtr(), &ForwardRotationSpeed, sizeof(float));
		SafeWriteBuf(RelocAddr<uintptr_t>(0xF49668).GetUIntPtr(), &ReverseRotationSpeed, sizeof(float));
		{
			struct Patch1_Code : Xbyak::CodeGenerator {
				Patch1_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label timerLabel;

					mov(r13, ptr[rip + timerLabel]);
					mulss(xmm2, dword[r13]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf1 = g_localTrampoline.StartAlloc();
			Patch1_Code code1(codeBuf1, RelocAddr<uintptr_t>(0xF49645).GetUIntPtr(), FrametimeAddress);
			g_localTrampoline.EndAlloc(code1.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0xF49645).GetUIntPtr(), uintptr_t(code1.getCode()));
			SafeWriteBuf(RelocAddr<uintptr_t>(0xF49645 + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
		{
			struct Patch2_Code : Xbyak::CodeGenerator {
				Patch2_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;

					mov(r13, ptr[rip + valueLabel]);
					movss(xmm2, dword[r13]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(valueLabel);
					dq(a_value);
				}
			};
			void* codeBuf2 = g_localTrampoline.StartAlloc();
			Patch2_Code code2(codeBuf2, RelocAddr<uintptr_t>(0xF49623).GetUIntPtr(), RelocAddr<uintptr_t>(0xF49664).GetUIntPtr());
			g_localTrampoline.EndAlloc(code2.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0xF49623).GetUIntPtr(), uintptr_t(code2.getCode()));
			SafeWriteBuf(RelocAddr<uintptr_t>(0xF49623 + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
		{
			struct Patch3_Code : Xbyak::CodeGenerator {
				Patch3_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;

					mov(r13, ptr[rip + valueLabel]);
					movss(xmm2, dword[r13]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(valueLabel);
					dq(a_value);
				}
			};
			void* codeBuf3 = g_localTrampoline.StartAlloc();
			Patch3_Code code3(codeBuf3, RelocAddr<uintptr_t>(0xF4962D).GetUIntPtr(), RelocAddr<uintptr_t>(0xF49668).GetUIntPtr());
			g_localTrampoline.EndAlloc(code3.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0xF4962D).GetUIntPtr(), uintptr_t(code3.getCode()));
			SafeWriteBuf(RelocAddr<uintptr_t>(0xF4962D + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
	}
	if (FixStuttering) {
		//cvttss2si rcx,xmm3 fix
		//= 1
		float value1 = 1;
		{
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6D470).GetUIntPtr(), &value1, sizeof(float));
			struct Patch1_Code : Xbyak::CodeGenerator {
				Patch1_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_fps60Value) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;

					mov(r9, ptr[rip + valueLabel]);
					movss(xmm3, dword[r9]);
					cvttss2si(rcx, xmm3);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x5);

					L(valueLabel);
					dq(a_fps60Value);
				}
			};
			void* codeBuf1 = g_localTrampoline.StartAlloc();
			Patch1_Code code1(codeBuf1, RelocAddr<uintptr_t>(0x1D6EB96).GetUIntPtr(), RelocAddr<uintptr_t>(0x1D6D470).GetUIntPtr());
			g_localTrampoline.EndAlloc(code1.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x1D6EB96).GetUIntPtr(), uintptr_t(code1.getCode()));
			//C24 = 0
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EDA1).GetUIntPtr(), "\x90\x90\x90\x90\x90\x90\x90\x90", 8);
		}
		{
			//C20 = FPS
			struct Patch2_Code : Xbyak::CodeGenerator {
				Patch2_Code(void* buf, uintptr_t a_hookTarget) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;

					minss(xmm2, ptr[rsp + 0x20]);
					movss(xmm2, xmm6);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x6);
				}
			};
			void* codeBuf2 = g_localTrampoline.StartAlloc();
			Patch2_Code code2(codeBuf2, RelocAddr<uintptr_t>(0x1D6EB45).GetUIntPtr());
			g_localTrampoline.EndAlloc(code2.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x1D6EB45).GetUIntPtr(), uintptr_t(code2.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EB45 + 0x5).GetUIntPtr(), "\x90", 1);
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EB45 + 0xE).GetUIntPtr(), "\x90\x90\x90\x90", 4);
			//C1C = FPS
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6EB71).GetUIntPtr(), "\x90\x90\x90\x90", 4);
		}
		{
			//fix moving objects
			struct Patch3_Code : Xbyak::CodeGenerator {
				Patch3_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_fps60Value) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;

					mov(r8, ptr[rip + valueLabel]);
					movss(xmm1, dword[r8]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x5);

					L(valueLabel);
					dq(a_fps60Value);
				}
			};
			void* codeBuf3 = g_localTrampoline.StartAlloc();
			Patch3_Code code3(codeBuf3, RelocAddr<uintptr_t>(0x14D58A0).GetUIntPtr(), FPS60ValueAddress);
			g_localTrampoline.EndAlloc(code3.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x14D58A0).GetUIntPtr(), uintptr_t(code3.getCode()));
		}		
	}
	if (FixWhiteScreen) {
		unsigned char data23[] = { 0x90, 0x90 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x172A893).GetUIntPtr(), &data23, sizeof(data23));
	}
	if (FixWindSpeed) { //fix hair/clothes
		{
			struct Patch1_Code : Xbyak::CodeGenerator {
				Patch1_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_fps60Value) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;

					movaps(ptr[rsp + 0x30], xmm6);
					mov(r9, ptr[rip + valueLabel]);
					movss(xmm6, dword[r9]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(valueLabel);
					dq(a_fps60Value);
				}
			};
			void* codeBuf1 = g_localTrampoline.StartAlloc();
			Patch1_Code code1(codeBuf1, RelocAddr<uintptr_t>(0x1DCF701).GetUIntPtr(), DefaultValueAddress);
			g_localTrampoline.EndAlloc(code1.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x1DCF701).GetUIntPtr(), uintptr_t(code1.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x1DCF701 + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
		{
			struct Patch2_Code : Xbyak::CodeGenerator {
				Patch2_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label timerLabel;

					mov(r8, ptr[rip + timerLabel]);
					movss(xmm9, dword[r8]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x9);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf2 = g_localTrampoline.StartAlloc();
			Patch2_Code code2(codeBuf2, RelocAddr<uintptr_t>(0x1D6F84E).GetUIntPtr(), FrametimeAddress);
			g_localTrampoline.EndAlloc(code2.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x1D6F84E).GetUIntPtr(), uintptr_t(code2.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6F84E + 0x5).GetUIntPtr(), "\x90\x90\x90\x90", 4);
		}
		{
			struct Patch3_Code : Xbyak::CodeGenerator {
				Patch3_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label timerLabel;

					mov(r8, ptr[rip + timerLabel]);
					movss(xmm9, dword[r8]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x9);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf3 = g_localTrampoline.StartAlloc();
			Patch3_Code code3(codeBuf3, RelocAddr<uintptr_t>(0x1D6F8F7).GetUIntPtr(), FrametimeAddress);
			g_localTrampoline.EndAlloc(code3.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x1D6F8F7).GetUIntPtr(), uintptr_t(code3.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6F8F7 + 0x5).GetUIntPtr(), "\x90\x90\x90\x90", 4);
		}
		{
			struct Patch4_Code : Xbyak::CodeGenerator {
				Patch4_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf) {
					Xbyak::Label retnLabel;
					Xbyak::Label timerLabel;

					mov(r8, ptr[rip + timerLabel]);
					movss(xmm0, dword[r8]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf4 = g_localTrampoline.StartAlloc();
			Patch4_Code code4(codeBuf4, RelocAddr<uintptr_t>(0x1D6FADB).GetUIntPtr(), FrametimeAddress);
			g_localTrampoline.EndAlloc(code4.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x1D6FADB).GetUIntPtr(), uintptr_t(code4.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x1D6FADB + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
	}
	if (FixRotationSpeed) {
		//fix lockpick rotation speed
		struct Patch_Code : Xbyak::CodeGenerator {
			Patch_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;
				Xbyak::Label valueLabel;

				mov(r10, ptr[rip + valueLabel]);
				mulss(xmm1, dword[r10]);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_hookTarget + 0x8);

				L(valueLabel);
				dq(a_value);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		Patch_Code code(codeBuf, RelocAddr<uintptr_t>(0x129CA32).GetUIntPtr(), FPS60ValueAddress);
		g_localTrampoline.EndAlloc(code.getCurr());

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x129CA32).GetUIntPtr(), uintptr_t(code.getCode()));

		SafeWriteBuf(RelocAddr<uintptr_t>(0x129CA32 + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
	}
	if (FixWorkshopRotationSpeed) {
		struct Patch_Code : Xbyak::CodeGenerator {
			Patch_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;
				Xbyak::Label timerLabel;

				mov(r8, ptr[rip + timerLabel]);
				mulss(xmm1, dword[r8]);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_hookTarget + 0x8);

				L(timerLabel);
				dq(a_frameTimer);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		Patch_Code code(codeBuf, RelocAddr<uintptr_t>(0x2182B2).GetUIntPtr(), FrametimeAddress);
		g_localTrampoline.EndAlloc(code.getCurr());

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x2182B2).GetUIntPtr(), uintptr_t(code.getCode()));
	}
	if (FixLoadingModel) {
		{
			//Fix repeat rate
			struct Patch0_Code : Xbyak::CodeGenerator {
				Patch0_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;

					mov(ecx, ptr[rbx + 0x26C]);
					mov(r8, ptr[rip + valueLabel]);
					movss(xmm8, dword[r8]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x6);

					L(valueLabel);
					dq(a_value);
				}
			};
			void* codeBuf0 = g_localTrampoline.StartAlloc();
			Patch0_Code code0(codeBuf0, RelocAddr<uintptr_t>(0x1297474).GetUIntPtr(), DefaultValueAddress);
			g_localTrampoline.EndAlloc(code0.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x1297474).GetUIntPtr(), uintptr_t(code0.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x1297474 + 0x5).GetUIntPtr(), "\x90", 1);

			//Fix rotation
			SafeWriteBuf(RelocAddr<uintptr_t>(0x1298D0F).GetUIntPtr(), "\x90\x90\x90\x90\x90\x90", 6);
		}
		{
			//Fix pan speed
			//Up
			float PanSpeed = 0.2941176470588235; //0.005
			SafeWriteBuf(PanSpeedAddress.GetUIntPtr(), &PanSpeed, sizeof(float));

			struct Patch1_Code : Xbyak::CodeGenerator {
				Patch1_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;
					Xbyak::Label timerLabel;

					mov(r10, ptr[rip + valueLabel]);
					movss(xmm0, dword[r10]);
					mov(r10, ptr[rip + timerLabel]);
					mulss(xmm0, dword[r10]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(valueLabel);
					dq(a_value);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf1 = g_localTrampoline.StartAlloc();
			Patch1_Code code1(codeBuf1, RelocAddr<uintptr_t>(0x129758C).GetUIntPtr(), PanSpeedAddress, FrametimeAddress);
			g_localTrampoline.EndAlloc(code1.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x129758C).GetUIntPtr(), uintptr_t(code1.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x129758C + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
		{
			//Down
			struct Patch2_Code : Xbyak::CodeGenerator {
				Patch2_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;
					Xbyak::Label timerLabel;

					mov(r10, ptr[rip + valueLabel]);
					movss(xmm0, dword[r10]);
					mov(r10, ptr[rip + timerLabel]);
					mulss(xmm0, dword[r10]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(valueLabel);
					dq(a_value);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf2 = g_localTrampoline.StartAlloc();
			Patch2_Code code2(codeBuf2, RelocAddr<uintptr_t>(0x12975E6).GetUIntPtr(), PanSpeedAddress, FrametimeAddress);
			g_localTrampoline.EndAlloc(code2.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x12975E6).GetUIntPtr(), uintptr_t(code2.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x12975E6 + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
		{
			//Left
			struct Patch3_Code : Xbyak::CodeGenerator {
				Patch3_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;
					Xbyak::Label timerLabel;

					mov(r10, ptr[rip + valueLabel]);
					movss(xmm0, dword[r10]);
					mov(r10, ptr[rip + timerLabel]);
					mulss(xmm0, dword[r10]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(valueLabel);
					dq(a_value);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf3 = g_localTrampoline.StartAlloc();
			Patch3_Code code3(codeBuf3, RelocAddr<uintptr_t>(0x129763F).GetUIntPtr(), PanSpeedAddress, FrametimeAddress);
			g_localTrampoline.EndAlloc(code3.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x129763F).GetUIntPtr(), uintptr_t(code3.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x129763F + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
		{
			//Right
			struct Patch4_Code : Xbyak::CodeGenerator {
				Patch4_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value, uintptr_t a_frameTimer) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label valueLabel;
					Xbyak::Label timerLabel;

					mov(r10, ptr[rip + valueLabel]);
					movss(xmm0, dword[r10]);
					mov(r10, ptr[rip + timerLabel]);
					mulss(xmm0, dword[r10]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(a_hookTarget + 0x8);

					L(valueLabel);
					dq(a_value);

					L(timerLabel);
					dq(a_frameTimer);
				}
			};
			void* codeBuf4 = g_localTrampoline.StartAlloc();
			Patch4_Code code4(codeBuf4, RelocAddr<uintptr_t>(0x12976A4).GetUIntPtr(), PanSpeedAddress, FrametimeAddress);
			g_localTrampoline.EndAlloc(code4.getCurr());
			g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x12976A4).GetUIntPtr(), uintptr_t(code4.getCode()));

			SafeWriteBuf(RelocAddr<uintptr_t>(0x12976A4 + 0x5).GetUIntPtr(), "\x90\x90\x90", 3);
		}
	}
	if (FixStuckAnimation) {
		struct Patch_Code : Xbyak::CodeGenerator {
			Patch_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;
				Xbyak::Label valueLabel;

				mov(r14, ptr[rip + valueLabel]);
				movss(xmm3, dword[r14]);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_hookTarget + 0x5);

				L(valueLabel);
				dq(a_value);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		Patch_Code code(codeBuf, RelocAddr<uintptr_t>(0x252E789).GetUIntPtr(), FPS60ValueAddress);
		g_localTrampoline.EndAlloc(code.getCurr());

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x252E789).GetUIntPtr(), uintptr_t(code.getCode()));
	}
	if (FixMotionResponsive) {
		float MotionFeedback = 294.1176470588235; //5
		SafeWriteBuf(MotionFeedBackAddress.GetUIntPtr(), &MotionFeedback, sizeof(float));

		struct Patch_Code : Xbyak::CodeGenerator {
			Patch_Code(void* buf, uintptr_t a_hookTarget, uintptr_t a_value) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;
				Xbyak::Label valueLabel;

				mov(r9, ptr[rip + valueLabel]);
				movss(xmm1, dword[r9]);
				mulss(xmm1, xmm6);
				comiss(xmm0, xmm1);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_hookTarget + 0x7);

				L(valueLabel);
				dq(a_value);
			}
		};
		void* codeBuf = g_localTrampoline.StartAlloc();
		Patch_Code code(codeBuf, RelocAddr<uintptr_t>(0x2844F0).GetUIntPtr(), MotionFeedBackAddress);
		g_localTrampoline.EndAlloc(code.getCurr());

		g_branchTrampoline.Write5Branch(RelocAddr<uintptr_t>(0x2844F0).GetUIntPtr(), uintptr_t(code.getCode()));
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
				GetProcess();
				_MESSAGE("Getting the number of processor threads...");
				GetNumberOfThreads();
				_MESSAGE("Getting the number of processor threads OK");
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

		if (EnableDisplayTweaks) {
			PatchDisplay();
			Hook();
		}

		//Disable Bethesda auto Vsync and FPS cap
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