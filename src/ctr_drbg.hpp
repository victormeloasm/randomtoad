#pragma once
#include "ctr_drbg.h"
#include "module_fips.h"
#include <array>
#include <stdexcept>
#include <cstdint>
#include <string>

namespace randomtoad {
struct Drbg {
    rt_ctr_drbg ctx{};
    Drbg() {
        int rc = rt_module_init();
        if (rc != 0) throw std::runtime_error("module init failed");
        rc = rt_ctr_drbg_instantiate_system(&ctx);
        if (rc != 0) throw std::runtime_error("rt_ctr_drbg_instantiate_system failed: " + std::to_string(rc));
    }
    ~Drbg() { rt_ctr_drbg_uninstantiate(&ctx); }
    std::array<uint64_t,2> u128() {
        std::array<uint64_t,2> v{};
        if (rt_ctr_drbg_generate_u128(&ctx, v.data()) != 0) throw std::runtime_error("generate failed");
        return v;
    }
};
}
