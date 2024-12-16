#pragma once

#include <atomic>
#include <mutex>

#include <immintrin.h>

// non-reentrant
class FastSpinLock
{
	static inline constexpr std::uint64_t MAX_SPIN_CYCLES = 10000ULL;

public:
	FastSpinLock() noexcept = default;

	[[nodiscard]] inline bool try_lock() noexcept;
	void lock() noexcept;
	inline void unlock() noexcept;

private:
	std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
};

bool FastSpinLock::try_lock() noexcept
{
	return m_lock.test_and_set(std::memory_order_acquire) == false;
}

void FastSpinLock::unlock() noexcept
{
	m_lock.clear(std::memory_order_release);
}

class WCriticalSection
{
public:
	WCriticalSection() noexcept
	{
		InitializeCriticalSection(&m_cs);
	}

	~WCriticalSection() noexcept
	{
		DeleteCriticalSection(&m_cs);
	}

	WCriticalSection(const WCriticalSection&) = delete;
	WCriticalSection(WCriticalSection&&) = delete;
	WCriticalSection& operator=(const WCriticalSection&) = delete;
	WCriticalSection& operator=(WCriticalSection&&) = delete;

	inline void lock();
	inline void unlock();
	inline bool try_lock();

private:
	CRITICAL_SECTION m_cs;
};

void WCriticalSection::lock()
{
	EnterCriticalSection(&m_cs);
}

void WCriticalSection::unlock()
{
	LeaveCriticalSection(&m_cs);
}

bool WCriticalSection::try_lock()
{
	return TryEnterCriticalSection(&m_cs) == TRUE;
}

template <class T, class = std::enable_if_t<stl::is_any_base_of_v<T, FastSpinLock, WCriticalSection, std::mutex, std::recursive_mutex>, void>>
class IScopedLock
{
public:
	IScopedLock() = delete;

	IScopedLock(const IScopedLock&) = delete;
	IScopedLock(IScopedLock&&) = delete;
	IScopedLock& operator=(const IScopedLock&) = delete;
	IScopedLock& operator=(IScopedLock&&) = delete;

	inline IScopedLock(T& a_mutex) noexcept :
		m_mutex(a_mutex)
	{
		a_mutex.lock();
	}

	inline ~IScopedLock() noexcept
	{
		m_mutex.unlock();
	}

private:
	T& m_mutex;
};