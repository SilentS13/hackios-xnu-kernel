#!/bin/bash
# Build everything a device needs, in order, skipping what cannot be built on this machine.
#
# usage: build/build.sh <device> [system/build.sh variables...]
#   build/build.sh qemu-virt
#   build/build.sh redwood GL=1
#
#   1. kernel   kernel/xnu/apply.sh, kernel/deps/setup.sh, kernel/build.sh   -- only with theos + iPhoneOS16.5.sdk,
#                                                                  otherwise prebuilts/ is used
#   2. loader   bootloader/loader/build.sh                              -- only with clang + lld-link
#   3. images   system/build.sh (REST=1 and the profile's USERLAND_VARS)
#                                                               -- needs system/ios/fetch.sh to have run
#   4. boot     bootloader/mkbootimg.py -> out/<device>-boot.img             -- phones: the UEFI image for `fastboot boot`
#
#   SKIP="kernel loader images boot"   leave steps out
. "$(dirname "$(readlink -f "$0")")/lib.sh"
load_device "$1"; shift
skip() { [[ " $SKIP " == *" $1 "* ]]; }
mkdir -p out

if ! skip kernel; then
	if [ -d "${THEOS:-$HOME/theos}/sdks/iPhoneOS16.5.sdk" ] && command -v clang >/dev/null; then
		[ -d xnu-src ] || kernel/xnu/apply.sh
		[ -x kernel/deps/host/bin/ld64 ] || kernel/deps/setup.sh
		# Wi-Fi radio driver (phones with a WCN6750): fetched and built once
		[ "$WIFI_ATH11K" = 1 ] && [ ! -f kernel/deps/build/ath11k/out/ath11k.o ] && { host/mk-ath11k.sh && host/mk-wlfw.sh; }
		# the export check fails after the kernel is linked, on purpose (kernel/xnu/README.md): judge by the file
		touch out/.kernel-start
		kernel/build.sh -j"$(nproc)" -k > out/kernel-build.log 2>&1 || true
		K=xnu-src/xnu-8796.121.2/BUILD/obj/DEVELOPMENT_ARM64_BCM2837/mach.development.bcm2837
		[ -f $K ] && { [ $K -nt out/.kernel-start ] || ! grep -q "error:" out/kernel-build.log; } &&
			echo "build: kernel $(stat -c %s $K) bytes" || die "kernel build failed, see out/kernel-build.log"
	else
		echo "build: no theos/iPhoneOS16.5.sdk: using prebuilts/mach.development.bcm2837"
	fi
fi

if ! skip loader; then
	if command -v clang >/dev/null && command -v lld-link >/dev/null; then
		bootloader/loader/build.sh && echo "build: loader bootloader/loader/loader.efi"
	else
		die "no clang/lld-link: the loader cannot be built (bootloader/loader/build.sh needs both)"
	fi
fi

if ! skip images; then
	have_userland
	[ -d system/ios/rootfs ] || die "no iOS files yet: run system/ios/fetch.sh first (it downloads them from Apple to this machine)"
	env REST=1 $USERLAND_VARS "$@" system/build.sh
fi

if ! skip boot && [ "$KIND" = phone ]; then
	# the UEFI with the loader inside, the way `xl enter` makes it: the loader then brings up its USB link
	L=bootloader/loader/loader.efi; [ -f $L ] || die "no $L: build the loader first (bootloader/loader/build.sh)"
	python3 bootloader/loader/mkpayload.py -o out/usb-payload.bin loader.efi=$L --usb > /dev/null
	FD_BASE=$FD_BASE FD_SIZE=$FD_SIZE PAYLOAD_BASE=$PAYLOAD_BASE UEFI_FD=$UEFI_FD DTB=$DTB UEFI_TARGET=$UEFI_TARGET \
		python3 bootloader/mkbootimg.py -p out/usb-payload.bin -o out/$DEVICE-boot.img
	echo "build: out/$DEVICE-boot.img -- boot it with: fastboot boot out/$DEVICE-boot.img (or build/run.sh $DEVICE)"
fi
