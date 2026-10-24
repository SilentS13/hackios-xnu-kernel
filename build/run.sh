#!/bin/bash
# Boot HackiOS on a device whose images are built (build/build.sh <device>).
#
# usage: build/run.sh <device> [seconds]
#   build/run.sh qemu-virt          QEMU window, 20 minutes (mouse = finger, H = home, L = lock)
#   build/run.sh redwood 900        the phone over USB: loader in RAM, system image in RAM, 15 minutes
#
# Nothing is flashed: on a phone everything lives in RAM and `host/xl reboot` (or a power-key reset)
# brings back Android. The session's log is printed (QEMU) or saved in logs/ (phones).
. "$(dirname "$(readlink -f "$0")")/lib.sh"
load_device "$1"
SECS=${2:-$DEFAULT_SECS}
mkdir -p logs
[ -f system/root.apfs ] && [ -f system/system.apfs ] || die "no images: run build/build.sh $DEVICE first"

case "$KIND" in
qemu)
	QEMU_SMP=${QEMU_SMP:-$SMP} CMDLINE="${CMDLINE:-$BOOTARGS}" host/qemu/run-ios.sh "$SECS" -display "${DISPLAY_KIND:-gtk}" |
		tee logs/qemu-$(date +%H%M%S).log
	;;
phone)
	command -v fastboot >/dev/null || die "fastboot is not installed"
	host/xl enter || die "could not get the phone into the loader (USB? unlocked bootloader?)"
	host/xl ramdisk system/system.apfs
	XL_SECS=$SECS XL_CMDLINE="${XL_CMDLINE:-$BOOTARGS} $BOARD_ARGS" host/xl run
	echo "run: 'host/xl screenshot shot.png' for the screen, 'host/xl reboot' to go back to Android"
	;;
*) die "unknown KIND=$KIND in $(device_conf "$DEVICE")" ;;
esac
