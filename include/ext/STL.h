#pragma once

#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <list>

#include <string>

#include "Mem.h"
#include "Hash.h"

namespace stl
{
    // aligned on 16/32 byte boundary

    template <class K, class V, class H = std::hash<K>, class P = std::equal_to<K>>
    using unordered_map_simd = std::unordered_map<K, V, H, P, mem::aligned_allocator<std::pair<const K, V>, SKMP_ALLOC_ALIGN>>;

    template <class K, class V, class C = std::less<K>>
    using map_simd = std::map<K, V, C, mem::aligned_allocator<std::pair<const K, V>, SKMP_ALLOC_ALIGN>>;

    template <class K, class H = std::hash<K>, class E = std::equal_to<K>>
    using unordered_set_simd = std::unordered_set<K, H, E, mem::aligned_allocator<K, SKMP_ALLOC_ALIGN>>;

    template <class K, class C = std::less<K>>
    using set_simd = std::set<K, C, mem::aligned_allocator<K, SKMP_ALLOC_ALIGN>>;

    template <class V>
    using vector_simd = std::vector<V, mem::aligned_allocator<V, SKMP_ALLOC_ALIGN>>;

    template <class V>
    using list_simd = std::list<V, mem::aligned_allocator<V, SKMP_ALLOC_ALIGN>>;

    template <class V>
    using queue_simd = std::queue<V, std::deque<V, mem::aligned_allocator<V, SKMP_ALLOC_ALIGN>>>;

    /*template <class K, class V, class H = std::hash<K>, class P = std::equal_to<K>>
    using sparse_map_simd = tsl::sparse_map<K, V, H, P, mem::aligned_allocator<std::pair<K, V>, SKMP_ALLOC_ALIGN>>;

    template <class K, class V, class H = std::hash<K>, class P = std::equal_to<K>>
    using sparse_set_simd = tsl::sparse_set<K, V, H, P, mem::aligned_allocator<std::pair<K, V>, SKMP_ALLOC_ALIGN>>;*/

    // aligned on 16/32 byte boundary (case-insensitive)

    template <class K, class V>
    using iunordered_map_simd = std::unordered_map<K, V, hash::i_fnv_1a, hash::iequal_to, mem::aligned_allocator<std::pair<const K, V>, SKMP_ALLOC_ALIGN>>;

    template <class K, class V>
    using imap_simd = std::map<K, V, hash::icase_comp, mem::aligned_allocator<std::pair<const K, V>, SKMP_ALLOC_ALIGN>>;

    template <class K>
    using iunordered_set_simd = std::unordered_set<K, hash::i_fnv_1a, hash::iequal_to, mem::aligned_allocator<K, SKMP_ALLOC_ALIGN>>;

    template <class K>
    using iset_simd = std::set<K, hash::icase_comp, mem::aligned_allocator<K, SKMP_ALLOC_ALIGN>>;

    // case-insensitive

    template <class K, class V, class A = std::allocator<std::pair<const K, V>>>
    using iunordered_map = std::unordered_map<K, V, hash::i_fnv_1a, hash::iequal_to, A>;

    template <class K, class V, class A = std::allocator<std::pair<const K, V>>>
    using imap = std::map<K, V, hash::icase_comp, A>;

    template <class K, class A = std::allocator<K>>
    using iunordered_set = std::unordered_set<K, hash::i_fnv_1a, hash::iequal_to, A>;

    template <class K, class A = std::allocator<K>>
    using iset = std::set<K, hash::icase_comp, A>;
}
