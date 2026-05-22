#!/usr/bin/env python3
"""
patch_binary.py
===============
Aplica patch estático no gta3sa para hookar funções sem precisar
de mprotect/vm_write em runtime (que o iOS bloqueia).

Estratégia:
1. Encontra espaço livre (cave) no segmento __TEXT
2. Escreve trampolins ARM64 na cave
3. Substitui o início das funções alvo por B (branch) para os trampolins
4. Os trampolins executam as instruções originais + voltam para a função

Uso:
    python3 patch_binary.py --binary gta3sa --output gta3sa_patched
"""

import argparse, struct, sys, os, lief

BASE = 0x100000000

def fva(o): return BASE + o
def vaf(v): return v - BASE

def pack_b(from_va, to_va):
    diff = (to_va - from_va) // 4
    if not (-(1<<25) <= diff <= (1<<25)-1):
        raise ValueError(f"Branch fora do range: from=0x{from_va:x} to=0x{to_va:x} diff={diff}")
    return struct.pack('<I', 0x14000000 | (diff & 0x3FFFFFF))

NOP = struct.pack('<I', 0xD503201F)
RET = struct.pack('<I', 0xD65F03C0)

# Offsets confirmados do gta3sa ARM64
HOOKS = {
    'CRunningScript_Process': 0x001CFF1C,
    'CPopCycle_Update':       0x002F7894,
    'LoadingScreen_func':     0x0024017C,
    'Render2dStuffAfterFade': 0x002A9294,
}

# Localização da cave de código (espaço livre no binário)
CAVE_OFF  = 0x00668004
CAVE_VA   = fva(CAVE_OFF)
CAVE_SIZE = 16380

def find_arm64_slice_offset(fat_raw):
    """Encontra o offset do slice ARM64 dentro do fat binary"""
    magic = struct.unpack_from('>I', fat_raw, 0)[0]
    if magic != 0xCAFEBABE:
        return 0, len(fat_raw)  # Não é fat, é arquivo único
    narch = struct.unpack_from('>I', fat_raw, 4)[0]
    for i in range(narch):
        off = 8 + i * 20
        cpu_type = struct.unpack_from('>I', fat_raw, off)[0]
        file_off = struct.unpack_from('>I', fat_raw, off+8)[0]
        size     = struct.unpack_from('>I', fat_raw, off+12)[0]
        if cpu_type == 0x0100000C:  # ARM64
            return file_off, size
    raise ValueError("Slice ARM64 não encontrado no fat binary")

def apply_patch(fat_raw, slice_offset):
    cave_off = CAVE_OFF
    cave_va  = CAVE_VA
    """Aplica todos os patches no binário"""
    raw = bytearray(fat_raw)
    base = slice_offset  # offset absoluto do slice ARM64 no fat

    def abs_off(rel_off):
        return base + rel_off

    # Verifica cave
    cave_sample = bytes(raw[abs_off(cave_off):abs_off(cave_off)+8])
    if cave_sample != b'\x00' * 8:
        print(f"[AVISO] Cave não está vazia: {cave_sample.hex()}")
        print(f"[AVISO] Procurando outra cave...")
        # Procura outro bloco livre
        for test_off in range(0x00660000, 0x006A0000, 16):
            sample = bytes(raw[abs_off(test_off):abs_off(test_off)+64])
            if all(b == 0 for b in sample):
                cave_off = test_off
                cave_va  = fva(test_off)
                cave_off = test_off
                cave_va  = fva(test_off)
                print(f"[OK] Nova cave: 0x{CAVE_OFF:08x}")
                break

    print(f"[*] Cave: 0x{CAVE_OFF:08x} (vaddr=0x{CAVE_VA:010x})")

    # Monta os trampolins na cave
    code = bytearray(CAVE_SIZE)
    pos  = 0

    def w32(val):
        nonlocal pos
        code[pos:pos+4] = struct.pack('<I', val) if isinstance(val, int) else val
        pos += 4

    def w_bytes(data):
        nonlocal pos
        code[pos:pos+len(data)] = data
        pos += len(data)

    def align(n=16):
        nonlocal pos
        while pos % n != 0:
            pos += 1

    tramp_offsets = {}

    for name, hook_rel in HOOKS.items():
        if name == 'CPopCycle_Update':
            continue  # Esse vai direto como RET

        tramp_rel  = cave_off + pos
        tramp_va   = cave_va  + pos
        hook_va    = fva(hook_rel)
        cont_va    = fva(hook_rel + 8)  # continua após as 2 instruções substituídas

        tramp_offsets[name] = (tramp_rel, tramp_va)

        # Salva as 2 instruções originais
        orig1 = struct.unpack_from('<I', raw, abs_off(hook_rel))[0]
        orig2 = struct.unpack_from('<I', raw, abs_off(hook_rel+4))[0]

        w32(orig1)  # instrução original 1
        w32(orig2)  # instrução original 2

        # B de volta para hook+8
        b_back = pack_b(tramp_va + 8, cont_va)
        w_bytes(b_back)
        w32(0xD503201F)  # NOP
        align(16)

        print(f"[✓] Trampolim {name}: cave=0x{tramp_rel:08x}")

    # Escreve cave no binário
    raw[abs_off(cave_off):abs_off(cave_off)+CAVE_SIZE] = code

    # ── Aplica os branches nas funções originais ──────────────────────────────

    # CPopCycle → RET direto
    raw[abs_off(HOOKS['CPopCycle_Update']):abs_off(HOOKS['CPopCycle_Update'])+4] = RET
    print(f"[✓] CPopCycle_Update → RET")

    # CRunningScript → B para trampolim
    for name in ['CRunningScript_Process', 'LoadingScreen_func', 'Render2dStuffAfterFade']:
        hook_rel = HOOKS[name]
        hook_va  = fva(hook_rel)
        _, tramp_va = tramp_offsets[name]
        b_instr = pack_b(hook_va, tramp_va)
        raw[abs_off(hook_rel):abs_off(hook_rel)+4]   = b_instr
        raw[abs_off(hook_rel)+4:abs_off(hook_rel)+8] = NOP
        print(f"[✓] {name} @ 0x{hook_rel:08x} → B 0x{tramp_va:x}")

    return bytes(raw)

def main():
    parser = argparse.ArgumentParser(description='Patch estático no gta3sa iOS')
    parser.add_argument('--binary', '-b', required=True, help='Binário gta3sa original')
    parser.add_argument('--output', '-o', required=True, help='Binário patchado de saída')
    args = parser.parse_args()

    print(f"[*] Lendo: {args.binary}")
    with open(args.binary, 'rb') as f:
        fat_raw = f.read()
    print(f"[*] Tamanho: {len(fat_raw)/1024/1024:.1f} MB")

    slice_offset, slice_size = find_arm64_slice_offset(fat_raw)
    print(f"[*] Slice ARM64: offset=0x{slice_offset:08x} size=0x{slice_size:08x}")

    patched = apply_patch(fat_raw, slice_offset)

    with open(args.output, 'wb') as f:
        f.write(patched)

    print(f"\n[✓] Binário patchado: {args.output} ({len(patched)/1024/1024:.1f} MB)")

if __name__ == '__main__':
    main()
