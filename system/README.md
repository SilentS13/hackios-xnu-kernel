# HackiOS — userland

The user-space part of **HackiOS**: Apple's open-source XNU kernel on Android phone hardware (POCO X5 Pro 5G) and in
QEMU, running the unmodified iOS 16.5 user space. The kernel, loader, UEFI and the `build/` scripts are in the
directories next to this one — **start with the [README at the top](../README.md)**.

> **No Apple files are in this repository.** `system/ios/fetch.sh` downloads the iOS 16.5 IPSW from Apple's CDN to your
> machine and extracts what is needed there. HackiOS is an independent research project by OOSEX GANG and is not
> affiliated with, authorised or endorsed by Apple Inc.

## What is here

| Path | What |
|---|---|
| `system/build.sh` | builds the APFS images: `root.apfs` (our init, libraries, launchd jobs, firmware) and `system.apfs` (the selected part of iOS 16.5 and its cryptexes); every option is documented at the top |
| `system/src/` | `init.c` (process 1), injected libraries (`inject.c` backboardd, `inject_sb.c` SpringBoard, `inject_media.c` mediaserverd, `inject_tweaks.c`), the CoreAudio output device (`audio_plugin.c`), the OpenGL ES bridge to Mesa (`eagl.c`), Metal driver work, tests |
| `system/libmini/` | a tiny libc for the static programs |
| `system/rc.*`, `com.redwood.*.plist` | job lists and launchd jobs for test scenarios (`RC_JOBS=rc.jobs.tour`, `rc.jobs.ssh`, ...) |
| `system/mkjobs.py`, `jobinfo.py` | the stock iOS launchd jobs: which are loaded, who provides a Mach service |
| `system/var/` | our default preferences for `/private/var` |
| `system/ios/` | `fetch.sh` / `fetch-from-ipsw.py` (take iOS out of the IPSW), `select-system.py` (what goes into the image) |
| `host/apfs/` | APFS on the computer: `mkapfs.py` (image writer), reader, checks, write tests |
| `graphics/mk-mesa.sh`, `graphics/mesa/` | Mesa 25.0.7 with our patches: freedreno on Darwin, the AIR (Metal IR) frontend |
| `graphics/air/`, `graphics/metal/`, `graphics/metallib.py` | Metal shader (AIR) checks and analysis; notes in `docs/air.md` |
| `host/mk-jb.sh`, `jb-preinstall.py` | Procursus bootstrap + Sileo for the package manager prefix |
| `host/font_trim.py` | trims two oversized iOS fonts so the image fits |
| `prebuilts/userland/` | our programs and libraries, built — used when there is no cross toolchain |

## Building the images

```sh
system/ios/fetch.sh               # once
REST=1 system/build.sh      # -> system/root.apfs, system/system.apfs
GL=1 REST=1 system/build.sh # with OpenGL ES (Mesa) for the window server; on the phone with gpu=1
```

Without the cross toolchain (theos + iPhoneOS16.5.sdk, see [kernel/xnu/README.md](../kernel/xnu/README.md)) the build takes our programs from
`prebuilts/userland`; with it, it compiles `system/src`.

A job list (`RC_JOBS=file`) is run by our `init`, not by a shell: iOS 16.5's root has no shell, lines are split on
blanks, there is no quoting and no variable assignment; only `>> FILE` and a trailing `&` are understood.

## Licenses

Our code in this directory is released under the [MIT License](../LICENSE). Mesa (MIT) is not in the
repository: `graphics/mk-mesa.sh` fetches release 25.0.7, `graphics/mesa/*.patch` are our changes to it. apfsprogs
(GPL-2.0) is not in the repository either: `host/apfs/check-image.sh` fetches and builds only `apfsck`.

OOSEX GANG — [t.me/oosexgangdevs](https://t.me/oosexgangdevs)
