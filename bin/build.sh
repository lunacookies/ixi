#!/bin/sh

set -e

clang-format -i code/**/*.c code/**/*.h

rm -rf build
mkdir build

clang \
	-g3 \
	-fsanitize=undefined \
	-W \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wconversion \
	-Wimplicit-fallthrough \
	-Wmissing-prototypes \
	-Wshadow \
	-Wstrict-prototypes \
	-o build/arp \
	-I code \
	code/arp/arp_entry_point.c
