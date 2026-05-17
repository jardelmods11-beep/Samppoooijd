#pragma once
#include <cstdint>
#include <cstring>
#include "ios_offsets.h"

#ifdef __APPLE__
#include <sys/mman.h>

static inline bool _hook_write_branch(uintptr_t target, uintptr_t destination) {
    if (!target) return false;
    int64_t diff = (int64_t)(destination - target) / 4;
    if (diff < -(1 << 25) || diff > ((1 << 25) - 1)) return false;
    uint32_t instr = 0x14000000 | (uint32_t)(diff & 0x3FFFFFF);
    uintptr_t page = target & ~(uintptr_t)(getpagesize() - 1);
    mprotect((void*)page, getpagesize() * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy((void*)target, &instr, sizeof(instr));
    __builtin___clear_cache((char*)target, (char*)(target + 4));
    return true;
}
#else
static inline bool _hook_write_branch(uintptr_t, uintptr_t) { return false; }
#endif

#define HOOK_JMP(offset, hook_fn) \
    _hook_write_branch(GTASA_ADDR(offset), (uintptr_t)(hook_fn))
#define HOOK_CALL(offset, hook_fn) \
    _hook_write_branch(GTASA_ADDR(offset), (uintptr_t)(hook_fn))
