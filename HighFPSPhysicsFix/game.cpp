#include "pch.h"

namespace SDT
{

    //RelocAddr<uintptr_t> FrametimeAddress(0x5A66FE8);
    namespace Game
    {
        const int* g_extInt = reinterpret_cast<const int*>(SDT::DRender::ExteriorInteriorAddress);
        const float* g_frameTimer = reinterpret_cast<const float*>(SDT::DRender::FrametimeAddress);
    }
}