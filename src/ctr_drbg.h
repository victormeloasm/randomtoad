// SPDX-License-Identifier: MIT
// NIST SP 800-90A Rev.1 — CTR_DRBG(AES-256) no_df (§10.2.1.3)

#ifndef RANDOMTOAD_CTR_DRBG_H
#define RANDOMTOAD_CTR_DRBG_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RT_ALIGN16
#  ifdef __cplusplus
#    define RT_ALIGN16 alignas(16)
#  elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#    include <stdalign.h>
#    define RT_ALIGN16 _Alignas(16)
#  else
#    define RT_ALIGN16 __attribute__((aligned(16)))
#  endif
#endif

#define RT_MAX_BYTES_PER_REQUEST 65536u
#define RT_RESEED_INTERVAL ((uint64_t)281474976710655ULL) // 2^48 - 1

typedef struct rt_ctr_drbg {
    RT_ALIGN16 uint8_t Key[32];
    RT_ALIGN16 uint8_t V[16];
    uint64_t reseed_counter;
} rt_ctr_drbg;

// 90A core
int rt_aesni_available(void);
int rt_ctr_drbg_instantiate_no_df(rt_ctr_drbg* ctx, const uint8_t seed_material48[48]);
int rt_ctr_drbg_reseed_no_df(rt_ctr_drbg* ctx, const uint8_t seed_material48[48]);
int rt_ctr_drbg_generate(rt_ctr_drbg* ctx, uint8_t* out, size_t outlen);
int rt_ctr_drbg_generate_u128(rt_ctr_drbg* ctx, uint64_t out[2]);
int rt_ctr_drbg_generate_all(rt_ctr_drbg* ctx, uint8_t* out, size_t len);
void rt_ctr_drbg_uninstantiate(rt_ctr_drbg* ctx);

// Linux helper
int rt_system_seed48_nonblocking(uint8_t out48[48]);
int rt_ctr_drbg_instantiate_system(rt_ctr_drbg* ctx);

#ifdef __cplusplus
}
#endif
#endif
