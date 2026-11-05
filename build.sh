#!/usr/bin/env bash
# Copyright (c) 2026, A733 contributors
# SPDX-License-Identifier: BSD-3-Clause
#
# build.sh - build the A7A ARISC (E902/RISC-V) firmware scp.bin.
#
# The E902 is a T-HEAD XuanTie core; its crt0.S uses T-HEAD CSRs
# (mtvt/mtve) that stock GCC rejects.  Point ARISC_TOOLS at the bin
# directory of a T-HEAD XuanTie V1.10.2 (GCC 8.4.0) toolchain.
#
# The ARISC OEM library (arisc_liboem/libar100s.a) is not in this
# repository.  Point LICHEE_DRAMLIB_PATH at a radxa/allwinner-dramlib
# checkout (pin b1c31a9, v1.4.10), or place one next to this tree as
# ../allwinner-dramlib.
#
# Output: ar100s/scp.bin.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CORE_DIR="$SCRIPT_DIR/ar100s"
DEFCONFIG=sun60iw2p1_defconfig
PLATFORM=sun60iw2p1
JOBS="$(nproc)"

ARISC_TOOLS="${ARISC_TOOLS:-}"
if [ -z "$ARISC_TOOLS" ]; then
	for d in "$CORE_DIR/tools/bin" "$SCRIPT_DIR/../tools/bin"; do
		if [ -x "$d/riscv64-unknown-elf-gcc" ]; then
			ARISC_TOOLS="$d"
			break
		fi
	done
fi
if [ -z "$ARISC_TOOLS" ] || [ ! -x "$ARISC_TOOLS/riscv64-unknown-elf-gcc" ]; then
	cat >&2 <<'EOF'
ERROR: T-HEAD toolchain not found.
  Set ARISC_TOOLS to the bin directory of a T-HEAD XuanTie V1.10.2
  (GCC 8.4.0) toolchain.  Stock GCC does not accept the mtvt/mtve CSRs
  used by crt0.S.  The toolchain is not part of this repository.
EOF
	exit 1
fi

DRAMLIB="${LICHEE_DRAMLIB_PATH:-}"
if [ -z "$DRAMLIB" ]; then
	for d in "$SCRIPT_DIR/../allwinner-dramlib" "$SCRIPT_DIR/../dramlib"; do
		if [ -f "$d/$PLATFORM/arisc_liboem/libar100s.a" ]; then
			DRAMLIB="$d"
			break
		fi
	done
fi
if [ -z "$DRAMLIB" ] || [ ! -f "$DRAMLIB/$PLATFORM/arisc_liboem/libar100s.a" ]; then
	cat >&2 <<'EOF'
ERROR: ARISC OEM library not found (arisc_liboem/libar100s.a).
  git clone https://github.com/radxa/allwinner-dramlib.git ../allwinner-dramlib
  git -C ../allwinner-dramlib checkout b1c31a9
  ARISC_TOOLS=... LICHEE_DRAMLIB_PATH=$PWD/../allwinner-dramlib ./build.sh

The library is a build input and is not redistributed here.
EOF
	exit 1
fi
export LICHEE_DRAMLIB_PATH="$DRAMLIB"

CC="${CC:-$ARISC_TOOLS/riscv64-unknown-elf-gcc}"
LD="${LD:-$ARISC_TOOLS/riscv64-unknown-elf-ld}"
AR="${AR:-$ARISC_TOOLS/riscv64-unknown-elf-ar}"
OBJC="${OBJC:-$ARISC_TOOLS/riscv64-unknown-elf-objcopy}"
OBJCOPY="${OBJCOPY:-$OBJC}"
OBJDUMP="${OBJDUMP:-$ARISC_TOOLS/riscv64-unknown-elf-objdump}"
NM="${NM:-$ARISC_TOOLS/riscv64-unknown-elf-nm}"
SIZE="${SIZE:-$ARISC_TOOLS/riscv64-unknown-elf-size}"
STRIP="${STRIP:-$ARISC_TOOLS/riscv64-unknown-elf-strip}"
MARCH_LD_FLAGS="${MARCH_LD_FLAGS:--melf32lriscv}"
MARCH_FLAGS="${MARCH_FLAGS:--mcmodel=medany -mabi=ilp32e -march=rv32emc -fsingle-precision-constant -Wno-error=deprecated}"

echo "== arisc: toolchain $(basename "$CC") ($( "$CC" --version 2>&1 | head -1 ))"
if command -v sha256sum >/dev/null 2>&1; then
	echo "== arisc: liboem $(sha256sum "$DRAMLIB/$PLATFORM/arisc_liboem/libar100s.a" | awk '{print $1}')"
fi

if [ ! -f "$CORE_DIR/.config" ]; then
	if ! make -C "$CORE_DIR" "$DEFCONFIG"; then
		echo "== arisc: defconfig tool unavailable, using the checked-in config"
		rm -f "$CORE_DIR/.config"
		cp "$SCRIPT_DIR/configs/$PLATFORM.config" "$CORE_DIR/.config"
		mkdir -p "$CORE_DIR/include/config" "$CORE_DIR/include/generated"
		cp "$SCRIPT_DIR/configs/auto.conf" "$CORE_DIR/include/config/auto.conf"
		cp "$SCRIPT_DIR/configs/autoconf.h" "$CORE_DIR/include/generated/autoconf.h"
		touch "$CORE_DIR/.config" \
			"$CORE_DIR/include/config/auto.conf" \
			"$CORE_DIR/include/generated/autoconf.h"
	fi
fi

echo "== arisc: building scp ($JOBS jobs)"
make -C "$CORE_DIR" \
  CC="$CC" LD="$LD" AR="$AR" OBJC="$OBJC" OBJCOPY="$OBJCOPY" \
  OBJDUMP="$OBJDUMP" NM="$NM" SIZE="$SIZE" STRIP="$STRIP" \
  MARCH_LD_FLAGS="$MARCH_LD_FLAGS" \
  CFG_CHIP_PLATFORM="$PLATFORM" CFG_ARCH_PLATFORM=riscv CFG_RISCV=y \
  MARCH_FLAGS="$MARCH_FLAGS" \
  -j"$JOBS"

SCP="$CORE_DIR/scp.bin"
if [ ! -f "$SCP" ]; then
  echo "ERROR: scp.bin not produced: $SCP" >&2
  exit 1
fi
echo "== arisc: done -> $SCP ($(stat -c%s "$SCP") B)"
