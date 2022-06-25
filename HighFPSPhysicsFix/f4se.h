#pragma once

#include <f4se/IF4SE.h>
#include <f4se/ISettingCollection.h>

namespace SDT
{
    static inline constexpr std::size_t MAX_TRAMPOLINE_BRANCH = 320;
    static inline constexpr std::size_t MAX_TRAMPOLINE_CODEGEN = 1024;

    class IF4SE :
        public IF4SEBase<F4SEInterfaceFlags::kTrampoline | F4SEInterfaceFlags::kMessaging, 440ui64, 1120ui64>,
        public ISettingCollection
    {
    public:

        [[nodiscard]] __forceinline static auto& GetSingleton() {
            return m_Instance;
        }

        [[nodiscard]] __forceinline static auto& GetBranchTrampoline() {
            return m_Instance.GetTrampoline(TrampolineID::kBranch);
        }

        [[nodiscard]] __forceinline static auto& GetLocalTrampoline() {
            return m_Instance.GetTrampoline(TrampolineID::kLocal);
        }

    private:
        IF4SE() = default;

        virtual void OnLogOpen() override;
        virtual bool CheckInterfaceVersion(UInt32 a_interfaceID, UInt32 a_interfaceVersion, UInt32 a_compiledInterfaceVersion) const override;

        static IF4SE m_Instance;
    };
}

