# SAMP iOS Port

Port do SA-MP (San Andreas Multiplayer) do Android para iOS (ARM64).

## Estrutura do Projeto

```
├── src/                    # Código-fonte principal
│   ├── CNetGame.cpp/h      # Gerenciamento de rede principal
│   ├── CPlayerPool.cpp/h   # Pool de jogadores
│   ├── CPlayer.cpp/h       # Jogador remoto
│   ├── CLocalPlayer.cpp/h  # Jogador local
│   ├── RPC.cpp/h           # Chamadas RPC do SA-MP
│   ├── hooks.cpp/h         # Hooks de função no GTA SA
│   ├── ios_entry.cpp       # Ponto de entrada da dylib (iOS)
│   ├── ios_hooks.h         # Macros de hook ARM64 para iOS
│   ├── ios_offsets.h       # Offsets do binário GTA SA iOS (gerado automaticamente)
│   ├── AuthTable.h         # Tabela de autenticação SA-MP
│   ├── types.h             # Tipos básicos
│   └── utils.cpp/h         # Utilitários e logging
│
├── raknet/                 # Biblioteca RakNet (versão SA-MP)
│   └── SAMP/               # Extensões específicas do SA-MP
│
├── sa_include/             # Headers das classes do GTA SA
│   ├── GTASA.h             # Inclui tudo + macro GTASA_FUNC
│   ├── CPlayerPed.h        # Ped do jogador
│   ├── CVector.h           # Vetor 3D
│   └── ...                 # Outras classes do GTA
│
├── ttmath/                 # Biblioteca de aritmética de precisão arbitrária
├── scripts/                # Scripts Python do pipeline de build
│   ├── extract_offsets.py  # Extrai offsets do binário iOS automaticamente
│   ├── unpack_ipa.py       # Desempacota o IPA do GTA SA
│   └── inject_dylib.py     # Injeta a dylib no IPA
│
└── libs/android/
    └── libGTASA.so         # Binário Android (referência para extração de offsets)
```

## Como Fazer o Build

### Via GitHub Actions (recomendado)

1. Faça fork/push do repositório
2. Vá em **Actions → SAMP iOS Build → Run workflow**
3. Informe a URL direta do IPA do GTA SA iOS
4. Aguarde o build — o IPA modificado estará nos Artifacts

### Como funciona o pipeline

```
IPA do GTA SA iOS
       ↓
scripts/unpack_ipa.py     → extrai o binário gta3sa
       ↓
scripts/extract_offsets.py → compara com libGTASA.so Android
                             → gera src/ios_offsets.h com os offsets reais
       ↓
clang++ ARM64              → compila src/*.cpp + raknet/*.cpp
                           → linka libSAMP.dylib
       ↓
scripts/inject_dylib.py   → injeta dylib no IPA original
       ↓
gtasa_samp.ipa (pronto)
```

## Diferença Android × iOS

| | Android | iOS |
|---|---|---|
| Formato | `.so` (ELF) | `.dylib` (Mach-O) |
| Entrada | `JNI_OnLoad` | `__attribute__((constructor))` |
| Hooks | substrates/inline | `ios_hooks.h` (branch ARM64) |
| Offsets | hardcoded | gerados por `extract_offsets.py` |

## Offsets (ios_offsets.h)

O arquivo `src/ios_offsets.h` é **gerado automaticamente** pelo script `extract_offsets.py` a partir do binário do GTA SA iOS. Ele usa o `libGTASA.so` Android como referência para mapear os símbolos.

Se precisar adicionar um offset manualmente, edite `ios_offsets.h` e adicione:
```c
#define OFFSET_MINHA_FUNCAO  0x00ABCDEF  // substitua pelo offset real
```

## Créditos

- SA-MP Android: base do código de rede e RakNet
- ttmath: biblioteca de criptografia serial
- RakNet: biblioteca de rede (versão SA-MP fork)
