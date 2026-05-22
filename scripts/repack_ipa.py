#!/usr/bin/env python3
"""
repack_ipa.py
=============
Substitui o binário gta3sa dentro do IPA pelo binário patchado.

Uso:
    python3 repack_ipa.py --ipa gtasa.ipa --binary gta3sa_patched --output gtasa_patched.ipa
"""

import argparse, os, shutil, sys, zipfile, tempfile
from pathlib import Path

def find_app_binary(zf):
    for name in zf.namelist():
        parts = Path(name).parts
        if len(parts) == 3 and parts[0] == 'Payload' and parts[1].endswith('.app'):
            if '.' not in parts[2] and not name.endswith('/'):
                return name, parts[1]
    return None, None

def repack(ipa_path, binary_path, output_path):
    print(f"[*] IPA: {ipa_path}")
    print(f"[*] Binário patchado: {binary_path}")

    with tempfile.TemporaryDirectory() as tmpdir:
        # Extrai IPA
        with zipfile.ZipFile(ipa_path, 'r') as zf:
            zf.extractall(tmpdir)
            binary_zip_path, app_name = find_app_binary(zf)

        if not binary_zip_path:
            print("[ERRO] Binário não encontrado no IPA")
            sys.exit(1)

        print(f"[✓] Binário encontrado: {binary_zip_path}")

        # Substitui o binário pelo patchado
        binary_dest = os.path.join(tmpdir, binary_zip_path)
        shutil.copy2(binary_path, binary_dest)
        print(f"[✓] Binário substituído pelo patchado")

        # Reempacota o IPA
        print(f"[*] Empacotando: {output_path}")
        with zipfile.ZipFile(output_path, 'w', zipfile.ZIP_DEFLATED, allowZip64=True) as zf_out:
            for root, dirs, files in os.walk(tmpdir):
                for file in files:
                    file_path = os.path.join(root, file)
                    arcname = os.path.relpath(file_path, tmpdir)
                    # Binários sem compressão
                    if arcname.endswith(('.dylib',)) or file in ('gta3sa', 'GTASA', 'GTA San Andreas'):
                        zf_out.write(file_path, arcname, zipfile.ZIP_STORED)
                    else:
                        zf_out.write(file_path, arcname)

        size_mb = os.path.getsize(output_path) / 1024 / 1024
        print(f"[✓] IPA gerado: {output_path} ({size_mb:.1f} MB)")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--ipa',    '-i', required=True)
    parser.add_argument('--binary', '-b', required=True)
    parser.add_argument('--output', '-o', required=True)
    args = parser.parse_args()
    repack(args.ipa, args.binary, args.output)

if __name__ == '__main__':
    main()
