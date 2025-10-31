format ELF64

section '.text' executable

public fasm_randomtoad_u128
extrn rt_ctr_drbg_generate_u128

fasm_randomtoad_u128:
        push rbp
        mov  rbp, rsp
        ; expects: rdi = ctx*, rsi = out[2]*
        call rt_ctr_drbg_generate_u128
        leave
        ret
