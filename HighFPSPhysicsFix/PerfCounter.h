#pragma once

#include <chrono>
long long perf_freq;

	class PerfCounter
	{
	public:
		PerfCounter()
		{
			perf_freq = _Query_perf_frequency();
			perf_freqf = static_cast<float>(perf_freq);
		}

		template <typename T>
		__inline static T delta(long long tp1, long long tp2)
		{
			return static_cast<T>(tp2 - tp1) / perf_freqf;
		}

		__inline static long long deltal(long long tp1, long long tp2)
		{
			return ((tp2 - tp1) * 1000000) / perf_freq;
		}
	private:
		static long long perf_freq;
		static float perf_freqf;
		static PerfCounter m_Instance;
	};