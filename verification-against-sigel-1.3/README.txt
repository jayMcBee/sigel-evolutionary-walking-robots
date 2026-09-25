Evidence for Phase V of the Qt port: the comparison against SIGEL 1.3.

Every gate in this repository compares the port against ITSELF. These files
are the other side of the comparison. They come from the frozen 1.3 release
binary running on the x86 machine, reached through the sigel-x86 session.

The rule is CAPTURE ONCE, DIFF FOR EVER. 1.3 never changes, so its output for
a given input never changes. Each file is captured once and read later to
compare against. Do not re-run the machine for a quantity already here.

  v1-1.3-roundtrip.txt                 container order: load and save.
                                       Results added 2026-09-18: all 14
                                       .exp on both machines; the robot a 1.3
                                       slave receives over PVM, octopus line
                                       by line; and how 1.3 numbers a robot
                                       built from a .rrb.
  v5-1.3-mdh-compared.txt              joint arithmetic, 1.3 against ours.
                                       An appendix added 2026-09-08 holds
                                       the sensor and force paths for the
                                       V5 step: live addresses, the eight
                                       constants, the truncated-pi radian
                                       factor, and which routines are dead.
                                       Re-derived from the binary the same
                                       day, except two lines marked in it.
                                       A result added 2026-09-18 holds the
                                       order of initializeJoint calls, the
                                       negative zeros our master formats, a
                                       master-against-slave fitness check,
                                       and fitness per individual.
  v5-ours-mdh.txt                      OUR half of that probe, not 1.3's
  v6-1.3-friction-nocollide.txt        do both setters negotiate
  v7-1.3-friction-nocollide-rules.txt  the four remaining rules for those
  v8-1.3-gp-blocks.txt                 PVMHOST and HISTORY, captured BEFORE
                                       the conversion they serve. Results 6
                                       and 7 were added 2026-09-08 for V2:
                                       hammer's container order, and the fact
                                       that 1.3's GUI and headless save paths
                                       write different files.

PORTING.md section 7 has the plan these belong to and what is still open.


RUNNING THE ORIGINAL SIGEL 1.3 BINARIES (2003) ON MODERN LINUX
===============================================================

Tested on x86_64 Debian 12, August and September 2026. No root, no VM,
no container, no multiarch, nothing installed on the host.

1. Ingredients
   - kdesigel/ : the unmodified 1.3 binary distribution (kbin.tar.gz from
     sigel.sourceforge.net): sigel, sigel_slave, supportingLibs/pvm3, ...
   - 12 Debian 3.0 "woody" i386 packages from archive.debian.org. sigel was
     built there (its .comment: GCC 2.95.4 Debian prerelease), so these are
     its exact libraries: libc6 2.2.5-11.8, libstdc++2.10-glibc2.2
     2.95.4-11woody1, libqt2-mt + libqt2-gl 2.3.1-22, xlibs + xlibmesa3
     4.1.0-16woody6, libpng2 1.0.12, libmng1 1.0.3, liblcms 1.08, libjpeg62 6b,
     libfreetype6 2.0.9, zlib1g 1.1.4.
     Unpack each into one directory:   dpkg-deb -x <pkg>.deb woody-root/

2. Run every 1.3 binary through woody's own loader:
     L=woody-root; LP=$L/lib:$L/usr/lib:$L/usr/X11R6/lib
     $L/lib/ld-linux.so.2 --library-path $LP kdesigel/sigel [-evolve x.exp]
   Environment:
     SIGEL_ROOT=<path to kdesigel>
     PVM_ROOT=<a pvm3 tree whose lib/LINUX/pvmd3 is a sh wrapper that execs
               the real pvmd3 through the same loader>
     PVM_ARCH=LINUX  PVM_ARCHDIR=LINUX
     GUI only: XLOCALEDIR=$L/usr/X11R6/lib/X11/locale  XMODIFIERS=@im=none

3. PVM and slaves
   - Export SIGEL_ROOT in pvmd's environment, not just the shell's: slaves
     inherit pvmd's environment, and an unset SIGEL_ROOT segfaults every
     slave (master then idles at 1% CPU forever).
   - The experiment's PVMHOST line is   <host> <maxProcs> 1 "<slaveDir>"
     and <slaveDir>/sigel_slave must be a sh wrapper that exports SIGEL_ROOT
     and execs kdesigel/sigel_slave through the woody loader.
   - SIGTERM is trapped: stop with SIGKILL. A killed pvmd leaves
     /tmp/pvmd.<uid>; delete it or the next pvmd exits at once.

4. Evolving headless
   - cd into the experiment's directory and pass the bare file name
     (sigel -evolve x.exp); an absolute path breaks pool-image file names.
   - Work on a copy: SAVEEXIT 1 overwrites the .exp.
   - Shipped experiments terminate on load (2001 TERMINATIONTIME); set
     TERMINATIONUSESDATE 0 and a generation limit. TIMETOSIMULATE is
     h m s ms, not seconds.
   - Use one slave (maxProcs 1) when fitness values must be trusted. With
     several slaves on one host, every slave rewrites the shared
     $SIGEL_ROOT/Terrain.ter while others read it: 0.5-0.85% of evaluations
     die ("Invalid storage access", fitness 0) and some score absurdly.
     Measured: a program scored 249.503 (Simple) under 8 slaves; the same
     program scored 0.0327516 three times out of three under 1 slave. That
     the absurd scores come from a half-read floor is inferred from the
     terrain reader's code (no stream checks), not observed. Fixed in
     SIGEL 2.0 (temp file + rename).

5. GUI and 3-D view
   Run in Xephyr started with +iglx (woody's libGL only does indirect GLX);
   without it the window opens but the GL view draws nothing. Qt 2 uses core
   X fonts. A modifier key stuck down in Xephyr (e.g. Alt from an Alt+Tab)
   turns typed letters into dialog shortcuts.
