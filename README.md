# HackiOS

**HackiOS** boots Apple's open-source **XNU kernel** (`xnu-8796.121.2`, the kernel of iOS 16.5) on Android phone
hardware and runs the **unmodified iOS 16.5 user space** on top of it: launchd → backboardd → SpringBoard →
apps. Our own EFI loader, our own replacements for Apple's closed drivers, a UEFI port, and a Linux Wi-Fi driver
on a compatibility layer make that possible.

It runs in two places:

- **POCO X5 Pro 5G (redwood, Snapdragon 778G)** — nothing is flashed: the UEFI is started with `fastboot boot`,
  the whole system lives in RAM, and after a reboot the phone is back in Android;
- **QEMU** (`virt`, aarch64) — the same thing on any Linux computer, no phone needed.

> **No Apple files are in this repository.** `system/ios/fetch.sh` downloads the iOS 16.5 IPSW from Apple's own CDN
> to *your* machine and extracts what is needed there. HackiOS is an independent research project by OOSEX GANG and
> is not affiliated with, authorised or endorsed by Apple Inc. iOS, iPhone and Apple are trademarks of Apple Inc.
> Read [DISCLAIMER.md](DISCLAIMER.md) before building an image.

<p>
<img src="docs/lockscreen.jpg" width="190" alt="Lock screen">
<img src="docs/homescreen.jpg" width="190" alt="Home screen">
<img src="docs/control-center.jpg" width="190" alt="Control Center">
<img src="docs/settings.jpg" width="190" alt="Settings">
</p>

Photographed on the phone, the Adreno GPU drawing (`gpu=1`). More in [Screenshots](#screenshots).

## What works

| Works | Does not work (yet) |
|---|---|
| Lock screen, swipe to unlock, home screen with all icons (two pages), stock iOS 16 wallpapers | Cellular, Bluetooth |
| System gestures: swipe up (home, app switcher), edge swipe (back), Control Center | Fast charging (PD/PPS); the charger is read, not yet driven |
| Apps: Settings, Clock, Calendar, Photos, Notes, Safari and more; widgets (Weather, Calendar) on the phone | Metal for apps (in progress: 293 of 456 system shaders already run on the Adreno, see `docs/air.md`) |
| Phone touchscreen (Goodix GT9916), multitouch, wake on touch; real power and volume keys | App Store content and Apple ID sign-in (no Secure Enclave; FairPlay daemon exits) |
| Up to 8 CPU cores (`cpus=8`), big core at 2.4 GHz, ~20 s to the lock screen | Video and Live Photo recording |
| Adreno 642L GPU through Mesa/freedreno (`gpu=1`, OpenGL ES for CoreAnimation) | |
| Wi-Fi (WCN6750, ath11k): join from Settings, Safari over Wi-Fi, 55–90 Mbit/s | |
| Sound (speakers, main microphone), camera in Camera.app (front/rear, preview, focus, HEIC photos) | |
| Data that iOS writes survives a reboot (`var_store=1 ufs_write=1`); iOS root volume on the phone's flash | |
| Charging with `adsp=1`: the audio DSP's charger takes current from USB (seen: +0.4 A from a computer's port), battery health and charger type in `sysctl kern.hackios.charger` | |
| Frames follow the panel's own scan (`fb_scan=1`): 120 Hz, each frame lands whole in one refresh -- no tearing | |
| Sileo + Procursus (dpkg, apt), tweaks, SSH | |

It is a research bench, not a ROM: a session lasts a given number of seconds, then the kernel returns to the
loader and hands over its log.

## Quick start: QEMU, no kernel build

`prebuilts/` holds the kernel and our programs built from this repository; the loader is built by
`build/build.sh` (clang + lld-link). All you need besides is the system image, which is made on your machine
from Apple's IPSW.

Needs: Linux x86-64, ~12 GB free disk, 4 GB free RAM. Packages (Arch):

```sh
sudo pacman -S --needed git python 7zip lz4 gcc qemu-system-aarch64 qemu-ui-gtk edk2-aarch64
```

(Debian/Ubuntu: `git python3 7zip liblz4-dev gcc qemu-system-arm qemu-efi-aarch64`; 7-Zip must be 23 or newer — it
reads APFS.)

```sh
git clone https://github.com/oosexgang/hackios-xnu-kernel.git hackios
cd hackios
build/setup.sh               # checks the host for the tools the build needs
system/ios/fetch.sh          # downloads 7.4 GB from Apple's CDN and extracts what is needed (5-15 min)
build/build.sh qemu-virt     # builds the APFS images (and the kernel, if the toolchain is there)
build/run.sh qemu-virt       # a 20-minute session in a QEMU window
```

