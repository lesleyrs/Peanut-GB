NAME		:= Peanut-GB
DESCRIPTION	:= A DMG emulator using SDL2
COMPANY		:= Deltabeard
COPYRIGHT	:= Copyright (c) 2020 Mahyar Koshkouei
LICENSE_SPDX	:= MIT


CPPFLAGS := -DCOMPANY=Deltabeard		\
	-DDESCRIPTION="$(DESCRIPTION)"		\
	-DLICENSE="$(LICENSE_SPDX)"		\
	-DNAME="$(NAME)"			\
	-DICON_FILE=./meta/icon.ico		\
	-DENABLE_SOUND -DENABLE_SOUND_MINIGB -DMINIGB_APU_AUDIO_FORMAT_S16SYS

LIBC=../../../../wasmlite/libc
WCC = ../../../../xcc/wcc
CLANG = clang

DEBUG = 0
OUT = peanut.wasm wcc-peanut.wasm

ifeq ($(DEBUG),0)
CFLAGS = -Oz -ffast-math -flto #-Wall -Wextra
else
CFLAGS = -g -Wall -Wextra
endif

SOURCES := peanut_sdl.c minigb_apu/minigb_apu.c

peanut.wasm: CC = $(CLANG) --target=wasm32 --sysroot=$(LIBC) -Wl,--export-table -Wl,--export=malloc -nodefaultlibs -lc -Wl,--stack-first
wcc-peanut.wasm: CC = $(WCC) -isystem=$(LIBC)/include -L$(LIBC)/lib -Wl,--export-table -e=malloc --stack-size=64000 -Dgoto=void*

all: $(OUT) serve
$(OUT): $(SOURCES)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)
ifeq ($(DEBUG),0)
	wasm-opt $@ -o $@ -Oz -all && wasm-strip $@
else
	../../emscripten/tools/wasm-sourcemap.py $@ -w $@ -p $(CURDIR) -s -u ./$@.map -o $@.map --dwarfdump=/usr/bin/llvm-dwarfdump
endif

serve:
	esbuild --servedir=.
