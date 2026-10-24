# Disclaimer

HackiOS is an independent, non-commercial research project by OOSEX GANG: running Apple's open-source XNU
kernel on Android phone hardware, and studying how the iOS user space behaves on it.

**Not affiliated with Apple.** HackiOS is not affiliated with, authorised, sponsored or endorsed by Apple Inc.
Apple, iPhone, iOS, Metal and the other names of Apple products are trademarks of Apple Inc., used here only to
say what the software works with.

## What these repositories contain

- Our own code: the changes to XNU, the drivers, the EFI loader, the UEFI port, the image builder, scripts and
  tools. The licences are listed in README.md ("Licenses") and in `prebuilt/NOTICES.md`.
- No iOS programs, libraries, frameworks, dyld shared cache, kernelcache or firmware images, and no data
  extracted from them. The loader describes the hardware to the kernel as what it is.
- No decryption keys, no signing keys or certificates of Apple, and no tool that removes copy protection from
  apps or content.

If you believe something in these repositories should not be here, open an issue or contact the maintainers
(README.md, "Team"): it will be looked at and, where it is so, removed.

## What you do on your own machine

The iOS files a HackiOS image is made of are **not distributed by us**. `ios165/fetch.sh` downloads Apple's own
firmware archive from Apple's servers (`updates.cdn-apple.com`) to your computer, and the image is assembled
there, for your own use.

Apple licenses iOS under its own Software License Agreement, which limits its use to Apple-branded devices.
Downloading iOS and running it on other hardware may not be permitted by that agreement or by the law where you
live. **Whether you may do so is your decision and your responsibility.** Do not use HackiOS if you are not
sure that you may.

Do not share images built with HackiOS: they contain Apple's files. Do not use HackiOS to obtain paid apps,
content or services without paying for them, or to get around activation, account or device locks.

The firmware of the phone's own hardware (GPU, Wi-Fi, audio and so on) belongs to its vendors; it is taken from
your phone and stays on your computer.

## No warranty

HackiOS is experimental software, provided "as is", without warranty of any kind, as set out in sections 7 and 8
of the Apple Public Source License 2.0 (LICENSE) and in the licences of the third-party code. It can lose data
and it can leave a phone unbootable; installing it is at your own risk.

This file is not legal advice.