The lock screen appears after 2–3 minutes.

- **Mouse with the left button held = finger.** Unlock / leave an app: swipe up from the very bottom edge; back:
  swipe from the left edge.
- Keys: `H` home, `L` lock/power, `↑`/`↓` volume, `F11` (hold) five fingers.
- From a script: `host/qemu/touch.py swipe 0.5 0.985 0.5 0.3 250`, `host/qemu/touch.py tap 0.615 0.71`,
  `host/qemu/touch.py key home`; screenshot: `host/qemu/screendump.sh shot.png`.
- Right after SpringBoard appears the system is still registering apps: give it 30–40 s.
- Debian/Ubuntu UEFI firmware: `QEMU_EFI=/usr/share/AAVMF/AAVMF_CODE.fd QEMU_VARS=/usr/share/AAVMF/AAVMF_VARS.fd`.

## On the phone (POCO X5 Pro 5G)

You need a **POCO X5 Pro 5G (redwood)** with an unlocked bootloader, `adb` and `fastboot`, and the images from the
section above. Nothing is written to the phone's storage.

**The phone's own firmware** is not in the repository and cannot be: it is the vendor's and is taken from the very
phone it will run on, from its Android:

```sh
adb pull /vendor/firmware/a660_{gmu.bin,sqe.fw,zap.mdt,zap.b00,zap.b01,zap.b02} fw-gpu/      # GPU (gpu=1)
adb pull /vendor/firmware_mnt/image/wpss.{mdt,b02,b03,b04,b05,b06,b07} fw-wifi/              # Wi-Fi (wifi=1)
adb pull /vendor/firmware_mnt/image/adsp.{mdt,b02,...,b36} fw-audio/                         # sound (adsp=1)
adb pull /vendor/firmware/aw8697_haptic.bin fw-haptic/                                       # haptics
```

(Put the `fw-*` directories next to the workspace. Wi-Fi also needs `fw-wifi/mac` — the radio's address on one
line — and `fw-wifi/network` — network name and password. `system/build.sh` prints which firmware went in.)

```sh
build/build.sh redwood GL=1          # images + out/redwood-boot.img
build/run.sh redwood 900             # loader into RAM, system image into RAM, a 15-minute session
```

or step by step with `xl`:

```sh
host/xl enter            # Android -> fastboot -> our loader in RAM (fastboot boot); it brings up its own USB link
host/xl ramdisk          # upload the system image into the phone's RAM (2.5 GB, ~80 s), once per loader session
XL_SECS=900 XL_CMDLINE="touchscreen=1 cpufreq=1 bootcpu=7 cpus=8 gpu=1 wifi=1 adsp=1" host/xl run
host/xl screenshot shot.png
host/xl reboot           # back to Android
```

`host/xl` with no arguments lists every command (logs, screen and sound recording, camera frames, stop, state...).

## Install as a dual boot (no computer needed afterwards)

HackiOS can live on the phone next to Android: the UEFI with our loader in the spare A/B slot, the kernel and the
boot volume in darwin, a boot menu at power-on, sessions with no time limit. Android is never written, and fastboot
always brings it back. See [docs/INSTALL.md](docs/INSTALL.md).

| | |
|---|---|
| **Boots from the flash** | no `fastboot boot`, no computer, no RAM-only session: power on, menu, HackiOS |
| **Two versions installed** | updates go into the other slot; "HackiOS, previous version" is always in the menu |
| **Boot guard** | a version that does not come up 3 times is skipped by itself -- the previous one boots, then Android |
| **Verified boot, your key** | SHA-256 of every bundle; with an owner key pinned, Ed25519-signed bundles only |
| **A/B switch in the loader** | back to Android from the menu: backup GPT first, CRCs, both tables read back |
| **Device identity checks** | nothing is written to a phone that is not the profile's `CODENAMES` |
| **Tested on every push** | eight QEMU scenarios of the boot menu and the guard, crypto checked against Python |

```sh
build/hackios detect                # which profile is the phone on adb? (writes nothing)
build/hackios doctor redwood        # every precondition, checked (writes nothing)
build/hackios install redwood
build/hackios switch redwood hackios
build/hackios health redwood        # the boot guard's view
build/hackios test                  # the whole dual boot in QEMU, no phone
```

## Prebuilt boot image

