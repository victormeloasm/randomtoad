// SPDX-License-Identifier: MIT
// Minimal SP 800-90B-like health tests (illustrative; tune for formal 90B)

#ifndef RANDOMTOAD_ENTROPY_90B_H
#define RANDOMTOAD_ENTROPY_90B_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*rt_entropy_source_fn)(uint8_t* buf, size_t len, void* user);

typedef struct rt_entropy_90b_state {
    uint8_t last_block[16];
    int     last_valid;
    unsigned repetition_count;
    unsigned adaptive_buckets[256];
} rt_entropy_90b_state;

void rt_entropy_90b_init(rt_entropy_90b_state* st);
int  rt_entropy_90b_get(rt_entropy_90b_state* st,
                        rt_entropy_source_fn src, void* user,
                        uint8_t* out, size_t len);

#ifdef __cplusplus
}
#endif
#endif
