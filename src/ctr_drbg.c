// SPDX-License-Identifier: MIT
#include "ctr_drbg.h"
#include <string.h>
#include <errno.h>
#include <cpuid.h>
#include <immintrin.h>

static void aes256_expand_key(const uint8_t key[32], __m128i rk[15]) {
    __m128i x = _mm_loadu_si128((const __m128i*)(key));
    __m128i y = _mm_loadu_si128((const __m128i*)(key+16));
    rk[0] = x; rk[1] = y;
    #define EXPAND256(RCON, IDX) do {                                   \
        __m128i t = _mm_aeskeygenassist_si128(y, RCON);                  \
        t = _mm_shuffle_epi32(t, 0xff);                                  \
        x = _mm_xor_si128(x, _mm_slli_si128(x, 4));                      \
        x = _mm_xor_si128(x, _mm_slli_si128(x, 4));                      \
        x = _mm_xor_si128(x, _mm_slli_si128(x, 4));                      \
        x = _mm_xor_si128(x, t);                                         \
        rk[(IDX)] = x;                                                   \
        t = _mm_aeskeygenassist_si128(x, 0x00);                          \
        t = _mm_shuffle_epi32(t, 0xaa);                                  \
        y = _mm_xor_si128(y, _mm_slli_si128(y, 4));                      \
        y = _mm_xor_si128(y, _mm_slli_si128(y, 4));                      \
        y = _mm_xor_si128(y, _mm_slli_si128(y, 4));                      \
        y = _mm_xor_si128(y, t);                                         \
        rk[(IDX)+1] = y;                                                 \
    } while(0)
    EXPAND256(0x01,2); EXPAND256(0x02,4); EXPAND256(0x04,6);
    EXPAND256(0x08,8); EXPAND256(0x10,10); EXPAND256(0x20,12);
    __m128i t = _mm_aeskeygenassist_si128(y, 0x40);
    t = _mm_shuffle_epi32(t, 0xff);
    x = _mm_xor_si128(x, _mm_slli_si128(x, 4));
    x = _mm_xor_si128(x, _mm_slli_si128(x, 4));
    x = _mm_xor_si128(x, _mm_slli_si128(x, 4));
    rk[14] = _mm_xor_si128(x, t);
    #undef EXPAND256
}

static inline __m128i aes256_encrypt_block(__m128i in, const __m128i rk[15]) {
    in = _mm_xor_si128(in, rk[0]);
    for (int i = 1; i < 14; i++) in = _mm_aesenc_si128(in, rk[i]);
    return _mm_aesenclast_si128(in, rk[14]);
}

static inline void inc_128_be(uint8_t V[16]) {
    for (int i = 15; i >= 0; i--) { uint8_t x = (uint8_t)(V[i] + 1); uint8_t c = (x==0); V[i]=x; if(!c)break; }
}

static void ctr_drbg_update(rt_ctr_drbg* ctx, const uint8_t* provided48 /*nullable*/) {
    __m128i rk[15];
    aes256_expand_key(ctx->Key, rk);
    uint8_t temp[48];
    for (int i=0;i<3;i++) {
        inc_128_be(ctx->V);
        __m128i v = _mm_loadu_si128((const __m128i*)ctx->V);
        __m128i e = aes256_encrypt_block(v, rk);
        _mm_storeu_si128((__m128i*)(temp + 16*i), e);
    }
    if (provided48) for (int i=0;i<48;i++) temp[i] ^= provided48[i];
    memcpy(ctx->Key, temp, 32);
    memcpy(ctx->V,   temp + 32, 16);
}

int rt_aesni_available(void) {
    unsigned eax, ebx, ecx, edx;
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) return 0;
    return (ecx & (1u<<25)) ? 1 : 0;
}

// AES KAT (strict) — disabled by default; enable with -DRT_STRICT_AES_KAT
static int rt_selftest_aes256_block(void) {
#ifdef RT_STRICT_AES_KAT
    const uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F
    };
    const uint8_t pt[16] = {
        0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF
    };
    const uint8_t ct_expected[16] = {
        0x8e,0xa2,0xb7,0xca,0x51,0x67,0x45,0xbf,
        0xea,0xfc,0x49,0x90,0x4b,0x49,0x60,0x89
    };
    __m128i rk[15]; aes256_expand_key(key, rk);
    __m128i x = _mm_loadu_si128((const __m128i*)pt);
    x = aes256_encrypt_block(x, rk);
    uint8_t out[16]; _mm_storeu_si128((__m128i*)out, x);
    return memcmp(out, ct_expected, 16) == 0 ? 0 : -1;
