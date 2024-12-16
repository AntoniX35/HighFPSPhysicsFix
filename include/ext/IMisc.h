#pragma once

#include <type_traits>

namespace stl
{
	template <class T>
	constexpr typename std::underlying_type<T>::type underlying(T a_value) noexcept
	{
		return static_cast<typename std::underlying_type<T>::type>(a_value);
	}

	template <class T>
	constexpr typename std::underlying_type<T>::type* underlying(T* a_ptr) noexcept
	{
		return reinterpret_cast<typename std::underlying_type<T>::type*>(a_ptr);
	}

	template <class T>
	using is_scoped = std::integral_constant<bool, !std::is_convertible_v<T, int> && std::is_enum_v<T>>;

	template <
		class T,
		class value_type = std::remove_cvref_t<T>,
		class = std::enable_if_t<
			std::is_enum_v<value_type>,
			void>>
	struct flag
	{
		flag() = delete;

		inline flag(const T& a_rhs) noexcept :
			value(a_rhs)
		{
		}

		inline flag& operator=(value_type const a_rhs) noexcept
		{
			value = a_rhs;
			return *this;
		}

		template <
			class Ta,
			class = std::enable_if_t<
				std::is_same_v<
					Ta,
					std::underlying_type_t<value_type>>>>
		inline flag(Ta const a_rhs) noexcept :
			value(static_cast<value_type>(a_rhs))
		{
		}

		template <
			class Ta,
			class = std::enable_if_t<
				std::is_same_v<
					Ta,
					std::underlying_type_t<value_type>>>>
		inline flag& operator=(Ta const a_rhs) noexcept
		{
			value = static_cast<value_type>(a_rhs);
			return *this;
		}

		[[nodiscard]] inline constexpr bool test(value_type const a_rhs) const noexcept
		{
			return (value & a_rhs) == a_rhs;
		}

		[[nodiscard]] inline constexpr bool test_any(value_type const a_rhs) const noexcept
		{
			return (value & a_rhs) != static_cast<value_type>(0);
		}

		inline constexpr void set(value_type const a_rhs) noexcept
		{
			value |= a_rhs;
		}

		inline constexpr void toggle(value_type const a_rhs) noexcept
		{
			value ^= a_rhs;
		}

		inline constexpr void lshift(std::uint32_t const a_offset) noexcept
		{
			value <<= a_offset;
		}

		inline constexpr void rshift(std::uint32_t const a_offset) noexcept
		{
			value >>= a_offset;
		}

		inline constexpr void clear(value_type const a_rhs) noexcept
		{
			value &= ~a_rhs;
		}

		[[nodiscard]] inline constexpr operator T() const noexcept
		{
			return value;
		}

		[[nodiscard]] inline constexpr auto underlying() const noexcept
		{
			return static_cast<std::underlying_type_t<value_type>>(value);
		}

		T value;
	};

	enum class flag_test_dummy : std::uint32_t
	{
	};

	static_assert(offsetof(flag<flag_test_dummy>, value) == 0);
	static_assert(sizeof(flag<flag_test_dummy>) == sizeof(std::underlying_type_t<flag_test_dummy>));
}

#define DEFINE_ENUM_CLASS_BITWISE(x)                                             \
	inline constexpr x operator|(x a_lhs, x a_rhs) noexcept                      \
	{                                                                            \
		return static_cast<x>(stl::underlying(a_lhs) | stl::underlying(a_rhs));  \
	}                                                                            \
	inline constexpr x operator&(x a_lhs, x a_rhs) noexcept                      \
	{                                                                            \
		return static_cast<x>(stl::underlying(a_lhs) & stl::underlying(a_rhs));  \
	}                                                                            \
	inline constexpr x operator^(x a_lhs, x a_rhs) noexcept                      \
	{                                                                            \
		return static_cast<x>(stl::underlying(a_lhs) ^ stl::underlying(a_rhs));  \
	}                                                                            \
	inline constexpr x operator~(x a_lhs) noexcept                               \
	{                                                                            \
		return static_cast<x>(~stl::underlying(a_lhs));                          \
	}                                                                            \
	inline constexpr x operator<<(x a_lhs, std::uint32_t a_offset) noexcept      \
	{                                                                            \
		return static_cast<x>(stl::underlying(a_lhs) << a_offset);               \
	}                                                                            \
	inline constexpr x operator>>(x a_lhs, std::uint32_t a_offset) noexcept      \
	{                                                                            \
		return static_cast<x>(stl::underlying(a_lhs) >> a_offset);               \
	}                                                                            \
	inline constexpr x& operator>>=(x& a_lhs, std::uint32_t a_offset) noexcept   \
	{                                                                            \
		a_lhs = static_cast<x>(stl::underlying(a_lhs) >> a_offset);              \
		return a_lhs;                                                            \
	}                                                                            \
	inline constexpr x& operator<<=(x& a_lhs, std::uint32_t a_offset) noexcept   \
	{                                                                            \
		a_lhs = static_cast<x>(stl::underlying(a_lhs) << a_offset);              \
		return a_lhs;                                                            \
	}                                                                            \
	inline constexpr x& operator|=(x& a_lhs, x a_rhs) noexcept                   \
	{                                                                            \
		a_lhs = static_cast<x>(stl::underlying(a_lhs) | stl::underlying(a_rhs)); \
		return a_lhs;                                                            \
	}                                                                            \
	inline constexpr x& operator&=(x& a_lhs, x a_rhs) noexcept                   \
	{                                                                            \
		a_lhs = static_cast<x>(stl::underlying(a_lhs) & stl::underlying(a_rhs)); \
		return a_lhs;                                                            \
	}                                                                            \
	inline constexpr x& operator^=(x& a_lhs, x a_rhs) noexcept                   \
	{                                                                            \
		a_lhs = static_cast<x>(stl::underlying(a_lhs) ^ stl::underlying(a_rhs)); \
		return a_lhs;                                                            \
	}
