# 🐸 RandomToad — CTR_DRBG AES‑256 (NIST SP 800‑90A Rev.1)

[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](#license)
![C](https://img.shields.io/badge/C-17-blue)
![C%2B%2B](https://img.shields.io/badge/C%2B%2B-20-blue)
![AES-NI](https://img.shields.io/badge/AES--NI-required-important)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Ubuntu-lightgrey)
![Clang+LLD](https://img.shields.io/badge/toolchain-clang%2Blld-brightgreen)
![FASM](https://img.shields.io/badge/FASM-ready-purple)
![Made with love](https://img.shields.io/badge/made%20with-love-ff69b4)
[![dieharder](https://img.shields.io/badge/dieharder-APPROVED-brightgreen)](#-dieharder-approved)



<p align="center">
  <img src="assets/wk.png" alt="RandomToad CTR_DRBG (AES-256 CTR_DRBG)" width="420">
</p>

**RandomToad** is a **C/C++** library for cryptographically secure random number generation based on **CTR_DRBG (AES‑256) no_df** as defined in **NIST SP 800‑90A Rev.1**, with entropy collection and health tests aligned with **SP 800‑90B**, and a **90C** composition layer (optional prediction resistance and periodic reseed). It uses **AES‑NI** and avoids blocking on `/dev/urandom` by sourcing from **RDSEED** and `getrandom(GRND_NONBLOCK)` with exponential backoff.

> 🐸 *Sapic summary:* fast, robust, and easy to integrate — with startup **self‑tests** (KAT) and a hardened **FIPS mode** (error latch and integrity checks). Ideal for services, CLIs, and libraries that need a solid DRBG core.



[![randomtoad.zip](https://img.shields.io/badge/randomtoad.zip-Download-brightgreen?logo=github)](https://github.com/victormeloasm/randomtoad/releases/download/v1/randomtoad.zip)




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
    char buf[64]; int p = 63; buf[p] = '\\0';
    if (v == 0) { puts("DEC: 0"); return; }
    while (v) { buf[--p] = '0' + (v % 10); v /= 10; }
    printf("DEC: %s\\n", buf + p);
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


### 🧪 DIEHARDER — Full run (Approved)

**dieharder** v3.31.1
**RNG:** `stdin_input_raw` (from `./build/stream_randomtoad`)
**Throughput:** `1.12e+08 rands/second`
**Seed:** `3724320235`

```bash
./build/stream_randomtoad | dieharder -a -g 200
```

| test_name            | ntup | tsamples | psamples |    p-value | Assessment |
| -------------------- | ---: | -------: | -------: | ---------: | ---------- |
| diehard_birthdays    |    0 |      100 |      100 | 0.83031320 | PASSED     |
| diehard_operm5       |    0 |  1000000 |      100 | 0.74481735 | PASSED     |
| diehard_rank_32x32   |    0 |    40000 |      100 | 0.42400949 | PASSED     |
| diehard_rank_6x8     |    0 |   100000 |      100 | 0.78709691 | PASSED     |
| diehard_bitstream    |    0 |  2097152 |      100 | 0.43467389 | PASSED     |
| diehard_opso         |    0 |  2097152 |      100 | 0.74516730 | PASSED     |
| diehard_oqso         |    0 |  2097152 |      100 | 0.00588422 | PASSED     |
| diehard_dna          |    0 |  2097152 |      100 | 0.45683585 | PASSED     |
| diehard_count_1s_str |    0 |   256000 |      100 | 0.63288635 | PASSED     |
| diehard_count_1s_byt |    0 |   256000 |      100 | 0.87491359 | PASSED     |
| diehard_parking_lot  |    0 |    12000 |      100 | 0.82824986 | PASSED     |
| diehard_2dsphere     |    2 |     8000 |      100 | 0.52509591 | PASSED     |
| diehard_3dsphere     |    3 |     4000 |      100 | 0.11927057 | PASSED     |
| diehard_squeeze      |    0 |   100000 |      100 | 0.64031189 | PASSED     |
| diehard_sums         |    0 |      100 |      100 | 0.39643969 | PASSED     |
| diehard_runs         |    0 |   100000 |      100 | 0.25113099 | PASSED     |
| diehard_runs         |    0 |   100000 |      100 | 0.56733791 | PASSED     |
| diehard_craps        |    0 |   200000 |      100 | 0.56910441 | PASSED     |
| diehard_craps        |    0 |   200000 |      100 | 0.06140167 | PASSED     |
| marsaglia_tsang_gcd  |    0 | 10000000 |      100 | 0.99089350 | PASSED     |
| marsaglia_tsang_gcd  |    0 | 10000000 |      100 | 0.42155183 | PASSED     |
| sts_monobit          |    1 |   100000 |      100 | 0.52943540 | PASSED     |
| sts_runs             |    2 |   100000 |      100 | 0.47104321 | PASSED     |
| sts_serial           |    1 |   100000 |      100 | 0.61481979 | PASSED     |
| sts_serial           |    2 |   100000 |      100 | 0.08333684 | PASSED     |
| sts_serial           |    3 |   100000 |      100 | 0.87893428 | PASSED     |
| sts_serial           |    3 |   100000 |      100 | 0.11961521 | PASSED     |
| sts_serial           |    4 |   100000 |      100 | 0.46168006 | PASSED     |
| sts_serial           |    4 |   100000 |      100 | 0.09396190 | PASSED     |
| sts_serial           |    5 |   100000 |      100 | 0.30215362 | PASSED     |
| sts_serial           |    5 |   100000 |      100 | 0.31749830 | PASSED     |
| sts_serial           |    6 |   100000 |      100 | 0.55510378 | PASSED     |
| sts_serial           |    6 |   100000 |      100 | 0.31838512 | PASSED     |
| sts_serial           |    7 |   100000 |      100 | 0.86334003 | PASSED     |
| sts_serial           |    7 |   100000 |      100 | 0.96053285 | PASSED     |
| sts_serial           |    8 |   100000 |      100 | 0.68708662 | PASSED     |
| sts_serial           |    8 |   100000 |      100 | 0.88148598 | PASSED     |
| sts_serial           |    9 |   100000 |      100 | 0.52679296 | PASSED     |
| sts_serial           |    9 |   100000 |      100 | 0.03046673 | PASSED     |
| sts_serial           |   10 |   100000 |      100 | 0.05631905 | PASSED     |
| sts_serial           |   10 |   100000 |      100 | 0.08362386 | PASSED     |
| sts_serial           |   11 |   100000 |      100 | 0.15366399 | PASSED     |
| sts_serial           |   11 |   100000 |      100 | 0.85669406 | PASSED     |
| sts_serial           |   12 |   100000 |      100 | 0.59655549 | PASSED     |
| sts_serial           |   12 |   100000 |      100 | 0.78678816 | PASSED     |
| sts_serial           |   13 |   100000 |      100 | 0.45213662 | PASSED     |
| sts_serial           |   13 |   100000 |      100 | 0.82469028 | PASSED     |
| sts_serial           |   14 |   100000 |      100 | 0.07735550 | PASSED     |
| sts_serial           |   14 |   100000 |      100 | 0.71070974 | PASSED     |
| sts_serial           |   15 |   100000 |      100 | 0.67420298 | PASSED     |
| sts_serial           |   15 |   100000 |      100 | 0.87286353 | PASSED     |
| sts_serial           |   16 |   100000 |      100 | 0.35428064 | PASSED     |
| sts_serial           |   16 |   100000 |      100 | 0.79934307 | PASSED     |
| rgb_bitdist          |    1 |   100000 |      100 | 0.82079433 | PASSED     |
| rgb_bitdist          |    2 |   100000 |      100 | 0.94232090 | PASSED     |
| rgb_bitdist          |    3 |   100000 |      100 | 0.29581975 | PASSED     |
| rgb_bitdist          |    4 |   100000 |      100 | 0.70209456 | PASSED     |
| rgb_bitdist          |    5 |   100000 |      100 | 0.61439614 | PASSED     |
| rgb_bitdist          |    6 |   100000 |      100 | 0.09692964 | PASSED     |
| rgb_bitdist          |    7 |   100000 |      100 | 0.68356198 | PASSED     |
| rgb_bitdist          |    8 |   100000 |      100 | 0.62835271 | PASSED     |
| rgb_bitdist          |    9 |   100000 |      100 | 0.87724158 | PASSED     |
| rgb_bitdist          |   10 |   100000 |      100 | 0.09457181 | PASSED     |
| rgb_bitdist          |   11 |   100000 |      100 | 0.37751974 | PASSED     |
| rgb_bitdist          |   12 |   100000 |      100 | 0.89674755 | PASSED     |
| rgb_minimum_distance |    2 |    10000 |     1000 | 0.93589150 | PASSED     |
| rgb_minimum_distance |    3 |    10000 |     1000 | 0.78120942 | PASSED     |
| rgb_minimum_distance |    4 |    10000 |     1000 | 0.66091291 | PASSED     |
| rgb_minimum_distance |    5 |    10000 |     1000 | 0.55541312 | PASSED     |
| rgb_permutations     |    2 |   100000 |      100 | 0.95735949 | PASSED     |
| rgb_permutations     |    3 |   100000 |      100 | 0.68570548 | PASSED     |
| rgb_permutations     |    4 |   100000 |      100 | 0.57733738 | PASSED     |
| rgb_permutations     |    5 |   100000 |      100 | 0.17846122 | PASSED     |
| rgb_lagged_sum       |    0 |  1000000 |      100 | 0.58831307 | PASSED     |
| rgb_lagged_sum       |    1 |  1000000 |      100 | 0.33432956 | PASSED     |
| rgb_lagged_sum       |    2 |  1000000 |      100 | 0.33136089 | PASSED     |
| rgb_lagged_sum       |    3 |  1000000 |      100 | 0.50617832 | PASSED     |
| rgb_lagged_sum       |    4 |  1000000 |      100 | 0.42195544 | PASSED     |
| rgb_lagged_sum       |    5 |  1000000 |      100 | 0.04977336 | PASSED     |
| rgb_lagged_sum       |    6 |  1000000 |      100 | 0.06863526 | PASSED     |
| rgb_lagged_sum       |    7 |  1000000 |      100 | 0.85918031 | PASSED     |
| rgb_lagged_sum       |    8 |  1000000 |      100 | 0.64081771 | PASSED     |
| rgb_lagged_sum       |    9 |  1000000 |      100 | 0.74190191 | PASSED     |
| rgb_lagged_sum       |   10 |  1000000 |      100 | 0.81436408 | PASSED     |
| rgb_lagged_sum       |   11 |  1000000 |      100 | 0.98798686 | PASSED     |
| rgb_lagged_sum       |   12 |  1000000 |      100 | 0.69985697 | PASSED     |
| rgb_lagged_sum       |   13 |  1000000 |      100 | 0.90576053 | PASSED     |
| rgb_lagged_sum       |   14 |  1000000 |      100 | 0.12770246 | PASSED     |
| rgb_lagged_sum       |   15 |  1000000 |      100 | 0.25179663 | PASSED     |
| rgb_lagged_sum       |   16 |  1000000 |      100 | 0.92564301 | PASSED     |
| rgb_lagged_sum       |   17 |  1000000 |      100 | 0.44555747 | PASSED     |
| rgb_lagged_sum       |   18 |  1000000 |      100 | 0.96169465 | PASSED     |
| rgb_lagged_sum       |   19 |  1000000 |      100 | 0.04168866 | PASSED     |
| rgb_lagged_sum       |   20 |  1000000 |      100 | 0.10457283 | PASSED     |
| rgb_lagged_sum       |   21 |  1000000 |      100 | 0.90938422 | PASSED     |
| rgb_lagged_sum       |   22 |  1000000 |      100 | 0.04969054 | PASSED     |
| rgb_lagged_sum       |   23 |  1000000 |      100 | 0.98911739 | PASSED     |
| rgb_lagged_sum       |   24 |  1000000 |      100 | 0.68325416 | PASSED     |
| rgb_lagged_sum       |   25 |  1000000 |      100 | 0.38978879 | PASSED     |
| rgb_lagged_sum       |   26 |  1000000 |      100 | 0.47365270 | PASSED     |
| rgb_lagged_sum       |   27 |  1000000 |      100 | 0.56837631 | PASSED     |
| rgb_lagged_sum       |   28 |  1000000 |      100 | 0.94750992 | PASSED     |
| rgb_lagged_sum       |   29 |  1000000 |      100 | 0.40207508 | PASSED     |
| rgb_lagged_sum       |   30 |  1000000 |      100 | 0.31056655 | PASSED     |
| rgb_lagged_sum       |   31 |  1000000 |      100 | 0.88054847 | PASSED     |
| rgb_lagged_sum       |   32 |  1000000 |      100 | 0.92074409 | PASSED     |
| rgb_kstest_test      |    0 |    10000 |     1000 | 0.05024557 | PASSED     |
| dab_bytedistrib      |    0 | 51200000 |        1 | 0.57924133 | PASSED     |
| dab_dct              |  256 |    50000 |        1 | 0.11382006 | PASSED     |
| dab_filltree         |   32 | 15000000 |        1 | 0.78316042 | PASSED     |
| dab_filltree         |   32 | 15000000 |        1 | 0.30601986 | PASSED     |
| dab_filltree2        |    0 |  5000000 |        1 | 0.12269215 | PASSED     |
| dab_filltree2        |    1 |  5000000 |        1 | 0.94876227 | PASSED     |
| dab_monobit2         |   12 | 65000000 |        1 | 0.66243250 | PASSED     |

> ✅ **All tests PASSED.** 

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
