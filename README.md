# SLASH'EM

**Super Lotsa Added Stuff Hack, Extended Magic. The Amulet of Yendor
has been stolen, and NetHack's dungeon was not big enough.**

The Wizard of Yendor took the Amulet and went to ground in the Dungeons
of Doom. Armies were suggested, and a superhero or two, but in the end
it was cheaper to offer a reward and let some poor adventurer with
dreams of glory go and fetch it. That adventurer is you.

SLASH'EM is NetHack with more of everything. Eighteen roles, among them
the Flame Mage and the Ice Mage, the Necromancer who raises the dead,
the Undead Slayer and the Yeoman. Ten races, from drow and hobbits to
doppelgangers who change shape at will, lycanthropes and vampires.
Every role learns techniques of its own, special moves that come as
you grow. There are lightsabers in the dungeon, and pistols and
shotguns.

And the dungeon goes deeper and wider. Past the Gnomish Mines and
Sokoban wait Grund's Stronghold, an orc fortress; the Wyrm Caves; the
Lost Tomb; the Spider Caves; the Sunless Sea; the Temple of Moloch and
the Giant Caverns. One-eyed Sam keeps a black market behind a portal.
A guild of disgruntled adventurers holds a level of its own. In
Gehennom, beside Vlad's Tower, stands Frankenstein's Lab. And beside
your role's own quest, three more portals lead to the aligned key
quests, where the Nightmare, the Beholder and Vecna wait.

## Playing

Run `slashem` from this folder, with `nhshare` and `nhushare` beside
it. It plays in a terminal of at least 80x24.

- **hjkl yubn**: move, the last four diagonally; with **Shift**, run
- **F** and a direction: fight; **t**: throw, **f**: fire, **z**: zap
- **i**: inventory, **,**: pick up, **d**: drop
- **e** eat, **q** quaff, **r** read, **w** wield, **x** swap weapons,
  **W** wear, **T** take off, **P** put on, **R** remove, **a** apply
- **Alt-t** or `#technique`: use one of your role's techniques
- **o** and **c**: open and close doors; **s**: search; **.**: rest
- `>` and `<`: stairs down and up
- **#**: extended commands, such as `#pray`
- **?**: help, with the Guidebook; **;**: what is that symbol;
  **Ctrl-P**: earlier messages
- **S**: save and quit

Your saved game, scores and options are kept in the state folder,
`$XDG_STATE_HOME/slashem` (else `~/.local/state/slashem`), and SLASH'EM
carries on from your save the next time you start it. Options go in
`slashemrc` there, as `OPTIONS=` lines (or in `~/.slashemrc`);
`OPTIONS=name:Freya,role:Valkyrie` skips the questions at the start.

## Where it comes from

In February 1996 Tom Proudfoot released SLASH, which grew out of his own
NetHack-- and Stephen White's NetHack Plus; Enrico Horn later carried it
onto NetHack 3.2. In November 1997 Warren Cheung, a high school student
in his graduating year who had never written C before, patched SLASH
4.1.2 and the Wizard Patch together by hand one bored night. "I had to
fix a couple typos. It compiled. It ran. SLASH'EM was born." He went on
adding features, the bug reports and thank-you notes kept coming, and a
team grew around it: J. Ali Harlow, Pekka Rousu, Clive Crous, Kevin
Hugo, Paul Hurtley and many more. Their last releases came on 30
December 2006.

## Credits

- SLASH'EM: Warren Cheung, J. Ali Harlow, Pekka Rousu, Clive Crous, Paul
  Hurtley, Kevin Hugo and the SLASH'EM development team, after SLASH by
  Tom Proudfoot and Enrico Horn, and NetHack by the NetHack DevTeam
- Licence: NetHack General Public License (`license`)
- This package: Asciiverse, for the av launcher and the catalog at
  https://asciiverse.sh. What we changed is in PATCHES.md.

## Building

`sh asciiverse/build.sh` builds `out/slashem` and its data libraries
with the tty interface only (statically in Alpine for the release,
against the system curses on macOS), and `sh asciiverse/stage.sh`
gathers them with these files into `stage/`. The release workflow
(`.github/workflows/asciiverse-release.yml`) builds Linux and macOS
archives and Linux system packages. The original readme is
`readme.txt`.
