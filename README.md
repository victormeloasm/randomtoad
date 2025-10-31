# 🐸 RandomToad — CTR_DRBG AES‑256 (NIST SP 800‑90A Rev.1)

[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](#license)
![C](https://img.shields.io/badge/C-17-blue)
![C%2B%2B](https://img.shields.io/badge/C%2B%2B-20-blue)
![AES-NI](https://img.shields.io/badge/AES--NI-required-important)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Ubuntu-lightgrey)
![Clang+LLD](https://img.shields.io/badge/toolchain-clang%2Blld-brightgreen)
![FASM](https://img.shields.io/badge/FASM-ready-purple)
![Made with love](https://img.shields.io/badge/made%20with-love-ff69b4)

<p align="center">
  <img src="assets/wk.png" alt="RandomToad CTR_DRBG (AES-256 CTR_DRBG)" width="420">
</p>

**RandomToad** is a **C/C++** library for cryptographically secure random number generation based on **CTR_DRBG (AES‑256) no_df** as defined in **NIST SP 800‑90A Rev.1**, with entropy collection and health tests aligned with **SP 800‑90B**, and a **90C** composition layer (optional prediction resistance and periodic reseed). It uses **AES‑NI** and avoids blocking on `/dev/urandom` by sourcing from **RDSEED** and `getrandom(GRND_NONBLOCK)` with exponential backoff.

> 🐸 *Sapic summary:* fast, robust, and easy to integrate — with startup **self‑tests** (KAT) and a hardened **FIPS mode** (error latch and integrity checks). Ideal for services, CLIs, and libraries that need a solid DRBG core.


<p align="center">
  <a href="https://github.com/victormeloasm/randomtoad/releases/download/v1/randomtoad.zip">
    <img src="https://img.shields.io/badge/Download-randomtoad.zip-brightgreen?logo=github" alt="Download randomtoad.zip">
  </a>
</p>

---

## ✨ Highlights

- ✅ **CTR_DRBG (AES‑256) no_df** — NIST SP 800‑90A Rev.1
- 🔐 **Robust seeding**: **RDSEED** → `getrandom(GRND_NONBLOCK)` (non‑blocking, with backoff)
- 🩺 **Self‑tests**: power‑up Known‑Answer Test (DRBG KAT)
- 🧪 **Stat tests**: ready stream for **dieharder**, PractRand‑friendly
- 🧯 **FIPS mode**: error latch, power‑up self‑tests, `RT_FIPS_MODE=1`
- ⚙️ **No entropy stalls** (great for containers/early boot)
- 🧵 **Thread‑safe by context**: 1 `rt_ctr_drbg` per thread
- 🧰 Demos in **C**, **C++**, and **FASM**; `make install` for headers + lib

---

## 🚀 Quick start

### Dependencies
```bash
sudo apt-get update
sudo apt-get install -y clang lld fasm dieharder
```

### Clone & build
```bash
git clone https://github.com/<YOUR_USER>/randomtoad-ctr-drbg.git
cd randomtoad-ctr-drbg/src

make                   # builds everything
./build/selftest       # expected: SELFTEST: OK
```

> 🧪 Quick statistical smoke test:
```bash
./build/stream_randomtoad | dieharder -a -g 200
```

---

## 🛠️ Make targets

- `make` — Build library + demos. Auto‑generates `kat90a_vectors.h` on first build if needed (DRBG KAT).
- `make fips` — Build with hardened flags for **FIPS mode** (stricter checks/startup).
- `make strict_kat` — Enable strict AES + DRBG KAT (useful for deterministic environments/CI).
- `make regen_kat` — Manually regenerate KAT vectors for the DRBG (`kat90a_vectors.h`, 64 bytes).
- `make install` — Install headers and static library:
  - headers → `/usr/local/include/randomtoad`
  - lib     → `/usr/local/lib/librandomtoad.a`
- `make clean` — Clean build artifacts.

Binaries in `./build/`:
- `selftest`, `test_randomtoad`, `demo_cpp`, `demo_fasm_c`, `demo_fasm_cpp`, `stream_randomtoad`

---

## 🔐 Self‑test & FIPS mode

- **Power‑up self‑test** (default): validates DRBG (KAT), basic integrity, and init.
  ```bash
  ./build/selftest
  # SELFTEST: OK
  ```

- **FIPS mode (runtime)**: enables extra checks and error latch via env var.
  ```bash
  RT_FIPS_MODE=1 ./build/selftest
  # SELFTEST: OK
  ```

- **FIPS mode (compile‑time)**: hardens the binary at build time.
  ```bash
  make clean && make fips && ./build/selftest
  ```

> Tip: after touching the DRBG/AES core or flags/ISA, run `make regen_kat` to refresh the KAT header and commit it to the repo.

---

## 🔡 C API (essentials)

Headers:
```c
#include "randomtoad/ctr_drbg.h"
#include "randomtoad/module_fips.h"   // optional (FIPS mode helpers)
```

### Minimal example (generate 32 bytes)
```c
#include <stdio.h>
#include "randomtoad/ctr_drbg.h"
#include "randomtoad/module_fips.h"

int main(void) {
    rt_module_init(); // global latch/state (FIPS mode)
    rt_ctr_drbg ctx;

    if (rt_ctr_drbg_instantiate_system(&ctx) != 0) {
        fprintf(stderr, "instantiate failed\n");
        return 1;
    }

    uint8_t out[32];
    if (rt_ctr_drbg_generate_all(&ctx, out, sizeof out) != 0) {
        fprintf(stderr, "generate failed\n");
        return 1;
    }

    for (size_t i = 0; i < sizeof out; i++) printf("%02x", out[i]);
    printf("\n");

    rt_ctr_drbg_uninstantiate(&ctx);
    return 0;
}
```

### 128‑bit numbers (hex + decimal)
```c
#include <inttypes.h>
#include <stdio.h>
#include "randomtoad/ctr_drbg.h"

static void print_u128_hex_dec(uint64_t x[2]) {
    // hex (big‑endian view)
    printf("HEX: 0x%016" PRIx64 "%016" PRIx64 "\n", x[0], x[1]);

    // decimal via __int128
    __uint128_t v = (((__uint128_t)x[0]) << 64) | x[1];
    char buf[64]; int p = 63; buf[p] = '\0';
    if (v == 0) { puts("DEC: 0"); return; }
    while (v) { buf[--p] = '0' + (v % 10); v /= 10; }
    printf("DEC: %s\n", buf + p);
}

int main(void) {
    rt_ctr_drbg d; rt_ctr_drbg_instantiate_system(&d);
    uint64_t u128[2];
    rt_ctr_drbg_generate_u128(&d, u128);
    print_u128_hex_dec(u128);
    rt_ctr_drbg_uninstantiate(&d);
}
```

### Core functions
```c
int  rt_module_init(void);                       // init global latch/state
int  rt_ctr_drbg_instantiate_system(rt_ctr_drbg*);
int  rt_ctr_drbg_reseed_no_df_ex(rt_ctr_drbg*, const uint8_t seed_material48[48], const uint8_t* add_input48_or_null);
int  rt_ctr_drbg_generate_ex(rt_ctr_drbg*, uint8_t* out, size_t outlen, const uint8_t* add_input48_or_null);
int  rt_ctr_drbg_generate_all(rt_ctr_drbg*, uint8_t* out, size_t outlen);
int  rt_ctr_drbg_generate_u128(rt_ctr_drbg*, uint64_t out[2]);
void rt_ctr_drbg_uninstantiate(rt_ctr_drbg*);
int  rt_module_is_error(void);                   // 0=OK; non‑zero indicates error latch (FIPS mode)
```

---

## 🐍 C++ wrapper

```cpp
#include <array>
#include <string>
#include "randomtoad/ctr_drbg.hpp"

int main() {
    randomtoad::Drbg rng;          // instantiate and seed
    auto u = rng.u128();           // std::array<uint64_t,2>
    auto bytes = rng.bytes(32);    // std::vector<uint8_t>
    auto hex = rng.hex(32);        // std::string (64 hex chars)
    return 0;
}
```

Main signatures:
```cpp
namespace randomtoad {
  struct Drbg {
    Drbg();                       // instantiate_system()
    ~Drbg();                      // uninstantiate()
    std::array<uint64_t,2> u128();
    std::vector<uint8_t>   bytes(size_t n);
    std::string            hex(size_t n);
  };
}
```

---

## 🧩 FASM demo

- `demo_fasm_c` and `demo_fasm_cpp` show how to link the ASM object (`randomtoad_fasm.o`) with C/C++.
- Built automatically by `make` when FASM is installed.

```asm
; randomtoad_fasm.asm (excerpt)
format ELF64
public rt_fasm_u128_gen
section '.text' executable
rt_fasm_u128_gen:
    ; ... call C generator and return u128 ...
    ret
```

---

## 🌱 Seeding & Reseed (SP 800‑90B / 90C alignment)

- **Primary source:** `RDSEED` (hardware).
- **Non‑blocking fallback:** `getrandom(GRND_NONBLOCK)` + exponential backoff retries.
- **Personalization String:** optional to separate instances/services.
- **Prediction Resistance:** via the 90C‑aligned layer (`rng90c_*`), which forces `Reseed()` for PR requests.
- **Policies:** periodic reseed based on generation counters/time.

> Containers fresh from boot may return `-EAGAIN` from `getrandom(GRND_NONBLOCK)`; the collector applies backoff and retries without blocking.

---

## ⚡ Performance & build tips

- Requires **AES‑NI**. Recommended compile flags:
  ```bash
  CFLAGS="-O3 -march=native -maes -mrdseed" make
  ```
- Link with **LLD** for faster linking: `-fuse-ld=lld` (already used in the Makefile).
- Use **one `rt_ctr_drbg` per thread**; avoid shared state across threads.
- For large buffers, call `rt_ctr_drbg_generate_all()` (efficient internal loop).

---

## 🧪 Statistical testing

### dieharder
```bash
./build/stream_randomtoad | dieharder -a -g 200
```

### PractRand (optional)
```bash
./build/stream_randomtoad | RNG_test stdin64 -tf 2 -te 8
```

---

## 🔒 KAT (Known‑Answer Test)

- `kat90a_vectors.h` carries the **64‑byte** expected output for a fixed DRBG seed.  
- If you modify the DRBG/AES core or change ISA/flags, regenerate:
  ```bash
  make regen_kat
  git add src/kat90a_vectors.h
  git commit -m "Update DRBG KAT vectors"
  ```
- The **selftest** compares runtime output with KAT; mismatches fail initialization (FIPS mode keeps the error latch set).

---

## 📦 Install & link

```bash
cd src
sudo make install
# headers → /usr/local/include/randomtoad
# lib     → /usr/local/lib/librandomtoad.a

# Example (C)
clang -O3 main.c -I/usr/local/include/randomtoad -L/usr/local/lib -lrandomtoad -fuse-ld=lld -o app
```

If you want `pkg-config` or `CMake`, open an issue — v2.2 skeleton is ready.

---

## 🗺️ Repository layout

```
randomtoad-ctr-drbg/
├─ README.md
├─ LICENSE
├─ .gitignore
├─ .editorconfig
├─ .github/workflows/ci.yml
└─ src/
   ├─ *.c / *.h / *.hpp / *.cpp / *.asm / Makefile
   ├─ build/ (generated by make)
   └─ kat90a_vectors.h (generated by prepare_kat/regen_kat)
```

---

## 🤝 Contributing

- Please attach logs (`make V=1`), `clang --version`, and `grep aes /proc/cpuinfo`.
- Pull Requests: focus on **security**, **clarity**, and **tests** (selftest + dieharder smoke).
- Style: `clang-format` (Google style) recommended.

---

## 📜 License

**MIT** — simple and permissive. See [LICENSE](LICENSE).

---

## 💚 Sapic outro

> “Forged in the Arcane Swamp with love, hex, and a pinch of **croaack**.”  
> If this project helps you, drop a ⭐ on GitHub — random frogs will smile. 🐸✨
