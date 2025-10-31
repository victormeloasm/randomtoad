#include "ctr_drbg.h"
#include "module_fips.h"
#include <stdio.h>
#include <inttypes.h>

extern int fasm_randomtoad_u128(rt_ctr_drbg* ctx, uint64_t out[2]);

static void print_u128_hex_dec(uint64_t v[2]) {
    printf("HEX: 0x%016" PRIx64 "%016" PRIx64 "\n", v[1], v[0]);
#if defined(__SIZEOF_INT128__)
    __uint128_t x = (((__uint128_t)v[1]) << 64) | v[0];
    char buf[64]; int idx=63; buf[idx]='\0';
    if (x==0){ puts("DEC: 0"); return; }
    while (x>0 && idx>0){ unsigned d=(unsigned)(x%10); x/=10; buf[--idx]=(char)('0'+d); }
    printf("DEC: %s\n", &buf[idx]);
#else
    printf("DEC parts: hi=%" PRIu64 ", lo=%" PRIu64 "\n", v[1], v[0]);
#endif
}

int main(void){
    if (rt_module_init() != 0) { fprintf(stderr,"module init failed\n"); return 1; }
    rt_ctr_drbg drbg;
    int rc = rt_ctr_drbg_instantiate_system(&drbg);
    if (rc){ fprintf(stderr,"instantiate failed: %d\n", rc); return 1; }
    uint64_t v[2];
    rc = fasm_randomtoad_u128(&drbg, v);
    if (rc){ fprintf(stderr,"fasm_generate failed: %d\n", rc); return 1; }
    print_u128_hex_dec(v);
    rt_ctr_drbg_uninstantiate(&drbg);
    return 0;
}
