// SPDX-License-Identifier: MIT
#include "ctr_drbg.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/random.h>
#include <immintrin.h>
#include <cpuid.h>

static int have_rdseed(void) {
    unsigned eax, ebx, ecx, edx;
    if (!__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) return 0;
    return (ebx & (1u<<18)) ? 1 : 0;
}

static long sys_getrandom(void *buf, size_t buflen, unsigned int flags) {
#ifdef SYS_getrandom
    return syscall(SYS_getrandom, buf, buflen, flags);
#else
    errno = ENOSYS; return -1;
#endif
}

static int rdseed_fill(uint8_t* out, size_t len) {
    unsigned long long prev = 0; int prev_valid = 0;
    size_t done = 0;
    while (done < len) {
        unsigned long long x;
        if (_rdseed64_step(&x)) {
            if (prev_valid && x == prev) continue;
            prev = x; prev_valid = 1;
            size_t to_copy = (len - done) < 8 ? (len - done) : 8;
            memcpy(out + done, &x, to_copy);
            done += to_copy;
        } else { asm volatile("pause"); }
    }
    return 0;
}

int rt_system_seed48_nonblocking(uint8_t out48[48]) {
    if (!rt_aesni_available()) return -ENOSYS;
    int used = 0;
    if (have_rdseed()) { rdseed_fill(out48, 48); used = 48; }
    if (used < 48) {
        ssize_t r = sys_getrandom(out48 + used, 48 - used, GRND_NONBLOCK);
        if (r < 0) {
            if (errno == EAGAIN && used == 0) return -EAGAIN;
        } else used += (int)r;
    }
    return (used >= 48) ? 0 : -EAGAIN;
}

int rt_ctr_drbg_instantiate_system(rt_ctr_drbg* ctx) {
    uint8_t seed[48];
    int rc = rt_system_seed48_nonblocking(seed);
    if (rc) return rc;
    rc = rt_ctr_drbg_instantiate_no_df(ctx, seed);
    volatile uint8_t* p = seed;
    for (size_t i = 0; i < sizeof(seed); i++) p[i] = 0;
    return rc;
}
