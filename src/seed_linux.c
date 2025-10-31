// seed_linux.c (trecho no topo)
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/random.h>
#include <cpuid.h>

static int cpu_has_rdseed(void) {
    unsigned int a, b, c, d;
    if (!__get_cpuid_max(0, NULL)) return 0;
    // CPUID.(EAX=7,ECX=0):EBX.RDSEED[bit 18]
    __cpuid_count(7, 0, a, b, c, d);
    return ((b >> 18) & 1);
}

#if defined(__RDSEED__)
#include <immintrin.h>
static int try_rdseed64_once(uint64_t *out) {
    return _rdseed64_step(out) ? 1 : 0;
}
#else
static int try_rdseed64_once(uint64_t *out) {
    (void)out; return 0; // sem -mrdseed: nunca tenta
}
#endif

static int fill_from_rdseed(uint8_t *dst, size_t len) {
    if (!cpu_has_rdseed()) return 0;
    size_t i = 0;
    uint64_t w;
    while (i < len) {
        if (!try_rdseed64_once(&w)) break;
        size_t take = (len - i) < 8 ? (len - i) : 8;
        memcpy(dst + i, &w, take);
        i += take;
    }
    return (int)i; // bytes preenchidos
}

int rt_sys_collect_seed(uint8_t *out48) {
    // 1) RDSEED (melhor caso)
    int got = fill_from_rdseed(out48, 48);
    if (got == 48) return 0;

    // 2) getrandom() non-blocking com backoff leve
    size_t off = (got > 0 && got < 48) ? (size_t)got : 0;
    unsigned tries = 0;
    while (off < 48 && tries < 8) {
        ssize_t r = getrandom(out48 + off, 48 - off, GRND_NONBLOCK);
        if (r > 0) off += (size_t)r;
        else if (errno != EAGAIN && errno != EINTR) break;
        tries++;
        // backoff (curtinho)
        usleep(1000u * tries);
    }
    return (off == 48) ? 0 : -1;
}
