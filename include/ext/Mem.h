#pragma once

#include "ICommon.h"

#include <stdexcept>
//#include <xstddef>

namespace mem
{
#if defined(SKMP_MEMDBG)
    extern std::atomic_size_t g_allocatedSize;
#endif

    /**
 * Allocator for aligned data.
 *
 * Modified from the Mallocator from Stephan T. Lavavej.
 * <http://blogs.msdn.com/b/vcblog/archive/2008/08/28/the-mallocator.aspx>
 */
    template <typename T, std::size_t Alignment>
    class aligned_allocator
    {
    public:

        // The following will be the same for virtually all allocators.
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef T value_type;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        SKMP_FORCEINLINE T* address(T& r) const
        {
            return std::addressof(r);
        }

        SKMP_FORCEINLINE const T* address(const T& s) const
        {
            return std::addressof(s);
        }

        SKMP_FORCEINLINE std::size_t max_size() const
        {
            // The following has been carefully written to be independent of
            // the definition of size_t and to avoid signed/unsigned warnings.
            return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) / sizeof(T);
        }


        // The following must be the same for all allocators.
        template <typename U>
        struct rebind
        {
            typedef aligned_allocator<U, Alignment> other;
        };

        SKMP_FORCEINLINE bool operator!=(const aligned_allocator& other) const
        {
            return !(*this == other);
        }

        SKMP_FORCEINLINE void construct(T* const p, const T& t) const
        {
            void* const pv = static_cast<void*>(p);

            new (pv) T(t);
        }

        SKMP_FORCEINLINE void construct(T* const p, T&& t) const
        {
            void* const pv = static_cast<void*>(p);

            new (pv) T(std::move(t));
        }

        SKMP_FORCEINLINE void destroy(T* const p) const
        {
            p->~T();
        }

        // Returns true if and only if storage allocated from *this
        // can be deallocated from other, and vice versa.
        // Always returns true for stateless allocators.
        SKMP_FORCEINLINE bool operator==(const aligned_allocator& other) const
        {
            return true;
        }

        // Default constructor, copy constructor, rebinding constructor, and destructor.
        // Empty for stateless allocators.
        aligned_allocator() { }

        aligned_allocator(const aligned_allocator&) { }

        template <typename U> aligned_allocator(const aligned_allocator<U, Alignment>&) { }

        ~aligned_allocator() { }


        // The following will be different for each allocator.
        SKMP_FORCEINLINE T* allocate(const std::size_t n) const
        {
            // The return value of allocate(0) is unspecified.
            // Mallocator returns NULL in order to avoid depending
            // on malloc(0)'s implementation-defined behavior
            // (the implementation can define malloc(0) to return NULL,
            // in which case the bad_alloc check below would fire).
            // All allocators can return NULL in this case.
            if (n == 0) {
                return NULL;
            }

            // All allocators should contain an integer overflow check.
            // The Standardization Committee recommends that std::length_error
            // be thrown in the case of integer overflow.
            if (n > max_size())
            {
                throw std::length_error("aligned_allocator<T>::allocate() - Integer overflow.");
            }

            size_t size = n * sizeof(T);

            // Mallocator wraps malloc().
            void* const pv = _mm_malloc(size, Alignment);

#if defined(SKMP_MEMDBG)
            g_allocatedSize += size;
#endif

            // Allocators should throw std::bad_alloc in the case of memory allocation failure.
            if (pv == NULL)
            {
                throw std::bad_alloc();
            }

            return static_cast<T*>(pv);
        }

        SKMP_FORCEINLINE void deallocate(T* const p, const std::size_t n) const
        {
#if defined(SKMP_MEMDBG)
            g_allocatedSize -= n * sizeof(Td);
#endif

            _mm_free(p);
        }


        // The following will be the same for all allocators that ignore hints.
        template <typename U>
        SKMP_FORCEINLINE T* allocate(const std::size_t n, const U* /* const hint */) const
        {
            return allocate(n);
        }


        // Allocators are not required to be assignable, so
        // all allocators should have a private unimplemented
        // assignment operator. Note that this will trigger the
        // off-by-default (enabled under /Wall) warning C4626
        // "assignment operator could not be generated because a
        // base class assignment operator is inaccessible" within
        // the STL headers, but that warning is useless.
    private:
        aligned_allocator& operator=(const aligned_allocator&);

    };

};

#define SKMP_DECLARE_ALIGNED_ALLOCATOR(x)                                                                     \
	SKMP_FORCEINLINE void *operator new(size_t sizeInBytes) { void* const ptr = _mm_malloc(sizeInBytes, x); return ptr ? ptr : throw std::bad_alloc(); }   \
	SKMP_FORCEINLINE void operator delete(void *ptr) { _mm_free(ptr); }                              \
	SKMP_FORCEINLINE void *operator new(size_t, void *ptr) { return ptr; }                                \
	SKMP_FORCEINLINE void operator delete(void *, void *) {}                                              \
	SKMP_FORCEINLINE void *operator new[](size_t sizeInBytes) { void* const ptr = _mm_malloc(sizeInBytes, x); return ptr ? ptr : throw std::bad_alloc(); } \
	SKMP_FORCEINLINE void operator delete[](void *ptr) { _mm_free(ptr); }                            \
	SKMP_FORCEINLINE void *operator new[](size_t, void *ptr) { return ptr; }                              \
	SKMP_FORCEINLINE void operator delete[](void *, void *) {}

#define SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO() SKMP_DECLARE_ALIGNED_ALLOCATOR(SIMD_ALIGNMENT)