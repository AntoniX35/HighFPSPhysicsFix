#pragma once

namespace HFPF
{
	namespace AID
	{
		constexpr REL::ID FPS_Cap_Patch1(2228907);
		constexpr REL::ID FullScreen_Patch1(2227231);
		constexpr REL::ID ResizeBuffers(2276824);
		constexpr REL::ID ResizeTarget(2276825);
		constexpr REL::ID D3D11Create(2277018);
		constexpr REL::ID LoadScreenPlusLimiterInject(2276834);
		constexpr REL::ID FrameTimer(2696498);
		constexpr REL::ID CreateWindowEx_a(2276814);
		constexpr REL::ID Upscale(2276833);
		constexpr REL::ID Untie(2267969);
		constexpr REL::ID BlackLoadingScreens(2249217);
		constexpr REL::ID LoadingScreens(2227631);
		constexpr REL::ID PostLoadInject(2248711);
		constexpr REL::ID ExtInt(359440);
		constexpr REL::ID FixStuttering1(2277709);
		constexpr REL::ID FixStuttering2(2277710);
		constexpr REL::ID ObjectsTransfer(2255886);
		constexpr REL::ID FixWhiteScreen(2258401);
		constexpr REL::ID FixWindSpeed1(2278751);
		constexpr REL::ID FixWindSpeed2(2277711);
		constexpr REL::ID FixRotationSpeed(2234879);
		constexpr REL::ID FixLockpickRotation(2249260);
		constexpr REL::ID FixWSRotationSpeed(2195211);
		constexpr REL::ID FixRepeateRate(2249218);
		constexpr REL::ID FixTriggerZoomSpeed(2249220);
		constexpr REL::ID FixLoadScreenRotationSpeed(2249233);
		constexpr REL::ID FixStuckAnim(2302542);
		constexpr REL::ID FixMotionFeedback(2196089);
		constexpr REL::ID FixSittingRotationX(2248271);
		constexpr REL::ID ActorFade(2214659);
		constexpr REL::ID PlayerFade(2248393);
		constexpr REL::ID BudgetGame(2251303);
		constexpr REL::ID BudgetUI(2251305);
		constexpr REL::ID Budget(2251306);
		constexpr REL::ID Write_iLocationX(2228990);
	}

