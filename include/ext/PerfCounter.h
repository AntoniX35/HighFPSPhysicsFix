#pragma once

#include <profileapi.h>

class PerfCounter
{
public:
	PerfCounter() noexcept
	{
		if (::QueryPerformanceFrequency(&m_perf_freq) == FALSE)
		{
			m_perf_freq.QuadPart = 1ll;
		}
		else
		{
			if (!m_perf_freq.QuadPart)
			{
				m_perf_freq.QuadPart = 1ll;
			}

			m_perf_freqf = static_cast<double>(m_perf_freq.QuadPart);
		}
	}

	inline static long long Query() noexcept
	{
		LARGE_INTEGER t;
		if (::QueryPerformanceCounter(&t) == TRUE)
		{
			return t.QuadPart;
		}
		else
		{
			return 0ll;
		}
	}

	template <class T = float>
	inline constexpr T delta(
		long long tp1,
		long long tp2) const noexcept
	{
		return static_cast<T>(static_cast<double>(tp2 - tp1) / m_perf_freqf);
	}

	inline constexpr long long delta_us(
		long long tp1,
		long long tp2) const noexcept
	{
		return ((tp2 - tp1) * 1000000LL) / m_perf_freq.QuadPart;
	}

	inline constexpr long long T(
		long long tp) const noexcept
	{
		return (m_perf_freq.QuadPart / 1000000LL) * tp;
	}

private:
	LARGE_INTEGER m_perf_freq;
	double m_perf_freqf;
};

class IPerfCounter
{
public:
	inline static long long Query() noexcept
	{
		return m_Instance.Query();
	}

	template <class T = float>
	inline static constexpr auto delta(
		long long tp1,
		long long tp2) noexcept
	{
		return m_Instance.delta<T>(tp1, tp2);
	}

	inline static constexpr auto delta_us(
		long long tp1,
		long long tp2) noexcept
	{
		return m_Instance.delta_us(tp1, tp2);
	}

	inline static constexpr auto T(
		long long tp) noexcept
	{
		return m_Instance.T(tp);
	}

private:
	static PerfCounter m_Instance;
};

class PerfTimer
{
public:
	inline void Start()
	{
		m_tStart = IPerfCounter::Query();
	}

	inline constexpr auto Stop() const noexcept
	{
		return IPerfCounter::delta(m_tStart, IPerfCounter::Query());
	}

private:
	long long m_tStart{ 0 };
};

class PerfTimerInt
{
public:
	PerfTimerInt(long long a_interval) :
		m_interval(a_interval),
		m_tIntervalBegin(IPerfCounter::Query())
	{
	}

	inline void Begin() noexcept
	{
		m_tStart = IPerfCounter::Query();
	}

	inline void End() noexcept
	{
		End(m_tLast);
	}

	inline bool End(long long& a_out) noexcept
	{
		auto tEnd = IPerfCounter::Query();

		m_tAccum += IPerfCounter::delta_us(m_tStart, tEnd);
		m_tCounter++;

		m_tInterval = IPerfCounter::delta_us(m_tIntervalBegin, tEnd);
		if (m_tInterval >= m_interval)
		{
			if (m_tCounter > 0)
			{
				a_out = m_tAccum / m_tCounter;

				m_tCounter = 0;
				m_tAccum = 0;
				m_tIntervalBegin = tEnd;
			}
			else  // overflow
			{
				a_out = 0;
				Reset();
			}

			return true;
		}

		return false;
	}

	inline void SetInterval(long long a_interval) noexcept
	{
		m_interval = a_interval;
	}

	inline void Reset() noexcept
	{
		m_tIntervalBegin = IPerfCounter::Query();
		m_tAccum = 0;
		m_tCounter = 0;
		m_tLast = 0;
	}

	inline constexpr auto GetIntervalTime() const noexcept
	{
		return m_tInterval;
	}

	inline constexpr auto NodeProcessorGetTime() const noexcept
	{
		return m_tLast;
	}

private:
	long long m_interval;
	long long m_tIntervalBegin;

	long long m_tStart{ 0 };
	long long m_tCounter{ 0 };
	long long m_tAccum{ 0 };
	long long m_tInterval{ 0 };
	long long m_tLast{ 0 };
};