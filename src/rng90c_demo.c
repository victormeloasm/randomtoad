#include "rng90c.h"
#include "module_fips.h"
#include <stdio.h>
int main(void){
    if (rt_module_init() != 0) { fprintf(stderr,"module init failed\n"); return 1; }
    rt_rng90c r;
    int rc = rt_rng90c_init(&r);
    if (rc){ fprintf(stderr,"rng90c_init=%d\n", rc); return 1; }
    uint8_t buf[32];
    rc = rt_rng90c_generate(&r, buf, sizeof buf, 1);
    if (rc){ fprintf(stderr,"rng90c_generate=%d\n", rc); return 1; }
    printf("rng90c (PR) 32B:");
    for (int i=0;i<32;i++) printf(" %02x", buf[i]);
    printf("\n");
    rt_rng90c_uninit(&r);
    return 0;
}
