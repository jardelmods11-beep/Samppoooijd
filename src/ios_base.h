#pragma once
#include <cstdint>
#include <cstring>

#ifdef __APPLE__
#include <mach-o/dyld.h>
static inline uintptr_t GetBase() {
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char* n = _dyld_get_image_name(i);
        if (n && (strstr(n,"GTASA")||strstr(n,"gta3sa")||strstr(n,"gta_sa")))
            return (uintptr_t)_dyld_get_image_header(i);
    }
    return (uintptr_t)_dyld_get_image_header(0);
}
#else
// Stub Linux — não executa no device, só compila
static inline uintptr_t GetBase() { return 0; }
#endif

// Chama função do GTA SA pelo offset
#define CALL(type, offset) reinterpret_cast<type>(GetBase() + (uintptr_t)(offset))
