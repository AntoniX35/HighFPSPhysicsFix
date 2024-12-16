#pragma once

#include <algorithm>
#include <limits>

namespace Math
{
    SKMP_FORCEINLINE constexpr float Normalize(float a_val, float a_min, float a_max) {
        return (a_val - a_min) / (a_max - a_min);
    }

    SKMP_FORCEINLINE constexpr float NormalizeClamp(float a_val, float a_min, float a_max) {
        return std::clamp(Normalize(a_val, a_min, a_max), 0.0f, 1.0f);
    }

    template <class T>
    SKMP_FORCEINLINE constexpr bool IsEqual(T a_lhs, T a_rhs)
    {
        return std::abs(a_lhs - a_rhs) <= std::numeric_limits<T>::epsilon() * std::abs(a_lhs);
    }

	SKMP_FORCEINLINE constexpr float NormalizeSafe(float a_val, float a_min, float a_max)
	{
		float s = (a_max - a_min);
		if (IsEqual(s, 0.0f))
		{
			return 0.0f;
		}
		return (a_val - a_min) / s;
	}

	SKMP_FORCEINLINE constexpr float NormalizeSafeClamp(float a_val, float a_min, float a_max)
	{
		return std::clamp(NormalizeSafe(a_val, a_min, a_max), 0.0f, 1.0f);
	}

    template <class T>
    SKMP_FORCEINLINE constexpr T zero_nan(T a_value)
    {
        return std::isnan(a_value) ? 0.0f : a_value;
    }

}
