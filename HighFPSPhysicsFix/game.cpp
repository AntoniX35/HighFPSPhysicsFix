#include "pch.h"

namespace SDT
{

    //RelocAddr<uintptr_t> FrametimeAddress(0x5A66FE8);
    namespace Game
    {
        const float* g_frameTimer = reinterpret_cast<const float*>(SDT::DRender::FrametimeAddress);
    }
}