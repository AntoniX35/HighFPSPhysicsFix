#pragma once

namespace Hook
{
#pragma pack(push, 1)
	struct CB5Code
	{
		std::uint8_t opcode;
		std::int32_t displ;
	};

	struct CB6Code
	{
		std::uint8_t escape;
		std::uint8_t modrm;
		std::int32_t displ;
	};
#pragma pack(pop)

	template <std::uint8_t _opcode>
	constexpr bool CheckDst5(std::uintptr_t a_addr)
	{
		static_assert(_opcode == std::uint8_t(0xE8) || _opcode == std::uint8_t(0xE9), "invalid opcode");

		auto ins = reinterpret_cast<CB5Code*>(a_addr);

		return ins->opcode == _opcode;
	}

	template <std::uint8_t _opcode, class T>
	constexpr bool GetDst5(
		std::uintptr_t a_addr,
		T&             a_out)
	{
		static_assert(_opcode == std::uint8_t(0xE8) || _opcode == std::uint8_t(0xE9), "invalid opcode");

		auto ins = reinterpret_cast<CB5Code*>(a_addr);

		if (ins->opcode != _opcode) {
			return false;
		}

		auto oa = a_addr + sizeof(CB5Code) + ins->displ;

		if constexpr (std::is_same_v<T, std::uintptr_t>) {
			a_out = oa;
		} else {
			a_out = reinterpret_cast<T>(oa);
		}

		return true;
	}

	template <std::uint8_t _modrm, class T>
	constexpr bool GetDst6(
		std::uintptr_t a_addr,
		T&             a_out)
	{
		static_assert(_modrm == std::uint8_t(0x15) || _modrm == std::uint8_t(0x25), "invalid modr/m byte");

		auto ins = reinterpret_cast<CB6Code*>(a_addr);

		if (ins->escape != 0xFF || ins->modrm != _modrm) {
			return false;
		}

		auto oa = *reinterpret_cast<std::uintptr_t*>(a_addr + sizeof(CB6Code) + ins->displ);

		if constexpr (std::is_same_v<T, std::uintptr_t>) {
			a_out = oa;
		} else {
			a_out = reinterpret_cast<T>(oa);
		}

		return true;
	}

	template <class T>
	constexpr bool Call5(
		F4SE::Trampoline& a_trampoline,
		std::uintptr_t    a_addr,
		std::uintptr_t    a_dst,
		T&                a_orig)
	{
		std::uintptr_t o;
		if (!GetDst5<0xE8>(a_addr, o)) {
			return false;
		}

		if constexpr (std::is_same_v<T, std::uintptr_t>) {
			a_orig = o;
		} else {
			a_orig = reinterpret_cast<T>(o);
		}

		a_trampoline.write_call<5>(a_addr, a_dst);

		return true;
	}

	template <class T>
	constexpr bool Jmp5(
		F4SE::Trampoline& a_trampoline,
		std::uintptr_t    a_addr,
		std::uintptr_t    a_dst,
		T&                a_orig)
	{
		std::uintptr_t o;

		if (!GetDst5<0xE9>(a_addr, o)) {
			return false;
		}

		if constexpr (std::is_same_v<T, std::uintptr_t>) {
			a_orig = o;
		} else {
			a_orig = reinterpret_cast<T>(o);
		}

		a_trampoline.write_branch<5>(a_addr, a_dst);

		return true;
	}


	template <class T>
	constexpr bool Call6(
		F4SE::Trampoline& a_trampoline,
		std::uintptr_t    a_addr,
		std::uintptr_t    a_dst,
		T&                a_orig)
	{
		std::uintptr_t o;

		if (!GetDst6<0x15>(a_addr, o)) {
			return false;
		}

		if constexpr (std::is_same_v<T, std::uintptr_t>) {
			a_orig = o;
		} else {
			a_orig = reinterpret_cast<T>(o);
		}

		a_trampoline.write_call<6>(a_addr, a_dst);

		return true;
	}

	template <class T>
	constexpr bool Jmp6(
		F4SE::Trampoline& a_trampoline,
		std::uintptr_t    a_addr,
		std::uintptr_t    a_dst,
		T&                a_orig)
	{
		std::uintptr_t o;

		if (!GetDst6<0x25>(a_addr, o)) {
			return false;
		}

		if constexpr (std::is_same_v<T, std::uintptr_t>) {
			a_orig = o;
		} else {
			a_orig = reinterpret_cast<T>(o);
		}

		a_trampoline.write_branch<6>(a_addr, a_dst);

		return true;
	}
}
