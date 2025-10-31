// SPDX-License-Identifier: MIT
#include "rng90c.h"
#include <string.h>
#include <errno.h>

static int rt_linux_entropy_source(uint8_t* buf, size_t len, void* user) {
    (void)user;
    if (len != 48) return -EINVAL;
    return rt_system_seed48_nonblocking(buf);
}

int rt_rng90c_init(rt_rng90c* r) {
    memset(r, 0, sizeof(*r));
    rt_entropy_90b_init(&r->ent);

    uint8_t seed[48];
    if (rt_entropy_90b_get(&r->ent, rt_linux_entropy_source, NULL, seed, 48) != 0)
        return -EAGAIN;
    int rc = rt_ctr_drbg_instantiate_no_df(&r->drbg, seed);
    memset(seed, 0, sizeof seed);
    if (rc) return rc;
    r->gen_counter = 0;
    r->reseed_interval = 1ULL << 20; // example: 1M generates
    return 0;
}

int rt_rng90c_reseed(rt_rng90c* r) {
    uint8_t seed[48];
    if (rt_entropy_90b_get(&r->ent, rt_linux_entropy_source, NULL, seed, 48) != 0)
        return -EAGAIN;
    int rc = rt_ctr_drbg_reseed_no_df(&r->drbg, seed);
    memset(seed, 0, sizeof seed);
    if (rc) return rc;
    r->gen_counter = 0;
    return 0;
}

int rt_rng90c_generate(rt_rng90c* r, uint8_t* out, size_t len, int prediction_resistance) {
    if (prediction_resistance) {
        int rc = rt_rng90c_reseed(r);
        if (rc) return rc;
    } else if (r->gen_counter >= r->reseed_interval) {
        int rc = rt_rng90c_reseed(r);
        if (rc) return rc;
    }
    int rc = rt_ctr_drbg_generate_all(&r->drbg, out, len);
    if (rc) return rc;
    r->gen_counter++;
    return 0;
}

void rt_rng90c_uninit(rt_rng90c* r) {
    rt_ctr_drbg_uninstantiate(&r->drbg);
    memset(r, 0, sizeof(*r));
}
