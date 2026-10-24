#!/bin/bash
# Check the host for the tools a HackiOS workspace needs. Nothing is fetched or written: what is
# missing only matters for the step that needs it.
#
# usage: build/setup.sh
set -e
cd "$(dirname "$(readlink -f "$0")")/.."
git log -1 --format='setup: hackios at %h (%cs) %s' 2>/dev/null || true

need() { # need <command> <what for>
	command -v "$1" >/dev/null 2>&1 && return 0
	printf '  %-22s missing -- needed %s\n' "$1" "$2"; MISSING=1
}
echo "setup: checking the host"
MISSING=0
need python3 "everywhere"
need git "everywhere"
need curl "to fetch iOS, XNU and dependencies"
need 7z "by system/ios/fetch.sh (7-Zip 23+, reads APFS)"
need gcc "by system/ios/fetch.sh (builds the LZBITMAP decoder)"
need qemu-system-aarch64 "to run in QEMU (DEVICE=qemu-virt)"
need fastboot "to boot a phone"
need adb "to take a phone's own firmware off it"
need clang "to build the kernel and the loader (not needed with prebuilts/)"
need lld-link "to build the loader (not needed with prebuilts/)"
[ -d "${THEOS:-$HOME/theos}/sdks/iPhoneOS16.5.sdk" ] ||
	echo "  theos + iPhoneOS16.5.sdk missing -- needed only to rebuild the kernel and our programs (prebuilts/ has them)"
[ $MISSING = 0 ] && echo "  all there"
cat <<EOT

Next:
  system/ios/fetch.sh               # once: takes iOS 16.5 out of Apple's IPSW onto this machine (7.4 GB download)
  build/build.sh qemu-virt          # images (+ kernel and loader when the toolchain is there)
  build/run.sh qemu-virt            # iOS in a QEMU window
See README.md; porting to another device: docs/PORTING.md and build/new-device.sh.
EOT
