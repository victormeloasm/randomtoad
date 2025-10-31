#include "ctr_drbg.h"
#include "module_fips.h"
#include <stdio.h>
#include <inttypes.h>

static void print_u128_hex_dec(uint64_t v[2]) {
    printf("HEX: 0x%016" PRIx64 "%016" PRIx64 "\n", v[1], v[0]);
#if defined(__SIZEOF_INT128__)
    __uint128_t x = (((__uint128_t)v[1]) << 64) | v[0];
    char buf[64]; int idx = 63; buf[idx] = '\0'; idx--;
    if (x == 0) { puts("DEC: 0"); return; }
    while (x > 0 && idx >= 0) { unsigned d = (unsigned)(x % 10); x /= 10; buf[idx--] = (char)('0' + d); }
    printf("DEC: %s\n", &buf[idx+1]);
#else
    printf("DEC parts: hi=%" PRIu64 ", lo=%" PRIu64 "\n", v[1], v[0]);
#endif
}

int main(void) {
    if (rt_module_init() != 0) { fprintf(stderr, "module init failed\n"); return 1; }

    rt_ctr_drbg drbg;
    int rc = rt_ctr_drbg_instantiate_system(&drbg);
    if (rc != 0) { fprintf(stderr, "instantiate failed: %d\n", rc); return 1; }

    for (int i = 0; i < 3; i++) {
        uint64_t v[2];
        rc = rt_ctr_drbg_generate_u128(&drbg, v);
        if (rc != 0) { fprintf(stderr, "generate failed: %d\n", rc); return 1; }
        printf("u128[%d]\n", i); print_u128_hex_dec(v);
    }
    rt_ctr_drbg_uninstantiate(&drbg);
    return 0;
}
