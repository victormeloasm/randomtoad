# RandomToad CTR_DRBG (AES‑256) — NIST/FIPS‑like

**CTR_DRBG (AES‑256) no_df · SP 800‑90A Rev.1**, com coletor 90B‑like (health tests RCT/APT) e composição 90C‑like (PR opcional + reseed periódico).  
Modo **FIPS‑like** com self‑tests (DRBG KAT) e *error latch*. AES‑NI obrigatório.

> ⚠️ **Aviso importante:** Este projeto é **NIST/FIPS‑like** (compatível conceitualmente), **não** é FIPS 140‑3 **validado**. Para validação formal, é necessário CAVP/CMVP.

## Build rápido
```bash
sudo apt-get update && sudo apt-get install -y clang lld fasm
cd src
make
./build/selftest   # espera: SELFTEST: OK
```

### FIPS‑mode (like)
```bash
make fips           # compile-time flag
RT_FIPS_MODE=1 ./build/selftest
```

### Dieharder (opcional)
```bash
sudo apt-get install -y dieharder
./build/stream_randomtoad | dieharder -a -g 200
```

## Instalação (headers + lib)
```bash
cd src
sudo make install
# headers: /usr/local/include/randomtoad
# lib:     /usr/local/lib/librandomtoad.a
```

## API (resumo) — C
```c
#include "randomtoad/ctr_drbg.h"
#include "randomtoad/module_fips.h"

rt_ctr_drbg d;
rt_module_init();
rt_ctr_drbg_instantiate_system(&d);
uint8_t out[32];
rt_ctr_drbg_generate_all(&d, out, sizeof out);
rt_ctr_drbg_uninstantiate(&d);
```

## API — C++
```cpp
#include "randomtoad/ctr_drbg.hpp"
randomtoad::Drbg rng;
auto u = rng.u128();
```

## Estrutura
- `src/` — código‑fonte, Makefile e binários de teste (em `build/`)
- `docs/` — notas 90B/90C, security policy (skeleton)
- `examples/` — (opcional) exemplos adicionais
- `.github/workflows/ci.yml` — CI básico (build + selftest)

## Licença
MIT — ver [LICENSE](LICENSE).
