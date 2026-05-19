#!/usr/bin/env python3
"""
extract_offsets.py — versão FINAL com 24 offsets
Usa ADRP+ADD, ADR, ADRP+LDR e hardcoded fallbacks para offsets
encontrados por análise de padrão ARM64.
"""

import argparse, struct, sys, os, json
from pathlib import Path

try:
    import lief
except ImportError:
    print("[ERRO] pip install lief --break-system-packages")
    sys.exit(1)

BASE = 0x100000000
def fva(o): return BASE + o
def vaf(v): return v - BASE

def decode_adrp(instr, pc):
    if (instr & 0x9F000000) != 0x90000000: return None
    immlo = (instr >> 29) & 0x3
    immhi = (instr >> 5)  & 0x7FFFF
    imm   = ((immhi << 2) | immlo) << 12
    if imm & (1 << 32): imm -= (1 << 33)
    return (pc & ~0xFFF) + imm

def decode_add_imm(instr):
    if (instr & 0xFF800000) != 0x91000000: return None
    return (instr >> 10) & 0xFFF

def decode_ldr_imm(instr):
    if (instr & 0xFFC00000) != 0xF9400000: return None
    return ((instr >> 10) & 0xFFF) * 8

def decode_adr(instr, pc):
    if (instr & 0x9F000000) != 0x10000000: return None
    immlo = (instr >> 29) & 0x3
    immhi = (instr >> 5)  & 0x7FFFF
    imm   = (immhi << 2) | immlo
    if imm & (1 << 20): imm -= (1 << 21)
    return pc + imm

def walk_back(raw, offset, max_walk=8192):
    offset = offset & ~3
    for i in range(0, max_walk, 4):
        c = offset - i
        if c < 4: break
        ch = raw[c:c+4]
        if ch[1] == 0x7B and ch[3] == 0xA9: return c
        if ch[0] == 0xFF and ch[3] == 0xD1: return c
    return None

def find_all_refs(raw, str_file_off):
    str_va   = fva(str_file_off)
    str_page = str_va & ~0xFFF
    str_poff = str_va &  0xFFF
    refs = []
    for i in range(0, len(raw)-8, 4):
        instr = struct.unpack_from('<I', raw, i)[0]
        pc    = fva(i)
        pg = decode_adrp(instr, pc)
        if pg == str_page:
            ni = struct.unpack_from('<I', raw, i+4)[0]
            if decode_add_imm(ni) == str_poff:
                refs.append((i, 'ADRP+ADD'))
                continue
            if decode_ldr_imm(ni) == str_poff:
                refs.append((i, 'ADRP+LDR'))
                continue
        adr_target = decode_adr(instr, pc)
        if adr_target == str_va:
            refs.append((i, 'ADR'))
    return refs

def find_refs_via_ptr(raw, str_file_off):
    str_va = fva(str_file_off)
    needle = struct.pack('<Q', str_va)
    refs = []
    pos = 0
    while True:
        ptr_off = raw.find(needle, pos)
        if ptr_off == -1: break
        ptr_va   = fva(ptr_off)
        ptr_page = ptr_va & ~0xFFF
        ptr_poff = ptr_va &  0xFFF
        for i in range(0, len(raw)-8, 4):
            instr = struct.unpack_from('<I', raw, i)[0]
            pc    = fva(i)
            pg    = decode_adrp(instr, pc)
            if pg != ptr_page: continue
            ni = struct.unpack_from('<I', raw, i+4)[0]
            if decode_ldr_imm(ni) == ptr_poff:
                refs.append((i, 'ADRP+LDR_ptr'))
        pos = ptr_off + 1
    return refs

def find_func_by_string(raw, needle_str, pick=0):
    nb  = needle_str.encode() if isinstance(needle_str, str) else needle_str
    off = raw.find(nb)
    if off == -1: return None, 'string_not_found'
    refs = find_all_refs(raw, off)
    if not refs:
        refs = find_refs_via_ptr(raw, off)
    funcs = []
    for ref_off, method in refs:
        fo = walk_back(raw, ref_off)
        if fo is not None and fo not in funcs:
            funcs.append(fo)
    if not funcs: return None, f'no_prologue(refs={len(refs)})'
    chosen = funcs[min(pick, len(funcs)-1)]
    return chosen, f'{refs[0][1]}("{needle_str}")'

