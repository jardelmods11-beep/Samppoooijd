#!/usr/bin/env python3
"""
inject_dylib.py
===============
Injeta a dylib do SAMP dentro do IPA do GTA SA iOS.

O que faz:
  1. Descompacta o IPA (é um ZIP)
  2. Adiciona a libSAMP.dylib dentro do app
  3. Usa insert_dylib ou modifica o Load Command do binário via lief
     para o app carregar a dylib automaticamente
  4. Reempacota tudo num novo IPA

Uso:
    python3 inject_dylib.py --ipa gtasa.ipa --dylib libSAMP.dylib --output gtasa_samp.ipa
"""

import argparse
import os
import shutil
import sys
import zipfile
import tempfile
from pathlib import Path

try:
    import lief
except ImportError:
    print("[ERRO] lief não instalado. Rode: pip install lief")
    sys.exit(1)


def find_app_binary(zf: zipfile.ZipFile) -> tuple[str, str]:
    """
    Retorna (caminho_no_zip, nome_do_app) do binário principal.
    """
    for name in zf.namelist():
        parts = Path(name).parts
        if len(parts) == 3 and parts[0] == "Payload" and parts[1].endswith(".app"):
            binary_name = parts[2]
            if "." not in binary_name and not name.endswith("/"):
                app_name = parts[1]  # ex: "GTA San Andreas.app"
                return name, app_name
    return None, None


def inject_load_command(binary_path: str, dylib_name: str) -> bool:
    """
    Adiciona um LC_LOAD_DYLIB ao binário MachO para carregar a dylib.
    Usa lief para modificar o binário sem precisar de insert_dylib externo.
    """
    print(f"[*] Injetando load command: @executable_path/Frameworks/{dylib_name}")

    try:
        # Para fat binary (universal), pega o slice ARM64
        fat = lief.MachO.parse(binary_path)
        
        # Pega o binário ARM64
        target_binary = None
        for b in fat:
            cpu = str(b.header.cpu_type)
            print(f"  [*] Slice encontrado: {cpu}")
            if "ARM64" in cpu or "arm64" in cpu.lower():
                target_binary = b
                break
        
        if target_binary is None:
            # Se não achou ARM64, pega o primeiro
            target_binary = fat[0]
            print(f"  [!] ARM64 não encontrado, usando primeiro slice")

        # Verifica se o load command já existe
        dylib_path = f"@executable_path/Frameworks/{dylib_name}"
        for cmd in target_binary.libraries:
            if dylib_name in str(cmd.name):
                print(f"  [!] Load command já existe: {cmd.name}")
                return True

        # Adiciona o load command
        lib = lief.MachO.DylibCommand.weak_lib(dylib_path)
        target_binary.add(lib)

        # Salva o binário modificado
        fat.write(binary_path)
        print(f"  [✓] Load command injetado com sucesso")
        return True

    except Exception as e:
        print(f"  [ERRO] Falha ao injetar load command: {e}")
        return False


def repack_ipa(
    original_ipa: str,
    dylib_path: str,
    output_ipa: str,
    extra_files: list[tuple[str, str]] = None
):
    """
    Cria um novo IPA com a dylib injetada.
    
    extra_files: lista de (caminho_local, caminho_no_ipa)
    """
    print(f"\n[*] Empacotando novo IPA...")

    if not os.path.exists(dylib_path):
        print(f"[ERRO] dylib não encontrada: {dylib_path}")
        sys.exit(1)

    dylib_name = Path(dylib_path).name

    with tempfile.TemporaryDirectory() as tmpdir:
        print(f"[*] Extraindo IPA original para {tmpdir}")

        # Extrai IPA original
        with zipfile.ZipFile(original_ipa, "r") as zf:
            zf.extractall(tmpdir)
            binary_zip_path, app_name = find_app_binary(zf)

        if binary_zip_path is None:
            print("[ERRO] Binário não encontrado no IPA")
            sys.exit(1)

        # Caminhos locais
        binary_local = os.path.join(tmpdir, binary_zip_path)
        app_dir = os.path.join(tmpdir, "Payload", app_name)
        frameworks_dir = os.path.join(app_dir, "Frameworks")
        os.makedirs(frameworks_dir, exist_ok=True)

        # Copia a dylib para Frameworks/
        dylib_dest = os.path.join(frameworks_dir, dylib_name)
        shutil.copy2(dylib_path, dylib_dest)
        print(f"[✓] Dylib copiada para: Payload/{app_name}/Frameworks/{dylib_name}")

        # Injeta o load command no binário
        success = inject_load_command(binary_local, dylib_name)
        if not success:
            print("[AVISO] Load command não injetado — pode precisar de insert_dylib manual")

        # Copia arquivos extras (ex: config do servidor SAMP)
        if extra_files:
            for local_path, zip_dest in extra_files:
                dest = os.path.join(tmpdir, zip_dest)
                os.makedirs(os.path.dirname(dest), exist_ok=True)
                shutil.copy2(local_path, dest)
                print(f"[✓] Arquivo extra: {zip_dest}")

        # Reempacota em novo IPA
        print(f"[*] Empacotando: {output_ipa}")
        with zipfile.ZipFile(output_ipa, "w", zipfile.ZIP_DEFLATED) as zf_out:
            for root, dirs, files in os.walk(tmpdir):
                for file in files:
                    file_path = os.path.join(root, file)
                    arcname = os.path.relpath(file_path, tmpdir)
                    zf_out.write(file_path, arcname)

        size_mb = os.path.getsize(output_ipa) / 1024 / 1024
        print(f"[✓] IPA gerado: {output_ipa} ({size_mb:.1f} MB)")


def main():
    parser = argparse.ArgumentParser(
        description="Injeta a dylib do SAMP no IPA do GTA SA iOS"
    )
    parser.add_argument("--ipa", "-i", required=True, help="IPA original do GTA SA")
    parser.add_argument("--dylib", "-d", required=True, help="libSAMP.dylib compilada")
    parser.add_argument("--output", "-o", required=True, help="IPA de saída com SAMP")
    parser.add_argument(
        "--extra", "-e",
        nargs=2, action="append", metavar=("LOCAL", "DEST"),
        help="Arquivo extra para incluir no IPA (ex: --extra config.json Payload/App.app/config.json)"
    )

    args = parser.parse_args()

    extra_files = args.extra or []

    repack_ipa(
        original_ipa=args.ipa,
        dylib_path=args.dylib,
        output_ipa=args.output,
        extra_files=extra_files,
    )

    print(f"\n[✓] Pronto! Instale {args.output} com o GBox.")


if __name__ == "__main__":
    main()
