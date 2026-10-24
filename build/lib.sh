# shellcheck shell=bash
# Shared by build/*.sh: find the workspace and load a device profile.
#   . "$(dirname "$(readlink -f "$0")")/lib.sh"; load_device "$1"
ROOT=$(cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")/.." && pwd)
cd "$ROOT" || exit 1

die() { echo "${0##*/}: $*" >&2; exit 1; }

# A device profile is device/<vendor>/<device>/<name>.conf, e.g. device/xiaomi/redwood/redwood.conf or
# device/qemu/virt/qemu-virt.conf (device/_template.conf documents its variables).
device_conf() {        # device_conf <name>: the profile's path, if there is one
	local c
	for c in device/*/*/"$1".conf; do [ -f "$c" ] && { echo "$c"; return 0; }; done
	return 1
}
device_names() {       # every profile's name
	local c
	for c in device/*/*/*.conf; do [ -f "$c" ] && basename "$c" .conf; done | tr '\n' ' '
}

# load_device <name>: sources the profile
load_device() {
	local name=${1:-${DEVICE:-}} conf
	[ -n "$name" ] || die "which device? $(device_names)"
	conf=$(device_conf "$name") || die "no profile device/*/*/$name.conf (build/new-device.sh $name makes one)"
	# shellcheck disable=SC1090
	. "$conf"
	DEVICE=$name
	export DEVICE KIND
	echo "${0##*/}: device $DEVICE -- $NAME ($KIND)"
}

have_userland() {
	[ -f system/build.sh ] || die "system/build.sh is missing from this checkout"
}
