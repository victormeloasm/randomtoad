// SPDX-License-Identifier: MIT
#include "entropy_90b.h"
#include <string.h>

void rt_entropy_90b_init(rt_entropy_90b_state* st) {
    memset(st, 0, sizeof(*st));
}

#define RT_REP_MAX 32
#define RT_ADAPT_MAX 64

int rt_entropy_90b_get(rt_entropy_90b_state* st,
                       rt_entropy_source_fn src,
                       void* user,
                       uint8_t* out,
                       size_t len)
{
    int rc = src(out, len, user);
    if (rc) return rc;

    if (len >= 16) {
        if (st->last_valid && memcmp(st->last_block, out, 16) == 0) {
            st->repetition_count++;
            if (st->repetition_count > RT_REP_MAX) return -1;
        } else {
            memcpy(st->last_block, out, 16);
            st->last_valid = 1;
            st->repetition_count = 0;
        }
    }
    st->adaptive_buckets[out[0]]++;
    if (st->adaptive_buckets[out[0]] > RT_ADAPT_MAX) return -1;
    return 0;
}
