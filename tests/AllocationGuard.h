#pragma once
#include <cstdlib>
#include <new>
#include <malloc.h>
thread_local bool watchAllocations = false;
thread_local unsigned watchedAllocations = 0, watchedDeallocations = 0;
void* operator new(size_t count)
{
    if (watchAllocations) ++watchedAllocations;
    if (auto* p = std::malloc(count == 0 ? 1 : count)) return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { if (p && watchAllocations) ++watchedDeallocations; std::free(p); }
void operator delete(void* p, size_t) noexcept { ::operator delete(p); }
void* operator new[](size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete[](void* p, size_t) noexcept { ::operator delete(p); }
void* operator new(size_t n, std::align_val_t alignment)
{
    if (watchAllocations) ++watchedAllocations;
    if (auto* p = _aligned_malloc(n == 0 ? 1 : n, static_cast<size_t>(alignment))) return p;
    throw std::bad_alloc();
}
void operator delete(void* p, std::align_val_t) noexcept
{ if (p && watchAllocations) ++watchedDeallocations; _aligned_free(p); }
void operator delete(void* p, size_t, std::align_val_t a) noexcept { ::operator delete(p, a); }
struct AudioAllocationGuard
{
    AudioAllocationGuard() { watchedAllocations = watchedDeallocations = 0; watchAllocations = true; }
    ~AudioAllocationGuard() { watchAllocations = false; }
};
