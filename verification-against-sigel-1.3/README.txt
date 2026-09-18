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
