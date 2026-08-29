Evidence for Phase V of the Qt port: the comparison against SIGEL 1.3.

Every gate in this repository compares the port against ITSELF. These files
are the other side of the comparison. They come from the frozen 1.3 release
binary running on the x86 machine, reached through the sigel-x86 session.

The rule is CAPTURE ONCE, DIFF FOR EVER. 1.3 never changes, so its output for
a given input never changes. Each file is captured once and read later to
compare against. Do not re-run the machine for a quantity already here.

  v1-1.3-roundtrip.txt                 container order: load and save
  v5-1.3-mdh-compared.txt              joint arithmetic, 1.3 against ours
  v5-ours-mdh.txt                      OUR half of that probe, not 1.3's
  v6-1.3-friction-nocollide.txt        do both setters negotiate
  v7-1.3-friction-nocollide-rules.txt  the four remaining rules for those
  v8-1.3-gp-blocks.txt                 PVMHOST and HISTORY, captured BEFORE
                                       the conversion they serve

PORTING.md section 7 has the plan these belong to and what is still open.