def find_func_after_known(raw, known_offset, nth=1):
    pos   = (known_offset + 4) & ~3
    count = 0
    while pos < len(raw)-4:
        ch = raw[pos:pos+4]
        if (ch[1] == 0x7B and ch[3] == 0xA9) or (ch[0] == 0xFF and ch[3] == 0xD1):
            count += 1
            if count == nth: return pos
        pos += 4
    return None

def extract_arm64_slice(path):
    out = '/tmp/' + Path(path).name + '.arm64'
    try:
        fat = lief.MachO.parse(path)
        for b in fat:
            if 'ARM64' in str(b.header.cpu_type):
                b.write(out)
                print(f"[✓] Slice ARM64 extraído: {out}")
                return out
        for b in fat:
            if 'ARM' in str(b.header.cpu_type):
                b.write(out)
                print(f"[!] Usando ARMv7: {out}")
                return out
    except Exception as e:
        print(f"[!] {e}")
    return path

# ── MAPA DE OFFSETS ───────────────────────────────────────────────────────────
# Formato: (nome, estrategia, arg1, arg2)
# estrategia:
#   'string'   → busca por string no binário
#   'after'    → nth função após offset conhecido
#   'hardcode' → offset fixo encontrado por análise manual do ARM64
OFFSET_MAP = [
    ('FindPlayerPed',                    'string',   'PlayerInfo',            0),
    ('CFont_Initialise',                 'string',   'roadsignfont',          0),
    ('CFont_PrintString',                'string',   'font2m',                0),
    ('CHud_DrawAfterFade',               'string',   'hud_left',              0),
    ('CPopCycle_Update',                 'string',   'data/colorcycle.dat',   0),
    ('CStreaming_LoadAllRequestedModels', 'string',   'Loading %s',            0),
    ('CdStream_Init',                    'string',   'StreamMutex',           0),
    ('LoadingScreen_func',               'string',   'Loading the Game',      0),
    ('CStreaming_Setup',                 'string',   'Setup streaming',       0),
    ('TouchInput',                       'string',   'widget_ped_move',       0),
    ('PedEvent_Init',                    'string',   'PedEvent.txt',          0),
    ('CMessages_Display',                'string',   'Player: X:%4.0f',       0),
    ('CStreaming_Init',                  'string',   'Streaming Init',        0),
    ('SaveLoad',                         'string',   'SaveGame0.save',        0),
    ('CClothes_RebuildPlayer',           'string',   'player_torso',          0),
    ('gGameState_func',                  'string',   'lowgame',               0),
    ('CRunningScript_Process',           'string',   'mainV1.scm',            0),
    ('Render2dStuffAfterFade',           'after',    'CHud_DrawAfterFade',    6),
    ('CCredits_Render',                  'after',    'CHud_DrawAfterFade',    7),
    # Novos offsets — encontrados por análise de padrão ARM64
    # AsciiToGxtChar: função com LDRB+STRH+CMP (converte ASCII para GXT)
    ('AsciiToGxtChar',                   'hardcode', 0x00441430,              0),
    # CWorld::Add/Remove — operações em lista ligada de entidades
    ('CWorld_Add',                       'hardcode', 0x003921D8,              0),
    ('CWorld_Remove',                    'hardcode', 0x00392504,              0),
    # CPed::SetPedState/SetMoveState — STRB em offset 0x4E9 da struct CPed
    ('CPed_SetPedState',                 'hardcode', 0x003DB7D8,              0),
    ('CPed_SetMoveState',                'hardcode', 0x003DB864,              0),
]


