// SPDX-License-Identifier: MIT
// SP 800-90C-like composition (entropy + DRBG + reseed/PR policy)

#ifndef RANDOMTOAD_RNG90C_H
#define RANDOMTOAD_RNG90C_H

#include <stdint.h>
#include <stddef.h>
#include "ctr_drbg.h"
#include "entropy_90b.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rt_rng90c {
    rt_ctr_drbg          drbg;
    rt_entropy_90b_state ent;
    uint64_t             gen_counter;
    uint64_t             reseed_interval;
} rt_rng90c;

int rt_rng90c_init(rt_rng90c* r);
int rt_rng90c_generate(rt_rng90c* r, uint8_t* out, size_t len, int prediction_resistance);
int rt_rng90c_reseed(rt_rng90c* r);
void rt_rng90c_uninit(rt_rng90c* r);

#ifdef __cplusplus
}
#endif
#endif
