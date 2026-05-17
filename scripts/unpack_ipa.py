#!/usr/bin/env python3
"""
unpack_ipa.py
=============
Extrai o binário GTASA de dentro do IPA do GTA SA iOS.
O IPA é um ZIP. Dentro dele: Payload/GTA San Andreas.app/GTASA

Uso:
    python3 unpack_ipa.py --ipa gtasa.ipa --output ./extracted/
"""

import argparse
import os
import shutil
import sys
import zipfile
from pathlib import Path


POSSIBLE_BINARY_NAMES = [
    "GTASA",
    "gta_sa",
    "GTA San Andreas",
    "Grand Theft Auto San Andreas",
]


def find_binary_in_zip(zf: zipfile.ZipFile) -> str | None:
    """Encontra o binário principal dentro do IPA."""
    candidates = []

    for name in zf.namelist():
        # Binário fica em: Payload/AppName.app/BinaryName
        parts = Path(name).parts
        if len(parts) == 3 and parts[0] == "Payload" and parts[1].endswith(".app"):
            binary_name = parts[2]
            # Sem extensão e sem / no final = é o binário
            if "." not in binary_name and not name.endswith("/"):
                candidates.append(name)
                print(f"  [?] Candidato a binário: {name}")

    # Prioriza nomes conhecidos do GTA SA
    for candidate in candidates:
        for known in POSSIBLE_BINARY_NAMES:
            if known.lower() in candidate.lower():
                return candidate

    # Retorna primeiro candidato se não achou nome conhecido
    return candidates[0] if candidates else None


def extract_ipa(ipa_path: str, output_dir: str) -> str:
    """
    Extrai o IPA e retorna o caminho do binário GTASA.
    """
    print(f"[*] Abrindo IPA: {ipa_path}")

    if not os.path.exists(ipa_path):
        print(f"[ERRO] IPA não encontrado: {ipa_path}")
        sys.exit(1)

    os.makedirs(output_dir, exist_ok=True)

    with zipfile.ZipFile(ipa_path, "r") as zf:
        all_files = zf.namelist()
        print(f"[*] Arquivos no IPA: {len(all_files)}")

        # Encontra o binário
        binary_zip_path = find_binary_in_zip(zf)

        if binary_zip_path is None:
            print("[ERRO] Não foi possível encontrar o binário dentro do IPA")
            print("[*] Arquivos disponíveis:")
            for f in all_files[:30]:
                print(f"      {f}")
            sys.exit(1)

        print(f"[✓] Binário encontrado: {binary_zip_path}")

        # Extrai o binário
        binary_filename = Path(binary_zip_path).name
        output_binary = os.path.join(output_dir, binary_filename)

        with zf.open(binary_zip_path) as src, open(output_binary, "wb") as dst:
            shutil.copyfileobj(src, dst)

        size_mb = os.path.getsize(output_binary) / 1024 / 1024
        print(f"[✓] Binário extraído: {output_binary} ({size_mb:.1f} MB)")

        # Extrai também os frameworks se existirem (podem ter símbolos úteis)
        frameworks_extracted = 0
        for name in all_files:
            if "Frameworks" in name and name.endswith(".dylib"):
                fw_output = os.path.join(output_dir, "Frameworks", Path(name).name)
                os.makedirs(os.path.dirname(fw_output), exist_ok=True)
                with zf.open(name) as src, open(fw_output, "wb") as dst:
                    shutil.copyfileobj(src, dst)
                frameworks_extracted += 1

        if frameworks_extracted > 0:
            print(f"[✓] {frameworks_extracted} frameworks extraídos")

        return output_binary


def main():
    parser = argparse.ArgumentParser(
        description="Extrai o binário GTASA de um IPA do GTA SA iOS"
    )
    parser.add_argument("--ipa", "-i", required=True, help="Caminho para o arquivo .ipa")
    parser.add_argument(
        "--output", "-o",
        default="./extracted",
        help="Diretório de saída (padrão: ./extracted)"
    )

    args = parser.parse_args()

    binary_path = extract_ipa(args.ipa, args.output)
    
    # Imprime o caminho para ser usado por outros scripts
    print(f"\nBINARY_PATH={binary_path}")


if __name__ == "__main__":
    main()
