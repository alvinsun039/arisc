# A733 ARISC firmware (E902)

This branch adds the Allwinner A733 (sun60iw2p1) fixes to the vendor
`arisc` snapshot `0170020` (Tina 1.5.0).  The E902 is the RISC-V core
that runs the always-on firmware: console, mailbox transport, clock and
power-domain hand-off for the AP.

## Changes

- Bound every UART FIFO and mailbox wait, drain the FIFO with a deadline
  and drop a broken frame instead of spinning; a clock change while the
  console was printing no longer stalls the S-domain switch.
- Move S_UART0 to PL6/PL7 (the A7A routes it to the 40-pin header) and
  run it at the real 115200 baud.
- Notify the TWI/UART clock-change handlers before the source clocks
  switch, so their request lines leave at the correct rate.
- Program the CPU boot address after the core domain is powered (RVBAR
  loses its value while the domain is off) and keep the standby task the
  only mailbox consumer while it waits for the resume notify.
- Drop the per-frame mailbox query logs.

## Build

The E902 uses T-HEAD CSRs (`mtvt`/`mtve`) that stock GCC rejects, and
the ARISC OEM library is a vendor build input.  Both are external:

```sh
git clone https://github.com/radxa/allwinner-dramlib.git ../allwinner-dramlib
git -C ../allwinner-dramlib checkout b1c31a9      # v1.4.10

ARISC_TOOLS=/path/to/t-head/bin ./build.sh
```

`ARISC_TOOLS` points at a T-HEAD XuanTie V1.10.2 (GCC 8.4.0) toolchain.
`LICHEE_DRAMLIB_PATH` can point at the dramlib checkout explicitly; the
script also finds a sibling `../allwinner-dramlib`.

Output: `ar100s/scp.bin`.

On hosts where the vendor's x86_64 `scripts/conf` cannot run (for
example an aarch64 workstation), `build.sh` falls back to the
checked-in `configs/` files for the fixed A733 configuration.

## CI

`.github/workflows/build.yml` builds `ar100s/scp.bin` on every push,
pull request and manual dispatch, and uploads it as the
`arisc-a733-scp` artifact.

It needs the T-HEAD XuanTie V1.10.2 (GCC 8.4.0) toolchain.  Set the
repository variable `ARISC_TOOLS_URL` (Settings -> Secrets and
variables -> Actions -> Variables) to a downloadable tarball of that
toolchain - the same `riscv64-elf-x86_64-20201104.tar.gz` used for
bring-up.  The job also checks out `radxa/allwinner-dramlib` at
`b1c31a9` for `arisc_liboem/libar100s.a`.  Without the variable the
build stops with a clear error; the local build takes the same inputs
through `ARISC_TOOLS` and `LICHEE_DRAMLIB_PATH`.

## Base and licence

This is a branch of the public `tina5.0_aiot/lichee/arisc` snapshot
`0170020`.  The vendor files keep their original copyrights and licence
terms (per-file SPDX tags include BSD-3-Clause and GPL-2.0/GPL-2.0+).
The A733 changes and the new files (`build.sh`, `README.md`, `configs/`,
`LICENSE`, `NOTICE`) are BSD-3-Clause.  See `NOTICE` for the build
inputs and the exact scope.
