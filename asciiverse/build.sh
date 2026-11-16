#!/bin/sh
# Builds SLASH'EM's terminal game, the tty interface only, into out/: the
# binary, slashem, and its two data libraries, nhshare (help texts,
# rumours, oracles, the encyclopedia) and nhushare (the dungeon and the
# compiled levels). The compiler and flags come from the environment, so
# one script serves every target (.github/workflows/asciiverse-release.yml):
#
#   asciiverse/build.sh                         # Debian, Ubuntu
#   LDFLAGS=-static asciiverse/build.sh         # Alpine, fully static
#   CFLAGS="-O2 -arch arm64" LDFLAGS="-arch arm64" asciiverse/build.sh  # macOS
#
# Needs a C compiler, make, flex, bison and ncurses with its headers
# (Debian and Ubuntu: build-essential flex bison libncurses-dev).
#
# configure and make run in a copy of the tree, out/build, since the
# build writes into its source folders. ASCIIVERSE switches on our own
# changes (PATCHES.md): the playground in the player's state folder, the
# data read from beside the binary, saves without an external compressor.
# Seduction by succubi and incubi is left out (--disable-seduce): no sex
# content in the Asciiverse catalog.
set -eu
cd "$(dirname "$0")/.."
OUT="$PWD/out"
B="$OUT/build"
rm -rf "$B" && mkdir -p "$B"
tar --exclude=./.git --exclude=./out --exclude=./dist --exclude=./stage -cf - . | (cd "$B" && tar -xf -)
cd "$B"

# SLASH'EM is K&R C in places (old-style definitions, empty parameter
# lists), which C23 no longer accepts: keep to C17 whatever the
# compiler's default
CFLAGS="${CFLAGS:--O2} -std=gnu17 -DASCIIVERSE"
# Apple clang makes some of old C's habits errors (calls to undeclared
# functions, implicit int); they stay warnings, as in GCC's C17
case "$(uname -s)" in
  Darwin) CFLAGS="$CFLAGS -Wno-error=implicit-function-declaration -Wno-error=implicit-int -Wno-error=int-conversion -Wno-error=incompatible-function-pointer-types" ;;
esac
LDFLAGS="${LDFLAGS:-}"

./configure --enable-tty-graphics --disable-x11-graphics \
  --disable-proxy-graphics --disable-sdl-graphics --disable-gl-graphics \
  --enable-data-librarian --disable-seduce \
  --with-owner=no --with-group=no \
  CC="${CC:-cc}" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" > configure.log 2>&1 \
  || { tail -20 configure.log; exit 1; }
grep -q '^#define DLB' include/autoconf.h || { echo "no data librarian" >&2; exit 1; }
grep -q '^#define SEDUCE' include/autoconf.h && { echo "SEDUCE still on" >&2; exit 1; }

# the game JOBS at a time (default 2), then the level compilers and the
# data libraries one at a time (their yacc steps may race under -j)
make -j"${JOBS:-2}" -C src > make.log 2>&1 || { grep -n "error" make.log | head -20; tail -30 make.log; exit 1; }
# (no Guidebook, which needs nroff, and no recover)
make data rumors oracles options quest.dat dungeon spec_levs dlb >> make.log 2>&1 \
  || { tail -30 make.log; exit 1; }

cp src/slashem dat/nhshare dat/nhushare "$OUT/"
chmod 644 "$OUT/nhshare" "$OUT/nhushare"
echo "built slashem"
