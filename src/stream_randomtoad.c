#include "ctr_drbg.h"
#include "module_fips.h"
#include <stdio.h>
int main(void){
    if (rt_module_init() != 0) return 1;
    rt_ctr_drbg d;
    if (rt_ctr_drbg_instantiate_system(&d) != 0) return 2;
    unsigned char buf[1<<16];
    for(;;){
        if (rt_ctr_drbg_generate(&d, buf, sizeof buf) != 0) return 3;
        if (fwrite(buf,1,sizeof buf,stdout) != sizeof buf) return 0;
    }
}
