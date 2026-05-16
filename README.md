# SAMP Mobile — iOS Port

Port do SA-MP Mobile para iOS usando GitHub Actions.  
Funciona sem jailbreak via **GBox**.

---

## Como usar

### 1. Suba o repositório no GitHub

```
git init
git add .
git commit -m "SAMP iOS port"
git remote add origin https://github.com/SEU_USUARIO/samp-ios-port.git
git push -u origin main
```

### 2. Copie os arquivos do SAMP Mobile Android

Coloque os arquivos do `GTA-SAMP-Android-master` assim:

```
samp-ios-port/
├── src/               ← arquivos .cpp do SAMP
├── sa_include/        ← headers do GTASA
├── raknet/            ← fonte do RakNet
├── ttmath/            ← biblioteca math
└── libs/
    └── android/
        └── libGTASA.so   ← stub Android (para referência)
```

### 3. Rode o GitHub Actions

1. Vá em **Actions** no seu repositório
2. Clique em **SAMP iOS Build**
3. Clique em **Run workflow**
4. Cole a **URL direta do IPA** do GTA SA iOS
5. Aguarde ~10 minutos

### 4. Baixe o IPA

Ao terminar, baixe o `gtasa-samp-ios` nos Artifacts do workflow.

### 5. Instale com GBox

Abra o GBox no iPhone → Instalar IPA → selecione o `gtasa_samp.ipa`.

---

## O que o workflow faz automaticamente

```
IPA do GTA SA iOS
       │
       ▼
[unpack_ipa.py]          ← extrai o binário GTASA
       │
       ▼
[extract_offsets.py]     ← busca os offsets das funções automaticamente
       │                    usando lief (análise do MachO ARM64)
       ▼
[Compilação ARM64]       ← clang++ com target arm64-apple-ios12.0
       │
       ▼
[inject_dylib.py]        ← injeta a libSAMP.dylib no IPA via lief
       │
       ▼
gtasa_samp.ipa           ← pronto para instalar com GBox
```

---

## Scripts

| Script | O que faz |
|--------|-----------|
| `scripts/extract_offsets.py` | Analisa o binário GTASA iOS e gera `ios_offsets.h` |
| `scripts/unpack_ipa.py` | Extrai o binário do IPA |
| `scripts/inject_dylib.py` | Injeta a dylib e reempacota o IPA |

---

## Arquivos importantes

| Arquivo | O que é |
|---------|---------|
| `src/ios_entry.cpp` | Entry point iOS (substitui `JNI_OnLoad` do Android) |
| `src/ios_hooks.h` | Macros de hook ARM64 (substitui ARMCALL/ARMJMP do Android) |
| `src/ios_offsets.h` | **Gerado automaticamente** — offsets das funções do GTASA |

---

## Possíveis problemas e soluções

### "X offsets não encontrados"
O `extract_offsets.py` tentou mas não achou algumas funções pelo nome.  
Isso acontece se o GTA SA iOS está com símbolos stripped (sem debug info).

**Solução:** O script já tem fallback por pattern scan.  
Se ainda falhar, edite os `// TODO` no `src/ios_offsets.h` manualmente  
usando o Ghidra ou IDA Pro.

### "Build falhou na compilação"
Pode ser que algum código use APIs específicas do Android (`JNI`, `android/log.h`).

**Solução:** O `#ifdef IOS_PORT` nos arquivos já isola isso.  
Verifique se tem `#include <jni.h>` em algum `.cpp` sem o ifdef.

### "IPA não instala no GBox"
Verifique se o GBox aceita IPA sem assinatura original.  
Na maioria dos casos o GBox re-assina automaticamente.

### "Game crasha ao abrir"
Provável que algum offset está errado.  
Veja o log em: `Arquivos do app → Documents → samp.log`

---

## Arquitetura técnica

```
Android (original)          iOS (port)
──────────────────          ──────────
JNI_OnLoad()         →      __attribute__((constructor))
ARM32 Thumb hooks    →      ARM64 trampolins (16 bytes)
libGTASA.so offsets  →      ios_offsets.h (gerado automaticamente)
__android_log_write  →      fprintf(stderr) + arquivo .log
.so injetado no APK  →      .dylib injetada via LC_LOAD_DYLIB no IPA
```
