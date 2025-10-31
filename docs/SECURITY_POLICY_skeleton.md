# Security Policy (Skeleton) — RandomToad DRBG Module (NIST‑like)

- **Module Name:** RandomToad DRBG Module
- **Version:** 2.1.1 (NIST‑like)
- **Roles & Services:** User — {Instantiate, Generate, Reseed, Uninstantiate}. Sem key entry/exit.
- **Algorithms:** CTR_DRBG(AES‑256) por SP 800‑90A Rev.1 (*não FIPS‑validado*).
- **90C Composition:** 90B collector + DRBG 90A. PR opcional.
- **Self‑Tests:**
  - Power‑up: DRBG KAT (seed fixa, 64 bytes). AES KAT estrito opcional.
  - Conditional: pós‑generate update.
- **Error Handling:** Em falha, entra em erro (latched) e todos serviços falham.
- **OE:** Linux x86‑64 com AES‑NI e RDSEED.
