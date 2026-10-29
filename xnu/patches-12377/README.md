# patches/xnu-12377

The series that turns Apple's **xnu-12377.121.6** (the iOS 26.5 kernel) into a `DEVELOPMENT
BCM2837` kernel — the machine config for a non-Apple ARMv8 SoC, and source for the machine layer
that Apple only ships as closed `lib<SoC>.os.<config>.a` archives in the Kernel Debug Kit.

Taken unmodified from **UniXNU** (`none24smu/UniXNU`, branch `ios26`, `patches/xnu-12377/`),
BSD-2-Clause-Patent, Copyright (c) 2026 UniXNU contributors. They are applied in name order by
`xnu/apply.sh` on this branch. Their result is a kernel that boots as far as `arm_init` on an
SM8350 (the Surface Duo 2 port); it carries no device drivers.

Exception: `zzz-linux-host.patch` is ours. It is the same host-portability fix `redwood.patch`
already carries for 8796 (Linux CI has no `sysctl` and no `/usr/bin/xcrun`), and it sorts last so
it lands on top of the series.

`config/MASTER.arm64.bcm2837` comes from this series; `config/MASTER` gets `CONFIG_APPLE_AMCC` and
the machine layer gets source replacements. Nothing here is redwood-specific — our own tree
(`redwood.patch` and `files/`, which live on the other branches) is the layer that goes on top.
