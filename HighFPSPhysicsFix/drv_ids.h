#pragma once

namespace SDT
{
    enum class DRIVER_ID : std::uint32_t
    {
        INVALID,
        EVENTS,
        WINDOW,
        OSD,
        INPUT,
        MISC,
        RENDER,
        PAPYRUS
    };
}