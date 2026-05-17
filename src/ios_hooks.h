#pragma once
// ios_hooks.h
// Macros de hook ARM64 para iOS usando escrita direta de instrução B (branch)
// Para uso em produção, considere usar Dobby ou Substrate.

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include "ios_offsets.h"

// Macro para chamar função pelo offset no binário do GTA SA
// Uso: GTASA_FUNC(tipo_funcao, OFFSET_NOME)(args...)
#define GTASA_FUNC(type, offset) \
    ((type)(GTASA_ADDR(offset)))

// ------------------------------------------------------------
// Hook simples: escreve um BL/B incondicional ARM64 (4 bytes)
// substituindo os primeiros 4 bytes da função-alvo por um
// branch para a nossa função hook.
// Limitação: funciona apenas se o hook estiver dentro de ±128 MB.
// Para hooks mais robustos use Dobby (https://github.com/jmpews/Dobby).
// ------------------------------------------------------------
static inline bool _hook_write_branch(uintptr_t target, uintptr_t destination)
{
    // Calcular offset relativo (em palavras de 4 bytes)
    int64_t diff = (int64_t)(destination - target) / 4;

    // Verificar alcance do B imm26 (±128 MB)
    if (diff < -(1 << 25) || diff > ((1 << 25) - 1)) {
        return false; // fora do alcance — use Dobby para trampoline
    }

    uint32_t instr = 0x14000000 | (uint32_t)(diff & 0x3FFFFFF); // B imm26

    // Tornar página gravável
    uintptr_t page  = target & ~(uintptr_t)(getpagesize() - 1);
    mprotect((void*)page, getpagesize() * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    memcpy((void*)target, &instr, sizeof(instr));

    // Flush de cache de instrução (necessário em ARM)
    __builtin___clear_cache((char*)target, (char*)(target + 4));

    return true;
}

// Hook de uma função pelo offset no binário
// Uso: HOOK_JMP(OFFSET_MINHA_FUNCAO, minha_funcao_hook);
#define HOOK_JMP(offset, hook_fn) \
    _hook_write_branch(GTASA_ADDR(offset), (uintptr_t)(hook_fn))
