#include "Threads.h"

void FastSpinLock::lock() noexcept
{
    while (!try_lock())
    {
        /*auto start = __rdtsc();

        do
        {
            _mm_pause();

            if (try_lock())
            {
                return;
            }

        } while ((__rdtsc() - start) < MAX_SPIN_CYCLES);*/

        _mm_pause();

        if (try_lock()) {
            return;
        }

        SwitchToThread();
    }
}