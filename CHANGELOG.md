# Changelog
All notable changes to this project will be documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2025-10-31
### Added
- **CTR_DRBG (AES-256) no_df** per **NIST SP 800-90A Rev.1**.
- Entropy collection aligned com **SP 800-90B** e camada de composição **90C** (opcional).
- **C API** estável (`rt_ctr_drbg_*`, `rt_module_*`) e **C++ wrapper** minimalista (`randomtoad::Drbg`).
- **Self-tests** de inicialização (KAT) com `kat90a_vectors.h` (64 bytes).
- **FIPS mode**: error latch e integridade (runtime via `RT_FIPS_MODE=1` e build via `make fips`).
- **Demos**: C, C++ e **FASM** (linkando objeto `.asm`).
- **Make targets**: `make`, `make fips`, `make strict_kat`, `make regen_kat`, `make install`, `make clean`.
- **Stream tool** (`./build/stream_randomtoad`) pronto para **dieharder** e **PractRand**.
- **Docs**: README com quickstart, API, exemplos, DIEHARDER-approved block.

### Changed
- Toolchain padrão para **clang + lld**; flags recomendadas: `-O3 -march=native -maes -mrdseed -fuse-ld=lld`.

### Performance
- Throughput observado no run de referência do usuário: **~1.12e+08 rands/s** (dieharder stdin, seed 3724320235).

### Security
- Política de reseed periódico e suporte a **Prediction Resistance** via camada 90C.
- Inicialização falha se **KAT** não corresponder (em FIPS mode, error latch permanece setado).

### Known Issues
- Em ambientes sem suporte a **RDSEED**, a coleta usa `getrandom(GRND_NONBLOCK)` com backoff exponencial (sem bloquear).
- Runners de CI podem não ter `dieharder` instalado por padrão; o workflow inclui instalação e um smoke test reduzido.

[1.0.0]: https://github.com/victormeloasm/randomtoad/releases/tag/v1
