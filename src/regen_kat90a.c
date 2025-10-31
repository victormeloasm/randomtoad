// Generates kat90a_vectors.h content for DRBG KAT (64-byte out)
#include "ctr_drbg.h"
#include <stdio.h>

int main(void){
    const unsigned char seed[48] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
        0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
        0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F
    };
    rt_ctr_drbg d;
    if (rt_ctr_drbg_instantiate_no_df(&d, seed) != 0) return 1;
    unsigned char out[64];
    if (rt_ctr_drbg_generate_all(&d, out, sizeof out) != 0) return 1;
    printf("// SPDX-License-Identifier: MIT\n");
    printf("#ifndef RANDOMTOAD_KAT90A_VECTORS_H\n#define RANDOMTOAD_KAT90A_VECTORS_H\n#include <stdint.h>\n");
    printf("static const uint8_t rt_kat_seed_0[48] = {");
    for(int i=0;i<48;i++){ printf("%s0x%02x", (i? ",": " "), seed[i]); }
    printf("};\n");
    printf("static const uint8_t rt_kat_out0[64] = {");
    for(int i=0;i<64;i++){ printf("%s0x%02x", (i? ",": " "), out[i]); }
    printf("};\n#endif\n");
    return 0;
}
