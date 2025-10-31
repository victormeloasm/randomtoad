# SP 800‑90B Notes (NIST‑like)

Inclui **health tests ilustrativos**:
- **Repetition Count Test (RCT)**: granularidade de 16 bytes, limiar=32.
- **Adaptive Proportion Test (APT)**: bucket grosseiro no primeiro byte, limiar=64.

> Para 90B formal, ajuste janelas/limiares com base no **seu** coletor real, documente e estime **min‑entropy**. Para **RDSEED**, cite docs do fabricante; para `getrandom(GRND_NONBLOCK)`, relatar testes/garantias do SO.
