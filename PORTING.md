# SIGEL — Qt 2.3 → Qt 6 migration plan

**THE GOAL — restated 2026-08-27, and it governs every decision below.**

**SIGEL runs under modern Qt 6, complete, with its original interface migrated
progressively from Qt 2 to Qt 6.** Not the core alone. Not a rewritten
interface — the 2003 forms and widgets, carried forward.

Two rules follow from it:

- **No rebuilding.** The interface is ported, not redesigned. Every dialog,
  form and widget arrives at Qt 6 as the same interface it was in Qt 2.
- **The end state is plain, clean, modern Qt 6.** No `q2compat.h`, no
  Qt3Support, no compatibility flags in SIGEL's own code. Stepping stones are
  allowed — the shim and Qt3Support are both stepping stones — but the port is
  not finished while any of them is still in the tree. See D25 and §10.
- **Use Qt's own migration tools.** `qt3to4` and `uic3 -convert`, from Qt 4.8,
  run in a container with an old toolchain (§4). Hand-written substitutes are a
  last resort, for what the tools leave behind — see D19, D20 and §4.

**Scope widened 2026-08-22.** Was Qt API only. Now also covers getting SIGEL to
build and run, because nothing else can be verified without it — see §3.

**Status — 2026-09-05**

| phase | state |
|---|---|
| 0 — **DONE 2026-09-05.** comments to English | Two passes on 2026-09-05: 30 lines in 19 files, then a further 101 comment lines in 46 files that the first pass's method could not see. **No `.c`/`.cpp`/`.h`/`.ui` file under the source root holds a byte above 127.** Every count this row used to carry was wrong — §7 says how |
| A — core onto Qt 6 | **done**, tags `step-A0`…`step-A9` |
| B — ownership explicit | **subsumed by Phase D**, which deletes the containers rather than converting them. **0 `setAutoDelete` calls left in core**, re-measured 2026-08-30 after D25c: D11 removed the last in `SIGEL_Robot`, D24 the last in `MT_Control`, D25b replaced the two `fitTaskList` calls with an RAII guard, and D25c wrote out `tours`' two real frees at their sites. **0 tree-wide as of 2026-09-05** — every remaining `autoDelete` mention is a comment explaining what the Qt 2 original did |
| R — build and run | core builds and runs. **No longer checked only against itself** — Phase V has confirmed both the ordering and the arithmetic against the 1.3 binary, §7 |
| T — old-Qt tool container | **done 2026-08-27**, §4. `tools/` removed
2026-09-20; the forms it converted are committed |
| D — delete the shim, migrate the data | **DONE 2026-08-30.** `q2compat.h` and `q2compat_check.cpp` deleted; `include/compat/` gone; **no `Q2*` shim type is used anywhere**. D1–D27. *D27 once said 71 lines of Qt 2 containers survived for Phase C to convert. **Phase C is done and none survive**: re-measured 2026-09-05, five textual mentions remain in the GUI modules and all five are comments.* The shim's self-check step is gone from `check.sh`. §10 |
| P — PVM | **DONE 2026-08-28.** Vendored 3.4.3 replaced by upstream 3.4.6; nine patches carry the four config lines and Debian's eight source fixes; `libpvm3.a` and `pvmd3` build; SIGEL's two PVM objects link against them and `SIG_GPPVMData` round-trips through real PVM. `sigel`/`sigel_slave` still need Phase C. §7 |
| C — GUI | **DONE.** C1–C10, C11a–C11d, C12. *The API conversion is complete — zero Qt 2 spellings in live code, swept 2026-09-05.* All 20 Designer forms converted; five GUI modules build as archives; both programs link and run; 100 dead `connect()`s repaired, tree-wide count 0. Nine port defects were found by DRIVING the interface that reading it could not see — `clear()` emitting a signal Qt 2 blocked and killing the app on a large delete, an eaten ampersand, a dead `key()` virtual, `truncate(-1)`, a pre-filled field Qt 6 selects and Qt 2 did not, ten unpinned validators in `MT_GUI`, and three in the Create-constants dialog that reached generated data. Detail in §9 |
| V — check against the 1.3 binary | **V1, V5's MDH probe, V6, V7 and V8 all done, all PASS.** Ordering: 10 of 10 container orders match. Arithmetic: `twoBases` exact bit for bit, `octopus` 9/9 with three joints exact and 5 ulp worst. **V6, V7 and V8 done 2026-08-29** — friction and no-collide negotiation, their four remaining rules, and the GP parameter blocks captured *before* their conversion. `verification-against-sigel-1.3/v6`, `v7`, `v8`. **V9 done 2026-08-29, 3 of 3** — three function bodies disassembled, which symbol lookups cannot see. **V3 SATISFIED 2026-09-02** — same-box determinism, demonstrated twice by the oracle (`serA`≡`serB`, `octGateA`≡`octGateB`). **V4 DROPPED 2026-09-03** — whole-run digests cannot cross an x87/IEEE boundary, and the counts that appear to agree are forced by the code. **Replaced by a measurement of OUTPUT needing no reference: the port EVOLVES** — C11 has the figures. **V2 DONE 2026-09-08** — a whole experiment through the GUI save path, twice, against the 1.3 capture; checked as `v2 round trip vs 1.3`. **Every V step is now done or dropped.** |

**SCOPE — DECIDED 2026-08-23. Read this before changing anything.**

**The reference for this port is SIGEL 1.3 and nothing else.** The 1.3 source in
`sigel/` and the 1.3 binary running on the x86 box, reachable
through the `sigel-x86` Claude session. A port must not change results, so the
target is that our build reproduces what the 1.3 binary does.

**The 14 published experiments cannot check this port.** They were produced in
August 2001 by SIGEL 1.0. Validating a port of 1.3 against them measures every
1.0 → 1.3 change as though it were ours.

**The 1.0 → 1.3 regression is real, pre-dates this migration, and is DEFERRED.**
Written up in `regression_1.0_to_1.3.md`. Not to be worked on, and not to be
mixed into these commits, until the Qt port is complete.

Three jobs, in order, no overlap:

1. **The Qt port** — the interface (Phase C), and PVM so the app can actually
   evolve a gait. This is the work.
2. **The 1.0 → 1.3 regression** — after, if wanted.
3. **The diagnostics wishlist** from the `sigel-x86` session — 9 items on
   validating robot models at load. Recorded in `regression_1.0_to_1.3.md`.
   Not part of either job above.

**THE ROUNDING QUESTION — IMPACT MEASURED AT ZERO 2026-09-09. The decision is
still open and needs sign-off**, per the 2026-09-07 parking note, after the
options are reviewed in code.
Nothing is recorded here as decided. What follows is measurement.

The question: whether to route every real number through a C-style formatter to
match 1.3's tie rounding.

**WHAT 1.3 DOES.** Its `QTextStream` does not format doubles — it builds a format
string and calls `sprintf` (`qtextstream.cpp:1776-1805`, Qt 2.3's source; the oracle
disassembled `__ls__11QTextStreamd` in `libqt-mt.so.2.3.1` and found the same).
So every rounding decision is glibc's. Qt 6 formats doubles itself, through
`libdouble-conversion`, and rounds a tie **away from zero** where glibc rounds
**to even**.

**SIGEL WRITES AT THREE PRECISIONS, and that is what decides the impact.**

| set to | where | what it writes |
|---|---|---|
| **5** | `SIG_Renderer.cpp, vectorToPovray` | every POV `<x, y, z>` |
| **6** (default) | — | `.exp`, `.rrb`, and the POV rotation matrix |
| **50** | `SIG_GPPVMData.cpp:116, :157` | the master↔slave PVM transfer |

Both non-default calls are 1.3's own, so the port carried them faithfully.

**Tie rates**, measured with `tiecheck.cpp`, removed 2026-09-20 and recoverable
from git history. Ties live at precision 5:

| sample | prec 5 | prec 6 | prec 50 |
|---|---|---|---|
| multiples of 1/16 over [0,100] | **22.49%** | 0% | 0% |
| multiples of 1/64 over [0,100] | 6.97% | **13.50%** | 0% |
| ordinary doubles in [-1000,1000] | 0% | 0% | **0.21%** |

**IMPACT PER WRITER. NOTHING OBSERVABLE DIFFERS.**

| output | impact |
|---|---|
| shipped `.exp` and `.rrb` | **none** — precision 6, see below |
| POV | a sub-pixel shift in a rendered image. **Nothing reads a `.pov` back**, zero readers in the tree; it is input to an external renderer |
| PVM transfer | ours at both ends within one run. Only matters against a 1.3 slave, which nobody runs. **Measured against 1.3's octopus payload 2026-09-18:** 15 tokens differ at 50th-digit ties, each the same double on both sides |
| the graveyard `.ind` file name, `SIG_GPTournament.cpp, inhume` | **never written.** `LIVEUNDEAD` is 0 in all 14 shipped experiments, and nothing lists or parses the graveyard — it has a writer and no reader |
| `-0` (Qt 6 `0`, glibc `-0`, differs at every precision) | absent from all shipped data. **Not absent from the PVM stream:** our master formats as many −0.0 values as 1.3's — 2 in octopus's payload, 88 in walker's — and prints each as `0` where 1.3 printed `-0`, so our slave reads +0 where 1.3's reads −0 (oracle, 2026-09-18; `v5-1.3-mdh-compared.txt`). On our machine the slave's robot scores the same as the master's for individuals 0 to 2 of both experiments; no other individual measured |

**THE SHIPPED-DATA CLAIM WAS WRONG AND IS WITHDRAWN.** This section used to say
the difference "is in a shipped file, `twoBasesHighMutationRate.exp:112290`". The
**value** is there; the **difference** is not. `0.703125` has exactly six
significant digits, so at precision 6 it prints verbatim. The `0.70312` /
`0.70313` split exists **only at precision 5**, and no `.exp` is written at
precision 5. Measured by round-tripping that file through the port: generation
114 comes back unchanged and the whole experiment-history section is
byte-identical. All five shipped dyadic values print verbatim at precision 6 —
`0.5` (103 occurrences), `-0.5` (80), `1.5` (12), `-0.75` (2), `0.703125` (1),
out of 165,549 numeric tokens.

**THERE IS NO Qt 6 CONFIGURATION OPTION.** Measured on Qt 6.10.2 across ten
formatting paths: `QTextStream <<`, the same under `setLocale` at `c()`, `C`,
`en_US`, `de_DE` and `OmitGroupSeparator`, plus `QLocale::toString`,
`QString::number`, `QString::asprintf` and `QString::arg`. All ten agree with
each other and disagree with glibc. **`QString::asprintf` runs Qt's own engine,
not the C library**, so swapping to it changes nothing. `setLocale` is worse than
useless: `en_US` inserts a group separator that plain `QTextStream` never writes.
No `QT_*` environment variable and no exposed feature flag reaches it.

**WHAT A FIX WOULD COST, if it is ever wanted.** **99 lines in 30 files** stream
a double — counted by injecting a deleted `operator<<(QTextStream&, double)` and
compiling all 185 sources, not by grep. Four more sites write a double outside a
stream. **Do not subclass `QTextStream`:** it is `final` since Qt 6.9 under
`QT_DISABLE_DEPRECATED_UP_TO`, inheritance goes in Qt 7, and the obvious form is
silently broken — every base `operator<<` returns `QTextStream&`, so
`file << "TAG " << value` takes the base overload. That route needs 357
declaration changes across 118 files against 99 line changes. **Use
`std::to_chars`, not `snprintf`:** measured byte-identical to glibc over ~8.6
million values at precisions 5, 6, 15, 17 and 50 in all three notations, and
locale-independent by design. `snprintf` follows `LC_NUMERIC` (`0,5` under
`en_DK.utf8`), `snprintf_l` does not exist on glibc, and `strfromd` is **not**
locale-safe there despite its reputation.

**TWO ATTEMPTS ON THIS WERE WITHDRAWN, and both failed the same way — a test that
could not fail.** The 2026-09-07 attempt argued from "only a dyadic rational can
be an exact decimal tie", which excludes nothing because every finite double is
one, and confirmed itself by reformatting `%.6g` output with `%.6g`, which is the
identity. The 2026-09-09 assessment then priced a 99-line rewrite before tracing
one difference to an observable consequence, and repeated this section's own
shipped-file error back as evidence. **Ask what breaks if nothing changes,
before costing a fix.**

**Order of work, revised 2026-08-27.** Reordered around the goal above: the
interface is the work, so it goes first, and PVM follows because without it the
ported interface has nothing to drive.

1. ~~Build core plus a small program that runs one fitness evaluation, under
   AddressSanitizer.~~ **Done.**
2. ~~**Stand up the old-Qt container**~~ — **done**. §4. Removed 2026-09-20.
3. ~~**Get a reference from the 1.3 binary**~~ — **V1 and V5's MDH probe done,
   both pass.** This was item 1's missing half and is no longer outstanding.
4. **Phase D — delete the shim.** **DONE 2026-08-30, D1–D27.** The data
   migration is done and only the 7 `.rrb` needed it. All six container types
   and the header itself are gone. Ordered before C by **D25** so the interface is ported once, to
   the final target. §10.
5. **Phase C — the interface**, one module or one form at a time. §7.
6. Fix PVM — **superseded 2026-08-28.** That count measured the vendored 3.4.3,
   now replaced by upstream 3.4.6: four config lines, no source edits. Phase P.
7. Full headless run, compared against the captured 2003 run.
8. ~~The evolution-loop containers, still untestable~~ **The loop RUNS as of 2026-09-03** —
   30 generations, best fitness 0.063794 → 0.141625. C11.

**A caveat that governs the order of what is left.** Everything after Phase D's
simulation-side work is in the **evolution loop**, which now runs (C11):
~~the 13 remaining `setAutoDelete` sites, `Q2PtrList`'s `fitTaskList` and
`toSpawnList`, and the rest of `Q2PtrVector`~~ — **all converted as of D25c;
core `setAutoDelete` is 0, and D26 converted the last two GUI-side members.
The shim itself was deleted in D27.** Both checks and AddressSanitizer reach
none of that code, so it was converted blind, which was the largest risk in
this plan. It was met by
per-step review with differential harnesses against the shim, and by the 1.3
binary; the record of what that caught is in the step sections. **Phase P was expected to retire it and did not.** PVM builds and runs as
of 2026-08-28, but `SIG_GPFitnessTrainer` dispatches through `pvm_spawn` of
`sigel_slave`, and Phase C has to build `sigel_slave` first. The blocker moved;
it did not lift.

**Every step is reviewed by an independent agent, and every round so far has
found a real defect.** §0 has the rule; it is not optional.

## 0. Working on this

- Source root: `sigel/`
- ~~The shim: `include/compat/q2compat.h`; its self-check:
  `include/compat/q2compat_check.cpp`~~ **Both deleted in D27.** Recover them
  from git history if you need the Qt 2 semantics they recorded — see D27 for
  the four that unported code still depends on.
- Build: `make` at the repo root builds `sigel` and `sigel_slave` into `build/`
  and fills `sigelApp/`, the folder SIGEL is started from:
  `cd sigelApp && ./sigelLauncher`. The checks also need `make guidrive
  sigel_eval`, and `make B=build-asan sigel_eval pvm-link` for the two gates
  that run under AddressSanitizer and UndefinedBehaviorSanitizer.
- Verify: `./checks/check.sh`, from anywhere, compiles every module and header.
  Takes several minutes, and **exits non-zero if anything fails or is skipped**.
  *This said "it runs no code", which was true between D27 and Phase C.* It now
  runs `sigel_slave`, a headless GUI structure probe, and `guidrive` through
  eleven scenarios plus two locale re-runs. Further execution is in
  `./checks/fitness-check.sh`, which runs `sigel_eval -selfcheck`.
- PVM: `make pvm && make B=build-asan pvm-link`, then `./checks/pvm-check.sh` starts a daemon
  and runs both round trips. It has no baseline: it is PASS/FAIL.
- Run the published experiments: `./checks/replicate.sh`. Read the
  scope note at the top first — those experiments are from SIGEL 1.0 and do not
  test this port.
- **After every step, an independent agent reviews the diff with fresh eyes.**
  Not optional — every round so far has found a real defect, and a compile check
  proves nothing about ownership.
- **SIGEL source comments explain SIGEL code and nothing else.** They never cite
  this document or its siblings, a decision number, a step, a test program or
  script, a review, the oracle, a date, or the deleted shim. This document cites
  SIGEL code by file and function name; SIGEL code never cites this document.
- When adding an assertion to `sigel_eval -selfcheck`, break the code it covers
  and confirm the check aborts. Nine assertions have passed on broken code.
  *This rule was written for the shim self-check, deleted in D27; it applies
  unchanged to its successor.*
- **This file is the only log of what was done, besides git.** Decided 2026-09-20.
  Anything that records finished work belongs here, not in a new file.
  `future_refactorings.md` holds the to-do list and nothing else.
  `regression_1.0_to_1.3.md` is deferred analysis of that regression, and is not
  touched. `physics_backends.md` was folded into this file on 2026-09-20.

---

## 1. Repo

```
sigel/                                      the repo root
├── Makefile                                the build, §7 Phase R
├── PORTING.md                              this file
├── future_refactorings.md                  the to-do list this work defers into
├── regression_1.0_to_1.3.md                sibling doc, DEFERRED
├── checks/                                 the four gates and replicate, §7
│   ├── check.sh                            compiles every module and header,
│   │                                       runs the interface. The gate
│   ├── dictorder-dump.sh                   prints the container order; the
│   │                                       check is the diff vs its baseline
│   ├── fitness-check.sh                    21 fitness values, and -selfcheck
│   ├── pvm-check.sh                        does PVM run? Phase P, P3 and P4
│   ├── replicate.sh                        the 1.0 experiments; not a gate
│   ├── baselines/                          the six files the gates diff against
│   └── programs/
│       ├── guidrive.cpp                    the GUI harness, 33 scenarios
│       ├── sigel_eval.cpp                  one fitness evaluation
│       ├── expstruct.py                    fingerprint of an .exp, C11
│       ├── pvm_link.cpp                    SIGEL's PVM objects vs real PVM
│       └── pvm_smoke.c                     one PVM round trip
├── sigel/                                  the 2003 source and its data
│   ├── src/       15 module dirs           ~40k LOC
│   ├── include/   15 module dirs           ~25k LOC
│   ├── ui/        20 .ui files             Qt 2 Designer format
│   ├── pixmaps/  textures/                 loaded at run time by name
│   ├── Terrain.ter  stdConf.mt             run-time data
│   ├── doc/                                2 Doxygen settings files, 2003
│   ├── sigelLauncher  sigelDynClient       sigelDynClient: item 47
│   ├── povrayLauncher                      renders the POV-Ray export
│   ├── COPYING  README  kdesigel.doxygen   upstream, 2003
│   └── 9 Windows build files               future_refactorings.md item 35
├── vendor/                                 third-party code and our fixes to it
│   ├── patches/                            12 patches, applied by `make`
│   ├── supportingLibs.tar.gz               tracked, so a clone builds offline
│   └── pvm3.4.6.tgz                        upstream PVM, tracked -- Phase P
├── experiments/                            the 7 kept experiments, tracked
├── robots/                                 the 7 robots, tracked
├── shim/                                   4 pre-standard C++ headers
├── verification-against-sigel-1.3/         the 1.3 captures, Phase V
├── build/  build-asan/                     untracked; `make clean` removes build/
├── sigelApp/                               untracked, made by `make`: start SIGEL here
└── downloads/                              UNTRACKED ONLY
    ├── supportingLibs/                     dynamechs, cv97, newmat09, fparser,
    │                                       pvm3, Dynamo (maths only). Extracted
    │                                       from vendor/, then patched by `make`
    └── sigelSourceDistribution.1.0/        the 1.0 release, for the regression
```

Pristine source tagged `v1.3-pristine`. Everything diffs against it.

SIGEL, Uni Dortmund LS11, 2001–2003, GPLv2. Genetic programming evolves walking
gaits for simulated robots. Upstream called it "KDESIGEL" and there is **no KDE
dependency** — zero `k*.h` includes — so the source folder is `sigel/`, renamed
2026-09-20, when it was hoisted out of its three levels of nesting.

---

## 2. Facts — measured, do not re-derive

Counts from the extracted 1.3 tree.

Measure with `command grep` or Python. Plain `grep` here wraps `ugrep -I`, which
silently skips the 9 Latin-1 files — it has produced wrong counts several
times.

### Qt API that must change

| Item | Sites | Notes |
|---|---|---|
| `QArray<T>` | 74 | Qt 2 value array |
| `QDict<T>` + `QDictIterator` | 61 | |
| `QVector<T>` | 48 | **array of pointers** in Qt 2 — not Qt 6's `QVector` |
| `QList<T>` | 37 | **list of pointers** in Qt 2, with `autoDelete()` |
| `QCString` / `QValueList` / `QQueue` / `QListIterator` | 16 | |
| `setAutoDelete` / `autoDelete` | 47 | 38 `TRUE`, 9 `FALSE`. *Pristine-tree figures.* Today: **0 anywhere** — the 19 remaining textual hits are all comments recording what the Qt 2 code used to free |
| `QString::null` | 42 | |
| `sprintf` | 23 | |
| `lower` 5, `findRev` 4, `latin1` 3, `simplifyWhiteSpace` 3, `upper` 2 | 17 | |
| implicit `QString` → `const char*` | 74 | needs inspection, not sed |
| `WFlags` / `WType_*` / `WStyle_*` | 60 | |
| `QListView` / `QListViewItem` | 105 | |
| `insertItem()` | 50 | menus, combos, listboxes |
| `QPopupMenu` | 29 | |
| `qApp` 19, `setCaption` 16, `QApplication` 9 | 44 | |
| `QMultiLineEdit` | 6 | |
| `QGLWidget` | 6 | |
| `.ui` files | 20 | `<!DOCTYPE UI><UI>`, no version attr = Qt 2 format |

**≈660 call sites + 20 forms.**

### Qt API that does not change

`QString` 958, `QTextStream` 232, `QDateTime`/`QTime`/`QDate` 91, `QObject` 28,
`QFile` 15, `QDir` 14, `QStringList` 12, `QDataStream` 12, `QThread` 3 *(pristine-tree figure; today 0 — step A removed the last of them, 2026-09-09)*,
`QTimer` 2.

**Absent, and usually the worst part of a Qt port:** 0 `QPainter`, 0
`paintEvent`.

**`SIGNAL()`/`SLOT()` survived as a MECHANISM; nine kinds of signal did not.**
Tree-wide: **299 `SIGNAL()` + 312 `SLOT()` = 611 macros, in 246 `connect()` and
54 `disconnect()` calls.** *A tree-wide `\bconnect *\(` returns **248**, and two
of those are not Qt at all — the BSD socket `connect()` in
`src/manage_dyn_slave.c` and one inside a doc comment in
`include/SIGEL_Simulation/SIG_Simulation.h`. Every real one is in a `.cpp`.*
The tree has **37 distinct signal signatures**, not 41: 41 counts *raw spellings*,
which differ only by whitespace, and `connect()` compares the normalised form.
Normalised they partition exactly — **17** SIGEL's own, **12** dead in Qt 6 (the
nine kinds below, across their overloads), **8** still live.
A string-based connect to a signal that is not there compiles, links, runs and
never fires, and nothing in the compiler or in `check.sh` could see it. Each
signature below was resolved with `QMetaObject::indexOfSignal` against the real
Qt 6.10.2 meta-object — the same lookup `connect()` performs — not read off a
porting guide.


| Qt 2 signal | sites | Qt 6 |
|---|---|---|
| `activated()` on `QAction` | 74 | `triggered()` |
| `rightButtonClicked(QListView/QListBoxItem*, …)` | 5 | **no equivalent** — `setContextMenuPolicy` + `customContextMenuRequested(QPoint)` |
| `activated(const QString&)` on `QComboBox` | 6 | `textActivated(const QString&)` |
| `doubleClicked(QListViewItem*)` | 4 | `itemDoubleClicked(QTreeWidgetItem*,int)` |
| `selectionChanged()` on `QListView`/`QListBox` | 4 | `itemSelectionChanged()` |
| `selectionChanged(QListViewItem*)` | 2 | `currentItemChanged(item, previous)` |
| `selected(QListBoxItem*)`, `selected(QAction*)` | 2 | `itemActivated(…)`, `QMenu::triggered(QAction*)` |
| `currentChanged(QListView/QListBoxItem*)` | 2 | `currentItemChanged(item, previous)` |
| `clicked(int)` on `QButtonGroup` | 1 | `idClicked(int)` |

**The ones that look dead and are not, so nobody re-breaks them:** `clicked()`
is live — moc emits a cloned method for the default argument of `clicked(bool)`;
`activated(int)` on a `QComboBox` is live and only the `QString` overload went;
`valueChanged(int)`, `textChanged(…)`, `stateChanged(int)`, `toggled(bool)`
(including on `QAction`), `pressed()` and `timeout()` are unchanged.

**A `SIGNAL()` name does not tell you the class, and that cuts both ways.**
`lostFocus()` was listed here as `QLineEdit`'s, to become `editingFinished()`.
**Qt 2.3's `QLineEdit` had no `lostFocus()` at all** — the signal arrived in Qt
3. Both sites connect on an **`MT_Editor`**, which declares its own
`void lostFocus();` and emits it from `hideEvent`. `editingFinished()` would
have been an outright regression: **`MT_Editor` exists to distinguish Return
(sets `acceptChange`, commits) from focus-out (does not)**, and
`editingFinished()` fires on both. Confirmed on the running 1.3 — typing `999`
and pressing Return commits; typing `111` and clicking away leaves the old
value. The counter-check is to parse every `signals:` block in the tree and
intersect; against the corrected list that intersection is empty. **The live
column has the same blindness one column over**: `textChanged(const QString&)`
is unchanged on `QLineEdit`, which all 27 senders are, but Qt 2's `QComboBox`
declared it too and Qt 6's does not (`currentTextChanged`), and
`QSpinBox::valueChanged(const QString&)` is likewise gone. **Check the class,
not the spelling.**

**100 dead connects in the pristine tree; C4, C6, C7 and C8 repaired all of
them, and `check.sh` fails any module whose count exceeds its baseline of zero.**
83 are `connect()` and 17 `disconnect()` — a dead `disconnect` is equally a
no-op. The check also scans `sigel.cpp`, `sigel_slave.cpp` and
`SIGEL_RealInterface`, which sit in no module directory. **The forms need a
separate pattern**: a `.ui` `<connection>` carries the bare signal name as XML,
so the main regex structurally cannot match one — *it was pointed at the `.ui`
directory anyway and claimed a coverage it could not have.* All 49 form
connections are live. **And the regex self-tests**, because it is the one check
here that is a pattern rather than a compiler and so the only one that can
quietly stop matching — which it did, for a whole commit, when
`selectionChanged` was anchored to `( )`. 33 rows of spellings that must match
and near-misses that must not, including a converted
`selectionModel()->selectionChanged(sel, desel)`, which must pass.

### Structure

| Metric | Value |
|---|---|
| Core files needing **no** container work | **210 of 266** |
| Core files touching dead Qt 2 containers | 56 — worst is `SIG_Robot.cpp` (30) |
| `Q_OBJECT` in core | **3** — `SIG_Simulation`, `MT_GPManager`, `MT_Controller`. Was 4; `SIG_DynaSystem` went with the Dynamo backend, §7 |
| Core files touching dialogs | 7 |
| Core → GUI back-edges | 4 edges, 5 includes |

```
SIGEL_Tools, SIGEL_Environment, MT_GPSystem     no deps
SIGEL_Robot      ← Tools                        (+1 back-edge → Simulation)
SIGEL_Program    ← Tools, Robot                 (+1 back-edge → GP)
SIGEL_RobotIO    ← Tools, Robot
SIGEL_Simulation ← Tools, Program, Environment, Robot
MT_Control       ← MT_GPSystem                  (+1 back-edge → MT_GUI)
SIGEL_GP         ← all of the above             (+2 back-edges → MasterGUI)
SIGEL_Visualisation, SIGEL_CommonGUI, MT_GUI, SIGEL_MasterGUI, SIGEL_SlaveGUI
```

---

## 3. Build and run — AUTHORIZED 2026-08-22

Previously out of scope. Authorized because the port cannot be checked without
it: `check.sh` runs `g++ -fsyntax-only` and never links or executes SIGEL, so
every ownership change made in Phase B is verified only by inspection and
review. A missing `delete` and a doubled `delete` both compile.

Known work:

- vendored libraries on gcc 15
- PVM. **Superseded by Phase P**, which replaced the vendored 3.4.3 with
  upstream 3.4.6. In 3.4.6 `pvmlog.c:499-505` is guarded by `USESTRERROR`,
  which `conf/LINUX64.def` defines, so the `sys_errlist` defect is gone. The 23
  failures on `rpc/types.h` still hold and `-ltirpc` is the answer. §7 Phase P.

- a build system to replace a `configure.in` that detects the OS by grepping
  `/proc/version` for `SuSE`
- pre-standard `for`-scope in `MT_*`

**LP64 portability is clean** — 2 pointer-to-`int` casts, nothing else.

**The first milestone needs none of that.** The seven fitness functions run a
simulation locally, with no PVM and no interface. A small program that loads a
robot and runs one evaluation exercises `SIG_Robot`, `SIG_Body`, the six
dictionaries, `SIG_DynaMechsSimulationData` and the recorders — 8 of the 14
containers, everything Phase B has converted. Do that before touching PVM.

The other 5 containers (`pool`, `tours` in the master's evolution loop;
`pvmHosts`, `pvmTasks`, `toSpawnList` in the PVM trainer) only run during a full
distributed evolution, which is why they are last.

---

## 4. Old Qt as a tool, not as a build target — REVISED 2026-08-27

Qt 3 and Qt 4 do not have to **build** on gcc 15 — they have to **run**, once,
as converters, and an old toolchain in a container is where they run. The
project already has the precedent: the 2003 i386 binary runs against Debian
woody libraries on the x86 box (§9).

**`qt20fix` is not a Qt 2 → Qt 3 tool**, and nobody should go looking for one.
It is Qt 2's own Qt **1.x** → Qt **2.x** script; the proof is `src/doc/porting.doc`
in the Qt 2.3 source, headed *"Help with porting from Qt 1.x to Qt 2.x"*, whose
line 156 points at `qt/bin/qt20fix`. That extract was deleted 2026-09-20; it is
in the full supportingLibs archive, on SourceForge and in git history. The
"20" is "2.0". **Qt never shipped a Qt 2 → Qt 3 converter at all**; Qt 3's
porting guidance is a manual change list plus the `QT_COMPAT` headers.

**The staged path (D19):**

```
Qt 2.3  --uic3 -convert, qt3to4-->  Qt 4 + Qt3Support  --hand-->  Qt 6
```

Two legs. The first has Qt's own tools behind it; the second is hand work.

**That the first leg starts at Qt 2.3 and not Qt 3 is measured, not assumed.**
Phase T ran `uic3 -convert` over all 20 Qt 2 forms: 20 converted, 0 failed, no
warnings. Whether `qt3to4` reads Qt 2 *sources* as willingly is **untested** and
is the next thing to find out.

**The honest limit.** Qt 5 removed Qt3Support, so `qt3to4` output — `Q3PtrList`,
`Q3ListView`, `Q3PopupMenu` — cannot be the destination. The final leg off
Qt3Support onto Qt 6 is hand work whatever we do. What the tools buy is the
mechanical bulk and, above all, **the 20 `.ui` forms**, for which there is
otherwise no path short of writing a Qt 2 form parser (old C7, now deleted).

**What the container needs:** Qt 4.8's `uic3` and `qt3to4`. Built — see
Phase T in §7. No Qt 3 is needed, because there is no Qt 2 → Qt 3 leg.

**OPEN — the current SDK's half of this.** The goal at the top of this file says
to use the tools of *current and past* Qt SDKs. This section covers only the
past ones. Qt 6 ships **`Qt5Compat`**, and Qt 5.15 ships the staging machinery
(`QT_DISABLE_DEPRECATED_BEFORE`, `QT_DEPRECATED_WARNINGS`) built for exactly the
last leg we are calling hand work. Whether an intermediate Qt 5 build earns its
keep is **not decided** — see §9.

**TQt3** (Trinity, R14.1.4) builds on a current toolchain but renames every
`Q*` → `TQ*`. Fallback if the container proves harder than expected, not the
first choice, because the rename has to be undone afterwards.

---

## 5. Decisions — signed off 2026-08-18

| # | Decision | Answer |
|---|---|---|
| **D1** | Migration strategy | ~~**(a)** compat shim over Qt 6~~ — **superseded 2026-08-27 by D19** |
| **D2** | Target Qt version | **6.9.2**, Ubuntu `qt6-base-dev`; recorded, not pinned. `qmake6 -query QT_VERSION` and `uic --version` both say **6.10.2** on 2026-08-30 at C1. Why it moved from the recorded 6.9.2 is not established here; what is measured is that nothing was changed to follow it and all four checks reproduce, warning count included |
| **D3** | GUI scope | ~~**(b)** core only~~ — **superseded 2026-08-27: Phase C is the goal and is AUTHORIZED** |
| **D4** | GUI toolkit | **(a)** Qt 6 Widgets *(Phase C)* |
| **D5** | `.ui` handling | ~~**(a)** converter script~~ — **superseded 2026-08-27 by D20**: `uic3 -convert` |
| **D6** | Container targets | **(a)** `QList<T*>` |
| **D7** | The `autoDelete` sites | **(a)** blanket rule from D6; exceptions in §9 |
| **D8** | `QString`→`const char*` | **(b)** `.toUtf8()` |
| **D9** | `QListView` → | **(a)** `QTreeWidget` *(Phase C)* |
| **D10** | Back-edge cutting | **(a)** forward-declare; no callback needed anywhere |
| **D11** | Verification depth | **(a)** per-file syntax check; Phase B adds the §9 audit |
| **D12** | Step granularity | **15 steps** — A0–A9, B1–B5, plus Phase 0 per D14 |
| **D13** | Qt 2 behaviour that is itself a defect | **fix it, and fix the cause** — §9 |
| **D14** | Comment language | German → English as **Phase 0**, before A1 |

## 5b. Decisions — signed off 2026-08-27, for the interface

These restate the goal at the top of this file as decisions. D19 supersedes D1,
D20 supersedes D5, D24 supersedes D3.

| # | Decision | Answer |
|---|---|---|
| **D19** | Migration strategy | **staged and tool-assisted.** Qt 2 → Qt 4 + Qt3Support → Qt 6, via `uic3 -convert` and `qt3to4` in a container (§4). Supersedes D1(a). The existing `q2compat.h` shim **stays** for the core, which is already through it and works |
| **D19a** | The core/GUI boundary | **SETTLED 2026-08-27 by D25: delete the shim from core first, then port the GUI once, straight to clean Qt 6.** Measured, not assumed: `qt3to4` leaves Qt 2's `QList`/`QDict`/`QArray`/`QVector` **completely untouched** — 9 `QList<>` in `SIG_GPParameter.cpp` before, 9 after, 0 `Q3PtrList`. Qt 3 had already renamed `QList` to `QPtrList`, so `q3porting.xml` maps *that*; our Qt 2 spelling reads to it as a Qt 4 class. So the boundary fails loudly at compile time (Qt 6's `QList` is a value list) rather than silently, and the choice is real: spread the shim to 466 more GUI sites (**534**, re-measured at C1 — §7) and remove it twice, or remove it once, first |
| **D20** | `.ui` handling | **`uic3 -convert` in the container**, straight from the Qt 2 form — measured in Phase T, no Qt 3 uplift needed. Supersedes D5(a). No hand-written form parser |
| **D21** | Interface fidelity | **ported, not redesigned.** The 2003 interface arrives at Qt 6 as itself. A widget with no Qt 6 successor gets the nearest equivalent, recorded here — not a redesign |
| **D22** | The Qt 2 style classes | **`QStyleFactory::create("Fusion")` for the `#else` (Motif) branch.** `QMotifPlusStyle` has no successor in Qt 6; Fusion is the closest it offers. Chosen 2026-08-27 after comparing the two styles Qt 6.9 offers here. **The `#ifdef _WINDOWS` branch keeps Windows** — `QWindowsStyle` is no longer a public class but Qt 6 still creates that style by name, so under D21 its nearest equivalent is `QStyleFactory::create("Windows")`, not Fusion |
| **D23** | Phase C granularity | **one module or one form at a time**, each its own commit, each independently reviewable. No API-wide sweeps across modules |
| **D24** | GUI scope | **Phase C is authorized.** Supersedes D3(b), which scoped the interface out. Named separately because D19–D23 did not carry it and the status table cited a struck-through row |
| **D26** | What the 1.3 binary is asked for | **structure and arithmetic, not fitness equality.** Three tiers, in descending confidence: the container ordering and numbering, which compare exactly (V1, V2); per-individual fitness, which is chaotic across architectures and is therefore a judgement (no cross-machine check; V4 dropped); the non-integrating quantities, which compare exactly but need `gdb` (V5). Bit-exact agreement on an integrated trajectory is **not** a target and its absence proves nothing — §7. Recorded because this file repeatedly described the missing reference as "fitness numbers", which is the one thing that binary cannot usefully give |
| **D25** | What "done" means | **Plain modern Qt 6, nothing left over.** `q2compat.h` deleted, no Qt3Support class anywhere, no compatibility flag on SIGEL's own code. This moves §10's "drop the Qt 2 emulation" from optional debt into a **required phase**, and with it the data migration that section describes — the shim exists because `Q2Dict`'s hash order numbers the links, so the 7 `.rrb` and 12 `.exp` files must be rewritten before it can go. **Ordered before Phase C**, so the GUI sites (466 as counted then, **534** re-measured at C1 — §7) are ported once, to the final target, instead of twice. Vendored third-party code is out of scope for this rule: qhull, cv97, Dynamo and PVM keep `-w -fpermissive` |

## 5c. Decisions — signed off 2026-09-02, from driving the interface

| # | Decision | Answer |
|---|---|---|
| **D27** *(decision; §10 also has a **step** D27, the shim deletion — the two D-series overlap and this is the first collision)* | The duplicate MetaGP `A&bout` | **removed**, with its trailing separator. Present in 1.3 and verified there; wired to the same `slotAbout()` as `Help > About` and opening the identical `SIG_InfoBox`. The port's first intentional difference from 1.3. `Help > About` untouched |
| **D28** | The `QSpinBox` over-range divergence (C11a) | **accepted, not fixed.** 1.3 accepts out-of-range digits and clamps on commit; the port refuses the keystroke and commits a truncated prefix. It is reachable **only by typing a number outside the box's own range**, and the differing value is **visible in the box** before anything is saved — 1.3 shows 99, the port shows 10. Contrast what the port did fix: `clear()` killed the application, the ampersand rendered wrong, a negative width silently wrote no file — all reachable with valid use. The fix is not the 33 lines of it, it is **owning a custom widget forever**: every future form edit and every new spin box must remember `SIG_SpinBox` or silently opt out. Pinned in `guibehaviour-baseline.txt` (`commits=`) so it cannot drift; the prototype and the measured comparison are in `future_refactorings.md`'s history at `1dba5f4`, and the "Not doing" list in §7 points there. **Revisit if** a dialog spin box turns out to feed something unvalidated, or if anyone actually hits it |
| **D29** *(signed off 2026-09-04)* | Changing run parameters **while an evolution is running** | **FORBIDDEN in the port, whatever 1.3 permits.** The reason is the specification, not 1.3: GA and GP systems are not commonly built that way — the parameters define the run. **The port's second intentional divergence**, after D27. **Implementation, and the wrong versions it went through, are in §10 — read that before changing the guard** |
| **D30** *(signed off 2026-09-07)* | The mid-run lock's first two holes | **A run check belongs wherever an action is RE-ENABLED, not only where it is disabled.** `SIG_MainWindow::slotEnableNoExperimentActions` handed 24 run-locked actions back on a tree click, and `New Experiment` and `Open Experiment` were in no lock list at all. Both measured by reverting the fix and re-running `runlock`. The pattern for any further route: a run check where the action is re-enabled, plus the action in `evolutionRunningActions`, plus a `runlock` case with a positive control. **Its own section is in §7, "D30 — parameter changes during a run are forbidden"**, and D30a is the hole it missed |
| **D31** *(signed off 2026-09-09)* | Line endings | **LF ONLY, tree-wide. No more DOS.** This decision overrides the guard that existed to prevent it. **100 files of the 2003 source converted, 17,750 CRLF pairs.** **The conversion is line endings only except for two bytes, and `git diff --ignore-cr-at-eol` is NOT what proves it** — that flag strips a trailing CR from *both* sides, so it would equally hide a CRLF being *introduced*. The proof is a direct comparison of every one of the 100 files: `re.sub(rb"\r+\n", b"\n", git show HEAD:f) == working file`, exact, with no `\r` surviving anywhere. Zero anomalies. **Two lines of `sigel_slave.mak` are the one real content change**, and calling them line endings flatters them: `:598` and `:647` ended `\r\r\n`, so the byte removed is an INTERIOR one — under NMAKE that trailing CR is part of the variable's value. The `\r+` in the proof above is what swallows the case, so the proof cannot tell it from a line ending; it is called out here instead. Nothing else in the tree has a run of two. **Binaries are excluded and this is not cosmetic** — three tracked binaries hold 12 incidental `\r\n` byte pairs (`pvm3.4.6.tgz` 9, `altLogo.png` 2, `noExperiment.png` 1), and a blind repo-wide replace would corrupt all three. Extensions touched: 36 `.cpp`, 35 `.h`, 19 `.xpm`, 5 `.dsp`, 3 `.mak`, 1 `.mt`, 1 `.dsw`. **No `.exp` and no `.ui`**, so no reference artefact was touched. **Lone CRs are left alone, and NOT because they are Mac-classic line endings** — the first version of this row said that and it was wrong. Six tracked files held lone CRs then and git called all six binary, so this check never even reads them; 27 do today: `pvm3.4.6.tgz` 3859, `noExperiment.png` 691, `JustGreen.pnm` 2848, `altLogo.png` 208, `Hippie.pnm` 208, `Stone.pnm` 68. All five `.pnm` are **P6 raw raster**: those bytes are pixel values that happen to equal `0x0d`. They were never line endings. **The `encodings` check was turned round in the same commit**, so that commit is not line endings alone — `check.sh`, `PORTING.md` and `future_refactorings.md` change with it. The check used to say *a file that HAD a CR must still have one*, with `ENC_BASELINE=25`; it now says **no tracked text file may carry CRLF**, expected zero, reads every tracked file rather than five extensions present in the root commit, lists them with `-z` so a C-quoted path cannot break `open()`, and reconciles — every file lands in exactly one of ok / CRLF / binary / unreadable, or it aborts. **It asks `git ls-files --eol` what is binary rather than testing for a NUL byte**, because the NUL test got two files wrong: `Hippie.pnm` has no NUL in its 196,668 bytes and `UniDo_LSXI.pnm`'s first NUL is at offset 15,456, so both were judged as text and passed only by luck. **Read the `w/` column, not `i/`**: while this change was being made, `sigel_slave.mak`'s index blob read `i/-text` — HEAD still held its two `\r\r\n`, which git's own heuristic calls binary — against a working file of `w/lf`, and testing both columns dropped a real text file out of the check. **Both columns read `lf` once this is committed, so the demonstration is gone and only the rule survives.** Read 610 text files and 8 binaries then; 571 and 29 today. **A floor of 500 was added**, because zero failures is also what a check that read nothing reports: a dead `git ls-files` gave `COUNTS 0 0 0 0`, four non-empty numbers, which the fail-closed branch did not catch. Teeth-tested: CRLF into a `.cpp` and into `sigel_slave.mak` both caught and named, CRLF into a texture correctly ignored, and all seven branch states driven by hand — including a **tree-wide** CRLF regression, which the first version of the floor misreported as *"it did not run"* with one failure instead of 611, and a below-floor count, which the first version printed as `0 pass` while adding up to 499 passes to the total. Both found by review 2026-09-09. The bucket reconciliation is a tautology as the loop is now written and is **not** counted as coverage; it is kept only so the earlier bare-`continue` shape cannot come back. **`SIGEL_ROOT` is the source tree**, so `stdConf.mt` and the 19 `.xpm` pixmaps the conversion touched are the very files the GUI checks load at runtime; the `gui behaviour` check covers them. The `.xpm` are loaded by path and `#include`d nowhere, and a C string literal cannot span a raw newline, so no removed CR was ever inside a quoted pixel row. **No `.gitattributes` exists and none was added.** `* text=auto eol=lf` would make git enforce this rather than only detect it; not done, because it changes what every future checkout writes and that is a separate decision. On a clone with `core.autocrlf=true` the working tree comes back CRLF and this check goes red tree-wide — which is the check working |
| **D32** *(signed off 2026-09-09)* | `SIG_Experiment::gpManager` renamed to `guiGPManager` | **A deliberate divergence from the 1.3 name, and the only one of its kind so far.** Four members across the tracked tree were called `gpManager`; three hold an `MT_GPManager *` inside the meta modules, where the name is right. The fourth, `SIG_Experiment.h, SIG_Experiment`, holds a `SIG_GUIGPManager *` — and it was the **only** `SIG_`-typed member in that class not named after its own type with the `SIG_` prefix stripped. The other nine follow the rule exactly (`gpExperiment`, `gpParameter`, `simulationParameter`, `environmentView`, `robotView`, `experimentView`, `allIndividualsView`, `languageParameters`, `experimentItem`); the class's remaining members are named by role (`widgetDict`, `menuGPParameter`, …) and were never in scope. So this is the class's own rule applied to the one member that broke it, not a new scheme. **20 sites** when this was signed off: 13 in the files D34 has since renamed to `SIG_GUIGPExperiment.{h,cpp}`, 5 in this file, 2 in `guidrive.cpp`, both comments. The three `MT_GPManager` members and the `SIG_GPManager gpManager` local in `sigel.cpp, main` are correctly named and were left alone; the 1.0 tree holds the same member and is untracked, so a future sweep will re-find it there and should leave it. **VERIFIED AS `.text`-IDENTICAL, NOT AS BYTE-IDENTICAL OBJECTS** — a data member's name never reaches a mangled symbol, but `-g` is on and DWARF records member names, so the objects legitimately differ. `sigel.cpp` is the interesting one and was checked: it is the single translation unit where both names coexist, and its `.text` is unchanged |
| **D33** *(signed off 2026-09-09)* | Where the mid-run protection lives | **IN THE UI. The model is not to be touched.** Work stays on the interface side; the GP manager and the other model classes are not touched. No new behaviour goes into the model. Removing a dead 2003 stub is not new behaviour, so `SIG_GPManager::running()` was deleted — see D29's passage in §10. Nothing is added to `SIG_GPManager` or `MT_Controller`. **The D29 counter, `g_runningEvolutions`, is to be removed, not moved into the model.** A move into `SIGEL_GP` was rejected: the core model is not changed to fix an interface enablement issue. **Replaced 2026-09-15 by a UI-side run state.** Each experiment has `evolutionRunning`, which `SIG_GUIGPExperiment::isRunning()` returns. Every run check asks `SIG_ExperimentListView::isRunning()`, which is true while any experiment runs. The decision is to lock the whole application during a run, and it is done — §9, "The run lock is DONE". Running several experiments at once makes no sense: a run needs all the resources there are. **Exceptions: D37, D41, and a slave directory of `.` in `SIG_GPFitnessTrainer`, which spawns a bare `sigel_slave` for PVM to find on its own path** — item 39, 2026-09-21: required for the experiments' `. 1 1 "."` host entry to work. |
| **D34** *(signed off 2026-09-15)* | `SIG_Experiment` renamed to `SIG_GUIGPExperiment` | **By decision, and the second deliberate divergence from a 1.3 name, after D32.** The interface experiment class now follows the rule the manager pair already uses: model `SIG_GPManager`, interface `SIG_GUIGPManager`; model `SIG_GPExperiment`, interface `SIG_GUIGPExperiment`. Its files follow it: `SIG_Experiment.h` and `SIG_Experiment.cpp` became `SIG_GUIGPExperiment.h` and `SIG_GUIGPExperiment.cpp`, with the include guard, every include and the 2003 build files. D32's row keeps the old class name, because it records a rename made under it |
| **D35** *(signed off 2026-09-15)* | Overwrite prompts on save and export | **In these slots the file dialog's own prompt is the only one.** SIGEL's "File exists..." prompt is gone from `SIG_ExperimentListView::slotSaveExperiment`, the five parameter and population exports and `SIG_GUIGPExperiment::slotRobotSave`; the file is written once. When SIGEL adds the extension itself and that file exists, `SIG_GUIGPExperiment::checkEnding` puts the date stamp `-yyyy-MM-dd-hh-mm-ss` between name and ending, one second later while that name is taken too. So nothing is overwritten and nothing asks; `slotGNUPlotExport` gets the same rule. **Not covered yet:** save paths that add an extension without `checkEnding` — `SIG_AllIndividualsView` (`.prg`, `.ind`), MT_GUI, and `MT_Controller`. Several of them still show their own "There is another file with this name" prompt: `MT_PopulationWidget::slotExpInd` and `slotSavePop`, six `MT_StatisticsWidget` exports, `MT_IndividualsWidget::slotExportConstants` and `MT_Controller::slotSaveSetup` |
| **D36** *(signed off 2026-09-15)* | Dialogs out of sight | **No SIGEL dialog may end up out of sight and block the window.** This holds whatever the save rules become. **The interface half is done — D39**, all 32 sites in `SIGEL_MasterGUI`, after D35 gave the eight save and export file dialogs a parent. What is left is in D39's row and in `future_refactorings.md` item 25; some of it is in model classes that D33 keeps untouched |
| **D37** *(signed off 2026-09-15; its review passed 2026-09-16)* | One exception to D33: the generation counter during a run | **`SIG_GUIGPManager::updateIndividualView` writes the pool generation into the Experiment page's counter.** The line was commented out in 1.3 and is active again; nothing else in `SIGEL_GP` changes. `SIG_GPManager::run` calls `updateIndividualView` for every individual right after it raises `poolGeneration`, so the counter changes when a generation completes. It was chosen over a timer in `SIGEL_MasterGUI` that would read the generation. It was done as its own step, with an independent review. **Not checked:** no check starts a run. The unchecked `evolution` scenario samples the counter during a run |
| **D38** *(signed off 2026-09-16)* | The generation progress bar | **`generationProgBar` is driven from the interface.** The widget sat in `SIG_ExperimentViewBase.ui` since before 1.3 and nothing ever wrote to it. A `QTimer` in `SIG_GUIGPExperiment` counts the individuals whose `SIG_GPIndividual::upToDate()` is true and sets the bar; it starts in `slotStartEvolution` and stops in `slotEvolutionStopped`. **Nothing in `SIGEL_GP` changes, so this is not a second exception to D33.** The count is not monotonic: a tournament writes offspring with fitness -1, so the bar steps back when new work appears. Tried in use and accepted: the bar reaches 100%, but moves up and down a good deal on the way, and shows progress between counter updates. Its tooltip says what it counts. **Not checked:** no check starts a run |
| **D39** *(signed off 2026-09-16)* | Dialogs out of sight — the interface half of D36 | **Every dialog in `SIGEL_MasterGUI` has a parent.** 32 sites: 26 static `QMessageBox` and `QFileDialog` calls that passed `0` or `nullptr`, plus three constructed dialogs (`SIG_EditHostDialog` twice, `SIG_InfoBox` once) and three `QMessageBox` calls that passed `this` inside `SIG_GUIGPExperiment` — where `this` is a `QWidget` that never gets a parent and is never shown, which is no better than none. Real widgets use `this`; `SIG_GUIGPExperiment` uses `experimentListView`, as D35's seven export dialogs already did. Confirmed on the real desktop: the main window can no longer cover them. Checked: the `dialogs` scenario prints the parent of Edit Host and of About, and the `exportall` scenario's two `parentIsTheMainWindow` lines moved from 0 to 1. **Still open:** six prompts in `MT_StatisticsWidget` that D35 wants deleted rather than re-parented, five in `MT_Control` and two `QProgressDialog` in `SIGEL_GP` that D33 keeps untouched, and three in `SIG_EnvironmentRenderer` that are reachable but were not asked for |
| **D40** *(signed off 2026-09-16)* | The window and splitter at start-up | **1280x860, tree 280 px, splash unscaled.** 1.3's `resize( 900, 750 )` already opened taller than the work area on a 1366x768 screen. The splitter asked for `setSizes( {2, 6} )` meaning a 1:3 split; **measured, Qt gives the tree 71 %** — numbers far below the splitter's width are ignored and the surplus goes by size policy, and `QTreeWidget` expands where the stacked widget does not. Real pixel widths and `setStretchFactor( 0, 0 )` / `( 1, 1 )` fix it and keep it fixed on resize. `widgetBase` had `setScaledContents( true )`, which stretched the square 448x448 `noExperiment.png` across the whole content area; it is centred at its own size instead. All three are deliberate divergences from 1.3. A fixed size, on purpose: no sizing to the screen, for now. Geometry is **not** remembered between sessions; `QSettings` was proposed and rejected as overkill |
| **D41** *(signed off 2026-09-17)* | The second exception to D33: a run that has lost PVM | **`SIG_GPFitnessTrainer` may REPORT that PVM is unreachable; it may not act on it.** `checkTask` split `pvm_probe`'s three returns — above zero receive, zero wait, below zero give up — because the old `if (info != 0)` sent an error into the receive branch, where `pvm_recv` blocks for a message that cannot come or fails and leaves the caller waiting for a task already destroyed. On `PvmSysErr` it sets `pvmLost`; `spawnTask` sets it too, because an unreachable daemon shows up there first and `pvm_spawn` reports that through its own return rather than through `taskId`. `SIG_GPManager::pvmIsLost()` is the only way out, `start()` clears it, and nothing in `SIGEL_GP` acts on it. `SIG_GUIGPExperiment`'s progress timer reads it, ends the run the way `Stop` does and names the reason. **The option that was measured and rejected:** watching `$PVM_TMP/pvmd.<uid>` disappear — a run completed three further generations with that file removed, because an enrolled task keeps its socket. **Not checked:** no check starts a run; the `evolution` scenario dismisses the new dialog and prints it |
| **D42** *(signed off 2026-09-17)* | Saying why a run ended | **The interface tells the user when a run ended without doing anything.** `SIG_GUIGPExperiment::slotEvolutionStopped` shows the message when three things hold: the pool generation did not move, `guiGPManager->userTerminated` is false, and `terminationAlreadyMet()` finds the condition already true. The message names the setting — the date, the duration or the generation number — and the tab it is on. 1.3 shows nothing, so this is a deliberate divergence. Decided 2026-09-15: the interface must give feedback in that case. **Nothing in `SIGEL_GP` changes**, so this is not a third exception to D33: `terminationAlreadyMet` reads `gpParameter` and the interface's own copy of the run start time, because `SIG_GPManager::startTime` is private. **That copy is what makes the duration model work** — testing for an all-zero duration, as a first version did, missed every non-zero duration that expires inside the first evaluation, which is the ordinary case at about 98 s per generation. **Two cases not covered:** MetaGP with `SAVEEXIT` completes its first generation so the branch never runs, and a run that throws says only that an error stopped it. **Not checked:** no check starts a run; the `evolution` scenario dismisses the dialog and prints it, measured for a date and for a duration |


---

## 5a. Decisions — signed off 2026-08-22, for Phase R

| # | Decision | Answer |
|---|---|---|
| **D15** | Where the robot models come from | downloaded from `sigel.sourceforge.net`, §9. ~~Untracked, in `data/`~~ **superseded 2026-09-19:** the 7 kept experiments and their robots are tracked, in `experiments/` and `robots/`, by decision |
| **D16** | The Dynamo branch in `SIG_Simulation.cpp` | ~~build Dynamo, SOLID and qhull~~ **superseded 2026-08-28**, recorded in §7: it was decided and executed: the branch and its 13 file pairs are deleted, `SIMULATIONLIBRARY 0` now fails loudly, and the Dynamo archive is cut to the maths objects. SOLID and qhull went with it, and both were deleted from disk on 2026-09-20, with SOLID's include path |
| **D17** | Build system | **plain `Makefile`** at the repo root. Phase C can bring its own for `moc` and `uic` |
| **D18** | What "runs clean under ASan" means for R3 | ASan and UBSan errors are pass/fail; LeakSanitizer output is a recorded baseline, because §10's leaks are out of scope |

Established while answering these:

- **`autoDelete` splits 38 owning / 9 non-owning**, all literal `TRUE`/`FALSE`,
  which is what makes D7(a) a lookup rather than a judgement.
- **Non-ASCII existed only in comments** — 46 of 380 files, 119 lines, no string
  literal. So no runtime string carries a byte ≥ 0x80 and `.toLatin1()` /
  `.toUtf8()` are identical over all real data. The three explicit `.latin1()`
  sites feed POSIX paths, where UTF-8 is right.

**What D11(a) does not buy.** Phase A steps are pure renames, so a syntax check
covers the whole error class they can produce. Phase B is hand-written
ownership, where the correct and the double-freeing version both compile. The
self-check and independent review are the substitute.

---

## 6. Git protocol

- One branch per phase. One commit per step, prefixed with the step ID.
- Tag each completed step: `step-A4`.
- No squashing — the per-step history is what makes a bad step bisectable.
- Commit messages: **one line, no body**. Detail goes in this file. The code is the commit.

**Phase P departed from this, by decision.** No `step-P*` tags exist — none
were wanted, so the last tags are `step-R1` and `step-A9`. P1's
commit is not prefixed either. And each step has **two** commits, not one: the
step, then the fix for what its review found. Every round found something, so
folding the fix into the step would have hidden it. Eight commits, `ee981a1`
through `f0f2daa`.

---

## 7. Steps

**Exit criterion per step:** `./checks/check.sh` from anywhere — **1097 pass, 0
fail**. *The figure moves with the number of tracked text files, because the
`encodings` check adds its own count to the total. Measured trail: **1136**
until 2026-09-19, when `experiments/` and `robots/` arrived and
`dictorder-reorder.py` went; **1168** once two unused vendored patches went the
same evening; **1162** after the 2026-09-20 clean-up of three shim headers, the
qtmig container and `tiecheck.cpp`; **1098** once 64 files of 2003 build
machinery went — 64 files, 64 passes; **1097** when `physics_backends.md` was
folded into this file.*
**The pass count was 853 until D31 and the jump is not new coverage of SIGEL's
code.** The `encodings` check used to read 404 files of five extensions and now
read all 618 tracked files then, 8 of which git called binary: its pass count went
327 → 610, and 853 + 283 = 1136 exactly. *The check reads every tracked file, so
adding or deleting one moves the total by one; `portinglog.txt` did that on
2026-09-09.*
**The warning figure is not an exit criterion and moves with the code.** It was
508 at D31 and D32 and reads 503 today. A step that changes no code should not
move it; one that does, will.
**It now needs `sigel_eval` built**, which `check.sh` does not build for you:
the V5 section reads constants out of `build/sigel_eval`, so run
`make sigel_eval` first or that section fails, correctly,
with 852 pass and 1 fail. `check.sh` builds `guidrive` and `sigelApp/` itself,
which builds the two programs too, but not `sigel_eval`.
*It was 851 after `real clicks`, earlier the same day.* It was 850 on
2026-09-05, after the `slave gui` and `form minimums`
sections and the two forms corpus assertions. It **exits non-zero** when
anything fails or is skipped. Zero is reachable because the two permanently
Windows-only `WIN_*` files are an explicit exclusion rather than a standing
red — see C11c.

**Earlier pass/fail figures are not comparable and have been removed.** The basis
changed at C4 (standalone headers folded into the totals, plus the regex self-test
and the parsers check), and again when the Dynamo backend was deleted. The current
figure is the one in "Handover" below; every historical arithmetic trail that used
to sit here was superseded, and it sat at the top of the section §0 sends a new
session to. Warnings are on an unchanged basis and remain comparable throughout.

**There are four baseline checks plus `pvm-check.sh`, which has no baseline —
five invocations in all. The full list with its caveats is in "Handover" below;
use that one.** `check.sh` compiles every converted module
and every converted header standalone, and since C1 also runs `uic`, `moc` and
`rcc` over the converted forms. **It also RUNS SIGEL**: **twenty-three
`guidrive` invocations over sixteen distinct scenarios**, twenty-two where no
comma-decimal locale is installed — the eleven of
`gui behaviour`, plus **two locale re-runs of `pages`**, `clipcheck`, `formsize`,
`slavegui`, `xtest`, `pagesave` twice, and **`pagesave` four more times in
`v2 round trip vs 1.3`**, which chains two saves each over two experiments.
*These numbers have been wrong against this sentence's OWN list at every
revision so far: the list once gave 14 distinct and 15 invocations while the
prose said 12/13 then 13/14; the two locale re-runs went uncounted; then
`xtest` was added 2026-09-07 and left out of both the count and the list.
Re-counted from `check.sh` 2026-09-18, site by site; `openfocus` was the one
missing that time. **Do not trust the line
numbers in this sentence; re-derive them.** They were `1716` for the last site
when first written and are `1898` now, because two later edits in the same
session inserted lines above it — a citation into a file that is still being
edited goes stale within the hour. Find them with
`command grep -n 'timeout 300 "$ROOT/build/guidrive"' checks/check.sh`.
Count them again; do not increment.*  — the slave's
headless smoke test, the `widgets` and `parsers` probes and
`expstruct --selfcheck`. *"Ten" counted only the `gui behaviour` list and stood
while three more sections were added around it.* *This said "it
executes no SIGEL code", which was true only between D27 deleting the shim
self-check and C9 producing a runnable binary.*
Vendored headers are `-isystem`, so their warnings do not bury the ones in our
own code. *The current figure is on the `total:` line; do not quote one from
here.*

**Warnings count.** They were not read up to A8, and the `Qt::endl`-on-
`std::cerr` regression in A3 was reported by this very command at the step that
introduced it, then shipped as "0 errors".

**`check.sh` covers Phase C as of C1.** Its module list is the 9 core modules
plus `SIGEL_Visualisation` (C5) and `SIGEL_CommonGUI` (C3) — a GUI module joins
only when every file in it compiles. *When this was written, `sigel.cpp`, `sigel_slave.cpp` and three GUI modules
were checked by nothing.* **All five GUI modules are in `MODULES` as of C7, and
C8 added the `programs` section for the two `.cpp` files.** What C1 added is a
`forms (Phase C)` section listing the forms converted so far and, per form,
running `uic`, compiling the generated header standalone, compiling the
committed base class, running `moc` over it, and checking the `.qrc` against
the header in both directions. Extending it further is part of each step, not
an afterthought.
The gates' own programs are still in the hole: `sigel_eval.cpp`, `pvm_smoke.c`
and `pvm_link.cpp` are compiled only by their own targets, never by `check.sh`.
A break in them shows up as a build failure rather than a check failure.

### Phase P — PVM — DONE 2026-08-28

Upstream PVM 3.4.6 replaces the vendored 3.4.3. `libpvm3.a` and `pvmd3` build
and run, SIGEL's PVM code links against them, and the evolution loop RAN on
2026-09-02 — three generations through the GUI against the oracle's 1.3,
single-slave, at 185 s/generation. C11 has the result.

```
make pvm         libpvm3.a, pvmd3         28 objects, 0 errors, 6 warnings
make B=build-asan pvm-link   build-asan/pvm_link   P4's link and round trip
./checks/pvm-check.sh   starts a daemon, runs both, PASS/FAIL, non-zero if either fails
```

#### P1 — the tree

`downloads/supportingLibs/pvm3/` is upstream 3.4.6, 844 files where
3.4.3 had 576. `vendor/pvm3.4.6.tgz` is committed, md5
`7b5f0c80ea50b6b4b10b6128e197747b`, identical to netlib's and to Debian's
`.orig`. **It is tracked here**: netlib is the only host still
publishing it, Fedora retired PVM in 2015 and Debian removed it in 2024.
**3.4.3 could not have been patched instead: it has no `conf/LINUX64.def`**, and
neither version contains any `aarch64` in `lib/pvmgetarch`. **Nothing of SIGEL's
was discarded** — the two 3.4.3 tarballs differ from each other in one genuine
line, a clean rule in `Makefile.aimk`, the rest being CVS keyword expansion, and
no patch of ours touched PVM before this phase. *That compares the two TARBALLS;
the untracked directory an `rm -rf` destroyed cannot be compared against, so this
is provenance, not a byte-for-byte proof.* *For 1.3 the supporting libraries were
a separate download; only the 1.0 distribution bundled them.*

**The Makefile guards the swap** (`PVM_GUARD`). Re-extracting
the full supportingLibs archive over the tree restores 3.4.3 with no error and a
plausible file count, because `tar` overwrites but never deletes. **The guard
reads `PVM_VER` from `include/pvm3.h` on every `make`, not from the patch
stamp**, which survives exactly that accident; `clean` and `unpatch` still run
when it trips, because those two are how you recover.

#### P2 — nine patches

The plan said one Debian patch. Measured against a pristine 3.4.6: the `series`
has 29 entries, 26 apply, 10 touch `.c`/`.h`, and **8 touch a file in the 28
objects** that build `libpvm3.a` and `pvmd3`. **All 8 are applied, signed off
2026-08-28**, on the rule that a maintained distribution's judgement beats ours
on a package nobody maintains. The ninth patch carries the four config lines.
The rule is "everything Debian applies that we compile", so the last column is a
record, not the reason any of them is there. Line numbers are **pre-patch**.


| `vendor/patches/` file | Debian | Fixes something here? |
|---|---|---|
| `pvm3-linux64-aarch64-tirpc` | ours | n/a — this is the port |
| `pvm3-debian24-ddpro-unistd-include` | 24 | **yes.** `ddpro.c:1509` casts `getcwd`'s implicit `int` back to a pointer; on 64-bit the address is cut in half |
| `pvm3-debian06-ctime-r` | 06 | **yes.** `pvmd.c:1738` passes `ctime()`'s result to `pvmlogprintf` as a format string |
| `pvm3-debian22-exec-path-sized` | 22 | **yes.** `pvmd.c:3855` builds an exec path in a fixed `char path[MAXPATHLEN]` with `strcpy`/`strcat` |
| `pvm3-debian09-pvmnametag-prototype` | 09 | no. `-std=gnu17` keeps `char *pvmnametag();` legal, and Debian patched only `lpvmgen.c:683` of eight such declarations |
| `pvm3-debian17-global-h-pvmtev-include` | 17 | no. All 28 objects compile without it |
| `pvm3-debian23-auth-file-o-rdwr` | 23 | no. The descriptor at `lpvm.c:3120` is only read and closed; the write at `:3181` goes to a different one opened `O_WRONLY`. Debian's reason is that `O_TRUNC` without write access is undefined in POSIX |
| `pvm3-debian16-default-pvmroot` | 16 | no — the hunk is inside `#ifdef PVM_DEFAULT_ROOT` |
| `pvm3-debian20-kfreebsd-ifflags` | 20 | no — the hunk is inside `#ifdef __FreeBSD_kernel__` |

`03-new-readline-api` and `25-format-security` touch only `console/`, `tracer/`,
`hoster/` and `src/OS2/`, neither in the 28. **All 28 objects compile with all
nine applied: 0 errors**, and `lib/pvmgetarch` answers `LINUX64`, the single
fact the config patch exists for. **`make unpatch` now reverses the list** —
applying order undid `06` before `22`, both touching `pvmd.c`, and `22` came off
with an offset and a `.orig` backup; the round trip is byte-identical to the
tarball.

#### P3 — build and run

`make pvm` drives PVM's own build with target `s` (`src` only); SIGEL's 2003
link line names `-lpvm3` and nothing else. **PVM is the one vendored library we
do not compile ourselves**: `pvmd3` is a daemon `libpvm3` locates under
`$PVM_ROOT/lib/$PVM_ARCH`, so the products must sit in that layout inside the
vendored tree rather than in `build/`, and `make clean` names them or they
survive it. `LINUX64` is hardcoded because a target name expands before
`vendor/patches/` is applied, and **the recipe tests for the products
explicitly** — make does not check that a recipe made its targets, so without
that `make pvm` would exit 0 having produced nothing. 439 KB `libpvm3.a`, 235 KB
`pvmd3`, all 13 `pvm_*` symbols the built core leaves undefined defined.

`./checks/pvm-check.sh` runs `pvm_smoke.c` against a live daemon: double, int and
string exact. **The round trip really goes through XDR** — `pvm_send` to one's
own tid does not short-circuit, measured with
`-Wl,--wrap=xdr_double,--wrap=xdr_int` at 2 and 30 calls. **And it fails when it
should:** a `--wrap` swallowing `pvm_send` gives `nothing arrived in 10 s` and
exit 1 rather than hanging, because it uses `pvm_trecv`; a `--wrap` adding 1.0
to `xdr_double` on decode gives `DIFFERS`.

**`-ltirpc` is required.** `libpvm3.a` leaves 9 XDR symbols undefined because
glibc dropped `rpc/types.h`, and the plain C link of `pvm_smoke.c` fails without
it with 50 undefined references. **P4's sanitized C++ link does NOT fail** —
see P4.

**DEFECT, not fixed: `PVM_TMP` longer than 92 characters kills the daemon.**

```
pvmd.c:5066   (void)PVMTMPNAMFUN(spath);      /* char spath[PVMTMPNAMLEN=128] */
pvmd.c:5067   strcpy(uns.sun_path, spath);    /* sockaddr_un.sun_path[108]    */
```

`pvmtmpnam` (`pvmcruft.c:760`) builds `"$PVM_TMP/pvmtmp%06d.%d"` and `sun_path`
holds 107 characters plus the NUL. Measured: 92 starts, 93 aborts with
`*** buffer overflow detected ***` before the daemon prints anything — that is
the 6-digit-pid, 1-digit-counter case. **`pvm-check.sh` refuses at 87, and
refused at 88 until 2026-09-05.** Two things make the worst case one character
shorter than the old guard allowed: the counter is `n = 1 + (f++ % 10000)`
assigned **after** the `sprintf` that uses it, so it runs 0,1,…,10000 and reaches
**five** digits, not four; and `%06d` pads without truncating, so a pid above
999999 — `pid_max` is 4194304 here — contributes seven. `107 − 7 − 7 − 1 − 5 =
87`. *The old guard admitted exactly one length that could still abort the
daemon. Found by review.* The same `strcpy` is at `lpvm.c:1249` and `:2465` on
the task side. **None of the 26 applicable Debian patches fixes it.** Left alone
because it needs a source edit nobody upstream has made and the default
`/tmp/pvm-sigel-<uid>` is 20 characters. *A first diagnosis blamed a `sprintf`
gcc does warn about, at a limit near 110 — nothing reaches that line, because
108 is the lower ceiling, and the guard built from it measured a different and
shorter path, so every `PVM_TMP` from 93 to 109 passed it and killed the daemon
anyway.*

#### P4 — SIGEL against real PVM

**Linked: all 13 symbols.** `SIG_GPFitnessTrainer.o` and `SIG_GPPVMData.o` are
named on the link line rather than left to the archive. Those two are the whole
of it, verified with `nm` over all 14 archives and all 284 objects, and **the
recipe asserts it** so a core file that later gains a `pvm_*` call fails the
build by name instead of quietly dropping out of coverage — broken deliberately
with a probe and confirmed to fire.

**Run: 7 of the 13, through SIGEL's own code** — `pvm_initsend`, `pvm_pkint`,
`pvm_pkstr`, `pvm_send`, `pvm_recv`, `pvm_upkint`, `pvm_upkstr`, exercised by
`pvm_link.cpp` round-tripping a `QString` through `SIG_GPPVMData`'s
`sendQStringToPVM` / `getQStringFromPVM`. The other six —`pvm_addhosts`,
`pvm_delhosts`, `pvm_kill`, `pvm_probe`, `pvm_spawn`, `pvm_upkdouble` — are
`SIG_GPFitnessTrainer`'s spawn-and-manage calls and are **link-checked only**.
*Dropping `libpvm3.a` gives 28 undefined references; do not read that against the
13, because 11 are `pvm_link.cpp`'s own calls, across 17 distinct symbols.*

**TRAP — why it runs and does not merely link without `-ltirpc`.** `libasan.so`
exports weak `xdr_double`, `xdr_int`, `xdrmem_create` and the rest as
interceptors, so under the sanitizers PVM's XDR references bind to those and
**the link succeeds with nothing behind them**. glibc keeps the same names only
as compat symbols (`xdr_double@GLIBC_2.17`) that `ld` will not bind. Built
without `-ltirpc` and run: `SEGV on unknown address 0x0` in `enc_xdr_init`.

**Cost of running SIGEL's code: one suppressed leak.** `SIG_Environment`'s
default constructor loads terrain through vendored DynaMechs and leaks 20,400
bytes in 51 allocations — pre-existing, unrelated to PVM. `pvm-check.sh`
suppresses that one function by name rather than turning leak detection off, and
prints what it suppressed.

**FIXED — `sendQStringToPVM` overflowed on multi-byte strings.** The wire length
was a **character** count and the payload was UTF-8 bytes. *The only direct
observation of the `+ 2` margin this depends on came from the pvmd log —
`declared 10933, received 10931` for a 10,931-byte payload — and it is not
reproducible from anything committed. D21 claimed `pvm-check.sh` verifies the
`+ 2`; it does not, and with `+ 0` the check still passes because `QList`
over-allocates.* **Upstream's defect,
not a port regression**: `v1.3-pristine` has it identically, because Qt 2's
`QString::length()` was the Latin-1 byte count. Full entry in §9's "Defects
fixed rather than preserved"; `pvm_link.cpp` round-trips 200 `ü` plus 50 `€` as
the regression test.

#### The flag choice, which must not be simplified

```
conf/LINUX64.def   ARCHCFLAGS += -I/usr/include/tirpc -std=gnu17 \
                                 -Wno-implicit-function-declaration -Wno-implicit-int
                   ARCHDLIB   = -ltirpc
                   ARCHLIB    = -ltirpc
lib/pvmgetarch     Linux,aarch64 )  ARCH=LINUX64 ;;
```

`getcwd` is used without its header, so its return truncates to `int` on a
64-bit machine. **`-fpermissive` would demote `int-conversion`,
`incompatible-pointer-types` and `return-mismatch`** — the gcc-14 error classes
that catch exactly that. The two targeted `-Wno-` flags silence only the K&R-era
classes and leave truncation a hard error. **They are not interchangeable.**

---


### Handover — one owner at a time

**2026-09-21 — IN PROGRESS: ITEMS 14 AND 13. THE GERMAN NAMES AND OUTPUT TEXT
IN `sigel/` BECOME ENGLISH.**
Start here.

- **One name or one output text per step.** Each step has a clean build with no
  new compiler warning, the five gates green, no baseline change, and an
  independent review. Behaviour does not change, apart from the translated text.
- **Out of scope, by decision:** the history text saved in `.exp` files —
  `Fitness (Elter 1)`, `(Elter 2)`, `CREATED NEW INDIVIDUUM` and `INDIVIDUUM IS
  GENERATED RANDOMLY` — and the authors' names and nicknames in the credits.
- **Translated:**
  - `SIG_GPManager::run() wurde mehr als einmal aufgerufen!` →
    `SIG_GPManager::run() was called more than once!`, in both `run` variants.
- **Renamed:**
  - `SIG_GPManager::schlussJetzt` → `stopEvolutionNow`. `messageEvolutionStop()`
    sets it; each loop reads it straight after `stopIfNecessary()` and returns.
  - `liesdas` → `sourceText`, the constructor parameter of `SIG_Scanner`,
    `SIG_RobotScanner` and `SIG_UnstreamerScanner`: the whole text to scan.
  - `dichte` → `density`, a local in `SIG_RobotCompiler::nextIsMaterial`: the
    number after the `density` keyword of a material in a robot file.
  - `konstante` → `frictionConstant`, in the same function: the number in
    `friction <number> on <material>;`.
  - `anderes_material` → `otherMaterialName`, in the same function: the name
    after `on` in that line.
  - `rot`, `gruen`, `blau` → `red`, `green`, `blue`, in the same function: the
    three numbers of `colour red <r> green <g> blue <b>;`. Here `rot` means
    "red"; the `rot` that means rotation, in other files, stays.
- **`check.sh` passes 1098, not 1097:** `README.md` added one tracked file.

**2026-09-21 — DONE: ITEM 49. THE EXPERIMENTS RUN UNTIL 1 JANUARY 2030, 12:00.**

- **All 7 experiments stopped by a date in August or September 2001**, so Start
  ended at once and a user had to change the date first. Their
  `TERMINATIONTIME` is now 2030-01-01 12:00; nothing else in them changed.
  The x86 oracle's copies stay as shipped.
- **What moved with it, regenerated from a run:** the four date fields and the
  parameter export's checksum in `guibehaviour-baseline.txt`, the date lines of
  both saves in `pagesave-baseline.txt`, and the `expstruct` checksum in
  `check.sh`'s `v2 round trip` section.
- **`guidrive`'s `evolution` and `pvmcrash` scenarios** relied on the past date
  to end a run at once. Without `SIGEL_GENERATIONS` or `SIGEL_RUN_LONGER=1` they
  now refuse to Start while the loaded experiment's end date is still to come,
  since that run would last until then. A 1.3 original with its 2001 date still
  starts and ends at once.
- **The gate-names item on the 2026-09-20 open list is dropped** by decision.

**2026-09-21 — DONE: THE D18 CURSOR MODEL IS GONE FROM THE SELF-CHECK.**

- **`Qt2CursorList` and its test are removed from `sigel_eval.cpp`**, 149 lines,
  by decision: the conversion is done and proven. D18's rewrite of
  `SIG_GPFitnessTrainer::sweepToSpawn` was proven by its review's run against
  the shim: 28,672 walks, 0 divergences. When the shim went, D27 put this model
  in its place. But the self-check ran the model against its own copy of the
  walk, never `sweepToSpawn`, so it guarded nothing: a change to the real walk
  passed every gate. The proof stays recorded in the D18 row of the steps table
  and in D27.
- **Five gates green** afterwards; `-selfcheck` passes on both builds, with no
  new compiler warning.

**2026-09-21 — DONE: ITEM 39 FINISHED, AND `sigelLauncher` MODERNISED.**

- **No 2003 author home folder and no 2003 machine name is left in any tracked
  file.** Home folders became `/home/user/…`: in `sigel/README`,
  `sigelDynClient`, the v6 capture's method note, `PORTING.md`, and inside the 20
  `robots/*/*.blend`. The 2003 machine names became `hostNN`, one name per
  machine, in the v8 capture and `PORTING.md`; the capture says so.
- **The `.blend` files were edited byte for byte.** Each holds the path once, in
  a zero-padded text field of Blender's file dialog. The new path and zeros
  were written over it: size unchanged, 15 to 23 bytes changed per file, all
  inside that field, carriage-return counts unchanged. Blender 2.11 and 2.14
  files. Not opened in Blender — none is available here.
- **`vendor/supportingLibs.tar.gz` is cut down to the five libraries the build
  uses**, by decision. The full archive held 27 generated Qt makefiles and an
  old SPARC binary naming 2003 home folders, all in its unused `qt/`, and every
  entry was owned by an author's login. The new one keeps `cv97`, `dynamechs`,
  `Dynamo`, `fparser` and `newmat09` byte for byte, with the owner reset: 525
  entries and 653 KB, against 2843 and 6.7 MB. Its PVM 3.4.3 went too — the
  build takes PVM from `pvm3.4.6.tgz`. The full archive is on SourceForge and in
  git history. Checked by unpacking it fresh, rebuilding from clean, and running
  the five gates.
- **`sigel/kdesigel.doxygen`** named an author's login in three `/tmp/…` paths;
  they read `/tmp/user/…` now.
- **Kept, by decision:** the project's contact email in `sigel/README` and the
  authors' credits — this is open source. **Git history is not rewritten.**
- **`sigelLauncher`:** the dead Solaris branch, with its 2003 Qt path, and the
  dead `PVM_ROOT` default are gone; paths are quoted, and arguments pass as
  `"$@"`. `PVM_ROOT` now follows `SIGEL_ROOT`, so a cron start that edits only
  the `SIGEL_ROOT` line, as 1.3's README describes, finds PVM. Tested from
  `sigelApp/`, and cron-style from the home folder with an almost empty
  environment.
- **Not done:** `sigelDynClient` and `manage_dyn_slave` need a second machine —
  item 47.
- **Item 35, removing Windows support, moved to the back:** it touches the whole
  codebase. What `~/sigel-vs-state.md` held and item 35 lacked is now in item 35;
  the note is deleted.

**2026-09-21 — DONE: ITEM 48. SIGEL IS STARTED THE WAY EVERY USER STARTS IT.**

- **`make` fills `sigelApp/`**, and a user runs `cd sigelApp && ./sigelLauncher`,
  as 1.3's README describes. The folder holds the two programs, the three
  scripts, `Terrain.ter`, `stdConf.mt`, `pixmaps/`, `textures/`, and
  `supportingLibs/pvm3`, a link to the built PVM. The data are copies, because
  SIGEL rewrites `Terrain.ter` on every evaluation and writes `movie/` where it
  runs. `make clean` leaves the folder alone, but removes PVM's build, so run
  `make` again before starting SIGEL.
- **1.3 had no separate folder.** Its autotools `make` built the programs in the
  source folder, beside the data, and the README told users to delete the rest
  by hand. A separate folder was chosen instead, and its name.
- **`build-fast` is gone.** Plain `make` builds without the sanitizers, as a user
  expects. The sanitized build is `make B=build-asan`, for the two gates that
  need it; the Makefile switches the sanitizers on by that folder name.
- **A fresh clone builds with plain `make`.** SIGEL's objects and the moc
  objects now wait for the vendor patches and the generated form headers.
  Measured: a fresh checkout, the libraries unpacked with `vendor/README.md`'s
  commands, plain `make`, the four build lines of §7, then all five gates
  green.
- **The checks run SIGEL from `sigelApp/`**, not from `sigel/`, so no gate
  rewrites the tracked `Terrain.ter` any more. Measured: a full gate run changed
  no tracked file. `check.sh` generates the forms into a scratch folder of its
  own, because regenerating `build/ui` would leave its own programs out of date
  halfway through the run.
- **Checked on the desktop 2026-09-21**, SIGEL started by `sigelLauncher`
  alone: evolution runs with the experiments' `. 1 1 "."`, and Visualize works.
  SIGEL's working folder is `sigelApp/`, where `robots/<robot>/` does not exist,
  so the slave does not open the experiments' `Body` directories.
- **Found on the way:** `sigelDynClient` runs `manage_dyn_slave`, which the port
  does not build, from a fourth 2003 home directory. Items 39 and 47.
- **D43 is parked on branch `d43-unfinished`**, pushed, with its 12 review
  findings open.
- **Item 39 is not carried out on the x86 machine**, by decision: its
  experiment files stay exactly as shipped. They are not published, its prepare
  script replaces the host block before every run, and no check compares the
  two machines' files.

**2026-09-21 — DONE: `x/` IS NOW `downloads/`.**

- **Renamed, by decision.** `x/` held two unpacked archives under a name that
  said nothing. It is now `downloads/`, still untracked, with the same 4399
  files. The libraries moved up one level: `x/supportingLibs/supportingLibs/` is
  now `downloads/supportingLibs/`, because the archive is unpacked into
  `downloads/` itself rather than into a folder of the same name.
- **Changed with it:** the `Makefile`'s library path, `check.sh`, `pvm-check.sh`,
  `.gitignore`, and the unpack commands in `vendor/README.md`. Run in a scratch
  copy, those commands give the tree the old ones gave, one level up. That
  includes `qt`, `qhull` and `SOLID-2.0`, which the archive holds and the build
  does not use.
- **Backup:** `~/sigel-x-before-downloads-20260921.tar.gz`, checked by unpacking
  and diffing.
- **Both build folders were cleaned and rebuilt.** 354 dependency files in
  `build/` and `build-fast/` named `x/`, and the next vendor rebuild would have
  stopped on them. None do now.

**2026-09-21 — DONE: ITEM 39, THE EXPERIMENTS. THE 2003 HOSTS AND HOMES ARE OUT.**

- **Replaced in all 7 `.exp`**, by one script:

  | field | was | now |
  |---|---|---|
  | `PVMHOST` | 8 to 21 entries, 22 host names | one entry, `. 1 1 "."` |
  | `GRAVEYARDDIRECTORY`, `POOLIMAGEDIRECTORY` | a 2003 home directory | `/tmp` |
  | 20 `Body` directories | a 2003 home directory | `robots/<robot>/` |

- **`.` is PVM's own name for the local machine**: `nametohost()` in PVM's
  `src/host.c`, called from `tm_spawn()`. `localhost` does not work — measured,
  `PvmNoHost`.
- **`.` as the slave directory needed a code change, an exception to D33.** On
  Unix `SIG_GPFitnessTrainer` built `<dir>/sigel_slave` verbatim, so `.` gave
  `./sigel_slave`, measured `PvmNoFile`. Both spawn sites now pass a bare
  `sigel_slave` for `.`, and PVM searches its own path:
  `$HOME/pvm3/bin/$PVM_ARCH`, then `$PVM_ROOT/bin/$PVM_ARCH`, or `PVM_PATH` if
  set. The `_WINDOWS` branch already did this.
- **`sigel_slave` must be on that path, and nothing puts it there yet.**
  `sigelLauncher` links it, now for `LINUX64`, but it works only inside 1.3's
  one-folder layout, which the port's build does not produce — item 48. The
  desktop run used a link made by hand.
- **`/tmp`, not `.`, for the two output directories.** `SIG_GPParameter` shows
  them through `absolutePath()` and saves that back, so `.` became the saving
  machine's path. Both are off in all 7.
- **Checked on the desktop 2026-09-21**: loads without error, evolution
  runs, and a save after three generations keeps all three fields and adds only
  1.3's ten first-save keys.
- **Checks.** `markers`, `pvmhost` and the hammer robot-block hash in the
  `v2 round trip` section now pin the file in `experiments/`, not 1.3; with one
  host, host order is no longer tested. `v8-1.3-gp-blocks.txt` still holds
  the order of 1.3's 20 hosts for the file as shipped. Both baselines were
  regenerated from
  runs. The GUI dump replaces the repo's location with a token, because the GUI
  shows a slave directory as an absolute path. Five gates green, run on the
  committed content alone in a clean checkout.
- **Still open, item 39:** the x86 machine's copies, the 20 `.blend` files, two
  lines in `sigel/README`, the two 1.3 captures, older quotes in this file, and
  git history.

**2026-09-20 — DONE: THE REPO WAS AUDITED FILE BY FILE AND REORGANISED.**

- **All 14 judged.** Keepers: `hammerNiceWalkingFitness`,
- **Every folder and every root file was reviewed and signed off one at a
time**, for
  what the finished port no longer needs. Each removal was proved first: the
  copy that survives it was compared byte for byte before anything went.
- **Removed:** a stale `build/guidrive`, 198 MB by `ls`; two vendored patches
  nothing compiles, on the evening of 2026-09-19; three `shim` headers nothing
  includes; `tools/`, the Qt 4.8 container whose conversion is committed;
  `tiecheck.cpp`, which nothing ran; the vendored `qt`, `qhull` and `SOLID-2.0`
  extracts, with SOLID's include path; `xb/` and `kbin.tar.gz`, the 2003 binary;
  `kdesigelSources.1.3.tar.gz`, whose content is in git at `v1.3-pristine`; and
  64 files of 2003 build machinery from the source tree — the autotools build,
  the KDevelop project and its templates, and four upstream files holding
  nothing. The untracked deletions are not in git; the tracked ones are, in the
  commits of 2026-09-19 and 2026-09-20.
- **Kept, with the reason measured:** `experiments/` and `robots/`;
  `verification-against-sigel-1.3/`, because `v1-1.3-roundtrip.txt` holds 1.3's
  answer to the open `.rrb` question; the 1.0 release and its archive until the
  regression is assessed; the three Doxygen settings files, because nothing has
  ever run them and a check comes first; and the two launcher scripts, to be
  modernised in place — item 47.
- **The layout is now:** `checks/` holds the four gates and `replicate.sh`,
  `checks/baselines/` their six reference files, `checks/programs/` their five
  programs; `vendor/`
  holds the 12 patches and **both upstream archives, tracked**, so a clone
  builds with no download; `sigel/` is the 2003 source, hoisted out of three
  levels of nesting and renamed — upstream's "kdesigel" claimed a KDE dependency
  that never existed. The only tracked files left at the root are the
  `Makefile`, `.gitignore` and the three documents.
  `sigelSourceDistribution.1.0.tar.gz` still sits there, untracked.
- **Each gate refuses to run from a wrong repo root**, and runs from the repo
  root whatever the caller's directory is. Both guards came from review
  findings: a copy at the old path would have pointed `rm -rf $ROOT/build/ui` at
  the parent of the repo, and `guidrive` opens `robots/twoBases/twoBases.rrb`
  relative to the caller.
- **The encodings check lost its two counts.** They could not fail, and after
  the rename they measured nothing. The line-ending test, the floor and the
  one-bucket assertion stay.
- **Every commit had the five gates green and an independent review**, except
  `662bb14`, reviewed after it was pushed. The gate logs of each round are in
  the session scratchpad, not in the repo. **Three defects were found by review
  and fixed:**
  a `make -q` guard that reported a false failure from another directory, a
  clone setup command that could not work, and a self-counting command in
  `guidrive.cpp` that printed a plausible wrong answer. **Two mistakes of mine
  were caught and repaired:** a commit that swept in D43's uncommitted lines,
  and two commits that left a file at both its old and its new path.
- **`physics_backends.md` was folded into this file** and deleted, by decision:
  no new documents; this file is the only log of what was done, besides git.
- **Open, in order of priority:** item 39, the 2003 host names and
  the authors' home directories in the experiment files, now public on GitHub —
  to be done on both machines so they stay comparable, planned and approved
  first; the `.rrb` numbering, the one measured divergence from 1.3; the 1.0
  regression; and the names of the gates, which say what they run and not what
  they compare against.
- **Still uncommitted: D43.** The behaviour was confirmed on the desktop
  2026-09-18; the 2026-09-19 entry below records that its review left 12
  findings unacted. Untouched all day.

**2026-09-19 — DONE: EVERY EXPERIMENT WAS JUDGED BY EYE, 1.3 BESIDE THE PORT.**

  `insectNiceWalkingFitness`, `octopusNiceWalkingFitness`,
  `walkerNiceWalkingFitness`, `runnerNiceWalkingFitness`,
  `shortHammerNiceWalkingFitness`. The two-bases family: keep
  `twoBasesHardlyReducedIS`, to be renamed `twoBases`; the other five go — item
  40 in `future_refactorings.md`. `octopusSimpleFitness` goes too — item 44 — and
  `runnerSimpleFitness` — item 45. **Done on this machine 2026-09-19**, by
  decision: both data trees hold the 7 under their new names, and every check that
  loaded a dropped file now loads a kept one, against the port's own output.
  **Done on the x86 machine the same day**, with approval there: its seven
  files hash as ours, and its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.
  **Both machines end with the same set under the same names**, decided
  2026-09-19, so the two can still be compared. Each kept experiment is renamed
  to its robot's
  name — `hammer`, `insect`, `octopus`, `runner`, `shortHammer`, `twoBases`,
  `walker` — all seven approved — item 46.
- **The kept experiments are in git**, in `experiments/` and `robots/`, and every
  check reads them there. `main` is the working branch; the old `main` is tagged
  `old-main`.
- **The repo is on GitHub**: `origin` is
  `git@github.com:jayMcBee/sigel-evolutionary-walking-robots.git`, `main` and all
  13 tags pushed 2026-09-19. **The history was rewritten once, before that first
  push**, by decision: a build folder, `build-fp/` (761 files, 407 MB), had
  been committed by mistake on 2026-08-22 and deleted on 2026-08-27, and it made
  the push 104 MB instead of 4. It is gone from every commit; each of the 329
  commits was checked equal to its original apart from that folder, and 276
  hashes changed. The docs cite the new ones. The old history is in
  `~/sigel-repo-before-rewrite-20260919.bundle` (verified), the uncommitted D43
  edits of that moment in `~/sigel-d43-uncommitted-20260919.patch`.
- **The old copies are gone**, by decision, 2026-09-19: `data/`,
  `data-reordered/`, the two download archives, an older backup and the git
  bundle of the history before the rewrite. **The one backup is
  `~/sigel-data-both-20260919.tar.gz`**, sha256 `37006ffb…2035`: both data trees
  as they were before the clean-up — all 14 experiments as downloaded, the 14
  result archives, the robots in both orders — checked by unpacking and diffing.
  `dictorder-reorder.py` went too: its input and output are gone, and `robots/`
  holds its result. Git history keeps the script.
- ~~**Next: review every folder and file for what the port no longer needs**~~ —
  **DONE 2026-09-20**, see the entry above.
- **How each one was shown.** Score every individual with `build-fast/sigel_eval`
  (a scratch `SIGEL_ROOT` holding a copy of `Terrain.ter`). Make a viewing copy:
  swap the text between the `INDIVIDUAL(0)` and `INDIVIDUAL(k)` labels and swap
  their `POOLPOS`; check that individual 0 of the copy scores what k scored. Show
  it here with `DISPLAY=:0 QT_QPA_PLATFORM=xcb`, `SIGEL_ROOT` the source tree,
  `build-fast/sigel_slave -visualize <copy>`. Send `sigel-x86` the shipped file's
  sha256, k, and the copy's sha256; it builds the same bytes, repoints the `Body`
  lines and shows it in a nested display that can draw (`Xephyr :98` with
  `-glamor`, `LIBGL_ALWAYS_SOFTWARE=1`, a live `pvmd3`), camera 3 steps out, at
  time 0.
- **The x86 machine is the 1.3 reference and nothing more**; it keeps no
  records for this project.
- **Open:** the `.rrb` reordering choice (below, under 1.3's numbering);
  D43, uncommitted — both `SIGEL_GP` comments are done, 12 other review findings
  remain; items 40 to 46.

**2026-09-18 — THE EXPERIMENT DATA WAS DAMAGED, AND IT IS REPAIRED.**

**What happened.** Both untracked trees were written over while clearing the
2003 Dortmund PVM hosts out of the experiments (item 39 in
`future_refactorings.md`), and neither was copied first. `data-reordered/` was
the only copy of the tree the checks had proved. `data/` was then deleted.

**`data/` is the clean download again**, built from archives that are
byte-identical to `sigel.sourceforge.net` today: `experiments.tar.gz`,
`robots.tar.gz` and the 14 `*Experiment.tar.gz` from `seiten/ergebnisse_de.html`.
The two `runner*.exp` come from their archives, and each archive is also
extracted to `results/<name>/`. `results/endbericht.pdf` matches the live copy;
`endbericht.txt` is the local `pdftotext` output from before the loss. **The two
tarballs are git-ignored, not tracked.** Everything that reads `data/` works
unchanged: `check.sh` in three places, `replicate.sh` in two, and
`dictorder-reorder.py` as its source; `check.sh` and `dictorder-dump.sh` also
name it in messages. `check.sh`'s v2 round trip still reads the clean input that
1.3's reference was captured from.

**`data-reordered/` is the proved tree again**, and three measurements say so:

- `./checks/dictorder-dump.sh | diff -u dictorder-baseline.txt -` is **empty**, sampled
  twice. The script reorders 7 of 7 `.rrb` and 0 of 14 `.exp`, which is what D2
  recorded in August.
- `dictorder-reorder.py` and `dictorder-baseline.txt` **from `ff6db93`**, the
  commit that built the August tree, run on today's `data/`, give a tree
  byte-identical to today's `data-reordered/` in all 21 files.
- `diff -rq data data-reordered` lists the 7 `.rrb` and nothing else. Every
  `.rrb` is a pure permutation of whole blocks and point lines.

**The script defect, fixed.** `read_baseline` expected four fields; the dump has
printed five — `rrb      link       0 #0   base` — since `70087ae`, so no link,
joint, sensor or drive target was parsed, and the script reordered points only
without a word. It now refuses a target that does not name exactly the entities
the file declares, an `.exp` without a robot block, a file set that differs from
the baseline's, and a missing destination directory — and it writes nothing
until every file has passed. Tested by breaking the baseline or the tree nine
ways: each exits 1 and writes nothing.

**What the rebuild before this repair still carried, measured.** It restored the
12 `.exp` from `experiments.tar.gz` and nothing else: both `runner*.exp` and all
14 `results/*/*.exp` still carried the host edit, and that edit had also
rewritten 37 `Body … /home/user/…/ y` entries inside robot blocks. Its dump
differed from the baseline in 602 changed lines, 16 hunks, all in 6 `.rrb`
sections; no `.exp` section differed.
The three backups in `~` taken during the loss held that damaged state and were
deleted. **The one backup is `~/sigel-data-reordered-verified-20260919.tar.gz`**,
sha256 `b7077af6…e5a3`: `data-reordered/` after the repair, checked by extracting
it and diffing it against the tree.
Two check outputs from 2026-09-02, `x11b-gpp.gpp` and `x11b-ow.sip`, were
removed from `data-reordered/Experiments/`; `guidrive` writes to
`$SIGEL_SCRATCH` or `/tmp` now.

**What three independent reviews found**, one told to disprove the claim:

- **The 7 `.rrb` do not number like 1.3, and never did.** 185 of 212 stored
  numbers name a different entity — D2's choice, recorded below as open to
  reversal, and unchanged. 1.3's side is now measured; see the `.rrb` bullet
  below. The 14 `.exp` carry their own numbers and are not affected.
- **The dump does not print the order that numbers the DynaMechs bodies.**
  `SIG_DynaMechsSimulationData::initializeArticulation` numbers them by a
  depth-first walk over `SIG_Link::getJoints()`. That list is in the order the
  joints were attached: the stream's order for a robot read from an `.exp`, a
  copy or PVM, and declaration order for a `.rrb`. `drives` and `sensors` are
  indexed by stored number, and the containers' own order is not used.
  `sigel_eval.cpp`, `dictorder-dump.sh`, `dictorder-reorder.py`, "Dynamo removed,
  DynaMechs kept" below and D2 said container position becomes the body index;
  corrected. V5's note that V1 checks call order was wrong too: V1 checks
  container order.
- **1.3's real evolution rebuilds the robot once more than `sigel_eval` does**:
  `SIG_GPFitnessTrainer` copies it, and the slave re-reads it from the PVM
  stream. The slave receives that stream in file order (below), and its
  containers then hold `insect`, both `octopus` and `walker` in hash order.
  Derived from source; nothing in the simulation reads that order. Each link's joint order, which does reach the physics, comes out
  the same under file order and hash order for all 14 `.exp` — derived for all
  14, and measured on both machines for octopus and walker, below.

**What 1.3 itself says, asked through the oracle 2026-09-18.** Raw output in
`verification-against-sigel-1.3/`: `v1-1.3-roundtrip.txt` and
`v5-1.3-mdh-compared.txt`, each under a result dated 2026-09-18. Our half is
beside it in the same place.

- **Same inputs.** The oracle fetched the four archives fresh; each sha256 equals
  ours. The robot blocks of all 14 `.exp` hash the same on both machines.
- **A real 1.3 slave receives the robot in file order.** Captured at the
  master's `pvm_send`, tag 23: octopus 70 of 70 payloads, walker 37 of 37. The
  robot our master writes for a slave matches octopus in all 43 entities, in
  order and stored number.
- **The physics walk is the same.** `initializeJoint`: octopus 18 of 18 calls
  and walker 36 of 36 identical on 1.3 and ours, in name, number, calling link
  and order — walker's two legs whose calls come out of order included. Ours was
  captured before 1.3's arrived. Our `drives`, `sensors` and link tables for
  octopus: 28 entries, each the entity whose stored number in 1.3's payload is
  its index.
- **Load and save, all 14, robot block only.** 1.3's second save has the input's
  robot block for 14 of 14; its first differs for 8. Our robot block is a fixed
  point on all 14 and equals 1.3's second save byte for byte on 14 of 14. The 8
  are D3's known difference. Whole files differ, as V8 result 5 says: 1.3 adds
  ten keys on the first save.
- **Fitness per individual, 42 values**, the same individuals on both sides,
  bound by name. 1.3 repeats bit for bit. Bit-equal across the two machines:
  8 of 42, every one an exact 0. Where 1.3 gives several individuals one value,
  ours does too, on the same individuals. Nothing more is read from the numbers:
  x87 against aarch64.
- **Side by side by eye, 2026-09-19.** 1.3's simulation viewer beside
  ours, the same individual on both — the same bytes, checked by hash.
  `hammerNiceWalkingFitness`, individual 0: it walks on both and behaves alike
  on the port and on 1.3 — kept. The six two-bases experiments, each with its
  best individual by our
  scoring: they behave alike on both, 1.3 only slower on its machine;
  `twoBasesHardlyReducedIS` is kept and the other five dropped — item 40.
  `insectNiceWalkingFitness`, individual 56, score 0.611: works on both machines
  — kept. `octopusNiceWalkingFitness`, individual 13, score 0.515: alike on both
  — kept. Six individuals share that score — 13, 27, 45, 61, 82 and 84, six
  different programs by the oracle's check; 13 is the lowest number.
  `octopusSimpleFitness`, individual 4, score 0.841: an early or failed run of the
  same robot, so only the first octopus experiment is kept — item 44.
  `walkerNiceWalkingFitness`, individual 35, score 0.255: kept.
  `runnerNiceWalkingFitness`, individual 86, score 0.083: kept. 83 of 100 score
  exactly 0 here. The 2001 values stored in the file
  are 0.9195 for 59, 0 for 40 — number 86 among them — and 0.8569 for one; the
  oracle's 1.3 gives 0 for numbers 0, 1 and 2, as ours does.
  `runnerSimpleFitness`, individual 30, score 0.734: removed, the first runner
  experiment being the better one — item 45.
  `shortHammerNiceWalkingFitness`, individual 5, score 0.491: it moves — kept.
  87 of 100 score exactly 0 here. Number 5 is stored at 0 in the
  file, yet its own history has it bred in this run — crossovers, then a mutation
  at pool position 5 on 13 August 2001 — from ancestors stored at 0.490151.
- **The payload, octopus, all 259 lines.** The 182 Polygon lines are
  byte-identical. The 13 Geometry lines carry the same values; four differ in
  text only, at exact decimal ties in the 50th digit, where glibc rounds half
  to even and Qt 6 rounds half away from zero — the fourth family below. Of the
  64 other lines, 46 are identical; the rest differ in computed link frames and
  joint values, by at most 4.4e-16, in 7 more such ties, and in two `-0` tokens.
  Ties are common at precision 50; the fourth family below has the counts. No
  name, stored number or token count differs.
- **One difference found: `-0` on the PVM wire.** Our master formats as many −0.0
  values as 1.3's — 2 in octopus's payload, at the same two places, and 88 in walker's,
  counted only; read as sign bits before formatting — but Qt 6 prints them as
  `0` where 1.3 printed `-0`, so our slave reads +0 where 1.3's reads −0. On our
  machine the master's robot and the slave's rebuilt robot score the same, bit
  for bit, for individuals 0 to 2 of both experiments. Not observed on those 6;
  no other individual measured. See the `-0` row in §2.
- **How 1.3 numbers a robot built from a `.rrb`: by declaration index.**
  Measured through 1.3's own File > Import > Robot for hammer, octopus and
  walker, and the shipped experiments carry exactly that numbering: 212 of 212
  entities over all 7 models. Our port numbers by declaration position too.
  From the `.rrb` as shipped, in `data/`, 123 of 123 entities of those three
  robots carry 1.3's number; from D2's reordered `.rrb`, 12 of 123. **So D2's
  reordering costs 1.3's numbering for a new experiment built from a model.**
  It also changes each link's joint list for insect, octopus, runner and walker,
  because a `.rrb` attaches joints in declaration order.
- **What joint order a 1.3 slave gets, measured on octopus.** It depends on how
  often the experiment was saved: a fresh import and the same robot saved and
  reloaded hand the slave different joint orders, 16 payloads each. But in both,
  base's three joints come second leg, third leg, first leg, and that is the
  order DynaMechs numbers the legs' bodies in. D2's reordered `octopus.rrb`
  declares them in that order; the shipped one declares first, second, third.
  **So there is a choice, not taken here:** keep D2, and match 1.3's body order
  but not its numbering, 185 of 212 entities; or reverse D2, and match 1.3's
  numbering but not its body order for insect, octopus, runner and walker —
  measured on octopus, derived from the files for the other three.
  Numbering decides which joint `MOVE` and `SENSE` address. Body order decides
  the order in which DynaMechs numbers and processes the links. Neither touches
  the 14 shipped experiments, which carry their own numbers.
- **Not yet answered:** walker's payload line by line.

**The job that caused this is still open** — item 39 in `future_refactorings.md`,
clearing the 2003 Dortmund hosts, which are both a privacy concern and dead
logic. **Do not use one repeated placeholder name.** `check.sh`'s
`v2 round trip` section compares the *order* of 20 host names against the
oracle's capture in `v8-1.3-gp-blocks.txt`; identical names would leave that
check testing nothing. Distinct placeholders, mapped consistently across the
experiments, `pagesave-baseline.txt` and `check.sh`'s host-parser test. Item 39
records where that rule conflicts with another.

**Uncommitted at handover, and why.** D43 in four `SIGEL_GP` files, the
responsiveness probe in `guidrive.cpp`, and the matching text in this file and
`future_refactorings.md`. Reviewed, both controls measured, and confirmed on the
real desktop. It waited for this repair, because the checks that clear it
read the data tree. With it in the working tree and the repaired tree in place,
`./checks/check.sh` gives 1097 pass, 0 fail; checks 2 and 3 diff empty; check 4 exits 0;
`./checks/pvm-check.sh` passes both halves.

**Never two sessions on this repository at once.** Sequential is fine;
concurrent is not. On 2026-08-27 three concurrent sessions nearly corrupted a
reference capture.

This document is the handover. A new session should read §0, this section,
**§9's open list — which is ordered so the conversion work still to do comes
first** — and then the phase it is taking on.

**CHANGING ANY PARAMETER AFTER A RUN HAS STARTED IS USELESS AND IS FORBIDDEN.**
Decided 2026-09-07, and it settles a whole class of question before it is
asked: changing parameters while a run is going makes no sense, so it is not
tested — it is forbidden.

- **Whether 1.3 allows it is irrelevant.** Do not preserve it, do not test it,
  and do not compare it against 1.3. **What 1.3 actually does**, checked in the
  tarball rather than assumed: it greys 23 of them at run start via
  `evolutionRunningActionGroup` (`SIG_MainWindow.cpp:417-440`), greys neither
  the four MetaGP actions nor New/Open at all, and hands the 23 back on the
  first tree click because its `running()` was a stub returning false. So it
  tries and fails, which is a bug there, not a behaviour to carry over.
- It covers MetaGP settings, `New Experiment`, `Open Experiment`, and any other
  parameter route found later.
- **Find another route? Block it, add it to `runlock`, move on.** D30 is the
  pattern: a run check where the action is re-enabled, plus the action in
  `evolutionRunningActions`, plus a `runlock` case with a positive control.
- **Do not propose driving such behaviour to "measure it first".** There is
  nothing to learn from a behaviour that is going to be forbidden either way.

**SAMPLE TWICE AND COMPARE; NEVER SAMPLE ONCE AND INTERPRET.** Two probes
failed this way on 2026-09-06/07, and each was
*structurally incapable* of seeing the transition it existed to find:

- A `guidrive` scenario read `Configure System`'s enabled state **once, before
  a tree click**, and so reported the lock holding. The lock was applied
  and undone one line apart; a second sample is the whole finding.
- An empty `pgrep sigel_slave` and an empty daemon log were read as "nothing
  ran". Slaves live **0.2 s** and a working PVM dispatch logs **nothing**, so
  that is exactly what success looks like from one sample.

Both were repaired the same way: sample on both sides of the
event and print both. **An absence is not a measurement.** This is the same rule
as "every probe needs its own positive control", arrived at from the other
direction: the control proves the probe can see a positive, and the second sample
proves it can see a *change*.

**REBUILD EVERYTHING BEFORE RUNNING THE CHECKS, and name the targets.** Touching
one source file makes every program below stale, and the checks refuse a stale
binary rather than scoring it — correctly, but it costs a full run each time.
This cost three runs on 2026-09-05:

```
make                              sigel, sigel_slave and sigelApp/
make guidrive sigel_eval          the driver and the evaluator
make B=build-asan sigel_eval      the sanitized evaluator, for the fourth gate
make B=build-asan pvm-link        pvm-check's binary
```

**`make -q` with no target answers for `all`, which does NOT depend on
`guidrive` or on `sigel_eval`** — it reports up to date while both are stale.
Always name the target.

**`check.sh` no longer makes the other gates' programs stale.** It used to
regenerate the forms into `build/ui`, which left `sigel_eval` and `pvm_link` out
of date during its run and after it. It now generates them into a scratch folder
of its own. Run the gates in sequence all the same, in this order:

```
the four make lines above  →  ./checks/check.sh  →  ./checks/dictorder-dump.sh
  →  ./checks/fitness-check.sh
  →  ASAN_OPTIONS=detect_leaks=0 ./checks/fitness-check.sh build-asan
  →  ./checks/pvm-check.sh
```

**Do not run the gates while SIGEL runs from `sigelApp/`.** Both write
`sigelApp/Terrain.ter`, and `check.sh` rebuilds the `sigel_slave` a running
evolution spawns.

**And pipe check 3 without `2>&1`.** `fitness-check.sh` prints `selfcheck: ok` and
the two-line "no sanitizer, leak test SKIPPED" note on **stderr**, deliberately,
so that `| diff` sees only the fitness values. Folding stderr into the pipe puts
those three lines at the top of the diff and the check reads red with every
number identical. *Done here 2026-09-07; it is a mistake in the invocation, not
a regression.*

**AND DO NOT RUN ANYTHING ELSE HEAVY EITHER, which is a wider rule than the one
above.** A `check.sh` run on 2026-09-05 came back `gui behaviour 0 pass 1 fail`
with two markers from `roundtrip` — `!! menu [&File] did not open` then
`!! second export failed` — while tree-wide Python walks and greps were running
alongside it in the same session. **It did not reproduce**: `roundtrip`
standalone, then all ten scenarios in sequence diffed clean against
`guibehaviour-baseline.txt`, then a whole `check.sh` with nothing else running
gave 849 pass, 0 fail. The scenarios call `QTest::qWait` with fixed delays, so
they are wall-clock sensitive, and the driver reports a menu that did not open
as a scenario failure rather than retrying — correctly, since a real one must
not pass. **The cause is not established** and the correlation with load is one
observation, not a measurement. What follows from it is only this: a `!!` marker
is not automatically a regression, and the first thing to do with one is re-run
that scenario alone on an idle machine.

**The `sigel-x86` channel** reaches the machine holding the 1.3 reference binary
and a working PVM. **That PVM is no longer the only one** — this machine has had
one since Phase P — so the channel's value is now the 1.3 binary alone. It is currently owned by the Qt 6 session. If it transfers,
identify yourself to it by **verifiable facts** — repo path, recent commit
hashes, a reference file you authored — never by a session name, because names
are assigned per side and neither end sees the other's.

**A failed `make` leaves the previous binary in place.** All three scripts now
run `make -q` first and refuse a stale binary, because a test for existence
passes on one. D13 hit this: its build failed on two sites, the checks were run
straight after, and both came back green against the binary from before the
change. `pvm-check.sh` had carried the guard from the start; the other two did
not. **Check results mean nothing unless the build that produced them
succeeded.**

**Checks any session must keep green**, all committed:

```
./checks/check.sh                                        1097 pass, 0 fail, exit 0
./checks/dictorder-dump.sh | diff -u checks/baselines/dictorder-baseline.txt -   empty
./checks/fitness-check.sh  | diff -u checks/baselines/fitness-baseline.txt -     empty
ASAN_OPTIONS=detect_leaks=0 ./checks/fitness-check.sh build-asan   exit 0
./checks/pvm-check.sh                                    both PASS
```

**The fourth line is not optional and was missing from this list until
2026-08-29.** The default `fitness-check.sh` runs `build`, which has no
sanitizer, so it **skips the self-check's leak test** — and says so on stderr,
which `| diff` discards. The self-check is what covers the evolution loop's
ownership (D16), so without that fourth line a dropped free passes everything
here.

**The fifth line has no baseline** and is PASS/FAIL, which is why older copies of
this block listed four. It needs `make pvm && make B=build-asan pvm-link` first.

Never edit a baseline to make a diff go away. If a change moves one, that is the
finding.

**WHAT THE GATES READ, AND WHAT IS EVIDENCE.** Asked 2026-09-09 of the flat
root, answered by measurement; rewritten 2026-09-20 after the reorganisation. The six
baselines now live in `checks/baselines/`, and the gates that read them in
`checks/`. Line counts are `wc -l`.

*Check inputs. Deleting one fails loudly in every case, but only two say why:
the `pagesave` and `real clicks` sections name the missing file. The others fail
as a maximal diff — every line of the dump reported as an addition — which is
loud and uninformative.*

| file | lines | what it holds |
|---|---|---|
| `dictorder-baseline.txt` | 2071 | every container order that reaches the simulation, as `dictorder-dump.sh` prints it. Check 2 is a diff against this |
| `fitness-baseline.txt` | 21 | the 21 fitness values, three for each of the 7 experiments. Check 3 is a diff against this |
| `guibehaviour-baseline.txt` | 2613 | the `guidrive` scenarios under QTest |
| `guidump-baseline.txt` | 122 | the widget dump |
| `pagesave-baseline.txt` | 432 | the port's own save of `twoBases.exp` — see below |
| `xtest-baseline.txt` | 235 | what the port does under real X11 input — see below |

**`pagesave-baseline.txt` holds the port's own save of `twoBases.exp`.** Its
BASE block used to be 1.3's own output for `twoBasesSimpleFitness2.exp`,
captured from the 2003 i386 binary on 2026-09-03 — 192 lines, sha256
`a327150c…` — and our writer reproduced it byte for byte. That experiment was
dropped on 2026-09-19, and so was the rule that only the oracle may make this
file, decided the same day: once proven, the port's own output is the
baseline later work checks against. `check.sh`'s `pagesave` section guards it
and prints
*"this check tested NOTHING"* if it is absent.

**`xtest-baseline.txt` covers what QTest structurally cannot.** QTest's
**widget** overload — the only one `guidrive` uses — builds a `QMouseEvent` and
hands it to `QApplication::notify`, so it never passes through
`QWindowSystemInterface` — window activation, a popup's pointer grab and Qt's
synthesis of a double click from two presses are all invisible to it. This file
is `guidrive` driven by `xdotool` XTEST inside a nested Xvfb. Reproducible here,
unlike the one above, but it is the only reference for that class of behaviour.
`check.sh`'s `real clicks` section guards it and prints the same *"tested
NOTHING"* warning.

*Evidence, read by no script.* `regression_1.0_to_1.3.md`, 194 lines, holds the
deferred 1.0 → 1.3 regression and the oracle's diagnostics wishlist.
`future_refactorings.md` is the to-do list this file defers work into; no line
count, it moves every session. **`tiecheck.cpp` was the third and is gone**,
removed 2026-09-20: nothing ran it, and its figures are in §0 and C5.

**`portinglog.txt` WAS DELETED 2026-09-09, and this paragraph is what it held.**
It was the raw output of Qt 4.8's `qt3to4` from one run in the `tools/qtmig`
container, over one file, `SIG_GPParameter.cpp`, captured 2026-08-27 — container
paths, `/w/.q34tmp/…`. No script ever read it. It is recoverable from git at any commit up to and including `65c57f9` if the raw text is ever wanted; the two things it evidenced are here so
that it does not need to be:

- **17 of its 19 entries pull in Qt3Support.** `QListViewItem` →
  `Q3ListViewItem` nine times, `QListViewItemIterator` → `Q3ListViewItemIterator`
  five, `QFileDialog` → `Q3FileDialog` twice, and one include-path rewrite,
  `qfiledialog.h` → `q3filedialog.h`. The remaining two are not Qt3Support at all:
  `WFlags` → `Qt::WFlags` is a namespace qualification, and one added
  `#include <QPixmap>`. That is the measurement behind D25's rule that the tool's
  output cannot be kept as it stands.
- **It recorded no container substitution at all**, which is the evidence for
  D19a. The file was **`SIGEL_MasterGUI/SIG_GPParameter.cpp`** — say the module,
  because a second tracked file shares that basename and it is the wrong one — and
  the copy fed to the tool was the pristine 1.3 source, not this repo's. It held
  Qt 2 containers and the tool left every one alone. D19a carries the count.

**What it was not:** one file, one run — a sample of the tool's behaviour, not a
record of the conversion. It was never evidence about the tree, and the deletion
costs no check.

**THE FOURTH LINE WAS ALSO THE BLINDEST, until 2026-09-07.** `fitness-check.sh`
read each evaluation as

```
v=$("$ROOT/$B/sigel_eval" "$f" "$i" 2>/dev/null | tail -1 | awk '{print $3}')
```

which throws away both halves of the evidence: a pipeline's status is its
**last** command's, so `awk`'s 0 hid a `sigel_eval` that segfaulted, aborted or
was OOM-killed, and `2>/dev/null` discarded the stream a sanitizer reports on.
`dictorder-dump.sh`'s evaluation loop had closed this exact hole and says so
in its own
comment; this script never did. The first three check lines would still have
caught a *changed number* through the baseline diff — **but the fourth line is
checked by exit status alone**, so a UBSan `runtime error:` under
`ASAN_OPTIONS=detect_leaks=0 ./checks/fitness-check.sh build` went to `/dev/null` and
the check read green. `[ -n "$v" ]` was not a substitute: it only catches a crash
that printed *nothing*.

**Measured both ways, against a stub `sigel_eval` that exits 1 *and* prints
`SIG_Robot.cpp:41:12: runtime error: signed integer overflow` on every one of
the 42 runs:** the old script exits **0** and reports the word `individual` as
the fitness of all 42; the new one exits **1** at the first evaluation with
`hammerNiceWalkingFitness.exp 0: sigel_eval exited 1` and the sanitizer line
beneath it. It now captures to files, tests the status, greps stderr for
`AddressSanitizer|LeakSanitizer|runtime error:`, and only then reads the value —
the shape `dictorder-dump.sh` already used. *Found by review 2026-09-07.*

**`./checks/pvm-check.sh` is a fifth check but not a fifth check.** It has no baseline —
it prints PASS/FAIL and exits non-zero if either half fails. Needs
`make pvm && make B=build-asan pvm-link` first. Run it after touching PVM, `SIG_GPPVMData` or
`SIG_GPFitnessTrainer`; **the four lines above cannot see any of them.**

---

### Phase 0 — comments to English (D14)

German comments → English, removing the Latin-1 bytes. No code change.

Criterion: strip comments from before and after, diff the remainder — must be
byte-identical. Must not be interleaved with A1–A9, which are reviewable only
because they are pure renames.

**DONE 2026-09-05, in TWO passes, and the second is the one that finished it.**

**Pass 1 — 30 comment lines in 19 files.** Every edit was a substring
replacement in binary mode, so tabs, CRs and code are untouched.

**Pass 2 — a further 101 comment lines in 46 files, found by review.** Pass 1
was scoped from this document's own "15 lines across 10 GUI files", and that
figure came from counting lines with a byte above 127. **German without an
umlaut has no such byte**, so the method could not see most of the work. The
review that caught it swept every comment in the tree against an English
dictionary instead, and the count was not close.

**BOTH PASSES ARE VERIFIED BY D14'S CRITERION**, run over pass 1's 19 files and
pass 2's 46: strip comments from before and after, and the remainder is
byte-identical in every one, with CR and CRLF counts unchanged. **The criterion has its own
controls** — a one-token code change, a change inside a string literal and a CR
strip are all caught; a comment-only change is not.

**HOW PASS 2 WAS DONE, because the method is the point.** A comment-aware
rewriter: parse the file, apply the German→English word map ONLY inside comment
bodies, leave every other byte alone. D14's criterion is what makes that safe —
if the rewriter had reached into code, the stripped remainder would differ.
Three words are deliberately NOT in the map: **`Typ`, `Instruktion` and
`Selektion` are spellings of REAL member and method names**
(`MT_Substitute::Typ`, `getRandomInstruktion`, `setSelektionValue`), so a comment
naming one is correct and stays. That is the same rule that protects the port's
own historical notes.

**WHAT PASS 2 FOUND. 94 of the 101 lines are in CORE modules this phase had
called done since A1** — `SIGEL_GP` 18 files/51 lines, `MT_GPSystem` 10/18,
`MT_Control` 3/17, `SIGEL_Simulation` 4/4, `SIGEL_Robot` 3/3,
`SIGEL_Program` 1/1. Only 7 lines are in the GUI modules Phase 0 had left:

| where | what |
|---|---|
| `MT_Search.cpp` :100 :106 :113 | `// Mutiere Befehl!`, `// Mutiere Variable - 1.Operand`, `// Mutiere 2. Operand!` |
| `MT_Classifier.cpp` ×4 | `//MetaProgError := prozentualer Fehler` |
| `MT_FitnessTrainer.cpp`, `MT_Interpreter.cpp`, `MT_Trainingset.cpp` | `// weitere Methoden` |
| `SIG_CommandParameters.cpp`, `SIG_Geometry.cpp`, `SIG_Polygon.cpp` | `// FEHLER` |
| `SIG_GPRemoteZORCFitnessFunction.cpp` and its `WIN_` twin | `/* aktuelle Einstellungen lesen */`, `// Frame/Parity-Fehler ignorieren` |
| `MT_Substitute.h:128`, `SIG_AllIndividualsView.cpp, SIG_AllIndividualsView`, `SIG_GPParameter.cpp:628` | `sonst :=`, `Eventuell noch set AutoDelete enablen!!!`, `oder empty name` |
| `MT_GPManager.cpp` :414 :542, `MT_Interpreter.cpp` :59 :99, `MT_Tournament.cpp:8`, four `MT_GUI` sources | `macht mehr!!`, `Evolutionsschleife`, `Operanden`, `zweiter Operand`, `Konstruktion/Destruktion` |
| `SIG_DynaMechsCommandInterface.cpp:134` | `wat anderes kucken wa garnich an !` — Ruhr dialect, "we do not look at anything else at all" |
| **15 banner lines** in `MT_Classifier.cpp`, `MT_Substitute.cpp`, `SIG_GPManager.cpp` | `// NEU NEU NEU …` → `NEW` |
| **the rest: one German word inside an otherwise-English 2003 comment** | `programm`, `prozess`, `hierarchie`, `spezial`, `roboter`, `actuell`, `absolut`, `reproduktion`, `methode`, `Elter`, `Construktor`, `starte`, `simpel`, `zwischenziel`, `changeen`, `Laenge` |

**THE `SIG_GPOperations.cpp, reproduction` CARVE-OUT WAS WRONG ON ALL THREE OF ITS CLAIMS
AND HAS BEEN WITHDRAWN.** It said the line "is a string literal the program
prints, not a comment, and changing it would change output". The line is
`//SIGEL_Tools::SIG_IO::cerr << "\n--> Programm-Laenge:"` — the whole statement
is commented out, inside an `#ifdef SIG_DEBUG` that **nothing in this repo
defines**. So it is a comment, it is never printed, and changing it changes
nothing. It and the same shape at `SIG_Program.cpp, ~SIG_Program` are both translated.

**WHAT IS DELIBERATELY LEFT, and it is German:** `Sigel.dsw` and the three
`.mak` files under the source root carry CP437 German — `GELÖSCHT`,
`Ungültige Konfiguration`, `Sie können beim Ausführen`. They are **generated
MSVC 6 project files**, not comments and not built by anything here. Left as
found; recorded so the next reader does not have to rediscover them. The port's
own harness files (`guidrive.cpp`, `sigel_eval.cpp`) keep a `§` in
four places, but as **well-formed UTF-8** — the rule that came out of pass 1 is
that files under the SIGEL source root are ASCII-only, and the port's own files
may be UTF-8. What pass 1 removed from `sigel.cpp` and
`MT_AddConstantsWidget.cpp` was a **bare Latin-1 `0xa7`**, which is not valid
UTF-8 at all.

**Banner wording follows what the core pass used**, checked against
`MT_Control` and `MT_GPSystem` rather than invented: `interface for class X.`
in the header, `implementation of class X.` in the source.

**THE CRLF FILES WERE EDITED IN BINARY MODE** and the `encodings` check still
reported the same 25 known CRLF losses — no new one. *(Written before D31. There
are no CRLF files left and no baseline of 25; the check now expects zero CRLF
anywhere. The sentence is kept as the record of what pass 1 measured.)* **Its `translated` count
moved 35 → 44 over pass 1**, measured at both revisions, not the 43 → 44 this
section first claimed; that 43 was back-derived from 44−1 rather than measured,
and this file's own rule forbids exactly that. The count is of FILES that lost
their high bytes, nine of them, not lines. This was the task most likely to
produce a 3,227-line diff by accident; it produced none. See §2's encoding trap.

*Two smaller figures this section got wrong and the review corrected: "4 of the
15 were not German" is **1 of the 15** — only `MT_AddConstantsWidget.cpp`'s line
was in the list of ten, and `sigel.cpp`'s three were never in it, as the same
paragraph then admitted. And "8 German lines have no umlaut" is **9**, of which
**3** rather than 2 are in `SIG_SimulationVisualisationWidget.cpp`.*

### Doc comments naming a member by its Qt 2 type — DONE 2026-09-05

**26 lines in 14 files, not the 28 in 16 this section used to claim.** Every one
was checked against the member's ACTUAL declared type in the same file before
being touched, which is what the old figure was not:

| file | lines | was | is |
|---|---|---|---|
| `SIG_GPManager.h` | 5 | `QArray` | `tours` is `QList<SIG_GPTournament *>`, `taskCanDoList` is `QList<int>` |
| `SIG_GPParameter.h` | 4 | `QArray` | see below — `QStringList` |
| `MT_Trainingset.h` | 4 | `QArray` | `TCases` is `QList<MT_TrainingCase *>` |
| `MT_Statistics.h` | 2 | `QArray` | `TotalCrossoverEvent` is `QList<unsigned int>` |
| `SIG_VisualSceneObject.h` | 2 | `QArray` | both are `QList<GLdouble>` |
| `MT_Population.h`, `MT_StatisticsElement.h` | 1 each | `QArray` | `QList` |
| `SIG_GPPopulation.h`, `SIG_GPTournament.h`, `SIG_Register.h`, `SIG_EnvironmentRenderer.h`, `SIG_RobotRenderer.h`, `SIG_SceneObject.h` | 1 each | `QVector` | `QList` |
| `SIG_ExperimentListView.h` | 1 | `QDict` | `experimentDict` is `QHash<QString, SIG_GUIGPExperiment *>` |

**`SIG_GPParameter.h`'s four are the odd ones and did not become `QList`.** They
sit inside a commented-out doc block for `setFunctionSet` / `getFunctionSet` /
`setTerminalSet` / `getTerminalSet`, **none of which exists** — the class has no
such member or method. At `:160` and `:167` of the same file — some 300 lines ABOVE the four doc
lines at `:464`-`:491`, not eleven, as this said — the same 2003 author's
commented-out `private:` block declares `QStringList terminalSet;` and
`QStringList functionSet;`. So the `QArray` there was the ORIGINAL author's own
inconsistency, not something the port introduced, and `QStringList` is the only
spelling the file itself supports. The surrounding prose says "given as a
QString", which agrees with neither; that is left as found.

**FIVE NEAR-MISSES WERE DELIBERATELY NOT TOUCHED**, and they are why the count
came out lower. `MT_FitnessTrainer.h, MT_FitnessTrainer` and `MT_Trainingset.h, MT_Trainingset` say `QQueue`
of a parameter that really is `QQueue<MT_TrainingCase *> *`. `MT_Statistics.h`
:13, :25 and :48 say `QList` and the members really are `QList`. Naming a type
correctly is not a defect, and a sweep that matched Qt 2 spellings without
reading the declaration would have "fixed" all five.

**31 mentions of a dead Qt 2 container name remained in comments on 2026-09-05, all 31 the
port's own historical notes** — "at() was writable on Qt 2's const `QArray`;
`QList`'s is not", "Qt 2's `QDict` returned the NEWEST binding". The two quoted above stay.
`SIG_ProgramLine.cpp, randomRobotinstruction`'s `// QList<int> instr;` is commented-out code that
already names the Qt 6 type.

**No `QList`/`QHash`/`QQueue` mention was left pointing at the wrong container in
the other direction** — the five near-misses above were re-read against their
declarations, not assumed.

### Form minimums — DONE 2026-09-05

**Six forms declared a `minimumSize` below what Qt 6 needs to lay them out**, so
a user could drag one down until its children compressed. Raised to the per-axis
maximum of declared and measured, and **checked**, which is the half that was
missing: `GroupBox6` and `groupboxDirectory` had the same fix in C11d and were
checked by `slave gui`, but nothing measured a whole FORM.

**RE-MEASURED FIRST, and one of the six figures in this file was wrong.**
`MT_StatisticsWidgetBase`'s `minimumSizeHint` under Qt 6.10.2 is **402x555**,
not the 427x555 recorded here — a 25 px difference in width, on the form this
document called the worst case. The other five reproduced exactly.

| form | declared | hint | raised to |
|---|---|---|---|
| `MT_StatisticsWidgetBase` | 220x390 | **402x555** | 402x555 |
| `SIG_SimulationWidgetBase` | 780x640 | 373x752 | 780x752 — already wider than it needs |
| `MT_SelectionWidgetBase` | 410x240 | 472x301 | 472x301 |
| `MT_EstimationWidgetBase` | 230x260 | 323x274 | 323x274 |
| `MT_SearchWidgetBase` | 240x400 | 228x420 | 240x420 |
| `MT_IndividualsWidgetBase` | 440x362 | 338x404 | 440x404 |

**The check is a new `formsize` scenario in `guidrive` and a `form minimums`
section in `check.sh`.** It measures **all twenty forms**, not the six that were
wrong — a list of the known-bad ones passes the moment a twenty-first is added
or a good one regresses — and the scenario **asserts the corpus is 20** so a
shortened list cannot pass by testing nothing. Thirteen of the twenty declare no
`minimumSize` at all; those are reported as `unset`, not failed, because Qt then
uses the hint by itself and they are correct as they are. `MT_PopulationWidgetBase`
declares 350x220 against a hint of 330x121 and was already fine.

**Two independent teeth, because "TOO SMALL: 0" is not evidence on its own.**
The scenario's own selftest forces one form's minimum to 1x1 and requires the
same comparison to report it — without that, an invalid hint on every form would
read as a clean pass. And the section was measured the other way: putting
`MT_StatisticsWidgetBase` back to 220x390 and rebuilding makes `check.sh` fail
by name with `TOO SMALL: 1`, then restoring it passes again.

**THAT SECOND MEASUREMENT STOPPED BEING TRUE 34 MINUTES AFTER IT WAS TAKEN, and
this file went on asserting it for two days.** `0bdc4ee` (2026-09-05 16:15:49)
wrote the section as `if make -q ... && ... guidrive formsize ...; then`, with
the run **inside the `if` condition**, where `set -e` is exempt — so a failing
`formsize` set `mf=1`, printed, and the script carried on. That is the version
the teeth test above was measured against. `2527c0a` (2026-09-05 16:49:51),
*"Close four holes in the form-minimums check"*, restructured it into an `else`
branch with a bare run followed by `mrc=$?`. A bare command in an `else` branch
is **not** exempt: `check.sh` sets `-e`, so from that commit a real form-minimum
regression **killed the shell at that line** — no `form minimums` row and
nothing after it. A commit that closed four holes opened a fifth, in the one
place that could not report it.

Confirmed on the shell rather than reasoned:

```
$ dash -c 'set -e; f(){ return 3; }; if true; then f >/dev/null; rc=$?; echo REACHED; fi'
$ echo $?
3
```

`REACHED` is never printed. **Two** of the three branches went with it — `mrc =
124` and `mrc != 0`. The third, the `ngot != nui` form-count check, sits on the
`mrc=0` path and was live throughout: `guidrive` returns 0 only when it measured
20 forms, so that branch fires on a 21st `.ui` nobody added to the scenario's
table, which is what it is for. *This paragraph said "three"; corrected by
review.*

Fixed 2026-09-07 to `mrc=0` and `|| mrc=$?`, which puts the run back in a
context `set -e` exempts. **The teeth test was then re-run in full**, and it is
the original claim that is restored, not a new one — with `MT_StatisticsWidgetBase`
at 220x390, `check.sh` prints, verbatim:

```
      TOO SMALL: 1   (compared: 7, unset: 13, no hint: 0, of 20 forms)
      selftest OK -- the same comparison the loop uses reports it
form minimums           0 pass   1 fail
```

… **and then runs `slave gui`, `expstruct selfcheck`, `programs`, `gui vs 1.3`,
`gui behaviour` and `pagesave vs 1.3` after it.** The form was restored with
`git checkout`, the five targets rebuilt, and the check re-measured green.

**Was the regression unchecked for those two days? YES — within `./checks/check.sh` it
was.** `gui behaviour` moves under the same perturbation, to
`[window] class=MT_MainWindow title=[SIGEL MetaGP] 680x595`, which is the same
595 the oracle settled below (*"The MetaGP window grew 59 px"*) — so a second
section *can* see this defect. **But it never got the chance.** `form minimums`
prints at `check.sh:730` and `gui behaviour` at `check.sh:1280`: under the
`2527c0a` shape the shell died 550 lines before `gui behaviour` ran. The second
check only helps somebody running that scenario by hand. *An earlier version of
this paragraph claimed the opposite — that `gui behaviour` "would have caught
it" — which is exactly backwards for a failure that stops the script. Corrected
by review, which read the two line numbers.*

**FOUR DEFECTS IN THIS CHECK'S FIRST VERSION, all found by review, all fixed.**
The check as first written could not have caught the regression it exists for.

- **It passed on ZERO comparisons.** 13 of the 20 forms declare no minimum and
  take an early-out, so only **7** are ever compared. `tooSmall == 0` was
  satisfied equally by "every form is big enough" and by "no form reached the
  comparison" — so deleting the six `<minimumSize>` blocks, or breaking whatever
  carries them out of the `.ui`, made every form `unset` and the check returned 0
  with all six fixes gone. **`compared >= 7` is now asserted**, and `noHint != 0`
  now fails. *`check.sh:1502` learned this exact lesson for the forms corpus two
  commits earlier and the new check did not carry it over.* Teeth-tested: deleting
  one `<minimumSize>` block gives `compared: 6` and exit 1.
- **The "positive control" validated a COPY of the comparison.** The loop's test
  and the selftest's were two hand-typed expressions, so flipping the loop's `<`
  left the control passing. **Both now call one `classify()` lambda** — the shape
  `clipcheck` next door already used, which this had regressed from. Teeth-tested:
  flipping `<` to `>` now gives `NOT REPORTED` and exit 1.
- **The control ran on a form the loop never compares.** `forms[0]` is
  `MT_AddConstantsWidgetBase`, which declares no minimum and takes the `unset`
  branch — so the control exercised a branch its chosen form never reaches in the
  real loop. It now runs on the first form the loop actually **compared**.
- **The `unset` test was per-FORM where Qt's rule is per-AXIS.** `setMinimumSize`
  clears the explicit-minimum flag for whichever axis is 0, so a form declaring
  `240x0` would have been called TOO SMALL for a height Qt takes from the layout.
  No form is in that state today; closed before one is.

**And `check.sh`'s section reported three different failures as a fourth.**
Stale `guidrive`, a 300 s timeout kill and a crash all printed "a form declares a
minimum below what Qt 6 needs" — and on the stale path `/tmp/fmin.$$` was never
created, so that assertion arrived with an empty body. Four causes now have four
messages, stderr is kept, and `make -q` carries `-C "$ROOT"` because this script
never `cd`s. **The `.ui` files on disk are now the independent source for the
corpus size**: a hand list asserted against itself catches only shrinkage, so the
section compares the scenario's count against `find ui -name '*.ui' | wc -l`.
Teeth-tested both ways.

*Honest limit: `formsize` reads `minimumSize()` on the TOP-LEVEL widget only. The
two defects that started this family — `GroupBox6` at 90x37 and
`groupboxDirectory` at 465x37 — are on INNER group boxes and are invisible to it.
`slave gui` covers those two, so the combined coverage is real, but "all twenty
forms are measured" is true of the top-level widget and not of what is inside it.*

**Raising these RESTORES 1.3 rather than diverging from it.** The declared values
are 1.3's, readable under Qt 2's smaller default font; Qt 6's larger metrics are
what make the layout need more. Confirmed that the hint does not move when the
minimum is raised — the hint comes from the layout's children, so there is no
feedback loop and the new values are stable.

### D29's guard placement — CONFIRMED CORRECT BY THE ORACLE 2026-09-05

**The question was whether 1.3 refreshes the generations LCD on a page switch
DURING a run.** The guard in `SIG_ExperimentView::putIntoExperiment()` sits
below that read on the assumption that it does. **It does.** Measured on
`twoBasesSimpleFitness2`, whose file base is 532:

| when | LCD |
|---|---|
| pre-run | 532 |
| **7 passive captures over 3 generations** | 532, byte-identical every time |
| switch to Population and back, after gen 4 | **536** = 532 + 4 |
| 2 more generations, no switch | 536, unchanged |
| switch to Simulation-Parameters and back | **538** = 532 + 6 |

**Two independent cycles, and the value moves only when a page is switched, to
base + generations completed, exactly.** The seven passive captures are what rule
out a spontaneous refresh as the explanation — without them a frozen LCD and a
broken capture look the same. So C11 stays right that on 1.3 the counter does not
move by itself during a run, AND a mid-run page switch is one of `putIntoExperiment()`'s
updates. **A guard below the read is therefore correct: the read still has to
happen on a page switch.** No change needed. *Since D37 the port moves the counter
during a run by itself.*

### MetaGP needs `stdConf.mt`

`Configure System` needs a real `stdConf.mt` in `SIGEL_ROOT`. Only the 1.3
source carries one, and this repo has it at `sigel/stdConf.mt`, which is the
`SIGEL_ROOT` the checks use. *The tarball it came from,
`kdesigelSources.1.3.tar.gz`, was deleted on 2026-09-20; git holds the file.* In the GUI, without it,
`MT_Controller::readFromFile` shows "An error occurred in loading the meta
experiment".

### The MetaGP window grew 59 px — SETTLED BY THE ORACLE 2026-09-05

**`guibehaviour-baseline.txt` moves by one line, from 680x595 to 680x654, and
the oracle's measurement is why that is right rather than tolerated.**

**The cause here is measured by PREDICTION, not back-derivation.** The window's
chrome is **99 px** and the six MT_* pages sit in a `QStackedWidget` whose
minimum is the maximum over its pages. Four values, rebuilt each time:

| statistics min height | predicted window | measured |
|---|---|---|
| 555 | 654 | **680x654** |
| 500 | 599 | **680x599** |
| 450 | 549 → floor of 595 | **680x595** |
| 390 | 489 → floor of 595 | **680x595** |

So at the declared 390 the Statistics page received **595 − 99 = 496 px where its
Qt 6 layout needs 555**, at the DEFAULT size, not only when dragged.

**WHAT THE ORACLE MEASURED ON 1.3, and it corrects this document twice.**

- **1.3 does not open that window at 595 either. It opens at 680x605**, with a
  `WM_NORMAL_HINTS` program minimum of **625x605**. **Qt 2 already overrode
  `MT_MainWindow.cpp, MT_MainWindow`'s `resize(680, 595)` by 10 px to satisfy its own
  layout.** So 595 is a number the program asks for and has never got, and
  reasoning from that line describes the source rather than the application.
- **1.3's window cannot be made shorter at all** — 605 is both the opening
  height and the minimum; a window manager honouring the hint refuses the drag.
  Forced below it, **1.3's layout CLIPS rather than compresses**: the Search
  Operator Effects table is cut mid-row.
- **1.3's Statistics page is ALREADY CLIPPED at its own default.** Selecting
  Statistics adds a second toolbar row that eats ~30 px, and at 605 the group-box
  bottom border and the page frame bottom border are both cut. The oracle's
  ladder: 605 borders cut, 620 group border appears, 640 both visible,
  **654 fully laid out**.

**SO 654 IS WHERE 1.3'S OWN STATISTICS PAGE FIRST RENDERS COMPLETE**, and a port
that opens taller to honour its layout minimum is doing exactly what Qt 2 did —
the same override, from a larger `minimumSizeHint`. Matching 595 would reproduce
neither 1.3's size nor its behaviour. **The baseline was not moved on this
session's reasoning: 1.3 moved it first, by 10 px, for the same reason.**

**THIS FILE'S "the labels compress to 3-8 px tall" IS WITHDRAWN, on two counts.**
1.3's window cannot be dragged shorter than it opens, so the drag it describes is
not reachable; and the failure mode is **clipping, not compression**. The oracle
measured text row heights at two window heights and they are **identical**: 9 px
for plain labels, 12 px for ones with descenders, 25 px row pitch at both 605 and
780. What an undersized window costs is frame borders and whole cut rows, not
legibility. *The 3-8 px figure was never measured on either binary.*

### D30 — parameter changes during a run are forbidden — 2026-09-07

**A DELIBERATE DIVERGENCE FROM 1.3, and it is a decision rather than a finding.**
Changing parameters while a run is going makes no sense and is blocked;
whether 1.3 allows it is irrelevant — in 1.3 it is a bug, not behaviour to keep.
*An earlier version of this section said
"1.3 greys none of these during a run", which is **false** and was asserted with
no measurement behind it. 1.3 greys 23 of them at run start
(`evolutionRunningActionGroup`, `SIG_MainWindow.cpp:417-440` in the tarball); what
it does not do is KEEP them grey, because its `running()` stub always returned
false and the first tree click handed them back. It greys neither the four MetaGP
actions nor New/Open at all. Found by review.*
D29 had already begun this divergence for the same reason; D30 finishes it.

**THREE HOLES, all MEASURED by reverting each fix and re-running `runlock`:**

| after, in one cumulative run | `Add` | `Configure System` | `Use MetaGP` |
|---|---|---|---|
| a tree click | 0 | **1** | 0 |
| `slotEnableNoExperimentActions(true)` | **1** | 1 (already on, from the row above) | **1** |

*The two rows are ONE run, not two independent measurements, and the second row's
`Configure System` is left over from the first — `mtConfigureAction` is **not** in
`noExperimentActions` (its append is commented out), so that slot cannot have turned
it on. An earlier version of this table read as though it had. Found by review.*

**The route into the second hole was open too**, and nothing tested it: `File >
New Experiment` and `File > Open Experiment` were in no lock list at all. With
their appends reverted, `runlock` now reports `NewExperiment=1 OpenExperiment=1`
and fails. *Before 2026-09-07 that third of the change was checked by nothing —
reverting it left every check green.*

### D30a — the hole D30 missed: `Stop` unlocked everything mid-run

**D30 did not keep the lock, and this is why.** `SIG_GUIGPExperiment::slotStopEvolution`
opened with `emit signalEvolutionNotRunning( true )` as its **first statement**,
and it is a request to stop rather than a stop: it only sets
`guiGPManager->userTerminated` at the end, `start()` has not returned, and
`isRunning()` is still true. So one click
on `Stop` re-enabled all 29 locked actions **while the run continued** — for as
long as the manager takes to notice the flag, which is a whole generation, 58 to
208 s on this machine. None of D30's guards is consulted on that path.

**The fix is a deletion.** `slotEvolutionStopped()` already emits exactly that
signal, and it runs after `start()` returns. The premature emit is gone.

**Two more, found in the same review and fixed with it:**

- **The unlock was not exception-safe.** `slotEvolutionStopped()` sits *after*
  the call to `start()`, so a throw out of `start()` skipped it and left `Start`
  and the five pages disabled. Now called on the throw path as well.
- **D30 itself introduced a stuck state.** Its guard disabled
  `mtChoiceTypeActionGroup`, which nothing ever re-enables —
  `slotEnableEvolutionRunningActions( true )` walks actions, not groups — so
  Evaluator/Classifier stayed grey after the run ended. That line is removed; the
  group is not in `noExperimentActions` anyway (its append is commented out), so it
  was never needed.

**AND THE SECOND LAYER IS NOW IN, for three slots that had no check of their
own.** `slotMTUseMT`, `slotMTConfigureSystem` and `slotMTSwitchSystem` each
refuse when a run is going, on top of the greying. `slotMTUseMT` was the same
shape as the hole D30 fixed — `mtConfigureAction->setEnabled(state)` with no run
check — and was unreachable only because `useMeta()` returns false when the state
is unchanged, which is luck rather than a guard. *This is the layering that was
decided: keep the greying, add the refusal.*

**NOT CHECKED: the `Stop` fix.** `runlock` fakes a run by setting `evolutionRunning`
itself and never calls `slotStopEvolution`, which dereferences `guiGPManager` and would need a
real run. The fix is verified against source and by the review that found it, not
by a check. **Nor are the three slot refusals above**: `guidrive` never reaches
`slotMTUseMT`, `slotMTConfigureSystem` or `slotMTSwitchSystem` during a run, and
`runlock` checks only that `Use MetaGP` and `Configure System` are grey.

**Two slots re-enabled locked actions, neither with a run check.**
`SIG_MainWindow::slotCurrentExperimentChanged` runs on every change of the current tree
item, one line after the emit that applies the lock
(`SIG_ExperimentListView::slotSelectionChanged`), and, when MetaGP was on, turned
`Configure System` and the Evaluator/Classifier group back on.
`SIG_MainWindow::slotEnableNoExperimentActions` enables `noExperimentActions`:
30 active entries, 24 of them also in `evolutionRunningActions`.

**THE FIX, three places in `SIG_MainWindow.cpp`:**

1. `slotCurrentExperimentChanged` enables the two MetaGP controls only when
   `!experimentListView->isRunning()`.
2. `slotEnableNoExperimentActions` re-applies the run lock after its own loop,
   rather than filtering its list — so the two lists cannot drift apart.
3. `newExperimentAction` and `openExperimentAction` join
   `evolutionRunningActions`, which shuts the route as well as the symptom.

**THE TEST HAS TEETH, and it has a positive control.** `runlock` now switches
MetaGP on *before* the run and **asserts `Configure System` is enabled** — every
check after it asks whether a MetaGP action is OFF, and all four are off at rest
too, so without that control the block would pass on a window where MetaGP was
never enabled. It then checks `Use MetaGP`, `Configure System` and `Add` after a real
selection change and after the `New Experiment` route. *The tree-click check
also had to be repaired: it re-selected `topLevelItem(0)`, which emits nothing
when that item is already current, so it could have been inert.* Reverting the
fix makes it print `!! D30: a locked action came back during a run` and fail.

`guibehaviour-baseline.txt` moves by three lines, all in `runlock`, and nothing
else in the file changes.

### A real evolution under `guidrive`

#### The evolution works, and three claims about it are withdrawn

| withdrawn | what is actually true |
|---|---|
| "no `pvm_spawn` ever reaches the daemon" | it reaches it constantly — **235 slave executions in one 7-minute run** |
| "no `sigel_slave` process ever exists" | they exist for **~0.2 s each**, so a `pgrep` between spawns sees nothing. `sigel_eval` on the same individuals takes 0.20-0.37 s, and the oracle measures 1.3's slave lifetime at a 0.296 s median — 1.5x, not a mystery |
| "the daemon log records no task" | **a working dispatch logs nothing at all.** The oracle's log from a run that spawned 165 slaves is the same two startup lines as mine. It was never evidence |

*The reasoning that produced them was: `SIG_GPFitnessTrainer.cpp, spawnTask` prints on
the `pvm_spawn` failure path, nothing was printed, therefore it returned 1 —
which was right. What was wrong was concluding from an empty `pgrep` and an empty
daemon log that nothing ran.* `strace -e trace=write` had already shown the spawn
message reaching the daemon twice with the correct absolute path, so the trainer,
the host list and `getNextHost()` were never in doubt.

**HOW IT WAS MEASURED, and the method is the transferable part.** A shell script
named `sigel_slave` in the PVMHOST directory, the real binary moved alongside as
`sigel_slave.real`, logging one line per execution and `exec`ing the real one.
Positive control: running the wrapper by hand writes a line, so an empty log
would have meant no execution. **The oracle supplied the idea** by repointing a
working experiment's host directory at an empty one and reproducing this exact
symptom on demand — establishing that the PVMHOST line's **fourth field is the
path SIGEL builds `<dir>/sigel_slave` from**, and `pvm_spawn`'s own search path
never enters into it — except for a directory of `.`, which since item 39 spawns
a bare `sigel_slave` for PVM to find on its path.
*Field two is `maxSlaves`, read by the `SIG_GPPVMHost( QString )` constructor.*

A control run to completion, single slave:

```
  [artefact] POOLGENERATION=138 (was 136, +2 expected)  fitness values=100 of which zero=0
  [evolved saved] exists=1 size=1548652 path=/tmp/evolved.exp
```

and SIGEL's own `Computing Generation` timestamps, visible only because of the
flush fix below. **One slave: 208 s** for the single gap measured. **Four
slaves: 45, 59, 69, 55 and 64 s, mean 58** — which agrees with the 61.6
s/generation at four slaves already recorded in C11, and the single-slave figure
sits near the 185 s recorded there. It was never broken; it was slow, and a
90-second watchdog was killing it mid-generation and calling that a hang.

**`sigel_eval` and the PVM path agree exactly**, to all printed digits, for the
same individuals. Nothing is lost or corrupted in the spawn-transfer-harvest
round trip.

#### The harness

**THE TRAP THAT COST TWO DAYS.** `SIG_IO::cerr` and `::cout` are
**`QTextStream`s** over `stderr`/`stdout` (`SIG_IO.cpp:27-29`), and the 2003 code
ends every message with `"\n"`, never `endl` — so SIGEL's diagnostics sit in the
stream's buffer until it is destroyed at a normal exit. **Measured**: such a
stream survives a clean return and is lost entirely on a kill, while a plain
`fprintf` on the same descriptor survives both. **The `Computing Generation`
timestamps above depend on the flush.**

**PVM erased every exit status** whenever this process started the daemon
itself. `pvm_halt()` waits for a reply the daemon never sends, and the
daemon's `pvmbailout()` SIGTERMs every local task on its way out.
Demonstrated: a 25-second watchdog under a 90-second
`timeout` exited **143**, far too early to be the timeout. `main` now decides
the status first, **flushes before the teardown** — a flush after it never
runs, because the handler `_exit`s — and installs a handler that re-exits with
the status. Confirmed from both sides: a passing `evolution` run exits **0**
and a watchdog abort exits **3**.

**THE SAMPLER PROVES LESS THAN IT LOOKS.** `generations=136` never moving was
**expected** in that run: both live-update calls in `SIG_GUIGPManager.cpp` were
commented out. *Since D37 the call in `SIG_GUIGPManager::updateIndividualView`
is active.* *This said the only live write is a page
refresh at `SIG_ExperimentView.cpp, putIntoExperiment`, "which fires on a page switch during a
run but never from the evolution loop". **Wrong on the first half**: the
page-switch route runs `putAllIntoExperiment`, which returns under
`SIG_ExperimentListView::isRunning()` **before** reaching
`experimentView->putIntoExperiment()` — so during a run a page switch
never reaches the writes in `SIG_ExperimentView::putIntoExperiment` either.* The
live callers are `SIG_ExperimentView::slotHistory` and
`SIG_ExperimentView::slotIntervallChanged`, which is what the comment in
`SIG_ExperimentView::putIntoExperiment` says. And the sampler counts **pumps of the event loop,
not seconds**: the run blocks the main thread, the only pump is `haveABreak()`'s
`processEvents` once per outer pass, with `usleep(300000)` per tournament touch
in between. One line in 100 seconds is what that predicts; the `samples == 0`
assertion it feeds is satisfied by a single pump.

**`check.sh` could not tell a binary from a shell script.** The wrapper above
passed the whole check: it is `-x`, it execs the real binary so the no-PVM smoke
test still prints its guard, and `make -q` calls the target current because the
wrapper's mtime is newer than every prerequisite — so the link recipe, and the
`ctor_size` assertion inside it, never ran. `check.sh` now requires ELF magic on
both programs in `build/`. *It closes the wrapper shape only: a copy of the real
binary, a symlink, or a stale ELF still passes and still skips the link recipe.*

**INHERITED, NOT A PORT DEFECT: with MetaGP enabled, `File > Save Experiment`
does not complete — and 1.3 does exactly the same.** The oracle ran the pair on
1.3, same experiment, same four slaves, same clicks, stopped at the same point,
one click different: **MetaGP on — the file is truncated to 0 bytes and nothing
further happens**, the descriptor still open write-only, the process idle at a
CPU count frozen for four minutes; **MetaGP off — the save completes in under
three seconds.** So the port reproduces 1.3 down to the zero-byte artefact. *The
oracle's caveat, recorded because it is the honest boundary of the result: both
its runs had history off, so MetaGP-on-plus-history-on is untested — but both
cells shared that setting, and the MetaGP run never wrote a byte.*

**A REAL DEFECT IN THE DRIVER CAME OUT OF IT.** There is a **second** modal after
the file dialog — class `warning`, title `File exists...`, "Do you want to
overwrite?", Yes/No — and it appears in **both** cells, because the target
already exists. `guidrive` arms one handler and answers one dialog, so it would sit on that
prompt for ever. *Since D35 the port has no such modal. A save onto an existing
`.exp` now raises the file dialog's own confirmation inside the click on Save,
and `acceptFileDialog` does not answer that either.* **Latent here rather than the cause**: these runs deleted
`/tmp/evolved.exp` first, so no overwrite prompt appeared. The silence has a
simpler explanation that matches the oracle's process state exactly —
`QFileDialog::getSaveFileName` is blocking, SIGEL stalls inside
`saveExperiment` after it returns, so `slotSaveExperiment` never returns,
`clickMenu` never returns, and nothing after it can print. *My hypothesis that
`useMeta` blanking `saveName` armed the second dialog was half right and the
wrong half mattered: the prompt is the overwrite one and has nothing to do with
MetaGP.* **The driver defect is real, unfixed, and will bite the first save onto
an existing file.**

Isolated here to one variable — same input, same four slaves, same two
generations, one click different:

| MetaGP | save |
|---|---|
| not enabled | completes, 1,648,437 bytes, process **exits 0** |
| enabled | dialog opens, save never completes, killed by the watchdog |
| enabled, 6 generations | same, and a **zero-byte** file is left behind |

*An earlier guess that a grown history was the cause is withdrawn.* **Not
attributed.** The driver prints the modal's class and title and then produces no
further output, which is odd on its own because every wait in `acceptFileDialog`
is bounded — so the fault may be in the driver. One candidate that would make it
neither: `MT_Controller::useMeta` blanks `saveName` with the comment "force the
routine to show a filedialog", so enabling MetaGP may arm a **second** dialog and
the driver answers exactly one. The oracle has been asked whether 1.3 saves with
MetaGP on. It affects no committed check.

**REBUILDING THE INPUT, because it is not committed and `/tmp` is cleaned
between sessions.** A missing input is not diagnosed as missing: the load fails,
a `QMessageBox` opens, and the scenario sits on it until the watchdog fires with
`active modal: QMessageBox` — which reads exactly like a hang in the evolution.
One session was spent on that. From `data/Experiments/twoBasesSimpleFitness1.exp`
(C11's recipe): the GP `RANDOMSEED` (**the second of the two**) to 12345, the
eight 2003 `PVMHOST` blocks replaced by ONE local block, and every
2003 home folder path, written here as `/home/user/projects/sigel`, rewritten
to the build directory. The host
line is `<hostname> <maxSlaves> 1 "<dir holding sigel_slave>"`, and the shipped
file repeats the bare keyword `PVMHOST` before *every* host line rather than once
as a header. Correct result: 70,779 lines, 14 fewer than the source, differing
only in the host block and that one seed.

**Ruled out along the way, so nobody repeats it:** the host list loads
(`listviewHosts rows=1`, this machine's host name, `maxSlaves 1`, right directory);
`col0check=0` is an icon column, not a disabled flag — all eight shipped hosts
read it too; `noOfSlaves` starts at 0 (`SIG_GPActivePVMHost.cpp:30`) so a
one-slave host is not self-blocking; `pvm_addhosts`' return is ignored
(`SIG_GPFitnessTrainer.cpp, SIG_GPFitnessTrainer`) so `PvmDupHost` cannot stop it; the slave binary
is healthy — run by hand against a live daemon it enrols and answers `Program
hasn't been started as a PVM slave!`; PVM itself is up (`pvm_start_pvmd`
succeeds, the daemon logs `ready`, `./checks/pvm-check.sh` passes); a stalled trainer
waits in `hrtimer_nanosleep` at ~15% CPU; and **`PVM_ROOT`, `PVM_ARCH` and
`PVM_TMP` must be exported** or `libpvm` prints `PVM_ROOT environment variable
not set` and the run produces no transcript at all (`pvm-check.sh:66-77`).

*`ptrace_scope` is 1 on this machine, so `gdb -p` cannot attach to a running
process — but `strace` of a process you START works, which is how the spawn
message was seen.*

### Phase A — core onto Qt 6 — DONE

| # | Work | LOC |
|---|---|---|
| A0 | `compat/q2compat.h` — Qt 2 container semantics over Qt 6 | 541 new |
| A1 | `SIGEL_Tools` | 666 |
| A2 | `SIGEL_Environment` | 1,028 |
| A3 | `MT_GPSystem` | 6,880 |
| A4 | `SIGEL_Robot` + cut back-edge → `SIGEL_Simulation` | 7,469 |
| A5 | `SIGEL_Program` + cut back-edge → `SIGEL_GP` | 1,932 |
| A6 | `SIGEL_RobotIO` | 3,017 |
| A7 | `SIGEL_Simulation` | 8,715 |
| A8 | `MT_Control` + cut back-edge → `MT_GUI` | 2,724 |
| A9 | `SIGEL_GP` + cut back-edges → `SIGEL_MasterGUI` | 14,066 |

The shim exists so A1–A9 are pure renames, deferring ownership to Phase B. The
trap it defuses: Qt 2's `QVector`/`QList` hold pointers, Qt 6's hold values,
**and the names are identical** — a naive rename compiles clean, then
double-frees.

Back-edges cost nothing: 2 were dead includes, 3 forward-declare.

*This said "4 files still fail… blocked on Phase C", listing `MT_Controller.cpp`,
`SIG_GUIGPManager.cpp` and both ZORC files.* **C6, C7 and C8 fixed three of
them and the Makefile now excludes exactly one**,
`WIN_SIG_GPRemoteZORCFitnessFunction.cpp`, which needs `windows.h` and is
permanently out of scope. `check.sh` reports **0 fail** and skips that one
explicitly. *An earlier version of this said 5 and listed
`SIG_GPPopulation.cpp`, which Phase R fixed.*

### Phase B — make ownership explicit — SUBSUMED BY PHASE D

Goal was: every owning container frees its items explicitly at the owner, and
`setAutoDelete` disappears container by container. **14 of 14 core containers
done**, the last five in D15, D18, D19 and D25c; the per-container record is in
those D-steps. **`setAutoDelete` in core is 0** — 2 apparent hits are inside
comments, which is the miscount this file has made five times.

**16 calls remain, every one in a GUI module, and Phase C owns them** — §7's
C1 table. Two of them flip the flag at runtime, which a single destructor free
does not reproduce; §9 "Toggling containers" names both.

*This section used to carry the five-container deferral table, its exit
criterion, and a note that `SIG_Body.cpp`'s local `vertices` "uses
`setAutoDelete` again". That last is false — D7 made it a `QList<DL_vector>` by
value and the only match in the file today is a comment.*

### Phase R — build and run (§3, order item 1) — DONE

`make` at the repo root built 205 vendored objects, 108 of SIGEL's 109 core
sources, two moc outputs and `build/sigel_eval`. Was 251 / 118 of 122 / three
until the Dynamo backend was deleted (see that section in §7): 46 vendored
Dynamo `.cpp`, 13 of SIGEL's own and the `SIG_DynaSystem` moc target went with
it.
*Since item 48, plain `make` builds `sigel`, `sigel_slave` and `sigelApp/`
without the sanitizers; `make sigel_eval` builds the evaluator, and
`make B=build-asan` the sanitized tree, in its own directory.*

```
make sigel_eval
SIGEL_ROOT=$PWD/sigelApp \
  ./build/sigel_eval experiments/twoBases.exp 0
```

One evaluation is 0.2 s. **All 14 published experiments run clean under
AddressSanitizer and UndefinedBehaviorSanitizer**, with identical results
sanitized and not. `replicate.sh` sets `SIGEL_ROOT` itself.

**The defect that stood between building and running** was Qt 2's `QTime()`
being 00:00:00.000 and valid where Qt 6's is null. `SIG_Simulation::start`'s
`while (act < max)` never ended and `fitness = distance / simulatedSeconds`
divided by zero. **12 sites, all now `QTime( 0, 0 )`** — the hazard class and
the twelfth site are in §9's name-collision table, which is the register Phase C
reads.

`SIG_GPPopulation.cpp` also moved off the Qt 2 `QProgressDialog`:
`setProgress` → `setValue`, `wasCancelled` → `wasCanceled`, `setCaption` →
`setWindowTitle`, the 6-argument constructor to Qt 6's, and
`qApp->wakeUpGuiThread()` deleted, which Qt 6 has no equivalent for. Its
`if (qApp)` guards mean none of it runs headless, but it has to compile:
`SIG_GPExperiment` holds a `SIG_GPPopulation` by value.

*This said four core files do not build — `MT_Controller.cpp`,
`SIG_GUIGPManager.cpp` and both ZORC fitness files.* **Three were fixed by C6,
C7 and C8; one remains and always will**, the `WIN_` ZORC variant, which needs
`HANDLE` and `OVERLAPPED` from `windows.h`. None was ever on the evaluation
path.

**Three vendored patches**, applied by `make` against a stamp inside the
untracked tree:

| patch | why |
|---|---|
| `cv97/JVector.h:29` | `remove()` is a member of the dependent base `CLinkedListNode<T>`; two-phase lookup binds it to `::remove(const char *)`. `this->remove()` |
| `cv97/CLinkedList.h:37` | the list header node is a bare `CLinkedListNode<T>`, so `(T *)` is a downcast that never holds and UndefinedBehaviorSanitizer reports it. `reinterpret_cast` |
| `dynamechs/dm/svd_linpack.cpp:180` | the inlined copy of `f2c.h` declares `struct complex`, ambiguous with `std::complex` under the `using namespace std` the pre-standard `<iomanip.h>` carried. `::complex` |

One shim header changed with them: `iomanip.h` now includes `<iostream>`, which
the pre-standard header did. A second, `new.h`, was added for SOLID's 5 sources
and deleted again on 2026-09-20 with SOLID itself.

**OpenGL is on the link line and is never called.** `dmLink::draw()` is pure
virtual and every override lives in `gldraw.cpp`, so every `dm*` vtable
references it and the linker pulls it in. `-lGL`, no `-lGLU` — nothing
references GLU. **That is true of core and FALSE of the interface**: C5 found
`gluPerspective` and `gluLookAt` in `SIG_Visualisation.cpp`, so C9's link line
needs `-lGLU` and the build host needs `libglu1-mesa-dev`.

**Sanitizer split.** SIGEL's own code gets `-Wall -Wextra`, no `-fpermissive`
and the full AddressSanitizer plus UndefinedBehaviorSanitizer. The vendored
libraries get `-w -fpermissive` and UndefinedBehaviorSanitizer minus three
checks they trip by construction: alignment and signed overflow in the f2c
translation of `ssvdc`, and `vptr` in cv97. The exemptions were written for
qhull too, which is no longer compiled as of 2026-08-28; they are left in place
because `ssvdc` still needs them.

**AddressSanitizer.** Clean. `SIG_Environment::generateTerrain` rewrites
`$SIGEL_ROOT/Terrain.ter` on **every evaluation** and reads it straight back.
Workers sharing a root used to read it half-written, get a zero-size grid, and
take a real heap-buffer-overflow in `dmEnvironment::getGroundElevation`; the
write is now atomic, so a reader sees the old file or the new one and never a
half-written one. `replicate.sh` still gives each worker its own root, which
keeps the runs independent of each other, and treats a non-zero exit as an
error; an earlier version scored crashes as a fitness of 0, which made the
headline number load-dependent.

**Leak baseline (D18): 41,254 bytes in 109 allocations** per evaluation, from
§10's pre-existing leak — `SIG_Simulation` is `new`ed and never deleted, and
its destructor is empty. Check on ASan and UBSan errors, not on this.

### Replication — checked against the 1.3 binary since Phase V

`./checks/replicate.sh` runs every individual of every published experiment. It is
**not currently a test of this port**, because the 14 published `.exp` files
were produced in August 2001 by SIGEL 1.0 and the source being ported is 1.3.
See the scope note at the top and `regression_1.0_to_1.3.md`.

Against those 2001 files the current 1.3-faithful build gets 7 of 13 distinct
experiments within 10%. That number measures the 1.0 → 1.3 regression, not us.

**What is needed to make this a port test is Phase V**, below. Not fitness:
fitness is chaotic across architectures — the next paragraph measures it — and
the 1.3 binary never prints one anyway (§9). What compares exactly is the
container ordering the robot serialisation carries, and Phase V starts there.
Everything else is already in place: the harness, the data and the driver.

**Fitness is a chaotic metric.** A 1-ULP change to the robot's start height
moves an individual's fitness by 45% and best-of-100 by up to 18%. The 2003
build was i386 using 80-bit x87 registers; this is aarch64 with IEEE doubles.
Bit-exact agreement on a long run is not achievable and its absence proves
nothing. What is sound: short trajectory traces taken before divergence grows,
and the deterministic non-integrating quantities — link and joint numbering, the
register value a given joint angle produces, the force a given register value
produces.

**Verified faithful to Qt 2 by independent audit**, each by measurement rather
than inspection:

| | |
|---|---|
| `Q2Dict` hash order | the shim reproduces Qt 2's ELF hash, seed, shift, mask and ascending bucket walk exactly; link and joint order checked against an independent model of Qt 2's table for all 7 robots |
| `SIG_Randomizer` | identical sequence — the LCG's extracted bits 16..30 are unaffected by `unsigned long` widening. **Confirmed against the 1.3 binary 2026-08-29, no longer audit-only** — see V9 |
| `QTextStream` double formatting | byte-identical to Qt 2's `%.6lg` over 200,000 random bit patterns, except `-0`. **That sample cannot see exact decimal ties, where Qt 6 and Qt 2 round differently — see the fourth family, reopened at C5** |
| `Q2PtrVector`, `Q2PtrList` | size/count/insert/remove/resize and the internal cursor checked against `qgvector.cpp` and `qglist.cpp` |

**Still open, found by audit, not yet acted on:**

- `QTextStream` latches its error status in Qt 6 where Qt 2 did not: reading
  `"1.5 abc 2.5 3.5"` as four doubles gives `1.5, 0, 0, 0`, because Qt 6 sets
  `ReadCorruptData` and refuses every later read. Qt 2 returned 0 and un-got the
  character, so a following `>> QString` resynchronised. All robot, experiment
  and language parsing goes through `>>`. Does not fire on the shipped data.
- `SIG_EarlyRunTermSimulation.cpp, getMaxRecorderSteps` was a twelfth `QTime()` site, missed by
  the first sweep because it is a declaration rather than a call. Fixed. It made
  `getMaxRecorderSteps` return 2 instead of 182 for three fitness functions no
  shipped experiment selects.

### Phase V — check against the 1.3 binary — NEW 2026-08-27

Replication above says what is missing: **every check in this repo compares the
port against itself.** `dictorder-baseline.txt` and `fitness-baseline.txt` both
prove that nothing changed, not that anything matches 1.3. Phase V supplies the
other side of the comparison, from the 1.3 binary on the x86 box, reachable
through the `sigel-x86` session.

**Capture once, diff for ever.** 1.3 is frozen, so its output for a given input
never changes. Each step captures a reference file once, commits it under
`verification-against-sigel-1.3/`, and every later step diffs against it locally. The x86 box is
needed **once per quantity, not once per step** — after V1 this is another line
in `check.sh`, not a remote call.

**Static inspection of the 1.3 binary does not need the remote box.** It was
`xb/kdesigel/sigel` and `xb/kdesigel/sigel_slave` — ELF 32-bit i386, **not
stripped**, dated 30 April 2003 — and `nm` and `objdump -d` read them locally,
cross-architecture, on this aarch64 host. **Both were deleted on 2026-09-20**,
with `kbin.tar.gz` that held them: nothing read them any more, the port is
finished and its agreement with 1.3 is proved. Every figure below was taken
while they were here; to re-check one, download `kbin.tar.gz` from sourceforge
again. Verified
by reproducing the remote session's figures exactly: 24 `MT_FitnessTranier`,
0 `MT_FitnessTrainer`, 22 `SIG_GPFitnessTrainer`, 0
`SIG_GPEnergyFitnessFunction`. *Several V9 questions were sent to the remote
session that could have been answered here in seconds.* The remote box is still
required for anything that must **run** — 32-bit execution against Qt 2.3 shared
libraries, PVM, and every behavioural capture V1 and V5–V8 rest on. Symbols and
disassembly are local.

| # | Step | What it checks |
|---|---|---|
| V1 | ~~Capture 1.3's load-and-save round trip for three shipped `.exp`~~ **DONE 2026-08-27** — `verification-against-sigel-1.3/v1-1.3-roundtrip.txt` | the `Q2Dict` hash, all order-carrying containers, the parser and the serialiser |
| V2 | ~~Our half: a save path in `sigel_eval`~~ **RESHAPED 2026-09-08, and the `sigel_eval` half is dropped for good.** What V2 has to answer is whether our saved file matches the one 1.3 saved. A save path in `sigel_eval` cannot answer it: `sigel_eval` links `SIG_GPExperimentClean.o` (the Makefile's `CLEAN_OBJ`), which is the SLAVE variant of the writer, and `sigel_slave.cpp` never calls `saveExperiment` at all. So that variant is dead code in the only binary that ships it, and a test there would test something no user reaches. It is also the wrong side of the one difference between the two writers: the master writes the MetaGP block when `mtController->IsEnabled()` (`SIG_GPExperiment.cpp:107`) and the slave has no `mtController` (`SIG_GPExperimentClean.cpp:87`). Everything else is the same source compiled twice. Against a V1 reference written by 1.3's master, the slave writer gives identical bytes with MetaGP off and cannot produce the right bytes with it on. **V2 DONE 2026-09-08, and it is a check.** Two shipped experiments go through `File > Open Experiment` and `File > Save Experiment` twice each, and the run is compared against two captures of the running 2003 binary — `v8-1.3-gp-blocks.txt` for `hammer`, `v1-1.3-roundtrip.txt` for `octopusSimpleFitness`. Both predate this conversion, so a failure is a regression against 1.3. **Two experiments, because hammer alone proves less than it looks.** Hammer has 5 links, 4 joints, 4 drives and no sensors, few enough that no hash bucket need collide, and 1.3 does not permute those containers either — so agreement there is not evidence. Hammer really tests material order, `Body` emission order, `middle3`'s axis points, and everything outside the robot. **Octopus supplies the rest:** V1 measured 1.3 permuting its joint, drive and sensor containers, its body order and its command list, and our writer moves none of them. That is where *we reproduced the order* and *we never permute* come apart. The oracle made this point against the first version of this row, which claimed more for hammer than hammer can carry. **What agrees with 1.3:** all six `#####` marker line numbers in all three hammer passes; `PVMHOST` 20 of 20 in 1.3's order; the experiment history byte-stable at 161 lines; the first HISTORY block at 10574, 10581 and 10588 characters; one added line in all 100 blocks per save; the individual name sequence; nine of the ten first-save keys with 1.3's values; and octopus's material and link order, which 1.3 returns unchanged too. **Input against pass 1 is not the test**, per V8 result 5. **Two things differ and both are explained.** (a) Our robot block is a FIXED POINT where 1.3's is an involution — D3's flip to insertion order. A single save cannot tell those apart, which is why there are two saves. Measured on 1.3 for hammer 2026-09-08 (result 6) and for octopus by V1. (b) `TEXALPHA` reads 99 here against the hammer capture's 255, because **1.3 has two save paths and they write different files.** Measured on 1.3, hammer, both paths in one sitting: the GUI save and the headless save differ in **exactly one line and nothing else**, confirmed by a sha256 over both files with only that value masked. 1.3's GUI writes 99, its headless path writes 255, and `pagesave-baseline.txt` shows the same on a second robot. `SIG_EnvironmentView.cpp, getOutOfExperiment` pushes the 255 default into `sliderAlpha`; the pristine form gives that slider no maximum — read it at `git show 0516d62:…/SIG_EnvironmentBase.ui`, since the file at that path today is the converted one — and Qt 2 caps it at `QRangeControl`'s default of 99 (`qrangecontrol.cpp:111-119`, from `qslider.cpp:124`). Qt 6 has no `QRangeControl`; its 0–99 default comes from `QAbstractSlider`. **Same number, different mechanism, which is weaker than it looks:** nothing enforces that the two toolkits keep the same default, so a Qt release could move our side alone and the check would be the only thing that noticed. If it ever moves, set the maximum on the form rather than chase the toolkit. **Not measured:** that 1.3's slider really reports 99 — the oracle's X server went down mid-probe and the reading was not retaken. Result 7. **The check is one diff of a 58-line report, not a dozen ifs**, so a generator that produces the wrong text fails on the whole report rather than on the one predicate someone remembered to write. Its own teeth test found a hole in it: three lines hashed an extract and compared three hashes, which reads *identical* when the extractor dies — replacing `expstruct.py` with `/bin/false` passed the check 1/0. Each of those lines now carries the size of what it hashed. A second teeth round found the mirror of it: every *stable across all three passes* line passes when the INPUT changes, because all three passes change together — renaming an individual in the input went unnoticed. Those lines now carry content as well, and section 6's first and last entries are compared against 1.3's, which V8 result 4 quotes | checked as `v2 round trip vs 1.3` |
| V3 | ~~Determinism on the x86 box — one experiment run twice, both `RANDOMSEED`s pinned~~ **SATISFIED 2026-09-02.** The oracle ran it twice on each of two models: `serA`≡`serB` and `octGateA`≡`octGateB`, identical on every field but the run-directory paths | closed. It is SAME-BOX determinism, which is sound; it was never a cross-machine claim |
| V4 | ~~Two whole-run digests validated against the reference machine~~ **DROPPED 2026-09-03.** A whole-run digest cannot cross an x87/IEEE boundary, which §7 and D26 already said and C11 then proved: the identifier counts that appear to agree are FORCED — consumed in the tournament constructors before any fitness is read, with `createTours` building a constant 50 per generation — while the contents are decided by a float comparison and cannot agree. Estimated ~98% chance the counts matched even under maximal divergence | dropped, not deferred. What replaced it is a measurement of OUTPUT on one machine — see C11 |
| V5 | **MDH probe DONE 2026-08-27, PASS** — `verification-against-sigel-1.3/v5-1.3-mdh-compared.txt`. **SENSOR PROBE DONE 2026-09-08, PASS, and it needed no gdb** — the constants are readable from both images, so the comparison is source-against-binary rather than breakpoint-against-breakpoint. **All eight constants the live `sense` references agree with 1.3 as values** — computed from our source expressions, not read out of our binary, and the difference matters: on aarch64 only ONE of the eight (the radian factor) is emitted into `.rodata` at all, four appear nowhere as 8-byte doubles, and two appear only in debug sections. The one that matters is the radian factor: 1.3 converts with pi TRUNCATED TO EIGHT DECIMALS, `180/3.14159265` = `404ca5dc1af05a77`, against the true `404ca5dc1a63c1f8`, a relative error of 1.14e-09. **The decisive measurement is an absence, and it reproduces on both sides:** the true constant appears in NEITHER 1.3 binary and in none of ours. Our source spells it `360.0/(2.0*3.14159265)` where 1.3's image holds the folded quotient — computed, not assumed, to be the same bits. The multiply-before-divide order, the ±`DBL_MAX` sentinel and the plain-`double` widths all match too. **Checked as `truncated pi (V5)`**, because the way to break it is a one-word edit that looks like tidying — and 1.3 itself uses the true `M_PI` in `IFunctions.cpp, calculateAnyJoint`, so the truncated literal reads as an oversight. **FORCE PROBE SCOPED AND NOT ATTEMPTED AS A BIT COMPARISON.** `moveDrive` carries its value chain in `long double`, and this machine's `long double` is IEEE binary128 against 1.3's x87 80-bit — `LDBL_MANT_DIG` 113 against 64. **Use that, not `sizeof`:** `sizeof` is 16 on x86-64 as well, where the format matches 1.3's exactly, so it cannot tell the platforms apart. An earlier version of this row cited `sizeof` and a rounding inequality, and the oracle showed both hold on x86-64 too. **This is a property of the BUILD HOST, not of the port:** an x86-64 build would make `moveDrive` comparable, though closing the plain-`double` gap as well would need `-mfpmath=387`, because x86-64 computes `double` in SSE where i386 used x87 intermediates. D26 and §7. `sense` is comparable further, because `scaledState` and `posRange` are plain `double` up to the divide, so a future value probe should stop at `scaledState` rather than at `registerValue` | the port's **arithmetic**, which V1–V4 never touch |
| V6 | **DONE 2026-08-29, PASS, 5 of 5** — `verification-against-sigel-1.3/v6-1.3-friction-nocollide.txt` | the two Phase D paths **no shipped data exercises**: friction pairs and no-collide pairs, and whether both setters negotiate |
| V7 | **DONE 2026-08-29, 4 runs on `walker`** — `verification-against-sigel-1.3/v7-1.3-friction-nocollide-rules.txt` | the remaining rules for those two paths: multiple partners, unloaded partners, duplicates, and whether a dropped entry is resurrected |
| V8 | **DONE 2026-08-29, captured BEFORE the conversion** — `verification-against-sigel-1.3/v8-1.3-gp-blocks.txt` | `SIG_GPParameter::hostList` and `SIG_GPExperiment::experimentHistory`, the two `Q2PtrList` the checks run on every load and the next to convert |
| V9 | **DONE 2026-08-29, 3 of 3** — three function *bodies* disassembled, recorded below rather than as a capture file | whether a reworked body hides under an unchanged name. Symbol lookups cannot see that |

### V4 — the whole-run check — DROPPED 2026-09-03

V4 proposed whole-run digests captured on the 1.3 reference machine and compared
against ours, on the argument that *"a match proves the interpreter, physics,
genetic operators, selection and RNG all agree in composition"*. **It cannot
work.** The digest lines were `NAME|FITNESS|sha256(program)`, and **fitness
cannot cross an x87/IEEE boundary** — §7 measures a 1-ULP change in start height
moving it 45%. Worse, the quantities that *do* agree agree for free: identifiers
are consumed in the tournament constructors before any fitness is read, and
`createTours` builds a constant 50 tournaments per generation, so the counts are
forced by the code — ~98% chance they matched even under maximal divergence.

**Two lessons kept.** Reference data that cannot be verified in this repository
is *unfalsifiable locally* — a later mismatch could be a reconstruction error
rather than a port defect, so commit the inputs or do not rely on the digest. And
**a comparison surface that includes a float is not a comparison surface across
machines.** What replaced V4 is a measurement of OUTPUT on one machine, needing
no reference at all — see C11.


### V9 — three function bodies from the 1.3 binary, 2026-08-29

Each prediction was written down *before* the disassembly was requested, so a
match is evidence rather than agreement. All three hold.

**1. `SIG_Randomizer::getRandomInt` — the whole generator, as immediates.**

```
imul $0x41c64e6d,(%edx),%eax   ; * 1103515245
lea  0x3039(%eax),%edx         ; + 12345          modulus: none, implicit 32-bit wrap
shr  $0x10,%eax                ; >> 16   LOGICAL
and  $0x7fff,%ecx              ; & 0x7FFF          extraction = bits 16..30
cmpl $0x0,0xc(%ebp) / je -> xor %eax,%eax          ; maximum==0 returns 0
idivl (%esi)                   ; SIGNED reduction
```

`getRandomLong` is a pure wrapper. State is a 32-bit int at object offset 0.
**"Bits 16..30" is now a fact from the binary, not a second reading of our own
source.** **The widening is safe, and here is the actual reason:** the low 32
bits of a product depend only on the low 32 bits of the operands, so bits 16..30
evolve identically whether the multiply is done at 32 or 64 bits — our
`unsigned long int next` is 64-bit here and was 32-bit in 2003, and it does not
matter. Two riders: the shift must be **logical** (`shr` there; unsigned
division here) and is **safe only because `% 32768` discards bits 31 and up — if
that mask ever moves or widens, the two part**; and the reduction is **signed**
on both, with `randomNumber` always in `[0, 32767]` so a negative `maximum`
yields a non-negative result either way.

**Seed 0 means "seed from the clock"** — `QTime(0,0,0,0).secsTo(currentTime())`,
seconds since midnight, in both, and `QTime(0,0)` is explicitly constructed so
§9's `QTime()` collision does not touch it. **Original 2001 code**, identical in
`downloads/sigelSourceDistribution.1.0`.

**There are two `RANDOMSEED` keys per `.exp`, and only one of them seeds
anything.** Measured over all 14 files in `data/Experiments/`:

| key | line | value | reaches a randomizer? |
|---|---|---|---|
| `SIG_SimulationParameters` | 10 | **0 in all 14** | **no.** Its only consumer is a GUI spinbox |
| `SIG_GPParameter` | 66 | **1 in 8, 0 in 6** | **yes** — `SIG_GPManager`'s `randomizer( gpParameter.getRandomSeed() )` |

So the key that is 0 everywhere seeds nothing, and the key that feeds
`SIG_Randomizer` is **1** in 8 of the 14 — "as distributed they are clock-seeded"
is false for a majority of the corpus. **There is a genuine unconditional clock
seed** elsewhere: `SIG_GPPopulation`'s default constructor builds a
`SIG_Randomizer()`, which runs `setNewSeed(0)` **regardless of any `.exp` key**.
**Consequence: a determinism run has to pin the GP key AND account for
`SIG_GPPopulation`'s own randomizer, which no key reaches.**

**2. `SIG_GPPopulation::deleteIndividual` — D15 was written against the right
reference.** Outer loop from `poolpos` to `size-2`; fetch successor,
`setPoolPos(i)`, shift by `take(i+1)` then `insert(i, ptr)`, then `resize(size-1)`
after the loop. **Zero `__builtin_delete` and zero `free` in the whole function**
— counted, not eyeballed. The single free is inside the container's `insert`,
disposing of the slot's previous occupant on the first iteration: exactly the
hidden free §9 lists, and exactly what D15's explicit `delete pool[poolpos]`
replaces.

**3. `SIG_Material::setFrictionValue` — the shape behind V6 and V7.** Iterator
walk; update in place on found, `__builtin_new` + `append` on not-found; then the
recursive call guarded by **both** the `negotiate` flag and `otherObj != this`,
passing `false` so it cannot recurse further. **Exactly one recursive call**,
counted. External behaviour and internal shape agree.

**What V9 does not establish.** Three matching bodies raise confidence that the
`SIGEL_*` code is common between our source and this binary. They do not prove
all of it is. `MT_FitnessTranier` remains the only confirmed divergence.
It shows that the oracle's binary predates the published source in `MT_`, so
the oracle's `MT_` observations are not evidence about this source. The caveat
stops at `MT_`: nothing measured widens it to other code.

### V5 RESULT — the arithmetic agrees with 1.3

**`twoBases` is exact, bit for bit, all four fields. `octopus` agrees on all 9
joints, `alpha` is bit-identical on every one, three joints are exact in all four
fields, and the worst real disagreement is 5 ulp.** No high-bit disagreement
anywhere. Exact in all four: `thirdLegJoint2`, `thirdLegJoint3`,
`firstLegJoint2`; five joints within 1-5 ulp; one near-zero residual. Call
counts 2 and 18, each real call preceded by `dmMDHLink`'s own
`setMDHParameters(0,0,0,0)`, alternating without exception, so nothing was
filtered. With V1 covering ordering, the port is checked against 1.3 on both
ordering and arithmetic.

**The one substantive disagreement is a near-zero residual, and it points our
way.** `secondLegJoint2`'s `d` should be zero: ours is exactly
`0000000000000000`, 1.3's is `3ca6a09e667f3bcd` = 1.57e-16 — the 2003 build
carries a crumb from an x87 80-bit intermediate failing to round to zero the way
an IEEE double does. **That direction was predicted in advance**, which is what
makes it confirmation rather than a defect.

**MATCH REFERENCE DATA BY A KEY SOMETHING ELSE ALREADY CHECKS — NEVER BY
MINIMISING THE DIFFERENCE UNDER TEST.** The first table came from a greedy
minimum-total-ulp assignment across all four fields, described at the time as
"matched by `alpha`, which is unique per joint" — doubly wrong, since `alpha` is
shared by 6 of octopus's 9 joints and the matcher was not using it. **A
minimum-difference assignment optimises for the quantity the comparison exists
to measure**: it picks whichever pairing makes the two sides agree best, so it
cannot report a large disagreement even when one is there. It is circular, and it
fails silently and in the flattering direction. Rematched by call order — which
both sides emit natively — two
attributions swapped, the residual proved to be on one joint rather than two, and
**three joints are exact rather than one**: the circular method had also made the
result look worse than it was. The verdict never moved; the per-joint table did,
twice.

**`applyForce` and `senseJoint1/2` took zero hits, and the harness was live** —
the MDH breakpoint fired 18 times in the same session. **Explained by the Dynamo
deletion:** both have exactly one caller each, on the Dynamo path, and all 14
shipped experiments carry `SIMULATIONLIBRARY 1`. The breakpoints were on
functions the run could not reach; re-point them at the DynaMechs classes.
`walker` has not been run.

**What made V5 work, after two failed routes.** Under woody's loader there is no
exec event at all — `ld-linux.so.2` maps `sigel_slave` and jumps to it, so every
breakpoint gdb places before start-up is silently never inserted, which is why
`main` never fired either. **Let the slave start and attach to the running pid.**
Then, because `-visualize` builds the simulation during start-up, attaching after
the window appears has already missed it: **stop, then play** forces a full
reconstruction. A live `pvmd` is required or the slave exits at once.

**Method decisions, settled once so they are not re-argued per probe.**
**Function boundaries only** — the binaries carry `.symtab` but **no DWARF**, so
a breakpoint can read arguments off the stack and a return value out of a
register and nothing else; every probe must pick a function that carries the
wanted value across its own boundary as a plain `double`. **Compare raw IEEE 754
bits, never decimal** — a decimal rendering can hide a last-bit difference or
invent one. **Report by name, not by index**, which keeps the probe independent
of container order. **The 2003 binary uses Qt 2's own class names** — `QVector`,
`QList`, `QDict`, not our renamed `Q2PtrVector`, `Q2PtrList`, `Q2Dict`; searching
its symbol table for our spellings finds nothing. The one good target was
`dmMDHLink::setMDHParameters` — 4 doubles per joint at setup, computed by short
arithmetic and passed in, nothing integrating, so no tolerance is arguable.
`SIG_DynaMechsSimulationQueries::sense` is not a target: it writes into a
register object rather than returning a value, which is awkward with no type
information.

**When comparing, compare name→value, not whole lines.** A naive `sort | diff`
reports false differences, because a `Link` line's text changes when its own
internal point order permutes.

**V1 has a trap.** `SAVEEXIT=1` rewrites the `.exp` in place, so every run works
on a copy — never on the pristine download. The dead 2003 paths have to be
repointed first.

### V7 and V8 RESULTS — the rules, and a reference captured in advance

Full captures: `verification-against-sigel-1.3/v7-…txt` and `v8-…txt`.

**V7, four runs on `walker`.** Multiple partners work, each keeping its own
value. A duplicate partner is refused on both sides, confirming D10's
`!contains` and D11's append-versus-update as upstream. **A partner not yet
loaded is dropped in silence** — `walker`'s `body` is its second link, so
`body → foot1 leg1` lost both entries with no warning and exit 0.

**And the drop is worse than a loss.** The valid declaration from the other
side refills the emptied slot with *its* value: `bodyMaterial → shoulderMaterial`
was written as `0.3` and came back `0.7`. Present, correctly named, `nfric`
right, symmetric — and the value is somebody else's. A count check cannot see
that; only a value check can. Our self-check could not: it read the partner
after the first set and never after an update.
`SIG_WANT(b.getFrictionValue(&a) == 0.75)` is added, and moving
`setFrictionValue`'s negotiate call inside its `if (!found)` makes exactly that
one assertion fail.

**V8 was captured before the conversion it serves.** `PVMHOST` order is stable,
20 of 20 over three round trips, as predicted from `hostList` being a
`Q2PtrList` that appends rather than hashes — D13 converted it against this.
`HISTORY` blocks grow **one line** per save without limit — uniformly, all 100
by exactly one, counted per block. The defect is in §9 and
the measurement in `v8-…txt`.

**V8 result 5 — the first save is a format upgrade, and it will break V2 if
built naively.** The shipped `.exp` are a 2001 format revision; the 2003 binary
adds **ten keys** they predate — `FLOORDIMENSION`, `FLOORFUNCTION`,
`FLOORPICTUREFILE`, `FLOORFUNCSELECTED`, `TEXTUREFILE`, `TEXALPHA`,
`WITHTEXTURE`, `AUTOSAVETIME`, `RESEVGEN`, `WITHHISTORY` — with defaults, on
the **first save only**, then holds. So **a check comparing a shipped file
against its own round trip fails no matter how correct the port is.** Compare
pass 1 against pass 2, and still normalise trailing whitespace inside `HISTORY`
blocks. This also closed a discrepancy in V8's own arithmetic: pass 1
decomposes as 17 + 2 + 102, where section 4's 102 is 100 history lines plus 2
one-off; passes 2 and 3 are +100, all history.

Stable from the input onward, and therefore safe to check on directly:
`PVMHOST` order, experiment-history contents, and section 6 as a whole.

### V6 RESULT — negotiation is upstream, and our material order is not

Full capture: `verification-against-sigel-1.3/v6-…txt`. 5 of 5, prediction held.
Both setters negotiate in 1.3, so the partner gains the reverse entry. Nothing
to fix in D10 or D11.

**Our build does not reproduce 1.3's material order for hammer.** 1.3 emits
`plastic, rubber, metal`. We emit `plastic, metal, rubber`, from `data/` and
`data-reordered/` alike, `loaded` and `copy`.
`git show 5ebc9a8:dictorder-baseline.txt` has `plastic, rubber, metal`, so the
shim-era build matched and D3's flip to insertion order moved us off it. V1
never covered hammer, so this is the first 1.3 observation of that order.

**WIDENED 2026-09-08 by V2's oracle capture, and it is not materials only.**
`v8-1.3-gp-blocks.txt` result 6 has hammer's other five containers from 1.3.
Three things toggle on 1.3, not one: material order, `Body` emission order, and
the named axis points inside the `middle3` `Link` line, at positions 3 and 4.
Link, joint and drive do not move on hammer. **That is a size effect. It does
not mean those containers preserve order.** Hammer has 5 links, 4 joints and 4
drives, few enough that no bucket need collide. V1 saw all three permute on
`octopus`. Hammer's sensor container carries no evidence at all, because the
robot has no `Sensor` records. **So hammer cannot test them and `octopus` must** —
which is why the V2 check round-trips both.

**Our robot block is a fixed point and 1.3's is an involution.** Measured the
same day over four states of hammer: our md5 does not move, 1.3's toggles
between two values, `state0 == state2` and `state1 == state3`. That is a fifth
robot confirming V1's order-2 finding on 1.3's side.

D3 chose it deliberately: `loaded` had to move, body and material order moved
with it, and nothing numbers either. `dictorder-reorder.py` leaves both alone
on purpose.

**It composes with V7.** Material order decides which friction declarations are
forward references; a forward reference is dropped and then refilled by
negotiation with the other side's value. A file with friction pairs written by
us and read by 1.3, or the reverse, can differ in which pair is dropped and
what refills it, with the structure symmetric at both ends. No shipped file
declares friction, so this is latent. **Do not revisit D3 without it.**

### V1 RESULT — the hash model is confirmed against the real binary

**10 of 10 container orders match, 0 differ**, including two containers this plan
never modelled: `octopus` and `walker` link / material / joint / drive / sensor,
plus `walker` `leg1`'s axis points *inside* the Link line and `octopus`'s body
emission order. 105 entities. Compared 1.3's *saved* order against the `loaded`
order our `Q2Dict` produced, recovered from the pre-flip baseline at
`7f7410a:linkorder.txt`.

**The prediction held in advance**, which is what makes this evidence rather than
agreement: `twoBases` (2 links, 1 joint) came back byte-identical while `octopus`
and `walker` came back permuted. **This is the first thing in this repo checked
against something other than itself** — until now `q2compat.h`'s hash had only
been checked against a second reading of `qgdict.cpp` by the same hand, with
D2–D7 stacked on it. The **round trip is an involution**, an order-2 permutation,
exactly what bucket reversal on prepend predicts, and it independently confirms
D2's finding that the 14 `.exp` needed no migration. *The per-link axis points
and the Body/Geometry emission order are already in `dictorder-baseline.txt` and
both match, so the count of "eight order-carrying containers" was wrong and is
**ten**.*

**Why the round trip is the sharp test.** The `.exp` carries the robot as a
`StreamedRobot` block, and that block *is* dict iteration order —
`SIG_Robot::writeToFileTransfer` walks the six dicts to write it, loading inserts
each entity into a `QDict`, saving iterates it back out. Qt 2's `QDict::insert`
**prepends**, so every bucket holding more than one name comes back reversed.
Load-and-save is not the identity but a permutation, and which permutation is a
fingerprint of the hash. The `LanguageParameters` command list rides along in the
same artifact — shipped, it reads `MUL MOVE CMP COPY LOAD SENSE SUB DIV MIN DELAY
ADD MOD MAX`, neither alphabetical nor declaration order, so it is hash order too.

**Traps this run found:**

- **The dead 2003 paths differ per experiment.** `twoBases*` use one 2001
  author's home folder; `octopus*` and `walker*` use another's, plus a
  separate model directory on the `Body` lines. One of those occurrences is
  *inside* the `StreamedRobot` block, so repointing necessarily edits what is
  being compared; V1 diffed edited-input against output for that reason.
- **`twoBases` is a control for the robot containers only.** Its command list
  permuted too, because that dict holds 13 entries whatever the robot's size.
- **SIGEL traps SIGTERM**, so only SIGKILL stops it — and SIGKILL means
  `SAVEEXIT` never runs and there is no output file, so a kill-based fallback
  yields nothing.
- All three shipped populations are already fully evaluated with a 2001
  termination date, which is why load/save exits immediately. **With no `pvmd`
  running, every `pvm_addhosts` fails and SIGEL loads, saves and exits 0
  regardless** — evaluation impossible rather than merely disabled, which made V1
  cleaner than specified.

### Reference material — all of it, downloaded 2026-08-22

`sigel.sourceforge.net/seiten/ergebnisse_de.html` carries the published results:
14 `*Experiment.tar.gz` (the same 12 `.exp` as `experiments.tar.gz`, **plus
`runnerNiceWalkingFitness` and `runnerSimpleFitness`, which are not in it**), 9
`.mpg` films of the evolved gaits, and the per-robot model archives.
`.../berichte/endbericht.pdf` is the project's final report, and its chapter 5
documents these experiments one by one with fitness curves and stated speeds —
the only independent source of numbers. All in `data/`, untracked.

### Phase T — the old-Qt tool container — §4 — T1, T2 DONE 2026-08-27

| # | Work | state |
|---|---|---|
| T1 | `tools/Dockerfile.qtmig` + `tools/qtmig`: Qt 4.8's `uic3` and `qt3to4` over this repo | **done.** Both removed 2026-09-20, in git history. They carried one fact recorded nowhere else: jessie has left the mirrors, so the image needs `archive.debian.org` and `Acquire::Check-Valid-Until false` |
| T2 | Prove it on one form, then all 20 | **done** |

**T2 EXERCISED ONE OF `uic3`'s SIX MODES.** *This said five; C1 corrects the count and the list.* Everything below, and the residue
table, is about `-convert`. The tool also has declaration, implementation,
subclass and image-extraction modes, and three of them answer residue rows this
plan was treating as hand work — including the QWidget-derived base class, whose
absence from Qt 6's `uic` this section never noticed. Found at **C1**; the table
is there, not repeated here.

Build-time only. Nothing ships from it and nothing links against it.

**Debian jessie is the last release that packages Qt 4's migration tools *and*
has an arm64 port**, so this runs natively — no qemu, no x86 emulation, which
is not registered on this host anyway. `libqt4-dev-bin` carries `uic3` and
`qt3to4`; `libqt4-dev` carries `/usr/share/qt4/q3porting.xml`, the class-rename
rules. **`QTDIR` is not needed** — the path is compiled into both tools and the
converted output is byte-identical with `QTDIR` unset.
`/usr/bin/uic3` is a qtchooser stub — the image puts
`/usr/lib/aarch64-linux-gnu/qt4/bin` first on `PATH` to skip it, and it resolves
the multiarch triplet at build time rather than hardcoding aarch64.

**T2 result — `uic3 -convert` reads the Qt 2 forms directly.** All 20 convert,
0 failures, output `version="4.0"`, which Qt 6's `uic` accepts.

**"No warnings" was wrong** — corrected by review. `uic3` writes the form to
stdout and warnings to stderr; the first measurement kept only exit codes. There
are **28 warnings across 7 forms**. Redirecting stderr into the same file puts a
warning on line 1 and the output stops being XML.

**No Qt 2 → Qt 3 leg is needed for forms** — measured 2026-08-27. Whether
`qt3to4` needs one for *sources* is untested.

**The residue is larger than the 6-entry table first claimed here.** `uic3` maps
Qt 2 widgets onto Qt3Support classes, which Qt 5 deleted, so Qt 6's `uic`
accepts the XML but 19 of 20 generated headers fail to compile. Corrected by
review, the full residue is:

| | |
|---|---|
| 6 Qt3Support widget classes | the table below |
| 11 Qt3Support **enum values** | `Q3ListBox::NoSelection` ×6, `Q3ListView::Extended` ×4, `Q3ListBox::Extended` ×1. Qt 6's are a different enum *and* a different value name — `QAbstractItemView::ExtendedSelection` |
| 27 custom **slot declarations**, silently dropped | in 6 forms. The 49 `<connection>` elements all survive; what goes is the `<slot>` declarations. Qt 6's `uic` then resolves the slot against the widget's Qt base class and emits `qOverload<>(&QDialog::slotFoo)` — **25 hard compile errors** |
| 18 `qPixmapFromMimeSource` | Qt3Support, removed, not merely obsolete |
| 9 embedded images in 2 forms | **the dangerous one.** Qt 6's `uic` omits `<images>` and emits `setIcon(QPixmap("image0"))`, which compiles clean and renders a blank button at runtime |
| **`Line` `orientation`, silently dropped** | `uic3` emits `<widget class="Line" name="X"/>` with no properties, and `orientation` is the only thing Qt 6's `uic` reads to pick a frame shape — so the separator becomes a bare `QFrame`, i.e. `NoFrame`, and paints nothing. **3 of the 6 `Line` widgets are affected** — those whose Qt 2 form set `orientation` **alone**: `MT_StatisticsWidgetBase/Line20`, `SIG_LanguageParametersBase/Line1`, `SIG_GPParameterBase/Line2`. `uic3` keeps an explicit `frameShape` and drops only the then-redundant `orientation`, so the other three are safe. *This row said "all 6"; measured at C2, corrected by review.* `check.sh` counts `Line`s against those carrying either property |
| **6** real size constraints, silently dropped — *not 4* | `QLayoutWidget` → `<layout>` discards them: `MT_IndividualWidgetBase` `Layout32`/`Layout33`/`Layout28` lose `maximumSize 130×32767`, `MT_PopulationWidgetBase` `Layout60` loses `minimumSize 200×0`, **and `MT_StatisticsWidgetBase` `Layout22`/`Layout22_2` lose `sizePolicy` Expanding/Expanding** — re-measured at C2 over all 101 `QLayoutWidget`s. Their `minimumSize 0×0` / `maximumSize 32767×32767` are Qt defaults and inert, and `SIG_MovieSettingsDialogBase/Layout15`'s `geometry` is meaningless once laid out. **C2 did NOT restore any of these.** The faithful conversion is a real `QWidget` carrying the constraint around the flattened layout; it is C2's one unfinished item and C6/C7 own it with those forms' modules |
| 11 widgets renamed by Qt 6's `uic` | duplicate names — `tab`→`tab1`… in **6** forms, not 5 (measured at C2). Breaks any hand-written subclass referring to them |

Verified preserved: tab order 143/143, combo and list box items 47/47, list view
columns 15/15, layout margins and spacing 196/196, and all seven property
renames. No buddies exist in the source.

The 6 widget classes, across all 20 forms:

| class | uses | Qt 6 |
|---|---|---|
| `Q3GroupBox` | 48 | `QGroupBox` |
| `Q3ListBox` | 7 | `QListWidget` |
| `Q3ButtonGroup` | 7 | `QGroupBox`, **plus a `QButtonGroup` only if the children need exclusivity** — C1's two needed none. Read the children per form |
| `Q3MultiLineEdit` | 6 | `QTextEdit`. All 6 live here, none in module code |
| `Q3ListView` | 5 | `QTreeWidget` — same as D9 |
| `Q3ProgressBar` | 1 | `QProgressBar` |

Two non-standard classes survive conversion and are not Qt's:
`SIG_SimulationVisualisationWidget`, SIGEL's own OpenGL widget (C5), and
`Line`, which is Qt Designer's separator and becomes a `QFrame`.

Qt 6's `uic` emits **53 warning lines over the 20 forms**, not the one an
earlier draft claimed: 20 obsolete-pixmap-function, 21 `QLCDNumber::numDigits`,
11 duplicate-name renames, 2 omitted `<images>`, 1 omitted `<sizepolicy>`.

### Phase C — the interface — AUTHORIZED 2026-08-27

**The pre-work inventory is deleted — Phase C is done and every row of it was
converted.** What is kept is the method, because the inventory was wrong twice
before it was right and both errors were about the *referent*, not the count.

- **Scope was the 5 GUI modules**, `src/<m>` and `include/<m>`.
  `SIGEL_RealInterface` is a sixth GUI directory in neither that list nor §2's,
  and it contributes **0** to every row.
- **Tool: `command grep -a` under `LC_ALL=C`.** Plain `grep` wraps `ugrep -I`
  and silently skips the Latin-1/CRLF files.
- **Unit: occurrences, not lines**, and `code` excludes a line whose only match
  is inside a comment where `raw` counts everything — a 17-site difference, with
  one row two-thirds comment.

**HOW THE ROWS WERE FOUND, because patching a row at a time is how this got
wrong twice.** Every `Q[A-Z]…` token the 5 GUI modules name — 85 of them — was
diffed against every class Qt 6's `QtCore`/`QtGui`/`QtWidgets` headers declare.
What Qt 6 does not have is by definition Phase C work. **It found 8 classes no
row had** — `QIconSet` 47, `QListViewItemIterator` 15, `QWidgetStack` 10,
`QListBoxItem` 10, `QButton` 3, `QTextView` 2, `QScrollView` 1, `QWMatrix` 1:
**89 code sites, a fifth of the total, none of them mentioned anywhere in this
plan before C1**, and the reason the total moved *up* rather than down (534 code
/ 556 raw, against an earlier 466 that was the sum of its own rows rather than a
measurement). The sweep cannot see the silent collisions — `QList`, `QVector`,
`QListIterator`, `QQueue` all still exist in Qt 6 meaning something else — but
those are exactly what the containers row already covered.

**MATCH THE PATTERN TO THE QUESTION BEFORE CONCLUDING A COUNT IS UNREACHABLE.**
`QListView` measured as a **substring** over the stated scope returns 128 on the
pre-C1 tree — `QListView` 18 + `QListViewItem` 93 + `QListViewItemIterator` 17.
A `\b…\b` pattern silently drops the 15 live iterator sites, and a "correction"
of 128 to 106 on that pattern was the error, not the original. *That is §9's
characteristic failure inside the table that names it.*

**No tool touches any of this.** D19a measured that `qt3to4` leaves Qt 2's
`QList`/`QDict`/`QArray`/`QVector` completely untouched, and C1 confirmed `uic3`
is a *form* converter that never reads a `.cpp` at all.


**Steps — one module or one form each, per D23.** Forms come first because the
modules include the headers `uic` generates from them.

| # | Work | Size |
|---|---|---|
| C1 | **DONE 2026-08-30.** `SIG_GPParameterBase`, the only form with both an embedded image and dropped slots. Settles the residue table, the base-class question, and the build and check wiring — see below | 1 form |
| C2 | **DONE 2026-08-30.** The remaining 19 forms. All 20 are Qt 6; 19 of 20 generated headers compile, the 20th blocked on C3 — see below | 19 forms |
| C3 | **DONE 2026-08-30.** `SIGEL_CommonGUI` — the `QGLWidget` → `QOpenGLWidget` step; all 6 sites are here, 2 in code. In `check.sh` | 665 LOC, 2 sources |
| C4 | **DONE 2026-08-31.** `SIGEL_SlaveGUI` — the toolbar, the movie path, and ten connects to signals Qt 6 does not have. Unblocks the last form | 2,344 LOC, 7 sources |
| C5 | **DONE 2026-08-30, done FIRST — it is the dependency root.** `SIGEL_Visualisation`, 23 Qt 2 sites, 12/12 sources and headers compile, in `check.sh` | 3,564 LOC, 12 sources |
| C6 | **DONE 2026-08-31.** `MT_GUI` — 31 dead connects, the prepending page list, and a right-click that cleared the selection | 4,513 LOC, 23 sources |
| C7 | **DONE 2026-08-31.** `SIGEL_MasterGUI` — all **44 dead connects** repaired (§2), **23 prepending item sites**, the three owning `QDict`s, and 21 validators that would have read the decimal point by system locale. 29/29 sources and 29/29 headers compile; the module is in `MODULES` with a dead-signal baseline of 0 | 8,791 LOC measured (6,164 source + 2,627 header), 29 sources, 20 hand-written |
| C8 | **DONE 2026-08-31.** `sigel.cpp`, `sigel_slave.cpp`, `MT_Control`'s 15 dead connects, and the four core files no module list reached. **The tree's dead-signal count is now 0 with no non-zero baseline anywhere.** `check.sh` gained a `programs` section and a `dead item virtuals` check | 15 sites + 4 files |
| C9 | **DONE 2026-08-31.** All five GUI modules build as archives, both programs link and run. Exclusions lifted, moc derived from source, resources named on the link line, `programs` check upgraded from compile to link+run | 5 modules, 2 programs |
| C10 | **DONE 2026-09-02.** Driving the interface rather than reading it. `guidrive.cpp` posts real Qt mouse, key and context-menu events into the real `SIG_MainWindow`; the 1.3 oracle drove the 2003 binary with XTest and the two were diffed. Found the `clear()` signal regression that killed the application on a large delete, and the eaten ampersand in the MetaGP dialog. New `gui behaviour` check with `guibehaviour-baseline.txt`. **That section said no X-level click was possible on this machine; C13 withdrew that in 2026-09-07 and checked real XTEST input.** Also carries the port's FIRST deliberate divergence from 1.3 — the duplicate MetaGP About, removed by decision 2026-09-02 | 2 defects, 15 scenarios, 1 divergence |
| C11a | **DONE 2026-09-02.** The five View pages C10 never opened. 29 spin boxes, 20 sliders, 7 combos, 4 checkboxes, 8 radios and 20 of C7's 21 validators driven and diffed against 1.3. The **12-probe validator battery matches character for character**, on both sides under a comma-decimal locale the oracle built with woody's own `localedef`. Nine parameter values typed on the pages come out **byte-identical** in the saved `.exp` across the two architectures. Found the `QIntValidator` Intermediate/Invalid trap: 1.3 clamps a typed over-range number to the maximum, the port commits a truncated prefix -- **accepted as a divergence, D28**; pinned in the check. `gui behaviour` grew from one scenario to two here, and to five by C11c | 1 regression, 2 interlocks, 3 probe errors |
| C11b | **DONE 2026-09-02.** The Import/Export round trips — 15 of the 16 children C10 never drove. **Seven of the eight exports are BYTE-IDENTICAL to what the 2003 i386 binary writes**, including the 2.7 MB `.pop` and, unexpectedly, the `.dat` with its 532 lines of floating point. Found and fixed a real defect: default-constructed language parameters came out alphabetical where 1.3 gives `QDict` hash order — predicted from `qgdict.cpp`, confirmed on the running binary character for character, and it had been silently wrong in `dictorder-baseline.txt` for all 7 robots (**98 lines corrected**). Confirmed and preserved 1.3's ignored overwrite prompt. Also fixed a C10-era harness bug that had been handing SIGEL the wrong filename — 2 exports out of 32 in the runs that caught it | 1 defect fixed, 1 baseline corrected, 1 defect preserved |
| C11c | **DONE 2026-09-02.** The six dialogs, plus two fresh-eyes reviews of the harness and of `check.sh` itself. **C7's 21st validator driven, closing that set at 21 of 21.** Found and fixed a defect with a data consequence and no need for invalid input: Qt 6 selects a pre-filled field when a dialog gives it focus and Qt 2 did not, so a user who types one digit into Add-individuals gets **12 on 1.3, which appends, and got 2 here, which replaced** — twelve individuals added where two were meant. Four sites, `end(false)` queued after show. A second divergence **kept on purpose**: allowing a command appends where 1.3 hash-inserts, predicted and confirmed character for character, but matching it would mean reimplementing what Phase D removed. Two reviews then went at the checking machinery: **five probes that could not fail**, including a Cancel test that never pressed Cancel and a round trip that was an identity test; and a **demonstrated false pass** — all 30 menu and toolbar icons replaced with garbage, whole check green. `check.sh` also never exited non-zero, counted a skipped section as 0 fail, and ran its locale check under a locale that is not installed here. All fixed; **842 pass, 0 fail** for the first time, with `WIN_*` excluded explicitly as the permanent known failure it is. A `!!` failure marker had already reached the committed baseline; the check now refuses those | 1 defect fixed, 1 kept, 5 probe defects, 7 check defects |
| C11d | **DONE 2026-09-02.** `MT_GUI` — the MetaGP window, 23 sources, which nothing had ever opened on either side. Its six pages driven. **C7 never reached MT_GUI's ten validators**: an unpinned `QIntValidator(0,1000)` calls `"1,000"` ACCEPTABLE under en_US while `toInt()` returns 0, so a user types one thousand and **zero** reaches the system. 1.3 rejects both separators — measured — so the fix RESTORES Qt 2. Applied at both creation sites in `MT_AddConstantsWidget`, whose type radio rebuilds them. The window is application-modal, proven with a control (0 pixels vs 4397). The `!!` guard added in C11c caught a probe defect before it could be baselined | 1 defect fixed, 2 probe defects |
| C13 | **DONE 2026-09-07.** A real click on both ends. `guidrive` runs as a normal X11 client in a nested `Xvfb` under `QT_QPA_PLATFORM=xcb`, driven by XTEST through `xdotool`. The oracle measured the same three questions on the 2003 binary the same day, on its own `Xephyr` with no window manager. Withdrew this file’s claim that no tool here can deliver a real X-level click. That was measured on the live Wayland session only. Its pointer readback used `xdotool getmouselocation`, which reports the screen centre whatever the pointer does. **Found a difference that `QTest` could not have found**: the click that closes an open menu is swallowed here and forwarded by 1.3. Double click synthesis agrees, at 400 ms on both sides, with a slow-click control that opens nothing on either. Corrected one claim this file made about `QTest`: enter and leave was an offscreen platform limit, not a `QTest` limit. `QEvent::spontaneous()` is recorded as a trap, not withdrawn — this file never claimed it. New `real clicks` check with `xtest-baseline.txt` | 2 differences, 4 probe errors, 2 claims withdrawn |

Each module step is the same shape: `qt3to4` in the container, hand-port off
Qt3Support, extend `check.sh` to cover the module, commit.

**THE STEP ORDER DOES NOT RESPECT THE DEPENDENCY ORDER, AND THIS LINE USED TO
CLAIM IT DID** — "smallest first, which also happens to respect the dependency
order". Measured at C3 by walking every `#include "<Module>/…"` between the five
GUI modules:

| step | module | includes |
|---|---|---|
| C3 | `SIGEL_CommonGUI` | **C5** |
| C4 | `SIGEL_SlaveGUI` | **C3, C5** |
| C5 | `SIGEL_Visualisation` | none |
| C6 | `MT_GUI` | none |
| C7 | `SIGEL_MasterGUI` | none — measured; it includes no other GUI module |

`SIGEL_Visualisation` is a **root**; `SIGEL_CommonGUI` and `SIGEL_SlaveGUI` both
need it. C3 reads `visualisation->floatingTexts`, a Qt 2 `QVector` living in
C5's `SIG_Visualisation.h`, so C3 cannot compile before C5 without dragging
containers across the boundary and leaving C5 half-converted.

**Execution order is therefore C5, C3, C4, then C6 and C7 in either order.** The
step IDs stay bound to their modules — "C5" always means `SIGEL_Visualisation` —
because renumbering would strand every earlier reference. *"Smallest first" was
also measuring the wrong thing: C5 is the largest by LOC (3,564) and among the
smallest by work — **23 Qt 2 code sites** — 19 container-type occurrences in code plus 4
`setAutoDelete` — against C3's **6** on the same unit. *An earlier version of
this sentence gave C3 as 9, which is a different unit from the 23 beside it:
exactly what C1's note about stating the unit was written to stop.*

**C8 detail.** Both programs fail to compile on `qmotifplusstyle.h`. 15 sites:
4 style includes, 6 `setStyle` constructions (`sigel.cpp:193,195`;
`sigel_slave.cpp:278,280,346,348`), 2 `setMainWidget` (`sigel.cpp:199`,
`sigel_slave.cpp:283`), 2 three-argument `QApplication` (`sigel.cpp:191,255`),
and `sigel_slave.cpp:285 setCaption`. `setMainWidget` survived Qt 4 under
`QT3_SUPPORT` and was **removed in Qt 5**, not Qt 4 — so the Qt 4 stage does not
force it and the final hand leg does. `sigel.cpp:255`'s
`QApplication(argc, argv, false)` is the Qt 2 spelling of "no GUI", is
`QCoreApplication` in Qt 6, and is on the headless `-me` path.

**Also still Qt 2, in the 4 core files the build excludes: 14 sites**, not the
2 an earlier draft claimed — `MT_Controller.cpp` 10 (9 × `QString::null`, 1
`findRev`), `SIG_GUIGPManager.cpp` 2, and 1 `QInputDialog::getDouble` in each
ZORC file. They ride along with C6 and C7.

**C7 context — rewritten 2026-08-27 after T2 measured it.** The previous text
here was wrong on its central claim; see Phase T above. `uic3 -convert` handles
the Qt 2 → Qt 4 XML in one step, including the property nesting and the
`QLayoutWidget` flattening it described. What is left per form is the 6-class
substitution in the Phase T table, and checking the connections: `uic3` discards
custom signals and slots, and there are 49 across the 20 forms.

#### Phase C — what it found, and what it left behind

**Every step is DONE and enforced by a compiler that no longer accepts the Qt 2
alternative.** What follows is what is still actionable: the defects, the
deliberate divergences, the checks and their blind spots, and the traps. The step
table above is the index.

##### The defects Phase C found

**Twenty rows below, plus C12's three, and all are fixed except `QIntValidator`
out of range, which D28 accepted.** **Not one was found by compiling** — the
column that matters is *how*, and **nine of them were found only by DRIVING the
interface**: C8's dead `key()`, C9's `truncate(-1)`, C10's two, C11c's pre-filled
field, C11d's ten unpinned `MT_GUI` validators, and C12's three.

| # | defect | how it was found |
|---|---|---|
| C2 | `QButton::stateChanged(int)` does not exist in Qt 6; string-based connect compiles and fails at run time, so the floor-selection UI silently stops responding | sweeping all 49 form connections against Qt 6 |
| C4 | **`QAction::activated()` is `triggered()` in Qt 6** — nine dead connects in two files, plus `QComboBox::activated(const QString&)` → `textActivated`. The slave's toolbar drew six buttons and **not one did anything** | tripping over two, then resolving every signal signature in the tree through `QMetaObject::indexOfSignal`: **nine kinds are dead, 100 connects**. §2 has the table; `$DEAD_SIGNALS` checks it at baseline 0 |
| C5 | **Qt 2's `QVector::insert` overwrites slot `i`; Qt 6's `QList::insert` shifts.** `SIG_SimulationVisualisation.cpp` came out twice its size with a null tail, which C3's `paintGL` indexes and dereferences — live code, 1785 hits. *The fill loop is at `:66-68` today; the `:63` this row used to cite is now the comment the fix added, which is what line references do* | review. *The sweep that missed it was keyed on the container **type**, not on the operations performed on it* — §9's characteristic failure, in a step whose own table cites D25c's insert/shift row |
| C6 | Qt 2's `QListView::clear()` blocked signals across its whole body (`qlistview.cpp:2303-2304, 2341`); Qt 6's does not, so `currentChanged` arrives with `nullptr` into a slot with no guard. `onShow()` opens with `clear()` and runs on every page raise | review after the step was committed |
| C6 | `Q2Queue::head()` on an empty queue returned **0** (`qqueue.h:62`); Qt 6's `QList::first()` segfaults in a release build. `MT_ExperimentWidget::lastSelected()` is reachable on the **first refused page switch** | the same review. *The first sweep looked for slots dereferencing a pointer parameter; this one dereferences a container's front element* |
| C6 | All four toolbars floated and eight widgets were unlaid-out overlays — Qt 2's `QToolBar` ctor docked itself and `init()` did `boxLayout()->setAutoAdd(TRUE)` | review. *C4 got this right for `SIG_SimulationWindow`; C6 did not* |
| C6 | `DISpinBox` threw away everything after the decimal point: Qt 6's `QSpinBox::validate()`/`fixup()` are an **integer** parser running *before* the virtual `valueFromText`, where Qt 2 called `mapTextToValue()` unconditionally. Affects `toleranceSpinBox`, `powerSpinBox`, `lineProbSpinBox` — **GP configuration values** | review; and **the first fix was locale-dependent where 1.3 never was**, making it strictly worse under a comma locale |
| C7 | **A Qt 6 `QAction` belongs to at most ONE `QActionGroup`** — `addAction` calls `setActionGroup`, which **evicts** it from the previous group. 30 actions went into `noExperimentActions` and 23 were later added to `evolutionRunningActions`, so "no experiment loaded" disabled **7 of 30**, leaving every import, export, save, rename and individuals action live | review, measured on Qt 6 |
| C7 | The toolbar showed the long menu label. Qt 2 kept `text` and `menuText` distinct and put **`text`** on the button (`qaction.cpp:220`); Qt 6 has one `text()`. The short label is `iconText()`, now on all 36 actions | review, verified by probe |
| C7 | A repeat click on the active MetaGP system re-entered `switchSystem()` — Qt 2 emitted `selected(s)` only on an actual change, Qt 6's `triggered(QAction*)` fires every click | review; the oracle then measured the click as a **complete no-op**, 0 differing status-bar pixels |
| C7 | `SIG_TextView`'s three `QScrollView::viewportMouse*Event` overrides have **no Qt 6 virtual at all** — they would have compiled, linked and never been called, and drag-scrolling would have gone silently dead | reading the base class. Dispatched from `viewportEvent()`, which must return `true`: 1.3 overrode them without chaining, so Qt 2's own drag-selection never ran |
| C8 | **A virtual that silently stopped overriding anything.** `SIG_IndividualListItem::key(int,bool)` zero-pads so the list sorts numerically — 1.3 shows `0 1 2 10 11 100`. **Qt 6's `QTreeWidgetItem` has no `key()`**; it sorts through `operator<`. The moment C7 changed the base class the override became dead code that still compiles and looks correct: raw items sort `0 1 10 100 11 2` | the oracle, reading the running 1.3. `check.sh` gained **`dead item virtuals`** |
| C8 | `QTextStream::read()` ran an unconditional `s/\r\n/\n` (`qtextstream.cpp:1531`) with no dependence on open mode; Qt 6's `readAll()` returns raw bytes unless opened `QIODevice::Text`, and the call site opened plain `ReadOnly` — so CRLF was copied into saved `.mexp` files where 1.3 wrote LF | review. *Same failure mode as the D6 CRLF losses, arriving through an API rename rather than an editor* |
| C9 | **`truncate(-1)`.** `SIG_IndividualListItem::key()` does `orgString.truncate( positionOfExponent )`, which is **-1** with no exponent. Qt 2's `truncate` took a **`uint`** — -1 wrapped huge, a no-op; **Qt 6's takes a signed `qsizetype` and clears the string.** Every plain value lost its mantissa and got an all-zero key | diffing the port's fitness sort against 1.3's. Identical source, opposite behaviour, from a parameter changing signedness |
| C10 | **Deleting most of the pool killed the application.** `clear()` emits `itemSelectionChanged()` while `currentItem()` still points into the *old* pool; `getIndividual()` prints "Wrong Position requested from Population!" and calls **`exit(1)`** — 1.3's own code | driving it. Confirmed with the oracle **before** changing anything: 1.3 deletes 113 of 118 and survives, same PID, nothing on stderr |
| C10 | The MetaGP dialog ate an ampersand — Qt 2 treats `"&x"` as a prefix only when `x` passes its own `ISPRINT`, `((x).row() \|\| (x).cell()>' ')` (`qpainter.cpp:2317`), and **a space fails it**. Qt 6 has no such exclusion | driving it; verified by pixel-comparing three rendered buttons, not by reading code |
| C11a | **`QIntValidator` out of range: Intermediate in Qt 2, Invalid in Qt 6** — see the TRAP below. **ACCEPTED as a divergence, D28** | typing into it |
| C11b | Default-constructed language parameters came out **alphabetical** where 1.3 gives **hash order**. Reachable through `Import > Robot` and `New Experiment`, neither on a load path, so no shipped `.exp` exercises it. **It had also corrupted `dictorder-baseline.txt` for all 7 robots — 98 lines** | **predicted** from `qgdict.cpp` before asking, then confirmed on the binary character for character. §7's rule is why the baseline was wrong: *every check in this repo compares the port against itself* |
| C11c | **A pre-filled field that 1.3 does not select and Qt 6 does.** SIGEL does `setText()` then `setFocus()`; Qt 6's `QLineEdit` selects on focus down the tab chain and Qt 2's did not. Edit Command `0.01` + `5` gives `0.015` on 1.3 and `5` here; **Add Individuals `1` + `2` gives 12 on 1.3 and gave 2 here — twelve individuals added where two were meant** | driving it. **The third needs no invalid input and leaves nothing wrong-looking behind** |
| C11d | **C7 never reached `MT_GUI`'s ten validators.** No foreign locale needed: an unpinned `QIntValidator(0,1000)` under `en_US` calls `"1,000"` ACCEPTABLE while `"1,000".toInt()` returns **0** — a user types one thousand and zero reaches the system | opening the window. 1.3 rejects both separators, so the fix **restores** Qt 2 |

##### The divergences from 1.3, all deliberate

**This is the list to check before calling anything a regression.** Everything
else that stops matching 1.3 still needs justifying as a defect.

| divergence | why it is not fixed | pinned by |
|---|---|---|
| **`QIntValidator` out-of-range clamp — D28.** 1.3 clamps a typed over-range number to the maximum; the port commits the truncated prefix | reachable only by typing outside the box's own range, and the differing value is visible before anything is saved. The cost is not the code — a 25-line `QSpinBox` subclass, in `future_refactorings.md`'s history at `1dba5f4`, reproduces all four readings — but **owning a custom widget forever**, promoted across 29 spin boxes in three forms and then the dialogs | the `commits=` value in `guibehaviour-baseline.txt`. **Measured 2026-09-03: 44 `spin` lines, of which 40 carry a committed value — 29 on the five pages and 11 more in the MetaGP block, which C11d added after C11a wrote the TRAP.** Four are not pinned: three survey-only lines (214, 1347, 1399) and one reading `commits=not-pressed(in a dialog)` (1404). *The TRAP block below still says "the other 18 … are not pinned". It is preserved as C11a wrote it and is out of date twice over — see the correction beneath it — and **the 18 does not reproduce as any real population either**: the 20 forms declare 46 spin boxes, 29 of them exactly the three page forms, so the non-page count is 17 in the forms and 20 counting the three declared in code. Do not carry the 18 forward.* |
| **A run that loses PVM ends and says so — D41.** 1.3 waits for a message that cannot arrive, for ever, with the interface still showing a run in progress | `pvm_probe` reports `PvmSysErr` and the trainer records it; the interface ends the run the way `Stop` does | none. No check starts a run. Measured by hand: SIGKILL the daemon mid-run and the dialog appears |
| **A run that ends at once says why — D42.** 1.3 shows nothing, so Start looks as though it did nothing | the message names the termination setting that ended the run and the tab it is on | none. No check starts a run. Measured by hand for a date and for a duration |
| **The duplicate MetaGP About is removed**, with its trailing separator. 1.3 has 43 menu items, the port 42 | a decision, 2026-09-02 — the port's first intentional behavioural difference. Not a defect, not a regression | `guidump-baseline.txt` |
| **`.lap` export order.** 1.3 does not round-trip its own order — `Q2Dict::insert` prepends, so re-inserting in iteration order reverses every colliding chain (three pairs and one triple; eight of thirteen commands move). The port is the stable one | §10 already says load and save become order-preserving once the shim goes. Restoring it means reimplementing `Q2Dict`'s bucket permutation on read — writing the defect back in | measured on the running binary, C11b |
| **The robot block in a saved `.exp` is a fixed point here and an involution on 1.3.** Same cause as the row above, on the larger surface: load and save is the identity for us, and a permutation that undoes itself on the second save for 1.3. Measured over four states of `hammer` 2026-09-08 — our md5 never moves; 1.3's toggles, `state0 == state2` and `state1 == state3`. Three things toggle on hammer: material order, `Body` emission order, and `middle3`'s axis points. On `octopus` it is joint, drive and sensor order, the body order and the command list, from V1 | D3, deliberate. Same answer as the row above — matching it means reimplementing `Q2Dict`'s bucket permutation on read. **It has not bitten anyone yet, and here is when it would:** a robot written by one side and read by the other keeps every name and every mesh, and only their order moves. That order composes with V7's friction rule, so read V6 RESULT before revisiting D3 | `v2 round trip vs 1.3` pins OUR side as a fixed point on both robots; `v8-1.3-gp-blocks.txt` result 6 and `v1-1.3-roundtrip.txt` hold 1.3's |
| **A newly-allowed command appends** where 1.3 inserts at its **hash position** (allowing JMP lands it sixth, straight after LOAD, on 1.3) | the insertion **point** depends on runtime hashing, so matching it means reimplementing what Phase D removed. *Distinct from the `.lap` case, where the port had a free choice of a **static** order and 1.3's was reproducible for nothing — so that one was fixed* | C11c |
| **The movie records frame N where 1.3 recorded frame N−1.** Every caller runs `makeTimeSteps(n); update();` with the grab **inside** `makeTimeSteps`, so Qt 2's `grabWindow` captured the last *presented* frame while the simulation stood one step further; `grabFramebuffer()` renders current content | forced, not chosen — Qt 6 removed `QPixmap::grabWindow`, and for a `QOpenGLWidget` it was wrong anyway, reading on-screen pixels where the widget renders into an FBO. **Raster path only**; `fileFormat == "pov"` is a different branch, and per the oracle the POV export produced every published film | nothing can see it |
| **HiDPI movie frames are resampled.** `grabFramebuffer()` returns device pixels; 1.3's frames are logical | normalised back with `scaled(size(), …)` + `setDevicePixelRatio(1.0)`. At ratio 1.0 the branch does not run and the bytes are unchanged | — |
| **The Motif look is gone.** Qt 6 ships exactly two styles here (`QStyleFactory::keys()` → `Windows, Fusion`); the X11 branch takes Fusion | unreproducible; recorded rather than papered over | — |
| **`-geometry` and `-title` are gone from Qt entirely**, so `setMainWidget`'s third job cannot be restored | unreproducible. `quitOnLastWindowClosed` covers the "closing ends the app" half | — |
| **Descending-sort tie order** on tied Age/Fitness values. Qt 2 sorted ascending with `qsort` then **reversed the sibling list**; Qt 6 inverts the comparator inside a *stable* sort | `qsort` is not stable, so Qt 2's tie order was never defined in the first place — reproducing it means reproducing a specific libc | — |
| **Two validator inputs**: Qt 2 ran `strtod` and required full consumption, so `" 9.81"` and `"0x10"` (glibc hex float = 16) were Acceptable; the pinned Qt 6 validator returns Invalid | matching `strtod` exactly means a hand-written validator on all 21 sites. **The hex case cannot be reproduced regardless** — Qt 6's `QString::toDouble("0x10")` *fails*, so accepting it would store 0 where 1.3 stored 16 | C7 measured all thirteen probe strings; the rest agree |
| **The Save dialog pre-selects** where 1.3 pre-fills and leaves the text unselected, so typing appends | neither dialog is built by SIGEL — both are `getSaveFileName` — and §7 already lists file-dialog behaviour among the accepted divergences. **Rename, which SIGEL does build, pre-selects in both** | — |
| **Icons whose two pixmaps straddle the chosen size are resampled.** Qt 2 named Small and Large explicitly and blitted each at its own size; a Qt 6 toolbar has one `iconSize` and picks from the `QIcon` by pixel size | removing it means splitting every `QIcon`. Sizes were re-measured across every referenced XPM — **five distinct sizes each** — and set to 25×25 / 48×48, the measured maxima, so nothing is enlarged past what 1.3 drew | — |
| **1.3 drops a spin box's suffix while editing; this port keeps it.** The register-width box reads `3 bit` at rest, plain `100` during typing and `99 bit` after commit on 1.3; here it reads `10 bit` throughout | Qt 2's `updateDisplay()` wrote prefix + text + suffix into the line edit unprotected, where Qt 6's `QAbstractSpinBox` keeps them out of the editable text | **Recorded, not chased.** No value differs; only what is on screen mid-edit |
| **Clicking the outer edge of a ticked slider pages on 1.3 and does nothing here.** Qt 2's Motif slider treats the WHOLE widget as clickable — the oracle got a clean page step at all twenty of `yawSlider`'s cross-axis offsets — where Qt 6 honours the groove sub-rect only: on `yawSlider`, y=3,5,7,9 page it and y=1,11,13,15,17,19 do not | a Qt framework behaviour rather than anything the conversion did. Nobody is likely to notice, but it is a fidelity difference | §7's probe-craft list: **take the cross-axis from `SC_SliderGroove`, never from the widget's middle**, and populate the `QStyleOptionSlider` fully — `tickPosition` unset makes `subControlRect` return a tickless groove. *Added here 2026-09-03: the divergences table was billed as complete and omitted this one* |
| **Overwrite on save and export — D35.** 1.3 asks "File exists... Do you want to overwrite?" after the file dialog, and in five exports and `slotRobotSave` writes the file whatever the answer. The port does not ask: the file dialog's own confirmation asks when the chosen name exists, and a name to which `checkEnding` adds the extension gets a date stamp when that file exists | a decision, 2026-09-15 | `overwrite` in `gui behaviour`: a date-stamped file and `sentinelSurvived=1` without the extension, `childOfTheFileDialog=1` and `sentinelSurvived=1` after No |
| **The generation counter moves during a run — D37.** 1.3 has the display call in `SIG_GUIGPManager::updateIndividualView` commented out, so on 1.3 the counter moves during a run only when `SIG_ExperimentView::putIntoExperiment` runs | a decision, 2026-09-15 | not checked: no check starts a run. The unchecked `evolution` scenario samples the counter during a run |
| **The generation counter is refreshed when a run ends.** 1.3's `slotEvolutionStopped` does not write the counter, so it keeps the last value `SIG_ExperimentView::putIntoExperiment` wrote until that function runs again | a decision, 2026-09-15 | `runlock` in `gui behaviour`: the `[counter]` lines |
| **A click that closes an open menu is swallowed here. 1.3 passes it on.** On 1.3 one real click closes the File menu and selects the list row under it. Here the menu closes and the row does not move, so the user must click again | Qt’s own popup handling, not SIGEL code. The Qt 2 side is in the vendored source: `qapplication_x11.cpp:3402-3416`, in `QApplication::closePopup`, calls **`XAllowEvents(…, ReplayPointer, CurrentTime)`** when the last popup closes on a press outside it. The X server then delivers that press again to the window below. The same code subtracts 10 s from `mouseButtonPressTime`, so the repeated press cannot count as a double click. Qt 6 does not do this. That half is measured, not read, because Qt 6’s sources are not on this machine. Matching 1.3 means overriding popup dismissal for the whole application, which is D28’s "owning a custom widget forever" applied to every popup. The swallowing behaviour is also what every modern toolkit does | `xtest-baseline.txt` section 4, with the control click printed below it |
| **A second click on the same menubar item closes the menu here. On 1.3 it stays open** | Same cause and same answer as the row above. Qt 6’s menubar toggles on a second click and Qt 2’s did not. Nothing in SIGEL decides it | `xtest-baseline.txt` section 4 |

**Three 1.3 defects preserved on purpose**, plus the one below them. `MT_GUI`'s
gnuplot export puts a constant x on datasets `2pt destr.` and `3pt destr.`
(pre-standard `for` scoping); `MT_PopulationWidget`'s save-individuals loop never
advances the cursor and writes the same individual into every file;
`callRenderPixMap` assigns `res` from `save()` and then `return true`
unconditionally, so a failed frame write is reported as success and the caller's
message box is dead code — **which is why that module carries a
`-Wunused-but-set-variable`**. And one that is checked: **saving grows the file**
by exactly **840 bytes = 120 × 7**, one `"      \n"` per individual per save,
because the reader takes everything between `HISTORY BEGIN{` and `}HISTORY END`
as ONE string and the writer re-emits it before a fresh terminator. The
overwrite prompt that asked and then wrote anyway is gone; see the D35 row in
the table above.

**And one that is not a defect at all, but is the easiest thing here to break
by accident: 1.3's TRUNCATED PI.** The sensor path converts radians to degrees
with `3.14159265`, so its factor is `180/3.14159265` and not `180/pi` — off by
1.14e-09. Writing `M_PI` shifts every joint-sensor reading, and the shipped
experiments hold programs that were evolved against readings carrying that
error, in a chaotic simulation. **The correct constant is absent from both 1.3
binaries and from all of ours**, which is the measurement rather than an
inference. The edit is one word, and 1.3 itself uses the true
`M_PI` in `IFunctions.cpp, calculateAnyJoint`, so the truncated literal reads as an
oversight. Checked as `truncated pi (V5)`, in
the source and in the built binary. Evidence in
`verification-against-sigel-1.3/v5-1.3-mdh-compared.txt`; the same file records
that `moveDrive`'s `long double` arithmetic **cannot** be bit-compared against
1.3, because this machine's `long double` is 128-bit quad and the i386 x87 one
is 80-bit.

##### The checks Phase C left, and the two that had no teeth

| check | what only it can see |
|---|---|
| `forms` — seven checks a form | a dropped `<images>` block; a `Line` that lost its `orientation` and now draws nothing; a `.qrc` disagreeing with the header **in either direction**; a view enabling sorting without pinning the direction |
| `$DEAD_SIGNALS`, baseline **0 for every module** | **only the nine Qt 2 spellings §2 tabulates, and only inside `SIGNAL(`.** It is a closed regex (`check.sh:89-92`), so a **tenth** kind of dead signal is invisible to it and a `SLOT()` naming a slot that no longer exists is invisible by construction. *Demonstrated 2026-09-03: injecting `connect(…, SIGNAL(highlighted(int)), …)` — Qt 2's `QListBox` signal, which `QTreeWidget` does not have — left `Qt 6 signals` at `0 dead (baseline 0)` and every other section green, while Qt itself printed `QObject::connect: No such signal QTreeWidget::highlighted( int )` on stderr. The positive control is that runtime line: the injected connect really was dead.* C7 measured every `SLOT()` in `SIGEL_MasterGUI` against its headers by hand — 0 mismatches — and nothing repeats that |
| **`runtime connect`** — NEW 2026-09-03 | Qt's own `No such signal` / `No such slot` warning, from a driver run — the only thing that catches a dead connect the regex has never heard of. `guidrive`'s stderr had been going to `/dev/null`; it is captured now and any such line fails the check. **It is NOT the general case the regex is a special case of, and a first draft of this row said so wrongly**: the regex is *static* over all 14 modules, this is *runtime* over only what the **ten** `gui behaviour` scenarios execute. **Partly disjoint, so both are kept.** What THIS row does not reach: the `clipcheck` run still discards stderr, and the two locale re-runs append to the log *after* it has been grepped. *`SIGEL_SlaveGUI` used to be the largest hole here — 44 `SIGNAL(` and 44 `SLOT(` with no runtime coverage — and is now covered by `slave gui` below, which runs its own copy of this check on its own stderr. There are three such checks in `check.sh` now, one per stderr stream, and they are deliberate duplicates: each stream has a different producer* |
| its **positive control**, and the reason the row above is not simply "and now it is covered" | **Qt emits that warning under the logging category `qt.core.qobject.connect`, and categories are filterable.** `QT_LOGGING_RULES='*=false'` in the ambient environment — `guidrive_run` uses `env` without `-i`, so everything passes through — or a `qtlogging.ini` silences it, and an empty stderr then looks exactly like a clean run. **Demonstrated: the full check ran green with a genuinely dead connect injected.** So `guidrive` now makes one deliberately bogus connect at startup and the check **fails if that warning is absent**. It must be a real connect, not a `qWarning`: `qt.core.qobject.connect` can be disabled on its own, leaving a `default`-category control visible and the check blind. *`check.sh` already says of `clipcheck` that "0 clipped" from a check that cannot detect clipping is worth nothing; this row is that sentence applied to itself* |
| **`pagesave`** — NEW 2026-09-03; named `pagesave vs 1.3` until 2026-09-19 | **`putAllIntoExperiment()`, the widget-to-file path, which nothing covered.** `pages` is widget to widget; `exportall` is widget to file for the eight export formats, and **none of the three compares what the SAVE path writes**. *A first version said the export scenarios "bypass the aggregator entirely"; they do not — `putAllIntoExperiment()` runs on every tree selection change, so `check`, `pages` and `exportall` all execute it many times. What none of them did was compare its output to a file.* Teeth-tested three ways: breaking the register write and the `WITHHISTORY` write both fail it — *but `gui behaviour` catches those too, through the `.lap` and `.pop` export checksums, so neither shows unique coverage.* **Dropping one page from `putAllIntoExperiment` does**: `pagesave vs 1.3` fails on `TEXALPHA 99 → 255` while `gui behaviour` passes 1/0. That is the gap §9 named, exactly |
| **`roundtrip`** — NEW 2026-09-03, the other half of it | **that a READER actually reads.** Export, change something, import the export, export again, require the two exports to match — so an importer that opens nothing, parses nothing or is never reached cannot pass. `exportall` cannot see this: it never reads anything back. **Teeth-tested by gutting each of the five readers in turn — `.gpp` `.sip` `.lap` `.env` `.pop` — and every one fails the check.** *Checking it is what exposed that it could not fail* |
| **`freed-pointer null`** | that `visualisation` is nulled between its `delete` and its `new`. `SIG_Simulation`'s `default:` case throws for `SIMULATIONLIBRARY 0` and unwinds out of `visualizeThis()` between the two statements. **The defect is real but not reachable today, and the argument matters because the first version of it was wrong in three ways.** The widget has 21 `visualisation->` dereferences behind 14 `if (visualisation)` guards, all reached constantly (`paintGL` runs every frame). `sigel_slave` is **not** the sole caller — `SIG_SimulationVisualisationWidget::slotStopSimulation()` calls `visualizeThis()` from a live Stop action, **inside `a.exec()` with no try/catch**. And `visualisation` is a **base-class** member that `~SIG_VisualisationWidget` deletes, so destruction after a throw **is** a double free; it does not bite on the slave path only because `simWindow` is leaked past the catch's `return 1`. **Why it is still unreachable:** every other caller is downstream of a first `visualizeThis()` that must have SUCCEEDED, and nothing in the slave calls `setSimulationLibrary`. **Deliberately NOT generalised** — the tree's other `delete x; ... x = new` pairs are benign, so a blanket rule would be noise. Teeth-tested by deleting the line |
| **`no clipped controls`** (`clipcheck`) | any widget whose rect leaves its parent's, over the master's **six View pages and their tabs only** — and it carries a positive control that FAILS the check if it does not fire, because "0 clipped" from a check that cannot detect clipping is worth nothing. Shrinking the window is not usable as that control: the converted pages carry real layouts and reflow where 1.3, absolutely positioned, clips — so **the port is better behaved than 1.3 on resize** — and it displaces a real widget instead |
| **`slave gui`** (`slavegui`) — NEW 2026-09-05 | **two things `check.sh` had never reached, because it never ran this scenario.** (a) The clipping defect **in the two containers where it was actually found**, which `clipcheck` structurally cannot see: they belong to the slave window and its movie dialog, not to anything the master's menus open. *Both fixes were UNCHECKED while this table said the section above covered them — found by review.* It greps the two clip totals rather than diffing the whole scenario, whose GL view does not render offscreen. **Teeth-tested by deleting each `<minimumSize>` block and rebuilding**: without `GroupBox6`'s, 14 controls clip; without `groupboxDirectory`'s, 1. (b) **`SIGEL_SlaveGUI`'s 44 `SIGNAL(` and 44 `SLOT(` sites, which had no runtime coverage at all** — its stderr is kept and checked for Qt's `No such signal`/`No such slot`, behind the same `guidriveStderrControl` positive control. **Teeth-tested both ways**: `QT_LOGGING_RULES='*=false'` fails it as suppressed, and renaming one live signal in `SIG_SimulationWindow.cpp` fails it by name |
| **`real clicks`** (`xtest`, `xtest-baseline.txt`) — NEW 2026-09-07 | **the platform layer, which nothing else here touches.** Every other section drives Qt through `QApplication::notify`. This one runs `guidrive` as a real X11 client in a nested `Xvfb` under `xcb`, and sends XTEST input with `xdotool`. It is the only section that exercises activation, the popup's pointer grab and Qt's double-click synthesis. It found C13's swallowed dismissing click on its first run. **Its control is inside the scenario, and the section fails without it.** The scenario compares one real click and one `QTest::mouseClick` at the same point, through a native event filter. It prints `DISCRIMINATES` only when the real click produced native `ButtonPress` events and `QTest` produced none. *Teeth-tested. `xdotool` was replaced by a stub that exits 0 and does nothing. The scenario stops at the coordinate check with a line-initial `!!` and exit 1, so the section fails on three predicates. **An earlier version of this row claimed it failed "on the control and on the `!!` marker", and review showed that was false**: all three of the scenario's mis-target messages put their `!!` in the MIDDLE of a line, and `check.sh` greps `^ *!!`, so not one of them was visible. A run whose own output said the finding was undecidable passed every guard the section had. The markers start their lines now, and the scenario returns 1 rather than carrying on.* `QEvent::spontaneous()` would not work as that control, because `qtestmouse.h` marks QTest's own events spontaneous. A missing `Xvfb` or `xdotool` **fails** rather than skips. The display is refused if something is already on it. The server is killed by pid, so a real session's own `Xvfb` survives |
| **`truncated pi (V5)`** — NEW 2026-09-08 | **that nobody "fixes" 1.3's truncated pi.** The sensor path converts radians to degrees with `3.14159265`, not `M_PI`. Every evolved program in the shipped experiments was selected against sensor readings carrying that 1.14e-09 error, and they feed a chaotic simulation, so correcting it changes what the robots do. **The edit that breaks it is one word and looks like tidying**, and 1.3 uses the true `M_PI` in `IFunctions.cpp, calculateAnyJoint`, so the truncated literal reads as an oversight to anyone who meets that line first. **Two checks, because neither covers the other:** the SOURCE check catches an edit at one of the four sites even while another site still supplies the constant, which no binary search can see, and it is compiler-independent; the BINARY check catches any spelling that yields the true value — `M_PI`, `4*atan(1)`, a longer literal, a header constant — which a grep for `M_PI` would miss. Only the radian factor is checked, and the reason is measured rather than assumed: on aarch64 four of the other seven appear ZERO times as 8-byte doubles in our image and two appear only in debug sections, so there is nothing of theirs in `.rodata` to compare. Costs 0.18 s. *Teeth-tested six ways, and the testing found three defects in the check itself. **The binary search covered the whole file, so its "the constant is missing" arm could never fire** — the Makefile compiles with `-g`, so two debug copies survive any patch of the real one; the search is bounded to `.rodata` now. **The source pattern was a prefix match**, so lengthening a site to `3.14159265358979` changed the factor while the count stayed at 4 and neither forbidden double appeared — the whole section passed on that edit. **And a comment mentioning `M_PI` or the literal failed the check**, which is documentation, not a defect; comments are stripped now. The six probes: a site tidied to `M_PI`, a site deleted, a site lengthened, the true `180/pi` patched into `.rodata`, the kept constant patched out of `.rodata`, and a comment naming both. Five fail with the message aimed at them, one passes. Two orderings had to be fixed for that: `M_PI` is tested before the site count, and the forbidden constant before the missing one, because each of those edits trips both tests and the specific diagnosis has to win.* |
| **`v2 round trip vs 1.3`** — NEW 2026-09-08 | **a whole experiment through `File > Save Experiment`, twice, against what the 2003 binary wrote.** `pagesave` compares a 178-line parameter block, over two saves that differ only in whether the pages were edited; this compares the WHOLE file across two CHAINED saves, where each save's output is the next one's input — marker line numbers, the experiment history, the per-individual HISTORY growth, the individual names, the robot block, the ten first-save keys, and `expstruct.py` over pass 1 against pass 2. The expected text is copied from `verification-against-sigel-1.3/v8-1.3-gp-blocks.txt`, captured before this conversion existed, so a failure is a regression against 1.3 rather than against yesterday — except the markers, the one-entry host list and the robot-block hash, which pin the file in `experiments/` since item 39. **Input against pass 1 is not the test** — the first save adds ten keys and would fail however correct the port is (V8 result 5). It is ONE diff of a 58-line report. Costs 35 s measured, four `pagesave` runs over two experiments; no new scenario was added. *Teeth-tested 2026-09-08, and the testing found two holes in the check itself, both since closed — see the V2 row above. Every predicate has been shown to fail on a change of the kind it exists to catch. The claim is one-way: a mutation moves the line it is aimed at, and usually others too, because a deleted key shifts every marker below it. It is NOT that each mutation moves exactly one line, which an earlier version of this row claimed and which the measurements never showed. The wrapper was tested too: missing data SKIPS and counts, a missing or stale binary FAILS, suppressed Qt connect logging FAILS, and the section was run from outside the repo root to check the `make -q -C` fix.* |
| `encodings` | **INVERTED BY D31 2026-09-09 — this row used to say the opposite.** It no longer catches *a file whose CRLF was stripped*; it catches **CRLF present at all**, in any tracked text file, expected zero. 571 LF-only files and 29 that git calls binary (2026-09-20). Baseline 0, floor 500. *Two counts went the same day — how many files had been converted from the German character set, and how many postdate the first commit. Neither could fail, and after the source tree was renamed neither could find a file* |
| `dead item virtuals` | a class declaring Qt 2's `key(int,bool)` without the `operator<` that replaces it. Matched against a **flattened** header and demanding the signature that actually overrides — a decoy `operator<( QTreeWidgetItem * )` and a two-line declaration both bypassed the first version |
| `widgets` | `DISpinBox` losing the fraction, under **`C` and `de_DE`** — without the second row it was blind to the locale bug the first fix introduced |
| **`expstruct selfcheck`** | that the structural fingerprint is **blind to fitness and sighted on structure** — nine assertions: both spellings of fitness in both float and integer form, a program-operand change that must move `PROGRAMS`, two individuals swapped that must move `ORDER`, and a structural floor recomputed from the raw bytes (individual count, total program lines, history length against `POOLGENERATION`) that catches a matcher which died and dumped its content into `SHAPE`. Costs 0.34 s. *Teeth-tested by disabling both fitness filters and by blinding the program matcher* |
| `parsers` | a file format the program parses but no other check opens: a `PVMHOST` round trip against `SIG_GPPVMHost`, pinning 1.3's own line. *Written because a defect of exactly that shape was found in `SIGEL_GP`* |
| `programs` | a missing moc, an unemitted vtable, a dropped resource, the wrong `SIG_GPExperiment`. It ran `-fsyntax-only` and could see none of them; it now requires both binaries built and current and runs the slave's headless smoke test, **failing rather than skipping** when they are absent |
| `gui vs 1.3` (`guidump-baseline.txt`) | an accelerator that went missing, an action that stopped being greyed, a toolbar showing the long label, a sort key that stopped being numeric. **Because the baseline was diffed against the running 1.3, a failure here is a regression against 1.3, not against yesterday's output** |
| `gui behaviour` (`guibehaviour-baseline.txt`, **ten** scenarios: check, pages, exportall, overwrite, dialogs, metagui, roundtrip, metadrive, runlock, rngseed) | everything C10–C12 drove, plus **two** `pages` re-runs required to be **identical** to the ambient run — *a check that needs no baseline of its own, which is why it is worth having*. **Two locales, because either alone is a tautology**: `de_DE.UTF-8` exercises Qt's half — `QLocale` reads the environment directly and reports a comma decimal even where no such locale is GENERATED, which is what C7's validator pinning is tested against — but `setlocale()` then fails and `LC_NUMERIC` stays `C`, so libc's half is never touched. **`en_DK.utf8` is a comma-decimal locale that IS installed here**, so libc really switches and a stray `sprintf("%f")` or `strtod` shows up only there |

**`check.sh` was reviewed again 2026-09-05 and four things were wrong with it.**
Each was demonstrated, not argued:

- **`no clipped controls` did not cover either fix it was credited with** — see
  its row above. Closed by the new `slave gui` section.
- **`guidrive` was scored STALE by the two clip sections**, which run 240 lines
  before the `gui behaviour` section that builds it. So the first `./checks/check.sh`
  after editing `guidrive.cpp` — or anything it links — measured yesterday's
  binary there and today's binary later in the same run. **And `make -q`
  with no target answers for `all`, which does not
  depend on `guidrive`: it reports up to date while the binary is stale.** Name
  the target. The clip sections now build it first.
- **`expstruct.py`'s float net had a real hole, and the probe for it failed on
  first run against the shipped file.** `DROP` matched `FITNESS=` and
  `Fitness (Elter n):` but **not `Fitness Value:`** — 1720 of the fitness values
  in a shipped experiment against 100 `FITNESS=` lines — so `FLOAT` was its only
  net, and `FLOAT` needs a decimal point. **An integer-valued `Fitness Value: 0`,
  which is exactly what a swallowed-throw individual scores, reached `SHAPE`.**
  `DROP` now matches it, and `--selfcheck` gained the integer probe that caught
  it: the asymmetry was that `FITNESS=` had both spellings probed and this one
  had only the float. *All 14 shipped experiments produce byte-identical
  fingerprints before and after, so no recorded cross-machine result moves.*
  Its pass count also printed `7 - len(fails)`, which goes **negative** once the
  structural floor appends more than seven failures; it is counted now.
- **Eight `grep` calls in `check.sh` were plain `grep`**, which on this machine
  wraps `ugrep -I` and silently skips the five non-ASCII `.cpp`. Two of them
  counted compiler warnings out of a log that quotes those files, so the warning
  total could silently undercount. All are `command grep -a` now.
- **Two `forms` checks passed on zero.** The `Line`-separator check is
  `[ "$nline" -le "$nshape" ]`, true at 0 and 0, and the sort-direction check is
  a `for` loop over a grep anchored to exactly eight leading spaces of `uic`
  output. A pattern that quietly stopped matching would make **every** form pass
  with nothing checked. The corpus totals — 6 `Line` widgets across the 20 forms,
  5 `setSortingEnabled(true)` in the generated headers — are asserted now.

**Two teeth tests were believed on a failure they did not cause, and a third
nearly was.** C6's spin-box rows: the probe's `struct P : DISpinBox` could not
reach a **private** `validate()`, so the whole `widgets` binary failed to build
and the "0 pass 1 fail" that read as the new rows biting was the build breaking —
it fails identically with and without the fix, and it silently took C6's
pre-existing check out of the run. **And the assertion could not have
discriminated anyway**: it tested `!= Acceptable`, but without
`RejectGroupSeparator` the state is `Intermediate`. C9's third teeth test
short-circuited on staleness left by the test before it, so the substituted
binary was never run while the check still printed a failure. And C9's own "run"
appeared to abort under `timeout`, which sends SIGTERM into SIGEL's handler —
`pvm_halt()` and `exit()` from signal context with Qt threads live. **A failure
observed while the harness is what broke is not evidence about the thing under
test**, and *the check that a check has teeth needs the same scepticism as the
check.*

**`check.sh` itself was reviewed in C11c and was passing things it should not.**
The review **demonstrated** a false pass rather than arguing for one, which is
the only kind worth acting on:

- **Every menu and toolbar icon could vanish and the whole check stayed green.**
  All 30 `.xpm` replaced with the text `NOT AN XPM AT ALL`: `gui behaviour 1
  pass 0 fail`. Nothing anywhere looked at an icon — `gui vs 1.3` prints
  `iconText` and `iconSize`, and the forms section covers only `.qrc`-backed
  Designer resources. **A pixmap that fails to load compiles, links, runs, and
  renders as nothing.** The `check` scenario now dumps every action's icon and
  `availableSizes()`; the corruption moves 66 lines, `withIcon=32` → `0`.
- **`check.sh` never exited non-zero** — there was no `exit` in it at all. It now
  returns 1 when anything failed **or was skipped**, which forced the decision to
  skip `WIN_*` explicitly: a permanent expected failure is a known exclusion, not
  a failure, and that is what took the run to 0 fail for the first time.
- **A skipped section reported `0 pass 0 fail` and vanished from the total**
  (`data-reordered/` is gitignored, so a fresh clone takes that path).
- **The locale check was a tautology.** `de_DE` is not installed here, so
  `setlocale` fails and only Qt's half was exercised. Under `en_DK.utf8` —
  comma-decimal and installed — `pages` differed by 8 lines, and **those 8 were
  the harness's own `%g`**: `QApplication`'s constructor calls
  `setlocale(LC_ALL, "")`, so `guidrive` had been following the locale in its own
  output. It now runs under **both** — `de_DE` for Qt's half, `en_DK` for libc's.
- **95% of the individuals list was outside the diff** (rows 0–5 of 120 and
  `... 114 more`). One order-sensitive `sha256` over every cell now covers it.

**And a failure marker had already reached a committed baseline.** An `exportall`
run's population export silently produced no file, `describeFile` printed
`!! no file at [x11b-pop.pop]`, and that line went into
`guibehaviour-baseline.txt` **as the expected output**. It was caught only
because the very next diff disagreed with it. `check.sh` now refuses the whole
class: a run whose output contains a `!!` marker fails whatever the diff says,
and so does a *baseline* that contains one. *A baseline is only as good as the
run it was captured from, and nothing in this project checked that the capture
had succeeded.*

##### Traps for whoever edits a form, a validator or a probe

**Forms — four silent-loss classes, all checked now, all worth knowing first.**

| loss | why it is silent |
|---|---|
| **`stdset="0"`** decides whether a lost property is loud or silent. `<property name="x">` emits a real setter — a compile error if Qt 6 lacks it. `stdset="0"` emits `setProperty("x", …)`, which compiles and does nothing | 616 removed across the 19 forms, which found `QLCDNumber::intValue` at once. **One must keep it**: `intValue`'s Qt 6 setter is named `display`, which `uic`'s set-plus-capital convention cannot reach. Rule: *stdset only where the Qt 6 setter is not named `set<Property>`* |
| a `Line` whose Qt 2 form set `orientation` **alone** loses it — `uic3` keeps an explicit `frameShape` and drops the redundant `orientation`, and a bare `QFrame` is `NoFrame` | it compiles, lays out, and draws nothing. 3 of 6 |
| Qt 2's `QListView` sorted **column 0 ascending** by default (`qlistview.cpp:1836-1837`); `setSortingEnabled(true)` leaves the indicator **descending** | two of the four views C2 left unpinned call `setText(0, …)` and would have displayed reversed |
| a **named layout dropped from the `.ui`** still generates a header that compiles | only a *user* of the member breaks |

Restoring a `<slots>` block verbatim is not enough: `uic3` normalises
`slot( int )` to `slot(int)` and drops the declaration, so pasting the Qt 2
spelling back creates a mismatch and Qt 6's `uic` silently falls back to
`&QWidget::slot`, a hard compile error.

**`QButtonGroup` WAS A WIDGET in Qt 2** — a `QGroupBox` that *also* managed its
buttons and their ids. Qt 6 splits the two, so `uic` emits only the `QGroupBox`
and **`find(id)` / `clicked(int)` lose their other half silently**. The logical
half is rebuilt by hand in `MT_AddConstantsWidget`, and exclusivity needs no flag
because Qt 2's `init` set `radio_excl = TRUE` and Qt 6 defaults `exclusive` to
true.

**TWO Qt 2 BEHAVIOURS ARE RESTORED BY HAND AND LOOK REDUNDANT. Do not delete
them.** (1) Qt 2's `rightButtonClicked` called `clearSelection()` when the click
missed an item (`qlistview.cpp:3390`, `qlistbox.cpp:1656-1658`), which is why 1.3
greys Delete on blank space; Qt 6's `customContextMenuRequested` does neither, so
the call is written out at **four sites**. (2) `SIG_MainWindow::closeEvent`
exists only to restore Qt 2's rule that closing the MAIN widget quits regardless
of other open windows.

**A PREPEND SWEEP KEYED ON `new QTreeWidgetItem(` MISSES INHERITED
CONSTRUCTORS.** Three sites sit in a base-initializer list — `: QTreeWidgetItem(
parent )` — and a grep for a constructor *call* cannot find a constructor that is
*inherited*. **The shape to look for is the initializer list.** Two order facts
fall out of the same trap: `MT_ExperimentWidget` builds its six pages 5→0 and
then selects `firstChild()`, so **1.3 opens on Strategy and an appending port
opens on Statistics**; and `Load Experiments` selects `topLevelItem(0)`, which in
1.3 was the **last** file loaded.

**Regenerating a form's base class.** 38 committed files cite this — 19 `.cpp`
and 19 `.h` say *"with the Qt 6 adjustments C1 lists"*. The modes: `uic3 <form>.ui`
gives the QWidget-derived base (Qt 6's `uic` emits only `Ui_<Form>`, so that
class has no other source); `-impl` gives its constructor and a `qWarning` stub
per custom slot; `-convert -extract` gives the embedded images plus a `.qrc`.
The three adjustments:

| `uic3` emits | Qt 6 |
|---|---|
| `Qt::WindowFlags fl = 0` | `= Qt::WindowFlags()` — `QFlags` has no converting constructor from `0` |
| `: QWidget(parent, name, fl)` | `: QWidget(parent, fl)` plus `setObjectName(name)` |
| `protected slots: languageChange()` | kept, driven from a `changeEvent` override. Qt 3 called it; Qt 6 has no such hook, so without the override the slot is dead code |

`QDialog` forms take a fourth argument, so the base does `QDialog(parent, fl)`
then `setObjectName` and `setModal`. **The pair is committed per form and cannot
be generated at build time** — Qt 6's `uic` cannot produce it, and requiring
docker per build was rejected.

**Validators — apply this to every new one.** Qt 6's `QDoubleValidator` and
`QIntValidator` follow `QLocale::system()` while `toDouble()`/`toInt()` never
do, so under a comma-decimal locale a typed `9,81` validates and reads back as
**0** — silently zeroing gravity. **Qt 2 could not have it**: `QApplication`
forced `setlocale(LC_NUMERIC, "C")` (`qapplication_x11.cpp:1389`) and its
validator hard-coded `'.'` (`qvalidator.cpp:387`). So pinning to `QLocale::c()`
with `RejectGroupSeparator` is a **restoration, not an improvement**. Do it with
one `findChildren<QValidator *>()` loop per constructor, which also covers
validators added later — and **at every creation site**: `MT_AddConstantsWidget`
rebuilds its validators when the type radio changes, so a constructor-only
pinning is undone the first time someone switches between integer and float.
`DISpinBox` additionally needs `setKeyboardTracking(false)`: Qt 6's
`QSpinBox::validate()`/`fixup()` are an *integer* parser running **before** the
virtual `valueFromText`, where Qt 2 called `mapTextToValue()` unconditionally.

**Two Qt 2 no-ops that Qt 6 turns into a crash or a warning.**
`QListView::setSelected(0, true)` returned early on null — and the
delete-experiment path calls it right after deleting the last experiment.
`QWidgetStack::raiseWidget(0)` returned early where `setCurrentWidget(nullptr)`
warns. Qt 2's `setSelected` was also *Single*-mode, deselecting the previous
focus item: that is `setCurrentItem()` in Qt 6, **not**
`item->setSelected(true)`, which leaves both rows selected. Related:
**`QDict::operator[]` is `const`; `QHash::operator[]` default-inserts a null and
grows the hash** — a straight rename accumulates nulls at every lookup of a
missing key, and `menuDict` has such a key by construction. Every lookup is
`.value()`.

**`updateGL()` → `update()` is not an equivalence.** Qt 2's *is* `glDraw()`:
`makeCurrent(); paintGL(); swapBuffers();`, all before the call returns. Qt 6's
posts a paint event. Safe at all 19 sites only because no caller reads back what
`paintGL` produced, and on the movie path the grab precedes the repaint.
Likewise `QMouseEvent::state()` → `buttons()` is safe only because both uses are
in `mouseMoveEvent`; on press and release Qt 2's `state()` was the state
*before* the event.

**A class that gains an owning destructor must be made non-copyable** — Qt 2's
`QGVector` copy constructor cleared `del_item`, so a copy freed nothing; a
`QList` copy shares the pointers and **both** destructors `qDeleteAll`.
`SIG_Renderer`, `SIG_EnvironmentRenderer`, `SIG_RenderRecorder`, `SIG_Geometry`,
`SIG_GPPopulation` and `SIG_GPManager` are all `= delete` on copy.
**But check the 1.3 binary first — one of these containers is deliberately
non-owning**: `SIG_Visualisation::floatingTexts` aliases the renderer's
pointers, and its constructor makes **0** `setAutoDelete` calls against
`SIG_Renderer`'s 2. Making it owning is a double free.

**A range check was lost at a FILE-DRIVEN index.** `SIG_RobotRenderer.cpp, SIG_RobotRenderer`
does `delete sceneObjects[ number ]; sceneObjects[ number ] = …` where `number`
is **read from the model file**. Qt 2's `QGVector::insert` was range-checked;
`QList::operator[]` asserts here and is an out-of-bounds **write** under
`-DQT_NO_DEBUG`. **Latent** — the compiler numbers links contiguously from 0, so
no shipped model reaches it.

**`WType_Modal` is not `Qt::Dialog`.** The bits coincide at 0x3, which is not an
equivalence: Qt 2's flag *made the window modal* (`WState_Modal`,
`qwidget.cpp:725`), while Qt 6's names a window **type** and `windowModality`
defaults to `NonModal`, so it needs `setWindowModality(Qt::ApplicationModal)` as
well. **The type test must mask** — `( f & Qt::WindowType_Mask ) == Qt::Dialog`,
because a bare `f & Qt::Dialog` is true for a plain `Qt::Window` too.

**`QProgressDialog`'s third argument changed meaning**, `totalSteps` in Qt 2
against `minimum` in Qt 6, and `cancel` was `0` in 1.3 so there is no Cancel
button at all. *C6's progress dialogs never appear — constant generation is
capped at 99 and completes faster than the minimum duration — so those claims are
read from the Qt 2 signature, not observed. The 99 cap is not a divergence: the
form sets no maximum and both Qt versions default a `QSpinBox` maximum to 99.*


**Three link requirements, all of which fail silently.** `MOC_HDRS` is **derived
from a `Q_OBJECT` grep**, never hand-written — a missing entry is an undefined
vtable at link, or a signal that never fires. **Resource objects are named on
the link line**, not left inside an archive, or the form icons vanish. And **the
slave must not link the master's meta-objects** — doing so drags in
`SIG_GUIGPExperiment` → `MT_Controller` → the master `SIG_GPExperiment`; §9's
assertion catches it.

**The build picks between the two `SIG_GPExperiment` variants BY LOCALE.**
Both objects go into `libSIGEL_GP.a` and the linker takes the first member
defining the symbol; `$(wildcard)` sorts by `strcoll`, and the two names order
differently either side of the `Clean` suffix — `LC_ALL=C` puts the master first
and the link fails. `SIG_GPExperimentClean.o` is now named explicitly ahead of
the archives with an assertion after the link. *Chasing a recurrence, look at
the environment, not at build freshness.*

**Two blind spots no check reaches.** Three Qt 2 APIs survive in `#ifdef _WINDOWS`
blocks that **cannot be compiled here** and were read by hand. And **the floating
text labels are unverified, not unexercised** — the code runs (1785 `paintGL`
hits) but the oracle's 3-D view renders nothing, so their absence there is
equally consistent with "they do not composite over a native GL window" and
"nothing renders in that environment". `QOpenGLWidget` composites children
correctly where `QGLWidget` often did not, so this port may make them *more*
visible than Qt 2 did, and neither side can tell.

**Probe craft — every apparent defect in C10–C12 that is not in the table above
was the probe, and there were twelve.** The recurring shapes:

- **Arm modal handlers narrowly.** `MT_MainWindow` is itself modal and comes back
  from `activeModalWidget()`, so a handler that closes whatever it finds closes
  the window under test. That one defect produced three wrong conclusions,
  including a claim that focus-out was undrivable.
- **Offscreen: use `isHidden()`, not `isVisible()`.** An unmapped window makes
  every widget inside it report invisible whatever the code did.
- **Check a lookup found something.** Two toolbar press blocks were dead code
  because the map keys carried a tab suffix and the lookups did not; the baseline
  simply carried no result line.
- **`QTest` posts key events straight at the widget, focus or no focus**, so a
  keyboard probe never measures focus. In 1.3 a groove click does not focus a
  slider at all.
- **Take a slider's cross-axis from `SC_SliderGroove`, never the widget middle**,
  and populate the `QStyleOptionSlider` fully — an unset `tickPosition` makes the
  style return a tickless groove.
- **A recovery path that does not announce itself** turns an intermittent failure
  into an intermittent wrong answer. The file-dialog flake took three diagnoses;
  the cause was that `QFileDialog::accept()` treats any filename carrying a
  directory as a navigation request and returns **without accepting**.
- **Measure a negative with a positive control.** "The main window did not
  respond" is also the shape of a click that missed: 0 pixels against a control
  of 4397.
- **A probe that mutates a WIDGET must push the widget into the model before it
  reads a file back.** `roundtrip` typed into a widget and then called an import
  that ends in `getOutOfExperiment()`, which refreshes every widget *from* the
  experiment — so the change was undone before the second export whether the
  reader ran or not, and a gutted importer printed
  `ROUND TRIP STABLE (and the import undid the change)`. Fixed by clicking a View
  page the mutation did not touch, which runs `putAllIntoExperiment()` through the
  path a user actually takes.
- **Check before changing.** The language-command list came out 13 against 1.3's
  15 because the page is built against a default experiment *before* the file
  loads. Probing in the wrong order invents the defect.

##### What was measured against the running 1.3, and how

**C9: 42 menu entries, zero mismatches.** A headless probe prints every menu
entry and toolbar button with shortcut, enabled state and check state; the oracle
read the same facts off 1.3. Every oddity agrees — the duplicate `Ctrl+Shift+L`,
`Reset` carrying `Alt+O` with no matching letter, `Evaluator System` showing its
tick while greyed, and the Import/Export submenu **parents** staying enabled
while all sixteen children are greyed, which is the direct observable proof of
the action-group fix. With the same experiment loaded every GP-page value
matches, **including the three that are not echoes of the file**: per-mille
50/670/280 displayed as 5/67/28; "Tournaments per generation" showing **60**, the
file's 0.5 × a 120-individual population; and pool-image frequency clamped to the
spin-box minimum. The PVM host list comes out **exactly reversed from file
order** — a prepend demonstration driven by file data rather than by constructor
calls.

**The `truncate(-1)` sweep.** *If the trigger is `uint` vs `qsizetype`, the same
shape exists anywhere 1.3 feeds a possibly-`-1` search result into a Qt size
parameter and relies on the unsigned wrap being harmless.* **30 such sites across
371 files, and they reduce to one:**

| call | Qt 2 (`uint`, -1 wraps huge) | Qt 6 (`qsizetype`, -1 stays -1) | same? |
|---|---|---|---|
| `left(-1)` / `right(-1)` | whole string | whole string | yes |
| `mid(p, negative)` | to end | to end | yes |
| `chop(-1)` | no change | no change | yes |
| **`truncate(-1)`** | **no-op** | **clears the string** | **NO** |

**The two byte-exact cross-machine anchors.** *"The same" cannot mean "the same
fitness"* across an x87/IEEE boundary, so C10 split the question into
machine-independent parts: that the GUI hands the evaluator the same individual,
that the PVM payload carries it unaltered, and that the individual evaluates the
same way (same machine only). The anchors are **the exported program** — 6616
bytes, `sha256 f940751765c869f16f62333dafd34dbe0e66b6474577bf4209e9e8de70cc3214`
on both architectures, reproduced again through a different scenario in C11b —
and **the saved 2.7 MB experiment file**, whose byte counts agree across two
saves on both machines, so the writer's formatting of every value agrees and not
merely the structure.

**~~No tool on this machine can deliver a real X-level click~~ — WITHDRAWN
2026-09-07.** As written this was too broad, and it is now false. A nested
`Xvfb` delivers real clicks. The port is checked on them. See C13.

What was measured, and still holds, is about the live session only. XTEST into
the running Xwayland session returns success and has no effect. This is a VMware
guest under GNOME/Wayland. `vmware-user` owns absolute pointer integration and
the compositor's pointer wins. Qt's VNC platform serves, but `sigel` crashes on
it and has no GL context, so that crash is an unsupported configuration and not
a port defect.

**A nested plain X server was never tried, and that was the gap.** It has no
compositor, no `vmware-user` and no absolute pointer integration. It is the same
kind of display the oracle already uses, and XTEST is ordinary there. It was
tried on 2026-09-07 with `Xvfb :N`, `QT_QPA_PLATFORM=xcb` and `xdotool`. The
port runs in it as an ordinary X11 client and a real click arrives. The scenario
is `guidrive xtest`. The output is `xtest-baseline.txt`. The check is `real
clicks` in `check.sh`.

**The pointer readback behind the original verdict cannot be trusted.** On the
nested server, `xdotool getmouselocation` reports the screen centre whatever the
pointer is doing. XTEST moved the pointer to (42,43). An independent client
watching the server, `xev -root`, saw it arrive there, marked `synthetic NO`.
`getmouselocation` still said (700,500), every time. This was measured on `:N`
only. Whether it behaves the same way on the live `:0` was not retested, and
claiming that would repeat the error being corrected here. So the original
verdict loses that piece of evidence, but not necessarily its conclusion: the
negative on the live session was also seen in behaviour. A readback that returns
a plausible constant is the worst kind, because it fails every test by passing.

**What this bought.** A real click on both ends of every comparison, instead of
only the oracle's. It is still XTEST input and not a physical mouse, but so is
the oracle's side. The first run found a difference: C13's swallowed dismissing
click. Nothing driven through `QTest` could have found it.

**One of the four items in the list below was wrong.** Enter and leave delivery
is not something `QTest` cannot reach.
`/usr/include/aarch64-linux-gnu/qt6/QtTest/qtestmouse.h` shows
`QTest::mouseMove(QWidget *)` with no button held. It calls `QCursor::setPos()`
and then `processEvents()`. That moves the real pointer through the platform. On
a real X server it produces real crossing events, as section 6 of the baseline
shows. The offscreen platform could not deliver them, because there is no
pointer to move. That is a platform limit, not a `QTest` limit. The other three
items hold: activation, the popup's pointer grab, and double click synthesis.
C13 measures those.

**`QEvent::spontaneous()` does not tell a real click from a `QTest` one.** This
is worth writing down, because it is the obvious first idea and it gives a false
pass. The same header calls `QSpontaneKeyEvent::setSpontaneous()` on the event
it posts, and only then calls `qApp->notify()`. A `QTest` click therefore
reports `spontaneous() == true`, exactly like a real one. A probe built on it
would pass with no real click in it. What does work is a native event filter. A
real click arrives from the server as an XInput 2 `GenericEvent`, and reaches
`QWidgetWindow` before the `QWidget`. `QTest` produces no native event and never
touches `QWidgetWindow`. Both halves are printed in section 2 of the baseline,
and the check fails if they stop differing.

**Trap for anyone running the harness under `xcb`: the file dialog goes
native.** Under `offscreen` no platform theme offers one, so all ten baselines
were captured against Qt's own widget dialog. Under `xcb` on `:N`, the same
`guidrive open` run reports `labelAccept=[]` and has no `fileNameEdit` child.
That is `QFileDialogPrivate::usingWidgets()` returning false: the dialog belongs
to another process, and `acceptFileDialog()` cannot type into it. None of these
changed it: `QT_NO_XDG_DESKTOP_PORTAL=1`, clearing `DBUS_SESSION_BUS_ADDRESS`
and `XDG_RUNTIME_DIR`, `XDG_CURRENT_DESKTOP=`, or
`QT_QPA_PLATFORMTHEME=minimal|gtk3`. `guidrive` now sets
`Qt::AA_DontUseNativeDialogs`. That also matches 1.3: Qt 2.3 had no native
dialog path, so 1.3 always drew its own widgets. It does nothing under
`offscreen`. With it, a `guidrive open` run under `xcb` is byte-identical to the
same run offscreen.

**The ten `gui behaviour` scenarios are driven by `QTest`, and `QTest` is not a
mouse.** `mouseClick`, `keyClicks` and a hand-posted `QContextMenuEvent` go
through `QApplication::notify`. Event handlers, hit-testing, `QMenu` popups,
item-view selection and the slots behind them all run. But they bypass
`QWindowSystemInterface`, so activation, grabs and double-click synthesis are
not what a real click produces. *Enter and leave were listed here too, and that
was wrong; see above. The `xtest` scenario now covers the other three.* It proves SIGEL's logic, not the platform layer's. **The oracle's
side was driven by real XTest, so every comparison has a genuine click on one
end.**

**The validator battery is 12 probes typed ONE CHARACTER AT A TIME**, seven into
a `QDoubleValidator` field and five into a `QIntValidator` field. A validator
that answers `Invalid` makes `QLineEdit` drop that keystroke, so the surviving
text carries a gap exactly where the rejection happened, **and that gap is the
measurement**. It is also the only thing the oracle *can* read: Qt 2 has no
accessibility API, so 1.3's side reports behaviour, never configuration. **Twelve
for twelve**, including the two that reasoning would have got wrong — the
exponent passing and the **integer** field taking a leading minus: `9.81`→`9.81`,
`9,81`→`981`, `0,375`→`0375`, `-2.5`→`-2.5`, `1e3`→`1e3`, `abc`→empty,
`1.2.3`→`1.23`; `42`→`42`, `-7`→`-7`, `4.2`→`42`, `4,2`→`42`, `abc`→empty. The
21st validator is on `SIG_EditCommandDialog`, driven in C11c with the same
result, **closing the set at 21 of 21**.

**Locale independence is measured on BOTH sides, which is the point of C7.** This
side: the whole scenario re-run under `de_DE.UTF-8` and `fr_FR.UTF-8` — both
comma-decimal, with different group separators, U+202F against a full stop — is
**byte-identical** to the en_US run, and Qt 6 genuinely saw them. The 1.3 side:
**the oracle built a de_DE with woody's own `localedef` and verified it with
woody's own `locale`**, because a locale a modern `localedef` produces may simply
be rejected by glibc 2.2.5 — which would have given a silent fallback to C and a
confident "no difference" that meant nothing.

**The two interlocks, neither visible without driving.** The genetic-operator
sliders are not independent: `slotMutationChanged` clamps Mutation + Crossover at
1000 and recomputes Reproduction as the remainder, Reproduction's own slider
disabled. Mutation does **not** reach 1000 on either side — it clamps at
`1000 − Crossover`, 330 on the oracle and 250 here because an earlier probe had
left Crossover at 750. **Same rule, different starting point, and the port lands
on the arithmetic the rule predicts rather than on the oracle's number — a
stronger agreement than matching the figure would have been.** And **the LCDs
show percent where the sliders and the file are per-mille**: 330/670/0 displays
as 33/67/0, so comparing an LCD reading against a file value directly is out by a
factor of ten. It cost the oracle time.

**The page-to-file round trip: nine values, byte-identical across
architectures** — `STEPSIZE`, `MAXIMALERROR`, `DYNAMECHSINTEGRATOR`,
`YPLANELEVEL`, `GRAVITY` x, a floor dimension, `RANDOMSEED`, maximal age and a
`LanguageParameters` field, typed identically on both sides from a fresh load,
**as bytes, not as numbers x87 and IEEE can round differently**.

**Checked as `pagesave`.** Load `twoBasesSimpleFitness2.exp`, visit no page,
change nothing, save once, and take everything above `POPULATION BEGIN{`. **The
port wrote those 192 lines byte for byte — 2069 bytes, sha256 `a327150c…` on
both machines.** *The capture carried a positive control: a second save after
one parameter change differed in exactly one line.* Since 2026-09-19 the check
loads `twoBases.exp` and compares against the port's own save, 178 lines and
1589 bytes, by the rule above. **`LanguageParameters` is checked separately and
would otherwise have been missed**: it sat at line 133288 of that saved file, far below
`POPULATION BEGIN{` at 193, so the 192-line block covers eleven of C11a's twelve
edits. In `twoBases.exp`'s saved file it is at line 71650, below a
178-line block. That was found because the oracle reported where `POPULATION BEGIN{` had
moved to — not by reading the code.

**Seven of the eight exports are byte-identical to what 1.3 writes** — `.gpp`
1299 B, `.sip` 305, `.env` 299, `.pop` 2,738,900, `.prg` 6616, `.ind` 22,113,
`.dat` 17,420, with the filename typed **without an extension** so
`checkEnding()` has to append it, and CR count 0 on all eight both sides. **The
`.dat` is the result worth pausing on**: 532 lines of *floating-point* output,
byte for byte the same on the two architectures. It was expected to differ — the
instruction sent to the oracle said in terms that a difference there would not be
a defect — and it does not differ at all. Round trips: `.gpp` `.sip` `.lap`
`.env` `.prg` byte-identical, `.pop` +840 with **nothing but blank lines moved**.
**`Import > Program` replaces, `Import > Individual` adds**, deliberately —
`slotImportProgram` calls `importProgram` on the *selected* item — so Individual
takes the pool 120 → 121 and the newcomer arrives renamed to the next identifier
with fitness -1 and the source's age: a fresh individual awaiting evaluation, not
a duplicate.

**The Qt 2 dict model has predicted three different answers and been confirmed on
the binary each time.** Reimplemented from the vendored source — `qgdict.cpp`'s
`hashKeyString`, `index = hash % vlen`, new buckets prepended by `look_string`,
iteration from bucket 0 upward, `qdict.h`'s default table size of **17** — it
gives the default-constructed command order
`MUL MOVE CMP COPY LOAD JMP SENSE NOP SUB DIV MIN DELAY ADD MOD MAX`, the
measured `.lap` reload permutation, and JMP's insertion point sixth after LOAD.
*It was checked against something already on disk before it was trusted:
`twoBasesSimpleFitness2.exp` stores 13 commands in exactly that order with JMP
and NOP removed.*

**C11c's certainty came from a reading eight steps earlier.** C10 measured the
**Rename** dialog as pre-selecting in **both** versions — the same apparent
pattern as the pre-fill defect, opposite result, which looks like a contradiction
in the oracle's own data until you read the source: Rename calls `selectAll()`
**explicitly** and the other three do not. *"These two readings disagree" is
sometimes a fact about the code rather than an error in the readings.* The fix
was wrong twice first: `deselect()` before `exec()` is a **no-op**, because the
selection does not exist until the dialog is *shown* and focus travels the tab
chain; and `deselect()` alone leaves the cursor at **position 0**, so a typed
digit lands *before* the value and `1` + `2` gave **21**. **`end( false )`**
clears the selection *and* puts the cursor after the text. `slotAddHost` is
deliberately not patched — it focuses an empty field.

**The MetaGP tree labels do not match the source class names**, and that cost
both sides a round trip: Strategy = `MT_EstimationWidget`, Individual =
`MT_IndividualsWidget`, Population = `MT_PopulationWidget`, **GP Parameter =
`MT_SearchWidget`**, Selection = `MT_SelectionWidget`, Statistics =
`MT_StatisticsWidget`. `MT_ExperimentWidget` is not a page at all — it *is* the
tree. **The 30 constants are deliberately not pinned**: `MT_IndividualsWidget`
generates them with a randomizer at construction, so they differ every run; the
count and "every one parses as an integer in range" are checked instead, which is
an invariant rather than noise.


##### TRAP — `QIntValidator` out of range: Intermediate in Qt 2, Invalid in Qt 6

Qt 2 (`qvalidator.cpp:236`) returned **Intermediate** out of range, so
`QLineEdit` accepted every digit; `qspinbox.cpp:186` gives every `QSpinBox`
exactly that validator over its own `[min,max]`, `interpretText()` maps the
whole text and calls `setValue()`, and `QRangeControl::directSetValue` "Forces
the new value to be within the legal range" — that is the clamp.
commit. **Qt 6 returns Invalid once the typed prefix passes the top**, so the
keystroke is refused and the box keeps a truncated prefix. Measured on both,
typing then Return:

| field | range | typed | 1.3 commits | port commits |
|---|---|---|---|---|
| Maximal program length | 2..32000 | `32001` | **32000** | **3200** |
| Year | 1752..8000 | `8001` | **8000** | **2030** |
| Hour | 0..23 | `24` | **23** | **2** |
| Register width | 1..99 | `100` | **99** | **10** |

Both end on a *valid* value; they are **different valid values**, and
`putAllIntoExperiment()` writes whichever the widget holds. *For Year the kept
prefix `800` is below the minimum, so the box falls back to the value it held:
the loaded experiment's year, 2030 since 2026-09-21.*

**ACCEPTED, NOT FIXED — D28.** Reachable only by typing outside the box's own
range, and the differing value is visible in the box before anything is saved.
The cost is not the 25-line `QSpinBox` subclass (in `1dba5f4`'s copy of
`future_refactorings.md`, and it reproduces all four readings) but **owning a
custom widget forever** across 29 spin boxes in three forms and then the
dialogs. Pinned by the `commits=` value in `guibehaviour-baseline.txt`: **44 `spin`
lines, 40 carrying a committed value** — 29 on the five View pages, 11 in the
MetaGP block. The four that are not are three survey-only lines and one reading
`commits=not-pressed(in a dialog)`. *The forms declare 46 `QSpinBox` (18 + 8 + 3
by form, 17 elsewhere; 20 counting the three declared in code).*

*Two cheaper fixes were rejected on measurement: swapping the validator on the
internal `QLineEdit` leaves `QAbstractSpinBox`'s interpret path unchanged and
`100` into a `[1..99]` box then commits **1**, worse than the divergence; and
widening the range changes what the pages display and write.*

**Where D28 does NOT apply.** In `MT_AddConstantsWidget` the same divergence
reaches the **generated constants** — the value is not visible in a box, it
becomes data — so Qt 2's rule is restored there instead. See §9's C12.

##### What Phase C did NOT exercise

- **A real X-level click.** No tool on this machine can deliver one — XTEST
  returns success and has no effect under this compositor, measured by reading
  the pointer back rather than trusting the call. The oracle's side *was* driven
  by real XTest, so every comparison has a genuine click on one end.
- **The rendered 3-D view, on either side.** It does not render offscreen here,
  and the oracle's capture of that region returns a constant image while the
  scene demonstrably moves. Nothing about it may be read as agreement.
- **`listviewCommands` and `listviewHosts` beyond their contents.** Both are
  double-click entry points to dialogs.
- **Only one of C10's two `blockSignals` sites is check-covered.** The second, in
  `slotAddIndividuals`, sits on a path that only ever *grows* the pool, so a stale
  `poolPosition` still resolves to the same individual and reverting it is
  invisible to any GUI observation. Kept because Qt 2 emitted nothing there
  either, but it is **faithfulness without a test**.
- **THE EVOLUTION PATH IS NOT IN `gui behaviour` AT ALL** — the eleven scenarios do
  not include `evolution`, so nothing in `check.sh` runs a generation. It was
  driven by hand on both machines and the result is prose; see §9's open box.
- **Native file dialogs.** Qt's own non-native dialog throughout.
- **The Robot page's buttons, `slotAddHost`, the Edit Command multi-selection
  branch, `Import > Population` beyond the round trip, and any malformed file
  into any importer.** Every import was given a file the matching exporter had
  just written.
- **`Import > Robot` with a robot that is not the experiment's own**, and the
  `Robot import error!` path.
- **`MT_GUI`'s toolbar actions have no 1.3-side measurement.** `metadrive`
  presses them here; nobody has pressed them on 1.3, so for that item this is
  coverage on one side, not an agreement.

---

## 9. Status and what is open
### Dynamo removed, DynaMechs kept — 2026-08-28

**DONE.** Analysed 2026-08-20, executed 2026-08-28. SIGEL shipped two physics
engines and chose one at run time. Dynamo crashed on most shipped robots, its own
authors labelled it "not recommended", and all 14 shipped experiments selected
DynaMechs. It is gone. This section records what was removed, what was kept and
why, and the follow-ups not taken. *Held in its own file until
2026-09-20, then folded in here: this file is the only log of what was done.*

#### The two engines

- **Dynamo** — "Dynamic Motion library", Bart Barenbrug, TU Eindhoven,
  1996–1999. Constraint-based. LGPL. **Deleted.**
- **DynaMechs** — Scott McMillan. Articulated-body (Featherstone). **Kept.**

Both implemented `SIG_SimulationData`, `SIG_SimulationQueries` and
`SIG_CommandInterface`; `SIG_Simulation.cpp` switched between them at run time
on the `SIMULATIONLIBRARY` key, which is `1` in 14 of 14 shipped `.exp`.

---

#### What was actually done — 2026-08-28

##### SOLID and qhull went too

The Dynamo backend was deleted in `5addd66`. **SOLID and qhull followed in a
separate commit**, because their only caller was the deleted code:

| library | files | lines no longer compiled |
|---|---|---|
| SOLID | 16 | 1,999 |
| qhull | 11 | 20,705 |
| **total** | **27** | **22,704** |

Measured before removing, not assumed: **zero `dt*` SOLID API calls anywhere in
SIGEL, zero in vendored DynaMechs, and zero `qh_*` references outside qhull
itself.** qhull existed only to give SOLID its convex hulls — that is what the
`-DQHULL` flag selected — so it could not outlive it.

The one surviving mention is `maximalSOLIDIterations`, a simulation parameter
still parsed, stored and written back but now read by nothing. It joins the six
others the Dynamo removal left in that state.

`libdynalib.a` is the one that could **not** go: see the deletion commit. Its
maths half is not separable from its physics half.

##### Deleted

**SIGEL's 13 Dynamo file pairs, 26 files, 3,226 lines.** `SIG_Dyna`,
`SIG_DynaCallbacks`, `SIG_DynaDrive`, `SIG_DynaJoint`, `SIG_DynaLink`,
`SIG_DynaMoCommandInterface`, `SIG_DynaMoSimulationData`,
`SIG_DynaMoSimulationQueries`, `SIG_DynaSensor`, `SIG_DynaSystem`,
`SIG_DynaSystemWrongNumberException`, `SIG_RotationalController`,
`SIG_TranslationalController` — every one of them `.cpp` and `.h`.

**3,226, not the 3,247 counted above.** The list of files was right; four of
them changed size between 2026-08-20 and the deletion, all in Phase D:
`SIG_DynaSystem` −8, `SIG_DynaMoSimulationData` −15,
`SIG_DynaMoSimulationQueries` +1, `SIG_DynaMoCommandInterface` +1.

**The backend switch.** `SIG_Simulation.cpp` keeps its `DynaMechs` case and
gains a `default:` that prints to `std::cerr` and throws. It does not fall
through — the previous switch had no `default` at all, so an unknown library
would have left `simulationData`, `simulationQueries` and `commandInterface`
uninitialised. Verified by building a copy of `sigel_eval` without its own
`SIMULATIONLIBRARY` guard (`sigel_eval.cpp, selfcheck`) and running an `.exp` edited to
`SIMULATIONLIBRARY 0`: one line of diagnostic, then abort, exit 134. It never
falls through and never runs DynaMechs.

**"Loud" is true of `sigel_eval` only.** The throw clears all six fitness
functions, which construct `SIG_Simulation` outside their own `try` — but one
frame further out, `sigel_slave.cpp:361-367` wraps `evalFitness()` in
`catch (SIG_Exception &) { fitnessValue = 0; }`. So under PVM the exception is
swallowed and the individual is scored 0.0 as though evaluated, the failure
`SIG_GPSimpleRecorder.cpp, init` documents. The printed line is therefore the only
evidence anyone gets, which is why it goes to `std::cerr` and not the buffered
`SIG_IO::cerr`. Hardening that `catch` is pre-existing and out of scope.

**Three dead includes and one wrong one.** `SIG_SimulationQueries.cpp` included
`SIG_DynaLink.h`, `SIG_Dyna.h` and `SIG_DynaSensor.h` and used none of them.
`SIG_Environment.h:26` included `<constraint.h>`, a Dynamo *physics* header,
only to reach `DL_vector`; it is `<pointvector.h>` now, as this document asked.

**One `moc` target.** `SIG_DynaSystem.h` is off `MOC_HDRS`.

**`SIG_Simulation` stays a `QObject`, and that is now vestigial — recorded
rather than glossed.** The deleted `connect()` was the only thing that wired
`slotDynamoMessage`, which is the class's only slot; it declares no signals. So
`Q_OBJECT`, the `QObject` base and the surviving `moc` target exist for a slot
nothing can invoke. Two consequences follow and are annotated in the source:
`stopSimulation` had no other writer, so `makeTimeSteps`' `if (stopSimulation)`
is permanently false — and that is the **only** throw site of
`SIG_SimulationCannotSolveException` in the tree, which makes the 2003-behaviour
boundary around `start()` and `makeTimeSteps()` guard a type that can no longer
arrive. All of it is left in place: removing a slot changes the Qt surface of a
class Phase C still has to port, which is a different decision from deleting a
physics backend.

**46 of the 60 vendored Dynamo `.cpp` stop being compiled**, 10,084 of 13,567
lines. Nothing vendored is *deleted* — that tree is untracked and comes out of
a tarball — only the `dynamo_SRC` list the build feeds `ar`.

**Four build files, and the fourth is the one that matters.** Both `Makefile.am`
in `SIGEL_Simulation` lose the 39 filenames that no longer exist, and
`SIGELCommon.dsp` — the 2003 Visual Studio project — loses 27 `Source File`
blocks: the 26 files plus `moc_SIG_DynaSystem.cpp` and its two custom-build
rules.

**`kdesigel.kdevprj` was the fourth**, and at the time it had to be kept in step:
both `Makefile.am` carried the marker `####### kdevelop will overwrite this
part!!! (begin)`, so the `.kdevprj` was what KDevelop 2 regenerated them from,
and a regeneration would have put `SIG_DynaSystem.cpp` and its twelve siblings
back. It named the 26 deleted files in 28 places, all removed then.
**All of that is moot since 2026-09-20**: the `.kdevprj` and all 43 `Makefile.am`
were deleted with the rest of the 2003 autotools build, which had been unrunnable
for years.

##### Kept, and why

**The maths library, and more of Dynamo than expected.**

- `pointvector.cpp` and `list.cpp` are genuinely empty — "no non-inline
  methods". `DL_vector` and `DL_point` really are header-only.
- **`matrix.cpp` is not.** It carries 27 out-of-line `DL_matrix` members, all 28
  of its symbols link into `sigel_eval`, so `libdynalib.a` **cannot** be dropped
  from the link line. Tested, not assumed.
- **`matrix.cpp` is not pure maths either.** It `#include`s `dyna_system.h` so
  that `DL_matrix::invert` can report a singular matrix through the physics
  engine's global callback: `matrix.cpp:233`,
  `DL_dsystem->get_companion()->Msg("singular matrix can't be inverted\n")`.
  That call, plus the `DL_geo` vtable `matrix.o` emits, is an undefined
  reference to `dyna_system.o` and `geo.o`, whose closure is nine more physics
  translation units.

So **the maths and the physics are not cleanly separable**. The archive holds
14 objects and **the linker pulls 12 of them**: `pointvector.o` and `list.o`
define no symbols at all and are never extracted, so they are compiled only to
keep the maths half of the library named rather than implied. The 11 physics
objects below are all pulled. Measured with `nm` over all 60 objects and
confirmed against a linker map; each entry names the symbol that pulled it in:

| object | pulled in by |
|---|---|
| `dyna_system` | `DL_dsystem` |
| `geo` | `DL_geo::move` |
| `dyna` | `DL_dyna::newkinenergy` |
| `constraint` | `DL_constraint::reset_undo` |
| `constraint_manager` | `DL_constraints` |
| `euler` | `DL_euler::DL_euler` |
| `m_integrator` | `DL_m_integrator::stepsize` |
| `largematrix` | `DL_largematrix::prep_for_solve` |
| `supvec` | `DL_supvec::A2q` |
| `force_drawable` | `DL_force_drawable::get_fd_info` |
| `vector4` | `DL_vector4::assign` |

3,056 lines of physics survive for one error message. Nothing is stubbed and no
symbol is defined away: breaking the coupling means patching a diagnostic out of
a vendored file, which is a separate decision and was not taken.

**The whole Dynamo include path.** `-isystem .../Dynamo/Src/Inc` stays on both
the build and `check.sh`: the maths headers live in the same directory as the
physics ones.

**SOLID and qhull are gone from disk, 2026-09-20.** They stopped being compiled
with the Dynamo backend on 2026-08-28, and for three weeks only the
`-isystem .../SOLID-2.0/include` path survived. Both folders were then deleted
from the vendored extract, the include path was removed from the `Makefile` and
from `check.sh`, and everything was rebuilt from scratch with no error. They
remain in the full supportingLibs archive, on SourceForge and in git history.

##### Checks — all clean, which is the point

| check | result |
|---|---|
| `dictorder-dump.sh` vs baseline | **empty diff** |
| `fitness-check.sh` vs baseline | **empty diff**, 42 of 42 |
| `sigel_eval -selfcheck` | pass |
| sanitized `fitness-check.sh build` | **empty diff**, no ASan or UBSan report |
| `check.sh` **as of 2026-08-28** | **105 pass, 4 fail** — see §7 for the current figure |

`check.sh` was 118 pass, 4 fail before this change. The pass count falls by
exactly 13 because 13 fewer `.cpp` exist, and "headers standalone" by exactly 13
for the same reason. The warning count is not quoted here because Phase D keeps
moving it; §7 has the current figure. **The 4 failures are the same 4 files** — `MT_Controller.cpp`,
`SIG_GUIGPManager.cpp` and the two ZORC files — and the one header failure is
the same one. No fitness value and no line of container ordering moved, which
is what "dead code" was supposed to mean.

##### Dead but not deleted — the predictions, re-measured

| predicted | measured 2026-08-28 |
|---|---|
| `SIG_GlueJoint`, `SIG_CylindricalJoint` | **holds.** Both still parsed (`SIG_Robot.cpp:367,369`), still compiled by `SIGEL_RobotIO`, still listed by `SIG_RobotView.cpp, getOutOfExperiment`. No simulator reads either |
| `getFrictionValue`, `getElasticity`, `getVeloDamping` | **holds, exactly.** Zero call sites anywhere in the tree; only the definition and the declaration remain |
| `getYPlaneLevel` | survives, as predicted — `SIG_EnvironmentRenderer.cpp, SIG_EnvironmentRenderer` and `SIG_EnvironmentView.cpp, getOutOfExperiment` |
| the six simulation parameters | **holds, with a correction.** `getAnalytical`, `getIntegrator`, `getMaximalIterations`, `getMaximalCollisionLoops`, `getSkipFrames` and `getSolveMode` lose their only *simulation* reader, but each is still read by `SIG_SimulationParameter.cpp` to fill its dialog. They are now exactly as dead as `getMaximalError` and `getMaximalSOLIDIterations` already were: parsed, displayed, editable, simulated by nothing |

**Three things the prediction missed, found by review of the change:**

| now dead | where |
|---|---|
| `SIG_Robot::prepareDynaMo` and `SIG_Link::transformToDynaMo` | `SIG_Robot.cpp:274-282` and `SIG_Link.cpp:200-206` — **16 lines of definition, 23 with the declarations and their doxygen blocks.** An earlier draft said "~45", which was a guess; this project's rule is that only measured figures go in a planning document. Their only callers are the three surviving `case DynaMo:` arms below |
| three `case DynaMo:` arms | `SIG_GPFitnessTrainer.cpp, SIG_GPFitnessTrainer`, `sigel_slave.cpp:252`, `SIG_AllIndividualsView.cpp, slotStatsClicked`. Each transforms the robot for a simulation that now always throws. Left because the `SimulationLibrary` enum has to survive — the parser, the GUI and four other switches name it |
| `SIG_Simulation::slotDynamoMessage`, `stopSimulation`, and the only throw of `SIG_SimulationCannotSolveException` | see "One `moc` target" above |

##### Follow-up this change deliberately did not take

1. **The GUI can still author an experiment that now aborts.**
   **Four surviving surfaces**: `SIG_SimulationParameter.cpp, putIntoExperiment` calls
   `setSimulationLibrary(DynaMo)`; `:224-225` reads the value back to re-check
   that button; `SIG_SimulationParameterBase.ui:143` offers "Dynamo  (not
   recommended)" and `:566-568` is a whole tab titled `DynaMo`; and
   `SIG_EnvironmentBase.ui:916-918` is a second such tab. Removing only the
   radio button leaves two dead tabs and a read-back for a value nothing can
   set.

   **The throw has a second consequence.**
   `SIG_SimulationVisualisationWidget.cpp:376-381` does
   `delete visualisation;` and then assigns the result of a constructor that
   now throws — so `visualisation` keeps a freed pointer. Fourteen sites in that
   widget test `if (visualisation)` and then dereference it, so the guard
   passes and each is a use-after-free; `renderRecorder` leaks with it. The
   path pre-existed — Dynamo's own constructor could throw — but this change
   turns a conditional hazard into a certain one for every robot.
   **FIXED 2026-09-02:** `visualisation = nullptr;` now sits between the delete
   and the new, at **line 414** (the 376-381 above is line drift). `check.sh`'s
   `freed-pointer null` holds it there.

   **Not reachable today, on the argument that survives review.** Two earlier
   arguments were wrong and are dropped rather than restated: `sigel_slave.cpp:293`
   is not the only caller, and the destructor is empty only in the DERIVED class
   (the base does `delete visualisation`, so a throw would be a double free).
   What holds: every other caller is downstream of a first `visualizeThis()` that
   must have succeeded, and nothing in the slave calls `setSimulationLibrary`, so
   the same parameters cannot begin throwing later. **The oracle then settled
   1.3's behaviour: with Dynamo selected the viewer opens and the slave SEGFAULTS
   on Play**, no dialog, master surviving — so that path was non-functional in
   1.3 and deleting it lost nothing. `renderRecorder` still leaks on the throwing
   path; recorded in `future_refactorings.md`.

2. **`SIG_SimulationQueries.cpp`: all seven non-self includes are dead.**
   Verified by compiling a translation unit
   holding only the class's own header and the empty constructor, under
   `check.sh`'s full flags: it passes. `<qdatetime.h>` and `SIGEL_Tools/SIG_IO.h`
   are dead too. This change removed only the three that named deleted files.
3. **Extract the maths into a small local header, and drop `libdynalib.a`.**
   *Decided 2026-08-28, deliberately not now.* The archive survives at 14
   objects — ~3,000 lines of physics — for **one line**: `DL_matrix::invert`
   reports a singular matrix through the physics engine's global system object
   (`matrix.cpp:233`), and that call plus the `DL_geo` vtable pulls in ten more
   translation units.

   The API is tiny: `DL_vector` is 18 methods in 296 lines, `DL_matrix` 81
   lines, and the whole vocabulary is get/set a component, add and subtract in
   place, scale, negate, norm, normalise, inner and cross product, multiply,
   invert. A 3-vector and a 3×3 matrix.

   **Pragmatic and minimal: a local header, not a dependency.** Boost's real
   candidate would be QVM rather than uBLAS, and Eigen would be the better
   library — but the port's direction is removing 2003 dependencies, not
   swapping them, and NEWMAT is already compiled and linked and used in 13
   files, so the tree already carries two matrix libraries. Replacing both with
   ~250 lines we own removes the last of Dynamo and adds nothing.

   **After Phase C, not before.** It is mechanical across ~1,100 references in
   93 files, and the payoff today is zero: the surviving objects link and
   nothing calls them. Doing it during the interface port would collide two
   large mechanical diffs in the same files.

   One thing to check when it happens: `matrix.cpp:233` is live code on a real
   error path, now calling into an engine with no running system behind it. It
   should be established whether that is a null dereference or something
   quieter.

##### Two corrections this deletion forced on this file

**`SIG_DynaMoSimulationData` is not "the site that numbers the DynaMechs
bodies"** (§10 D1, §10 D2, `sigel_eval.cpp:46`). It was the **Dynamo** site.
Both backends were said to walk links → joints → sensors → drives in that
order. **DynaMechs does not, corrected 2026-09-18:** it numbers its bodies by a
depth-first walk over each link's joint list and indexes drives and sensors by
stored number (the Handover section). Either way the orders the check
protects are unchanged and `dictorder-baseline.txt` is
untouched — but the file those sections cite is the wrong one, and it no longer
exists. The live site is `SIG_DynaMechsSimulationData.cpp`.

**V5's zero-hit `applyForce` probe is explained.** §7 records the `applyForce`
breakpoint arming and taking no hits "in a session where the MDH breakpoint
fired 18 times ... real but unexplained". `SIG_DynaDrive::applyForce` is called
from exactly one place, `SIG_DynaMoCommandInterface.cpp:54`, on the **Dynamo**
path. Every shipped experiment selects DynaMechs, so that breakpoint could
never fire. The same holds for the other open probe:
`SIG_DynaSensor::senseJoint1`/`senseJoint2` are called only from
`SIG_DynaMoSimulationQueries.cpp:45,47`. **Both remaining V5 probes were aimed
at code no shipped experiment executes**, and have to be re-pointed at
`SIG_DynaMechsSimulationQueries` / `SIG_DynaMechsCommandInterface` before they
can say anything about this port.

##### Two repo-state findings from checking this work

**Nothing vendored was deleted *by this change*.** That tree is untracked and was
left exactly as it extracts; 10,084 lines merely stopped being compiled. `diff -rq`
against a fresh extract showed five differences across the whole vendored tree:
the four recorded patches, plus the `.sigel-patched` stamp. *(Three folders —
`qt`, `qhull` and `SOLID-2.0` — were deleted from that tree on 2026-09-20, and
the patch count is 12 today, 3 of them outside PVM.)* — ~~**and three stale `.rej` files**~~ *(this listed `cv97/JVector.h.rej`,
`dynamechs/dm/svd_linpack.cpp.rej` and `SOLID-2.0/include/3D/Basic.h.rej`, and
concluded the re-extract cycle was not as clean as §4 describes)* —
**none remains; `find . -name "*.rej"` is empty, so the cycle is clean after
all.**

**The DynaMechs adapters are 2,077 lines**, not the 2,013 measured on
2026-08-20. They grew with Phase V5's MDH probe.

##### Still not fixed, deliberately

The `exit(1)` a glue joint would reach under DynaMechs is at
**`SIG_DynaMechsSimulationData.cpp, initializeJoint`**, not `:394` as recorded above — the
line moved with Phase V5's probe. It is still an `exit(1)` and should still
become a thrown exception. Left alone to keep this changeset single-purpose.

---

#### What was given up

Measured 2026-08-20, before the decision, and unchanged by it.

**Two joint types become dead data.** `SIG_GlueJoint` (186 lines) is still
parsed and still editable, and no simulator reads it. `SIG_CylindricalJoint`
(219) was already dead — both backends dropped it to `default:`. No shipped
robot uses either: all joints in all 7 `.rrb` are rotational — 18, 12, 9, 6, 4,
3, 1.

**Four properties lose their only reader.** `SIG_Material::getFrictionValue`,
`getElasticity` and `SIG_Environment::getVeloDamping` were read by
`SIG_DynaSystem.cpp` alone and are now write-only: parsed, editable, read by
nothing. `getYPlaneLevel` survives for the two renderers. DynaMechs uses global
friction constants rather than a per-material-pair table, so **per-material
friction is the one genuine capability loss.**

**Six simulation parameters become dead**: `getAnalytical`, `getIntegrator`,
`getMaximalIterations`, `getMaximalCollisionLoops`, `getSkipFrames`,
`getSolveMode`. They join `getMaximalError` and `getMaximalSOLIDIterations`,
which were already parsed, displayed, editable and simulated by nothing.

**In full:** glue joints, closed kinematic loops, link-to-link self-collision,
mesh-accurate collision geometry, per-material-pair friction. **None of it is
used by any shipped robot or experiment.**

---

#### Why Dynamo was the one to go

**It crashes on any robot with a joint sensor.** `SIG_DynaSystem.cpp:801` sets
a new sensor's joint pointer to zero and nothing ever assigns it. Then
`SIG_DynaSensor.cpp:28` does `switch (joint->joint->getJointType())`, and
`SIG_DynaMoSimulationQueries.cpp:46` dereferences `sensor.joint->joint`
directly. A null dereference on the first `SENSE` instruction.

Joint sensors per shipped robot: walker 18, insect 12, octopus 9, runner 6,
twoBases 1, hammer 0, shortHammer 0. **5 of 7 robots could not run on Dynamo at
all.**

Two more defects in the same files: `SIG_DynaSystem.cpp:335` inverts the
no-collision test, permitting collision precisely when the pair is on the
must-not-collide list; and `SIG_Simulation.cpp:120-149` throws on every Dynamo
diagnostic, including plain `"Warning:"` lines.

**The authors agreed.** `SIG_SimulationParameterBase.ui:143,154` labels the two
choices `"Dynamo  (not recommended)"` and `"DynaMechs   (preferred)"`, and
`SIG_GUIGPExperiment.cpp, slotRobotInfo` refuses to open the Robot Information dialog unless
DynaMechs is selected. All 14 shipped experiments carry `SIMULATIONLIBRARY 1`.

---

### Finished to-do items, and the refusals — moved here 2026-09-20

`future_refactorings.md` is the to-do list and holds only work to do. What had
been ticked there, and what was refused there, is recorded here instead, because
this file is the only log of what was done. The numbers are the ones the items
carried; other items and this file cite them, so they do not change.

#### Done

- [x] **4. Dynamic exception specifications** — done by the port, Phase A7.
  Three sites keep them only as `// NOTE: in 2003 this carried…` comments,
  which item 30 covers.

- [x] **17. `pvm_probe`'s error return is read as "a message is ready"** —
  done 2026-09-17, D41. `checkTask` now splits the three returns: above zero
  receive, zero wait, below zero give up on the task, kill it and re-queue the
  individual. The old `if (info != 0)` sent an error into the receive branch,
  where `pvm_recv` blocks for a message that cannot come — and **no
  `TIMEOUTMINUTES` value rescued it**, because the timeout lives in the `else`
  branch — or fails and leaves the task record destroyed while every caller in
  `SIG_GPManager` reads -1 as "not ready yet" and waits for ever.

- [x] **21. A run does not notice when the PVM daemon goes away** — done
  2026-09-17, D41. `pvm_probe` and `pvm_spawn` both report `PvmSysErr`, the
  trainer records it, and the interface ends the run and says why.
  **Two ways of detecting it were measured and rejected.** `pvm_mytid` cannot:
  `BEATASK` is `( pvmmytid == -1 ? pvmbeatask() : 0 )`, so an enrolled task gets
  its cached tid back without touching the daemon. Watching
  `$PVM_TMP/pvmd.<uid>` disappear cannot either: a run completed three further
  generations with that file removed, because an enrolled task keeps its socket.
  *Signature of the fault: main thread in `hrtimer_nanosleep`, seconds of CPU
  over hours, no `sigel_slave` at all.*

- [x] **22. Say why a run ended at once** — done 2026-09-17, D42.
  `SIG_GUIGPExperiment::slotEvolutionStopped` says so when the run completed no
  generation, nobody stopped it, and `terminationAlreadyMet` finds the condition
  already true. It names the setting that caused it and the tab to change it on.
  **Two cases it deliberately does not cover.** MetaGP with `SAVEEXIT` set —
  which all 14 shipped experiments carry — completes its first generation, so
  the branch never runs and only the counter moves. And a run that ends for any
  other silent reason says nothing rather than guess.

- [x] **31. Confirm the run lock is finished** — done 2026-09-18.
  **No leftovers:** `g_runningEvolutions`, `SIG_GPManager::running()` and
  `evolutionRunningActionGroup` have no match in `src/`, `include/`, `ui/`,
  `guidrive.cpp` or `check.sh`. Of the other names holding "running",
  `evolRunning` and `metaEvolutionRunning` are MetaGP's own run state,
  `slotEvolutionRunning` is `MT_Controller`'s and `simulationRunning` is the
  slave's 3D playback — none is a second mechanism.
  **One mechanism:** `SIG_GUIGPExperiment::evolutionRunning`, read through
  `SIG_ExperimentListView::isRunning`, and one signal `evolutionNotRunning`
  driving `SIG_MainWindow::slotEnableEvolutionRunningActions` over 29 actions
  and the two `slotEvolutionNotRunning` slots.
  **The one gap is closed.** `startEvolutionAction` and `stopEvolutionAction`
  had no check; `runlock` now reads both at rest, during a run on the running
  experiment, and during a run on another experiment. Teeth-tested both ways.
  That is also what makes the right-click `Stop` crash unreachable — PORTING.md
  section 9.

- [x] **40. Keep one two-bases experiment; remove the other five, here and on the
  x86 machine.** All six were judged side by side on 1.3 and the port, 2026-09-19,
  each with its best individual by the port's own scoring: **keep
  `twoBasesHardlyReducedIS`**; remove `twoBasesHighMutationRate` (a close second),
  `twoBasesSimpleFitness1`, `twoBasesReducedInstructionSet`,
  `twoBasesHighCrossOverRate` (the same file as `twoBasesSimpleFitness1`) and
  `twoBasesSimpleFitness2`, which is broken everywhere and adds nothing — its
  individual 0 scores 3.4e-05 on both, and its best
  (number 78) moved only slightly.
  **Every one of the five is in `dictorder-baseline.txt` and
  `fitness-baseline.txt`**, so both move. `twoBasesSimpleFitness1` is also the
  input of `check.sh`'s `expstruct selfcheck`, and `replicate.sh` names it and
  `twoBasesHighCrossOverRate` as the identical pair.
  **`twoBasesSimpleFitness2` is the hard one.** It is the experiment most checks
  load, so removing it moves them all:
  in `check.sh` it drives six sections — `gui behaviour`, `real clicks` and
  `pagesave` through `BEXP`, and `no clipped controls`, `form minimums` and
  `slave gui`, which name the file directly; `guidrive.cpp` loads it by default;
  `guibehaviour-baseline.txt` names it 24 times, `xtest-baseline.txt` 2,
  `fitness-baseline.txt` 3, and `dictorder-baseline.txt` has its section.
  **`pagesave-baseline.txt` was 1.3's own output for this file**, and the port
  matched it. It now holds the port's own save of `twoBases.exp`; no new
  capture from the oracle — see item 44.
  Removing it also ends item 39's conflict over this file.
  **Done on both machines 2026-09-19.** On the x86 machine with approval
  there; its seven files hash as ours. Its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.

- [x] **44. Keep one octopus experiment; remove `octopusSimpleFitness`, here and
  on the x86 machine.** Decided 2026-09-19: one octopus experiment is enough, and
  this one is an early or failed run. Keep `octopusNiceWalkingFitness`, judged a
  keeper side by side with
  individual 13. The same wait as item 40: not before every experiment has been
  judged.
  **What names it:** `fitness-baseline.txt` (3 lines), `dictorder-baseline.txt`
  (its section) and `check.sh`'s v2 round trip, which loads it as `V2OCT` for the
  containers that collide.
  **That check compares against 1.3's own output for this file**, in
  `verification-against-sigel-1.3/v1-1.3-roundtrip.txt`, and the port matches it.
  The two octopus robot blocks differ — sha256 `2b56be22…a2d1` against
  `8934a27c…fc2d` for `octopusNiceWalkingFitness` — so the kept file has no 1.3
  capture. It needs none: moved to the kept file, the check takes the port's own
  output as its reference. Decided 2026-09-19: a fact already proven is not
  re-checked, and later work checks against the proven port. The same holds for
  item 40.
  The 2026-09-18 oracle measurements named "octopus" in PORTING.md were taken on
  this file; they stay as the record of what was measured.
  **Done on both machines 2026-09-19.** On the x86 machine with approval
  there; its seven files hash as ours. Its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.

- [x] **45. Keep one runner experiment; remove `runnerSimpleFitness`, here and on
  the x86 machine.** Judged side by side with individual 30, 2026-09-19: the first
  runner experiment is the better one. Keep
  `runnerNiceWalkingFitness`. The same wait as item 40.
  **What names it:** `fitness-baseline.txt` (3 lines) and
  `dictorder-baseline.txt` (its section). No check loads it.
  `verification-against-sigel-1.3/` holds oracle measurements on it; they stay as
  the record of what was measured.
  **Done on both machines 2026-09-19.** On the x86 machine with approval
  there; its seven files hash as ours. Its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.

- [x] **46. Rename the kept experiments to their base names, here and on the x86
  machine.** The names carry notes added run by run as a kind of versioning, such
  as the fitness function used. Decided 2026-09-19: once the review is done, only
  the base names stay, each proposed and approved. One name at a time, after
  the review and after items 40, 44 and 45. Both machines use the same names, so
  the two stay comparable. Each kept experiment takes its robot's name.
  Approved: `twoBasesHardlyReducedIS` → `twoBases`; `hammerNiceWalkingFitness` →
  `hammer`; `insectNiceWalkingFitness` → `insect`; `octopusNiceWalkingFitness` →
  `octopus`; `runnerNiceWalkingFitness` → `runner`;
  `shortHammerNiceWalkingFitness` → `shortHammer`; `walkerNiceWalkingFitness` →
  `walker`. All seven approved 2026-09-19.
  **Done on both machines 2026-09-19.** On the x86 machine with approval
  there; its seven files hash as ours. Its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.

#### Not doing

Decisions, not work. Each is settled; reopen only with a reason.

- **Restore Qt 2's spin-box editing.** D28. Reachable only by typing a number
  outside a box's range, the value is visible before anything is saved, and the
  cost is owning a custom widget for ever. Current behaviour is pinned in
  `guibehaviour-baseline.txt`. The worked-out `SIG_SpinBox` subclass, the three
  cheaper routes that were measured and rejected, and the 47-widget promotion
  plan are in `future_refactorings.md`'s history at `1dba5f4`.
- **The history block grows by one line per individual per save.** Confirmed on
  the 1.3 binary; the port reproduces it exactly, which is the correct outcome.
  Recorded so nobody "fixes" it and silently diverges. If it is ever changed
  deliberately, that is a product decision and needs a note in PORTING.md saying
  the port stopped matching 1.3 on purpose.
- **A lower bound on the terrain index in DynaMechs.** Vendored code is not
  touched.
- **Validating the terrain header in `SIG_Environment`.** After the atomic
  write, nothing in the tree produces a `Terrain.ter` that exists and does not
  parse, and the guard would have to be repeated in the second reader,
  `SIG_DynaMechsSimulationData`, which this port does not touch.
- **The malformed-picture-file branch of `generateTerrain`.** Reached only with
  `FLOORFUNCSELECTED 0`; `operator>>(istream &, string &)` leaves its string
  unchanged when the sentry fails, so the previous token is reused and the
  terrain header comes out malformed. No P2 file exists in the repository and all
  14 shipped experiments have an empty `FLOORPICTUREFILE`, so a fix would be code
  no check could exercise. Reopen only if a picture-file experiment ever exists.

---


**No effort estimates in this file.** The column that held them carried six
invented figures. Step counts are counted and stay. **Do not put estimates back.**

**A single fitness evaluation runs locally with no PVM** — that is `sigel_eval`
and all of Phase R. What has no local path is the **evolution loop**:
`SIG_GPFitnessTrainer` dispatches every evaluation through `pvm_spawn` of
`sigel_slave` with no in-process fallback, and the one method that looks like a
fallback, `SIG_GPExperiment::calculateFitness`, is a stub that returns 0. So
without PVM the ported interface builds and shows its windows, and nothing
happens behind the Start button. **With the vendored PVM up it runs locally** —
three generations driven end to end on 2026-09-02.

**CLOSED 2026-09-16, sixteen commits, each with the five checks green.** The
File > Open crash; the whole run lock — one evolution at a time, every
experiment's Start button, five parameter pages, Experiment page and individuals
list dead during a run, the tree menu's Start, Stop and Export too; every dialog
in the master interface given a parent; the spelling and grammar of the text the
user reads; the comments cut from about 1300 lines to 400; `actExperiment`
renamed to `currentExperiment`; the window and splitter sizes; and a prompt
before quitting that names a running evolution. Decisions D38, D39 and D40 in
§5c. **Defects found on the way and fixed:** a use-after-free on right-click
Start during a run, `setName` not re-keying `menuDict` which left the tree menu
dead for every saved experiment, a wildcard disconnect that killed double-click
everywhere, and two harness reads that failed under load.

**WHAT IS ACTUALLY OPEN, as of 2026-09-18.** Closed items are not listed; their
lessons live in the step sections above. **Closed 2026-09-17:** the slave
crashes, the swallowed `pvm_probe` error, a run that loses PVM in silence, and a
run that ends at once in silence — D41 and D42.

**THE PORT IS NOT FINISHED.** No Qt 2 API remains in live code — swept
2026-09-05 over 33 spellings across `src/` and `include/`, code split from
comment, zero in code. **Phase 0 closed 2026-09-05.**

**THE CONVERSION WORK IS DONE.** Its last three items — Phase 0's last comment
lines, 28 doc comments naming a Qt 2 type, and six form minimums — closed on
2026-09-05. Each is written up in §7.

**~~Right-click `Stop` on an experiment that never ran crashes~~ — UNREACHABLE,
read in the source 2026-09-18.** `SIG_GUIGPExperiment::slotStopEvolution` sets
`guiGPManager->userTerminated` and `guiGPManager` is 0 until a Start passes the
three checks in `slotStartEvolution`, so the slot still has no null check and
1.3 still has the same code. **Both routes to it are shut.** `pushbuttonStop`
carries `enabled=false` in `SIG_ExperimentViewBase.ui`, and only
`slotStartEvolution` turns it on; `slotEvolutionStopped` and `slotStopEvolution`
turn it off again. `SIG_GUIGPExperiment::slotRightClick` sets
`stopEvolutionAction` from this experiment's own `isRunning()` on every popup.
**Nothing checks either guard** — `startEvolutionAction` and
`stopEvolutionAction` appear nowhere in `guidrive.cpp`. That is the gap to
close, not the crash.

**File > Open Experiment no longer crashes when the tree gets focus during the
load — FIXED 2026-09-16, `5de5776`.** `SIG_ExperimentListView::slotLoadExperiment`
adds the tree item, then reads the file, then inserts the experiment into
`experimentDict`. The read runs `processEvents`, in
`SIG_GPPopulation::readFromFile` and again in
`SIG_AllIndividualsView::slotCompleteRefreshList`, so a focus event from the
window manager could make the item current before its experiment existed and
`SIG_ExperimentListView::slotSelectionChanged` dereferenced a null pointer. 1.3
made the item non-selectable during the load; **Qt 6 makes even a non-selectable
item current on focus-in, and a disabled one it leaves alone** — measured with a
control. The load now clears `Qt::ItemIsEnabled` as well, and the two
`experimentDict` lookups the tree reaches are null-checked. Checked by the
`openfocus` scenario, which sends the focus event the window manager sends while
the file is being read; without the fix it segfaults.

**~~Slaves crash during a GUI run~~ — FOUND AND FIXED 2026-09-17. The cause was
the shared `Terrain.ter`.** `SIG_Environment::generateTerrain` wrote
`$SIGEL_ROOT/Terrain.ter` in place with `trunc` and
`SIG_Environment::loadDynaMechsEnvironment` read it straight back;
`SIG_DynaMechsSimulationData` reads the same path again. **One evaluation opens
that file five times: two truncating writes and three reads**, measured with
`strace`. Four slaves share it, so a reader could open the file between another
slave's truncate and its first write, get an empty one, and end with
`x_dim = 0`. `dmEnvironment::loadTerrainData` tests the open but never the read,
so a file that does not parse leaves the grid at 0; `getGroundElevation` then
clamps `xindex` to `x_dim - 2`, which is -2, and dereferences `depth[-2]`.
**The fix is in `generateTerrain` alone:** write `Terrain.ter.<host>.<pid>`,
then `rename` it over `Terrain.ter`, so a reader sees the old file or the new
one. Readers, `SIGEL_Simulation` and DynaMechs are untouched.

**What was measured, on both sides of the fix.** A reader loop parsing the
header the way `loadTerrainData` does, with four writers running: **9553
unreadable headers in 15,903,454 samples before, 0 in 16,444,433 after**, and 0
in about 1.5 M with no writers either way. Two real slaves caught under `gdb`
during real runs both showed `x_dim` 0, `y_dim` 0, `grid_resolution` 0 at the
fault, with an ordinary contact position — so it was never a runaway simulation
or a bad individual.

**The 2026-09-15 evidence this was found from.** A run of `twoBasesLocal.exp`,
termination by time with a date in 2030, 4 slaves on this machine. The PVM
daemon log has 123 lines `Invalid storage access`, from 18:34:11 to 19:53:02.
`sigel_slave.cpp, sigelStandardSignalHandler` prints that text on SIGSEGV, sends
fitness 0 to the master and exits. 1.3 has the same handler. Each line is one
individual: its slave crashed, so it got fitness 0. Slaves kept starting at about
one per second: the PVM task ids rose by 3937 from 18:34:11 to 19:33:18, and a
10-second sample saw 10 slaves. The two-generation run in §7, "A real evolution
under `guidrive`", had no fitness value of 0. **The crashes come in bursts** —
86 seconds held one, 14 held two and 3 held three — which is what one shared
file does and not what a bad individual does. *`twoBasesLocal.exp` was deleted
2026-09-17 by decision: nothing to compare it against, the robot behaved
oddly, and it no longer improved. Any experiment reproduces this, since the
defect does not depend on the experiment.*

**NOT OURS — confirmed on the 1.3 binary by the oracle.** `dmEnvironment.cpp`,
`dmEnvironment.hpp` and `dmContactModel.cpp` are byte-identical to
`v1.3-pristine` and no patch touches them; pristine `SIG_Environment.cpp` has
the same writer and the same callers, and the port changed only
`ios::trunc+ios::out` to `std::ios::trunc | std::ios::out`. The oracle ran
`sigel_slave` against an empty read-only `Terrain.ter` and got the same call
path with `x_dim` 0, `y_dim` 0, `grid_resolution` 0 and `xindex` -2, and
disassembled the clamp: `x_dim` at offset `0x24`, `add $0xfffffffe,%eax`, no
lower guard. Its control with a valid 5058-byte file simulated normally. **How
often 1.3 hits this was never measured** — those probes were stood down.
§7's "Not doing" list carries the two routes the fix does
not close, and item 37 the one that is still open.

**Before blaming the slaves, rule out a hang from outside.** On 2026-09-17 a run that had
been "started" for ten hours turned out to have lost its PVM daemon at the one-hour
mark: `pvm-check.sh` was run against the same `PVM_TMP` and deletes the daemon
socket. The symptom is easy to mistake for a slave problem — the window answers,
`Stop` works, the interface still says a run is on. **What it looks like:** the
main thread in `hrtimer_nanosleep`, a few seconds of CPU over many hours, and no
`sigel_slave` process at all. A slave crash looks different: slaves keep starting,
the daemon log fills with `Invalid storage access`, and fitness values of exactly
0 come back. **That SIGEL-side defect is fixed — D41.** `SIG_GPFitnessTrainer::checkTask`
records `PvmSysErr` and `SIG_GUIGPExperiment`'s progress timer ends the run and
says so, so the wait is no longer silent or endless.

**Do not run `pvm-check.sh` while an evolution is live.** It removes
`$PVM_TMP/pvmd.$(id -u)` and `pvml.$(id -u)` and starts its own daemon. Give it
its own `PVM_TMP`, which it honours, or wait.

**`real clicks` failed once on a raw event count.** On 2026-09-15 at 23:41 its
`fast pair` line counted one extra X motion event (`motion=2 xi2=10` against the
baseline's `motion=1 xi2=9`). Every Qt-level count on that line stayed the same.
The next run on the same tree passed. **It happened a second time on 2026-09-16**,
on the `outside click` line rather than `fast pair`, again with every Qt-level
count unchanged. Sections 1 to 5 of `xtest-baseline.txt` now print `motion>0` and
`xi2NoMotion`, which a spare X motion event cannot move. **Section 6, enter and
leave, still prints three raw `nativeMotion=` counts, and that is where it
failed a third time on 2026-09-18** — every Qt-level count again unchanged, and
the next run on the same tree passed. The mitigation never reached that section.
`guidrive.cpp, spies` carries the reason.

**The run lock is DONE — 2026-09-16, `69b3e4a`, `6494ab6`, `a541679`.** One
evolution at a time, and the whole application is locked while one runs.
`SIG_GUIGPExperiment::slotStartEvolution` refuses a second run, whichever route
reaches it — that also closed a **use-after-free**, because a right-click `Start`
during a run re-entered the slot through `haveABreak`'s `processEvents` and ran
`delete guiGPManager` on the manager whose `start()` was still on the stack.
`SIG_GUIGPExperiment::slotRightClick` sets `startEvolutionAction` from
`SIG_ExperimentListView::isRunning()` and `stopEvolutionAction` from this
experiment's own `isRunning()`, which also makes the right-click `Stop` crash
unreachable. `SIG_GUIGPExperiment::slotEvolutionNotRunning`, driven by
`SIG_ExperimentListView::evolutionNotRunning`, locks **every** experiment: the
Start button, the five parameter pages, and the Experiment page's history box,
autosave slider, comment box and two plot buttons. The same signal now drives
`SIG_AllIndividualsView::slotEvolutionNotRunning` for every experiment, so no
individuals list can start a slave through `Visualize` mid-run. `Stop` stays live
for the experiment that is running. **Two defects came out of it**:
`SIG_GUIGPExperiment::setName` re-keyed `widgetDict` but not `menuDict`, so the
tree menu was dead for every saved experiment; and
`SIG_AllIndividualsView::slotEvolutionNotRunning` disconnected with a wildcard,
which also cut `QTreeWidget`'s own relay for `itemDoubleClicked` — 1.3's line,
and it killed double-click on an individual. Both fixed. Checked by `runlock`,
which reads eleven widgets of two experiments and the second experiment's
individuals list, each with a control.

**D29's arming line was checked by the `evolution` scenario, and it held.** The
tree click is the only thing that reaches it —
`SIG_ExperimentListView::slotSelectionChanged` asks `isRunning()`, which only
`slotStartEvolution` sets — and `Save Experiment`, one of the 29 locked actions
that nothing re-enables, stayed greyed through it. *That run used the counter
that the run state replaced on 2026-09-15. Re-running it is open.*

**THREE COVERAGE ITEMS WERE DROPPED 2026-09-07. The attribution on this line
was corrected 2026-09-08: only the first one was a decision.** The other two are
this document's own arguments, once written as if they were decisions. A note
filed that way cannot be argued with, and nobody decided it should be. Each of
the three was this document's own proposal, and each failed a plain
question about what it would prove:

- **"Commit the 30-generation curve."** A randomised search gives a different
  curve every run. What matters is that the GP system works at all, not any one
  result. *Decided: concrete results always vary in a randomised search, and the
  port is not tied to any one experiment or run.*
- **"Check the evolution path."** Checked before dropping: the pieces are already
  checked separately — the arithmetic by `fitness-check` (42 evaluations, also
  sanitised), the PVM transfer by `pvm-check`'s `pvm_link`, the ordering by
  `dictorder`, the run lock by `runlock`. What only a full run adds is the
  trainer's spawn-and-collect bookkeeping and the generation loop, and **those
  fail loudly** — nothing spawns, or nothing returns. The one quiet failure, a
  whole population of exact 0.0 from a swallowed throw, is already asserted in
  the scenario. Minutes on every `check.sh` run, forever, to catch a failure
  that announces itself.
- **"V2 — a save path in `sigel_eval`."** The wrong shape. `expstruct.py`
  already fingerprints the population block — individuals, names, program text,
  ordering, `POOLGENERATION` — and drops every float on purpose. The GUI already
  writes a complete experiment; a 2026-09-07 run wrote 1.5 MB and the harness
  read `POOLGENERATION` and 100 fitness values back out of it. **Nothing is
  missing in code.** If a comparison against 1.3 is ever wanted it is
  `expstruct.py` on a file from each side, which needs the oracle rather than a
  new save path.
  *Re-opened 2026-09-08, because this bullet was written as a decision and the
  argument was this document's own. The conclusion holds, and the reason is now
  much stronger than
  "the wrong shape": `sigel_eval` links the SLAVE variant of the writer, and
  `sigel_slave` never calls it, so a save path there would test dead code, which
  is not
  tested — decided 2026-09-08. The V2 row in the Phase V table has
  the full argument and the reshaped step.*

**A MetaGP evolution is NOT open — it is unreachable on both versions.** `Start`
enables only once the training set fills; filling it needs a run; the MetaGP
window is application-modal so the run cannot proceed while the window showing
`Start` is open; and no `.mcnf` setup ships to shortcut it. Both versions refuse
for the same reason, which is an equivalence result rather than a gap.

**The most useful part of this section is C11's hazard list** — measured traps,
all forward-looking, and the thing most likely to save a future session time.

**A warning this section has earned.** Four claims were withdrawn in two days —
the item-5 reachability argument, the evolution headline, a slider divergence and
a seed. Each was inherited from a document and repeated before being checked
against source. **Verify before citing anything here.**


### C13 — a real click on both ends, 2026-09-07

`guidrive` now runs as a normal X11 client. It runs inside a nested `Xvfb` with
`QT_QPA_PLATFORM=xcb`. `xdotool` sends the input. The scenario is `xtest`. The
baseline is `xtest-baseline.txt`. The check is `real clicks` in `check.sh`.

The oracle measured the same three questions on the 2003 binary on the same day.
It used its own `Xephyr` with no window manager, and `python-xlib` `fake_input`.
Both sides send XTEST input to a nested server with no window manager. That is
why the two sides compare.

Neither display has a window manager. That much is the same, and it had to be
checked first: a different focus mode sends key events elsewhere, and any
difference would then come from the display and not from the port.

**The focus states are NOT the same, and an earlier version of this section said
they were.** Review measured it. An idle nested `Xvfb` reports `focus:
PointerRoot`, which is a focus mode and not the root window. The oracle's
`Xephyr` reports focus on the root window itself. The port then takes focus for
itself. Polling `xdpyinfo` from outside during a run shows X input focus change
five times before the first real click, ending on the main window and staying
there. 1.3 never calls `XSetInputFocus`, and with no window manager nothing else
does, so 1.3 stays on the root. The focus rows below therefore compare a port
that claims focus against a version that never does.

| question | 1.3, real XTest | this port, real XTEST | |
|---|---|---|---|
| Double click opens the Individual View | yes, and only a real double click does it | yes: `itemDoubleClicked` once, one window | **agree** |
| Two single clicks 1.2–2.0 s apart | no window. The probe still worked: the same clicks moved the selection and filled the properties pane | no window, `itemDoubleClicked` 0 | **agree**. This control is what makes the row above mean anything |
| The interval | 400 ms. The oracle bracketed it between 0.3807 s (opens) and 0.4012 s (does not), over twelve trials | `QApplication::doubleClickInterval()` reports 400 | **agree**. The two halves are independent. The oracle measured the behaviour and read no source. This side read the number and did not run 1.3. Qt 2.3's source gives the same number: `qapplication.cpp:280`, `int QApplication::mouse_double_click_time = 400` |
| Is there a real pointer grab while a menu is open? | yes. `XGrabPointer` from a second X client answers `AlreadyGrabbed` while the File menu is open, and `GrabSuccess` when it is closed | the popup is a `QMenu`. `QWidget::mouseGrabber()` is null, which is Qt's own record and not the X grab | not compared directly. See below |
| Does the dismissing click reach the widget below? | yes. One click closes the menu and selects the row under it. The list moved from row 1 to row 3. On the tree it also changed the right-hand page | no. The press goes to the `QMenu` only. The list selection does not move | **DIFFERS** |
| Second click on the same menubar item | the menu stays open. The oracle checked the map state, not the focus | the menu closes | **DIFFERS** |
| Does a real click move X input focus? | no. Focus stays on the root window. 1.3 never calls `XSetInputFocus`, and with no window manager nothing else does | no, but only because the port already holds focus. It takes the main window during startup, after five focus changes that all happen before the first click | the click changes nothing on either side, but the states it starts from differ |
| Where focus sits while a menu is open | on the popup. After an outside click it is `None(0)`, so keys go nowhere until something sets focus again | on the main window the whole time | differs. This is Qt, not SIGEL |

**The swallowed click is the difference that matters.** It is in the divergences
table. 1.3's side is not a guess, because Qt 2's source is on SourceForge and in git history.
`qapplication_x11.cpp:3402-3416` runs when the last popup closes. If the press
was outside the popup, it calls `XAllowEvents(…, ReplayPointer, CurrentTime)`.
The X server then delivers that press again, to the window below. The same code
subtracts 10 s from `mouseButtonPressTime`, so the repeated press cannot count
as a double click. The oracle measured this without reading any code. The first
run with real input found the difference. `QTest` could not have found it,
because `QTest` never creates the pointer grab.

**Four probe errors, all mine.** Three were caught by a control. The fourth was
caught by the check.

- The first outside point was inside the menu. The File popup covers the
  experiment tree. The probe clicked the menu and reported that the popup
  survived an outside click. That is a false difference, and it points the
  opposite way to the real one.
- The second point was the row that was already selected. Probe 2 had just
  clicked it. The selection could not change, whatever the port did. The oracle
  avoided this by clicking row 3 while row 1 was selected.
- The third point was a `QLabel`. Nothing there can change. "Nothing happened"
  then looks the same as a swallowed click.
- The fourth was in the check, not in the scenario. `xdotool` has no `--display`
  option. It answers `getdisplaygeometry: unrecognized option '--display'` and
  exits 1, whatever the server is doing. So the "display already in use" guard
  could never fire. The readiness loop could never succeed either. The loop was
  written as `... && break` inside a `for`, so it became a fixed 15 s wait. Runs
  by hand still worked, because nothing there checked the loop result.
  `check.sh` did check it, and failed with `Xvfb never came up on :97`. Both
  places now use `DISPLAY=:N xdotool …`. Both directions were then tested: the
  check succeeds on a live display and fails on a dead one. This is §7's "check
  a lookup found something", made inside the check that exists to enforce it.

**A fifth round of review found four more, and three of them let a bad run
pass.** All are fixed and all are demonstrated:

- **The `!!` markers were in the middle of their lines.** `check.sh` greps
  `^ *!!`. So the three messages that say the probe mis-targeted were invisible
  to the guard whose whole job is to stop a bad run being baselined. Review
  produced a run, under `QT_FONT_DPI=120`, whose own output said
  `UNDECIDABLE, the control did not fire` and which passed every predicate the
  section had. The markers now start their lines, and the scenario returns 1
  instead of carrying on.
- **A scaling variable in the caller's environment mis-aimed every click.**
  `mapToGlobal()` gives logical pixels and `xdotool` takes device pixels. At
  `QT_SCALE_FACTOR=1.25` the clicks were real, so the provenance control still
  said `DISCRIMINATES`, but they landed 20 per cent away and the run reported a
  false difference from 1.3. `check.sh` uses `env` without `-i`, so the whole
  environment passed through. This is the only section here that is exposed:
  offscreen pins the ratio, and review confirmed `QT_SCALE_FACTOR=1.25` leaves
  the offscreen `check` scenario byte-identical. `check.sh` now clears the
  scaling variables, and the scenario refuses any `devicePixelRatio` other
  than 1.
- **Section 5 could report a false difference with no marker at all.** A
  mis-aimed run printed "a fast double click opens nothing", which reads as a
  divergence from 1.3. It now prints a line-initial `!!` when two clicks inside
  the interval produce no `itemDoubleClicked`.
- **Pixel geometry from the local font and style was in the baseline.** The
  popup rectangle and the chosen click point were printed. Under
  `QT_STYLE_OVERRIDE=Windows` the File popup is 228x235 rather than 261x214,
  which moves both. Neither is printed now. The outside point is list **row 3**
  by index, the same row the oracle used, and the probe checks that the popup
  does not cover it rather than searching for a row that it does not.

**The control makes the finding readable.** The baseline prints it directly
below the result. The same click, at the same point, with no menu open, does
select the row: 16269 → 16494 in `twoBases.exp`. Without that line the result means nothing. If
the control ever stops working, the check prints `UNDECIDABLE` instead of a
verdict.

**Double click synthesis agrees, and it is the deepest agreement here.** Two
real presses go in. Qt decides for itself whether they are one double click. The
threshold is 400 ms on both sides. Slow clicks open nothing on either side.
`QTest::mouseDClick` supplies the `DblClick` event ready-made, so it never tests
that decision.

**Two counts in the baseline are not one per click.** A real click gives
`native press=2`, because Qt's xcb plugin uses XInput 2 and both the master and
the slave device report. The same click gives `qt press=2`, because
`QWidgetWindow` gets it first and the `QWidget` second. The `pressOn=[...]` line
names the receivers. It answers the swallowing question:
`[QWidgetWindow, QMenu]` with a menu open, `[QWidgetWindow, QWidget]` without
one.

**`nativeEnter` and `nativeLeave` are 0 in section 6, and that is correct.** X
sends crossing events per X window. A Qt 6 top level is one X window. Moving
between two widgets inside it crosses no X boundary. Qt works out those `Enter`
and `Leave` events itself, and the `qt` columns show them.

**What this does not show.** Section 3 samples activation before and after a
click. Both samples read `isActiveWindow=1`. That is a settled state, not a
change, because the port had already activated the window during startup.

*An earlier version of this paragraph explained the `1` as "there is no window
manager, so nothing deactivates it". That is backwards, and review showed it. A
run in which no click ever lands reports `isActiveWindow=0` throughout, and
section 2 of the baseline shows `act=1` on the first real click. Activation does
happen here. Section 3 simply samples too late to catch it.* If a later session
adds a window manager, section 3 will move. That is not a regression.
The grab itself is only proved on 1.3's side, by the oracle, with `XGrabPointer`
from a second client. Nothing here repeats that test. This side measures the
result of the grab, which is what a user sees.

### C12 — the MetaGP window's last undriven corner, 2026-09-04

`MT_Editor`, `MT_AddConstantsWidget`, `update statistics` and MT_GUI's toolbar
actions, driven here for the first time and checked as `metadrive`. **The question
was core functionality, not appearance.**

| what | 1.3, measured by the oracle | this port | |
|---|---|---|---|
| `MT_Editor`, **Return** | commits the typed value | commits | **agree** |
| `MT_Editor`, **Escape** | discards, original kept | discards | **agree** |
| `MT_Editor`, **focus away** | discards, original kept | discards, editor hides | **agree** |
| AddConstants, letters | rejected in both modes, field left empty | same | **agree** |
| AddConstants, `12.5` | stays `12.5` in float, becomes `125` in integer | same | **agree** |
| AddConstants, `-50000` in **float, as opened** | accepted | **dropped the minus — `50000`** → **FIXED**, accepted | **was a SIGN FLIP; restored** |
| AddConstants, `1.23456` and `9.87654321` in float | kept in full — **`decimals` is not enforced during typing at all** | **truncated to `1.2345`** → **FIXED**, kept | **was precision loss; restored** |
| AddConstants well-formedness, float | `abc` → empty, `1.2.3` → `1.23`, `--5` → `-5` | identical | **agree** — Qt 2 enforces **form only**: one leading minus, one point, no constraint on magnitude or decimal count |
| AddConstants, `-50000` in **integer** | keeps `-50000`, and **generates three constants of `-50000`** | kept `-5000` → **FIXED 2026-09-04**, keeps `-50000` | **was a data divergence; restored** |
| AddConstants, OK with count 5 | constants 30 → 35, no progress dialog | 30 → 35 | **agree** |
| `update statistics` | nothing visible (0 changed pixels, against a control that a real tab switch moves 34,188); Fitness fields read `ERR` | nothing visible; **the same three fields read `ERR`** | **agree on STATE — but see the limit below** |
| arrival enabled map | Start, Stop and Delete disabled; all others enabled | identical | **agree** |

**THREE PORT DEFECTS, all in the Create-constants dialog, all reaching the
GENERATED CONSTANTS rather than a display.** Qt 6 says Invalid in three places
Qt 2 said Intermediate:

    QIntValidator(-10000,10000)        typed -50000  -> "-5000"   tenfold
    QDoubleValidator(100000,-100000,4) typed -50000  -> "50000"   SIGN FLIP
    inherited fixup() on focus-out     typed 123.456789 -> 123.46 precision

**ONE DELIBERATE DEVIATION FROM THE TRANSCRIPTION, and it is the only one:
`toInt`, not `toLongLong`.** Qt 2's `QString::toLong` capped at `INT_MAX/base`
whatever the platform's `long` width, so it failed past ±INT_MAX; the first
version was over-permissive there. *One gap remains and is not closed: Qt 2's
`toDouble` required full consumption of the string, so `"1.5 "` was **Invalid**,
where Qt 6's `QString::toDouble` succeeds and `Qt2DoubleValidator` answers
**Acceptable**. Reachable by typing a trailing space; it does not change a stored
value. The int validator has no such gap.*

**Both validators are TRANSCRIBED from vendored `qvalidator.cpp`** as
`Qt2IntValidator` and `Qt2DoubleValidator`, rather
than patched on top of Qt 6's answers — patching fixed one case at a time and
missed the other two twice running. Both override `fixup()` as a **no-op**,
because Qt 2's `QIntValidator` and `QDoubleValidator` do not override it at all
and Qt 2's `QLineEdit` called it only on Return, never from focus-out.

**DO NOT "FIX" THE SWAPPED BOUNDS.** 2003 builds the float min validator as
`QDoubleValidator(100000.0, -100000.0, 4)`, bottom above top. It reads like a
defect and **does nothing observable on either version** — a prediction that it
narrowed the range tenfold was falsified on both binaries. Qt 2 enforces
well-formedness only: one leading minus, one decimal point, no constraint on
magnitude or decimal count. Measured agreement on eight inputs including
`abc` -> empty, `1.2.3` -> `1.23`, `--5` -> `-5`.

**D28 does not extend here** — see the TRAP in §7. There the differing value is
visible in a spin box before anything is saved; here it becomes data.

**WHAT THE `update statistics` CHECK CANNOT SEE.** It pins that the action
exists, is enabled, can be triggered without raising a box or changing the page,
and that the three Fitness fields read `ERR` exactly as 1.3's do — a real
agreement, because 1.3 does nothing visible there either. **It cannot tell
whether the slot behind the action ran**: gutting
`MT_StatisticsWidget::slotUpdateGUI()` to `return;` leaves the output
byte-identical. The `ERR` values are the `.ui`'s own static text, and the two
fields that are not `ERR` are written by `onShow()` at page-raise. **An agreement
about state, not a test of the slot.**

*Twelve probe errors came out of this step and the recurring shapes are in §7's
probe-craft list. The one worth repeating: the scenario CLOSED THE WINDOW IT WAS
TESTING, which produced three separate wrong conclusions including a claim that
focus-out was undrivable.*

### C11 — the coverage gap C10 leaves, closed 2026-09-02/03

C10 found two defects in fifteen driven scenarios, and that density was the
argument for continuing. **The gap is closed.** `guidump-baseline.txt` lists 58
entries — 42 menu items, 16 toolbar buttons. C11a drove all six View pages,
C11b all sixteen Import/Export children, C11c the six dialogs, C11d `MT_GUI` and
`SIGEL_SlaveGUI`, C12 MT_GUI's remaining toolbar actions, and the evolution path
ran end to end on both machines. Per-step findings are in the Phase C step table,
the divergences in the divergences table, the probe lessons in §7. Kept below is
only what a future run of this comparison needs.

**The enable/disable sweep matches**: Start GREYED and Stop ENABLED for the whole
run, then back. **The fitness curve is `experimentHistory`, one entry per
generation**, and it grew by exactly 3 to 139 entries, gen 1..139 contiguous, on
both machines.

**On 1.3 the generation counter DOES NOT MOVE BY ITSELF during a run.** The two statements
that would update it (`SIG_GUIGPManager.cpp`) are **commented out in the sources
as released**, one under `// update generations display (this line looks cool,
doesn't it ?!)`. On 1.3 the only live update is inside
`SIG_ExperimentView::putIntoExperiment()`, not a refresh hook — so it moves on
experiment selection and page switch, and never when a run ends. 187 samples here
over 554 s and 172 on the oracle across two generation boundaries all read the
starting value, and both versions then kept the old value after the run until a
page switch. *Since 2026-09-15 the port also writes the counter during a run, in
`SIG_GUIGPManager::updateIndividualView` (D37), and when a run ends, in
`SIG_GUIGPExperiment::slotEvolutionStopped`; see the divergences table.*

**EVERY SHIPPED EXPERIMENT TERMINATES ON A DATE IN 2001** —
`TERMINATIONUSESDATE 1`, `TERMINATIONTIME 2001`, all 14 — **so a correct Start
finishes in under 100 ms.** Sampling at 5 s and at 100 ms both missed it, and
only a `QSignalSpy` on `signalEvolutionNotRunning` catching `false` then `true`
distinguished it from a Start that did nothing. *The oracle measured ≈4 minutes
per generation on the 2003 i386 box, which is why C10 could not have seen a
generation either.* *Since 2026-09-21 the seven files in `experiments/` stop on
1 January 2030, 12:00, so a Start there runs until a lever stops it.*

**Reproducing the evolution comparison.** `guidrive.cpp`'s `evolution` scenario
takes **`SIGEL_GENERATIONS=N`**, which selects `Generation` in
`comboboxTerminationBy` and types N into `spinboxByGenerationNumber` on the
**Evolution control** tab, then reads the value back OUT of the experiment rather
than trusting the widget. It saves through `File > Save Experiment` at the end,
because a run that leaves no artefact cannot be diffed. The combo's items are
User / Time / Generation / Time-or-generation, which is **not** the enum order
(`byTime`, `byGeneration`, `byTimeGeneration`, `byUser`): index 2 maps to enum 1.
The old `SIGEL_RUN_LONGER` duration path is kept as the `else` branch. With
neither lever, the `evolution` and `pvmcrash` scenarios refuse to Start while
the loaded experiment's end date is still to come.
*The input is BUILT, not shipped:* `data/Experiments/twoBasesSimpleFitness1.exp`
with the GP `RANDOMSEED` — the **second** of the two, the first being the
simulation's — set to 12345, the eight 2003 PVMHOST lines replaced by one local
host at max-processes 1, and the paths rewritten.

**`expstruct.py` is the comparison tool**, in the repo because two machines had
to run identical logic rather than two readings of a spec. It prints counts, a
per-individual line in file order, and `ORDER`/`PROGRAMS`/`SHAPE`/`HISTORY`
hashes, and **drops every float by construction** — `--audit` prints every line
it refused to look at, `--selfcheck` asserts both halves of the property
everything rests on (blind to fitness, sighted on structure) and is a `check.sh`
section. **`ORDER`/`PROGRAMS` are all-or-nothing and hid an 87%-identical
population behind one mismatched hash** — when they differ, get the
per-individual names before concluding anything.

**WHAT THE FINGERPRINT CANNOT SEE, and it is not a small surface.** The `FLOAT`
net that keeps fitness out also drops **every live input containing a float**:
the 13 `<CMD> CommandParameters <f>` mutation step sizes, the `Link` /
`Geometry` / `Material` morphology lines, the environment constants — 22 such
lines in `twoBasesSimpleFitness1.exp`, 42 in `octopusSimpleFitness.exp`. The
concrete danger, found by the oracle rather than reasoned about: **`MOVE
CommandParameters` is `0.01` for twoBases and `0.1` for octopus**, a tenfold
difference in mutation magnitude on the one command with large integer operands.
A reconstructed input that got that wrong would evolve differently and **every
hash exchanged between the two machines would still agree.**

*Closed not by hashing the surface but by an exact diff of it*, between the
shipped experiment and the built input. **The cross-machine half needed no hash
exchange at all**, only that both sides derive from the same ancestor:
`twoBasesSimpleFitness1.exp` md5 `35bcdb3a7a2bb6c2af7ccf964761e87e`,
`octopusSimpleFitness.exp` md5 `d6a73c806137f492f285914e0e131c49` on both. **The
oracle's diff came back identical UNSORTED, which is the load-bearing part** —
and that works only because these inputs are built by rewriting
seed/PVMHOST/path lines TEXTUALLY. **Anything rebuilt through the GUI's
load-and-save must be normalised first**: 1.3 permutes the `CommandParameters`
block, the `Link`/`Geometry` order, and the named attribute groups within a
single `Link` line, all values preserved; Qt 6 permutes differently again.
Expected, not a divergence.

**THE PORT EVOLVES. Measured 2026-09-03 — the first measurement of OUTPUT this
project has made.** Population 100, 30 generations, twoBases with
`SimpleFitnessFunction` (fitness = distance / simulated seconds). **Best fitness
0.063794 → 0.141625, never decreasing at any generation.** Mean 0.011711 →
0.046059. 4 slaves, 61.6 s/generation, `POOLGENERATION` 136 → 166. The pool's
stored fitness was cleared first through `Individuals > Reset`, because survivors
otherwise keep their 2003 i386 values and a curve drawn from two architectures
means nothing. **No conclusion is drawn beyond the one question that is
answerable** — does best fitness improve for a reasonably large population over a
reasonable horizon. It does. Whether 0.14 is plausible, whether the parameters
are well chosen, what a flat stretch would imply: not answerable from one run of
a randomised process, and not asserted here. *Future runs should RANDOMISE the
starting pool rather than fitness-reset the shipped programs.*

**CROSS-MACHINE CONTENT CANNOT BE COMPARED, AND THE COUNTS THAT SEEMED TO SHOW
OTHERWISE ARE FORCED.** A claim that matching `NEXTIDENTIFIER`,
`POOLGENERATION`, `INDIVIDUALS` and `HISTORY` counts showed the GP drawing
identically on i386/x87 and aarch64/IEEE **is withdrawn.** Identifiers are taken
in the tournament CONSTRUCTORS, which all run inside `createTours` **before any
tournament runs and before any fitness is read**; at 50 tournaments a generation
plus one integer draw each, the per-generation delta is
`50 + Binomial(50, XOVERPROB/1000)` — computable from the .exp alone: *twoBases*
`XOVERPROB 670` predicts 250.5 ± 5.8 over three generations against a **measured
245**, *octopus* `XOVERPROB 300` predicts 195.0 ± 5.6 against a **measured 193**,
and the crossover rates implied by the data (0.633, 0.287) recover the file's.
**A replay of `SIG_Randomizer`'s LCG from seed 12345, in forty lines of Python
with no fitness, no physics and no machine, reproduces generation 1's 80 exactly
and octopus's 62** — the per-generation deltas were 80/84/81, `NEXTIDENTIFIER`
12552. Estimated probability that all three agreed even under maximal divergence:
**~98.6% for twoBases, ~60% for octopus**, so the octopus "replication"
replicates nothing. *The `/1000` holds because both files' probabilities happen
to sum to exactly 1000 (280+670+50 and 0+300+700); the code divides by their sum
(`SIG_GPManager.cpp:249-259`), so that is a property of the corpus, not of the
code.*
**And generation 1 cannot diverge in the first place** — `evalNewIndis` evaluates
only individuals with fitness < 0, and every individual in the input has positive
fitness, so no simulation runs before its tournaments. `POOLGENERATION` is 136+3,
`HISTORY` one append per generation, and pool size cannot change at all. What
survives: **contents cannot be compared across these architectures** (the
tournament decides on a bare `>=` between two physics-produced doubles);
**`SIG_Randomizer` is architecture-independent by construction** — `next` is
`unsigned long`, 32-bit on i386 and 64 on aarch64, but the LCG's low 32 bits are
congruent at both widths and `getRandomInt` returns only bits 16-30, a stronger
statement than the counts ever made; and **not ruled out** — a different
tournament EXECUTION ORDER gives identical counts and different contents with no
float comparison involved, a mechanism already measured at 64 of 100 names
between two 8-slave runs on ONE machine — **`runA==runB` and `serA==serB` show
each machine is internally stable; they do not show both used the same order.**
*And the raw "87 of 100 identical" is restated: 21 slots are untouched survivors
of the input, so agreement over the slots actually REPLACED is 66 of 79 = **84%**,
and the surviving 21 skew hard to high fitness (median 0.842 against 0.035), so
survivorship does much of the work. Sample is one seed, two models, and
effectively two generations of divergence on an already-converged 136-generation
pool.* The measurement to make instead is a
draw counter in `SIG_Randomizer` dumped per generation — **and it can only ever
be half a measurement, because the oracle has no source tree.** Design no check
that assumes both halves can be instrumented.

**THE SLAVE GUI NEEDS NO PVM AND NO MASTER**: `sigel_slave -visualize <exp>`
loads an experiment, takes individual 0 and opens the window; `guidrive`'s
`slavegui` scenario does the same in process. **THE ROBOT MUST BE PREPARED
FIRST** — `SIG_RobotRenderer`'s constructor walks every link's geometry, null
until `instantiateGeometries()` has run, so passing the experiment's robot
straight in SEGFAULTS. Copy it and call `prepareDynaMechs()` first, as the slave
does.

**NEITHER SIDE CAN SEE THE GL AREA.** `QT_QPA_PLATFORM=offscreen` gives no FBO
(`QOpenGLWidget: No fbo, cannot render`), so **any** conclusion about the 3-D
view from the offscreen harness is worthless — use `DISPLAY` +
`QT_QPA_PLATFORM=xcb`; under Wayland the menu popups fail and the scenario
cannot even open an experiment. The oracle's capture of that region returns a
constant image while the scene demonstrably moves. Agreement about the rendered
view is unobserved on both versions and must not be claimed. *On X11 the view
does render here and the position labels read 0/1/0, matching 1.3.*

**Layout, all measured under Qt 6.10.2 rather than derived.** **Exactly two
containers in all 20 converted forms** place their children by ABSOLUTE
geometry, which Qt 2 sized from those children
and Qt 6 sizes from a sizeHint that ignores them — `GroupBox6` ("Navigation",
90x37 against a needed 220x331) and `groupboxDirectory` ("File conventions" in
the movie dialog, 465x37 against 401x99). **Measured 2026-09-05 by deleting each
`<minimumSize>` block and rebuilding**: the first hides **14 controls** — seven
50x50 navigation buttons, three position labels with their three value readouts,
and `traceRobotCheckBox` — and the second hides the whole `Layout15` carrying
file prefix, leading-zeros, image format and JPEG quality. Both fixed with a NEW
minimum, because the pristine forms carry no size either. **The Navigation one
was invisible to every widget-level probe**: the buttons were present, correctly
sized and at 1.3's exact offsets, and only a SCREENSHOT showed the group was too
small to contain them. **Both are checked by `slave not clipped`, added
2026-09-05** — *not by `no clipped controls`, which walks only the master's View
pages and structurally cannot reach either container. This document claimed the
wrong check for two days, until a review checked it.* **C2's six dropped size
constraints are COSMETIC** and stay dropped: three `maximumSize` width ceilings
in `MT_IndividualWidgetBase.ui` (a dropped ceiling can only let a widget grow
wider), a 200 px minimum in `MT_PopulationWidgetBase.ui` of which 190 is covered
by `individualListView`'s own, and two in `MT_StatisticsWidgetBase.ui` carrying
Qt's literal defaults. **Six forms declare a minimum smaller than Qt 6's layout
needs**, inherited from 1.3 and not port regressions, but **fixable with the same
`minimumSize` mechanism the two group boxes above used** — the numbers and the
instruction are in §9's open list. Otherwise the 20
forms are clean, checked rather than assumed: 623 widgets, no negative
coordinates, no zero-sized geometry, no `min > max`, no child outside its
parent's declared size, no two grid items sharing a cell, every tab page laid
out, and the nine `enabled=false` widgets all re-enabled in code.

**Hazards a follow-up must inherit:**

- **Plain `grep` silently skips nine Latin-1 files** including
  `SIG_MainWindow.cpp` — see the TRAP below. Use `command grep`. **The CRLF half
  of this hazard is GONE since D31 (2026-09-09): the tree is LF only and no
  tracked file carries a CRLF**, so there is no longer a line ending to preserve
  and no reason to edit in binary mode for that. The Latin-1 half stands. What
  the CRs cost is still worth knowing: a text-mode Python edit once silently
  stripped 934 of them from `MT_Controller.cpp`, caught only because
  `check.sh`'s `encodings` section exists.
- **Any harness must link `$(MASTER_OBJ)`**, or the Clean `SIG_GPExperiment`
  leaves `mtController` uninitialised and produces a convincing false crash.
- **`slotStartEvolution` BLOCKS, so a sampling loop written after the click sees
  only the finished state.** `guiGPManager->start()` runs the whole evolution
  inline, and the GUI survives only because `SIG_GUIGPManager::haveABreak()`
  calls `qApp->processEvents()`. Anything that must observe a RUNNING evolution
  has to be armed BEFORE the click — the `evolution` scenario uses a `QTimer`,
  and the oracle hit the same shape driving 1.3 with XTest.
- **An unset `SIGEL_ROOT` segfaults every slave IN 1.3** — and PVM tasks inherit
  *pvmd's* environment, not the master's, so it can be set for the master and
  still absent for the slave. Set it for the daemon. The site is
  `SIG_Environment::generateTerrain()`, and **this port already guards it**;
  `v1.3-pristine` has the unguarded `string terrain(sigelRootCString)`.
  **Residual, and still live:** `SIG_Environment.cpp:409` builds a `QString`
  from the same unchecked `getenv` — harmless in Qt 6, but it silently yields
  `/Terrain.ter`.
- **`pvmd` resolves the slave executable relative to the directory in the
  PVMHOST line**, not `SIGEL_ROOT`. A wrong directory gives
  `pvm_spawn() failed ... (0/-7)`, once per individual. Here the two coincide,
  which is a property of this setup and not a general one.
- **`tearDownPvm()`'s `pvm_halt()` BLOCKS FOR EVER at exit, so every evolution
  run that started its own daemon ends by being killed** — by PVM itself, well
  before any `timeout`. *This said "the exit status of an evolution run means
  nothing", and that is no longer true:* `guidrive`'s `main` now flushes, records
  the status and installs a SIGTERM handler that re-exits with it, so a passing
  run gives **0** and a watchdog abort **3**. The
  halt itself is unchanged and still never returns. Pre-existing
  and deliberately unchanged: `pvm_halt()` is what stops the daemon this process
  started, and dropping it left `pvmd3` and its slaves running. Consequence:
  **a `printf` before an early `return` is LOST unless flushed on that path**,
  because stdout is block-buffered when redirected and the process never reaches
  exit. Flush on the failing path, not after the block.
- **Above one slave there is no baseline at all.** Two same-seed 8-slave runs on
  the oracle's single machine differed in 64 of 100 individual names, and
  whether that is GP evaluation order or the PVM layer is UNMEASURED. Any
  evolution comparison must pin PVMHOST field 2 to 1.
- **A page remembers its tab** — the View toolbar does not reset the tab widget.
  **Each file a scenario saves shifts the load dialog's row positions**, so a
  multi-run sequence can silently re-load what it just wrote. **Order the probes
  so a toggle cannot grey a field before it is typed into** — one radio click
  turned five subsequent probes into false negatives. **`QFileDialog` navigates
  as a path is typed and strips the directory out of the field**: use
  `acceptFileDialog()`, do not hand-roll a second one.
- **Qt 6's `getSaveFileName` raises its own "already exists" box when the typed
  name exists.** It is modal inside `accept()`, nested under the click on Save,
  so a handler armed after the click never runs. The `overwrite` scenario types
  the extension on purpose and answers the box with a poller started before the
  click. A probe that types an existing name needs the same.
- **Two clicks are not a double click.** A helper that sleeps 0.35 s and re-moves
  the pointer between them puts them ~750 ms apart, past Qt 2's 400 ms
  `doubleClickInterval`, and they arrive as two singles. The reusable control is a
  genuine double click in the load dialog, which accepts with no Open click. *The
  port reaches the editor through `itemActivated`, which fires on double-click or
  Return **only while the style says activate-on-single-click is false** — so that
  style hint is pinned in the check.*

**The oracle is the reason this works.** Both C10 defects were confirmed against
the running 1.3 binary *before* anything was changed — in one case that
confirmation is what distinguished a Qt 6 regression from a 1.3 defect that had
to be preserved.

### Ownership hazards Phase C inherits (was: the Phase B audit)

**No `setAutoDelete` call remains anywhere — this said 11, and C6/C7 removed the last of them.** Every remaining occurrence of the name, and of `QDict`, `QArray`, `QListViewItem`, `QPtrList` and `QCString`, is in a comment. Re-measured 2026-09-05 by sweeping `src/` and `include/` for 33 Qt 2 spellings and splitting code from comment: **zero in code.** The only apparent exception, `QButtonGroup`, is Qt 6's class, not Qt 2's widget-flavoured one.

**Those comments named the member by its Qt 2 type as though it were current — SWEPT 2026-09-05, 26 lines in 14 files.** See "Doc comments naming a member by its Qt 2 type" below. *The port's own historical notes — "Qt 2's `QArray` was writable through `at()`; `QList`'s is not" — are correct and stay; the sweep excluded them by looking for `Qt 2` / `used to` / `was` / a step number on the same line.* **One residue the sweep missed**, found 2026-09-18: `SIG_GPFitnessTrainer.h`'s constructor `@post` still says *"The QArrays are initialized with ZERO."* It is the only 2003 mention left; the other three in the tree are the port's own notes.

*Every step is DONE and enforced by a compiler that no longer accepts the Qt 2 alternative — the types themselves are gone from Qt 6, and the whole tree builds.*

**A `getFoo()` returning a container by reference puts free sites in other
modules**, including modules that do not compile yet. Grep the accessor, not
just the member name. `SIG_GPParameter::getHostList()` is how B3 leaked into
`SIGEL_MasterGUI`, and C7 meets it again.

*This section carried the pre-Phase-D audit — a 47/38/9 `setAutoDelete` split,
a count of hidden-free sites and a list of owning containers with no free path.
Every entry is closed: the hidden frees in D15, D18, D19, D25b and D25c, the
no-free-path class by D25c, and `SIG_Robot::clear()` by D4. The per-container
record is in those D-steps and the pristine-tree counts are in §2.*

### TRAP — `QTextStream >> char` skipped whitespace in Qt 2 and does not in Qt 6

**Qt 2's `QTextStream::operator>>(char &c)` was `c = eat_ws()`** — skip
whitespace, return the first non-whitespace character (`qtextstream.cpp:1029`,
and `eat_ws` at `:212`). The doc comment on the `QChar` overload immediately
below it says "Note that whitespace is \e not skipped", drawing the contrast
explicitly. **Qt 6's returns the very next character, whitespace included.**

**Three sites, all in `SIG_GPPVMHost::SIG_GPPVMHost( QString input )`, and the
effect is silent and total.** The parser primes with two char reads to step over
the space and the opening quote, then walks to the closing quote:

    inputStream >> name >> maxSlaves >> enabledInt >> buffer >> buffer;
    while (buffer != '"') { dirString.append(buffer); inputStream >> buffer; }

Measured on 6.10.2 with 1.3's own line, with its host and folder replaced,
`host05 2 1 "/home/user/…"`: the
first read returns `' '` where Qt 2 returned `'"'`, the second returns `'"'`
where Qt 2 returned `'/'`, the loop therefore never executes, and **every PVM
host parsed from an `.exp` file gets an empty slave directory.**

**It also drops spaces inside the quoted path, and that is preserved.** Each
read skips whitespace, so Qt 2 turns `/tmp/with space` into `/tmp/withspace`. A
2003 defect; converted, not fixed.

**`SIGEL_GP` was converted in Phase D and has been green on all four checks ever
since.** Nothing could see this: it is not a compile error, and no check reads a
`PVMHOST` line. **That is the actual lesson** — the checks cover what the checks
read, and a file format the program parses but no check opens is covered by
nothing at all. `check.sh` now has a `parsers` section that round-trips a
`PVMHOST` line; it fails on the naive conversion.

*Found while answering an unrelated question about a checkbox two files away.
The x86 box reported that `Edit host`'s `Enable host` box was unchecked for a
host the file marks enabled; chasing that led to the parser. **The finding was
not in the thing being examined.***

*Sweep: three `>> buffer` sites, all in this one constructor. No other file in
the tree extracts a stream into a `char` or `QChar`.*

### TRAP — Qt 2 INSERTS WHERE Qt 6 APPENDS: three defects, one root cause

**Noted 2026-09-05, after the third instance.** The same difference has now
produced three separate defects in three separate places, and they were each
fixed as if they were unrelated:

| where | what it did |
|---|---|
| `QListViewItem( QListView * )` | **prepends**; 26 sites in C7, 9 in C6. Wrong startup page, reversed command list, "Load Experiments" selecting the first file where 1.3 selected the last |
| `Q2Dict::insert` | **prepends**; the `.lap` export order, and the default-constructed command order that had corrupted `dictorder-baseline.txt` for all seven robots |
| `SIG_ExperimentItem` in the tree | **prepends**; a NEW experiment lands at index 0 and a previously loaded one moves to 1 |

*The third is not a port defect — the port is correct, because C7 restored Qt 2's
prepending deliberately. It is an AUTOMATION trap, and it bit this side's own
driver: a scenario deleted `topLevelItem(0)` as "the loaded one" and removed the
newly created experiment instead. **Select by name, not by index**, in anything
that drives this GUI — including the 1.3 GUI, where it is equally true.*

**The standing rule this earns:** wherever the port restores a Qt 2 insertion
order, index-based access from OUTSIDE the port — probes, drivers, scripts — is
wrong by default. The order is deliberate and it is not the one a Qt 6 reader
expects.

### TRAP — `QListViewItem` construction PREPENDS: 28 sites in C7, 9 in C6

**Qt 2's `QListViewItem` constructor inserts the new item at the *head* of its
parent's child list, not the tail.** `QListViewItem::insertItem` is

```cpp
newChild->siblingItem = childItem;
childItem = newChild;
```

so a loop that constructs items in file order builds a list in **reverse** file
order. Qt 6's `QTreeWidgetItem(parent)` and `QTreeWidget::addTopLevelItem`
**append**. A like-for-like port therefore reverses every one of these lists,
and nothing in the compiler, in `check.sh` or in either baseline can see it.

**Confirmed on the running 1.3 binary, not only in `qlistview.cpp`.** The x86
box loaded `twoBasesSimpleFitness1.exp`, whose `PVMHOST` entries are in file
order `host06, host16, host04, host07, host02, host22, host05, host01`, and
GP-Parameters → PVM draws them **exactly reversed**, `host01` first and `host06`
last.

**And it is visible there for a reason that generalises.** The view is sorted on
column 0, which holds only a tick pixmap and no text — every sort key is
identical, and the comparator has no tiebreak (`qlistview.cpp:754`, a plain
`key.compare`, handed to C `qsort` at `:809`, which the standard does not
require to be stable), so insertion order survives in practice and the prepend
shows through. *This is why C1's sort-direction work was correct and this is still
wrong: on this view the sort is not what orders the rows.*

**37 construction sites — 28 in `SIGEL_MasterGUI` and 9 in `MT_GUI`**, because
the trap is a property of the **constructor**, so every subclass inherits it.
Counting only the literal type name misses four subclasses:

| | direct `new QListViewItem` | via a subclass | total |
|---|---|---|---|
| `SIGEL_MasterGUI` | 24 (23 live, 1 commented) | `SIG_ExperimentItem` 2, `SIG_IndividualListItem` 2 | **28** |
| `MT_GUI` | 0 | `MT_ExperimentItem` 6, `MT_PopListViewItem` 3 | **9** |

*This has now been wrong twice. First "27, three in `MT_GUI`" from a name-keyed
sweep; then "24, `MT_GUI` has none" from a constructor-keyed sweep that keyed on
the wrong constructor. The second correction was more confident than the first
and no better. **Key on the hazard, not on a spelling.***

**And `MT_GUI`'s six are the strongest case in the tree — stronger than the PVM
one above, because nothing masks them.** `MT_ExperimentWidget`'s constructor:

```cpp
setSorting( -1 );                                    // sorting OFF
new MT_ExperimentItem( this, 5, "Statistics",   … );
new MT_ExperimentItem( this, 4, "Selection",    … );
new MT_ExperimentItem( this, 3, "GP Parameter", … );
new MT_ExperimentItem( this, 2, "Population",   … );
new MT_ExperimentItem( this, 1, "Individual",   … );
new MT_ExperimentItem( this, 0, "Strategy",     … );
setCurrentItem( firstChild() );                      // and selects the first row
```

Constructed 5→0, prepended, so 1.3 draws **Strategy first** and selects
Strategy. The `int pos` argument is the author encoding the intended order. A
like-for-like `QTreeWidget` port draws Statistics first **and selects
Statistics** — a different startup page. *Confirmed on the running 1.3: the
`SIGEL MetaGP` window's left-hand list reads `Strategy, Individual, Population,
GP Parameter, Selection, Statistics` — construction order exactly reversed, with
sorting explicitly off. Source, Qt 2 semantics and the binary all agree.*

**`SIG_ExperimentListView` calls `setSorting(-1)` too**, so a newly created
experiment appears at the **top** of the list, not the bottom.

*`MT_GUI`'s `insertItem` calls are mostly menus, combos and list boxes — but
three of them are `individualListView->insertItem( new MT_PopListViewItem(…) )`,
which are `QListView` items and are counted above.*

**Two of the 37 populate `listviewHosts`, in the form C1 converted and review
passed.** The form is correct; the defect is in the *population* code, which is
C7's and still Qt 2. Nothing is owed at C1 — but the pairing is the point: a
form can pass every check this project has while the code that fills it is what
holds the behaviour.

**`SIG_ExperimentItem.cpp, SIG_ExperimentItem` is a second, independent trap in the same file.**
`QListViewItem *childItem = new QListViewItem( newItem );` — `newItem` is the
*Robot* item, so `Language-Parameters` is a **child of Robot**, not a sixth
sibling. The four siblings above it use the same `newItem` variable and their
`setText` calls all look alike, so reading the labels rather than the
constructors gives a flat list of six. The 1.3 tree draws it indented.

### The register-to-index modulus, and the overflow under it

`SIG_DynaMechsCommandInterface.cpp:74` and `SIG_DynaMechsSimulationQueries.cpp:93`
map a register value onto a drive or sensor slot:

```cpp
int absoluteDriveNo = driveNo - static_cast< int >(minRegisterValue);
driveIndex = absoluteDriveNo % static_cast< uint >(simulationData.drives.size());
```

**The `uint` cast is load-bearing and D9 nearly lost it.** `Q2PtrVector::size()`
returned `uint`, so this modulus was unsigned and its result was always in
`[0, size)`. `QList::size()` is signed. D9 converted both containers and left
the expression alone, which silently turned a negative `absoluteDriveNo` into a
negative index — read into `drives[]`/`sensors[]`, and **write** into
`driveForcesTimeAccounts[]` at `SIG_DynaMechsCommandInterface.cpp:217,257`.
Found by the D9 review, restored, and the cast now carries a comment saying why.
It is the `Q2Array`/`Q2PtrVector` clamp story again in a place no clamp was
visible: `gcc` does not warn on `int % qsizetype`, so the five vanished
`-Wsign-compare` warnings were **not** the full inventory of `uint size()`
dependencies, and treating them as one was the mistake.

**Under it sits a real defect, preserved not fixed.** `minRegisterValue` is
`-2^(w-1)` for register width `w`, and `SIG_LanguageParameters`'s default
constructor sets `bitsPerRegister (32)` (`SIG_LanguageParameters.cpp:29`). At
`w = 32`, `driveNo - (int)(-2^31)` overflows `int` for every non-negative
`driveNo` — undefined behaviour, and the wrapped result is what the modulus then
folds back into range. All 14 shipped `.exp` carry `w` of 3 or 8, where
`absoluteDriveNo` stays in `[0, 2^w)` and nothing overflows, so no check sees it.
**Not fixed here**: the wrap decides which actuator a `MOVE` drives, so changing
it changes simulation results against 1.3. It predates the port.

### `QTextStream` default codec

232 sites. API unchanged, but Qt 2 defaulted to Latin-1 and Qt 6 to UTF-8. A
no-op over this corpus — the 2003 binary's own `.exp`, `.rrb` and `.wrl` output
is pure ASCII. Becomes live the moment a non-ASCII experiment file exists.

### ~~Toggling containers (Phase C)~~ — CLOSED BY C7

**Both are now `QHash`es owning nothing** (`SIG_ExperimentListView.cpp:63-65`,
`SIG_GUIGPExperiment.cpp:183-189`), so there is no `autoDelete` to toggle and
nothing here needs per-site thought. The table is kept because it records
what the Qt 2 code did. *It said:* two containers flip `autoDelete` at
runtime — Qt 2's "remove without deleting" idiom, which a single destructor
free does not reproduce; both GUI, both needing per-site thought:

| Container | File | Toggles |
|---|---|---|
| `experimentDict` | `SIG_ExperimentListView.cpp` | 3 pairs (102/110, 229/236, 249/256) |
| `widgetDict` | `SIG_GUIGPExperiment.cpp` | 1 pair (206/210) |

### Reference data and reference output

Experiment and robot files are **not** in any tarball — separate downloads,
verified live 2026-08-18:
`sigel.sourceforge.net/download/experimente/experiments.tar.gz` (12 `.exp`) and
`.../robotermodelle/robots.tar.gz` (7 models, `.rrb` + `.wrl`).

Both downloaded 2026-08-22 to `data/`, which is untracked, with the two runner
`.exp` from their own archives. **Since 2026-09-19 the 7 experiments kept are
tracked** in `experiments/`, and their robots in `robots/<robot>/` with the
`.rrb`, `.wrl` and `.blend` — §10, after D2's migration note.

The 2003 i386 binary runs on Debian woody libraries; all 12 experiments
validated end to end, one run captured verbatim (13 generations, 874 slave
evaluations). Two limits as a baseline:

- the run is bounded by **wall-clock, not generation count**, so counts and
  timings are machine-specific
- `SAVEEXIT=1` makes SIGEL **overwrite the experiment file it was given** and
  re-emit defaulted keys, so an evolved `.exp` is not byte-comparable with its
  input

**What the x86 box can and cannot be asked — inventoried 2026-08-27** by the
`sigel-x86` session, recorded so it is not re-derived:

| | |
|---|---|
| an instrumented rebuild | **impossible, permanently.** There is no SIGEL source on that machine at all — only the compiled 1.3 release. Not a toolchain problem, so no `-DSIG_DEBUG` build of the reference is ever available |
| the binaries | **not stripped.** 11,709 symbols in `sigel`, 10,936 in `sigel_slave`, 1,760 of them `SIG_*` with full g++ 2.95 mangling. This is what makes V5 possible without a rebuild |
| `gdb` | **installed 2026-08-27**, so V5 is unblocked. `strace` and `ltrace` are still absent, and no probe needs them |
| PVM | **works.** Four evolutions have completed, up to 300 generations at 8 concurrent slaves. `SIGEL_ROOT` must be in **pvmd's** environment, not the shell's, or every slave segfaults with the master idling |
| the master's output | one line per generation, and nothing else. **No fitness value is ever printed.** Fitness lives only in the `.pol` pool images and the rewritten `.exp`, as `FITNESS=<value>`; `-1` means unevaluated, which is what `Individuals > Reset` exploits (V4 is dropped) |
| per-step trajectories | POV-Ray export exists only in `sigel_slave`'s visualiser widget and is driven through its GUI. **Not reachable headlessly** |
| how the binaries run | natively on x86-64 — woody's own loader invoked explicitly against unpacked woody `.deb`s. No chroot, no container, no root |

**`twoTri` on that box is not reference material.** It is not in
`robots.tar.gz`, which holds exactly the 7 models named above, and it appears
nowhere in `data/`; verified 2026-08-27. **Nor are the four `twoTri*` runs one
robot family among themselves** — `twoTriNano` carries 2 `Geometry` lines and
`Material 1 2`, `twoTriDepth250` 4 and `Material 1 12`: different vertex sets,
different bodies. They are also the seed-0 runs. So of the oracle's fifteen
reference runs, **only the twoBases and octopus ones trace to a shipped
experiment and only those are usable as cross-machine references** (oracle,
2026-09-02). That box's own experiments, modified
robots and render pipeline are likewise out of scope — its render path
deliberately alters SIGEL's POV output.

**There are two `RANDOMSEED` keys, not one** — `SIG_SimulationParameters`
(`:102`) and `SIG_GPParameter` (`:386`) each parse their own, with different
values in the same shipped `.exp`. Both are already handled; any determinism
claim has to pin both. `q2compat.h:77` still says "a fixed `RANDOMSEED`",
singular, which is stale phrasing rather than a defect.

### Determinism — resolved

Qt 6 randomises the `QHash` seed per process; Qt 2's `QGDict` was deterministic.
This mattered because `SIG_Robot`'s copy constructor round-trips through a text
serialisation that walks six dictionaries, and joint order determines DynaMechs
link numbering — a fixed `RANDOMSEED` would not have reproduced a run.

~~`q2compat.h` calls `QHashSeed::setDeterministicGlobalSeed()` during static
initialisation.~~ **D27 deleted the shim, so nothing seeds implicitly any more.**

`sigel_eval.cpp` calls it as the first statement of `main()`, which is why the
dictorder check is unaffected. **`build/pvm_link` lost it** — `nm -C` finds the
symbol once in `sigel_eval` and **zero** times in `pvm_link`, because
`SIG_GPFitnessTrainer.o` used to pull the shim's per-TU initialiser onto that
link line. Unobservable today: **0 `QHash`/`QSet`/`QMultiHash` in the converted
core**, measured, and a build with a genuinely random seed reproduces
`dictorder-baseline.txt` exactly (D26 review action).

**`QHashSeed::setDeterministicGlobalSeed()` — DONE 2026-09-04, in `sigel.cpp`
only, and it was NOT purely bookkeeping.** Only `sigel` links the three `QHash`es
(`SIG_GUIGPExperiment`'s `widgetDict` and `menuDict`, `SIG_ExperimentListView`'s
`experimentDict`); `sigel_slave` links `GUI_SLAVE`, which has none. *An earlier
version demanded the call in `sigel_slave.cpp` too; that half was mis-scoped and
is withdrawn.* **The "the only iteration site is destruction, which is
order-insensitive" half was wrong on both counts.** There are **two** iteration
sites, both in `~SIG_GUIGPExperiment`, and the first is
`for ( QWidget *w : widgetDict ) widgetStack->removeWidget( w )` — **and that
stack is SHARED**: `SIG_ExperimentListView` hands its own `widgetStack` to every
experiment it constructs. So with a second experiment still loaded, the order in
which one experiment's widgets leave the stack can decide **which page is current
afterwards**, and an unpinned seed makes that differ between runs of the same
binary. *Not data — D26 measured that no hashed container's order reaches a file
— but run-to-run UI non-determinism, which is more than bookkeeping.*

### `MT_Control` is not headless

A8 cut the header include correctly, but **`MT_Controller.cpp` is deeply coupled
to the GUI**: 23 `mainWindow->` accesses, it constructs `new MT_MainWindow`
itself, and it wires toolbar actions with `connect`/`disconnect`. It cannot
compile against Qt6Core alone until `MT_GUI` is ported.

This matters because `sigel.cpp:302` reaches `MT_Controller::startTimedEvolution`
on the headless `-me` path. Either the GUI wiring moves out into `MT_GUI`, or
`MT_Control` is reclassified as a GUI module. **Not decided.**

**AND ON THAT SAME PATH `qApp` WAS A `static_cast` TO A TYPE THE OBJECT IS NOT.
FIXED 2026-09-07** — C8 left it for C9, C9 never closed it, and it stood open until
the one-line fix was asked for. Measured 2026-09-03 by
preprocessing the real translation unit with the Makefile's own include set, not
by grepping a header:

- `sigel.cpp:197` handles `-mtevolve` / `-me`, and `:285` constructs a plain
  **`QCoreApplication`** on that path — deliberately, because it is the Qt 6
  spelling of Qt 2's `QApplication(argc, argv, false)`.
- `MT_Controller.cpp, slotEvolutionRunning` called `qApp->exit(0)` (`:130` after the fix).
- That TU pulls **`qapplication.h`** — 22 references, transitively through
  `MT_GUI/MT_MainWindow.h`, whose chain reaches a generated `ui_<Form>.h` and
  its `<QtWidgets/QApplication>`. So `qApp` expands to
  `(static_cast<QApplication *>(QCoreApplication::instance()))->exit(0)`, on an
  object that is a `QCoreApplication`.

*A direct grep of `MT_MainWindow.h` for `QApplication` returns nothing and reads
as an all-clear; the include arrives **three** headers deeper —
`MT_MainWindow.h` → `MT_EstimationWidget.h` → `MT_EstimationWidgetBase.h` →
`ui_MT_EstimationWidgetBase.h` → `<QtWidgets/QApplication>`. Preprocess the TU.*
**Not a demonstrated crash**: a review probed the pattern under
`-fsanitize=undefined,vptr` without one — `QCoreApplication` is at offset 0 and
`QCoreApplication::exit()` is **static** (`qcoreapplication.h:200`), not merely
non-virtual — and **the path has never actually run**, which is the
real reason nothing has bitten. It is UB either way.

**THE FIX, 2026-09-07: `QCoreApplication::exit(0)`, qualified.** It was left open
for two days on this file's rule that an untested change to unrun code is not an
improvement — and the rule does not bind here, which a review established rather
than argued. `QApplication` and `QGuiApplication` declare no `exit` at all: taking
the address of `&QApplication::exit`, `&QGuiApplication::exit` and
`&QCoreApplication::exit` emits **one** symbol, `_ZN16QCoreApplication4exitEi`,
and `decltype(&QApplication::exit)` is `void(*)(int)` — a plain function pointer,
which independently proves it is static. Old code and new code call the identical
function; the only thing deleted is a cast that emits no machine code. That is
decidable by inspection, so the `-me` path did not need driving to close it.

**Two corrections the same review made to the paragraph above.** *The UB is in the
CAST, not the call* — C++17 [expr.static.cast]/11 makes the conversion undefined
the moment it is evaluated, because the pointee is a complete `QCoreApplication`
and not a base subobject of any `QApplication`. The offset-0 layout is true and
**irrelevant**: `exit` being static means no `this` is formed, so the offset never
enters it. *And therefore the clean UBSan run proves nothing* — with no member
access and no vptr load, `-fsanitize=vptr` has nothing to instrument, so the
absence of a report was expected rather than exculpatory.

**Qualified rather than `QCoreApplication::instance()->exit(0)`**, which this
section used to promise: with `exit` static, `instance()` would be evaluated and
`*p` formed for nothing.

**This does NOT mean the `-me` path works.** It is still undriven by every script
here, and that is unchanged by this fix.

Same module: **4 of 4** remaining `QMessageBox` calls are **interactive** — the
return value drives an `if` or a `switch`, at `MT_Controller.cpp:214`, `:260`,
`:446` and `:924`. Those cannot become console output. *This said "4 of its 15"
until 2026-09-07: 15 is the count in the 1.3 ORIGINAL, and the other 11 have
already become `SIGEL_Tools::SIG_IO::cerr`. The sentence read as open work that
was in fact done. Two of the four, `:214` and `:924`, are not behind the
`guiEnabled` guard that protects `:260` and `:446` — neither is on the
`startTimedEvolution` route, so this is a separate open question, not part of
the fix above.*

### TRAP — `toUtf8()` returns a temporary (all 74 D8 sites)

Qt 2's `latin1()` pointed into the QString's own buffer. Qt 6's `toUtf8()`
returns a temporary `QByteArray`:

```cpp
const char *p = s.toUtf8();                 // DANGLES
const QByteArray b = s.toUtf8();            // correct
const char *p = b.constData();
foo(s.toUtf8().constData());                // also fine: one full expression
```

`toLatin1()` behaves identically, so D8's choice does not affect this. Check
every D8 site for a stored `const char *`.

### Defects fixed rather than preserved (D13)

| Where | 2003 | Now | Why |
|---|---|---|---|
| `Q2Array::sort()` | `memcmp` byte order | numeric | Three sites need ascending numeric order and break once any element reaches 256: `SIG_GPManager.cpp:304,311`, `SIG_AllIndividualsView.cpp, slotDeleteIndividuals`. Qt 2's own source says *"Qt 3.0: Add a virtual compareItems()"* |
| out-of-range array access | ~~warn, clamp to 0~~ **BOTH HALVES OF THIS ROW ARE WRONG, corrected 2026-09-03.** "2003 warned and clamped" is true of **`QGArray::at`** (`qgarray.h:108-117`, `msg_index(index); index = 0;`) and **false of `QGVector::at`** (`qgvector.h:85-92`, which warns and then *reads out of range*) and of **`QGList::at`** (`qglist.h:172-176`, which returns **null**). Three behaviours, and this row is the **fourth** recorded instance of fusing them — in the very table §10's corrected semantics row points at | ~~same, in the shim~~ **nothing clamps today**: `q2compat.h` went with D27, and `shim/` now holds only `fstream.h`, `iomanip.h`, `iostream.h`, `minmax.h`, `new.h`, `strstream.h`, `vector.h`. The port uses `value()`, which yields null — **safer than Qt 2, not equal to it** | The original reason still stands for why a clamp was not replaced by `Q_ASSERT`: it compiles to nothing under `QT_NO_DEBUG`, so a release build would corrupt memory silently where 2003 returned a wrong value. *Neither `QT_NO_DEBUG` nor `NDEBUG` is defined by the Makefile or `check.sh`, so `QList`'s assert is live in both build trees today* |
| ~~`SIG_ProgramLine.cpp:215-224`~~ | writes `element[no]` in the branch entered *because* `no >= size()` | **FIXED** — `:215-232` now guards `if( no >= 0 && no < int(element.size()) )` | Its own comment is `// ToDo: Exception!` |
| ~~`SIG_DynaSystem.cpp:266-268`~~ | deletes `dynaJoints[k]` while looping to `dynaDrives.size()` | **moot 2026-08-28** — the file is deleted with the Dynamo backend, §7 | The two vectors grew independently |
| `SIG_EarlyRunTermSimulation.cpp, getMaxRecorderSteps` | `QTime zeroHour;` | `QTime( 0, 0 )` | Same class as the other 11 `QTime()` sites but a declaration, so the first sweep's pattern missed it. `getMaxRecorderSteps` returned 2 instead of 182 — a factor of 91 on the denominator of three fitness functions. No shipped experiment selects them, so `replicate.sh` cannot see it |
| `sigel_slave`, `getenv("SIGEL_ROOT")` | dereferenced unchecked | to be fixed | Segfaults if unset; the SIGSEGV handler masks it with no core. Bites under PVM specifically — spawned tasks inherit *pvmd's* environment, not the master's |
| `SIG_GPPVMData.cpp, sendQStringToPVM` `sendQStringToPVM` | sends `str.length() + 1`, a **character** count, then sends `str.toUtf8()`, up to 4x longer in bytes | `qCStringBuffer.size() + 2` (D21; was `+ 1` on a `Q2CString`) | `getQStringFromPVM` sizes its receive buffer from that count and lets `pvm_upkstr` write the bytes in. 20 `ü` gives `heap-buffer-overflow ... in byteupk` under ASan; short strings survive only because `QList` over-allocates. Qt 2's `length()` was the Latin-1 byte count, so 2003 was right for its own data. **Changes the wire format for non-ASCII** — safe only because both ends are this file and no distributed run exists. Found by Phase P's P4, regression-tested by `pvm_link.cpp` |
| `SIG_GPIndividual.cpp:557-559` / `:647` | the writer emits `"\n      "` before `}HISTORY END;`; the reader takes everything up to that marker as content, so the separator becomes data | **preserved, not fixed** | Every save grows every `HISTORY` block by 7 bytes, linearly and without limit — 100 blocks is ~700 bytes per round trip. Measured on the 1.3 binary over three consecutive round trips (V8) and confirmed to be the same code here. Fixing it would change file bytes against 1.3. Any check that diffs a round-tripped `.exp` must normalise trailing whitespace inside these blocks |
| `SIG_GPPVMData::sendQStringToPVM`, a **null** `QString` | `Q2CString`'s `const char *` conversion gave `nullptr`, and `pvm_pkstr` does `strlen(cp)` unguarded — a segfault | `constData()` gives `""`; an empty string is sent | Found by the D21 review, which showed the `+ 2` does not reproduce the old length for a null string. It never could: the old path died before the length was used. Unreachable today — the two live callers pass a string built by `savePVMDataTransfer` — but it is a crash removed, not a value preserved, and D21 first claimed otherwise |
| `SIG_GPForceFitnessFunction`'s cleanup loop | a `do`/`while` dereferencing `listForces.first()` **before** testing it | a range-for | `Q2PtrList::first()` returned null on an empty list, so an evaluation that recorded no frames took a null dereference **while freeing memory**. Identical with frames, a no-op without. Contrast D10, where the same shape's once-through was load-bearing and had to be kept — which side of the null the body is written for must be read each time, not pattern-matched |
| `SIG_Environment` terrain load | `getenv("SIGEL_ROOT")` unchecked | already checked, message on stderr | `sigel_eval` says "SIGEL_ROOT is not set, cannot locate Terrain.ter" instead of reading `/Terrain.ter` |
| `MT_GPSystem/MT_FitnessTrainer.cpp, loadSetup` | `loadSetup` sized `Result`/`ResultIst` from the **stale member** `TSetSize` while handing the training set the file's `NewTSetSize` | `TSetSize = NewTSetSize;` | Any setup file with a larger set made `calculateFitness` **write past both arrays**. `setSelektionValue` in the same file always did it correctly. Found by the D4–D6 review; **off the 42-evaluation path, so no check saw it**. *Added to this table 2026-09-03 — it had been recorded only in a D-step paragraph, which the compression then removed* |
| `MT_Control/MT_Substitute.cpp, changeErrorInfo` | `changeErrorInfo` looped to `CorrectFitness.size()`, a **high-water mark that only grows**, while indexing the caller's arrays | bounded by the smallest of the three | Those arrays shrink whenever the selection size is lowered. Same review, same table omission, same date |

~~**Open, from the R1 review:** SOLID is built without the `-DNDEBUG` its own
`Make-config` sets, so eight `assert(!eqz(x))` guards ahead of a division are
live that were not in 2003.~~ **Moot as of 2026-08-28** — SOLID is no longer
compiled at all. It went with the Dynamo backend, its only caller
(see "Dynamo removed, DynaMechs kept" in §7).

### Name collisions that survive into Qt 6

- `QVector`/`QList` — pointers in Qt 2, values in Qt 6. **Silent**: compiles
  clean, then double-frees. The trap the shim exists to defuse.
- `QListView` — a multi-column tree in Qt 2, a flat model-view list in Qt 6.
  Fails loudly, but misleads anyone reading the diff.
- **`QTime()`** — 00:00:00.000 and valid in Qt 2, null in Qt 6. Compiles, runs,
  and hangs: `addSecs` on a null `QTime` gives another null, `secsTo` gives 0,
  and a null `QTime` holds -1 ms so it sorts before every real time. Fixed at
  12 sites; found only by running, which is the argument for §3.

- **`QListIterator`** — a Qt 2 cursor iterator with `current()` and
  `operator++`; Qt 6's namesake is a Java-style iterator with neither. Fails
  loudly. Eight sites in `SIGEL_MasterGUI/SIG_GPParameter.cpp` alone, and
  Phase C's count of 75 container sites includes them without noting the
  interface changed.
- **`QList::remove`** — Qt 2's `Q2PtrList::remove(T *)` removed by value and
  returned `bool`. Qt 6 has only `remove(qsizetype, qsizetype)`. Fails loudly,
  which is lucky: `SIGEL_MasterGUI/SIG_GPParameter.cpp:520` does
  `if (hostList2.remove(host)) delete host;`. The equivalent is
  **`removeOne`** — returns `bool`, does not delete. `removeAll` would be
  wrong.
- **`QList::first()` on an empty list** — `Q2PtrList::first()` returned
  `nullptr`; Qt 6's returns `T &` and is **undefined behaviour**. It compiles
  either way, so a `first()`/`next()` walk converts silently while only the
  `.next()` fails to build. Live at the next step:
  `writeHistoryToFileTransfer` opens with `experimentHistory.first()`
  (`SIG_GPExperiment.cpp, writeHistoryToFileTransfer`, `SIG_GPExperimentClean.cpp:109,162`). All 14
  shipped `.exp` have a non-empty history, but an experiment saved before any
  generation runs does not. Use `value(0)` or a range-for.
- **`QQueue` on an empty queue** — the same trap one level up, and it caught
  D23's write-up backwards. Qt 2's `dequeue()` returned 0: it is
  `QGList::takeFirst` (`qqueue.h:59`), whose `unlink()` opens
  `if ( curNode == 0 ) return 0;` (`qglist.cpp:438-439`; :436 is the
  signature). `head()`, `current()` and
  `operator type *()` all route through `cfirst()` (`qglist.h:188`), also 0 on
  empty. **Every one of them asserts in Qt 6** — `dequeue()` is
  `QList::takeFirst()`, `head()` is `QList::first()`. Measured: abort at
  `-O1 -g`, segfault under `-DQT_NO_DEBUG`. Not reachable through
  `MT_Trainingset::updateTSet`, the only drain today. **Live for Phase C**:
  `MT_GUI/MT_ExperimentWidget.cpp, slotCurrentChanged` calls `prevSelectedItems.head()` with no
  emptiness guard, so `lastSelected()` before any selection returned 0 in 2003
  and will abort now. **STALE — Phase C closed this.** `MT_GUI` is in `check.sh`'s `MODULES` (`check.sh:94`) and the Makefile's `CORE`, and C11c's `metagui` scenario drives the whole MetaGP window. *It said:* `MT_GUI` is in neither `check.sh`'s `MODULES` nor the
  Makefile's `CORE`, so nothing flags it.

### TRAP — plain `grep` in this environment SKIPS A NON-UTF-8 FILE SILENTLY

In this shell `grep` is a **function**, not the binary: it execs `ugrep` with
`-I`, which means *ignore binary files*. ugrep classifies a non-UTF-8 file as
binary, and `-I` drops it **with no message and exit status 0** —
indistinguishable from "no matches". Use `command grep`, which bypasses the
function, or `grep -a`. Both were verified against a file with a known match.

**The nine source files this used to name are all `us-ascii` today**, and a
scan of `src/` and `include/` finds no non-ASCII `.c`, `.cpp` or `.h` at all, so
the mechanism no longer bites there. Four Windows build files are still 8-bit:
`Sigel.dsw`, `Sigel.mak`, `sigel_slave.mak`, `manage_dyn_slave.mak`. **The rule
stands whatever the current file list is** — a file only has to stop being ASCII
once. The nine were `SIG_MainWindow.cpp`,
`SIG_SimulationVisualisationWidget.cpp`, `MT_PopulationWidget.cpp`,
`MT_StatisticsWidget.cpp` and five `MT_GUI` headers, which is why **any NEGATIVE
search in this file's history that was produced with plain `grep` before
2026-09-09 is worth redoing.**

**`check.sh` is NOT affected, and that is not luck** — every load-bearing search
in it already uses `command grep`. Checked explicitly: the dead-signal counters
find an injected `rightButtonClicked` in a Latin-1 file (2 of 2 matches). The
five plain `grep` uses left in it read generated `ui_*.h`, `.qrc` and `.ui`
files, all ASCII; the one that reads a module source (`check.sh:869`,
`sortByColumn`) would report a loud `form FAIL` rather than a silent pass, and
no form base class is on the list anyway.

**What this cost.** Nothing that shipped: the C10 fixes live in
`SIG_AllIndividualsView.cpp` and `MT_Controller.cpp`, neither of which is
Latin-1, and the sweep for other `->clear()` sites was re-run with `command
grep` afterwards — it finds the same two, plus `listviewHosts->clear()` in
`SIG_GPParameter.cpp`, which is safe because that view connects only
`itemDoubleClicked` and `clear()` never emits it. But any NEGATIVE search result
in this file's history that was produced with plain `grep` is worth redoing
before it is relied on.

### The characteristic failure of this project: measuring the wrong thing

**Not wrong numbers — wrong *referents*.** Every one of these produced a real,
correct measurement that licensed nothing about the question being asked. They
are more dangerous than arithmetic errors because the result looks clean.

| what was measured | what was concluded | why it was empty |
|---|---|---|
| `RANDOMSEED` is 0 in all shipped `.exp` | "every shipped experiment is clock-seeded" | there are **two** keys; the one measured feeds a GUI spinbox and seeds nothing. The live one is 1 in 8 of 14 |
| a V5 probe targeted `SIG_DynaDrive::applyForce` | it would have checked the register-to-force path | that function is **DynaMo-only** and every shipped experiment selects DynaMechs. A clean "no divergence" would have meant nothing |
| an `-evolve` run would show whether `run()` is re-entered | it would settle the `isEmpty()` window | `-evolve` reaches `main`'s single straight-line call and **cannot reach the GUI slot**, which is the only re-entrant caller |
| a 7-instruction window after each accessor call | a sweep for null-tested accessors | misses any site that stores the result and tests it later — a **candidate list**, not an absence proof |
| a sweep keyed on **container type name** | iterate-and-mutate candidates | missed `evolutionLoop`, the one case measured as real, because the iterator and the container are different type strings |
| `head -4` on a symbol grep; a 772-byte window on a 2072-byte function | op counts stated as totals | **an undercount stated as a count**, four times on one side and once on mine — including in the message correcting it |

**The shape is always the same:** a bounded observation reported as a general
one, or a proxy measured in place of the thing it stands for.

**A sharper form of it, learned 2026-08-30 and worth more than the individual
findings it came from: an audit whose reference implementation shares the
property under test cannot see divergence in that property.** Two independent
audits of `QTextStream`'s double formatting both returned clean and both were
blind in the same place — the x86 side validated 1.3 against Python's `%g`,
which shares glibc's round-half-to-even, so a tie-rule difference was
unreachable by construction; this side sampled 200,000 random bit patterns,
which essentially never land on an exact tie. Neither method was wrong. **A
differential test is only as good as the difference between the two
implementations you chose**, and a random sample is only as good as the density
of the failure class within it.

**And its companion, from the same day: validate a parser on TWO independent
quantities.** A joint-limit sweep that reproduced the known record count of 61
was trusted, and undercounted the distinct limit pairs 4 against 7; the error
surfaced only when a second, independently derived figure disagreed. A single
agreeing number is consistent with a parser wrong in a way that number cannot
see. *One of the two regexes involved failed because it encoded an unstated
belief about the data — that these records end in zero — and then silently
enforced it. It did not error and it did not warn; it returned a smaller,
self-consistent, entirely plausible answer.*

**Which findings rest on what.** This file mixes two kinds of evidence and the
difference matters when deciding how much weight to put on one:

| evidence | examples | strength |
|---|---|---|
| **a running check** | the `initial` rest angle, the `Q2Dict` ordering, the 42 fitness evaluations, the eight duplicate-key lookups | strongest available here — byte-identical across five phases |
| **the 1.3 binary, statically** | the randomiser's constants, `setAutoDelete` per constructor, destruction order, the truncated-π factor | strong, and the only route to code no check reaches |
| **inspection and review alone** | the evolution-loop containers, the `±DBL_MAX` sentinel, the `-0` question, the tie class | weakest — and these are latent *precisely because* no check reaches them, which is why they needed the binary rather than a test suite |

**A second shape, found late: facts filed separately that are only wrong
together.** The `±DBL_MAX` sentinel was correctly recorded. The `%g` precision-6
writer was correctly recorded. The multiply-before-divide 1.3 regression was
correctly recorded *and deliberately kept*. Each is right on its own; the
undefined behaviour lives in all three composed, and nobody had crossed them.
**Cross serialisation facts with parsing facts, and cross "known and accepted"
facts with each other** — not just fresh findings. Both halves of this one were
already in the file.

*A third, smaller: test with a representative value, not an extreme.* Probing
this with `q = DBL_MAX` collapses both the 1.0 and 1.3 orders to `NaN` and hides
the difference between them; a realistic joint angle shows 1.0 giving a finite
constant and 1.3 giving `NaN`.

**Two rules, both learned the expensive way.**

1. **Before asking for a measurement, ask which paths it can reach.** A run of
   `-evolve` samples one caller. A grep over `src/` samples one scope. State the
   reachable set *first*, then decide whether the answer would settle anything.
2. **Establish the boundary before counting.** Function extent from the next
   symbol, not from an eyeballed byte range; corpus extent stated, not implied.
   Every truncation in this project produced an undercount presented as a total.

**And the discipline that catches it:** a sweep that cannot rediscover a known
positive is not evidence. The iterate-and-mutate sweep was rebuilt until it
found `evolutionLoop`; had it been trusted first time it would have returned a
clean six-item list with the only real case silently absent.

### The real remaining risk: Qt 2 defined it, Qt 6 leaves it undefined

**This is one hazard class, not a list of separate traps, and it is what is left
of this port.** The arithmetic is settled — the 1.3 binary confirmed the
randomiser's constants and the MDH results bit for bit. What a disassembly
**cannot** see is this class, because both versions compile to a call and the
difference lives in the callee. The twelve `QTime()` sites, the four accessors
below and the `insert` hazard in D24 are all one thing.

| Qt 2 | Qt 6 | found |
|---|---|---|
| `Q2PtrList::first()` on empty → nullptr | `QList::first()` → **UB** | §9 |
| `Q2Queue::dequeue()` on empty → 0 | `QList::takeFirst()` → **assert/segfault** | D23 review |
| `Q2Queue::head()` on empty → 0 | `QList::first()` → **assert/segfault** | D23 review |
| `Q2PtrList::at(i)` out of range → nullptr | `QList::at(i)` → **UB** | D24 |
| `Q2PtrList::insert(i>size)` → returns false, no-op | `QList::insert` → **assert/UB** | D24 review |
| `Q2PtrList::take(i)` out of range → nullptr | `QList::takeAt(i)` → **abort/heap read** | D24 review |
| `QTime()` → 00:00:00, valid | `QTime()` → **null**, sorts before all | Phase A |

**Three of the four accessors were found by review, after the code was already
converted.** That is the argument for checking this class deliberately at each
step rather than noticing it.

**A sweep of the 1.3 binary for the pattern — every pointer-container accessor
whose result is immediately null-tested — returned 12 sites.** Run against this
tree, converted-and-compiled code is **clean**: every `at()` in core is
loop-bounded, and `SIG_GPFitnessTrainer`'s `toSpawnList` cursor guards with
`if (cur < 0 || ++cur >= size())` before each `at(cur)`. Three sites were not
tracked anywhere:

| site | state | why it matters |
|---|---|---|
| ~~`SIG_GPManager.cpp, evalNewIndis` and `:1507`~~ **CLOSED by D25b** | `fitTaskList` is now `QList< QList<int> * >` (`:405`, `:1461`) and the walk is index-based | The hazard was real: `fitTaskList.first()` then `while (actFitTask)` — **the loop terminated on the null**, and Qt 6's `first()` is UB on empty. It became a cursor index (`isEmpty() ? -1 : 0`, then `at()`), not `value(0)` or a range-for, because the walk also needs `remove`/`current`/`next` semantics. This row is why the step was written the way it was |
| `SIG_ExperimentView.cpp:91`, `:105`, `:119` | `experimentHistory` is **already** `QList<T *>`; `.first()` unguarded | converted-code UB, latent only because `SIGEL_MasterGUI` is not in the build. Phase C |
| ~~`SIG_EnvironmentRenderer` `robotPathPoints`~~ **CLOSED by C5** | was **unconverted Qt 2** `QList<DL_vector>` under `#include <qlist.h>`, walked with `.first()`/`.next()` into a `DL_vector *` | the central pointer-versus-value trap. Now `QList<DL_vector *>` with index walks and an explicit `qDeleteAll`; both walks were guarded by `count() >= 2`, which is what kept `first()` off an empty list |

**`SIG_Robot::prepareDynaMechs` and the joint walks are safe here by accident.**
The 1.3 sweep flags them, and they were `first()`/`next()` cursor walks; the
conversion to range-for removed the `first()` calls as a **side effect**, not by
design. An empty list now skips the loop. Worth knowing, because a future edit
could undo that without noticing.

*The sweep is a candidate list, not a proof of absence — its window is seven
instructions, so it misses any site that stores the result and tests it later.*

#### The sensor conversion constant is deliberately wrong, and we match it exactly

**`SIG_DynaSensor::senseJoint1` is the wrong function to look at** — the same
referent error as the `applyForce` probe, and it recurred one message after the table
above was written. In `sigel_slave`, `senseJoint1` (`0x080c2d60`) has exactly one
caller: `SIG_DynaMoSimulationQueries::sense`. **DynaMo**, which no shipped
experiment selects. It is not in our tree at all — deleted with the backend on
2026-08-28 — and §7 already marks that probe INVALID for this reason.

**The live path is `SIG_DynaMechsSimulationQueries::sense`** (`0x080b1b8c`,
extent to `getActualSimulationTime` at `0x080b2580`). It calls
`getMechsMinPos`/`getMechsMaxPos` — the joint range, which is the denominator
half nobody had located — and references five doubles:

    0x81ec308 = 2.0
    0x81ec320 = 57.295779578552292      <- the radians-to-degrees factor
    0x81ec328 = 360.0
    0x81ec330 = 90.0
    0x81ec338 = -90.0

**That factor is not 180/π.** True 180/π is `57.295779513082323`; 1.3's is
`57.295779578552292`, a **1.14e-09 relative error**. It is `180 / 3.14159265` —
π truncated to eight decimals.

**Our source reproduces it bit-for-bit, by accident of writing the same
expression.** `SIG_DynaMechsSimulationQueries.cpp:137,179,182` all write
`360.0 / (2.0*3.14159265)`, which constant-folds to `0x404ca5dc1af05a77` —
**the identical bit pattern** to the constant baked into the 1.3 binary.
Verified, not assumed.

**DO NOT "FIX" THIS.** Replacing `3.14159265` with `M_PI` or `4*atan(1)` shifts
every joint-sensor reading by ~1.1e-9 relative. That is invisible in a
6-significant-digit file and irrelevant to a static comparison — **and it feeds
the simulation**, where it is exactly the kind of perturbation that amplifies.
Evolved programs in the shipped `.exp` were selected against sensor values
carrying this error.

The surrounding arithmetic must be preserved with it: `fmod(angle, 360.0)`, the
clamp to ±90 (`:190-196`), and `(angle + 90) / 180` (`:197`). *The DynaMo
function's `360 - angle` reflex wrap and its `pi = 4*atan(1)` idiom belong to
the dead backend and are not ours to reproduce.*

#### The `±DBL_MAX` sentinel cannot survive a save, in 1.3 or here

The live `sense` references **eight** constants: `2.0`, `-DBL_MAX`, `+DBL_MAX`,
the truncated-π factor, `360.0`, `90.0`, `-90.0`, `180.0`. *The first list gave
five — truncated by a `head` and reported as complete.*

**The pair is an unbounded-joint sentinel, not a min/max search seed**
(`SIG_DynaMechsSimulationQueries.cpp:122`):

```cpp
if ( (minPos == (- DBL_MAX)) && (maxPos == DBL_MAX) )
  posRange = 1;
else
  { posRange = maxPos - minPos;  /* and q is clamped to [minPos, maxPos] */ }
```

So the usual hazard of replacing a seed with `numeric_limits<double>::min()` —
the smallest *positive* normal — does not arise. What does arise is worse:

**The sentinel is compared with `==`, and it cannot round-trip through the file
format.** `minPos`/`maxPos` come from `SIG_Joint::getMechsMinPos/MaxPos`, whose
values are read from the model file (`SIG_Joint.cpp, SIG_Joint`). Measured:

    DBL_MAX written by our stream : 1.79769e+308
    read back                     : 1.7976900000000001e+308
    DBL_MAX                       : 1.7976931348623157e+308
    round-trips to == DBL_MAX ?   : NO

At `%g` precision 6 the sentinel loses 11 significant digits, so **a joint saved
as unbounded and reloaded is no longer recognised as unbounded**. What follows is
worse than a wrong interval, and worse again than first recorded here:

| step | value |
|---|---|
| `posRange = maxPos - minPos` | **`+inf`** — the true difference `3.595e+308` exceeds `DBL_MAX` |
| `scaledState = (q - minPos)` then `*= 57.2957…` | **`+inf`** — the multiply comes first (`:136-137`) and `1.8e308 × 57.3` overflows |
| `scaledState /= posRange` | **`NaN`** — `inf / inf` |
| `static_cast<int>( scaledState * … )` (`:151`) | **undefined behaviour** |

**And the 1.3 regression already noted at `:132-134` is what makes it
undefined.** 1.0 computed `(q - minPos) / posRange` — divide first — which gives
a finite **0**: an information-free constant sensor, but defined. 1.3's
multiply-before-divide turns that same input into `NaN`. Measured both orders:

    1.3  (q-minPos)*K then /posRange : nan
    1.0  (q-minPos)/posRange         : 0

So the regression this file already records as "kept because the reference is
1.3" also converts a quiet failure into an undefined one, on an input no shipped
file produces.

**And this defect reproduces on every platform**, unlike the excess-precision
differences above. The overflow is forced by the **type, not the FPU**: in the
1.3 binary the multiply's result is stored with `fstpl` at `0x80b1f28` — a
64-bit store, because `scaledState` is a `double` — and `1.79769e+308 × 57.3 ≈
1.03e+310` is comfortably representable in an x87 register but not in a
`double`. Verified here: the same product is finite in `long double` and `inf`
in `double`. So x87-versus-SSE genuinely does not matter for this one.

**This is 1.3's defect, reproduced exactly, not one the port introduces.** 1.3
writes with the same `%g` at precision 6 (§ the fourth-family finding) and makes
the same `==` comparison against the same two constants at `0x81ec310` and
`0x81ec318`. **Latent in both**, confirmed from two sides: `grep` finds `1.79769e+308` in
**no** shipped `.exp` or `.rrb`. **Re-measured 2026-08-30 over all 14 `.exp`:
73 `RotationalJoint` records, every limit finite, no sentinel.**

The seven distinct limit pairs across those 73 records:

    -45/45  26    0/35  12    45/80  11    -90/-50  10    -90/90  7    -85/85  6    0/90  1

The 7 `.rrb` models agree independently: 53 rotational joints, the same seven
pairs, spelled `minimal`/`maximal`. *An earlier reading gave 61 records and four
pairs — 61 is the count for the 12 experiments in `experiments.tar.gz` where this
file has 14, and three pairs were missing.*

**A joint record is TWO lines and the limits are on the second.** The first
carries `mechsMinPos`/`mechsMaxPos`, the DynaMechs-frame values, which are `0 0`
in every shipped file and are recomputed by `transformToDynaMechs` before use.
Parsing the first line yields `(0,0)` for all 73 and looks like a finding; it is
not. *The parser that produced the seven pairs was validated by reproducing the
known record count of 61 first — and the count agreeing while the pairs did not
is what exposed the error. Validate a parser on two independent quantities: a
single agreeing number is consistent with a parser wrong in a way that number
cannot see.*

**The third field on that second line is `initial`** — the joint's rest angle,
confirmed from `SIG_RotationalJoint::writeToFileTransfer` (`:180`), its reader
(`:42`) and the `.rrb` keyword `init`. It is **live**: `getGeomRelation`
(`:133-150`) feeds it to `calculateAnyJoint` as the rotation between the two
links, so it sets the pose DynaMechs starts from. 24 of 73 records carry a
non-zero value (±45, −30) across the four multi-limb robots. **Unlike everything
else in this section it is check-covered** — fitness is distance-over-time from
that pose, and `fitness-check.sh` has been byte-identical across 3 individuals ×
14 experiments through Phases A, D, P and C1–C5.

No distributed model has an unbounded joint to lose.

**Do not "fix" it.** The three tempting repairs all change behaviour against the
reference: widening the write precision alters every number in every file;
replacing `==` with a threshold test changes which joints count as unbounded;
and swapping the sentinel changes the file format. `numeric_limits<double>::lowest()`
and `-numeric_limits<double>::max()` are both exactly `-DBL_MAX` and would be
safe substitutions *if* one were ever wanted — `-INFINITY` and `min()` are not.

#### The one gap that is structural: 80-bit intermediates in `moveDrive`

**`SIG_DynaMechsCommandInterface::moveDrive` holds its intermediates in
`long double`, and what that means depends on the machine.** This is the first
place where the architecture difference is unavoidable rather than incidental,
and it sits upstream of every simulation step, because it turns the
interpreter's register output into the force the physics integrates.

*It also corrects the V5 probe list, which named the wrong function.*
`SIG_DynaDrive::applyForce` has exactly one caller —
`SIG_DynaMoCommandInterface::moveDrive` — and **every shipped experiment sets
`SIMULATIONLIBRARY 1`**, which is DynaMechs, so the DynaMo interface is never
constructed. A breakpoint on `applyForce` was watching a function that is dead
in the selected backend. The live `SIG_DynaMechsCommandInterface::moveDrive`
calls no `applyForce` at all; it computes the drive value inline.

**Verified locally against `xb/kdesigel/sigel_slave`** — a second unstripped 1.3
binary, alongside `xb/kdesigel/sigel`, while both were on disk; they went on
2026-09-20 and come back from sourceforge's `kbin.tar.gz`. `moveDrive` is at
`0x080b787c`:

| claim | measured |
|---|---|
| extended-precision ops | **14 `fstpt` + 22 `fldt` = 36**, over the function's true extent `0x080b787c`–`0x080b8094` (2072 bytes). *An earlier count said seven pairs, and its correction said nine ops — also wrong, and wrong the same way: it stopped at an arbitrary `0x080b7b80`, covering 772 of 2072 bytes. **The correction repeated the truncation error it corrected.** The boundary is the next symbol, `__static_initialization_and_destruction_0`.* |
| the exponent | **`getSize() - 1`**, from `lea -0x1(%eax),%edx` at `0x80b78d8`. *Reported as `pow(2.0, size)`, which is one instruction short.* Our source has `getSize() - 1` at `SIG_DynaMechsCommandInterface.cpp:66,69,103,107` — **it matches the binary** |
| `pow` itself | called with **doubles** (`fstpl`, 64-bit, at `0x80b78f6`/`0x80b7902`), not extended. Only the surrounding intermediates are 80-bit |

**Our source really does use `long double`**, at
`SIG_DynaMechsCommandInterface.cpp:49,65-69,102-111`. *This called it "the only
`long double` in the whole tree", and that was wrong when it was written:*
`SIG_DynaMechsSimulationQueries.cpp` has **17 more**, and the pristine tree has
the same split — so the 80-bit-intermediate exposure argued below is NOT
confined to `moveDrive`; the sensor path has it too.

**And on this machine it is *more* precise, not less:**

| platform | `long double` | mantissa |
|---|---|---|
| 1.3 on i386 | x87 extended | **64 bits** |
| this build, aarch64 | IEEE binary128 | **113 bits** |
| a build on x86-64 Linux | x87 extended | **64 bits** |

So the port does not lose precision here; it gains it, and therefore cannot be
bit-identical. An x86-64 build gets x87 `long double` back — **but that closes
only half the gap, and the other half cannot be closed by choosing a host.**

**1.3 uses x87 exclusively.** Measured over `sigel_slave`: **0** SSE scalar
float ops, **53,652** x87 ops. That is what 2003 gcc targeting i386 does, and it
has a consequence far beyond this one function: **on i386 every `double`
expression is evaluated on the x87 stack at a 64-bit mantissa and rounded only
when stored to memory.** The excess precision is not a property of variables
declared `long double`; it is a property of all floating-point evaluation in the
entire binary.

On x86-64, `long double` is x87 extended but plain `double` arithmetic goes
through SSE at true 64-bit. So an x86-64 build would match 1.3 wherever
`long double` is explicit and **still differ everywhere 1.3 got 80-bit
intermediates for ordinary `double` expressions** — which, at 53,652 x87 ops and
zero SSE, is everywhere.

Closing that would need `-mfpmath=387` on an x86-64 build, or accepting that
plain-`double` paths differ in the last bits. **Not worth chasing.** It is
recorded so that nobody concludes from "build on x86-64" that bit-identity
becomes achievable and then treats the residual as a defect. Given the
6-significant-figure format finding, none of it should matter for a
file-mediated check; it matters only to someone demanding bit equality.

**How to compare it.** Treat `moveDrive` as a function of its inputs — feed a
known register value and width, capture the drive value on both sides — rather
than expecting bit equality. Given the 6-significant-digit finding above,
agreement to 6 figures is very likely and is the check that matters. **Measure it
before the trajectory comparison**, because a difference here propagates into
every step downstream.

**Item ordering changes:** this belongs with item 5 (mass properties) as
non-integrating arithmetic to pin *before* items 1 and 2 — not after items 3 and
4 as a sensor-adjacent afterthought.

#### The fourth family: numeric text on serialisation — REOPENED AT C5

**Structurally invisible to every comparison run so far.** V1, V6, V7 and V8 all
compared 1.3 against 1.3, so both sides came from the same writer. A difference
in how a `double` is *rendered* could not appear in any of them — and it would
make every saved `.exp` and `.pol` differ from 1.3 in the text of every real
number while being numerically identical. That reads as file corruption in a
diff, or passes a numeric-tolerance check and fails a byte check with no visible
cause.

**1.3's writer is C `%g` at default precision 6.** Characterised on the x86 box
across all 126,871 numeric literals in the 12 shipped experiments: **0**
violations of the `%g` notation rule, **0** literals whose text differs from
`printf("%g", value)`, maximum 6 significant digits (115,896 sit at exactly 6,
none above), scientific notation only for exponents −05 and −06, and not one
trailing zero after the point anywhere.

**Our writer matches, tested locally two ways.** Nothing in the tree sets
`setRealNumberPrecision` or `setRealNumberNotation` on a file stream — only
`SIG_GPPVMData.cpp:116,157` do, at 50, for the wire — so Qt 6's defaults
(precision 6, `SmartNotation`) are what ships.

| test | result |
|---|---|
| the seven boundary values, including the **exponent −4 case** that stays decimal under `%g` and would differ on 2,036 corpus values if a writer switched to scientific one exponent early | **7 of 7 identical** to 1.3 and to `%g` |
| every distinct decimal literal in `data/Experiments/*.exp` and `data/Robots/*.rrb` — parse to `double`, write back through `QTextStream`, compare text | **4,330 literals, 0 differ** |

**THIS FAMILY IS NOT CLOSED, AND THE AUDIT'S METHOD IS WHY IT LOOKED CLOSED.**
Qt 2 wrote through `sprintf("%.*lg")` (`qtextstream.cpp:1776-1805`, Qt 2.3's
source). glibc rounds an exact decimal tie **half to even**; Qt 6's
`QTextStream` rounds **half away from zero**. Measured at C5:

| sample | precision 6 | precision 5 |
|---|---|---|
| 200,000 random bit patterns | **0 differ** | 0 differ |
| 50,000 values on a 1/10000 decimal grid | **0 differ** | 0 differ |
| 50,000 multiples of **1/16** | **7,850 differ** | 4,285 differ |

`100.3125` is `100.313` in Qt 6 and `100.312` in Qt 2. **A random 64-bit double
essentially never lands on an exact tie at 5–6 significant digits**, so the
200,000-pattern audit — and the 4,330-literal corpus test — are both structurally
blind to the entire class, while exact binary fractions hit it constantly and
are exactly what geometry is made of.

**What is still established:** 0 differences over the shipped corpus, so nothing
we *read back* moves. **What is not:** any file this port *writes* whose values
are exact binary fractions AND are written at a precision that cuts them. The
POV-Ray export does that, at precision 5, and so does the PVM stream, at 50 —
see the correction below. The `.exp`, `.rrb` and `.pol`
writers use precision 6, where no shipped value is long enough to round.
`QTextStream` has no tie-breaking control, so matching 1.3 would mean routing
every real number through a C-style formatter. **§0 now measures what that would
buy: nothing observable.** Read that before re-opening this.

**EXTENDED 2026-09-07, and this section was right all along.** An attempt to
close the question in §0 was withdrawn for not reconciling with this table. Two
things are now known that sharpen it. **The writers do not all use precision 6**:
`SIG_Renderer.cpp, vectorToPovray` sets **5** for every POV `<x, y, z>` and
`SIG_GPPVMData.cpp:116, :157` set **50** for the master↔slave transfer — so this
table's precision-6 column is the *least* affected case. At 5, **22.49%** of
multiples of 1/16 differ against 0% at 6. `tiecheck.cpp`, in git history, is the
measurement.

**CORRECTED 2026-09-09.** This paragraph used to add that `0.703125` "is in a
shipped file — once, `twoBasesHighMutationRate.exp:112290` — differing at
precision 5, so the class reaches real data". **The value is in that file; the
difference is not.** Nothing writes an `.exp` at precision 5, and at precision 6
that value has exactly six significant digits and prints verbatim. Measured by
round-tripping the file: the experiment-history section comes back
byte-identical. The class reaches synthetic grids and the POV export, and no
shipped file. **It does reach the PVM stream, measured 2026-09-18.** At precision
50 the exact decimal expansions of these doubles cluster at 50 to 55 significant
digits, so ties are common: built from the shipped experiments, our octopus
payload prints 18 non-zero tokens differently from C's rounding and walker's 83,
and 15 of octopus's differ from 1.3's own payload text. Each is the same double at both
ends, so the slave rebuilds the same robot.
*Found by the C5 review. The audit was correct and its sampling method could not
see the failure — §9's "test with a representative value, not an extreme", in
the other direction.* *`-0` remains the one
known exception. It appears in no shipped file, but it is in the PVM stream; the
handover has the measurement.*

*Scope: this characterises decimal literals in the shipped `.exp` and `.rrb`.
Integers and any binary-format path are outside it.*

**THERE ARE THREE PRECISIONS ON THREE PATHS, NOT ONE.** Confirmed from
`xb/kdesigel/sigel_slave`: `QTextStream::precision(int)` has exactly three call
sites, and the 22 other hits are qhull's unrelated `qh_precision`.

| call site | precision | path |
|---|---|---|
| *(none — the default)* | **6** | `.exp`, `.rrb`, `.pol` |
| `SIG_Renderer::vectorToPovray` | **5** | POV-Ray export |
| `SIG_GPPVMData::savePVMDataTransfer` / `loadPVMDataTransfer` | **50** | the PVM wire |

*This section said "nothing in the tree sets `setRealNumberPrecision` … on a
file stream — only `SIG_GPPVMData` does". True as worded — `vectorToPovray`
writes to a `QString` stream — but it reads as "1.3 writes doubles at precision
6", and someone porting `vectorToPovray` against that would have written the
wrong thing. C5 was porting exactly that function.* Precision 5 and 6 differ on
most values: `0.842208` against `0.84221`.

#### The third family: iterate and mutate the same container

Found by converting `taskCanDoList` (D25a), then swept for systematically in
the 1.3 binary: group container calls by **element type**, and flag any function
that both iterates and mutates something with the same element signature.

*The sweep's first version keyed on the container **type name** and missed
`evolutionLoop` — the one case measured as real — because the iterator calls are
on `QValueListIterator<int>` and the mutations on `QValueList<int>`, different
strings that never join. Keying on element type finds it. **A sweep that cannot
find a known positive is not evidence**, and this one was rebuilt until it
could.*

Seven functions, all now resolved:

| function | verdict |
|---|---|
| `evolutionLoop()` and `evolutionLoop(MT_Classifier *)` | **the known positive** — 80 appends inside the live loop, measured. Converted in D25a |
| `SIG_Link::transformToDynaMechs` | **safe** — walks `this->joints` (member at `+0x38`) and appends to a *local* list constructed at `0x80976f9`. Two container instances, one element type: an artefact of element-type keying |
| `SIG_Material::setFrictionValue` | **safe** — the loop exits at `0x8094f64` and the `new`+`append` at `0x8094f7e`/`0x8094fa7` are past it. Never mutates while the iterator is live |
| `SIG_Robot::clear` | **safe here, and not by luck** — Qt 2 built six `QDictIterator`s and deleted through them. Our conversion (`SIG_Robot.cpp:82-94`) is six `qDeleteAll` followed by six `clear()`, which deletes *items* and never mutates a container mid-walk. The shape is gone, not merely harmless |
| `SIG_GPParameter::slotAddHost`, `::slotDeleteHost` | **CONVERTED in Phase C; kept for the design note.** `slotDeleteHost` is safe by design — the 2003 comment says *"we don't delete the host directly as the iterator would get confused"*, and it defers into a list. It used to be a knot of three separate hazards at once: an unconverted Qt 2 `QList<SIG_GPPVMHost>` used as a pointer list, a Qt 2 cursor `QListIterator` with `.current()`/`++`, and `hostList2.remove(host)`. All three are gone — it is a Qt 6 `QList<SIG_GPPVMHost *>` with `removeOne` at `SIG_GPParameter.cpp:524`, and `QListIterator` appears nowhere in the tree |

**A refinement that does not work, recorded so nobody rebuilds it.** Comparing
the *receiver operand* of the iterate calls against the mutate calls looks like
an automatic discriminator. It is not: it declares `evolutionLoop` safe, because
for iterator-style walks the iterator is always a different object from the
container by construction. It would only work for cursor-style containers where
`first`/`next` are called on the container itself. **Every candidate needs its
loop body read.**

### `SIG_GPExperiment` is defined twice, on purpose — do not "fix" it

**Two files define `SIGEL_GP::SIG_GPExperiment`**, with different bodies:
`src/SIGEL_GP/SIG_GPExperiment.cpp` and `SIG_GPExperimentClean.cpp`. Their two
headers share one include guard, `SIGEL_GP_SIG_GPEXPERIMENT_H`, so a
translation unit only ever sees one of them.

**This is deliberate 2003 design, one variant per binary:**

| target | variant | how |
|---|---|---|
| `sigel` | `SIG_GPExperiment.cpp` | out of `libSIGEL_GP.a` |
| `sigel_slave` | `SIG_GPExperimentClean.cpp` | named in `sigel_slave_SOURCES`, compiled into the target |

They never collide: the slave's own object satisfies the symbols, so the
archive member is never extracted. The only difference is `MT_Controller` —
the master's constructors do `mtController = new MT_Controller(*this)` and the
slave's do not. "Clean" means "without the GUI controller".

`sigel_eval` is the slave's role and links the slave's variant, which is right.

**Recorded because it was nearly broken.** Scoping the `experimentHistory`
conversion, this was read as a duplicate definition and an ODR violation, and
deleting one of the pair was put forward as an option. `SIG_GPExperiment.cpp`
is the one that would have gone — the file `sigel` needs, which Phase C has to
build. The build files are what settle it, not the source: our `Makefile`
globs `*.cpp`, so it compiles both into one archive where 2003 compiled them
into separate targets, and that glob is what made the pair look accidental.

**Confirmed against the 1.3 binaries, 2026-08-29**, by symbol table and
disassembly on the x86 box — static only, nothing run. Both failure hypotheses
are excluded:

| | `sigel` | `sigel_slave` |
|---|---|---|
| `MT_*` symbols | **168** | **0** |
| `SIG_GPExperiment()` calls to `MT_Controller` | 1 | 0 |
| `SIG_GPExperiment(QString)` calls | 1 | 0 |

The slave contains **no `MT_Controller`, `MT_GPSystem` or any `MT_*` symbol at
all** — not an unused copy, absent. So the slave did not link the library's
variant, and the master did not use `Clean`. The master's call site
disassembles to `__builtin_new(248)` followed by
`__13MT_ControllerRQ28SIGEL_GP16SIG_GPExperiment`, which is
`MT_Controller::MT_Controller(SIG_GPExperiment &)` — a reference, matching
`new MT_Controller(*this)` as written.

**`sigel_eval.cpp:22` includes the MASTER header while linking the Clean
implementation.** Measured: `sizeof` is 1176 against 1168, and
`experimentHistory` sits at offset 1120 under both — benign **only because
`mtController` is the last member**. Convert one header and not the other and
that member changes size, every later member shifts, and the build still links
in silence. That is the concrete reason the rule below is load-bearing rather
than tidiness.

**When converting anything in this class, change both files and both headers.**
Renaming them so the pair is self-evident is in `future_refactorings.md`.

---

## 10. Debt — after the port is trusted

### Drop the Qt 2 emulation — REQUIRED, not optional (D25)

**Promoted 2026-08-27 from debt to Phase D**, and ordered before Phase C so the
interface is ported once. D25 says the finished port contains no `q2compat.h`,
no Qt3Support and no compatibility flag on SIGEL's own code, so everything below
is in scope rather than "after, if wanted".

The one thing that has not changed: this cannot be verified against the 1.3
binary until the x86 box gives us fitness numbers (§7). The link-order half
*can* be self-checked, and **that check now exists — step D1, done 2026-08-27**:

```
./checks/dictorder-dump.sh | diff -u dictorder-baseline.txt -
```

`dictorder-baseline.txt` is **2,189 lines over 21 blocks** — 14 experiments and 7 `.rrb`.
Every later Phase D step has to leave that diff empty.

**What the check covers.** `SIG_Robot`'s **six** `Q2Dict`s (`SIG_Robot.h:60-65`)
— bodies, materials, links, joints, drives, sensors — all written in iteration
order by `writeToFileTransfer` and read back in that order by
`SIG_DynaMechsSimulationData`, plus `SIG_Link::points`, one per link.
**The order-carrying containers number ten, not seven or eight** — V1 measured
that against the 1.3 binary and found two this plan never enumerated.

**Verified to have teeth:** rebuilding the core with `h % vlen` perturbed to
`(h + 1u) % vlen` fires the check on **8 of 14** experiments and **6 of 7**
`.rrb`. `twoBases` does not move, and should not: 2 links, 1 joint.

**Three load paths, three different orders, all three recorded:**

| tag | what |
|---|---|
| `loaded` | the robot as the `.exp` deserialises it |
| `copy` | after `SIG_Robot`'s copy constructor |
| `rrb` | the standalone model, read in declaration order by `SIGEL_RobotIO` |

`loaded` and `copy` are not the same order for 4 of the 14. The copy constructor
round-trips through `writeToFileTransfer`/`readFromFileTransfer`, and
`Q2Dict::insert` prepends, so re-inserting in iteration order **reverses every
colliding chain**. `q2compat.h`'s header comment claims this is unreachable
because no `Q2Dict` is copy-constructed; the round-trip has the same effect, so
that reasoning is wrong. Once the shim goes, load and save become
order-preserving and all three collapse into one — a baseline that recorded only
`copy` could not have seen `loaded` move.

**The script refuses to run** unless it finds exactly 7 `.exp` and 7 `.rrb`,
and it inspects `sigel_eval`'s stderr instead of discarding it. Both were real
holes: the previous version exited 0 on a missing build directory, a missing
`data/`, and a wrong `SIGEL_ROOT`, emitting a short file each time — and
Phase D re-captures this baseline at every step, so a silent short run would
have overwritten it and reported success.

**The experiments and robots are tracked, in `experiments/` and `robots/`.** All
14 experiments were judged by eye on 2026-09-19 and kept one per robot, each
renamed to its robot's name: `hammer`, `insect`, `octopus`, `runner`,
`shortHammer`, `twoBases`, `walker` — items 40, 44, 45 and 46 in
`future_refactorings.md`. The `.exp` bytes are the download's; only the names
and the set changed. `robots/<robot>/` holds the migrated `.rrb` (the order D2
proves), the `.wrl` meshes and the `.blend` model sources the meshes were
exported from. Both folders were copied byte for byte from `data-reordered/`,
which with `data/` was then removed; `~/sigel-data-both-20260919.tar.gz` holds
both trees as they were before the clean-up.

### D2 — what the migration actually has to preserve

Names, so this stops being ambiguous: `dictorder-dump.sh` produces the order
from `experiments/` and `robots/`, and `dictorder-baseline.txt` is the committed
reference. `dictorder-reorder.py` built **`data-reordered/`** from the download
tree `data/`, and `robots/` was copied from its result. The script and both
trees were removed on 2026-09-19; git history keeps the script.

**Only four of the six dicts are numbered.** The walk is, in this order,
**links → joints → sensors → drives**, and those four orders are the ones a
migration must reproduce exactly. **The citation here was wrong and is corrected
2026-08-28:** it named `SIG_DynaMoSimulationData.cpp:33-55`
(`dynaSystem.newLink/newJoint/newSensor/newDrive`), which is the **Dynamo**
class, deleted with that backend. Both backends walked the same four dicts in
the same order, so nothing this section concludes changes and
`dictorder-baseline.txt` did not move — but the live file is
`SIG_DynaMechsSimulationData.cpp`. **Corrected again 2026-09-18, by review:**
that file does not walk the four containers in order. It takes their count,
indexes `drives`, `sensors` and its links by stored number, and numbers the
DynaMechs bodies by a depth-first walk over `SIG_Link::getJoints()`. Container
order reaches that walk only through the next stream written from it — a copy
or a PVM send. The handover section has the measurement.

Bodies and materials are free of *numbering* — but an earlier draft said they
were "touched only by `loadGeometries`" and "only through `lookupMaterial`",
**and that was wrong**: both are also iterated by
`SIG_Robot::writeToFileTransfer`, so their order reaches `.exp` bytes, the PVM
stream (`SIG_GPPVMData.cpp, loadPVMDataTransfer`) and the POV-Ray export
(`SIG_RobotRenderer.cpp, createPovrayDeclarations`). No index, number or DynaMechs registration comes
from either, so the conclusion holds and no fitness moves — but for a different
reason than the one given. One latent path: `SIG_Material.cpp, SIG_Material` records a
friction pair only if the partner is already loaded, so an **asymmetric**
friction graph would lose pairs as a function of write order. Every material in
all 14 `.exp` has `nfric == 0` and no `.rrb` declares `friction`, so it is
latent, not live.

That matters because the two are not simultaneously satisfiable. The bodies dict
is filled as links are processed, so once it becomes insertion-ordered its order
follows link order. `octopus` wants links `firstFootLink, base, firstLegLink1…`
and bodies `footLink.wrl, legLink.wrl, base.wrl`; writing the links in their
target order yields bodies `footLink, base, legLink`. Since body order reaches
nothing, the link order wins and the conflict is not real. Had this gone
unchecked the tool would have been built around an impossible constraint.

**Sensor and drive order is not cosmetic.** Sensor numbers are what an evolved
program's `SENSE` indexes and drive numbers index the actuators, which is the
"register value a given joint angle produces" determinism §7 relies on.

**Two target orders, both already in the baseline:**

| file | target |
|---|---|
| `.exp` | today's **`copy`** order — the simulation runs on `SIG_Robot robot(experiment.robot)`, not on `experiment.robot` |
| `.rrb` | today's **`rrb`** order — declaration order once the loader preserves it |

**Result — the 14 `.exp` need no migration at all.** `dictorder-reorder.py`
rewrote 7 of 7 `.rrb` and **0 of 14 `.exp`**: their stored order already *is*
the order the simulation uses.

Why, and it is not luck. `Q2Dict::insert` prepends, so reading a file in order
*F* builds every colliding chain backwards and iteration yields some order *L*.
Saving writes *L*; loading that reverses each chain again and returns *F*.

**Narrower than it looks.** `hash ∘ hash` is not self-inverse in general — it is a stable sort by bucket, and is the
identity only on an order that is already bucket-grouped. It holds here because
every shipped file order is itself an iteration order, written by the 2003
binary. It does **not** license hand-editing an `.exp` and expecting the
property to survive. `copy` — `hash(hash(file))` — equals the
file order exactly, verified for all 14 files, every kind and all 87 point
lists, 0 mismatches. Verified on `walker`, where `loaded` ≠ `copy`:
file order is `shoulder5, body, shoulder6, …`, `copy` is identical, `loaded`
is `body, shoulder5, foot1, …`. So once the containers are insertion-ordered,
loading an `.exp` gives the order the simulation already ran on, and **not one
byte of the shipped experiments changes**.

The 7 `.rrb` do need it: `SIG_RobotCompilerObjects.cpp, linkFind` inserts in
declaration order, which the same prepend then reverses, so the file has to be
written in today's `rrb` order for the flip to preserve it.

**Link numbers shift in the `.rrb`. An earlier draft here said that was safe
because "nothing indexes by that number". THAT WAS WRONG** — corrected by
review, and the correction contradicts nothing else in this file only because
the risk was already stated two paragraphs up.

`SIG_DynaSystem::getJoint(int)` really is a linear search rather than a
subscript, but the search *key is the number*, so that fact settles nothing.
Four containers are subscripted by it directly —
`SIG_DynaMechsSimulationData.cpp, SIG_DynaMechsSimulationData` `drives.insert(getNumber(), …)`, `:170`,
`:183`, `:197` for sensors, and `:486-489` `jointIndices[joint->getNumber()]` —
and an evolved program's operand resolves straight through them:
`SIG_DynaMechsSimulationQueries.cpp:93-95` does
`sensorIndex = absoluteSensorNo % sensors.size()`, and
`SIG_DynaMechsCommandInterface.cpp:74` the same for drives.

**Measured: 185 of 212 stored numbers in the reordered tree now name a different
entity.** `walker` drive `#0` was `leg1Joint1Drive` and is now `leg6Joint3Drive`;
sensor `#0` was `leg1Joint1Sensor`, now `leg6Joint2Sensor`. So `MOVE 0` on a
`.rrb`-loaded walker drives a different actuator than it did in 2003.

**Iteration order and numbering cannot both be preserved for a `.rrb`**, because
the number *is* the declaration position (`SIG_RobotCompilerObjects.cpp, linkFind`,
`linknumber++`) and the file has nowhere to record a number independently. The
migration chose **iteration order**, which is what §10 asks for. For a robot
built from a `.rrb`, declaration order also sets each link's joint list, and so
the DynaMechs body index. That choice was made silently and should not have
been; it is written down now, and it is **open to reversal** — leaving the 7
`.rrb` untouched would instead give declaration order = position = number =
iteration order, fully self-consistent, at the cost of changing the order the
1.3 binary would have used.

**Nothing shipped is affected either way.** The 14 `.exp` embed their own robot
with its own stored numbers, are never reconciled against a `.rrb`
(`SIG_GPExperiment.cpp:86-101`), and `SIG_Link.cpp, SIG_Link` reads
`tx >> name >> number` straight back — every number stays as 2001 wrote it.
`dictorder-dump.sh` now prints the stored number next to the position (`#N`) so
this is visible to the check instead of invisible to it.

### D3–D26 — the shim's users, one container at a time

**COMPRESSED 2026-09-03, from ~1,910 lines.** D27 deleted `q2compat.h`, so every
conversion below is enforced by a compiler that has no `Q2*` type left to accept.
What is kept is the semantics table the conversions were derived from, the
defects they found, the pre-existing hazards they wrote down, and the coverage
list — because *that* is the part that is still true and still load-bearing.
**"What the checks actually reach" below is verbatim.**

*Cut: the per-step narrative of which `deleteContents()` became which
`qDeleteAll`, per-step warning arithmetic and `check.sh` totals (§7 has the
current figures), and the corrections made to earlier drafts of paragraphs that
no longer exist.*

##### The semantics table every conversion was derived from

**The four Qt 2 containers do not mean what the `QList` method of the same name
means.** This is the reference; getting a row wrong is how D5, D9, D15, D19 and
D25c each shipped or nearly shipped a defect.

| Qt 2 | what it did | `QList` equivalent |
|---|---|---|
| `Q2Dict::find` | returned the **newest** binding for a duplicate key | a **backwards** scan. A forward scan returns the first — D4 got this wrong at six of eight lookups and no shipped robot has a duplicate name (0 across 438 name-groups), so nothing in the data could have caught it |
| `QVector::insert(i, p)` / `Q2PtrVector::insert` | **overwrote** slot `i`, deleting the previous occupant; did **not** grow or shift; returned false if `i >= size` | `delete v[i]; v[i] = p;` — **`QList::insert` shifts**, which is wrong. C5 shipped the shifting version into live code |
| `Q2PtrVector::size()` / `count()` / `isEmpty()` | **allocated slots** / **non-null** slots / no non-null slots | `size()` matches only because the conversion keeps one element per slot, nulls included. `count()` and `isEmpty()` need a hand-written non-null count — D25c's `toursAreEmpty()` |
| `Q2PtrVector::remove(i)` | deleted the occupant and left a **null hole**; the slot stays | `delete v[i]; v[i] = 0;` — `QList::removeAt` *shifts* |
| `Q2PtrVector::take(i)` | returned the occupant, emptied the slot, **never deleted** | `p = v.value(i); v[i] = 0;` |
| shrinking `resize(n)` | **deleted the truncated tail** | delete `[n, size)` then `resize(n)`. Written once as a helper (`resizeOwning`, `resizeOwningHosts`) so no shrink is a special case a later reader has to re-derive |
| `Q2PtrList::at(i)` | returned **nullptr** out of range, and 2003 code relies on it defensively (`MT_Statistics.cpp, writeToFileMT_Statistics`) | **`value(i)`**, not `at()`. `QList::at()` out of range is UB and compiles silently |
| `Q2PtrVector::at(i)` / `operator[]` | **Qt 2 warned and then READ OUT OF RANGE** — `QGVector::at` is `if ( index >= len ) warningIndexRange( index ); return vec[index];` (`qgvector.h:85-92`), no clamp. **The clamp was the SHIM's own deliberate divergence**, and `q2compat.h`'s header comment said so | `value(i)`, which yields null. *Three distinct behaviours, and conflating them has now been done three times: `Q2PtrList::at` **returned null**, `Q2Array::at` **clamped** (`QGArray::at` does `msg_index(index); index = 0;`), and `Q2PtrVector::at` **read out of bounds**. §9's null-out-of-range row is the FIRST of the three. So where a D-step below says "the clamp went", what went was the **shim's safety net**, not 1.3 behaviour — and `value()` is safer than Qt 2 rather than equal to it.* Corrected 2026-09-03 by review; the compression had fused the distinction away |
| `Q2PtrList::insert(uint, const T*)` | returned false and did nothing when `i > size` | `QList::insert` is `Q_ASSERT_X(i <= size)` — abort in debug, UB in release |
| `Q2PtrList::take(i)` | returned nullptr out of range | `takeAt(i)` is out-of-range `operator[]` — abort or a heap read |
| `Q2Array::at()` | **clamped** an out-of-range index | dropped. It fired in none of the 42 evaluations, and both defects §9 said it masked are already fixed |
| `Q2Queue::dequeue()` / `head()` / `current()` | returned **0** on an empty queue (`qglist.cpp:438-439`, via `cfirst()`) | **`QQueue::dequeue()` is `QList::takeFirst()`, which is `Q_ASSERT(!isEmpty())`** — abort at `-O1 -g`, segfault under `-DQT_NO_DEBUG`. *The divergence runs the other way round from the obvious guess: the shim matched the reference and `QQueue` is the divergence* |
| `Q2ValueList::Iterator` | a **doubly-linked** list, so an iterator stays valid when the list is modified elsewhere, and `end()` is a fixed sentinel that appends splice in front of | `QList` is contiguous and an append can reallocate. **The faithful conversion is an index walk** — D25a |
| `Q2PtrList`'s internal cursor | `first()`/`next()` are real state; `remove()` returns the *next*; a dead cursor stays dead | an explicit index plus a step-back after `takeAt`: stay on whatever slid in, step back if the removed one was last, die if the list emptied, and **do not advance on `next()` from dead** |
| `Q2CString::size()` | `QByteArray::size() + 1` for a non-null string — Qt 2's `QCString` counted the terminating NUL — but **0 for a null string** | the plain byte count. **This is why `SIG_GPPVMData`'s wire length is `+ 2`, not `+ 1`**, and a mechanical rename would have quietly shortened every message by a byte |
| `Q2CString::operator const char *` | **nullptr** for a null string | `constData()` returns a pointer to an empty string. `pvm_pkstr` does `strlen(cp)` unguarded, so the old code **segfaulted** on a null `QString` — a crash removed, not a value changed |
| `setAutoDelete(true)` | at 8 sites, `insert()` or a shrinking `resize()` **was the only free** and the word `delete` appears nowhere | every one of those frees is now written out. **`setAutoDelete` in core is 0** |
| `QGVector`'s copy constructor | cleared `del_item`, so a copy freed nothing and could not double-free | a `QList` copy shares the raw pointers and **both** destructors `qDeleteAll`. **Every class that gained an owning destructor is `= delete` on copy and assignment**: `SIG_Geometry` (D7), `SIG_GPPopulation` (D15), `SIG_GPManager` (D25c), and C5's three renderers. *`SIG_GUIGPManager` derives from `SIG_GPManager`, so Phase C is precisely the case that would have found the missing one* |

##### The steps

| step | container(s) | what it found or had to preserve |
|---|---|---|
| **D3** | `Q2Dict` made insertion-ordered | `loaded` order moved for 8 of 14 — **correct**: it was `hash(file)` and is now file order, which is what collapses the three load paths onto one. `copy` order 0 of 14 changed; fitness identical on all 42 |
| **D4** | `SIG_Robot`'s six dicts → `QList<T*>` | every insert keyed on the object's **own name**, so the dictionary carried nothing the list does not. **Correction to §7's leak baseline**: 41,374 B / 117 allocs holds only for small robots — `walker` leaks **35,802,566 B in 630,138 allocations**, 866×, pre-existing and byte-identical on the commit before. **The leak scales with links and simulated frames** |
| **D5** | `SIG_Link::points`, `allowedCommands` | both needed the key carried explicitly (`struct NamedPoint { QString name; DL_vector *value; }`) because the value has no name of its own. `allowedCommands` is another **order-carrying** dictionary — its order rides inside every `.exp` and every PVM transfer |
| **D6** | `Q2Array` → `QList`, 180 sites, 40 files | **two** compile errors, both the one real divergence: Qt 2 handed out a **writable `T&` from a const array**. `sort()` was numeric (D13) → `std::sort`. The clamp dropped; residual risk is a release build alone |
| **D7** | `Q2PtrVector` for `SIG_Geometry`, `SIG_Body` | **the bounds check first added here made things worse.** `SIG_Polygon` self-registers with the geometry in its constructor, so creating one and then skipping every out-of-range vertex left a **0-vertex face**, which the Qt 2 clamp could never produce. The check now runs *before* the polygon is created, and `compFaceNormal` refuses a face with fewer than three vertices. *A step whose point was removing a latent dereference had moved the failure mode the wrong way* |
| **D8** | the `SIG_Register` cluster, 12 files | **a leak with no free path anywhere**: `SIG_Interpreter` has no destructor, never set `setAutoDelete` and never called `deleteContents`, so every interpreter leaked its whole register file. `SIG_Register` is two ints, so `QList<SIG_Register>` **by value** and the ownership question disappears. Exactly 8 allocations lost on both robots, matching `memSize 8`. **The file still leaks as one 80-byte buffer** because `new SIG_Interpreter` is never deleted — §10's pre-existing leak |
| **D9** | the last `Q2PtrVector` in `SIGEL_Simulation` | **`~Q2PtrVector` freed nothing either** (`del` false), so `DynaMechsLinkGuard` was already the only free while unwinding — but `~QList` can never free, so the guard now has no fallback of any kind. **And `sigel_eval` has no handler on that path**: the `new` sits *before* the `try`, so the throw reaches `std::terminate` and this toolchain does not unwind — the guard's destructor never runs. A converted free path with **zero coverage**, kept because `sigel_slave` does wrap the call |
| **D10** | `SIG_Link`'s two lists | **a `do`/`while` that must run on empty.** `first()` is null on an empty list and the body has an explicit "without successor" branch — a range-for drops that pass silently. Teeth-tested: a plain `while` makes **every** robot abort. *`getNoCollides()` has no caller anywhere in the tree; it is ported rather than deleted, per D21* |
| **D11** | `friction`, `usedByLinks`, `successors` | **`SIG_Material::friction` and `SIG_Link::noCollide` are exercised by no check**, so `-selfcheck` covers them through public API — and **the only assertion that catches the append bug is the count**, because `getFrictionValue` still returns the right value (a forward scan finds the correct entry before the stray duplicate). `fitness-check.sh` runs the self-check a **second** time under `detect_leaks=1`, which is what can see a dropped `qDeleteAll` at all |
| **D12** | `Q2CString` off the executed path | *a `grep` for shim **types** says nothing about shim **includes*** — six live `#include "compat/q2compat.h"` remained after the type sweep came back clean, every one reaching `QList` through the shim |
| **D13** | `hostList` | **a trap preserved rather than tidied**: an `if` with no braces whose next line is the cursor advance, so the loop advanced correctly and `-Wmisleading-indentation` was flagging 2003's layout. Verified against the pristine tarball bytes, not inferred from the indentation |
| **D14** | `experimentHistory`, in **both** variants of the class | the master's destructor is **not** byte-identical to Clean's — tab-indented, CRLF, and it also deletes `mtController`. See §9's "`SIG_GPExperiment` is defined twice". The file is one of the 46 mixed-encoding files §2 warns about |
| **D15** | `SIG_GPPopulation::pool` — five of §9's eight hidden frees | **`deleteIndividual` performed exactly one delete and nothing in the function said so**: it shifted with `insert(x, take(x+1))`, and `take` nulled each source slot, so only the *first* insert freed anything. **D15 then missed one of the five** — `readFromFile`'s `wasCanceled()` shrink, dead today only because `sigel_eval` has no `QApplication` and live the moment `slotPopulationImport` runs. Measured: **4,850 bytes in 45 allocations**. *`SIG_GPPopulation::sort()` has an EMPTY BODY despite a header comment saying it sorts the pool by fitness — a reader could implement it and silently renumber every individual and every stored `poolPos`* |
| **D16** | a self-check for the evolution loop's containers | **built because D15 proved the checks cannot see this code**: D15 shipped a real leak that `check.sh`, both diffs, the sanitized run, the self-check *and* the leak baseline all reported clean. Found **four pre-existing `SIG_GPPopulation` constructor defects**; two are fixed (missing initialiser lists) and **two are not**: `(int, SIG_Randomizer&)` and its four-argument sibling store `&r` and the destructor **deletes it** — a bad free on a borrowed randomizer. Repairing them means choosing an ownership policy, which is a design decision for whoever calls them. **Only the default constructor is used today** — exhaustive grep over the 1.3 tree and the 1.0 distribution |
| **D17** | the trainer's two host lists | **unexercised, not dead**, and the difference matters: `addDynHost` is reached only through a `pthread_create` **thread entry point** behind `-devolve`, and `flushAllDynHosts`' two call sites are both behind `if (serverIsUp)`, which only that thread sets — **so it is never entered under `-evolve`**. *And the modulus concern cannot be settled from the reference machine*: all four reference evolutions used exactly **one** `PVMHOST` (the `8` is slave slots on one host), so `nextHostNumber % 1` is 0 on every one of ~56,000 spawns. **Host rotation has never been exercised by anything** |
| **D18** | `toSpawnList` — the cursor walk | **no check covers it.** `sigel_eval -selfcheck` tested a copy of the walk, not `sweepToSpawn`, because the trainer is not linked into `sigel_eval`. That test was removed 2026-09-21. Dropping the step-back after `takeAt` in `sweepToSpawn` **in production** passed `check.sh`, `-selfcheck` and `pvm-check.sh`; the same change in the copy failed it. The evidence for the shipped rewrite is the review's differential run — **28,672 walks, 0 divergences**, mutation-tested at 8,256 and 12,544 divergences |
| **D19** | `pvmTasks`, `pvmHosts` | **D19 shipped a leak**: it removed both `setAutoDelete` flags and gave only `pvmTasks` a `qDeleteAll`, so every enabled `PVMHOST` leaked one `SIG_GPActivePVMHost` per trainer destruction, and **all 56 shipped `.exp` have at least one**. Found by a **19,500-scenario differential sweep**: 41 diverged, the smallest being one static host and no spawns; with the missing `delete`, all 19,500 agree. **It was the only behavioural difference in the entire conversion.** *Pre-existing and written down: `nextFreeNumber` is never reset — two writes in the 1.3 binary, `movl $0x0` in the constructor and `incl` in `spawnTask` — so `pvmTasks` grows without bound (~226 KB at 56,333 spawns, arithmetic not measurement), and `stopTrainersSlaves` uses it as its loop bound, making shutdown **O(total spawns ever)*** |
| **D20** | `SIG_GPFullDataRecorder`'s four lists, 10 files | **a null dereference fixed rather than reproduced**, per D13: `SIG_GPForceFitnessFunction`'s cleanup was a `do`/`while` that dereferenced `first()` before testing, so an evaluation recording no frames took a null dereference **while freeing**. *Same shape as D10's `do`/`while`, opposite conclusion — there the once-through was load-bearing and had to be preserved, here it is a crash. **The difference is which side of the null the body is written for, and it has to be read each time rather than pattern-matched.*** Coverage: `SIG_GPNiceWalkingFitnessFunction`'s walk runs on every check, but **setting its index to 1 — the classic error for this conversion — leaves the fitness check byte-identical across all 42 individuals.** The check discriminates one bit per individual, not which frame or how many |
| **D21** | `Q2CString`, 7 sites, all PVM | the **`+ 2`** above. **NOTHING VERIFIES IT** and D21 claimed `pvm-check.sh` did: with `+ 1` it passes, and with **`+ 0`** — one byte shorter than `pvm_upkstr` writes — it also passes with no sanitizer report, because `QList` over-allocates. The check is blind to a shortfall under about eight bytes |
| **D22** | `crossOver`'s return by value | neither end ever owned anything, so the conversion moves **no ownership at all**. What is new here is *returned by value*, not "owned nothing" — D9 and D11 already did the latter |
| **D23** | `Q2Queue` → `QQueue` | the `dequeue`/`head` reversal in the table above. It cannot fire through the only drain today — `updateTSet` dequeues exactly the whole queue, never one more — **confirmed by a differential probe** over `TSize` ∈ {0,1,2,3,5,8,100} × `N` ∈ 0..2·`TSize`+3, 0 mismatches. **Anything left in the queue at destruction leaks**: pre-existing, unchanged, now written down |
| **D24** | `TmpBuffer`, `StatisticsOfGeneration` | **the 1.3 binary confirms the loop including its dead code** — `8110fb6: decl -0x10(%ebp)`, a decrement whose `break` is nineteen lines later and which is never read again, on the same frame slot `count` wrote. *A dead store surviving in both is stronger evidence of a common source than any live path, because no behaviour forces it to agree* — and the 1.3 build is plainly unoptimised, so this is about the source text, not a compiler decision. **It does not license generalising to all of `MT_`.** *Pre-existing race written down: there is **no mutex on `TmpBuffer`**, unlike `TCaseBuffer`, so two concurrent `checkTask` calls can shrink it between the snapshot and the `takeAt` — where `take` returned nullptr and deterministically null-dereferenced, `takeAt` is out-of-range `operator[]`* |
| **D25a** | `taskCanDoList` — `Q2ValueList` | **a direct iterator conversion would have been a use-after-free**, and the trap was live rather than latent. Measured on the 1.3 binary under gdb, two generations, 100 individuals, 8 slaves: 33 `begin()`, **80 appends inside the live loop**, 94 removals. Both append addresses fall inside the loop body. *A negative result here would have been corpus-dependent; a positive is not — the path fired, so it can fire.* The index walk was checked against the node semantics it replaces over **200,000 randomised cases, 0 divergences**. **One pathology is preserved deliberately**: with `maxTouchsPerLoop == 0` the outer `while` spins forever — identical in both versions, and unreachable because all five presets set −1 |
| **D25b** | the two `fitTaskList` | `setAutoDelete(true)` was **the only free and it ran on four exits**, two of them early returns sitting after the list is populated. A plain container swap loses all four; an RAII guard restores them, following `DynaMechsLinkGuard` rather than inventing a second pattern. **155,641 exhaustive schedules, `diffs=0`, `oobAt=0`, `danglingPrev=0`, no leaks.** *`FitTaskListGuard` is copyable while holding a raw owning pointer — inherited from `DynaMechsLinkGuard`, worth fixing in both at once rather than diverging them* |
| **D25c** | `tours` — the sparse slot vector | **all six hidden-free sites are provably no-ops** and the deletes are written out anyway, preserving the *semantics* rather than the current behaviour: if a later edit breaks one of the proofs, the code still frees what Qt 2 would have freed. **Adding the owning destructor created a defect** — `SIG_GPManager` was copy-constructible, confirmed by `static_assert`; closed with `= delete`. *The precedent was recorded twice in this file and still not applied.* **`operator[]`'s clamp went and 13 pre-existing sites change silently** — latent, because `calcInitTourSet` rebuilds every index over the post-shrink vector |
| **D26** | `indis`, `individualItems` | the range guard first added here was **not** the guard `Q2PtrVector::insert` had: the shim takes `uint i`, so a negative index wraps huge and is **rejected**, where a bare `poolPosition < size()` is `int < qsizetype`, signed, so a negative **passes** and indexes out of range. Reachable through `POOLPOS` parsed from a saved `.exp` with `toLong()`. *The first version guarded the write and left the read* |

##### Blind spots and pre-existing defects Phase D wrote down — RESTORED 2026-09-03

Each of these is a live limit or an unfixed defect, not finished narrative. The
first compression cut them; a review reproduced the first one on the spot.

**`.rrb` LOADING HAS NO SANITIZED COVERAGE AT ALL.** The load path takes a
**heap-buffer-overflow inside vendored cv97** — `JString::regionMatches`
(`cv97/JString.cpp:150`) via `JString::endsWith` (`:189`) from
`SceneGraph::SceneGraph()` (`cv97/SceneGraph.cpp:31`) under
`SIG_Body::load()` (`src/SIGEL_Robot/SIG_Body.cpp, load`). It **aborts** under
AddressSanitizer, so `dictorder-dump.sh` cannot be run against `build/` at all
and the 7 `.rrb` — and everything the VRML/SceneGraph reader does — are
unreachable by ASan and UBSan. Vendored code, out of scope for the Qt port, but
**the blind spot is ours**: D9's write-up claimed "all 21 files load in
`dictorder-dump.sh`: 0 sanitizer reports", which is true only of the unsanitized
build. *Reproduced 2026-09-03 by review, unchanged.*

**`QT_NO_DEBUG` and `NDEBUG` are defined by neither the Makefile nor
`check.sh`**, so `QList::operator[]`'s `Q_ASSERT_X` is **live in both `build/`
and `build-asan/`** — confirmed to abort on a negative index. Several coverage
arguments above lean on this (an out-of-range index anywhere in a covered run
would have aborted rather than been absorbed), and the residual risk is a
release build alone.

**Two behaviour-changing fixes to 2003 code, both off the 42-evaluation path so
no check saw either** — `MT_FitnessTrainer`'s stale-`TSetSize` array sizing and
`MT_Substitute`'s high-water-mark loop bound. **They are recorded in §9's
"Defects fixed rather than preserved (D13)" table**, which is where this class
belongs; they are not repeated here. *A first version of this section added them
to that table and then restated both in a duplicate table right here, in the
present tense, two paragraphs after a correction whose own moral was that a
figure "stood for a day in two places at once". Found by review the same day.*

**The trainer's host-rotation modulus is cast, and it is D9's defect a third and
fourth time.** `SIG_GPFitnessTrainer.cpp, getNextHost` and `:602` are
`nextHostNumber % static_cast< uint >(pvmHosts.size())` over an `int`
`nextHostNumber` (header `:75`); the cast preserves Qt 2's unsigned wrap and is
what keeps `pvmHosts[ nextHostNumber ]` at `:598` non-negative. §9's
"register-to-index modulus" section names only the two `SIG_DynaMechs*` sites.
*Note the spelling — `static_cast< uint >` with spaces; a grep for
`static_cast<uint>` finds neither, which is how this went missing.* **And D17
records that host rotation has never been exercised by anything**, so both casts
are unrun.

**Three coverage statements that are easy to mistake for coverage.**

- **D16's self-check does not cover the defect D15 shipped.** Reverting all
  three `resizeOwning` calls **still passes every check, including the
  self-check** — the leak was behind `if (qApp)` and is unreachable headless.
  What the self-check does catch is `deleteIndividual`'s `delete`, and only under
  `detect_leaks=1`.
- **`sigel_eval`'s converted trace walk (D20) is exercised but unchecked.**
  `dictorder-dump.sh` filters stdout with `sed` and the only line that loop
  produces — `frames … height … last …` — is dropped by that filter, so patching
  the loop to skip its first element leaves the check empty.
- **D20's `listForces` is not owned, and only ONE of six fitness functions frees
  the force vectors**, while `record()` allocates a `vector<double*>` plus a
  `new double[6]` per link on every recorded frame regardless of which fitness
  function runs. Measured on one `-v` run of `hammer`: **11,891,420 bytes in
  252,265 allocations**, the majority from `dmArticulation::getForces`. That is
  the leak D4 records as scaling with links and frames.

**Two pre-existing leaks D24 wrote down.** `TmpBuffer`'s **never-matched
remainder** leaks — a *matched* case is freed, because it is enqueued on
`TCaseBuffer` and `MT_Trainingset::updateTSet` deletes it at
`MT_Trainingset.cpp, updateTSet` and `:145`. And `StatisticsOfGeneration` leaks
**entirely**: its elements are `new`'d at `MT_Statistics.cpp, MT_Statistics` and
`MT_GPManager.cpp, startEvolution`, `~MT_Statistics` is empty, and **no `delete` of an
`MT_StatisticsElement` exists anywhere in the tree**.

**D25b's forward-looking note.** `delete fitTaskList.takeAt( fitCur )` is
unguarded where Qt 2's `remove()` was a silent **no-op on an invalid cursor**, so
a future edit that mutates the list mid-loop turns a no-op into UB.

##### Three things that were nearly wrong about the whole exercise

**`sigel_eval.cpp` is in the repository root, and `src/` + `include/` is not "all
code".** D26 declared the shim deletable on a grep over those two directories.
`sigel_eval.cpp, selfcheck` and `:401` instantiate `Q2PtrList<int>` — the D18
differential check — and **that file is the dictorder and fitness check binary**.
Another instance of §9's characteristic failure and **the sharpest form of it:
the scope that was too narrow happened to exclude the file being certified.**

**The deterministic hash seeding the shim installed is vestigial, and the
experiment that first "proved" it could not have failed.** The worry was sharp —
V1 proved container iteration order is serialised into the `.exp`, so on that
path order is *output*, and Qt 6 randomises `QHash` iteration per process unless
seeded. But (a) there is **no `QHash`/`QSet`/`QMultiHash` anywhere outside
`compat/`**, because D3 replaced the six `Q2Dict`s with insertion-ordered
`QList`; and (b) the first check ran the binary under `QT_HASH_SEED` = 0, 1,
12345 and 999999 — **a null experiment: Qt 6 honours that variable only when it
is 0**, coercing anything else with `forced seed value is not 0; ignored`, and
the shim pinned the seed before `main` anyway. **The experiment that should have
been run** rebuilds `sigel_eval` with `QHashSeed::resetRandomGlobalSeed()` after
the deterministic call, so the seed is genuinely random per process: seeds
`278889441371735583`, `12972567734183481017`, `2771707303525248410`, **all three
dictorder runs byte-identical to `dictorder-baseline.txt`**. That is real
support, and unlike the argument from "no `QHash` in our code" it also covers
Qt's own internal hashes. *The call is now in `sigel.cpp`.*

**Self-consistency before fidelity — the order matters and it is cheap.** Run
this build **twice on the same input and compare it against itself** first; no
reference needed. Only then compare against the 1.3 digests. A mismatch at step 2
with a randomised seed underneath would look exactly like a fidelity failure and
send someone hunting through the interpreter and the physics for what is actually
one missing seed call.

##### Measurement rules this phase paid for

- **Leak byte totals are sensitive to the repo path length**, because a copy of
  `$SIGEL_ROOT` is among the leaked blocks. The figures reproduce exactly at a
  25-character repo root and shift at a longer path. **The allocation counts are
  stable** — quote those.
- **A warning can move because a *header* changed**, so a multiset diff scoped to
  the touched `.cpp` files cannot see it. D26's own numbers: 3 removed / 2 added
  over the six touched files raw, 1 / 0 with line numbers stripped, **4 / 2 over
  the whole tree**. *A verification has to state its scope and its normalisation,
  or it is not reproducible.*
- **Count sites, not files, and say which.** D23's "seven type sites across six
  files" and D24's 8 / 11 / 20 are three different questions with three different
  answers.
- **Re-measure at the commit you are describing, not the one you started from.**
  C6's LOC figure was wrong three times running for exactly this reason.
- **Sample the corpus, not one member of it.** The construction-count table below
  is given as ranges because the counts scale with the robot; an earlier single
  figure was `octopusSimpleFitness` alone, tabulated as though it were the run.
- **Do not quote a count from a step write-up.** §7 and the top of this file carry
  the current figures; every number in a step's prose is true of the commit that
  wrote it and of nothing later.

### What the checks actually reach — measured 2026-08-28, not assumed

This plan has described the remaining shim work as a **31 / 52 split**: 31 sites
the checks can execute and 52 in the evolution loop that nothing can run until
Phase C. **That split is wrong**, and D9 and D10 both leaned on it. Found by the
D10 review, then measured directly: a counter was put in each shim class's
constructor, the checks were run, and the counter was read back.

Constructions per `sigel_eval` run, **measured over all 14 experiments** and
given as the range, because the counts scale with the robot:

| type | one evaluation | with `-v`, which `dictorder-dump.sh` uses |
|---|---|---|
| `Q2PtrList` | **8 – 41** | **14 – 64** |
| `Q2ListIterator` | 1 – 4 | 1 – 4 |
| `Q2CString` | ~~3~~ **0** | ~~4~~ **0** |
| `Q2PtrVector` | **1** | **1** |
| `Q2Queue` | **0** | **0** |
| `Q2ValueList` | **0** | **0** |

*Given as ranges because they scale with the robot: an earlier single figure
was `octopusSimpleFitness` alone, tabulated as though it were the run. Sample
the corpus, not one member of it.*

`Q2PtrList` counts one construction per `SIG_DynaMechsLink` — its `successors`
member — plus one per material and body, so it tracks robot size.
`Q2ListIterator` counts `SIG_Material::friction` walks.

**`Q2CString`'s row was zeroed by D12 and is struck through above.** Its 3 and 4
were `SIG_Environment.cpp:416` — the constructor *and* `readFromFile` — and
`SIG_DynaMechsSimulationData.cpp, initializeEnvironment`, which were the only `Q2CString` the checks
ever constructed. The 7 that survive are PVM code `sigel_eval` never runs. D12
took the row to zero and left it standing while citing the table as current;
corrected by review.

**Four of the six types execute under the checks, not three.** The single
`Q2PtrVector` is `SIG_GPPopulation::pool`, which the "evolution loop" label had
written off. `sigel_eval` builds a whole `SIG_GPExperiment`, so
`SIG_GPParameter::hostList` (the shipped `.exp` carry 20, 21 or **8** `PVMHOST`
lines), `SIG_GPExperiment::experimentHistory` and, on the `-v` path,
`SIG_GPFullDataRecorder`'s four lists are all live too.

**`Q2Queue` and `Q2ValueList` are the genuinely blind ones** — 0 constructions
in either check. They are where the caveat at the top of this file actually
applies, and they are the two to leave for last and treat as the risk.

The probe was temporary and is not committed; the shim was restored from a copy
and both trees rebuilt before the checks were re-run.

**Converted code the checks do not exercise — keep this list growing.** Type
coverage above is necessary, not sufficient: a type can be constructed on every
run while a particular converted *branch* is never taken. Everything here was
converted in Phase D and is correct by inspection only. Two entries were found
by review after the step that introduced them claimed coverage it did not have,
which is why the list exists.

| what | why nothing reaches it |
|---|---|
| `DynaMechsLinkGuard`'s free, D9 | no shipped robot throws from the constructor, **and `sigel_eval` has no handler anywhere on that path** — see D9 |
| `sensors[…] = …` on the `tPitchRollSensor` and `tContactSensor` branches, D9 (`SIG_DynaMechsSimulationData.cpp:183,197`), and the two `dynaMechsLinks[…]` reads guarding them (`:182,:196`) | **0 `PitchRollSensor` and 0 `ContactSensor` in any shipped file** — 66 sensors, all `JointSensor`. Two of the four converted indexed writes in that constructor |
| `delete dynaMechsLinks[…]` on a **non-null** slot (`:365,:501`), D9 | needs two joints between one pair of links; no shipped robot has one. Only `delete nullptr` ever runs |
| the restored `uint` modulus, D9 | needs `bitsPerRegister` 32; all 14 `.exp` carry 3 or 8 |
| `SIG_Link::addNoCollide`, `getNoCollides()`, the `noCollide` write loop, D10 | **0 `nocollide` in all 7 `.rrb`, and `noCollideCount` is 0 in all 87 `Link` records of the 14 `.exp`** (348 over `data/` and `data-reordered/` together; an earlier draft said 261, which is neither scope). `getNoCollides()` has no caller in the tree at all |
| `SIG_Material::friction` — three walks and the owning free, D11 | **0 friction declarations in any `.rrb`, and `nfric` is 0 on all 31 `Material` lines**. The list is empty on every check run |
| `SIG_Body::usedByLinks`, D11 | appended on every `.rrb` load and **read nowhere in the tree** |
| both `SIG_GPFitnessTrainer` host walks, D13 | that object is **not linked into `sigel_eval` at all** |
| `SIG_GPParameter::writeToFile`'s `PVMHOST` loop, D13 | ~~linked, never called~~ **COVERED as of C11b** — `exportall` writes it and `guibehaviour-baseline.txt` pins the bytes |
| `readFromFile`'s `qDeleteAll` + `clear`, D13 | runs every load, always on an **empty** list |
| **all four sites in `SIG_GPExperiment.cpp`**, D14 | the master variant is compiled into `libSIGEL_GP.a` and **never linked** — `SIG_GPExperimentClean.o` satisfies the symbols first. `readelf --debug-dump=info` on `sigel_eval` has a CU for Clean and none for the master |
| `writeHistoryToFileTransfer`, D14 | called — `check.sh`'s `pagesave` and `v2 round trip vs 1.3` sections save six `.exp` per run, and `twoBases.exp` carries 203 history entries |
| `exportExperimentHistoryToGNUPlot`, D14 | linked; its only caller is `SIG_GUIGPExperiment.cpp, slotGNUPlotExport`, Phase C |
| **five of D15's eight `delete pool[…]`** | the three sized constructors, `importNewIndividual`, and `addRandomIndividuals`' — which is `delete nullptr` on every possible call, since `resize()` just made those slots. *D16 said six of nine; nine was a `grep` hit that counted a comment, and three of the eight are entered by the self-check as of D16* |
| both `wasCanceled()` shrinks, D15 | need a `QApplication`; `sigel_eval` has none, so `if (qApp)` is false |
| `readFromFile`'s shrink loop, D15 | the function runs on every load, but always on an **empty** pool, so the loop body never executes |
| `sort`, D15 | no caller anywhere |
| **the whole `TmpBuffer` loop, D24** | `MT_Evaluator` is **not in either check binary** — `nm -C build/sigel_eval \| grep -c 'MT_Evaluator::'` is **0**, same for `build-fast`. Modules link as static archives and nothing references `MT_Evaluator.o`, so its 14 symbols never leave `libMT_Control.a`. The three green baselines carry **no** evidence about this conversion; the only mechanical check that touches it is `check.sh`'s `-fsyntax-only` |
| **`MT_Statistics`'s three converted functions, D24** | linked, never run. `gdb` breakpoints on `updateStatistics`, `getStatisticElement` and `writeToFileMT_Statistics` across **all 14** shipped experiments at individual 0: **zero hits** |
| `MT_GUI/MT_StatisticsWidget.cpp`'s nine `.count()` calls on the converted member, D24 | ~~`MT_GUI` is in neither `check.sh`'s `MODULES` nor the Makefile's `CORE`~~ — **STALE, it is in both, and `metagui` drives the widget (C11c)** |
| **the whole `taskCanDoList` index walk, D25a** | compiled and archived (21 `SIG_GPManager::` symbols in `libSIGEL_GP.a`) but **linked into nothing** — 0 in `sigel_eval`, `build-fast/sigel_eval` and `pvm_link`. The `removeAt` path, the append path and the `:122` reference never execute here. The 1.3 binary shows the path is live in a real run (80 appends in two generations); our checks cannot reach it |
| **four of D25a's six `at(canDoIdx)` sites** | `:127`, `:151`, `:1310`, `:1336` sit inside `#ifdef SIG_DEBUG`, and **`SIG_DEBUG` is defined nowhere in this build** — its only occurrence is the `CPP_FLAGS += -DSIG_DEBUG` line of `downloads/sigelSourceDistribution.1.0/sigel/makefile`, the abandoned 1.0 tree. Neither `make` nor `check.sh` parses them. Compiled explicitly with `-DSIG_DEBUG` by review: exit 0, no errors — so no latent defect, but only two of the six were checked by the build |
| the `maxTouchsPerLoop` break, D25a | **dead on every shipped configuration**: the value is persisted in none of the 28 `.exp`, and all five presets call `setMaxTouchsPerLoop(-1)` (`SIG_GPParameter.cpp:325,330,335,340,345`), so `(maxTouchsPerLoop != -1)` is always false |
| all **seven** D23 sites | linked and never called — *in `sigel_eval`, which is a test harness, not the program*. 1.3's master links the whole MT subsystem (`MT_Evaluator` 16 symbols, `MT_Classifier` 25, `MT_Statistics` 83, `MT_Substitute` 20, `MT_TrainingCase` 46, `MT_Trainingset` 14; all 0 in `sigel_slave`), so "never linked" is a fact about our harness and Phase C will link these. Detail: `MT_Substitute`, `MT_Trainingset` and `MT_FitnessTrainer` **are** in `sigel_eval` and `pvm_link` — 59 symbols, pulled in by `moc/MT_GPSystem/MT_GPManager.o` on the link line — but `gdb` breakpoints on all three converted functions and on `MT_GPManager::checkForNewTCase` were not hit across a full evaluation. The classes that never link are **`MT_Classifier` and `MT_Evaluator`**, the queue's two fillers, both evolution-loop. *This row previously said the first three were the unlinked ones: inverted, and asserted without running the `nm` the D22 row had already established for exactly this* |
| **all six D22 sites** | `nm -C build/sigel_eval` finds **0** `SIG_GPOperations::` and **0** `SIG_GPCrossOverTournament::`, and the same in `pvm_link`. The objects are archived in `libSIGEL_GP.a` and never pulled into a link. The evolution loop needs `sigel`, which Phase C blocks |
| all six `Q2CString` sites in `SIG_GPFitnessTrainer`, D21 | zero trainer symbols in `sigel_eval`; `pvm_link` links the object but never constructs a trainer, so they are **link-checked and never run** |
| `SIG_GPPVMData`'s `+ 2`, D21 | `pvm_link` runs the function, but `pvm-check.sh` passes with `+ 1` **and** `+ 0` — `QList` over-allocation hides a shortfall under about 8 bytes |
| all five **unlinked** fitness functions' walks, D20 — `Adaptive`, `Zorc`, `Stepper`, `RealSpeed`, `Force` — plus `SIG_EarlyRunTermSimulation` | `nm` finds 0 symbols for each in `sigel_eval`. `Stepper` is the **only reader of `touchdowns`** in the tree; `Force` the only reader of `listForces` and the only code that ever frees a force vector |
| `sigel_eval`'s trace walk, D20 | runs on all 21 dictorder inputs; its output is dropped by the check's `sed`, so only a crash or a sanitizer report would show — **and there is no sanitizer report to be had.** `dictorder-dump.sh` defaults to `build`, which has no sanitizer, and it **cannot be run against `build-asan/` at all**: the `.rrb` load path aborts under ASan inside vendored cv97 (see the blind-spot section in §10). So this row's second half is empty and the walk is covered by a crash only. *Reconciled 2026-09-03; the restored blind spot falsified it* |
| `SIG_GPNiceWalkingFitnessFunction`'s walk, D20 | runs for 18 individuals, but the check has **one bit** of discrimination — an off-by-one in the index is invisible to it |
| **everything D17, D18 and D19 changed** in `SIG_GPFitnessTrainer` — including all six `delete v[i]`, `resizeOwningHosts`, both `qDeleteAll` in the destructor and both `static_cast<uint>` moduli | `nm -C build/sigel_eval \| grep -c SIG_GPFitnessTrainer` is **0**. `pvm_link` links the object but never constructs a trainer, so it is link-checked and never run. The rewritten walk needs a live `pvm_spawn`; `flushAllDynHosts` is `-devolve`-only; the destructor's three `qDeleteAll` run for no check |

### D27 — the compatibility layer is deleted

**`q2compat.h` and `q2compat_check.cpp` are gone.** `include/compat/` no longer
exists. **No `Q2*` shim type is used anywhere** — every remaining `Q2` mention is
a comment recording what the Qt 2 original did.

**"No shim type" was NOT "no Qt 2 container".** At D27 the six unported GUI
modules still declared **72 lines** of them across 22 files — `QArray` 21,
`QDictIterator` 15, `QList` (pointer list) 14, `QListIterator` 10, `QVector` 5,
`QDict` 3, `QValueList` 2, `QCString` 1, `QQueue` 1 — and **Phase C converted
every one.** *This said 71 with a breakdown that summed to 72: `QList` is 14
lines and 15 occurrences, and `QCString` was missing from the type list. Lines,
all nine types, comments excluded: 72. The correction was written down once,
then deleted while the wrong table stayed — exactly the shape §9 names, so it is
folded into the table this time rather than filed beside it.*
Re-measured 2026-09-05: five textual mentions of a Qt 2 container name survive in
those modules and **all five are comments** recording what the Qt 2 original did.
*An earlier version of this section, of the commit message and of the Phase D
status row all said "no Qt 2 container type exists anywhere in the tree" — false
at the time, and it read as "Phase C has no containers to port". Same too-narrow
scope as the Phase C inventory, in the paragraph that names it.*

**The two blockers, cleared in order.**

**1. `sigel_eval.cpp` tested the D18 cursor rewrite against `Q2PtrList`.** That
check could not simply be deleted — nothing executes `sweepToSpawn` (it needs a
live PVM spawn), so the walk has no other verification at all. It is now
compared against **`Qt2CursorList`**, a **37**-line model of the Qt 2 cursor
(`sigel_eval.cpp`) written from the vendored source, **cited by symbol rather
than by line, because the line references drifted**: `first`/`next`/`last`/
`current` and `clear()` from `qglist.cpp`, `remove()` and `unlink()` from the
same file, the cursor-after-removal rule from `QGList::remove`, and `autoDelete`
from `QList<type>::deleteItem` in `qlist.h` (`if ( del_item ) delete`). **Backed by `std::vector`, not `QList`** — it shares no
implementation with either side, so agreement means agreement. A differential
test needs a reference independent of the code under test; that is what the
model is, and it is not a second shim.
*Removed 2026-09-21, by decision: the conversion is done and proven, and the
check ran its own copy of the walk, never `sweepToSpawn`, so it guarded nothing
further.*

**2. Twenty files got `<QList>` (and often `<QString>`, `<QTextStream>`) only
through the shim** — 16 headers and 3 `.cpp` under `src/`, **plus
`sigel_eval.cpp` at the repository root**. The D26 review found two headers.
Found by removing the include and letting the compiler answer: **175 error
diagnostics**, `check.sh` down to 94 pass / 15 fail. *An earlier draft said
"nineteen files, 30 errors" — the first omits the root file, the second is
reproducible under no scope I can construct.* Each now includes what it uses. *My first
sweep tested only for shim **types** and missed every file that used a plain Qt
6 type transitively — the same "too narrow a scope" shape as the Phase C inventory.*

**`check.sh` lost its shim self-check step.** That step built and **ran**
`q2compat_check.cpp` under ASan and UBSan, and was described in the script as
the only mechanical check that could see an ownership error. It only ever
tested the compatibility layer, so nothing remains for it to check. *It fed only
the grand `fail` counter, never `pass`, so 105/4 is unchanged — verified by
running the parent's `check.sh` on the parent's tree. An earlier version of the
replacement comment claimed the count would drop by one.*

*This said `check.sh` executes no code at all, which was true when D27 was
written.* **Phase C put three running binaries back into it** — `sigel_slave`,
a headless GUI probe, and `guidrive` over twelve scenarios. What is still true,
and is the point of the paragraph, is that none of them reaches the evolution
loop. So **ownership of the evolution-loop containers (`fitTaskList`,
`toSpawnList`, `tours`), which no check can reach, now has no mechanical coverage
of any kind.** `sigel_eval -selfcheck` still runs, but from `fitness-check.sh`,
not `check.sh`.

**Nothing that covered live converted code was lost.** All 19 assertion blocks
were classified against the recovered file: every one tested a `Q2*` type that
no longer exists, and the two behaviours the converted code reproduces by hand
are now covered *better* — `Q2CString::size()` counting the NUL is checked by
`pvm-check.sh` through **real PVM**, and the copy-constructor ownership hazard is
enforced at **compile time** (`= delete`) instead of by a runtime assertion.

**It was also an executable specification for four Qt 2 semantics that unported
code depended on — and PHASE C HAS SINCE CONVERTED ALL FOUR CONSUMERS.**
Re-measured 2026-09-05: `MT_GUI`'s queue is a Qt 6 `QQueue` with the missing
`isEmpty()` guard written in at `MT_ExperimentWidget.cpp`, carrying the Qt 2
behaviour it replaces in a comment; `SIG_GPParameter.cpp:524` is
`if ( hostList2.removeOne( host ) )` over a Qt 6 `QList<SIG_GPPVMHost *>`; and
**`QListIterator` appears nowhere in `src/` or `include/`** — the row that said
"the 10 remaining `QListIterator` lines" is 0. The semantics themselves are still
worth reading before touching that code, and `q2compat_check.cpp` is recoverable
from git history:

| semantic | where it landed |
|---|---|
| `QQueue::dequeue` must unlink on a **null head** — the self-check noted it "used to spin forever" otherwise | `MT_ExperimentWidget`'s `isEmpty()` guard |
| `QQueue::remove()` exists (`qqueue.h:60`), and `head()` after it | same file; the guard covers both |
| **`remove(ptr)` removes *that* pointer, not the current one, reports whether it unlinked, never frees while non-owning, and a failed search KILLS the cursor** | `removeOne` in `SIG_GPParameter` |
| `QListIterator` stays dead off the end; `atFirst()`/`atLast()` both true on empty | no consumer left |


**`Qt2CursorList` was verified, not assumed.** Review checked it line by line
against vendored `qglist.cpp`, ran it against the recovered `Q2PtrList` over
**20,000 randomised trials × 40 operations** comparing full state after every
step (0 divergences), repeated that against an independent node-for-node
`QGList` transcription so a *shared misreading* would surface (0 divergences),
and mutation-tested both the probe and the shipped check — perturbing D18's
index walk made `-selfcheck` fail and exit 1. It covered 6 operations where the
self-check covered 11; the missing five (`prepend`, `at(i)`, `remove(idx)`,
`removeFirst`, `removeLast`) had no converted consumer. *The model and its test
were removed 2026-09-21 — see the D18 row of the steps table.*

**Checks unchanged: 105 pass, 4 fail, 309 warnings, both baselines empty,
sanitized clean, PVM both PASS, `sigel_eval -selfcheck` ok.**

**What the layer was.** 806 lines at its peak, 539 at deletion; six container
types reproducing Qt 2.3 semantics on Qt 6. It existed so the port could convert
one container at a time with both halves of the tree compiling throughout, and
so that the differential tests had something to compare against. **23 commits
changed `q2compat.h` and 20 changed the self-check** — *an earlier draft said
"twenty-seven steps used it", which is not measurable as stated.* The self-check
held **80** assertions, not the "~40" the deleted `check.sh` comment claimed and
this section repeated without measuring.

### A logging system

Qt 2's `QTextStream` wrote through to unbuffered `stderr` on every `<<`. Qt 6
buffers 16 KB and flushes only on `flush()`, `Qt::endl`, overflow or
destruction. A trailing `"\n"` does **not** flush.

**509 `SIG_IO::cerr`/`cout` statements; 9 of them flush.** Most diagnostics now
sit in a buffer and are lost if the process dies — exactly when they are wanted.

Needs levels, one place that decides where output goes and when it flushes, and
something the GUI can display. **Do not fix this by adding `Qt::endl` to 500
call sites.**

### Error reporting for core

`SIG_Environment.cpp` opened three `QMessageBox` dialogs on terrain load
failure. Core must build against Qt6Core alone, and `-evolve` has no
`QApplication`, so the dialog could never have worked headless. A2 replaced them
with console output.

**That is a stopgap.** A GUI user now gets no dialog. Core needs something it
can call without knowing whether a GUI exists — a callback, a signal, or an
error sink handed in at construction. Do not settle it per-site.

7 core files touch dialogs. Done: `SIG_Environment.cpp`. Remaining:
`MT_Controller.cpp`, `SIG_GPFitnessTrainer.cpp`,
`SIG_GPRemoteZORCFitnessFunction.cpp` (+ WIN variant, + both headers).

### THE RANDOMISER AGREES WITH 1.3, AND THE CHECK HAS NO PHYSICS IN IT — 2026-09-05

**The first cross-machine agreement on GP OUTPUT this project has.** Every
earlier one was a file the port wrote and 1.3 read, or a count that turned out
to be forced by `createTours`.

**The recipe, designed by the oracle, and the point is that BOTH SIDES CAN RUN
IT.** A draw counter in `SIG_Randomizer` was the obvious idea and would have
been half a measurement — the oracle has 2003 binaries and no source, so it can
never instrument anything, and §9 says to design no check that assumes both
halves can be instrumented. This needs only the GUI:

    File > New Experiment          -- an EMPTY population, 0 individuals
    GP-Parameters > Random seed    -- the GP seed
    Individuals > Add, N           -- N programs straight from the randomiser
    File > Save Experiment

No PVM, no DynaMechs, no fitness, no machine-dependent arithmetic. **Every
opcode, both operands and the program LENGTH are draws**, so one extra or
missing draw shifts the whole remaining stream — strictly more sensitive than
counting draws.

**Result, seed 12345, N=5, twoBases:**

| | 1.3 | this port |
|---|---|---|
| per-program lengths | 30, 152, 638, 45, 153 | **identical** |
| individual 0, first six | `LOAD 9988,-22117 / MOVE 19741 / ADD -5659,-9758 / MAX -26233,15212 / NOP / JMP -23212` | **identical, token for token** |
| two runs on one machine | 14 lines differ, all wall-clock | 12 lines differ, all wall-clock |

**AND THE HASHES MATCH TOO, on both variants**, once the extraction rule was
stated rather than assumed:

    instructions only        1018 lines  6283c93317ee296b8d16e761ea31ccc7fa01e5b3be6d866569b3cac4075e366f
    incl. BEGIN/END markers  1028 lines  3da84433537feb890e9b902450b30ce3345de2b4a36956eb669d4bae64763383

**THE RULE, because the hash is meaningless without it.** Every line strictly
between `PROGRAM BEGIN{` and `}PROGRAM END;`, trimmed of leading and trailing
whitespace, in file order, joined with `\n`, **and no trailing newline** — add
one and the first digest becomes `fb16518d0e8cb37db1e7a35cc4de3d0f997b41c9a5b9de59a8e21d683a39aae0`.

*This is not a negotiated hash. The rule was specified from this side before
seeing the oracle's data, and its two independently saved files satisfy it byte
for byte.*

**The oracle's first figure of 1,033 was wrong and is withdrawn**, along with
the digest that went with it. Its extraction was `grep -E '^\s+[A-Z]+ '` over
the population block, which sweeps in any indented line whose first token is
uppercase — catching `PROGRAM BEGIN{`, `INDIVIDUAL BEGIN{` and
`HISTORY BEGIN{INDIVIDUAL IS CREATED:`, exactly three per program, five programs,
fifteen junk lines. **A regex loose enough to be convenient is loose enough to
change what you are hashing, and the error is invisible in the digest: a hash
mismatch looks exactly like a content mismatch.** What caught it in one exchange
rather than a hunt through the randomiser was reporting the LINE COUNT beside
the digest. *Send the count with the hash.*

**Checked** as `rngseed`, the tenth `gui behaviour` scenario, so the stream is
pinned against drift.

**TRAP — a `.exp` has TWO `RANDOMSEED` keys.** The first is the SIMULATION seed,
the second the GP seed. Both ship as 0, and setting the GP one leaves the first
at 0. The scenario prints both every run — line 10 reads 0 and line 83 reads
12345 — because reading the first and concluding the seed never took is the
obvious mistake, and the oracle nearly made it.

**TWO SHIPPED EXPERIMENTS ARE THE SAME BYTES.**
`twoBasesSimpleFitness1.exp` and `twoBasesHighCrossOverRate.exp` are
**byte-identical**, both md5 `35bcdb3a7a2bb6c2af7ccf964761e87e`. So that hash,
which §9 cites as the ancestry anchor both machines derive from, does not name a
unique file. *It has been visible in `fitness-baseline.txt` all along — the two
experiments have identical fitness for all three individuals — and nobody had
noticed.* The ancestry argument still holds, because both sides have the same
bytes; the anchor is just less specific than it reads.

**Two harness traps, found by running the recipe against a session that already
had an experiment open.** The driver loads one at startup, so `New Experiment`
leaves TWO in the tree: the first attempt added to and saved the **loaded** one,
and the tell was `programs=125`, i.e. 120 + 5. The fix then deleted
`topLevelItem(0)` as "the loaded one" and removed the new empty experiment
instead — **because C7 restored Qt 2's PREPENDING item insertion, so the new
experiment is at index 0 and the loaded one at 1.** Select by name.

### FLAKE — two runs failed leaving no evidence; the diagnostics now would

**`pagesave` failed once**, 2026-09-04, with `a pagesave run did not finish:` and
**nothing after it**. Run directly straight afterwards it was clean, and the next
full `check.sh` was 846/0. Both stdout files were empty and the captured stderr
had been deleted unread. guidrive's watchdog `fflush()`es stdout before
`_exit(3)`, so an empty stdout with no `!! WATCHDOG` line means the watchdog
never fired and the process died without flushing.

**A first diagnosis said "blocked outside the event loop until `timeout` killed
it", and that is probably wrong.** A later `check.sh` run was **killed by the
system for low memory** — the box has 7.2 GiB and carries a desktop session, an
editor and two agent processes. **An OOM kill is SIGKILL: no flush, no watchdog,
no stderr, non-zero exit — every symptom, and simpler than a blocking call.**
*The blocking-call reading was a real deduction from real evidence and still
fitted; it was just not the only thing that fitted, and it was written as though
it were.* **Check free memory before chasing a blocking call.** The diagnostic
now prints both files' sizes and the captured stderr. *The `gui behaviour`
section had already been fixed to print its stderr on the failure path; this one
was written afterwards and did not inherit it. **A fix applied to one of two
near-identical blocks is half a fix.***

*This was the second flake this project has seen. The first was a 844/1 run
followed by 845/0 on an identical tree — cause unknown, never reproduced, and it
left no evidence either. This one now would.*

**A `roundtrip` run printed a self-contradiction, and that IS fixed.**
`export1 0 bytes 383d1f4f…` beside `export2 299 bytes 383d1f4f…` — sha256 of an
empty file is `e3b0c442…`, so those cannot both be true. The probe took the hash
at one instant and the size at another, thirty lines and one whole import apart,
and printed them as simultaneous. **`sha256Of` now returns the length of the
exact bytes it hashed.** *That does not explain what made a separate stat report
0; it removes the probe's ability to report a contradiction, and leaves a real
truncation to show up where it should — as a hash mismatch and a failed round
trip.*


### D29 — how the run lock is built, and the versions that were wrong

**THE GUARD IN `SIG_ExperimentView::putIntoExperiment()` SITS BELOW THE LCD READ,
DELIBERATELY — moved there 2026-09-05.** It was above it, and that would have
been a divergence **the run lock itself introduced**. `poolGeneration` IS
incremented per generation inside the loop (`SIG_GPManager.cpp, run`), 1.3 has no
guard anywhere in this function, and this document's own reading of 1.3 says the
counter "moves when the experiment is selected, when a page is switched, and at
the top of `slotStartEvolution`" — so on 1.3 a page switch **during** a run
refreshes that display to the advanced value, and a guard placed first would have
left the port showing a stale one. **Reading is not writing.** D29 exists to stop
parameter *changes* reaching a running experiment; every write in that function
is still behind the guard. *Source-derived, and NOT yet confirmed on the running
1.3 — the 187 local and 172 oracle samples that establish "the counter does not
move during a run" (on 1.3, and on the port before D37) were taken without a mid-run page switch, so they cannot
distinguish the two placements. Listed as open.*

**Read this before changing the guard.** The decision is in §5c; this is what it
took to implement, and each wrong version passed its own check.

**Where the guard is.** Each experiment has a bool, `evolutionRunning`, which
`SIG_GUIGPExperiment::isRunning()` returns. `slotStartEvolution` sets it just
before `guiGPManager->start()`. `slotEvolutionStopped` clears it, and
`slotStartEvolution` calls that slot also when `start()` throws.
`SIG_ExperimentListView::isRunning()` is true while any experiment runs. Every
check asks the list view, so each check refuses while any experiment runs. The
checks, by group:

| guarded | why it needs its own guard |
|---|---|
| `putAllIntoExperiment()` | the aggregator; a **page switch** reaches it — `slotSelectionChanged` ends with an unconditional call |
| `SIG_ExperimentView::putIntoExperiment()` | **the page that stays live.** See below |
| the seven import/load slots | they write `gpExperiment.*` directly and never go through the aggregator. `slotRobotLoad` replaces the whole robot. Reachable mid-run through context menus parented on `SIG_GUIGPExperiment`, not on the pages |
| the tree-click emit | drives the 29 `evolutionRunningActions` |
| the four MetaGP actions | none was in `evolutionRunningActions` at all |

**Version 1 was wrong: it guarded only the aggregator.**
`slotStartEvolution` disables five pages — `gpParameter`,
`simulationParameter`, **`robotView`**, `languageParameters`,
`environmentView` — and the aggregator commits a *different* five:
`experimentView`, `gpParameter`, `simulationParameter`, `languageParameters`,
`environmentView`. **`experimentView` is committed and never disabled.** It is
the page the user is looking at when they press Start, it stays live for the
whole run, and its history checkbox and autosave slider are wired straight to
`SIG_ExperimentView::putIntoExperiment()`. The running GP reads `getAutosave()`
**every generation** (`SIG_GPManager.cpp:805-807`). Meanwhile `robotView` is
disabled but `SIG_RobotView::putIntoExperiment()` has **no callers at all**.

**Version 2 was wrong: it asked `SIG_GPManager::running()`.** That was a 2003
stub returning `false` unconditionally, overridden nowhere, so a guard against
it could never fire. **Deleted 2026-09-09** — the line number this used to cite
is gone with it. **It is a leftover, not an
unfinished feature, and that matters before anyone implements it.**
`SIG_GPManager` was to derive from Qt 2's `QThread`; the base class is already
commented out in release 1.0 (**September 2001** — `sigelSourceDistribution.1.0.tar.gz`
is stamped 2001-09-06, and the file itself 2001-09-05; August 2001 is the date of
the shipped experiment DATA, not of the release) and in 1.3, so `running()`,
`wait()` and `msleep()` are its orphaned methods. The team's own final report
(the PG 368 Dortmund `endbericht.pdf`, read from `data/` before it was deleted on
2026-09-19, and still on sourceforge) never uses the word *thread* in 11,579
lines — their parallelism is PVM, separate processes. **So making `running()`
answer honestly is new design by us, not restoration**, and there is no earlier
revision to consult. **Where that comes from, since none of it is in this repo's
git:** SourceForge's CVS is shut down (rsync refused, ViewVC redirects to the
download page); the CVS metadata in the `xb/` tree — a 2003 working copy,
`xb/kdesigel/pixmaps/CVS/Root` and `Entries`, read there before `xb/` and
`kbin.tar.gz` were deleted on 2026-09-20 — names
`:ext:…@cvs.sigel.sourceforge.net:/cvsroot/sigel` and dates its oldest revision
`1.1.1.1` to 18 December 2001, a vendor import and therefore *after* the change;
and the project's own file listing offers source tarballs for **1.0 and 1.3
only**, with 1.1 and 1.2 as Debian binaries. (The 1.3 tarball's `README` still
says "v1.1" — a stale README, not a source release.) Step A (2026-09-09) removed the commented base class, the sole
`qthread.h` include, `wait()`, `msleep()` and the one `wait()` call.
`running()` was left for a separate decision at that point; that decision is D33,
taken 2026-09-09, and it is gone too.

**That always-false stub is also why 1.3 has the same visible defect from a
different cause: one click on the experiment tree
re-enabled all 23 locked actions mid-run.** The actions *are* correctly disabled
at start — `SIG_GUIGPExperiment` emits `signalEvolutionNotRunning( false )` and both
construction sites relay it — so the defect was never "they are not disabled".

**Ordering is load-bearing.** `slotStartEvolution` calls
`putAllIntoExperiment()` **before** `guiGPManager->start()`, so the settings a user
chose are committed at start and only later writes are refused.

**Coverage.** The `runlock` scenario executes the guard in its locked state,
which nothing did before — a review measured that both mechanisms could be
reverted wholesale with all checks green. It needs no live evolution: it sets
the protected `evolutionRunning` itself and clears it through
`slotEvolutionStopped`. Its observable is the **model**, `gpParameter.getMaxAge()`,
because two cheaper ones are wrong: reading the spin box back reports a false
refusal (nothing calls `getOutOfExperiment` on the way back), and saving is
impossible under the lock because `Save Experiment` is itself locked.
**NOT COVERED:** `runlock` sets `evolutionRunning` itself, so it cannot check
the line in `slotStartEvolution` that sets it. Delete that line and the check
still passes. Nor does it run the call to `slotEvolutionStopped` in the `catch`
of `slotStartEvolution`: it calls that slot directly, so it checks what the
slot does — clear the flag and show the pool generation — not that the `catch`
calls it. Checking it
needs a real run.

**The Individuals view is locked, not disabled.** `allIndividualsView->setEnabled(
false )` is commented out in the 2003 source.
`SIG_AllIndividualsView::slotEvolutionNotRunning` disconnects the list's context
menu, double-click and selection, and the Individuals actions are in
`evolutionRunningActions`. **That slot runs on every tree click, not only at run
start and stop** — since 2026-09-16 it is driven by
`SIG_ExperimentListView::evolutionNotRunning`, so every experiment's list is cut,
not just the running one. It disconnects by receiver: a wildcard also cuts
`QTreeWidget`'s own relay for `itemDoubleClicked`, which killed double-click
everywhere until it was found.

### PRE-EXISTING LEAK — the simulation backend is never freed

`SIG_Simulation.cpp, SIG_Simulation` allocates a `SIG_DynaMechsSimulationData` with `new`.
`~SIG_Simulation()` at `:86` is empty. Nothing deletes it — verified across
`src/`. Same for `simulationQueries` and the command interface. One simulation
object per fitness evaluation, so a GP run leaks the physics backend thousands
of times. Base `SIG_SimulationData` has no virtual destructor either.

Consequence for B4: the destructor added to `SIG_DynaMechsSimulationData` is
correct but **does not run today**. `dynaMechsLinks` has two other free paths,
both converted:

- `insert()` over an occupied slot, which fires only for a robot with two joints
  between the same pair of links — no shipped robot has one
- **the constructor throwing**, which is the one that happens. `SIG_Mirtich.cpp`
  throws on a NaN mass or inertia, from `computePhysics` inside the
  `SIG_DynaMechsLink` constructor. A scope guard in the constructor restores the
  free Qt 2 did while unwinding. **Do not delete it as redundant.**

Out of scope for the Qt port.
