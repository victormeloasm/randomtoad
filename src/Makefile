CC=clang
CXX=clang++
AR=ar
FASM=fasm

CFLAGS=-O3 -Wall -Wextra -march=native -maes -mrdseed -fPIC
CXXFLAGS=-O3 -std=c++20 -Wall -Wextra -march=native -maes -mrdseed
LDFLAGS=-fuse-ld=lld

BUILD?=build

SRC=ctr_drbg.c seed_linux.c entropy_90b.c rng90c.c module_fips.c
OBJ=$(SRC:.c=.o)
LIB=librandomtoad.a

all: $(BUILD) prepare_kat static test_c test_cpp test_fasm_c test_fasm_cpp rng90c_demo stream selftest

$(BUILD):
	mkdir -p $(BUILD)

static: $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)

# Auto-generate KAT header if placeholder is present
prepare_kat:
	@if grep -q "RT_KAT_PLACEHOLDER" kat90a_vectors.h 2>/dev/null; then \
		echo "[prepare_kat] Generating kat90a_vectors.h..."; \
		$(CC) $(CFLAGS) $(LDFLAGS) regen_kat90a.c ctr_drbg.c -o $(BUILD)/regen_kat90a; \
		./$(BUILD)/regen_kat90a > kat90a_vectors.h; \
	else \
		echo "[prepare_kat] kat90a_vectors.h present."; \
	fi

ctr_drbg.o: ctr_drbg.c ctr_drbg.h
	$(CC) $(CFLAGS) -c $< -o $@

seed_linux.o: seed_linux.c ctr_drbg.h
	$(CC) $(CFLAGS) -c $< -o $@

entropy_90b.o: entropy_90b.c entropy_90b.h
	$(CC) $(CFLAGS) -c $< -o $@

rng90c.o: rng90c.c rng90c.h ctr_drbg.h entropy_90b.h
	$(CC) $(CFLAGS) -c $< -o $@

module_fips.o: module_fips.c module_fips.h ctr_drbg.h kat90a_vectors.h
	$(CC) $(CFLAGS) -c $< -o $@

randomtoad_fasm.o: randomtoad_fasm.asm
	$(FASM) randomtoad_fasm.asm randomtoad_fasm.o

# demos/tests
test_c: $(BUILD) static test_randomtoad.c module_fips.h
	$(CC) -O3 $(LDFLAGS) test_randomtoad.c $(LIB) -o $(BUILD)/test_randomtoad

test_cpp: $(BUILD) static demo_cpp.cpp ctr_drbg.hpp module_fips.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) demo_cpp.cpp $(LIB) -o $(BUILD)/demo_cpp

test_fasm_c: $(BUILD) static demo_fasm_c.c randomtoad_fasm.o
	$(CC) -O3 $(LDFLAGS) demo_fasm_c.c randomtoad_fasm.o $(LIB) -o $(BUILD)/demo_fasm_c

test_fasm_cpp: $(BUILD) static demo_fasm_cpp.cpp randomtoad_fasm.o ctr_drbg.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) demo_fasm_cpp.cpp randomtoad_fasm.o $(LIB) -o $(BUILD)/demo_fasm_cpp

rng90c_demo: $(BUILD) static rng90c_demo.c
	$(CC) -O3 $(LDFLAGS) rng90c_demo.c $(LIB) -o $(BUILD)/rng90c_demo

stream: $(BUILD) static stream_randomtoad.c
	$(CC) -O3 $(LDFLAGS) stream_randomtoad.c $(LIB) -o $(BUILD)/stream_randomtoad

# FIPS-mode build variant
fips: CFLAGS+=-DFIPS_MODE
fips: CXXFLAGS+=-DFIPS_MODE
fips: clean all

# Strict AES KAT (enable once vector is agreed)
strict_kat: CFLAGS+=-DRT_STRICT_AES_KAT
strict_kat: CXXFLAGS+=-DRT_STRICT_AES_KAT
strict_kat: clean all

# Selftest binary
selftest: $(BUILD) static selftest.c
	$(CC) -O3 $(LDFLAGS) selftest.c $(LIB) -o $(BUILD)/selftest

# Manual KAT regeneration
regen_kat: $(BUILD) static regen_kat90a.c
	$(CC) -O3 $(LDFLAGS) regen_kat90a.c $(LIB) -o $(BUILD)/regen_kat90a
	./$(BUILD)/regen_kat90a > kat90a_vectors.h

clean:
	rm -f *.o $(LIB) randomtoad_fasm.o
	rm -rf $(BUILD)

PREFIX?=/usr/local
install: static
	install -Dm644 ctr_drbg.h       "$(DESTDIR)$(PREFIX)/include/randomtoad/ctr_drbg.h"
	install -Dm644 entropy_90b.h    "$(DESTDIR)$(PREFIX)/include/randomtoad/entropy_90b.h"
	install -Dm644 rng90c.h         "$(DESTDIR)$(PREFIX)/include/randomtoad/rng90c.h"
	install -Dm644 module_fips.h    "$(DESTDIR)$(PREFIX)/include/randomtoad/module_fips.h"
	install -Dm644 ctr_drbg.hpp     "$(DESTDIR)$(PREFIX)/include/randomtoad/ctr_drbg.hpp"
	install -Dm644 $(LIB)           "$(DESTDIR)$(PREFIX)/lib/$(LIB)"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/include/randomtoad/ctr_drbg.h"
	rm -f "$(DESTDIR)$(PREFIX)/include/randomtoad/entropy_90b.h"
	rm -f "$(DESTDIR)$(PREFIX)/include/randomtoad/rng90c.h"
	rm -f "$(DESTDIR)$(PREFIX)/include/randomtoad/module_fips.h"
	rm -f "$(DESTDIR)$(PREFIX)/include/randomtoad/ctr_drbg.hpp"
	rm -f "$(DESTDIR)$(PREFIX)/lib/$(LIB)"

.PHONY: all static clean install uninstall regen_kat fips selftest strict_kat prepare_kat