Every release has `hackios-redwood-boot.img` attached: the redwood UEFI with our loader inside, ready for
`fastboot boot`. It contains no Apple files; the system image is still made on your machine (above).

## Building from source

`build/build.sh <device>` runs every step that this machine can run and uses `prebuilts/` for the rest.

- **Kernel** — needs [theos](https://theos.dev/docs/installation-linux) in `~/theos` (cctools and
  `iPhoneOS16.5.sdk`) and `clang lld python perl`. Details: [kernel/xnu/README.md](kernel/xnu/README.md).
  ```sh
  kernel/xnu/apply.sh && kernel/deps/setup.sh && kernel/build.sh -j8 -k
  ```
- **Loader** — `bootloader/loader/build.sh` (clang + lld-link, an EFI application without EDK2).
- **Images** — `REST=1 system/build.sh` (variables documented at the top of that file).
- **UEFI** — [bootloader/uefi/README.md](bootloader/uefi/README.md) (Project Silicium port); not needed to run, `prebuilts/uefi` has it.
- After rebuilding: `host/update-prebuilt.sh` refreshes `prebuilts/`.

## Porting to another device

```sh
build/new-device.sh <codename> path/to/device.dtb   # profile + loader platform from the phone's device tree
```

Then follow [docs/PORTING.md](docs/PORTING.md): the UEFI port, the loader's platform description, the kernel's
drivers, the firmware. Device profiles live in `device/<vendor>/<device>/<name>.conf` (`xiaomi/redwood`, `qemu/virt`); `device/_template.conf`
documents every variable.

## How it works

```
fastboot boot ──► UEFI (Mu-Silicium) ──► xnuloader (EFI application)
                                           │  stays resident: USB link (fastboot protocol),
                                           │  watchdog, receives the kernel's log
                                           ▼
            XNU (BCM2837 config as the base + our platform)
              ├─ GICv3, timer, video console, return to the loader
              ├─ APFS (images in RAM, iOS volume on flash), ramfs (/private/var, /tmp)
              ├─ minicrypto instead of corecrypto, AMFI and Sandbox stand-ins (MAC policies)
              ├─ driver replacements: IOSurface, IOMobileFramebuffer, AppleKeyStore, network stack
              ├─ input: /dev/hk-touch ◄─ Goodix GT9916 (phone) / virtio-input (QEMU)
              ├─ GPU: Adreno 642L, Wi-Fi: WCN6750 (ath11k via LinuxKPI), sound: ADSP / virtio-sound
              ▼
   iOS 16.5 launchd → logd, notifyd, cfprefsd, runningboardd, backboardd, SpringBoard, apps
              ├─ our libraries are injected into backboardd, SpringBoard and mediaserverd
              │  (virtual HID services, a few ObjC method swaps — in memory only)
              └─ mediaserverd gets our CoreAudio output device (HKAudio.driver)
```

The iOS programs, frameworks and dyld shared cache are not changed by a single byte: what they miss is done by the
kernel (driver replacements) or by a library the kernel injects with `DYLD_INSERT_LIBRARIES` (`kern.exec_inject`).

## Layout

One repository, laid out by what each part is for:

| Path | What |
|---|---|
| `build/` | `build.sh` / `run.sh` (per device), `hackios` (every command behind one name), `setup.sh` (host check), `new-device.sh`, `lib.sh`, `flash/` (the dual-boot install, boot bundles, the owner key, the QEMU test of all of it) |
| `device/` | device profiles, `device/<vendor>/<device>/<name>.conf`: `xiaomi/redwood/`, `qemu/virt/`; `_template.conf` documents the variables |
| `kernel/xnu/` | our XNU changes: patch, new files, `apply.sh` / `export.sh` |
| `kernel/deps/` | everything for building XNU on Linux or macOS: `setup.sh`, tool wrappers, stubs |
| `kernel/build.sh`, `kernel/env.sh` | kernel build |
| `kernel/wifi/lx/` | the layer on which Linux's Wi-Fi driver builds for our kernel |
| `bootloader/loader/` | the loader: `loader.c` (UEFI application), `mkpayload.py` (kernel + images + arguments container) |
| `bootloader/uefi/`, `bootloader/mkbootimg.py` | the UEFI port and packing the boot image for `fastboot boot` |
| `system/` | our user space: `build.sh` (the APFS images), `src/` (init, injected libraries, the audio and Metal drivers, tests), `libmini/`, `rc.*` job lists, `var/`; see [system/README.md](system/README.md) |
| `system/ios/` | `fetch.sh` takes iOS 16.5 out of Apple's IPSW on your machine, `select-system.py` chooses what goes into the image |
| `graphics/` | Mesa with our patches (`mk-mesa.sh`, `mesa/`), the AIR frontend's checks (`air/`), Metal analysis (`metal/`, `metallib.py`) |
| `host/xl` | drives the phone over USB: loader, kernel runs, logs, screenshots, screen and sound recording |
| `host/qemu/` | QEMU runs, touch input, screenshots, stack symbolication |
| `host/` | APFS on the computer (`apfs/`), recording (`recording.py`, `sound.py`), UFS/flash tools, `mk-ath11k.sh`, `mk-jb.sh`, `new-workspace.sh`, host tests |
| `prebuilts/` | built kernel, UEFI pieces and our programs (`NOTICES.md` for the third-party code in the kernel) |
| `docs/` | screenshots, [checks.md](docs/checks.md), [PORTING.md](docs/PORTING.md), [INSTALL.md](docs/INSTALL.md), [air.md](docs/air.md) |

