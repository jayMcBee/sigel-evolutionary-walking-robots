# Checks

These scripts check SIGEL after a change. Run them from anywhere; each one
finds the repository by itself. Build first with `make`, `make guidrive
coredrive`, and `make B=build-asan coredrive pvm-link`.

## `check.sh`, the main check

    ./checks/check.sh

1. **Module compile.** Compiles every SIGEL source file and counts the
   warnings. A file that does not compile fails.
2. **Headers standalone.** Each header must compile on its own.
3. **Encodings.** No tracked text file may have Windows line endings.
4. **Programs.** `sigel` and `sigel_slave` must be built and up to date, and
   `sigel_slave` must start and stop cleanly when there is no PVM.
5. **GUI behaviour.** Drives the real SIGEL window with simulated mouse and
   keyboard input, and compares what happens with
   `baselines/guibehaviour-baseline.txt`. Eleven scenarios:
   - the main window: experiment tree, sorting, adding, deleting and
     resetting individuals;
   - every field on the five parameter pages;
   - all eight exports, byte for byte;
   - exporting over an existing file;
   - five dialogs;
   - the MetaGP number fields;
   - export, import and export again, so each importer really reads;
   - the MetaGP editor and statistics;
   - pages locked while an evolution runs;
   - the random numbers for a given seed;
   - a crash guard when an experiment is opened.

   It also fails on any broken signal-slot connection, and it runs the
   parameter pages again under German and Danish number formats.
6. **Page save.** Edits on the pages must reach the saved experiment file
   (`baselines/pagesave-baseline.txt`).
7. **Save round trip** (shown as "v2 round trip vs 1.3"). Two experiments are
   saved twice; the saved files must stay the same.
8. **Forms.** The form compiler gives no warnings, every image a form uses
   exists, and every form is in the build.
9. **Robot check.** The issues that the robot check raises on the 7 shipped
   experiments must match `baselines/robotcheck-baseline.txt`.
10. **Unit tests.** Runs `make test`: the unit tests in `sigel/tests/` must
    all pass.

It prints one row per check and a total, and exits non-zero if anything fails
or is skipped. The warning count on the total line is part of the result.

## Separate scripts

11. **`fitness-check.sh`.** 21 fitness values, 3 for each of the 7
    experiments, must match `baselines/fitness-baseline.txt`. On the
    sanitized build it also runs the unit tests, with leak detection on. Run
    it on both builds:

        ./checks/fitness-check.sh | diff -u checks/baselines/fitness-baseline.txt -
        ASAN_OPTIONS=detect_leaks=0 ./checks/fitness-check.sh build-asan \
            | diff -u checks/baselines/fitness-baseline.txt -

12. **`dictorder-dump.sh`.** The order in which robot parts reach the
    simulation must match the baseline.

        ./checks/dictorder-dump.sh | diff -u checks/baselines/dictorder-baseline.txt -

13. **`pvm-check.sh`.** Starts a PVM daemon, makes one plain PVM round trip,
    then sends SIGEL's own PVM data through real PVM under AddressSanitizer.
    Pass or fail; no baseline.

        make pvm && make B=build-asan pvm-link && ./checks/pvm-check.sh

## Other files

- `baselines/`: the five files the checks compare against.
- `programs/`: the check programs. `guidrive.cpp` drives the interface,
  `coredrive.cpp` runs one fitness evaluation or, with `-check`, lists the
  issues of the robot check, `expstruct.py` fingerprints a
  saved experiment, `pvm_link.cpp` and `pvm_smoke.c` test PVM.