	namespace Offsets
	{
		static inline constexpr std::uintptr_t FPS_Cap_Patch1 = 0xBA7;                   //Fallout4.exe+0x00BAB067
		static inline constexpr std::uintptr_t FPS_Cap_Patch2 = 0xBB0;                   //Fallout4.exe+0x00BAB070
		static inline constexpr std::uintptr_t Borderless_Patch = 0xB5E;                 //Fallout4.exe+0x00BAB01E
		static inline constexpr std::uintptr_t FullScreen_Patch1 = 0xC7;                 //Fallout4.exe+0x00B414A7
		static inline constexpr std::uintptr_t FullScreen_Patch3 = 0xF8;                 //Fallout4.exe+0x00B414D8
		static inline constexpr std::uintptr_t Screen_Patch = 0xB54;                     //Fallout4.exe+0x00BAB014
		static inline constexpr std::uintptr_t MovRaxRcx = 0x1D6;                        //Fallout4.exe+0x016FC8B6
		static inline constexpr std::uintptr_t ResizeBuffersDisable = 0x28;              //Fallout4.exe+0x016FC708
		static inline constexpr std::uintptr_t ResizeTargetDisable = 0x27;               //Fallout4.exe+0x016FCAB7
		static inline constexpr std::uintptr_t ResizeBuffers = 0x1C1;                    //Fallout4.exe+0x016FC8A1
		static inline constexpr std::uintptr_t ResizeTarget = 0x108;                     //Fallout4.exe+0x016FCB98
		static inline constexpr std::uintptr_t CreateDXGIFactory = 0x2B;                 //Fallout4.exe+0x0170968B
		static inline constexpr std::uintptr_t D3D11CreateDeviceAndSwapChain = 0x411;    //Fallout4.exe+0x01709A71
		static inline constexpr std::uintptr_t CreateWindowEx_a = 0x285;                 //Fallout4.exe+0x016FB045
		static inline constexpr std::uintptr_t BlackLoadingScreens = 0x116;              //Fallout4.exe+0x00FE08C6
		static inline constexpr std::uintptr_t LoadingScreens = 0x223;                   //Fallout4.exe+0x00B57D03
		static inline constexpr std::uintptr_t PostLoadInject = 0x38;                    //Fallout4.exe+0x00FBF928
		static inline constexpr std::uintptr_t BethesdaVsync = 0x332;                    //Fallout4.exe+0x01709992
		static inline constexpr std::uintptr_t LoadScreenPlusLimiterInject = 0xE;        //Fallout4.exe+0x016FD27E
		static inline constexpr std::uintptr_t PresentInject = 0x48;                     //Fallout4.exe+0x016FD2B8
		static inline constexpr std::uintptr_t Untie = 0x5F;                             //Fallout4.exe+0x015418AF
		static inline constexpr std::uintptr_t FixStuttering1 = 0x169;                   //Fallout4.exe+0x017590F9
		static inline constexpr std::uintptr_t FixStuttering2 = 0x19B;                   //Fallout4.exe+0x0175930B
		static inline constexpr std::uintptr_t FixStuttering3 = 0x122;                   //Fallout4.exe+0x017590B2
		static inline constexpr std::uintptr_t FixWhiteScreen = 0x10;                    //Fallout4.exe+0x01296F00
		static inline constexpr std::uintptr_t FixWindSpeed1 = 0x24;                     //Fallout4.exe+0x017A3814
		static inline constexpr std::uintptr_t FixWindSpeed2 = 0x115;                    //Fallout4.exe+0x01759F85
		static inline constexpr std::uintptr_t FixWindSpeed3 = 0x1B7;                    //Fallout4.exe+0x0175A027
		static inline constexpr std::uintptr_t FixWindSpeed4 = 0x3BD;                    //Fallout4.exe+0x0175A22D
		static inline constexpr std::uintptr_t FixRotationSpeed = 0x6E;                  //Fallout4.exe+0x00D5995E
		static inline constexpr std::uintptr_t FixLockpickRotation = 0x42;               //Fallout4.exe+0x00FE52D2
		static inline constexpr std::uintptr_t FixWSRotationSpeed = 0x94;                //Fallout4.exe+0x0034D8D4
		static inline constexpr std::uintptr_t FixRepeateRate = 0x3F5;                   //Fallout4.exe+0x00FE0D65
		static inline constexpr std::uintptr_t FixLeftTriggerZoomSpeed = 0xDD;           //Fallout4.exe+0x00FE116D
		static inline constexpr std::uintptr_t FixRightTriggerZoomSpeed = 0x11E;         //Fallout4.exe+0x00FE11AE
		static inline constexpr std::uintptr_t FixLoadScreenRotationSpeedUp = 0x525;     //Fallout4.exe+0x00FE0E95
		static inline constexpr std::uintptr_t FixLoadScreenRotationSpeedDown = 0x58A;   //Fallout4.exe+0x00FE0EFA
		static inline constexpr std::uintptr_t FixLoadScreenRotationSpeedLeft = 0x5EE;   //Fallout4.exe+0x00FE0F5E
		static inline constexpr std::uintptr_t FixLoadScreenRotationSpeedRight = 0x65E;  //Fallout4.exe+0x00FE0FCE
		static inline constexpr std::uintptr_t FixLoadScreenRotationSpeed = 0xBA;        //Fallout4.exe+0x00FE26AA
		static inline constexpr std::uintptr_t FixStuckAnim = 0xA9;                      //Fallout4.exe+0x01D5F9A9
		static inline constexpr std::uintptr_t FixMotionFeedback = 0x9FE;                //Fallout4.exe+0x0039353E
		static inline constexpr std::uintptr_t FixSittingRotationX = 0xC0;               //Fallout4.exe+0x00F9BD20
		static inline constexpr std::uintptr_t FixSittingRotationY = 0xDE;               //Fallout4.exe+0x00F9BD3E
		static inline constexpr std::uintptr_t Upscale = 0x1BA;                          //Fallout4.exe+0x016FD1DA
		static inline constexpr std::uintptr_t ActorFade = 0xAED;                        //Fallout4.exe+0x007CA1FD
		static inline constexpr std::uintptr_t PlayerFade = 0x169;                       //Fallout4.exe+0x00FA33D9
		static inline constexpr std::uintptr_t iSizeW = 0xB68;                           //Fallout4.exe+0x00BAB028
		static inline constexpr std::uintptr_t iSizeH = 0xB72;                           //Fallout4.exe+0x00BAB032
		static inline constexpr std::uintptr_t BudgetGame = 0x3C;                        //Fallout4.exe+0x0106406C
		static inline constexpr std::uintptr_t BudgetUI = 0xB4;                          //Fallout4.exe+0x01064374
		static inline constexpr std::uintptr_t Budget = 0xB4;                            //Fallout4.exe+0x01064484
		static inline constexpr std::uintptr_t LoadPluginINI_C = 0x5A1;                  //Fallout4.exe+0x00BAAA61
		static inline constexpr std::uintptr_t Write_iLocationX = 0x269;                 //Fallout4.exe+0x00BAAA61
		static inline constexpr std::uintptr_t Write_iLocationY = 0x2A4;                 //Fallout4.exe+0x00BAAA61
	}

