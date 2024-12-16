#include "Mem.h"

namespace mem
{
#if defined(SKMP_MEMDBG)
    std::atomic_size_t g_allocatedSize = 0ULL;
#endif
}