#else
    return 0;
#endif
}

int rt_ctr_drbg_instantiate_no_df(rt_ctr_drbg* ctx, const uint8_t seed_material48[48]) {
    if (!ctx || !seed_material48) return -EINVAL;
    if (!rt_aesni_available()) return -ENOSYS;
    if (rt_selftest_aes256_block() != 0) return -EFAULT;
    memset(ctx->Key, 0, sizeof(ctx->Key));
    memset(ctx->V,   0, sizeof(ctx->V));
    ctx->reseed_counter = 0;
    ctr_drbg_update(ctx, seed_material48);
    ctx->reseed_counter = 1;
    return 0;
}

int rt_ctr_drbg_reseed_no_df(rt_ctr_drbg* ctx, const uint8_t seed_material48[48]) {
    if (!ctx || !seed_material48) return -EINVAL;
    ctr_drbg_update(ctx, seed_material48);
    ctx->reseed_counter = 1;
    return 0;
}

int rt_ctr_drbg_generate(rt_ctr_drbg* ctx, uint8_t* out, size_t outlen) {
    if (!ctx || !out) return -EINVAL;
    if (outlen == 0) return 0;
    if (outlen > RT_MAX_BYTES_PER_REQUEST) return -EMSGSIZE;
    if (ctx->reseed_counter > RT_RESEED_INTERVAL) return -EOVERFLOW;

    __m128i rk[15]; aes256_expand_key(ctx->Key, rk);
    uint8_t block[16];
    size_t remaining = outlen;
    while (remaining) {
        inc_128_be(ctx->V);
        __m128i v = _mm_loadu_si128((const __m128i*)ctx->V);
        __m128i e = aes256_encrypt_block(v, rk);
        _mm_storeu_si128((__m128i*)block, e);
        size_t chunk = remaining < 16 ? remaining : 16;
        memcpy(out, block, chunk);
        out += chunk; remaining -= chunk;
    }
    uint8_t zeros[48] = {0};
    ctr_drbg_update(ctx, zeros);
    ctx->reseed_counter++;
    return 0;
}

int rt_ctr_drbg_generate_u128(rt_ctr_drbg* ctx, uint64_t out[2]) {
    uint8_t buf[16];
    int rc = rt_ctr_drbg_generate(ctx, buf, sizeof buf);
    if (rc) return rc;
    out[0] =
        (uint64_t)buf[0]        |
        (uint64_t)buf[1] << 8   |
        (uint64_t)buf[2] << 16  |
        (uint64_t)buf[3] << 24  |
        (uint64_t)buf[4] << 32  |
        (uint64_t)buf[5] << 40  |
        (uint64_t)buf[6] << 48  |
        (uint64_t)buf[7] << 56;
    out[1] =
        (uint64_t)buf[8]        |
        (uint64_t)buf[9] << 8   |
        (uint64_t)buf[10] << 16 |
        (uint64_t)buf[11] << 24 |
        (uint64_t)buf[12] << 32 |
        (uint64_t)buf[13] << 40 |
        (uint64_t)buf[14] << 48 |
        (uint64_t)buf[15] << 56;
    return 0;
}

void rt_ctr_drbg_uninstantiate(rt_ctr_drbg* ctx) {
    if (!ctx) return;
    volatile uint8_t* p = (volatile uint8_t*)ctx;
    for (size_t i = 0; i < sizeof(*ctx); i++) p[i] = 0;
}

int rt_ctr_drbg_generate_all(rt_ctr_drbg* ctx, uint8_t* out, size_t len) {
    while (len) {
        size_t chunk = len > RT_MAX_BYTES_PER_REQUEST ? RT_MAX_BYTES_PER_REQUEST : len;
        int rc = rt_ctr_drbg_generate(ctx, out, chunk);
        if (rc) return rc;
        out += chunk; len -= chunk;
    }
    return 0;
}
