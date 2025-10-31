// SPDX-License-Identifier: MIT
// "FIPS-mode" shim: power-up selftests + error latch (NIST-like)

#ifndef RANDOMTOAD_MODULE_FIPS_H
#define RANDOMTOAD_MODULE_FIPS_H

#ifdef __cplusplus
extern "C" {
#endif

int rt_module_init(void);
int rt_module_is_error(void);
void rt_module_force_fips_mode(int on);

#ifdef __cplusplus
}
#endif
#endif
