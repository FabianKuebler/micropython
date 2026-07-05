# MicroPython on real-mode x86 (gcc-ia16): experiment notes

Follow-up to micropython/micropython#19429 — building on @agatti's diff.
Toolchain: gcc-ia16 6.3.0 (tkchia PPA, 2026-06-12 build), target MS-DOS,
`-march=i286`, VARIANT=minimal. (Findings measured on a v1.28.0 tree; this branch carries the same changes rebased onto master.)

## What works

- Whole tree COMPILES for ia16 small model (`-march=i286`) with the
  changes in this branch (agatti's fixes + termios/signal/VFS guards
  under `__MSDOS__`, newlib `stat()`-based `mp_import_stat`).
- agatti's two link errors are resolved: `_start` comes from the medium
  crt0 (`lib/medium/dos-m-c0.o`, picked automatically when LINKING with
  `-mcmodel=medium` instead of passing `-T dos-mml.ld` by hand), and the
  `csegvma` overflow is the small/medium mismatch: the tree was COMPILED
  small-model (one near `.text`) but linked against the medium-model
  script's 64 KiB code regions.

## The actual wall

- Small model: total MicroPython `.text` for VARIANT=minimal at -Os is
  **96 KiB** (before libc) — 1.5x the 64 KiB near-code limit. Link dies
  in relocation overflows. No linker script can fix near calls.
- Medium model: function pointers become 4-byte far pointers while
  `void *` stays 2-byte near — and MicroPython's object model stores
  function pointers in `void *` slots (`MP_DEFINE_CONST_OBJ_TYPE`,
  `MP_DEFINE_CONST_FUN_OBJ_*`). The build fails with **168
  "initialization from pointer to non-enclosed address space" errors**
  across py/obj*.c.
- No cast rescues it (see probe.c): a far fn ptr doesn't fit a near
  `void *`, and near-data-to-far-fnptr casts are not link-time constants.

## Conclusion

A useful ia16/DOS test build needs one of:
1. an upstream slot-type abstraction so ports can define a slot type
   wider than `void *` (invasive: every slot accessor), or
2. text < 64 KiB (gutting the build far below tests/basics usefulness).

The 8-bit-CPU/16-bit-ABI routes without segmented function pointers
(AVR, MSP430 small-ish, or the existing pic16bit port under XC16's
sim30) don't hit this wall: there `sizeof(void *) == sizeof(void (*)())`.
