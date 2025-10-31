#include "module_fips.h"
#include <stdio.h>
int main(void){
    int rc = rt_module_init();
    if (rc != 0) { puts("SELFTEST: FAIL"); return 1; }
    puts("SELFTEST: OK");
    return 0;
}
