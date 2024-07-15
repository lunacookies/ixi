#!/bin/sh

set -e

clang-format -i code/**/*.c code/**/*.h

rm -rf build
mkdir build

clang \
	-g3 \
	-fsanitize=undefined \
	-fshort-enums \
	-std=c11 \
	-W \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wconversion \
	-Wimplicit-fallthrough \
	-Wmissing-prototypes \
	-Wshadow \
	-Wstrict-prototypes \
	-o build/ixi \
	-I code \
	code/ixi/ixi_entry_point.c
