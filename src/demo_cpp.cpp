#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <cstdint>
#include "ctr_drbg.hpp"

static std::string u128_hex(const std::array<uint64_t,2>& v) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setfill('0')
        << std::setw(16) << v[1]
        << std::setw(16) << v[0];
    return oss.str();
}

static std::string u128_dec(const std::array<uint64_t,2>& v) {
#if defined(__SIZEOF_INT128__)
    __uint128_t x = ( (__uint128_t)v[1] << 64 ) | v[0];
    if (x == 0) return "0";
    char buf[64]; int idx = 63; buf[idx] = '\0';
    while (x > 0 && idx > 0) { unsigned d = (unsigned)(x % 10); x /= 10; buf[--idx] = char('0'+d); }
    return std::string(&buf[idx]);
#else
    std::ostringstream oss; oss << "[hi=" << std::dec << v[1] << ", lo=" << v[0] << "]"; return oss.str();
#endif
}

int main() {
    try {
        randomtoad::Drbg rng;
        for (int i = 0; i < 3; ++i) {
            auto v = rng.u128();
            std::cout << "u128[" << i << "] HEX=" << u128_hex(v)
                      << "  DEC=" << u128_dec(v) << "\n";
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n"; return 1;
    }
}
