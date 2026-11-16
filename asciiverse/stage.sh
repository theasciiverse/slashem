#!/bin/sh
# Gathers what ships into stage/, after asciiverse/build.sh: the program,
# its two data libraries, the NetHack General Public License (the game
# shows it from its help menu), the Guidebook (also in the help menu),
# our README and PATCHES.md. The av archive is stage/ as it is;
# nfpm.yaml installs the same files in /usr.
set -eu
cd "$(dirname "$0")/.."
rm -rf stage
mkdir -p stage
cp out/slashem out/nhshare out/nhushare stage/
cp dat/license doc/Guidebook.txt stage/
cp README.md stage/README.md
cp PATCHES.md stage/
