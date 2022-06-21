#include "pch.h"

namespace SDT
{
    IF4SE IF4SE::m_Instance;

    void IF4SE::OnLogOpen()
    {
    }

    bool IF4SE::CheckInterfaceVersion(UInt32 a_interfaceID, UInt32 a_interfaceVersion, UInt32 a_compiledInterfaceVersion) const
    {
        return true;
    }
}
