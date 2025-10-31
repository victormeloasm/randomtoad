# Makefile (trechos)

# Detecta fasm
NO_FASM ?= 0
HAS_FASM := $(shell command -v fasm >/dev/null 2>&1 && echo 1 || echo 0)
ifeq ($(NO_FASM),1)
  BUILD_FASM=0
else ifeq ($(HAS_FASM),1)
  BUILD_FASM=1
else
  BUILD_FASM=0
endif

# Flags default (AES sempre; RDSEED só se passado pelo CI/local)
CFLAGS ?= -O3 -Wall -Wextra -fPIC -maes
LDFLAGS ?= -fuse-ld=lld

OBJS = ctr_drbg.o seed_linux.o entropy_90b.o rng90c.o module_fips.o

# FASM opcional
ifeq ($(BUILD_FASM),1)
  FASM_OBJS = randomtoad_fasm.o
  DEMOS_ASM = demo_fasm_c demo_fasm_cpp
else
  $(info [make] FASM indisponível → pulando demos ASM)
  FASM_OBJS =
  DEMOS_ASM =
endif

LIB = librandomtoad.a
BIN = build/selftest build/test_randomtoad build/demo_cpp $(DEMOS_ASM) build/rng90c_demo build/stream_randomtoad

all: prepare_kat $(LIB) $(BIN)

$(LIB): $(OBJS)
	@mkdir -p build
	ar rcs $(LIB) $(OBJS)

# -------- prepare_kat resiliente ----------
prepare_kat:
	@mkdir -p build
	@if [ -f kat90a_vectors.h ]; then \
	  echo "[prepare_kat] kat90a_vectors.h encontrado — OK"; \
	else \
	  echo "[prepare_kat] Gerando kat90a_vectors.h..."; \
	  clang $(CFLAGS) -c regen_kat90a.c -o build/regen_kat90a.o || true; \
	  clang $(LDFLAGS) build/regen_kat90a.o -o build/regen_kat90a || true; \
	  if [ -x build/regen_kat90a ]; then ./build/regen_kat90a > kat90a_vectors.h; fi; \
	  if [ -f kat90a_vectors.h ]; then echo "[prepare_kat] OK"; else echo "[prepare_kat] Aviso: não gerou; use a versão commitada"; fi; \
	fi

# -------- FASM opcional ----------
ifeq ($(BUILD_FASM),1)
randomtoad_fasm.o: randomtoad_fasm.asm
	fasm $< $@

build/demo_fasm_c: demo_fasm_c.c randomtoad_fasm.o $(LIB)
	clang $(CFLAGS) $(LDFLAGS) demo_fasm_c.c randomtoad_fasm.o $(LIB) -o $@

build/demo_fasm_cpp: demo_fasm_cpp.cpp randomtoad_fasm.o $(LIB)
	clang++ -std=c++20 $(CFLAGS) $(LDFLAGS) demo_fasm_cpp.cpp randomtoad_fasm.o $(LIB) -o $@
endif

# ... (restante das regras: selftest, demo_cpp, etc.)