## Debugging

- Session log: QEMU's stdout or `logs/HHMMSS-run.log` from the phone.
- `RC_JOBS=rc.jobs.tour REST=1 system/build.sh` — an image that unlocks and opens every app of the first page.
- `userabort:` in the log is the stack of a process that called `abort()`; `usertrap:` (with `systrace=1`) — crashes.
- Live stacks (QEMU): `host/qemu/touch.py stacks NAME`, then `host/qemu/symstack.py LOG NAME`
  (needs [`ipsw`](https://github.com/blacktop/ipsw)).
- How a check can lie to you: [docs/checks.md](docs/checks.md).

## Screenshots

From the phone's screen, one session (`GL=1 REST=1`, `gpu=1`):

<p>
<img src="docs/homescreen-2.jpg" width="160" alt="Home screen, page 2">
<img src="docs/search.jpg" width="160" alt="Search">
<img src="docs/settings-general.jpg" width="160" alt="Settings > General">
<img src="docs/calendar.jpg" width="160" alt="Calendar">
<img src="docs/clock.jpg" width="160" alt="Clock">
</p>
<p>
<img src="docs/wallet.jpg" width="160" alt="Wallet">
<img src="docs/messages.jpg" width="160" alt="Messages">
<img src="docs/music.jpg" width="160" alt="Music">
<img src="docs/maps.jpg" width="160" alt="Maps">
<img src="docs/appstore-tabs.png" width="160" alt="App Store">
</p>

## Licenses

- No iOS files are in the repository: `system/ios/fetch.sh` downloads them from Apple's CDN to your machine.
- `kernel/xnu/` is a derivative work of XNU and is distributed under APSL 2.0, like XNU itself; `prebuilts/mach.*` is the
  kernel built from it. It also contains ath11k (BSD-3-Clause-Clear) and FreeBSD LinuxKPI code (BSD-2-Clause):
  see [prebuilts/NOTICES.md](prebuilts/NOTICES.md).
- `kernel/xnu/files/bsd/miscfs/apfs/libzbitmap/` and `kernel/deps/build/lzbitmap/` — libzbitmap (Corellium LLC, MIT here);
  `kernel/xnu/files/bsd/miscfs/apfs/libdeflate/` — libdeflate 1.24 (Eric Biggers, MIT, `COPYING` there).
- ath11k, Linux and FreeBSD sources are not in the repository: `host/mk-ath11k.sh` fetches pinned releases.
  `kernel/wifi/lx/patches/*.patch` are our changes to FreeBSD headers; the rest of `kernel/wifi/lx/` is ours.
- `bootloader/uefi/files/Binaries/redwood` and `prebuilts/uefi/SILICIUM_UEFI.fd` contain UEFI drivers from the phone's
  Xiaomi firmware, distributed the way Project Silicium distributes them for other devices;
  `prebuilts/uefi/mkbootimg.py` — AOSP, Apache-2.0.
- This repository is licensed under the [Apple Public Source License 2.0](LICENSE): our changes to XNU are a derivative of XNU and must be, and our other code here (loader, scripts) is released under the same license. Third-party code keeps its own license, as listed above.

## Team

OOSEX GANG — [@oodze](https://t.me/oodze) (founder), [@netusd](https://t.me/netusd), [@wannqn](https://t.me/wannqn),
[@hxfuxyyXD](https://t.me/hxfuxyyXD). News: [t.me/oosexgangdevs](https://t.me/oosexgangdevs).
