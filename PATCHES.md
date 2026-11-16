# Changes from upstream

This is the Asciiverse packaging of SLASH'EM (Super Lotsa Added Stuff
Hack, Extended Magic), the NetHack variant by Warren Cheung, J. Ali
Harlow and the SLASH'EM development team.

SLASH'EM was developed in CVS on SourceForge and never had a git
repository of its own. Its history here is the CVS import at
github.com/osgamearchive/slashem (the `upstream` remote), up to
`0e2856a`, "Preparing for 0.0.8E0F1 release" (2006-12-30). On top of it
sits the release tarball, `slashem-source/0.0.8E0F1/se008e0f1.tar.gz`
from SourceForge (sha256
`e9bd3672c866acc5a0d75e245c190c689956319f192cb5d23ea924dd77e426c3`),
tagged `0.0.8E0F1`: it differs from the CVS commit only in expanded CVS
keywords and a stray `src/potion.c.orig`, both kept as released.

Why 0.0.8E0F1: it is the newest SLASH'EM release. It came out on the
same day as 0.0.7E7F3, the last of the 0.0.7 line (what Debian and most
servers carry), and adds the 0.0.8 development work: the dumplog
patch, the autoconf improvements and more bug fixes (`readme.txt`,
`history.txt`). CVS went on
for a year after it (nine commits, to 2007-12-30) without another
release; those are not included.

Licence: the NetHack General Public License, `dat/license` (SPDX
`NGPL`), shipped in the archive. Its section 2a asks that changed files
say so, with the date: every file we changed carries a "Changed for the
Asciiverse, 2026-11-16" notice beside the change.

## From Debian

Debian packages 0.0.7E7F3 (`slashem` 0.0.7E7F3-15; its packaging is
the `debian` remote, salsa.debian.org/games-team/slashem). Its patches
that apply to 0.0.8E0F1 are here as their own commits, with their
authors:

- `fix-libpng15-ftbfs` (Iain Arnell): `png_jmpbuf()` in the OpenGL
  interface and `tile2png`
- `spelling-error` (Vincent Cheng): spelling in the man page and messages
- `reproducible-build` (Reiner Herrmann): C locale for the data library,
  `SOURCE_DATE_EPOCH` for the build date
- `slashem-add-FDECLs-c99` (Peter Fordham, from Fedora): declarations of
  `rename_area` and `remove_area`
- `slashem-c99` (Florian Weimer, from Fedora): more C99 declarations

Not taken: `slashem-debian` (Debian's own paths and setgid layout),
`pointer-truncation-bug-860393` and `clang_FTBFS_Wreturn-type` (they
patch the proxy and GTK interface files of 0.0.7, which 0.0.8E0F1 does
not have).

## Source

- **GCC 14 and later** (`src/potion.c`, `util/lev_comp.y`,
  `sys/share/lev_yacc.c`): `upgrade_obj()` passed an int where
  `artifact_name()` writes a short, so only its low bytes were set; it
  is a short now. The level compiler cast to `struct room_door *`, a tag
  that does not exist; `room_door` is a typedef.
- **Files** (`sys/unix/unixmain.c`, `include/unixconf.h`,
  `src/files.c`): no setgid playground. With `ASCIIVERSE` defined, the
  playground is `slashem` under `$XDG_STATE_HOME` (else
  `~/.local/state`), made on the first run with its `save` folder, the
  empty `record`, `logfile` and `perm` files the game locks by linking to
  them, and a first options file, `slashemrc`, with `OPTIONS=color`
  (SLASH'EM leaves colour off on Unix). The data libraries `nhshare` and
  `nhushare`, the licence and the Guidebook are read from the folder the
  binary is in (`/proc/self/exe`, `_NSGetExecutablePath`, else
  `argv[0]`), else from the folder the game was started in, else from
  `/usr/share/slashem` (the system packages), through the data file
  prefix; `PREFIXES_IN_USE` is turned on for that. `HACKDIR`,
  `NETHACKDIR` and `-d` still choose another playground. Options:
  without `~/.slashemrc`, `slashemrc` in the state folder is read.
- **Saves** (`include/config.h`): on Unix SLASH'EM compressed saves with
  `/usr/bin/compress`, which most systems no longer have; saving then
  stopped at "Exec to compress save/... failed." With `ASCIIVERSE` it
  uses its own compression (`INTERNAL_COMP`).
- **No seduction scene**: no sex content in the Asciiverse catalog, so
  the succubus and incubus seduction scene is built out with the game's
  own option (`--disable-seduce`), as in our UnNetHack. Two things that
  go with it are now under `SEDUCE` too: a nymph, succubus or incubus
  "looks shocked! I'm not that way!" at a leash (`src/apply.c`), and the
  conduct list's "You have remained celibate" (`src/cmd.c`). A nymph
  still "seduces you and you start taking off" your armour to steal it,
  the classic NetHack line; left as it is.

## Packaging (new files)

- `asciiverse/build.sh`: configures and builds in a copy of the tree,
  `out/build`, since the build writes into its source folders, and
  leaves `out/slashem`, `out/nhshare` and `out/nhushare`. tty interface
  only (no X11, Qt, GTK, SDL, OpenGL or proxy), the data librarian on,
  `--disable-seduce`, `-DASCIIVERSE` for our changes, `-std=gnu17`
  (SLASH'EM's old-style C does not build as C23). The game `JOBS` at a
  time, then the level compilers and data libraries serially. No
  Guidebook build (it needs nroff; the shipped `doc/Guidebook.txt` is
  used) and no `recover`.
- Release fixes (v0.0.8E0F1-av.2): configure is told `--with-owner=no
  --with-group=no` (its check for the owner ran `find . -user`, which
  failed in the Alpine container); `sys/autoconf/config.guess` and
  `config.sub` are GNU's 2024 versions (the 2005 ones did not know
  aarch64 Linux); on macOS each architecture builds on its own runner
  (macos-15-intel, macos-15), since the build runs the tools it
  compiles, and Apple clang's errors for old C habits (calls to
  undeclared functions, implicit int) are warnings again, as in GCC's
  C17. On a failed build the script prints the error lines.
- `asciiverse/stage.sh`: the program, both data libraries, `license`,
  `Guidebook.txt`, README.md and this file in `stage/`, the av
  archive's contents.
- `asciiverse/nfpm.yaml`: .deb, .rpm, .apk and Arch packages:
  `/usr/bin/slashem`, the data in `/usr/share/slashem`, the licence in
  `/usr/share/doc/slashem`. No `.desktop` file or icon.
- `README.md`: the README in each archive, game first.
- `asciiverse/record-preview.py` and `asciiverse/slashem.cast`: the
  catalog preview.
- `asciiverse/slashem.toml`, `asciiverse/slashem.md`: the catalog
  manifest and store page, as drafts for hub.
- `.github/workflows/asciiverse-release.yml`: static Linux in Alpine
  (x86_64, aarch64; ncurses linked in) and macOS (x86_64, arm64; system
  ncurses), Linux system packages, on a `*-av.*` tag.
- `.gitignore`: `out/`, `dist/`, `stage/`.

## Not built

- Windows: SLASH'EM's Windows console port builds with its own
  makefiles (`sys/winnt`), not autoconf, and has not been tried.
- The X11, Qt, GTK, GNOME, SDL, OpenGL, GEM and proxy interfaces,
  tiles, sound and `recover`.
