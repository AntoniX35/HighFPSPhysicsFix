#pragma once

namespace HFPF
{
	namespace Game
	{
		inline static REL::Relocation<float*> g_frameTimer{ AID::FrameTimer, 0x21C };
		inline static REL::Relocation<float*> g_frameTimerSlow{ AID::FrameTimer, 0x218 };
	}
}
