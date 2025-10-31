# RandomToad RNG — SP 800‑90C Profile (NIST‑like)

**Construction:** RBG2‑style composition (Entropy Source per 90B + 90A DRBG).  
**DRBG:** CTR_DRBG (AES‑256) *no_df* — SP 800‑90A Rev.1 §10.2.1.3.  
**Entropy input size:** 384 bits (48 bytes).  
**Prediction Resistance (PR):** Optional per‑request (on/off).  
**Reseed Policy:** reseed on PR requests, and periodic reseed every 2^20 generate calls.

**Instantiate()**
1. Collect 48 bytes from the system entropy collector (RDSEED, then getrandom(GRND_NONBLOCK)).
2. Run 90B‑like health tests (repetition count, adaptive proportion).
3. Instantiate CTR_DRBG(no_df) with seed_material = entropy_input (48 bytes).

**Generate(len, PR)**
- If PR=1 → Reseed() before generating.
- Else, if `gen_counter >= reseed_interval` → Reseed().
- Then output with `rt_ctr_drbg_generate_all()` (splits into ≤ 65,536‑byte chunks).

**Reseed()**
- 48 bytes entropy via collector → 90B health tests → `rt_ctr_drbg_reseed_no_df()`.

**States & Errors**
- Power‑up selftests (AES KAT optional, DRBG KAT) — in *FIPS-mode* (build com `-DFIPS_MODE` ou env `RT_FIPS_MODE=1`) o módulo falha e *latcha* se KAT falhar.
