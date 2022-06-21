#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace SDT
{
    IDDispatcher IDDispatcher::m_Instance;

    void IDDispatcher::RegisterDriver(IDriver* const drv)
    {
        m_Instance.m_drivers.emplace_back(drv);
    }

    bool IDDispatcher::DriverOK(DRIVER_ID const id)
    {
        auto it = m_Instance.m_drivermap.find(id);
        if (it != m_Instance.m_drivermap.end()) {
            return it->second->IsOK();
        }
        else {
            return false;
        }
    }

    IDriver* IDDispatcher::GetDriver(DRIVER_ID const id)
    {
        auto it = m_Instance.m_drivermap.find(id);
        if (it != m_Instance.m_drivermap.end()) {
            return it->second;
        }
        else {
            return nullptr;
        }
    }

    bool IDDispatcher::InitializeDrivers()
    {
        return m_Instance.InitializeDrivers_Impl();
    }

    bool IDDispatcher::InitializeDrivers_Impl()
    {
        PreProcessDrivers();
        std::sort(m_drivers.begin(), m_drivers.end(),
            [](const auto& a, const auto& b) {
                return a->GetPriority() < b->GetPriority();
            });
        for (const auto& drv : m_drivers)
        {
            drv->SetOK(drv->Prepare());

            if (drv->IsOK()) {
                continue;
            }

            if (drv->IsEssential()) {
                _MESSAGE("[Dispatcher] Essential driver check failed: %s", drv->ModuleName());
                return false;
            }
            else {
                _MESSAGE("[Dispatcher] Driver check failed: %s", drv->ModuleName());
            }
        }
        decltype(m_drivers)::size_type count = 0;

        for (const auto& drv : m_drivers)
        {
            if (!drv->IsOK()) {
                continue;
            }

            _MESSAGE("[Dispatcher] Initializing: %s", drv->ModuleName());

            if (!drv->Initialize()) {
                _MESSAGE("[Dispatcher] Driver initialization failed: %s", drv->ModuleName());
                return false;
            }

            count++;
        }

        for (const auto& drv : m_drivers)
        {
            if (drv->IsOK()) {
                drv->PostInit();
            }
        }

        _MESSAGE("[Dispatcher] %zu driver(s) initialized", count);

        m_drivers.swap(decltype(m_drivers)());

        return true;
    }

    void IDDispatcher::PreProcessDrivers()
    {
        for (const auto& drv : m_drivers)
        {
            ASSERT(drv->GetPriority() > -1);

            auto& e = m_drivermap.try_emplace(drv->GetID(), drv);

            ASSERT(e.second == true);
        }
    }
}