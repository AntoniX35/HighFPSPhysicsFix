#pragma once

#include <type_traits>

namespace stl
{
	template <typename T>
	struct remove_all_pointers
	{
	public:
		using type = T;
	};

	template <typename T>
	struct remove_all_pointers<T*>
	{
	public:
		using type = typename remove_all_pointers<T>::type;
	};

	template <class T>
	using remove_all_pointers_t = typename remove_all_pointers<T>::type;

	template <class T>
	using strip_type = std::remove_cv_t<std::remove_pointer_t<std::remove_all_extents_t<remove_all_pointers_t<std::remove_reference_t<T>>>>>;

	template <class _Ty, class... _Types>
	inline constexpr bool is_any_base_of_v =
		std::disjunction_v<std::is_base_of<_Types, _Ty>...>;

	template <class _Ty, class... _Types>
	inline constexpr bool is_any_same_v =
		std::disjunction_v<std::is_same<_Types, _Ty>...>;

	template <class _Ty, class... _Types>
	using is_any_same = std::disjunction<
		std::is_same<_Types, _Ty>...>;

	/*template <class T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;*/

	template <class _Iter>
	[[nodiscard]] constexpr void* voidify_iter(_Iter _It) noexcept
	{
		if constexpr (std::is_pointer_v<_Iter>)
		{
			return const_cast<void*>(static_cast<const volatile void*>(_It));
		}
		else
		{
			return const_cast<void*>(static_cast<const volatile void*>(std::addressof(*_It)));
		}
	}

	template <class T, class... Args>
	constexpr void construct_in_place(T& a_obj, Args&&... a_args) noexcept(
		std::is_nothrow_constructible_v<T, Args...>)
	{
		if (std::is_constant_evaluated())
		{
			std::construct_at(std::addressof(a_obj), std::forward<Args>(a_args)...);
		}
		else
		{
			new (stl::voidify_iter(std::addressof(a_obj))) T(std::forward<Args>(a_args)...);
		}
	}

	template <class T>
	constexpr void destroy_in_place(T& a_obj) noexcept
	{
		static_assert(!std::is_array_v<T>);
		a_obj.~T();
	}

	template <class T, class... Args>
	constexpr void emplace(T& a_obj, Args&&... a_args) noexcept(
		std::is_nothrow_constructible_v<T, Args...>)
	{
		destroy_in_place(a_obj);
		construct_in_place(a_obj, std::forward<Args>(a_args)...);
	}

}