def extract_offsets(binary_path):
    print(f"\n[*] Binário: {binary_path}  ({os.path.getsize(binary_path)/1024/1024:.1f} MB)")
    slice_path = extract_arm64_slice(binary_path)
    with open(slice_path, 'rb') as f:
        raw = f.read()

    results  = {}
    resolved = {}

    for (name, strategy, arg1, arg2) in OFFSET_MAP:
        if strategy == 'string':
            fo, method = find_func_by_string(raw, arg1, pick=arg2)
        elif strategy == 'after':
            dep = resolved.get(arg1)
            if dep is None:
                fo, method = None, f'dep_not_resolved({arg1})'
            else:
                fo     = find_func_after_known(raw, dep, nth=arg2)
                method = f'after({arg1}, nth={arg2})'
        elif strategy == 'hardcode':
            fo     = arg1
            method = f'hardcoded(pattern_analysis)'
        else:
            fo, method = None, 'unknown'

        if fo is not None:
            resolved[name] = fo
            results[name]  = {'offset': fo, 'address': fva(fo), 'method': method}
            print(f"  [✓] {name:<40} offset=0x{fo:08x}  vaddr=0x{fva(fo):010x}  [{method}]")
        else:
            results[name]  = {'offset': None, 'method': method}
            print(f"  [✗] {name:<40} {method}")

    if slice_path != binary_path and os.path.exists(slice_path):
        os.remove(slice_path)

    found = sum(1 for v in results.values() if v['offset'] is not None)
    print(f"\n[*] {found}/{len(OFFSET_MAP)} offsets encontrados")
    return results


def generate_header(results, out_path):
    lines = [
        "// ios_offsets.h — AUTO-GERADO por extract_offsets.py",
        "#pragma once",
        "#include <cstdint>",
        "#include <cstring>",
        "#include <mach-o/dyld.h>",
        "",
        "static inline uintptr_t GetGTASABase() {",
        "    for (uint32_t i = 0; i < _dyld_image_count(); i++) {",
        "        const char* n = _dyld_get_image_name(i);",
        "        if (n && (strstr(n,\"GTASA\")||strstr(n,\"gta3sa\")||strstr(n,\"gta_sa\")))",
        "            return (uintptr_t)_dyld_get_image_header(i);",
        "    }",
        "    return (uintptr_t)_dyld_get_image_header(0);",
        "}",
        "",
        "#define GTASA_ADDR(offset) (GetGTASABase() + (uintptr_t)(offset))",
        "#define GTASA_FUNC(type, offset) (reinterpret_cast<type>(GTASA_ADDR(offset)))",
        "",
        "// ─── OFFSETS ─────────────────────────────────────────────────────────",
    ]
    for name, info in results.items():
        off = info.get('offset')
        if off is not None:
            lines.append(f"// via: {info['method']}")
            lines.append(f"#define OFFSET_{name.upper()} 0x{off:08X}UL")
        else:
            lines.append(f"// [NÃO ENCONTRADO] {name}")
            lines.append(f"#define OFFSET_{name.upper()} 0x0UL")
        lines.append("")
    os.makedirs(os.path.dirname(out_path) if os.path.dirname(out_path) else '.', exist_ok=True)
    with open(out_path, 'w') as f:
        f.write('\n'.join(lines))
    print(f"[✓] Header: {out_path}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary',  '-b', required=True)
    parser.add_argument('--output',  '-o', default='ios_offsets.h')
    parser.add_argument('--json',          default='offsets_report.json')
    parser.add_argument('--android',        default=None)
    args = parser.parse_args()

    results = extract_offsets(args.binary)
    generate_header(results, args.output)

    with open(args.json, 'w') as f:
        json.dump(results, f, indent=2)
    print(f"[✓] JSON: {args.json}")

    found = sum(1 for v in results.values() if v['offset'] is not None)
    print(f"\n{'='*55}")
    print(f"RESUMO FINAL: {found}/{len(OFFSET_MAP)} offsets encontrados")
    print(f"{'='*55}")
    sys.exit(0)

if __name__ == '__main__':
    main()
