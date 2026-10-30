# patches/xnu-12377

The series that turns Apple's **xnu-12377.121.6** (the iOS 26.5 kernel) into a `DEVELOPMENT
BCM2837` kernel — the machine config for a non-Apple ARMv8 SoC, and source for the machine layer
that Apple only ships as closed `lib<SoC>.os.<config>.a` archives in the Kernel Debug Kit.

Taken unmodified from **UniXNU** (`none24smu/UniXNU`, branch `ios26`, `patches/xnu-12377/`),
BSD-2-Clause-Patent, Copyright (c) 2026 UniXNU contributors. They are applied in name order by
`xnu/apply.sh` on this branch. Their result is a kernel that boots as far as `arm_init` on an
SM8350 (the Surface Duo 2 port); it carries no device drivers.

Exceptions, all ours and sorted last so they land on top of the series:

- `zzz-linux-host.patch` — the same host-portability fix `redwood.patch` already carries for 8796
  (Linux CI has no `sysctl` and no `/usr/bin/xcrun`).
- `zzzz-redwood-device-layer.patch` — the vili (SM8350) device layer edits; the sources it adds
  arrive as whole files in `xnu/files/`.
- `zzzz-sweet-arm64e-exec.patch` — accept an **arm64e** slice on this PAuth-less host when
  boot-arg `sweetarm64e=1` is set: `ml_grade_binary()` grades it below a real arm64 slice, and exec
  puts `ptrauth_disabled=1` in the `apple[]` vector so dyld takes its no-PAC paths. The code itself
  is rewritten offline by `tools/renegade/pacstrip.py` / `pacstrip_cache.py`. The AMFI stand-in
  (`xnu/files/bsd/kern/kern_minimac.c`) answers dyld's policy query with every `ALLOW_*` bit.
- `zzzz-sweet-fjcvtzs-emulation.patch` — trap emulation of `FJCVTZS` (`handle_uncategorized`) for
  EL0 threads, gated on boot-arg `sweetarm64e=1`; this CPU has no ARMv8.3-JSCVT and the iOS 26
  JavaScriptCore/WebCore use the instruction unconditionally.
- `zzzzz-loader-log-console.patch` — keep the kernel's `printf` output in the loader log.

`config/MASTER.arm64.bcm2837` comes from this series; `config/MASTER` gets `CONFIG_APPLE_AMCC` and
the machine layer gets source replacements. Nothing here is redwood-specific — our own tree
(`redwood.patch` and `files/`, which live on the other branches) is the layer that goes on top.
