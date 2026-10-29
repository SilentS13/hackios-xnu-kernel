#!/bin/bash
# Get Apple's XNU xnu-12377.121.6 (the iOS 26.5 kernel) and put UniXNU's patches/xnu-12377 series
# on top of it: the BCM2837 machine config and source for the machine layer Apple only ships
# closed, then the redwood device layer (patches-12377/zzzz-redwood-device-layer.patch and the
# whole files/ payload: the drivers and machine-layer sources it adds).
# usage: xnu/apply.sh [DIR]              (default: xnu-src/xnu-12377.121.6 next to this directory)
# env:   XNU_URL   where to fetch from (default: Apple's repository on GitHub)
set -e
HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
DIR=${1:-${XNU_SRC:-$HERE/../xnu-src/xnu-12377.121.6}}
URL=${XNU_URL:-https://github.com/apple-oss-distributions/xnu.git}
TAG=xnu-12377.121.6
BASE=ac9718fb1af618d5ce8678d0dc6e8a58f252216f

[ -e "$DIR" ] && { echo "apply.sh: $DIR already exists; not touching it" >&2; exit 1; }
[ -d "$HERE/patches-12377" ] || { echo "apply.sh: no patches-12377 in $HERE" >&2; exit 1; }
mkdir -p "$(dirname "$DIR")"
DIR=$(cd "$(dirname "$DIR")" && pwd)/$(basename "$DIR")
git init -q "$DIR"
trap 'echo "apply.sh: failed; removing the incomplete $DIR" >&2; rm -rf "$DIR"' ERR
cd "$DIR"
git remote add origin "$URL"
git fetch -q --depth 1 origin "refs/tags/$TAG:refs/tags/$TAG"      # that one release, without the history
git checkout -q -b ios26 "$TAG"
[ "$(git rev-parse HEAD)" = "$BASE" ] || { echo "apply.sh: $TAG is $(git rev-parse HEAD), expected $BASE" >&2; false; }
echo 'BUILD/' >> .git/info/exclude
n=0
for patch in "$HERE/patches-12377"/*.patch; do
	git apply --whitespace=nowarn "$patch" || { echo "apply.sh: $(basename "$patch") does not apply" >&2; false; }
	n=$((n + 1))
done
# The device layer's new sources (drivers, machine-layer files) arrive as whole files, and so does
# the CPU-counter source it replaces; the patch above carries only the edits to Apple's files.
cp -a "$HERE/files/." "$DIR/"
# .github/workflows/kernel.yml names the 8796 tree in its build and artifact paths, and the token
# has no `workflow` scope to change it: give that name to this tree instead.
ln -sfn "$(basename "$DIR")" "$HERE/../xnu-src/xnu-8796.121.2"
echo "apply.sh: $DIR: $TAG + $n patches (UniXNU patches/xnu-12377)"