	namespace Payloads
	{
		static inline constexpr std::uint8_t JMP8 = { 0xEB };
		static inline constexpr std::uint8_t INT3 = { 0xCC };
		static inline constexpr std::uint8_t NOP = { 0x90 };
		static inline constexpr std::uint8_t NOP2[] = { 0x66, 0x90 };
		static inline constexpr std::uint8_t NOP3[] = { 0x0F, 0x1F, 0x00 };
		static inline constexpr std::uint8_t NOP4[] = { 0x0F, 0x1F, 0x40, 0x00 };
		static inline constexpr std::uint8_t NOP5[] = { 0x0F, 0x1F, 0x44, 0x00, 0x00 };
		static inline constexpr std::uint8_t NOP6[] = { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 };
		static inline constexpr std::uint8_t NOP8[] = { 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };

		static inline constexpr std::uint8_t screen_patch[] = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90 };
		static inline constexpr std::uint8_t fullscreen1_patch[] = { 0x41, 0x80, 0xFB, 0x01, 0x90, 0x90, 0x90 };
		static inline constexpr std::uint8_t fullscreenJMP_patch[] = { 0xEB, 0x18 };
		static inline constexpr std::uint8_t fullscreenNOP_patch[] = { 0x90, 0x90 };
		static inline constexpr std::uint8_t untie_patch[] = { 0x00 };
		static inline constexpr std::uint8_t ifpsclamp_patch[] = { 0x38 };
		static inline constexpr std::uint8_t disable_blackloading_patch[] = { 0xEB };
		static inline constexpr std::uint8_t ResizeBuffersDisable[] = { 0xE9, 0x6A, 0x03, 0x00, 0x00, 0x90 };
		static inline constexpr std::uint8_t ResizeTargetDisable[] = { 0xE9, 0x3C, 0x01, 0x00, 0x00, 0x90 };
		static inline constexpr std::uint8_t res_patch[] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0x90 };

		static inline constexpr std::uint8_t SkipNoINI[] = { 0x48, 0x8B, 0xCF };

		static inline constexpr std::uint8_t player_fade_jmp[] = { 0xEB, 0x6C, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	}
}
