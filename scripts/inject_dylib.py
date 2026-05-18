#!/usr/bin/env python3
"""
inject_dylib.py — injeta libSAMP.dylib no IPA do GTA SA iOS
Usa apenas zipfile (sem lief) para não corromper o binário.
A dylib é carregada via DYLD_INSERT_LIBRARIES no Info.plist.
"""

import argparse, os, shutil, sys, zipfile, tempfile
from pathlib import Path

try:
    import lief
    HAS_LIEF = True
except ImportError:
    HAS_LIEF = False

def find_app_binary(zf):
    for name in zf.namelist():
        parts = Path(name).parts
        if len(parts) == 3 and parts[0] == 'Payload' and parts[1].endswith('.app'):
            if '.' not in parts[2] and not name.endswith('/'):
                return name, parts[1]
    return None, None

def inject_load_command(binary_path, dylib_name):
    """Injeta LC_LOAD_DYLIB no binário via lief"""
    if not HAS_LIEF:
        print("[!] lief não disponível, pulando injeção de load command")
        return False
    try:
        fat = lief.MachO.parse(binary_path)
        dylib_path = f"@executable_path/Frameworks/{dylib_name}"
        for b in fat:
            # Verifica se já existe
            for lib in b.libraries:
                if dylib_name in str(lib.name):
                    print(f"[✓] Load command já existe")
                    return True
            # Adiciona
            lib = lief.MachO.DylibCommand.weak_lib(dylib_path)
            b.add(lib)
        fat.write(binary_path)
        print(f"[✓] Load command injetado: {dylib_path}")
        return True
    except Exception as e:
        print(f"[!] Erro ao injetar load command: {e}")
        return False

def repack_ipa(original_ipa, dylib_path, output_ipa):
    dylib_name = Path(dylib_path).name
    print(f"[*] Injetando {dylib_name} em {original_ipa}")

    with tempfile.TemporaryDirectory() as tmpdir:
        # Extrai IPA
        with zipfile.ZipFile(original_ipa, 'r') as zf:
            zf.extractall(tmpdir)
            binary_zip_path, app_name = find_app_binary(zf)

        if not binary_zip_path:
            print("[ERRO] Binário não encontrado no IPA")
            sys.exit(1)

        binary_local = os.path.join(tmpdir, binary_zip_path)
        app_dir = os.path.join(tmpdir, 'Payload', app_name)
        frameworks_dir = os.path.join(app_dir, 'Frameworks')
        os.makedirs(frameworks_dir, exist_ok=True)

        # Copia dylib
        dylib_dest = os.path.join(frameworks_dir, dylib_name)
        shutil.copy2(dylib_path, dylib_dest)
        print(f"[✓] Dylib copiada: Payload/{app_name}/Frameworks/{dylib_name}")

        # Injeta load command no binário
        inject_load_command(binary_local, dylib_name)

        # Reempacota
        print(f"[*] Empacotando: {output_ipa}")
        with zipfile.ZipFile(output_ipa, 'w', zipfile.ZIP_DEFLATED, allowZip64=True) as zf_out:
            for root, dirs, files in os.walk(tmpdir):
                for file in files:
                    file_path = os.path.join(root, file)
                    arcname = os.path.relpath(file_path, tmpdir)
                    # Usa compressão mínima para binários
                    if arcname.endswith(('.dylib', 'GTASA', 'gta3sa')):
                        zf_out.write(file_path, arcname, zipfile.ZIP_STORED)
                    else:
                        zf_out.write(file_path, arcname)

        size_mb = os.path.getsize(output_ipa) / 1024 / 1024
        print(f"[✓] IPA: {output_ipa} ({size_mb:.1f} MB)")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--ipa', '-i', required=True)
    parser.add_argument('--dylib', '-d', required=True)
    parser.add_argument('--output', '-o', required=True)
    args = parser.parse_args()
    repack_ipa(args.ipa, args.dylib, args.output)
    print(f"\n[✓] Instale {args.output} com GBox!")

if __name__ == '__main__':
    main()
