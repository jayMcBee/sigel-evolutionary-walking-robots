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

**Status — 2026-09-02**

| phase | state |
|---|---|
| 0 — comments to English | done for the 9 core modules; 9 GUI files still hold Latin-1 |
| A — core onto Qt 6 | **done**, tags `step-A0`…`step-A9` |
| B — ownership explicit | **subsumed by Phase D**, which deletes the containers rather than converting them. **0 `setAutoDelete` calls left in core**, re-measured 2026-08-30 after D25c: D11 removed the last in `SIGEL_Robot`, D24 the last in `MT_Control`, D25b replaced the two `fitTaskList` calls with an RAII guard, and D25c wrote out `tours`' two real frees at their sites. Every remaining call in the tree is in `SIGEL_MasterGUI`, `SIGEL_Visualisation`, `SIGEL_CommonGUI` or `MT_GUI` — Phase C. *This row has been corrected five times, each time by review: it has read 13, 12, 11, 3 and 1. The recurring errors were counting comments as calls and quoting a `SIGEL_MasterGUI` figure as a core one.* **Not all of these were unreachable, and an earlier version of this row said they were** — `SIG_GPPopulation::pool` is owning, is constructed on every `sigel_eval` run, and takes 100 `insert()`s inside both gates; see "What the gates actually reach" in §10 |
| R — build and run | core builds and runs. **No longer checked only against itself** — Phase V has confirmed both the ordering and the arithmetic against the 1.3 binary, §7 |
| T — old-Qt tool container | **done 2026-08-27.** `tools/qtmig`, §4 |
| D — delete the shim, migrate the data | **DONE 2026-08-30.** `q2compat.h` and `q2compat_check.cpp` deleted; `include/compat/` gone; **no `Q2*` shim type is used anywhere**. D1–D27. *This is not "no Qt 2 container exists" — the unported GUI modules still declare **71 lines** of `QArray`, `QDict`, `QList`-as-pointer-list and friends, all of which Phase C must convert. See D27.* The shim's self-check step is gone from `check.sh`, which now runs no code. §10 |
| P — PVM | **DONE 2026-08-28.** Vendored 3.4.3 replaced by upstream 3.4.6; nine patches carry the four config lines and Debian's eight source fixes; `libpvm3.a` and `pvmd3` build; SIGEL's two PVM objects link against them and `SIG_GPPVMData` round-trips through real PVM. `sigel`/`sigel_slave` still need Phase C. §7 |
| C — GUI | **DONE 2026-09-02. C1–C10 complete; C11a–C11d done.** All 20 Designer forms converted; all five GUI modules build as archives; **both programs link and run**; **100 dead `connect()`s repaired, tree-wide count 0 with no baseline anywhere**. **C9** made it match what 1.3 SHOWS — 42 menu entries, the toolbars and the loaded values, now the `gui vs 1.3` gate. **C10** DROVE it, diffed against the oracle driving 1.3 with XTest, and found two defects reading it could not: `QTreeWidget::clear()` emitting a signal Qt 2 blocked, which killed the application on a large delete, and an eaten ampersand. Second gate, `gui behaviour`. **C11a** drove the five View pages C10 never opened — the 12-probe validator battery matches 1.3 character for character under two locales, nine typed values come out byte-identical in the saved `.exp` across the two architectures, and a `QIntValidator` over-range divergence was found and **accepted as D28**. **C11b** drove the Import/Export round trips — **seven of the eight exports are byte-identical to what the 2003 i386 binary writes** — and found default-constructed language parameters coming out alphabetical where 1.3 gives `QDict` hash order, which had also been wrong in `dictorder-baseline.txt` for all seven robots. **C11c** drove the six dialogs, closing C7's validator set at 21 of 21, and found Qt 6 selecting a pre-filled field where Qt 2 did not — a typed digit REPLACED the value instead of appending, so Add-individuals turned 1 into 2 where 1.3 makes it 12. Two reviews of the checking machinery then found five probes that could not fail and a **demonstrated false pass** (all 30 icons replaced with garbage, gate green). **C11d** then opened the MetaGP window for the first time and found C7's locale fix had never reached MT_GUI's ten validators — `"1,000"` accepted, `toInt()` returning zero. **The evolution path then ran end to end, and THE PORT EVOLVES** — population 100, 30 generations, best fitness 0.063794 → 0.141625. *A cross-machine claim once stood here and was WITHDRAWN: the counts it rested on are forced by the code. §9 item 2.* **845 pass, 0 fail.** §7 |
| V — check against the 1.3 binary | **V1, V5's MDH probe, V6, V7 and V8 all done, all PASS.** Ordering: 10 of 10 container orders match. Arithmetic: `twoBases` exact bit for bit, `octopus` 9/9 with three joints exact and 5 ulp worst. **V6, V7 and V8 done 2026-08-29** — friction and no-collide negotiation, their four remaining rules, and the GP parameter blocks captured *before* their conversion. `verification-against-sigel-1.3/v6`, `v7`, `v8`. **V9 done 2026-08-29, 3 of 3** — three function bodies disassembled, which symbol lookups cannot see. **V3 SATISFIED 2026-09-02** — same-box determinism, demonstrated twice by the oracle (`serA`≡`serB`, `octGateA`≡`octGateB`). **V4 DROPPED 2026-09-03** — whole-run digests cannot cross an x87/IEEE boundary, and the counts that appear to agree are forced by the code. **Replaced by a measurement of OUTPUT needing no reference: the port EVOLVES** — population 100, 30 generations, best fitness 0.063794 → 0.141625, §9. V2 remains open |

**SCOPE — DECIDED 2026-08-23. Read this before changing anything.**

**The reference for this port is SIGEL 1.3 and nothing else.** The 1.3 source in
`x/kdesigelSources.1.3/` and the 1.3 binary running on the x86 box, reachable
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

**Still needs a decision:** whether `QTextStream` no longer printing `-0`
matters. *This was struck during the 2026-08-30 streamlining on the grounds that
§9 says `-0` "appears in no shipped stream" — but that scope is the shipped
`.exp` and `.rrb`. **POV-Ray output is a third stream**, generated per frame,
never shipped, unmeasured, and per the x86 box it is the path that produced
every published film. C5 converted its writer. Restored, correctly scoped.*
And with it the larger question the fourth family reopens: whether to route
every real number through `snprintf` to match 1.3's tie rounding.

**Order of work, revised 2026-08-27.** Reordered around the goal above: the
interface is the work, so it goes first, and PVM follows because without it the
ported interface has nothing to drive.

1. ~~Build core plus a small program that runs one fitness evaluation, under
   AddressSanitizer.~~ **Done.**
2. ~~**Stand up the old-Qt container**~~ — **done**, `tools/qtmig`. §4.
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
   30 generations, best fitness 0.063794 → 0.141625. §9 item 2.

**A caveat that governs the order of what is left.** Everything after Phase D's
simulation-side work is in the **evolution loop**, which now runs (§9 item 2):
~~the 13 remaining `setAutoDelete` sites, `Q2PtrList`'s `fitTaskList` and
`toSpawnList`, and the rest of `Q2PtrVector`~~ — **all converted as of D25c;
core `setAutoDelete` is 0, and D26 converted the last two GUI-side members.
The shim itself was deleted in D27.** Both gates and AddressSanitizer reach
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

- Source root: `x/kdesigelSources.1.3/kdesigel/kdesigel/`
- ~~The shim: `include/compat/q2compat.h`; its self-check:
  `include/compat/q2compat_check.cpp`~~ **Both deleted in D27.** Recover them
  from git history if you need the Qt 2 semantics they recorded — see D27 for
  the four that unported code still depends on.
- Build: `make` at the repo root gives `build/sigel_eval` under ASan and UBSan.
  `make B=build-fast SAN= SIGSAN=` gives an unsanitised build about 15x faster,
  in its own directory.
- Verify: `./check.sh` from the repo root compiles every module and header.
  Takes several minutes, and **exits non-zero if anything fails or is skipped**.
  *This said "it runs no code", which was true between D27 and Phase C.* It now
  runs `sigel_slave`, a headless GUI structure probe, and `guidrive` through six
  scenarios plus two locale re-runs. Further execution is in `./fitness-check.sh`,
  which runs `sigel_eval -selfcheck`.
- PVM: `make pvm && make pvm-link`, then `./pvm-check.sh` starts a daemon and
  runs both round trips. Not one of the three checks below — it has no baseline,
  it is PASS/FAIL.
- Run the published experiments: `./replicate.sh build-fast`. Read the scope
  note at the top of this file first — those experiments are from SIGEL 1.0 and
  do not test this port.
- **After every step, an independent agent reviews the diff with fresh eyes.**
  Not optional — every round so far has found a real defect, and a compile check
  proves nothing about ownership.
- When adding an assertion to `sigel_eval -selfcheck`, break the code it covers
  and confirm the check aborts. Nine assertions have passed on broken code.
  *This rule was written for the shim self-check, deleted in D27; it applies
  unchanged to its successor.*
- Sibling docs, both independent of this port: `future_refactorings.md` (C++
  language level) and `physics_backends.md` (**decided and executed 2026-08-28**:
  the Dynamo backend is deleted, the Dynamo maths library stays).
  Do not mix their commits with this work. Third sibling:
  `regression_1.0_to_1.3.md`, deferred until the port is done.

---

## 1. Repo

```
/home/jan/Downloads/sigel/
├── PORTING.md                              this file
├── check.sh                                per-file compile check, §7
├── dictorder-dump.sh                       dictionary-order check, §7
├── expstruct.py                            structural fingerprint of an .exp, §9 item 2
├── guidrive.cpp                            the GUI behaviour harness, 23 scenarios
├── fitness-check.sh                        fitness check, §7
├── pvm-check.sh                            does PVM run? Phase P, P3 and P4
├── pvm_link.cpp                            SIGEL's PVM objects vs real PVM
├── pvm_smoke.c                             one PVM round trip, driven by it
├── replicate.sh                            runs the published experiments, §7
├── Makefile                                the build, §7 Phase R
├── sigel_eval.cpp                          one fitness evaluation, §7 Phase R
├── future_refactorings.md                  sibling doc, independent of the port
├── physics_backends.md                     sibling doc, independent of the port
├── regression_1.0_to_1.3.md                sibling doc, DEFERRED
├── patches/                                14 patches to the vendored tree
├── shim/                                   pre-standard C++ headers
├── build/                                  untracked, removed by `make clean`
├── data/                                   untracked, 7 robots and 14 experiments
├── kdesigelSources.1.3.tar.gz              upstream source (2003-04-30)
├── supportingLibs.tar.gz                   vendored deps
├── kbin.tar.gz                             2003 i386 binary, reference only
├── pvm3.4.6.tgz                            upstream PVM, tracked -- Phase P
├── x/kdesigelSources.1.3/kdesigel/kdesigel/
│   ├── src/       19 module dirs           ~40k LOC
│   ├── include/   16 module dirs           ~25k LOC
│   └── ui/        20 .ui files             Qt 2 Designer format
├── x/supportingLibs/supportingLibs/        Qt 2.3, dynamechs, cv97, newmat09,
│                                           fparser, pvm3, Dynamo (maths only),
│                                           SOLID + qhull (present, not built)
└── xb/                                     extracted 2003 binary
```

Pristine source tagged `v1.3-pristine`. Everything diffs against it.

SIGEL, Uni Dortmund LS11, 2001–2003, GPLv2. Genetic programming evolves walking
gaits for simulated robots. Despite the name "KDESIGEL" there is **no KDE
dependency** — zero `k*.h` includes. Plain Qt 2.3.

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
`QFile` 15, `QDir` 14, `QStringList` 12, `QDataStream` 12, `QThread` 3,
`QTimer` 2.

**Absent, and usually the worst part of a Qt port:** 0 `QPainter`, 0
`paintEvent`.

**But `SIGNAL()`/`SLOT()` is not a no-op row, and this file said it was.** It
read: "The 444 `SIGNAL()`/`SLOT()` macros in 174 `connect()` calls are **all
still valid** — string-based connect was never removed." *Neither figure
reproduces at any scope; 174 is `SIGEL_MasterGUI`'s macro count alone (80 + 94).
Measured over the whole source tree: **299 `SIGNAL()` + 312 `SLOT()` = 611
macros, in 246 `connect()` and 54 `disconnect()` calls.* A tree-wide
`\bconnect *\(` returns 248 — two of those are not Qt at all: the BSD socket
`connect()` in `src/manage_dyn_slave.c:121` and one inside a doc comment in
`include/SIGEL_Simulation/SIG_Simulation.h:126`. Every real one is in a `.cpp`.* The *mechanism*
survived; **nine kinds of signal did not** — 12 of the tree's 37 distinct
signatures, counting overloads — and a
string-based connect to a signal that is not there compiles, links, runs and
never fires. Nothing in the compiler or in `check.sh` could see it. C4 shipped
ten such connects and only review caught them.

Each signature below was resolved with `QMetaObject::indexOfSignal` against the
real Qt 6.10.2 meta-object — the same lookup `connect()` performs — not read off
a porting guide:

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
is live — moc emits a cloned method for the default argument of
`clicked(bool)`, and a `dumpObjectInfo` shows `clicked(bool)` then
`clicked() [CLONED]`; `activated(int)` on a `QComboBox` is live and only the
`QString` overload went; `valueChanged(int)` (checked on `QSlider` **and**
`QSpinBox`, since the 85 sites split across both), `textChanged(…)`,
`stateChanged(int)`, `toggled(bool)` (including on `QAction`, which two senders
are), `pressed()` and `timeout()` are all unchanged. **The tree has 37 distinct signal signatures, not 41.** 41 is the count of
*raw spellings*, which differ only by whitespace — `selectionChanged(
QListViewItem * )` and `selectionChanged(QListViewItem*)` are one signature and
two spellings, and `connect()` compares the normalised form. Normalised, the
pristine tree has **37**, and they partition exactly:

    17  SIGEL's own signals      (declared in the tree's own `signals:` blocks)
    12  dead in Qt 6             (the nine kinds above, across their overloads)
     8  still live, unchanged
    --
    37

**`lostFocus()` was in this table and was wrong in both halves.** It was listed
as `QLineEdit`'s, 2 sites, to become `editingFinished()`. **Qt 2.3's `QLineEdit`
had no `lostFocus()` at all** — its `signals:` block is `textChanged` and
`returnPressed`; the signal arrived in Qt 3. Both sites connect on `editor`,
an **`MT_Editor`**, which declares its own `void lostFocus();`
(`include/MT_GUI/MT_Editor.h:32`) and emits it from `MT_Editor::hideEvent`. It
is a user-defined signal and needs no conversion. `editingFinished()` would
have been an outright regression: `MT_Editor` exists to distinguish Return
(sets `acceptChange`, commits) from focus-out (does not), and
`editingFinished()` fires on both. **Confirmed on the running 1.3**: typing
`999` and pressing Return commits; typing `111` and clicking away leaves the
old value.

**The general rule this cost, and it points both ways.** A `SIGNAL()` name does
not tell you the class. A user-defined signal sharing a framework name is a
false positive whichever direction the sweep points — dead-signal hunts and
live-signal hunts alike. **The counter-check is to parse every `signals:` block
in the tree and intersect.** Against the *corrected* nine that intersection is
**empty** — `lostFocus` is precisely the name it removed. *Recorded as "exactly
one collision" at first, which was true of the wrong list.*

**And the live column is still recorded by signature, which is the same
blindness one column over.** `textChanged(const QString&)` is listed unchanged,
and it is — on `QLineEdit`, which all 27 senders are. But Qt 2's `QComboBox`
declared `textChanged(const QString&)` too and **Qt 6's does not**
(`currentTextChanged`), and `QSpinBox::valueChanged(const QString&)` is
likewise gone. Zero such senders today; C6 and C7 must check the class, not the
spelling. The
inverse error was in this table at the same time: `selectionChanged` was
recorded only in its no-argument form, and Qt 2's `QListView` declared a
`selectionChanged(QListViewItem*)` overload too. *The two errors cancelled in
the total, which is why the pristine count did not move.*

**100 in the pristine tree; C4 repaired 10, C6 repaired 31, C7 repaired 44 and
C8 the last 15 in `MT_Control`. None remain**, and
zero in the **thirteen other** modules `check.sh` compiles. *`MT_Control` is one
of the fourteen it compiles, so "zero everywhere it looks" was false; its 15 are
carried as a baseline until C8.* Of the 100, **83 are `connect()` and 17 are
`disconnect()`** — a dead `disconnect` is equally a no-op, so nothing is
mis-prioritised, but the column counts sites, not connects. *That split was
first recorded as 82/18, computed before the table was corrected and not
recomputed after: `lostFocus` contributed one of each and
`selectionChanged(QListViewItem*)` contributes two connects. **A derived figure
left standing after its source was corrected — the third time in three rounds.***
**`check.sh` fails any module whose count exceeds its baseline**, which is zero
for every converted module.
The gate also scans `sigel.cpp`, `sigel_slave.cpp` and `SIGEL_RealInterface`,
which sit in no module directory and so were under no baseline. **The forms need
a separate pattern**: a `.ui` `<connection>` carries the bare signal name as XML,
not a `SIGNAL()` macro, so the main regex structurally cannot match one. *It was
pointed at the `.ui` directory anyway and claimed a coverage it could not have.*
All 49 form connections are live today.

**And the regex now self-tests.** It is the one check here that is a pattern
rather than a compiler, so it is the only one that can quietly stop matching —
which it did, for a whole commit, when `selectionChanged` was anchored to `( )`.
33 rows of spellings that must match and near-misses that must not, including a
converted `selectionModel()->selectionChanged(sel, desel)`, which must pass.
Re-narrowing the pattern makes the self-test fail on exactly the two rows the
bug hid.

### Structure

| Metric | Value |
|---|---|
| Core files needing **no** container work | **210 of 266** |
| Core files touching dead Qt 2 containers | 56 — worst is `SIG_Robot.cpp` (30) |
| `Q_OBJECT` in core | **3** — `SIG_Simulation`, `MT_GPManager`, `MT_Controller`. Was 4; `SIG_DynaSystem` went with the Dynamo backend, `physics_backends.md` |
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
It is Qt 2's own Qt **1.x** → Qt **2.x** script; the proof is vendored here —
`x/supportingLibs/supportingLibs/qt/src/doc/porting.doc` is headed *"Help with
porting from Qt 1.x to Qt 2.x"* and line 156 points at `qt/bin/qt20fix`. The
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
| **D2** | Target Qt version | **6.9.2**, Ubuntu `qt6-base-dev`; recorded, not pinned. `qmake6 -query QT_VERSION` and `uic --version` both say **6.10.2** on 2026-08-30 at C1. Why it moved from the recorded 6.9.2 is not established here; what is measured is that nothing was changed to follow it and all four gates reproduce, warning count included |
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
| **D26** | What the 1.3 binary is asked for | **structure and arithmetic, not fitness equality.** Three tiers, in descending confidence: the container ordering and numbering, which compare exactly (V1, V2); per-individual fitness, which is chaotic across architectures and is therefore a judgement (no cross-machine gate; V4 dropped); the non-integrating quantities, which compare exactly but need `gdb` (V5). Bit-exact agreement on an integrated trajectory is **not** a target and its absence proves nothing — §7. Recorded because this file repeatedly described the missing reference as "fitness numbers", which is the one thing that binary cannot usefully give |
| **D25** | What "done" means | **Plain modern Qt 6, nothing left over.** `q2compat.h` deleted, no Qt3Support class anywhere, no compatibility flag on SIGEL's own code. This moves §10's "drop the Qt 2 emulation" from optional debt into a **required phase**, and with it the data migration that section describes — the shim exists because `Q2Dict`'s hash order numbers the links, so the 7 `.rrb` and 12 `.exp` files must be rewritten before it can go. **Ordered before Phase C**, so the GUI sites (466 as counted then, **534** re-measured at C1 — §7) are ported once, to the final target, instead of twice. Vendored third-party code is out of scope for this rule: qhull, cv97, Dynamo and PVM keep `-w -fpermissive` |

## 5c. Decisions — signed off 2026-09-02, from driving the interface

| # | Decision | Answer |
|---|---|---|
| **D27** *(decision; §10 also has a **step** D27, the shim deletion — the two D-series overlap and this is the first collision)* | The duplicate MetaGP `A&bout` | **removed**, with its trailing separator. Present in 1.3 and verified there; wired to the same `slotAbout()` as `Help > About` and opening the identical `SIG_InfoBox`. The port's first intentional difference from 1.3. `Help > About` untouched |
| **D28** | The `QSpinBox` over-range divergence (C11a) | **accepted, not fixed.** 1.3 accepts out-of-range digits and clamps on commit; the port refuses the keystroke and commits a truncated prefix. It is reachable **only by typing a number outside the box's own range**, and the differing value is **visible in the box** before anything is saved — 1.3 shows 99, the port shows 10. Contrast what the port did fix: `clear()` killed the application, the ampersand rendered wrong, a negative width silently wrote no file — all reachable with valid use. The fix is not the 33 lines of it, it is **owning a custom widget forever**: every future form edit and every new spin box must remember `SIG_SpinBox` or silently opt out. Pinned in `guibehaviour-baseline.txt` (`commits=`) so it cannot drift; prototype and the measured comparison in `future_refactorings.md`. **Revisit if** a dialog spin box turns out to feed something unvalidated, or if anyone actually hits it |
| **D29** *(signed off 2026-09-04)* | Changing run parameters **while an evolution is running** | **FORBIDDEN in the port, whatever 1.3 permits.** The user's reason is the specification, not 1.3: *"that's not how GAs/GPs are commonly implemented"* — the parameters define the run. **The port's second intentional divergence**, after D27. Implemented as a guard inside `SIG_Experiment::putAllIntoExperiment()`, NOT on the widgets, because a widget guard does not cover the path that matters: `SIG_ExperimentListView::slotSelectionChanged` ends with an **unconditional** `putAllIntoExperiment()` (`:323`) — two lines after it has already asked `manager->running()` for a different purpose — so a **page switch** can push widget state into a live run. *1.3 does disable the five parameter pages during a run (`SIG_Experiment.cpp:275-279`), so no user-typed value can reach that path today; the guard makes the property structural rather than incidental and covers any future caller.* **The obvious predicate could not have worked:** `SIG_GPManager::running()` is a 2003 stub that returns `false` unconditionally (`SIG_GPManager.h:115`) and is overridden nowhere, so a guard written against it would never fire — the flag is explicit instead. Ordering is load-bearing: `slotStartEvolution` calls `putAllIntoExperiment()` **before** `gpManager->start()`, so the settings a user chose are still committed at start and only later writes are refused. **Out of scope, deliberately, and flagged rather than decided:** `allIndividualsView->setEnabled( false )` is **commented out in the 2003 source** (`:280`), so the Individuals view stays live during a run. Individuals are the population, not parameters, so this decision does not touch them — but it is the one place a user can still act mid-run. **COVERAGE, stated plainly: only half of this is gated.** The "does not over-block" half is — `pagesave vs 1.3` and `pages` both drive `putAllIntoExperiment()` on the normal path and would fail if the guard refused a legitimate commit, and they pass. The "**does** block during a run" half is **verified by inspection only**: exercising it needs a live evolution, and the evolution scenario is not in the gate (every shipped experiment terminates on a 2001 date, so a correct Start returns in under 100 ms, and a run long enough to switch pages during takes minutes). Add it to the `evolution` scenario when that is next touched |

---

## 5a. Decisions — signed off 2026-08-22, for Phase R

| # | Decision | Answer |
|---|---|---|
| **D15** | Where the robot models come from | downloaded from `sigel.sourceforge.net`, §9. Untracked, in `data/` |
| **D16** | The Dynamo branch in `SIG_Simulation.cpp` | ~~build Dynamo, SOLID and qhull~~ **superseded 2026-08-28.** `physics_backends.md` was decided and executed: the branch and its 13 file pairs are deleted, `SIMULATIONLIBRARY 0` now fails loudly, and the Dynamo archive is cut to the maths objects. SOLID and qhull went with it — no `libsolid.a` is built; only the include path survives |
| **D17** | Build system | **plain `Makefile`** at the repo root. Phase C can bring its own for `moc` and `uic` |
| **D18** | What "runs clean under ASan" means for R3 | ASan and UBSan errors are pass/fail; LeakSanitizer output is a recorded baseline, because §10's leaks are out of scope |

Established while answering these:

- **`autoDelete` splits 38 owning / 9 non-owning**, all literal `TRUE`/`FALSE`,
  which is what makes D7(a) a lookup rather than a judgement.
- **Non-ASCII exists only in comments** — 46 of 380 files, 119 lines, no string
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

**Phase P departed from this, on instruction.** No `step-P*` tags exist — the
session owner asked for none, so the last tags are `step-R1` and `step-A9`. P1's
commit is not prefixed either. And each step has **two** commits, not one: the
step, then the fix for what its review found. Every round found something, so
folding the fix into the step would have hidden it. Eight commits, `ee981a1`
through `f0f2daa`.

---

## 7. Steps

**Exit criterion per step:** `./check.sh` at the repo root — **845 pass, 0 fail,
508 warnings** as of 2026-09-03, after §9 items 5 and 2 and the clipped-control gate, and it now **exits non-zero** when
anything fails or is skipped. Zero is reachable because the two permanently
Windows-only `WIN_*` files are an explicit exclusion rather than a standing
red — see C11c.

**Earlier pass/fail figures are not comparable and have been removed.** The basis
changed at C4 (standalone headers folded into the totals, plus the regex self-test
and the parsers check), and again when the Dynamo backend was deleted. The current
figure is the one in "Handover" below; every historical arithmetic trail that used
to sit here was superseded, and it sat at the top of the section §0 sends a new
session to. Warnings are on an unchanged basis and remain comparable throughout.

**There are four gates, not three, and the full list with its caveats is in
"Handover" below — use that one.** `check.sh` compiles every converted module
and every converted header standalone, and since C1 also runs `uic`, `moc` and
`rcc` over the converted forms; **it executes no SIGEL code**, D27 having
deleted the shim self-check that was the only step which did.
Vendored headers are `-isystem`, so their warnings do not bury the **341** in
our own code.

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
The repo-root programs are still in the hole: `sigel_eval.cpp`, `pvm_smoke.c`
and `pvm_link.cpp` are compiled only by their own targets, never by `check.sh`.
A break in them shows up as a build failure rather than a check failure.

### Phase P — PVM — DONE 2026-08-28

Upstream PVM 3.4.6 replaces the vendored 3.4.3. `libpvm3.a` and `pvmd3` build
and run; SIGEL's PVM code links against them and `SIG_GPPVMData` round-trips
through a live daemon. *This said `sigel` and `sigel_slave` "still do not link", blocked on Phase C.*
**C9 linked both and they run.** ~~What is still unreachable is the evolution
LOOP~~ — **the loop RAN on 2026-09-02**: three generations through the GUI on
this machine against the oracle's 1.3, single-slave, at 185 s/generation. §9
item 2 has the result.

```
make pvm         libpvm3.a, pvmd3         28 objects, 0 errors, 6 warnings
make pvm-link    build/pvm_link           P4's link and round trip
./pvm-check.sh   starts a daemon, runs both, PASS/FAIL, non-zero if either fails
```

#### P1 — the tree

`x/supportingLibs/supportingLibs/pvm3/` is upstream 3.4.6, 844 files where
3.4.3 had 576. `pvm3.4.6.tgz` is committed at the repo root, md5
`7b5f0c80ea50b6b4b10b6128e197747b`, identical to netlib's and to Debian's
`.orig`. It is the one tarball tracked here: netlib is the only host still
publishing it, Fedora retired PVM in 2015 and Debian removed it in 2024.

**3.4.3 could not have been patched instead: it has no `conf/LINUX64.def`.**
`lib/pvmgetarch` is not part of that — measured, *both* versions contain zero
`aarch64`, which is why the `Linux,aarch64` line is one of the four config
lines below.

**Nothing of SIGEL's was discarded.** The PVM in
`sigelSourceDistribution.1.0.tar.gz` and the one in `supportingLibs.tar.gz` are
both 576 files, and differ in **one** genuine line — a clean rule added to
`Makefile.aimk`. The other 357 differing files are CVS `$Id:`/`$Log:` keyword
expansion from SIGEL's import of 2001-11-20. `patches/` never held a PVM patch.
That compares the two *tarballs*; the untracked directory `rm -rf` destroyed
cannot be compared against, so this is provenance, not a byte-for-byte proof.

For 1.3 the supporting libraries were a **separate download**:
`kdesigelSources.1.3.tar.gz` carries four entries under `supportingLibs/` — the
directory and three Makefiles. Only the 1.0 distribution bundled them.

**The Makefile guards the swap** (`Makefile:54-83`). Re-extracting
`supportingLibs.tar.gz` over the tree restores 3.4.3 with no error and a
plausible file count, because `tar` overwrites but never deletes. The guard
reads `PVM_VER` from `include/pvm3.h` on every `make`, not from the patch stamp,
which survives exactly this accident. Confirmed in four states: right version,
everything runs; wrong version, build targets abort while `clean` and `unpatch`
still run, because those two are how you recover; header unparseable, abort;
tree absent, pass.

#### P2 — nine patches

The plan said one Debian patch. Measured against a pristine 3.4.6: the `series`
has 29 entries, 26 apply, 10 touch `.c`/`.h`, and **8 touch a file in the 28
objects** that build `libpvm3.a` and `pvmd3`. **All 8 are applied, signed off
2026-08-28**, on the rule that a maintained distribution's judgement beats ours
on a package nobody maintains. The ninth patch carries the four config lines.

The rule is "everything Debian applies that we compile", so this column is a
record, not the reason any of them is there. Line numbers are **pre-patch**.

| `patches/` file | Debian | Fixes something here? |
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

`03-new-readline-api` and `25-format-security` touch only `console/`,
`tracer/`, `hoster/` and `src/OS2/`. Neither is in the 28.

**All 28 objects compile with all nine applied: 0 errors.** `lib/pvmgetarch`
answers `LINUX64`, which is the single fact the config patch exists for.

**`make unpatch` was reversing in application order.** With `06` and `22` both
touching `pvmd.c`, undoing `06` first shifted the file and `22` came off with
`offset -2` and a `.orig` backup. It landed; a larger shift would not. It now
reverses the list, and the round trip is byte-identical to the tarball.

§3's PVM analysis measured 3.4.3 and is marked historical. Of the two defects
it said 3.4.6 already fixed, only `sys_errlist` was: `pvmlog.c:499-505` is
guarded by `USESTRERROR`, which `conf/LINUX64.def` defines. The `pvmtev.h`
include was **not** in vanilla 3.4.6 — patch 17 adds it — but all 28 objects
compile without it, so the conclusion held and the reason did not. Separately,
the research build that was reported "verified by building and running it" did
not carry patch 24, so the binary verified still had the `getcwd` truncation.

#### P3 — build and run

`make pvm` drives PVM's own build, target `s`, which is `src` only. `default`
is `s c f g` and `c` is `s t`, so it would also build the console, the tracer,
`libfpvm` and `libgpvm3`; SIGEL's 2003 link line names `-lpvm3` and nothing
else. **PVM is the one vendored library we do not compile ourselves**: its own
build works once patched, and `pvmd3` is a daemon `libpvm3` locates under
`$PVM_ROOT/lib/$PVM_ARCH`, so the products must sit in that layout inside the
vendored tree rather than in `build/`. `make clean` names them, or they would
survive it.

`LINUX64` is hardcoded because a target name expands before `patches/` is
applied. On another supported arch PVM builds into `lib/<thatarch>` and make
does not check that a recipe made its targets, so the recipe tests for the
products explicitly — without that, `make pvm` would exit 0 having produced
nothing.

```
439 KB libpvm3.a, 235 KB pvmd3
all 13 pvm_* symbols the built core leaves undefined: defined
```

`./pvm-check.sh` runs `pvm_smoke.c` against a live daemon: double, int and
string exact. **The round trip really goes through XDR** — `pvm_send` to one's
own tid does not short-circuit, measured with
`-Wl,--wrap=xdr_double,--wrap=xdr_int` at 2 and 30 calls. **And it fails when it
should:** a `--wrap` swallowing `pvm_send` gives `nothing arrived in 10 s` and
exit 1 rather than hanging, because it uses `pvm_trecv`; a `--wrap` adding 1.0
to `xdr_double` on decode gives `DIFFERS`.

**`-ltirpc` is required.** `libpvm3.a` leaves 9 XDR symbols undefined —
`xdrmem_create`, `xdr_double`, `xdr_int`, `xdr_float`, `xdr_long`, `xdr_short`
and the unsigned forms — because glibc dropped `rpc/types.h`. The plain C link
of `pvm_smoke.c` fails without it with 50 undefined references. **P4's
sanitized C++ link does not fail**, and that trap is described under P4.

**DEFECT, not fixed: `PVM_TMP` longer than 92 characters kills the daemon.**

```
pvmd.c:5066   (void)PVMTMPNAMFUN(spath);      /* char spath[PVMTMPNAMLEN=128] */
pvmd.c:5067   strcpy(uns.sun_path, spath);    /* sockaddr_un.sun_path[108]    */
```

`pvmtmpnam` (`pvmcruft.c:760`) builds `"$PVM_TMP/pvmtmp%06d.%d"`. `sun_path`
holds 107 characters plus the NUL. Measured: 92 starts, 93 aborts with
`*** buffer overflow detected ***` before the daemon prints anything.
`pvm-check.sh` refuses at 88, the worst case with a 7-digit pid and a 4-digit
counter. **None of the 26 applicable Debian patches fixes it.** Left alone
because it needs a source edit nobody upstream has made and the default
`/tmp/pvm-sigel-<uid>` is 20 characters — raise it if a real run ever sets
`PVM_TMP` somewhere deep.

*The first diagnosis named `pvmd.c:5178`, `sprintf(buf, "PVMSOCK=%s", p)` into a
`char buf[128]`, and put the limit near 110. gcc does warn about that line, and
that warning is what misled it — nothing reaches it, because 108 is the lower
ceiling. The guard built from it measured `$PVM_TMP/pvmd.<uid>`, a different and
shorter path, so every `PVM_TMP` from 93 to 109 passed the guard and killed the
daemon anyway.*

#### P4 — SIGEL against real PVM

**Linked: all 13 symbols.** `SIG_GPFitnessTrainer.o` and `SIG_GPPVMData.o` are
named on the link line rather than left to the archive, so the linker takes
them regardless. Those two are the whole of it — every object in the built core
leaving a `pvm_*` undefined, seven each with `pvm_recv` shared, verified with
`nm` over all 14 archives and all 284 objects. **The recipe asserts it**, so a
core file that later gains a `pvm_*` call fails the build by name instead of
quietly dropping out of coverage; broken deliberately with a probe in
`SIG_IO.cpp` and confirmed to fire.

**Run: 7 of the 13, through SIGEL's own code.** `pvm_link.cpp` constructs a
`SIG_GPPVMData` and round-trips a `QString` through its `sendQStringToPVM` and
`getQStringFromPVM` — its own `pvm_initsend`, `pvm_pkint`, `pvm_pkstr`,
`pvm_send`, `pvm_recv`, `pvm_upkint`, `pvm_upkstr`. The other six are
`SIG_GPFitnessTrainer`'s — `pvm_addhosts`, `pvm_delhosts`, `pvm_kill`,
`pvm_probe`, `pvm_spawn`, `pvm_upkdouble` — and they spawn and manage
`sigel_slave`, which Phase C blocks. **Those six are link-checked only.**

**Why it runs and does not merely link.** `libasan.so` exports weak
`xdr_double`, `xdr_int`, `xdrmem_create` and the rest as interceptors, so under
the sanitizers PVM's XDR references bind to those and **the link succeeds with
no `-ltirpc` and nothing behind them**. glibc keeps the same names only as
compat symbols (`xdr_double@GLIBC_2.17`) that `ld` will not bind. Built without
`-ltirpc` and run: `SEGV on unknown address 0x0, pc 0x0` in `enc_xdr_init`.

Dropping `libpvm3.a` gives 28 undefined references — not to be read against the
13: 11 are `pvm_link.cpp`'s own calls, across 17 distinct symbols.

**Cost of running SIGEL's code: one suppressed leak.** `SIG_Environment`'s
default constructor loads terrain through vendored DynaMechs and leaks 20,400
bytes in 51 allocations, `dmEnvironment.cpp:110` via `SIG_Environment.cpp:416` —
pre-existing, unrelated to PVM. `pvm-check.sh` suppresses that one function by
name rather than turning leak detection off, and prints what it suppressed.

#### FIXED — `sendQStringToPVM` overflowed on multi-byte strings

Found by P4 once it ran SIGEL's own code: the wire length was a **character**
count and the payload was UTF-8 bytes. Full entry, with the measurement and the
wire-format caveat, is in §9's "Defects fixed rather than preserved" table —
`SIG_GPPVMData.cpp:51`. **Upstream's defect, not a port regression**:
`v1.3-pristine` has it identically with `str.utf8()` and `QArray<char>`, because
Qt 2's `QString::length()` was the Latin-1 byte count. `pvm_link.cpp`
round-trips 200 `ü` plus 50 `€` as the regression test.

---

#### Phase P research — 2026-08-28, superseded by P1–P4 above

Kept for one thing only: **the flag choice, which must not be simplified.**

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
classes and leave truncation a hard error. They are not interchangeable.

*The rest of that research — its two wrong claims, and the vendored-3.4.3
alternative — is superseded by P1 and P2, which measured it.*

---

### Handover — one owner at a time

**Never two sessions on this repository at once.** Sequential is fine;
concurrent is not. On 2026-08-27 three concurrent sessions nearly corrupted a
reference capture.

This document is the handover. A new session should read §0, this section, and
the phase it is taking on.

**The `sigel-x86` channel** reaches the machine holding the 1.3 reference binary
and a working PVM. **That PVM is no longer the only one** — this machine has had
one since Phase P — so the channel's value is now the 1.3 binary alone. It is currently owned by the Qt 6 session. If it transfers,
identify yourself to it by **verifiable facts** — repo path, recent commit
hashes, a reference file you authored — never by a session name, because names
are assigned per side and neither end sees the other's.

**A failed `make` leaves the previous binary in place.** All three scripts now
run `make -q` first and refuse a stale binary, because a test for existence
passes on one. D13 hit this: its build failed on two sites, the gates were run
straight after, and both came back green against the binary from before the
change. `pvm-check.sh` had carried the guard from the start; the other two did
not. **Gate results mean nothing unless the build that produced them
succeeded.**

**Gates any session must keep green**, all committed:

```
./check.sh                                            845 pass, 0 fail, exit 0
./dictorder-dump.sh | diff -u dictorder-baseline.txt -    empty
./fitness-check.sh  | diff -u fitness-baseline.txt -      empty
ASAN_OPTIONS=detect_leaks=0 ./fitness-check.sh build      exit 0
```

**The fourth line is not optional and was missing from this list until
2026-08-29.** The default `fitness-check.sh` runs `build-fast`, which has no
sanitizer, so it **skips the self-check's leak test** — and says so on stderr,
which `| diff` discards. The self-check is what covers the evolution loop's
ownership (D16), so without that fourth line a dropped free passes everything
here.

Never edit a baseline to make a diff go away. If a change moves one, that is the
finding.

**`./pvm-check.sh` is a fourth check but not a fourth gate.** It has no baseline
— it prints PASS/FAIL and exits non-zero if either half fails. Needs
`make pvm && make pvm-link` first. Run it after touching PVM, `SIG_GPPVMData` or
`SIG_GPFitnessTrainer`; the three gates above cannot see any of them.

---

### Phase 0 — comments to English (D14)

German comments → English, removing the Latin-1 bytes. No code change.

Criterion: strip comments from before and after, diff the remainder — must be
byte-identical. Must not be interleaved with A1–A9, which are reviewable only
because they are pure renames.

Done for core. 9 GUI files remain: `SIG_SimulationVisualisationWidget.cpp`,
`MT_PopulationWidget.cpp`, `MT_StatisticsWidget.cpp`, `SIG_MainWindow.cpp`,
and 5 headers in `include/MT_GUI/`.

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

`make` at the repo root builds 205 vendored objects, 108 of SIGEL's 109 core
sources, two moc outputs and `build/sigel_eval`. Was 251 / 118 of 122 / three
until the Dynamo backend was deleted (`physics_backends.md`): 46 vendored
Dynamo `.cpp`, 13 of SIGEL's own and the `SIG_DynaSystem` moc target went with
it.
`make B=build-fast SAN= SIGSAN=` gives the same thing without the sanitizers,
in its own directory.

```
SIGEL_ROOT=$PWD/x/kdesigelSources.1.3/kdesigel/kdesigel \
  ./build/sigel_eval data/Experiments/twoBasesSimpleFitness1.exp 0
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

**Five vendored patches**, applied by `make` against a stamp inside the
untracked tree:

| patch | why |
|---|---|
| `cv97/JVector.h:29` | `remove()` is a member of the dependent base `CLinkedListNode<T>`; two-phase lookup binds it to `::remove(const char *)`. `this->remove()` |
| `cv97/CLinkedList.h:37` | the list header node is a bare `CLinkedListNode<T>`, so `(T *)` is a downcast that never holds and UndefinedBehaviorSanitizer reports it. `reinterpret_cast` |
| `dynamechs/dm/svd_linpack.cpp:180` | the inlined copy of `f2c.h` declares `struct complex`, ambiguous with `std::complex` under the `using namespace std` the pre-standard `<iomanip.h>` carried. `::complex` |
| `Dynamo/Src/Inc/containerlist.h` | `NULL` with no `#include <cstddef>`. `-fpermissive` was hiding this, which is why it is a patch and not a flag. **No longer load-bearing as of 2026-08-28**: `containerlist.h` is included only by `containerlist.cpp`, one of the 46 vendored Dynamo sources the backend deletion stopped compiling. It still applies cleanly, so it is left in place |
| `SOLID-2.0/include/3D/Basic.h:40,43` | `INFINITY` is a C99 macro from `<math.h>`; `abs(double)` is now declared in the global namespace, so SOLID's own conflicts with it. **No longer load-bearing as of 2026-08-28** — SOLID went with the Dynamo backend. Left in place because it still applies cleanly |

Two shim headers went with them: `new.h` is new (5 SOLID sources include it),
and `iomanip.h` now includes `<iostream>`, which the pre-standard header did.

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

**AddressSanitizer.** Clean, but only with one `SIGEL_ROOT` per worker.
`SIG_Environment::generateTerrain` rewrites `$SIGEL_ROOT/Terrain.ter` on **every
evaluation** and reads it straight back, so workers sharing a root read it
half-written, get a zero-size grid, and take a real heap-buffer-overflow in
`dmEnvironment::getGroundElevation`. `replicate.sh` gives each worker its own
root and treats a non-zero exit as an error; an earlier version scored crashes
as a fitness of 0, which made the headline number load-dependent.

**Leak baseline (D18): 41,254 bytes in 109 allocations** per evaluation, from
§10's pre-existing leak — `SIG_Simulation` is `new`ed and never deleted, and
its destructor is empty. Gate on ASan and UBSan errors, not on this.

### Replication — checked against the 1.3 binary since Phase V

`./replicate.sh` runs every individual of every published experiment. It is
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
- `SIG_EarlyRunTermSimulation.cpp:97` was a twelfth `QTime()` site, missed by
  the first sweep because it is a declaration rather than a call. Fixed. It made
  `getMaxRecorderSteps` return 2 instead of 182 for three fitness functions no
  shipped experiment selects.

### Phase V — check against the 1.3 binary — NEW 2026-08-27

Replication above says what is missing: **every gate in this repo compares the
port against itself.** `dictorder-baseline.txt` and `fitness-baseline.txt` both
prove that nothing changed, not that anything matches 1.3. Phase V supplies the
other side of the comparison, from the 1.3 binary on the x86 box, reachable
through the `sigel-x86` session.

**Capture once, diff for ever.** 1.3 is frozen, so its output for a given input
never changes. Each step captures a reference file once, commits it under
`verification-against-sigel-1.3/`, and every later step diffs against it locally. The x86 box is
needed **once per quantity, not once per step** — after V1 this is another line
in `check.sh`, not a remote call.

**The 1.3 binary is in this repository, and static inspection does not need the
remote box at all.** `xb/kdesigel/sigel` — ELF 32-bit i386, **not stripped**,
dated 30 April 2003. `nm` and `objdump -d` run on it locally — this host is **aarch64**, and the
tools work cross-architecture. Verified
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
| V2 | Our half: a save path in `sigel_eval`, the same round trip locally, diffed against V1. Becomes a gate. **Read V8 result 5 first** — a shipped `.exp` round-tripped through 1.3 differs from its input by ten keys, so an input-vs-pass-1 gate fails however correct the port is | equivalence instead of self-consistency |
| V3 | ~~Determinism on the x86 box — one experiment run twice, both `RANDOMSEED`s pinned~~ **SATISFIED 2026-09-02.** The oracle ran it twice on each of two models: `serA`≡`serB` and `octGateA`≡`octGateB`, identical on every field but the run-directory paths | closed. It is SAME-BOX determinism, which is sound; it was never a cross-machine claim |
| V4 | ~~Two whole-run digests validated against the reference machine~~ **DROPPED 2026-09-03.** A whole-run digest cannot cross an x87/IEEE boundary, which §7 and D26 already said and C11 then proved: the identifier counts that appear to agree are FORCED — consumed in the tournament constructors before any fitness is read, with `createTours` building a constant 50 per generation — while the contents are decided by a float comparison and cannot agree. Estimated ~98% chance the counts matched even under maximal divergence | dropped, not deferred. What replaced it is a measurement of OUTPUT on one machine: population 100, 30 generations, best fitness 0.063794 → 0.141625 — §9 |
| V5 | **MDH probe DONE 2026-08-27, PASS** — `verification-against-sigel-1.3/v5-1.3-mdh-compared.txt`. The sensor and force probes remain open | the port's **arithmetic**, which V1–V4 never touch |
| V6 | **DONE 2026-08-29, PASS, 5 of 5** — `verification-against-sigel-1.3/v6-1.3-friction-nocollide.txt` | the two Phase D paths **no shipped data exercises**: friction pairs and no-collide pairs, and whether both setters negotiate |
| V7 | **DONE 2026-08-29, 4 runs on `walker`** — `verification-against-sigel-1.3/v7-1.3-friction-nocollide-rules.txt` | the remaining rules for those two paths: multiple partners, unloaded partners, duplicates, and whether a dropped entry is resurrected |
| V8 | **DONE 2026-08-29, captured BEFORE the conversion** — `verification-against-sigel-1.3/v8-1.3-gp-blocks.txt` | `SIG_GPParameter::hostList` and `SIG_GPExperiment::experimentHistory`, the two `Q2PtrList` the gates run on every load and the next to convert |
| V9 | **DONE 2026-08-29, 3 of 3** — three function *bodies* disassembled, recorded below rather than as a capture file | whether a reworked body hides under an unchanged name. Symbol lookups cannot see that |

### V4 — the whole-run gate — DROPPED 2026-09-03

**121 lines of digest specification, provenance caveats and interpretation
rules were deleted here.** V4 proposed whole-run digests captured on the 1.3
reference machine and compared against ours. It cannot work, and the argument
it rested on — *"a match proves the interpreter, physics, genetic operators,
selection and RNG all agree in composition"* — is false in the direction that
matters.

**Why, established by doing it (§9 item 2).** The digest lines were
`NAME|FITNESS|sha256(program)`, and **fitness cannot cross an x87/IEEE
boundary** — §7 measures a 1-ULP change in start height moving it 45%. Worse,
the quantities that *do* agree agree for free: identifiers are consumed in the
tournament constructors before any fitness is read, and `createTours` builds a
constant 50 tournaments per generation, so the counts are forced by the code.
Estimated ~98% chance they matched even under maximal divergence.

**Two lessons kept from it.** Reference data that cannot be verified in this
repository is *unfalsifiable locally* — a later mismatch could be a
reconstruction error rather than a port defect, so commit the inputs or do not
rely on the digest. And a comparison surface that includes a float is not a
comparison surface across machines.

**What replaced it** is a measurement of OUTPUT on one machine, which needs no
reference at all: population 100, 30 generations, best fitness 0.063794 →
0.141625. See §9.


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

`getRandomLong` is a pure wrapper — it calls `getRandomInt` and returns it.
State is a 32-bit int at object offset 0. **The "bits 16..30" claim in the
table above is now a fact from the binary, not a second reading of our own
source**, which was the weakest evidence this file accepts.

**The widening is safe, and here is the actual reason.** The low 32 bits of a
product depend only on the low 32 bits of the operands, so bits 16..30 evolve
identically whether the multiply is done at 32 or 64 bits. Our
`unsigned long int next` is 64-bit here and was 32-bit in 2003; it does not
matter. Both riders check out against
`src/SIGEL_Tools/SIG_Randomizer.cpp:53-62`:

| rider | 1.3 | ours | verdict |
|---|---|---|---|
| shift must be logical, or sign bits reach the mask | `shr` | `next` is `unsigned long`, so `/ 65536` is unsigned division | safe **only because `% 32768` discards bits 31 and up**. If that mask ever moves or widens, the two part |
| reduction sign | `idivl`, signed | `int % int`, signed | matches. `randomNumber` is always in `[0, 32767]`, so a negative `maximum` yields a non-negative result on both |

**Seed 0 means "seed from the clock".** 1.3's `setNewSeed(0)` computes
`QTime(0,0,0,0).secsTo(QTime::currentTime())` — seconds since midnight. Ours is
the same code (`SIG_Randomizer.cpp:39-47`), and `QTime(0,0)` is explicitly
constructed, so §9's `QTime()` collision does not touch it. That much is solid,
and it is **original 2001 code**, identical in `x/sigelSourceDistribution.1.0` —
not something the port introduced.

**There are two `RANDOMSEED` keys per `.exp`, and only one of them seeds
anything.** Measured over all 14 files in `data/Experiments/`:

| key | line | value | reaches a randomizer? |
|---|---|---|---|
| `SIG_SimulationParameters` | 10 | **0 in all 14** | **no.** Its only consumer in the tree is a GUI spinbox, `SIGEL_MasterGUI/SIG_SimulationParameter.cpp:168` |
| `SIG_GPParameter` | 66 | **1 in 8, 0 in 6** | **yes** — `SIG_GPManager.cpp:52`, `randomizer( actExperiment.gpParameter.getRandomSeed() )` |

So the key that is 0 everywhere is the one that seeds nothing, and the key that
actually feeds `SIG_Randomizer` is **1** in 8 of the 14 — so "as distributed
they are clock-seeded" is false for a majority of the corpus.

**There is a genuine unconditional clock seed.** `SIG_GPPopulation.cpp:33` —
the default constructor — builds
`new SIGEL_Tools::SIG_Randomizer()`, which runs `setNewSeed(0)` and therefore
the clock, **regardless of any `.exp` key**. That is a stronger explanation for
run-to-run disagreement than the false one it replaces, and it does not depend
on the file contents at all.

**Consequence for V3.** Pinning "both `RANDOMSEED`s" is not sufficient and one
of the two is not even necessary. A determinism run has to pin the **GP** key,
and must also account for `SIG_GPPopulation`'s own randomizer, which no key
reaches.

**2. `SIG_GPPopulation::deleteIndividual` — D15 was written against the right
reference.** Outer loop from `poolpos` to `size-2`; fetch successor,
`setPoolPos(i)`, shift by `take(i+1)` then `insert(i, ptr)`, then
`resize(size-1)` after the loop. **Zero `__builtin_delete` and zero `free` in
the whole function** — counted, not eyeballed. The single free is inside the
container's `insert`, disposing of the slot's previous occupant on the first
iteration, which is exactly the hidden free §9 lists and exactly what D15's
explicit `delete pool[poolpos]` replaces.

**3. `SIG_Material::setFrictionValue` — the shape behind V6 and V7.** Iterator
walk; update in place on found, `__builtin_new` + `append` on not-found; then
the recursive call guarded by **both** the `negotiate` flag and
`otherObj != this`, passing `false` so it cannot recurse further. **Exactly one
recursive call in the function**, counted. The externally measured behaviour in
V6 and V7 and the internal shape agree.

**What V9 does not establish.** Three matching bodies raise confidence that the
`SIGEL_*` code is common between our source and this binary. They do not prove
all of it is. `MT_FitnessTranier` remains the only confirmed divergence.

**Why the round trip is the sharp test.** The `.exp` carries the robot as a
`StreamedRobot` block, and that block *is* dict iteration order —
`SIG_Robot::writeToFileTransfer` walks the six dicts to write it. Loading
inserts each entity into a `QDict`; saving iterates it back out. Qt 2's
`QDict::insert` **prepends**, so every bucket holding more than one name comes
back reversed. Load-and-save is therefore not the identity but a permutation,
and which permutation is a fingerprint of the hash. That makes the prediction
falsifiable in advance: `twoBases` has 2 links and 1 joint, so it must come back
**identical**, while `octopus` and `walker` must come back **permuted**. The
`LanguageParameters` command list rides along in the same artifact — shipped, it
reads `MUL MOVE CMP COPY LOAD SENSE SUB DIV MIN DELAY ADD MOD MAX`, which is
neither alphabetical nor declaration order, so it is hash order too.

**Why this is urgent rather than eventual.** `q2compat.h`'s hash was only ever
checked against a second reimplementation of `qgdict.cpp` by the same hand — two
readings that share one mistake agree perfectly. `Q2Dict` is already deleted
(D5), so the ordering it produced survives only in `data-reordered/` and
`dictorder-baseline.txt`, with D7 built on top. Every further step stacks work
on a foundation checked against nothing but itself, and V1 is three short runs.

### V5 RESULT — the arithmetic agrees with 1.3

**`twoBases` is exact, bit for bit, all four fields. `octopus` agrees on all 9
joints, `alpha` is bit-identical on every one, three joints are exact in all
four fields, and the worst real disagreement is 5 ulp.** No high-bit
disagreement anywhere. With V1 covering ordering, the port is now checked
against 1.3 on both ordering and arithmetic.

Exact in all four fields: `thirdLegJoint2`, `thirdLegJoint3`, `firstLegJoint2`.
Five joints within 1–5 ulp, one near-zero residual.

The call counts came out **2 and 18** — the corrected figures, each real call
preceded by `dmMDHLink`'s own constructor call of
`setMDHParameters(0,0,0,0)`, alternating without exception, so nothing was
filtered.

**The one substantive disagreement is a near-zero residual, and it points our
way.** `secondLegJoint2`'s `d` should be zero: ours is exactly
`0000000000000000`, 1.3's is `3ca6a09e667f3bcd` = 1.57e-16. The 2003 build
carries the crumb, from an x87 80-bit intermediate failing to round to zero the
way an IEEE double does. That direction was predicted in advance, which is what
makes it confirmation rather than a defect.

**The row matching had to be redone, and the reason generalises past this
project.** The first table came from a **greedy minimum-total-ulp assignment**
across all four fields — described at the time as "matched by `alpha`, which is
unique per joint", which is doubly wrong: `alpha` is shared by **6 of octopus's
9 joints**, and the matcher was not using it.

A minimum-difference assignment **optimises for the quantity the comparison
exists to measure**. It picks whichever pairing makes the two sides agree best,
so it cannot report a large disagreement even when one is there. It is circular,
and it fails silently and in the flattering direction.

Rematched by call order — which both sides emit natively and which V1 already
gates independently — two attributions swap and the near-zero residual proves to
be on one joint rather than two. **And three joints are exact rather than one:**
the mis-assignment had spread two exact rows onto their neighbours, so the
circular method also made the result look *worse* than it was. The verdict never
moved; the per-joint table did, twice.

**Match reference data by a key something else already checks — never by
minimising the difference under test.**

**Open on the 1.3 side, reported as open rather than glossed:** the `applyForce`
probe armed and took zero hits in a session where the MDH breakpoint fired 18
times, so the harness was live and the non-hit is real. **Explained 2026-08-28,
by the Dynamo deletion:** `SIG_DynaDrive::applyForce` has exactly one caller,
`SIG_DynaMoCommandInterface.cpp:54`, on the Dynamo path — and all 14 shipped
experiments carry `SIMULATIONLIBRARY 1`. The breakpoint was on a function the
run could not reach. `senseJoint1/2` are the same case and were never
attempted; both probes need re-pointing at the DynaMechs classes. `walker` has
not been run.

**What made V5 work, after two failed routes.** Under woody's loader there is no
exec event at all — `ld-linux.so.2` maps `sigel_slave` and jumps to it, so every
breakpoint gdb places before start-up is silently never inserted, which is why
`main` never fired either. The fix is to let the slave start and **attach to the
running pid**. Then, because `-visualize` builds the simulation during start-up,
attaching after the window appears has already missed it: **stop, then play**
forces a full reconstruction. A live `pvmd` is required or the slave exits at
once.

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
the **first save only**, then holds. So **a gate comparing a shipped file
against its own round trip fails no matter how correct the port is.** Compare
pass 1 against pass 2, and still normalise trailing whitespace inside `HISTORY`
blocks. This also closed a discrepancy in V8's own arithmetic: pass 1
decomposes as 17 + 2 + 102, where section 4's 102 is 100 history lines plus 2
one-off; passes 2 and 3 are +100, all history.

Stable from the input onward, and therefore safe to gate on directly:
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

**10 of 10 container orders match, 0 differ**, plus the two containers V1
discovered that this plan did not model. Compared: 1.3's *saved* order against
the `loaded` order our `Q2Dict` produced, recovered from the pre-flip baseline
at `7f7410a:linkorder.txt`. 105 entities across `octopus` and `walker`.

| container | result |
|---|---|
| `octopus` joint, drive, sensor, material, link | **match** |
| `walker` link, material, joint, drive, sensor | **match** |
| `walker` `leg1` axis points — a **ninth** container, inside the Link line | **match** |
| `octopus` body emission order — a **tenth** | **match** |

**The prediction held in advance**, which is what makes this evidence rather
than agreement: `twoBases` came back with its robot block byte-identical, and
`octopus` and `walker` came back permuted. **This is the first thing in this
repo checked against something other than itself.** Until now `q2compat.h`'s
hash had only ever been checked against a second reading of `qgdict.cpp` by the
same hand, and D2–D7 are all stacked on it.

The **round trip is an involution** — an order-2 permutation, exactly what
bucket reversal on prepend predicts. It also independently confirms D2's finding
that the 14 `.exp` needed no migration.

**Two containers this plan never enumerated.** The per-link axis points inside
each `Link` line permute independently — 8 of `walker`'s 19 links and 5 of
`octopus`'s 10 — and so does the Body/Geometry emission order. Both are already
in `dictorder-baseline.txt` and both match, so nothing is owed; the count of
"eight order-carrying containers" was wrong and is **ten**.

**Corrections to this section from the run, both ours:**

- **The dead 2003 paths differ per experiment.** `twoBases*` use
  `/home/pg368b/ross/projects/sigel`. `octopus*` and `walker*` use a different
  2001 author — `/home/pg368/sawitzki/sigel`, note `pg368` not `pg368b` — plus
  a separate model directory, `/home/pg368/sawitzki/octopus/` and
  `.../walker/`, on the `Body` lines. The trap note below covered the
  `twoBases` family only. One of those occurrences is *inside* the
  `StreamedRobot` block, so repointing necessarily edits what is being
  compared; V1 diffed edited-input against output for that reason.
- **`twoBases` is a control for the robot containers only.** Its command list
  permuted too, because that dict holds 13 entries whatever the robot's size.

**Also learned, and worth not rediscovering:** SIGEL **traps SIGTERM**, so only
SIGKILL stops it — and SIGKILL means `SAVEEXIT` never runs and there is no
output file, so a kill-based fallback yields nothing. All three shipped
populations are already fully evaluated with a 2001 termination date, which is
why load/save exits immediately. And with no `pvmd` running, every
`pvm_addhosts` fails and SIGEL loads, saves and exits 0 regardless — evaluation
impossible rather than merely disabled, which made V1 cleaner than specified.

### V5 method — settled before the first probe, 2026-08-27

`gdb` was installed on the x86 box on 2026-08-27, so V5 is no longer
conditional. Four decisions, made once so they are not re-argued per probe:

- **Function boundaries only.** The binaries carry `.symtab` but **no DWARF**,
  so there is no type information. A breakpoint can read arguments off the
  stack and a return value out of a register; it cannot print a C++ object
  field by field or walk a container. Every probe must therefore pick a
  function that carries the wanted value across its own boundary as a plain
  `double`.
- **Compare raw IEEE 754 bits, never decimal.** i386 from 2003 against aarch64
  from today: a decimal rendering can hide a last-bit difference, or invent one
  that is not there. Both sides print the 8 bytes.
- **Report by name, not by index.** Keying each value to its entity name makes
  the probe independent of container order, which is what lets V5 run alongside
  V1–V4 instead of behind them.
- **The 2003 binary uses Qt 2's own class names** — `QVector`, `QList`,
  `QDict` — not our renamed `Q2PtrVector`, `Q2PtrList`, `Q2Dict`. Searching its
  symbol table for our spellings finds nothing.

Three targets, best first:

| target | value | why |
|---|---|---|
| `dmMDHLink::setMDHParameters` | 4 doubles per joint, at setup | computed from the robot geometry by short arithmetic at `SIG_DynaMechsSimulationData.cpp:397` and passed in at `:442`. Nothing integrates, so no tolerance is arguable. A DynaMechs symbol, not a `SIG_` one |
| ~~`SIG_DynaSensor::senseJoint1` / `senseJoint2`~~ | returns one `DL_Scalar` | **INVALID, 2026-08-28.** Called only from `SIG_DynaMoSimulationQueries.cpp:45,47` — the **Dynamo** path, which no shipped experiment selects and which is now deleted. Re-point at `SIG_DynaMechsSimulationQueries` |
| ~~`SIG_DynaDrive::applyForce(double, double)`~~ | 2 doubles | **INVALID, same reason.** Called only from `SIG_DynaMoCommandInterface.cpp:54`. This is why the probe took zero hits — see the V5 result below |

Not a target: `SIG_DynaMechsSimulationQueries::sense(int, …)` writes into a
register object rather than returning a value, which is awkward with no type
information. `senseJoint1`/`senseJoint2` give the same number more cheaply.

**In progress:** the first probe is `twoBases` — 2 links and 1 joint, so
exactly one call and no ordering ambiguity — proving the method before it is
pointed at a robot with 18 joints.

**When comparing, compare name→value, not whole lines.** A naive
`sort | diff` reports false differences, because a `Link` line's text changes
when its own internal point order permutes.

**V1 has a trap.** `SAVEEXIT=1` rewrites the `.exp` in place, so every run works
on a copy — never on the pristine download. The 2003 paths listed above have to
be repointed first.

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
| T1 | `tools/Dockerfile.qtmig` + `tools/qtmig`: Qt 4.8's `uic3` and `qt3to4` over this repo | **done** |
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

**REBUILT 2026-08-27 by review.** The previous C1–C9 were API-wide sweeps
across all five modules — exactly what D23 forbids — and their Sites column was
§2's tree-wide count, which includes sites Phase A already converted in core.

**RE-MEASURED 2026-08-30 AT C1, AND THE PATTERN IS NOW STATED**, because the
container row did not reproduce and neither did one other. The old figure was
"466 Qt 2 sites, measured 2026-08-27 over the 5 GUI modules, 117 files,
Latin-1 safe", which names the scope but not the unit — and the unit is where
both errors are.

- **Scope:** the 5 GUI modules, `src/<m>` and `include/<m>`. **109 `.cpp`
  and `.h`** at HEAD, 107 before C1. *The old line said "117 files", which is a
  third referent again: 117 is every file including the 10 `Makefile.am`, and
  counted before C1 added two. The `Makefile.am` match nothing, so no figure
  moves — but the one line whose job is to state the referent had an unstated
  one.*
  `SIGEL_RealInterface` is a sixth GUI directory that is in **neither** this
  table nor §2's module list; it contributes **0** to every row, so it is not
  the gap it looks like. D27 counted it and got the same numbers.
- **Tool:** `command grep -a` under `LC_ALL=C`. Plain `grep` wraps `ugrep -I`
  and silently skips the Latin-1/CRLF files — §2 says so and it still bites.
- **Unit: occurrences, not lines.** 8 of the 11 rows agree with occurrences and
  disagree with lines, so that is the table's convention.
- **`code` excludes a line whose only match is inside a comment**; `raw` counts
  everything. Both are given because the difference is 17 sites and one row is
  two-thirds comment.

| category | code | raw | |
|---|---|---|---|
| `QListView` / `QListViewItem` / `QListViewItemIterator` | **121** | 128 | D9 → `QTreeWidget`, `QTreeWidgetItem`, `QTreeWidgetItemIterator` |
| containers — 9 Qt 2 types, **22 files** | **73** | 79 | `QArray` 21, `QDictIterator` 15, `QList` 14, `QListIterator` 10, `QVector` 5, `QDict` 3, `QValueList` 2, `QCString` 1, `QQueue` 1 — as **lines**, 72; one line carries two `QList`. *Was **75***. Owned by no step but C3–C7 |
| `insertItem` | 66 | 66 | menus, combos, list boxes |
| `WFlags` / `WType_*` / `WStyle_*` | 63 | 63 | → `Qt::WindowFlags`, `Qt::WA_*` |
| **`QIconSet`** | **47** | 48 | → `QIcon`. **Counted by no row until now** — the single largest omission, 6 files |
| `QString::null` | 41 | 42 | → `QString()` |
| `QPopupMenu` | 40 | 40 | → `QMenu` |
| `qApp` / `setCaption` / `QApplication` | 22 | 22 | |
| `setAutoDelete` / `autoDelete` | 16 | 17 | Owned by no step until C3–C7. The 17th is a comment this port wrote |
| **`QWidgetStack`** | **10** | 12 | → `QStackedWidget`. New row |
| **`QListBoxItem`** | **10** | 10 | → `QListWidgetItem`. New row |
| Qt 2 container `#include`s | 9 | 9 | `<qarray.h>` and friends. They carry no type name, so no row above counted them, and every one has to go |
| `sprintf` 4, `.lower`/`.findRev`/`.upper` 3 | 7 | 7 | reproduces **only** call-shaped: the bare words give 12, mostly prose |
| **`QButton`** | **3** | 3 | → `QAbstractButton`. New row; all 3 are `QButton::On/Off/NoChange` |
| `QGLWidget` | **2** | 6 | → `QOpenGLWidget`. *4 of the 6 are comments; the code is one base-class clause and one constructor initialiser, both in `SIG_VisualisationWidget`* |
| **`QTextView`** | **2** | 2 | → `QTextBrowser`. New row |
| **`QScrollView`** | **1** | 1 | → `QScrollArea`. New row |
| **`QWMatrix`** | **1** | 1 | → `QTransform`. New row |
| `QMultiLineEdit` | 0 | 0 | all 6 are in `.ui` files, not in module code |
| **total** | **534** | 556 | *was **466***, and that total was the sum of the rows rather than its own measurement |

**HOW THE ROWS WERE FOUND, because patching a row at a time is how this got
wrong twice.** Every `Q[A-Z]…` token the 5 GUI modules name — 85 of them — was
diffed against every class Qt 6's `QtCore`/`QtGui`/`QtWidgets` headers declare.
What Qt 6 does not have is by definition Phase C work, and that is the table
above. It found **8 classes no row had**: `QIconSet` 47, `QListViewItemIterator`
15, `QWidgetStack` 10, `QListBoxItem` 10, `QButton` 3, `QTextView` 2,
`QScrollView` 1, `QWMatrix` 1 — **89 code sites**, a fifth of the total, none of
them mentioned anywhere in this plan before C1. *This sweep is the reason the
total moved up rather than down.* It cannot see the silent collisions —
`QList`, `QVector`, `QListIterator`, `QQueue` all still exist in Qt 6 meaning
something else — but those are exactly what the containers row already covers.

**The original 128 was right; a "correction" of it to 106 was the error.**
Measured as a **substring** over exactly the stated scope, `QListView` returns
**128** on the pre-C1 tree — `QListView` 18 + `QListViewItem` 93 +
`QListViewItemIterator` 17. A `\b…\b` pattern silently drops the 15 live
iterator sites, and refuting the row with that pattern is §9's characteristic
failure. **Match the pattern to the question before concluding a count is
unreachable.** (Today the substring count is 129: C1's own base class carries
the word in a comment.)

**`75` still does not reproduce**, and that half stands — verified again by the
review independently. The GUI tree is **byte-identical** to commit `ba25643`,
which wrote this table, checked by extracting `src/` and `include/` there with
`git archive` and counting: the same 62/63 for the seven listed types, 78/79
with `QDictIterator` and `QQueue`. So it is not drift; no scope or pattern
yields it.

**D27's `71 lines` is right and its breakdown is not.** It gives `QList 15`
where that type is 14 lines and 15 occurrences, so its own column sums to 72
against a stated total of 71. It also omits `QCString` from its type list
entirely. Lines, all 9 types, comments excluded: **72**.

**The 89 container and `autoDelete` sites are no longer the largest thing this
plan was missing** — 73 and 16 as re-measured above, plus the 9 `#include`
lines. The 8 classes the sweep found, `QIconSet` at their head, are 89 more.
**No tool touches any of them**, which is what makes this row the work: D19a
measured that `qt3to4` leaves Qt 2's `QList`/`QDict`/`QArray`/`QVector`
completely untouched, and C1 confirms `uic3` is a *form* converter that never
reads a `.cpp` at all. Phase B covers the 14 *core* containers; D6/D7 were decided for
core *with the shim*, which D19 does not extend to the interface. They are now
part of each module's step below, and D19a has to be settled first.

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
| C9 | **DONE 2026-08-31.** All five GUI modules build as archives, both programs link and run. Exclusions lifted, moc derived from source, resources named on the link line, `programs` gate upgraded from compile to link+run | 5 modules, 2 programs |
| C10 | **DONE 2026-09-02.** Driving the interface rather than reading it. `guidrive.cpp` posts real Qt mouse, key and context-menu events into the real `SIG_MainWindow`; the 1.3 oracle drove the 2003 binary with XTest and the two were diffed. Found the `clear()` signal regression that killed the application on a large delete, and the eaten ampersand in the MetaGP dialog. New `gui behaviour` gate with `guibehaviour-baseline.txt`. **No X-level click was possible on this machine and the section says so.** Also carries the port's FIRST deliberate divergence from 1.3 — the duplicate MetaGP About, removed by decision 2026-09-02 | 2 defects, 15 scenarios, 1 divergence |
| C11a | **DONE 2026-09-02.** The five View pages C10 never opened. 29 spin boxes, 20 sliders, 7 combos, 4 checkboxes, 8 radios and 20 of C7's 21 validators driven and diffed against 1.3. The **12-probe validator battery matches character for character**, on both sides under a comma-decimal locale the oracle built with woody's own `localedef`. Nine parameter values typed on the pages come out **byte-identical** in the saved `.exp` across the two architectures. Found the `QIntValidator` Intermediate/Invalid trap: 1.3 clamps a typed over-range number to the maximum, the port commits a truncated prefix -- **accepted as a divergence, D28**; pinned in the gate. `gui behaviour` grew from one scenario to two here, and to five by C11c | 1 regression, 2 interlocks, 3 probe errors |
| C11b | **DONE 2026-09-02.** The Import/Export round trips — 15 of the 16 children C10 never drove. **Seven of the eight exports are BYTE-IDENTICAL to what the 2003 i386 binary writes**, including the 2.7 MB `.pop` and, unexpectedly, the `.dat` with its 532 lines of floating point. Found and fixed a real defect: default-constructed language parameters came out alphabetical where 1.3 gives `QDict` hash order — predicted from `qgdict.cpp`, confirmed on the running binary character for character, and it had been silently wrong in `dictorder-baseline.txt` for all 7 robots (**98 lines corrected**). Confirmed and preserved 1.3's ignored overwrite prompt. Also fixed a C10-era harness bug that had been handing SIGEL the wrong filename — 2 exports out of 32 in the runs that caught it | 1 defect fixed, 1 baseline corrected, 1 defect preserved |
| C11c | **DONE 2026-09-02.** The six dialogs, plus two fresh-eyes reviews of the harness and of `check.sh` itself. **C7's 21st validator driven, closing that set at 21 of 21.** Found and fixed a defect with a data consequence and no need for invalid input: Qt 6 selects a pre-filled field when a dialog gives it focus and Qt 2 did not, so a user who types one digit into Add-individuals gets **12 on 1.3, which appends, and got 2 here, which replaced** — twelve individuals added where two were meant. Four sites, `end(false)` queued after show. A second divergence **kept on purpose**: allowing a command appends where 1.3 hash-inserts, predicted and confirmed character for character, but matching it would mean reimplementing what Phase D removed. Two reviews then went at the checking machinery: **five probes that could not fail**, including a Cancel test that never pressed Cancel and a round trip that was an identity test; and a **demonstrated false pass** — all 30 menu and toolbar icons replaced with garbage, whole gate green. `check.sh` also never exited non-zero, counted a skipped section as 0 fail, and ran its locale check under a locale that is not installed here. All fixed; **842 pass, 0 fail** for the first time, with `WIN_*` excluded explicitly as the permanent known failure it is. A `!!` failure marker had already reached the committed baseline; the gate now refuses those | 1 defect fixed, 1 kept, 5 probe defects, 7 gate defects |
| C11d | **DONE 2026-09-02.** `MT_GUI` — the MetaGP window, 23 sources, which nothing had ever opened on either side. Its six pages driven. **C7 never reached MT_GUI's ten validators**: an unpinned `QIntValidator(0,1000)` calls `"1,000"` ACCEPTABLE under en_US while `toInt()` returns 0, so a user types one thousand and **zero** reaches the system. 1.3 rejects both separators — measured — so the fix RESTORES Qt 2. Applied at both creation sites in `MT_AddConstantsWidget`, whose type radio rebuilds them. The window is application-modal, proven with a control (0 pixels vs 4397). The `!!` guard added in C11c caught a probe defect before it could be baselined | 1 defect fixed, 2 probe defects |

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

#### C1–C11d — what each step found, and what it left behind

**COMPRESSED 2026-09-03, from ~2,640 lines.** Every step is DONE and its
conversion is enforced by a Qt 6 compiler that no longer accepts the Qt 2
alternative — a narrative of *how* a `QListViewItem` became a `QTreeWidgetItem`
cannot be acted on. What follows is what is still actionable: the defects, the
divergences, the gates, and the traps that will bite the next person who edits a
`.ui`, adds a validator or writes a probe. **The TRAP section and the five "did
NOT exercise" blocks below are verbatim; everything else is a table row or a
paragraph.** The step table above is the index; §9's hazard list is the other
half of this.

*Cut, deliberately: per-step API mapping tables (the compiler owns them),
per-step warning arithmetic and gate totals (§7 and the top of this file carry
the current figures, and this file's own rule is not to quote one from a step
write-up), and the record of corrections made to earlier drafts of paragraphs
that no longer exist. Deleting a paragraph retires its correction with it.*

##### The defects Phase C found

Fifteen, all fixed unless the last column says otherwise. **Not one was found by
compiling** — the column that matters is *how*.

| # | defect | how it was found |
|---|---|---|
| C2 | `QButton::stateChanged(int)` does not exist in Qt 6; string-based connect compiles and fails at run time, so the floor-selection UI silently stops responding | sweeping all 49 form connections against Qt 6 |
| C4 | **`QAction::activated()` is `triggered()` in Qt 6** — nine dead connects in two files, plus `QComboBox::activated(const QString&)` → `textActivated`. The slave's toolbar drew six buttons and **not one did anything** | tripping over two, then resolving every signal signature in the tree through `QMetaObject::indexOfSignal`: **nine kinds are dead, 100 connects**. §2 has the table; `$DEAD_SIGNALS` gates it at baseline 0 |
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
| C11b | Default-constructed language parameters came out **alphabetical** where 1.3 gives **hash order**. Reachable through `Import > Robot` and `New Experiment`, neither on a load path, so no shipped `.exp` exercises it. **It had also corrupted `dictorder-baseline.txt` for all 7 robots — 98 lines** | **predicted** from `qgdict.cpp` before asking, then confirmed on the binary character for character. §7's rule is why the baseline was wrong: *every gate in this repo compares the port against itself* |
| C11c | **A pre-filled field that 1.3 does not select and Qt 6 does.** SIGEL does `setText()` then `setFocus()`; Qt 6's `QLineEdit` selects on focus down the tab chain and Qt 2's did not. Edit Command `0.01` + `5` gives `0.015` on 1.3 and `5` here; **Add Individuals `1` + `2` gives 12 on 1.3 and gave 2 here — twelve individuals added where two were meant** | driving it. **The third needs no invalid input and leaves nothing wrong-looking behind** |
| C11d | **C7 never reached `MT_GUI`'s ten validators.** No foreign locale needed: an unpinned `QIntValidator(0,1000)` under `en_US` calls `"1,000"` ACCEPTABLE while `"1,000".toInt()` returns **0** — a user types one thousand and zero reaches the system | opening the window. 1.3 rejects both separators, so the fix **restores** Qt 2 |

##### The divergences from 1.3, all deliberate

**This is the list to check before calling anything a regression.** Everything
else that stops matching 1.3 still needs justifying as a defect.

| divergence | why it is not fixed | pinned by |
|---|---|---|
| **`QIntValidator` out-of-range clamp — D28.** 1.3 clamps a typed over-range number to the maximum; the port commits the truncated prefix | reachable only by typing outside the box's own range, and the differing value is visible before anything is saved. The cost is not the code — a 25-line `QSpinBox` subclass in `future_refactorings.md` reproduces all four readings — but **owning a custom widget forever**, promoted across 29 spin boxes in three forms and then the dialogs | the `commits=` value in `guibehaviour-baseline.txt`. **Measured 2026-09-03: 44 `spin` lines, of which 40 carry a committed value — 29 on the five pages and 11 more in the MetaGP block, which C11d added after C11a wrote the TRAP.** Four are not pinned: three survey-only lines (214, 1347, 1399) and one reading `commits=not-pressed(in a dialog)` (1404). *The TRAP block below still says "the other 18 … are not pinned". It is preserved as C11a wrote it and is out of date twice over — see the correction beneath it — and **the 18 does not reproduce as any real population either**: the 20 forms declare 46 spin boxes, 29 of them exactly the three page forms, so the non-page count is 17 in the forms and 20 counting the three declared in code. Do not carry the 18 forward.* |
| **The duplicate MetaGP About is removed**, with its trailing separator. 1.3 has 43 menu items, the port 42 | a decision, 2026-09-02 — the port's first intentional behavioural difference. Not a defect, not a regression | `guidump-baseline.txt` |
| **`.lap` export order.** 1.3 does not round-trip its own order — `Q2Dict::insert` prepends, so re-inserting in iteration order reverses every colliding chain (three pairs and one triple; eight of thirteen commands move). The port is the stable one | §10 already says load and save become order-preserving once the shim goes. Restoring it means reimplementing `Q2Dict`'s bucket permutation on read — writing the defect back in | measured on the running binary, C11b |
| **A newly-allowed command appends** where 1.3 inserts at its **hash position** (allowing JMP lands it sixth, straight after LOAD, on 1.3) | the insertion **point** depends on runtime hashing, so matching it means reimplementing what Phase D removed. *Distinct from the `.lap` case, where the port had a free choice of a **static** order and 1.3's was reproducible for nothing — so that one was fixed* | C11c |
| **The movie records frame N where 1.3 recorded frame N−1.** Every caller runs `makeTimeSteps(n); update();` with the grab **inside** `makeTimeSteps`, so Qt 2's `grabWindow` captured the last *presented* frame while the simulation stood one step further; `grabFramebuffer()` renders current content | forced, not chosen — Qt 6 removed `QPixmap::grabWindow`, and for a `QOpenGLWidget` it was wrong anyway, reading on-screen pixels where the widget renders into an FBO. **Raster path only**; `fileFormat == "pov"` is a different branch, and per the oracle the POV export produced every published film | nothing can see it |
| **HiDPI movie frames are resampled.** `grabFramebuffer()` returns device pixels; 1.3's frames are logical | normalised back with `scaled(size(), …)` + `setDevicePixelRatio(1.0)`. At ratio 1.0 the branch does not run and the bytes are unchanged | — |
| **The Motif look is gone.** Qt 6 ships exactly two styles here (`QStyleFactory::keys()` → `Windows, Fusion`); the X11 branch takes Fusion | unreproducible; recorded rather than papered over | — |
| **`-geometry` and `-title` are gone from Qt entirely**, so `setMainWidget`'s third job cannot be restored | unreproducible. `quitOnLastWindowClosed` covers the "closing ends the app" half | — |
| **Descending-sort tie order** on tied Age/Fitness values. Qt 2 sorted ascending with `qsort` then **reversed the sibling list**; Qt 6 inverts the comparator inside a *stable* sort | `qsort` is not stable, so Qt 2's tie order was never defined in the first place — reproducing it means reproducing a specific libc | — |
| **Two validator inputs**: Qt 2 ran `strtod` and required full consumption, so `" 9.81"` and `"0x10"` (glibc hex float = 16) were Acceptable; the pinned Qt 6 validator returns Invalid | matching `strtod` exactly means a hand-written validator on all 21 sites. **The hex case cannot be reproduced regardless** — Qt 6's `QString::toDouble("0x10")` *fails*, so accepting it would store 0 where 1.3 stored 16 | C7 measured all thirteen probe strings; the rest agree |
| **The Save dialog pre-selects** where 1.3 pre-fills and leaves the text unselected, so typing appends | neither dialog is built by SIGEL — both are `getSaveFileName` — and §7 already lists file-dialog behaviour among the accepted divergences. **Rename, which SIGEL does build, pre-selects in both** | — |
| **Icons whose two pixmaps straddle the chosen size are resampled.** Qt 2 named Small and Large explicitly and blitted each at its own size; a Qt 6 toolbar has one `iconSize` and picks from the `QIcon` by pixel size | removing it means splitting every `QIcon`. Sizes were re-measured across every referenced XPM — **five distinct sizes each** — and set to 25×25 / 48×48, the measured maxima, so nothing is enlarged past what 1.3 drew | — |
| **Clicking the outer edge of a ticked slider pages on 1.3 and does nothing here.** Qt 2's Motif slider treats the WHOLE widget as clickable — the oracle got a clean page step at all twenty of `yawSlider`'s cross-axis offsets — where Qt 6 honours the groove sub-rect only: on `yawSlider`, y=3,5,7,9 page it and y=1,11,13,15,17,19 do not | a Qt framework behaviour rather than anything the conversion did. Nobody is likely to notice, but it is a fidelity difference | §9's C11d row, which also carries the probe lesson: **take the cross-axis from `SC_SliderGroove`, never from the widget's middle**, and populate the `QStyleOptionSlider` fully — `tickPosition` unset makes `subControlRect` return a tickless groove. *Added here 2026-09-03: the divergences table was billed as complete and omitted this one* |

**Three 1.3 defects preserved on purpose**, plus the two below them. `MT_GUI`'s
gnuplot export puts a constant x on datasets `2pt destr.` and `3pt destr.`
(pre-standard `for` scoping); `MT_PopulationWidget`'s save-individuals loop never
advances the cursor and writes the same individual into every file;
`callRenderPixMap` assigns `res` from `save()` and then `return true`
unconditionally, so a failed frame write is reported as success and the caller's
message box is dead code — **which is why that module carries a
`-Wunused-but-set-variable`**. And two that are gated: **saving grows the file**
by exactly **840 bytes = 120 × 7**, one `"      \n"` per individual per save,
because the reader takes everything between `HISTORY BEGIN{` and `}HISTORY END`
as ONE string and the writer re-emits it before a fresh terminator; and **the
overwrite prompt asks and then writes anyway**, because the second
`file.open(WriteOnly)` sits *outside* the `switch` — so answering No writes the
file and answering Yes writes it twice. The guarding line is
`sentinelSurvived=0`; a `1` would mean the port had started honouring the prompt,
**which would be an improvement and a divergence**.

##### The gates Phase C left, and the two that had no teeth

| gate | what only it can see |
|---|---|
| `forms` — seven checks a form | a dropped `<images>` block; a `Line` that lost its `orientation` and now draws nothing; a `.qrc` disagreeing with the header **in either direction**; a view enabling sorting without pinning the direction |
| `$DEAD_SIGNALS`, baseline **0 for every module** | **only the nine Qt 2 spellings §2 tabulates, and only inside `SIGNAL(`.** It is a closed regex (`check.sh:89-92`), so a **tenth** kind of dead signal is invisible to it and a `SLOT()` naming a slot that no longer exists is invisible by construction. *Demonstrated 2026-09-03: injecting `connect(…, SIGNAL(highlighted(int)), …)` — Qt 2's `QListBox` signal, which `QTreeWidget` does not have — left `Qt 6 signals` at `0 dead (baseline 0)` and every other section green, while Qt itself printed `QObject::connect: No such signal QTreeWidget::highlighted( int )` on stderr. The positive control is that runtime line: the injected connect really was dead.* C7 measured every `SLOT()` in `SIGEL_MasterGUI` against its headers by hand — 0 mismatches — and nothing repeats that |
| **`runtime connect`** — NEW 2026-09-03 | Qt's own `No such signal` / `No such slot` warning, from a driver run — the only thing that catches a dead connect the regex has never heard of. `guidrive`'s stderr had been going to `/dev/null`; it is captured now and any such line fails the gate. **It is NOT the general case the regex is a special case of, and a first draft of this row said so wrongly**: the regex is *static* over all 14 modules, this is *runtime* over only what seven master-GUI scenarios execute. **Partly disjoint, so both are kept.** What it does not reach: `check.sh` never runs the `slavegui` scenario at all, so `SIGEL_SlaveGUI`'s **44 `SIGNAL(` and 44 `SLOT(`** sites have no runtime coverage; the `clipcheck` run still discards stderr; and the two locale re-runs append to the log *after* it has been grepped |
| its **positive control**, and the reason the row above is not simply "and now it is covered" | **Qt emits that warning under the logging category `qt.core.qobject.connect`, and categories are filterable.** `QT_LOGGING_RULES='*=false'` in the ambient environment — `guidrive_run` uses `env` without `-i`, so everything passes through — or a `qtlogging.ini` silences it, and an empty stderr then looks exactly like a clean run. **Demonstrated: the full gate ran green with a genuinely dead connect injected.** So `guidrive` now makes one deliberately bogus connect at startup and the gate **fails if that warning is absent**. It must be a real connect, not a `qWarning`: `qt.core.qobject.connect` can be disabled on its own, leaving a `default`-category control visible and the check blind. *`check.sh` already says of `clipcheck` that "0 clipped" from a check that cannot detect clipping is worth nothing; this row is that sentence applied to itself* |
| **`pagesave vs 1.3`** — NEW 2026-09-03, §9 item 1 | **`putAllIntoExperiment()`, the widget-to-file path, which nothing covered.** `pages` is widget to widget; `exportall` is widget to file for the eight export formats, and **none of the three compares what the SAVE path writes**. *A first version said the export scenarios "bypass the aggregator entirely"; they do not — `putAllIntoExperiment()` runs on every tree selection change, so `gate`, `pages` and `exportall` all execute it many times. What none of them did was compare its output to a file.* Teeth-tested three ways: breaking the register write and the `WITHHISTORY` write both fail it — *but `gui behaviour` catches those too, through the `.lap` and `.pop` export checksums, so neither shows unique coverage.* **Dropping one page from `putAllIntoExperiment` does**: `pagesave vs 1.3` fails on `TEXALPHA 99 → 255` while `gui behaviour` passes 1/0. That is the gap §9 item 1 named, exactly |
| **`roundtrip`** — NEW 2026-09-03, the other half of §9 item 1 | **that a READER actually reads.** Export, change something, import the export, export again, require the two exports to match — so an importer that opens nothing, parses nothing or is never reached cannot pass. `exportall` cannot see this: it never reads anything back. **Teeth-tested by gutting each of the five readers in turn — `.gpp` `.sip` `.lap` `.env` `.pop` — and every one fails the gate.** *Gating it is what exposed that it could not fail; see §9 item 1* |
| `encodings` | a CRLF or Latin-1 file silently rewritten by an editor: 327 files, against 25 known D6 losses, 35 translated and 51 that postdate the root |
| `dead item virtuals` | a class declaring Qt 2's `key(int,bool)` without the `operator<` that replaces it. Matched against a **flattened** header and demanding the signature that actually overrides — a decoy `operator<( QTreeWidgetItem * )` and a two-line declaration both bypassed the first version |
| `widgets` | `DISpinBox` losing the fraction, under **`C` and `de_DE`** — without the second row it was blind to the locale bug the first fix introduced |
| `parsers` | a file format the program parses but no other gate opens: a `PVMHOST` round trip against `SIG_GPPVMHost`, pinning 1.3's own line. *Written because a defect of exactly that shape was found in `SIGEL_GP`* |
| `programs` | a missing moc, an unemitted vtable, a dropped resource, the wrong `SIG_GPExperiment`. It ran `-fsyntax-only` and could see none of them; it now requires both binaries built and current and runs the slave's headless smoke test, **failing rather than skipping** when they are absent |
| `gui vs 1.3` (`guidump-baseline.txt`) | an accelerator that went missing, an action that stopped being greyed, a toolbar showing the long label, a sort key that stopped being numeric. **Because the baseline was diffed against the running 1.3, a failure here is a regression against 1.3, not against yesterday's output** |
| `gui behaviour` (`guibehaviour-baseline.txt`, seven scenarios) | everything C10–C11d drove, plus a `pages` re-run under `de_DE.UTF-8` required to be **identical** to the ambient run — *a check that needs no baseline of its own, which is why it is worth having* |

**Two teeth tests were believed on a failure they did not cause, and a third
nearly was.** C6's spin-box rows: the probe's `struct P : DISpinBox` could not
reach a **private** `validate()`, so the whole `widgets` binary failed to build
and the "0 pass 1 fail" that read as the new rows biting was the build breaking —
it fails identically with and without the fix, and it silently took C6's
pre-existing check out of the run. **And the assertion could not have
discriminated anyway**: it tested `!= Acceptable`, but without
`RejectGroupSeparator` the state is `Intermediate`. C9's third teeth test
short-circuited on staleness left by the test before it, so the substituted
binary was never run while the gate still printed a failure. And C9's own "run"
appeared to abort under `timeout`, which sends SIGTERM into SIGEL's handler —
`pvm_halt()` and `exit()` from signal context with Qt threads live. **A failure
observed while the harness is what broke is not evidence about the thing under
test**, and *the check that a gate has teeth needs the same scepticism as the
gate.*

**`check.sh` itself was reviewed in C11c and was passing things it should not.**
The review **demonstrated** a false pass rather than arguing for one, which is
the only kind worth acting on:

- **Every menu and toolbar icon could vanish and the whole gate stayed green.**
  All 30 `.xpm` replaced with the text `NOT AN XPM AT ALL`: `gui behaviour 1
  pass 0 fail`. Nothing anywhere looked at an icon — `gui vs 1.3` prints
  `iconText` and `iconSize`, and the forms section covers only `.qrc`-backed
  Designer resources. **A pixmap that fails to load compiles, links, runs, and
  renders as nothing.** The `gate` scenario now dumps every action's icon and
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

##### Traps for the next person who edits a form, a validator or a probe

**How to regenerate a form's base class — RESTORED 2026-09-03, because 38
committed source files point at it.** The 19 `.cpp` say *"This is Qt 4.8 uic3's
implementation mode (`uic3 -impl`) with the Qt 6 adjustments C1 lists"* and the
19 `.h` carry the declaration-mode variant of the same sentence, so the first
compression left **38** dangling comments. *It was recorded as 19, because in
the headers the phrase is line-wrapped and a single-line grep matched only the
sources — the same pattern-versus-question error as `static_cast< uint >`.* `uic3`'s modes
and what each produces:

| mode | what it produces |
|---|---|
| `uic3 <form>.ui` | **the QWidget-derived base class.** Qt 6's `uic` emits only `Ui_<Form>`, a struct with `setupUi()`, so the class the 20 hand-written subclasses inherit from has no other source |
| `uic3 -impl <hdr> <form>.ui` | its constructor, `languageChange()` → `retranslateUi`, and a `qWarning("Not implemented yet")` stub per custom slot |
| `uic3 -convert -extract <f>.qrc` | the embedded images as real files plus a `.qrc`, and `<images>` rewritten to `<resources>` |
| `uic3 -subdecl` / `-subimpl` | the *subclass* skeleton. **Not used** — all 20 subclasses exist and are the port's actual work |

**The three Qt 6 adjustments to `uic3`'s output**, which is what those 38
comments mean:

| `uic3` emits | Qt 6 |
|---|---|
| `Qt::WindowFlags fl = 0` | `= Qt::WindowFlags()` — `QFlags` has no converting constructor from `0` |
| `: QWidget(parent, name, fl)` | `: QWidget(parent, fl)` plus `setObjectName(name)`. The name argument was `QT3_SUPPORT` and did exactly this |
| `protected slots: languageChange()` | **kept, and driven from a `changeEvent` override.** Qt 3 called it itself; Qt 6 has no such hook, so without the override the slot is dead code rather than a port |

The base class keeps the **three-argument constructor**, so no subclass
hierarchy changed. `QDialog` forms take a fourth argument — `uic3` emits
`(QWidget*, const char*, bool modal, Qt::WindowFlags)` and Qt 6's `QDialog` has
neither `name` nor `modal`, so the base does `QDialog(parent, fl)` then
`setObjectName(name)` and `setModal(modal)`. **The pair is committed, per form,
as `include/<Module>/<Form>.h` and `src/<Module>/<Form>.cpp` — it cannot be
generated at build time**: Qt 6's `uic` cannot produce it, and requiring docker
for every build was rejected.

**Forms.** Four silent-loss classes, all now gated but all worth knowing before
the gate fires. (1) **`stdset="0"` decides whether a lost property is loud or
silent** — `<property name="x">` emits a real setter, a compile error if Qt 6
lacks it; `stdset="0"` emits `setProperty("x", …)`, which compiles clean and does
nothing. The same Qt 2 property proved it both ways in one commit. **616 removed
across the 19 forms**, which found `QLCDNumber::intValue` immediately; **one must
keep it**, because `intValue` *is* a Qt 6 property whose setter is named
`display` and `uic`'s set-plus-capital convention cannot reach it. The rule is
*"stdset only where the Qt 6 setter is not named `set<Property>`"*. (2) A `Line`
whose Qt 2 form set `orientation` **alone** loses it — `uic3` keeps an explicit
`frameShape` and drops the then-redundant `orientation`, and a bare `QFrame` is
`NoFrame`: it compiles, lays out, and draws nothing. 3 of 6. (3) Qt 2's
`QListView` sorted **column 0 ascending** by default (`qlistview.cpp:1836-1837`)
where `setSortingEnabled(true)` leaves the indicator **descending** — two of the
four views C2 left unpinned call `setText(0, …)` and would have displayed
reversed. (4) A **named layout dropped from the `.ui`** still generates a header
that compiles perfectly; only a *user* of the member breaks. Also: restoring a
`<slots>` block verbatim is not enough — `uic3` normalises `slot( int )` to
`slot(int)` and drops the declaration, so pasting the Qt 2 spelling back creates
a mismatch and Qt 6's `uic` silently falls back to `&QWidget::slot`, a hard
compile error.

**The prepend trap, and the blind spot in the grep for it.** Qt 2's
`QListViewItem(QListView*)` **prepends**; `QTreeWidgetItem` appends.
`MT_ExperimentWidget` builds its six pages 5→0 and selects `firstChild()`, so 1.3
draws and selects **Strategy** where an appending port selects **Statistics** — a
different startup page. C7 found 26 sites: 23 by sweep and **3 that a grep for
`new QTreeWidgetItem(` cannot see, because they sit in a base-initializer list**
(`: QTreeWidgetItem( parent )`). *A grep for a constructor call cannot find a
constructor that is inherited — the shape to look for is the initializer list.*
Their visible consequence: "Load Experiments" selects `topLevelItem(0)`, which
1.3 made the **last** file loaded. §9's TRAP section carries the full count.

**The locale rule — apply it to every new validator.** Qt 6's
`QDoubleValidator`/`QIntValidator` follow `QLocale::system()` while
`toDouble()`/`toInt()` never do, so under a comma-decimal locale a typed `9,81`
validates and reads back as **0** — silently zeroing gravity. **Qt 2 could not
have it**: `QApplication` forced `setlocale(LC_NUMERIC, "C")`
(`qapplication_x11.cpp:1389`) and its validator hard-coded `'.'`
(`qvalidator.cpp:387`). So pinning to `QLocale::c()` with `RejectGroupSeparator`
is a **restoration, not an improvement**. C7 does it with one
`findChildren<QValidator *>()` loop per constructor, which also covers validators
added later; C11d found ten more in `MT_GUI` that C7 never reached, **including
both creation sites in `MT_AddConstantsWidget` — the type radio deletes and
rebuilds its validators, so pinning only in the constructor is undone the first
time someone switches between integer and float constants.** `DISpinBox` needs
the pinning plus `setKeyboardTracking(false)`: Qt 2's `textChanged` only set an
`edited` flag and `valueChanged` came solely from `setValue`, where Qt 6
interprets every keystroke.

**Two Qt 2 no-ops that Qt 6 turns into a crash or a warning.**
`QListView::setSelected(0, true)` returned early on null — and the
delete-experiment path calls it right after deleting the last experiment.
`QWidgetStack::raiseWidget(0)` returned early where `setCurrentWidget(nullptr)`
warns. Qt 2's `setSelected` was also *Single*-mode, deselecting the previous
focus item: that is `setCurrentItem()` in Qt 6, **not**
`item->setSelected(true)`, which would leave both rows selected. Related:
**`QDict::operator[]` is `const`; `QHash::operator[]` default-inserts a null and
grows the hash** — a straight rename would have accumulated nulls at every lookup
of a missing key, and `menuDict` has such a key by construction. Every lookup is
`.value()`. *And the four `setAutoDelete`-off/`remove`/`insert`/on windows were
`QDict::take()` all along — a method Qt 2 already had and the 2003 author did not
use.*

**`updateGL()` → `update()` is not an equivalence.** Qt 2's `updateGL()` *is*
`glDraw()`: `makeCurrent(); paintGL(); swapBuffers();`, all before the call
returns. Qt 6's `update()` posts a paint event. Safe at all 19 sites only because
no caller reads back what `paintGL` produced, and on the movie path the grab
precedes the repaint. Likewise `QMouseEvent::state()` → `buttons()` is safe only
because both uses are in `mouseMoveEvent`; on press and release Qt 2's `state()`
was the state *before* the event.

**Every class that gains an owning destructor must be made non-copyable.** Qt 2's
`QGVector` copy constructor cleared `del_item`, so a copy freed nothing; a
`QList` copy shares the raw pointers and **both** destructors `qDeleteAll`.
`SIG_Renderer`, `SIG_EnvironmentRenderer` and `SIG_RenderRecorder` get `= delete`,
as D7, D15 and D25c each did. *C5 first added the destructors and left the
classes copyable — the fourth time this file records that precedent and the third
time it was not applied.*

**But read the ownership off the 1.3 binary first, because one of these
containers is deliberately NON-owning and making it owning is a double free.**
`SIG_Visualisation::floatingTexts` **aliases the renderer's pointers**, filled
from `robotRenderer.floatingTexts` — its constructor at `0x080ca020` makes **0**
`setAutoDelete` calls where `SIG_Renderer`'s at `0x080cf640` makes **2**. That is
the counter-example to the rule above, and without it the rule reads as "add a
destructor everywhere". *The source comment at `SIG_SimulationVisualisation.cpp`
says "Non-owning"; this is where the evidence for it lives.* Restored 2026-09-03.

**A range check was lost at a FILE-DRIVEN index, and D6, D8, D9 and D25c all say
to record that.** `SIG_RobotRenderer.cpp:46` is `delete sceneObjects[ number ];
sceneObjects[ number ] = newSceneObject;` where `number = actLink.getNumber()` is
**read from the model file** (`SIG_Link.cpp:65`). Qt 2's `QGVector::insert` was
range-checked and returned false; `QList::operator[]` asserts in these builds and
is an out-of-bounds **write** under `-DQT_NO_DEBUG`. Latent — the compiler numbers
links contiguously from 0 — but it is a file-data index, not a derived one, and
§9's link-number section enumerates `drives`, `sensors` and `jointIndices`
without reaching this one. Restored 2026-09-03.

**Two Qt 2 behaviours restored by hand that a later reader could delete as
redundant.** (1) **Qt 2's `rightButtonClicked` called `clearSelection()` when the
click missed an item** (`qlistview.cpp:3390`, `qlistbox.cpp:1656-1658`), which
fires `selectionChanged` and is why 1.3 greys `Delete` on blank space;
`customContextMenuRequested` does neither. Restored at four sites. (2)
**`SIG_MainWindow::closeEvent` exists to restore Qt 2's rule that closing the
MAIN widget quits regardless of other open windows** — Qt 6's
`quitOnLastWindowClosed` waits for the *last* one, so with an individual-view
window open, closing the main window would leave SIGEL running. The divergences
table says `quitOnLastWindowClosed` "covers the closing-ends-the-app half", which
on its own reads as though nothing more was needed. Both restored 2026-09-03.

**Two more form-conversion traps.** **Qt 2's `QButtonGroup` was a widget** — a
`QGroupBox` that *also* managed its buttons and their ids — so `uic` emits only
the `QGroupBox` and `find(id)`/`clicked(int)` lose their other half silently; the
logical half is rebuilt by hand in `MT_AddConstantsWidget`, and exclusivity needs
no flag because Qt 2's `init` set `radio_excl = TRUE` and Qt 6 defaults
`exclusive` to true. And **`WType_Modal` is not `Qt::Dialog`**: the bits coincide
at 0x3, which is not an equivalence — Qt 2's flag *made the window modal*
(`WState_Modal`, `qwidget.cpp:725`), while Qt 6's names a window type and
`windowModality` defaults to `NonModal`, so it needs
`setWindowModality(Qt::ApplicationModal)` as well. **The type test must mask**:
`( f & Qt::WindowType_Mask ) == Qt::Dialog`, because a bare `f & Qt::Dialog` is
true for a plain `Qt::Window` too.

**Three structural link requirements, all of which fail silently.** `MOC_HDRS` is
**derived from a `Q_OBJECT` grep**, never hand-written — a missing entry is not a
compile error but an undefined vtable at link, or a signal that never fires.
**Resource objects are named on the link line**, not left inside an archive:
nothing references their symbols, so a static library drops them and the form
icons silently vanish. And **the slave is not a cut-down master** — putting all
56 meta-objects on its line drags in `SIG_Experiment` → `MT_Controller` → the
master `SIG_GPExperiment` it must never have; §9's assertion caught it at 45
symbols. `sigel_eval` and `pvm_link` broke the same way and are pinned to the
core-only set they always had, which is how the fitness baselines stayed
byte-identical.

**The build was choosing between the two `SIG_GPExperiment` variants by luck, and
the variable is the LOCALE.** Both objects go into `libSIGEL_GP.a` and the linker
takes the first member defining the symbol; `$(wildcard)` sorts by `strcoll`, and
the two names order differently either side of the `Clean` suffix — `LC_ALL=C`
puts the master first and the link fails, `en_US.UTF-8` puts Clean first and it
links. `SIG_GPExperimentClean.o` is now named explicitly ahead of the archives
with an assertion after the link. *A future reader chasing a recurrence should
look at the environment, not at build freshness — the first diagnosis blamed
clean-versus-incremental builds and was wrong three ways.* See §9's
"`SIG_GPExperiment` is defined twice, on purpose".

**Two blind spots no gate reaches, stated so they are not mistaken for
coverage.** Three Qt 2 APIs survive in `#ifdef _WINDOWS` blocks that **cannot be
compiled here** (`windows.h` absent) and were read by hand. And **the floating
text labels are *unverified*, not unexercised** — the code demonstrably runs
(1785 `paintGL` hits) but the oracle's 3-D view renders nothing at all, so the
absence of labels there is equally consistent with "they do not composite over a
native GL window" and "nothing renders in that environment". `QOpenGLWidget`
composites children correctly where `QGLWidget`, a native child window, often did
not, **so this port may make them more visible than Qt 2 did, and nobody can
currently detect that in either direction.**

**Four more "this was not observed" statements, restored 2026-09-03 because each
is a live limit rather than finished narrative.**

- **C6's progress dialogs never appear**, so the Cancel-button and modality
  claims about them are read **from the Qt 2 signature, not observed**: constant
  generation is capped at 99 and completes faster than `QProgressDialog`'s
  minimum duration, and the evolution toolbar is greyed without a running SIGEL.
  *The 99 cap is not a divergence — the form sets no maximum and both Qt 2 and
  Qt 6 default `QSpinBox` maximum to 99.* Going with it: **`QProgressDialog`'s
  third argument changed meaning**, `totalSteps` in Qt 2 against `minimum` in
  Qt 6, and `cancel` was `0` in 1.3 so there is no Cancel button at all.
- **Only one of C10's two `blockSignals` sites is gate-covered.** The second, in
  `slotAddIndividuals`, sits on a path that only ever *grows* the pool, so a
  stale `poolPosition` still resolves to the same individual and reverting it is
  invisible to any GUI observation. It is kept because Qt 2 emitted nothing there
  either, but it is **faithfulness without a test**, and removing it later would
  be defensible rather than a regression.
- **C11b: two dialogs appear, not three.** Qt 6's `getSaveFileName` raises its own
  *"already exists. Do you want to replace it?"* box where Qt 2's had none. It
  cannot fire on the tested path **only because the typed name carries no
  extension** and `checkEnding()` appends it after the dialog closes — so Qt's
  check sees a name that does not exist. **Typing the extension does produce it**,
  and the oracle confirmed 1.3 shows only its own box in that case. A trap for the
  next probe that decides to type the extension.
- **C10 is the one direct observation of the PVM `+ 2` wire margin on a real
  daemon**: the pvmd log read *declared 10933, received 10931* for a 10,931-byte
  payload. D21's row says **NOTHING VERIFIES IT** and that remains true of the
  gates — `pvm-check.sh` passes with `+ 1` and with `+ 0` — but this observation
  exists and is not reproducible from anything committed.

**Probe craft — every apparent defect in C10–C11d that was not in the list above
was the probe.** Nine of them. (1) The harness linked the wrong
`SIG_GPExperiment` and called `IsEnabled()` through `this=0x2020202020220a2c`,
ASCII spaces read as a pointer. (2) `QTest::mouseClick` does not synthesise a
`QContextMenuEvent`, so all five context menus read as absent. (3) An "empty
space" click landed on a row, because with 120 individuals the viewport is full.
(4) A greyed field ate five probes — the integer battery ran *after* a page drive
that had clicked a radio whose side effect greys the field, and a disabled
`QLineEdit` correctly ignores key events. (5) A "clamp" that was a missed grab:
the drag pressed at x=6, which is groove, so it page-stepped *down* by one and
50 → 49 read as a clamp; the press now lands on the handle, positioned from
`QStyle::sliderPositionFromValue` rather than guessed. (6) An LCD that "did not
follow its slider" was correct — the slot displays
`(slider / 1000) * populationSize` truncated to int, and with 120 individuals
every value in the probe's range is 60. (7) The language-command list came out
with 13 rows against 1.3's 15: `SIG_LanguageParameters`' constructor creates a
row only for a command the experiment has **at construction time**, and the
application builds the page against a default experiment *before* loading the
file — **probing in the opposite order invents the defect**, and checking before
changing anything is the only reason working code was not "fixed". (8) A modal
handler closed the window under test, because `MT_MainWindow` comes back from
`activeModalWidget()`. (9) `select-all` + `Delete` does not empty every field, so
the probe would have measured a concatenation — **caught by the `!!` guard before
it could reach a baseline, the first time that guard paid for itself.**

**The oracle retracted its own readings twice in the same period**, so the
exchange corrected in both directions: an empty-space right-click that appeared
to leave the selection intact turned out to have an invisible application-modal
dialog holding the X input grab behind the main window, swallowing every click —
**the port was right and the measurement was wrong.** Its corrected result then
confirmed the `menuDict` work end to end: a context menu on exactly five rows and
none on Individuals or Language-Parameters, which is exactly the five keys
`menuDict` is inserted with.

**The file-dialog flake took three diagnoses and the first fix was worse than the
bug.** `openExperiment` had been intermittently loading nothing since C10. (1)
*"The completer popup eats the Return"* — the popup is real, so the fix escaped
it and pressed Return again, and the dialog then accepted with the *completion's*
filename: the experiment did not load, the tree was empty, **and the run carried
on regardless.** A hang is loud; a wrong file is not. **The only reason it was
caught is that the fix printed when it took the fallback.** (2) An absolute path
made the race rarer, not absent. (3) The cause: **`QFileDialog::accept()` treats
*any* filename carrying a directory as a navigation request** — it calls
`setDirectory()` and returns without accepting. Fixed by setting the directory on
the dialog and typing only the basename. *The general lesson is not about file
dialogs: a recovery path that does not say it fired turns an intermittent failure
into an intermittent wrong answer.*

**Measure a negative with a positive control.** C11d's modality claim: with the
MetaGP window open a click on the main window's tree changed **0 pixels**; with
it closed, the identical click at the same coordinates changed **4397**. *"The
main window did not respond" is the shape of a click that missed, and a single
null would not have distinguished the two.* The same step measured that closing
the window **unmaps it without destroying it**, so its presence in the window
list is not evidence that it is open.

##### What was measured against the running 1.3, and how

**C9 verification.** A headless probe prints every menu entry and toolbar button
with shortcut, enabled state and check state; the oracle read the same facts off
1.3. **42 menu entries, zero mismatches**, including every oddity — the duplicate
`Ctrl+Shift+L`, `Reset` carrying `Alt+O` with no matching letter, `Evaluator
System` showing its tick while greyed, and the Import/Export submenu **parents**
staying enabled while all sixteen children are greyed, which is the direct
observable proof of the action-group fix. Toolbars agree too, including the File
toolbar omitting Rename and putting Delete second. With the same experiment
loaded every GP-page value matches, **including the three that are not echoes of
the file**: per-mille 50/670/280 displayed as 5/67/28; "Tournaments per
generation" showing **60**, the file's 0.5 × a 120-individual population; and
pool-image frequency clamped to the spin-box minimum. The PVM host list comes out
**exactly reversed from file order** — a second prepend demonstration, and a
better one than the command list because it is driven by file data rather than by
constructor calls.

**The `truncate` sweep the oracle asked for afterwards.** *If the trigger is
`uint` vs `qsizetype`, the same shape exists anywhere 1.3 feeds a possibly-`-1`
search result into a Qt size parameter and relies on the unsigned wrap being
harmless.* **30 such sites across 371 files, and they reduce to one:**

| call | Qt 2 (`uint`, -1 wraps huge) | Qt 6 (`qsizetype`, -1 stays -1) | same? |
|---|---|---|---|
| `left(-1)` | whole string | whole string | yes |
| `right(-1)` | whole string | whole string | yes |
| `mid(p, negative)` | to end | to end | yes |
| `chop(-1)` | no change | no change | yes |
| **`truncate(-1)`** | **no-op** | **clears the string** | **NO** |

**C10's decomposition, and the two byte-exact cross-machine anchors.** *"The
same" cannot mean "the same fitness"* across an x87/IEEE boundary, so the
question was split into parts that are machine-independent: that the GUI hands
the evaluator the same individual (`Export > Program`, diffed by checksum), that
the PVM payload carries it unaltered (captured with a stub slave), and that the
individual evaluates the same way (`fitness-baseline.txt`, same machine only).
The anchors are **the exported program** — 6616 bytes, `sha256
f940751765c869f16f62333dafd34dbe0e66b6474577bf4209e9e8de70cc3214` on both
architectures, reproduced again through a different scenario in C11b — and **the
saved 2.7 MB experiment file**, whose byte counts agree across two saves on both
machines, so the writer's formatting of every value agrees and not merely the
structure.

**No tool on this machine can deliver a real X-level click, measured rather than
assumed.** XTEST is present and `XTestFakeMotionEvent`/`XTestFakeKeyEvent`
**return success and have no effect** — a VMware guest under GNOME/Wayland, where
`vmware-user` owns absolute pointer integration and the compositor's pointer
wins. *The pointer was read back rather than trusted, which is the only reason
this was caught.* Qt's VNC platform serves but `sigel` crashes on it, and that
platform has no GL context, so the crash is an unsupported configuration rather
than a port defect. **So the driving is `QTest`, and it is not a mouse.**
`mouseClick`, `keyClicks` and a hand-posted `QContextMenuEvent` go through
`QApplication::notify`, so event handlers, hit-testing, `QMenu` popups, item-view
selection and the slots behind them all run — but bypassing
`QWindowSystemInterface` changes activation, grabs, double-click synthesis and
enter/leave, **and it posts key events straight at the widget, focus or no
focus**. It proves SIGEL's logic, not the platform layer's. **The oracle's side
was driven by real XTest, so every comparison has a genuine click on one end.**

**C11a's validator battery is 12 probes typed ONE CHARACTER AT A TIME** — seven
into a `QDoubleValidator` field and five into a `QIntValidator` field. A
validator that answers `Invalid` makes `QLineEdit` drop that keystroke, so the
surviving text carries a gap exactly where the rejection happened, **and that gap
is the measurement**. It is also the only thing the oracle *can* read: Qt 2 has
no accessibility API, so 1.3's side reports behaviour, never configuration.
**Twelve for twelve**, including the two that would have been got wrong by
reasoning — the exponent passing and the **integer** field taking a leading
minus: `9.81`→`9.81`, `9,81`→`981`, `0,375`→`0375`, `-2.5`→`-2.5`, `1e3`→`1e3`,
`abc`→empty, `1.2.3`→`1.23`; `42`→`42`, `-7`→`-7`, `4.2`→`42`, `4,2`→`42`,
`abc`→empty. The 21st validator is on `SIG_EditCommandDialog` and was driven in
C11c with the same result, **closing the set at 21 of 21**.

**Locale independence is measured on both sides, which is the point of C7.** This
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
architectures.** `putAllIntoExperiment()` runs on save, so a written `.exp`'s
parameter block is exactly what the five pages hold — **and it is bytes, not a
number x87 and IEEE can round differently**. `STEPSIZE`, `MAXIMALERROR`,
`DYNAMECHSINTEGRATOR`, `YPLANELEVEL`, `GRAVITY` x, a floor dimension,
`RANDOMSEED`, maximal age and a `LanguageParameters` field, typed identically on
both sides from a fresh load: identical on i386/Qt 2 and aarch64/Qt 6, and
nothing else moved. **`pagesave` IS NOT GATED, and that is §9 item 1** — *superseded 2026-09-03, when it was gated as `pagesave vs 1.3`; the paragraph is preserved as C11a wrote it, and §9 item 1 carries the outcome* — the
strongest single result in C11a and nothing guards it. What *is* gated is the
same typing reaching the same widgets (`pages`), so a regression in the widgets
would be caught; **a regression in `putAllIntoExperiment()` between the widget
and the file would not.**

**C11b: seven of the eight exports are byte-identical to what 1.3 writes** —
`.gpp` 1299 B, `.sip` 305, `.env` 299, `.pop` 2,738,900, `.prg` 6616, `.ind`
22,113, `.dat` 17,420, with the filename typed **without an extension** so
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

**The Qt 2 dict model has now predicted three different answers and been
confirmed on the binary each time.** Reimplemented from the vendored source —
`qgdict.cpp`'s `hashKeyString`, `index = hash % vlen`, new buckets prepended by
`look_string`, iteration from bucket 0 upward, `qdict.h:49`'s default table size
of **17** — it gives the default-constructed command order
`MUL MOVE CMP COPY LOAD JMP SENSE NOP SUB DIV MIN DELAY ADD MOD MAX`, the
measured `.lap` reload permutation, and JMP's insertion point sixth after LOAD.
*It was checked against something already on disk before it was trusted:
`twoBasesSimpleFitness2.exp` stores 13 commands in exactly that order with JMP
and NOP removed.*

**C11c's certainty came from a reading eight steps earlier.** C10 measured the
**Rename** dialog as pre-selecting in **both** versions — same apparent pattern
as the pre-fill defect, opposite result, which looks like a contradiction in the
oracle's own data until you read the source: Rename calls `selectAll()`
**explicitly** (`SIG_ExperimentListView.cpp:97`) and the other three do not.
*"These two readings disagree" is sometimes a fact about the code rather than an
error in the readings.* The fix was wrong twice first: `deselect()` before
`exec()` is a **no-op**, because the selection does not exist until the dialog is
*shown* and focus travels the tab chain; and `deselect()` alone leaves the cursor
at **position 0**, so a typed digit lands *before* the value and `1` + `2` gave
**21**. **`end( false )`** clears the selection *and* puts the cursor after the
text. `slotAddHost` is deliberately not patched — it focuses an empty field.

**C11d's structure, because the tree labels do not match the source class names**
and that cost both sides a round trip: Strategy = `MT_EstimationWidget`,
Individual = `MT_IndividualsWidget`, Population = `MT_PopulationWidget`,
**GP Parameter = `MT_SearchWidget`**, Selection = `MT_SelectionWidget`,
Statistics = `MT_StatisticsWidget`. `MT_ExperimentWidget` is not a page at all —
it *is* the tree. **The 30 constants are deliberately not pinned**:
`MT_IndividualsWidget` generates them with a randomizer at construction, so they
differ every run; the count and "every one parses as an integer in range" are
checked instead, which is an invariant rather than noise.

##### TRAP — `QIntValidator` returned Intermediate out of range in Qt 2 and returns Invalid in Qt 6

**This is a real regression, found only by typing, and it changes the number
that reaches the file.**

Qt 2's `QIntValidator::validate` (`qvalidator.cpp:236`):

```cpp
else if ( tmp < b || tmp > t )
    return QValidator::Intermediate;
```

Out of range is **Intermediate**, so `QLineEdit` accepts every digit.
`qspinbox.cpp:186` gives every `QSpinBox` exactly that validator over its own
`[min,max]`. `QSpinBox::interpretText()` then maps the whole text and calls
`setValue()`, and `QRangeControl::directSetValue` "Forces the new value to be
within the legal range" — the clamp. **Qt 6's returns `Invalid` once the typed
prefix passes the top**, so the keystroke is refused and the box keeps the
truncated prefix.

Measured on both, four fields, typing then Return:

| field | range | typed | 1.3 reads | 1.3 commits | port reads | port commits |
|---|---|---|---|---|---|---|
| Maximal program length | 2..32000 | `32001` | `32001` | **32000** | `3200` | **3200** |
| Year | 1752..8000 | `8001` | `8001` | **8000** | `800` | **2001** |
| Hour | 0..23 | `24` | `24` | **23** | `2` | **2** |
| Register width | 1..99 | `100` | `100` | **99** | `10 bit` | **10** |

Four out of four clamp to the maximum on 1.3; none of them does here. Both end
on a *valid* value, they are **different valid values**, and
`putAllIntoExperiment()` writes whichever the widget holds into the `.exp`.
Only reachable by typing a number outside the range — but that is the case
where the two disagree, not a case where either refuses.

**A second, smaller difference in the same widget: 1.3 drops the suffix while
editing.** The register-width box reads `3 bit` at rest, plain `100` during
typing, `99 bit` after commit; the port shows `10 bit` throughout. Qt 2's
`updateDisplay()` writes prefix + text + suffix into the line edit and does
nothing to protect it, so select-all + Delete takes the suffix with everything
else; Qt 6's `QAbstractSpinBox` actively keeps prefix and suffix in the editor.
Transient — both show the suffix again after commit. Recorded, not chased.

**ACCEPTED, NOT FIXED — D28, 2026-09-02.** Reachable only by typing a number
outside the box's own range, and the differing value is visible in the box
before anything is saved. The cost of restoring it is not the code but owning
a custom widget forever. Pinned in the gate so it cannot drift. A 25-line `QSpinBox`
subclass restoring Qt 2's rule reproduces all four readings exactly
(`32001`→32000, `8001`→8000, `24`→23, `100`→99, in-range typing unchanged); it
is written out in `future_refactorings.md`. Applying it means promoting **29
spin boxes across three converted Designer forms**, and to be consistent it would
also have to cover the dialogs, which are a separate step. The current behaviour is **pinned in the gate** — the
`commits=` value of each of the **29 spin boxes on the five pages** is in
`guibehaviour-baseline.txt`. The other 18, on the dialogs and in `MT_GUI`, are
not pinned.

*Two cheaper fixes were tried and rejected on measurement, not on taste.
Swapping the validator on the spin box's internal `QLineEdit` lets the digits
through but leaves `QAbstractSpinBox`'s own interpret path unchanged, and
`100` into a `[1..99]` box then commits to **1** — worse than the divergence.
Widening the range would change what the pages display and write.*

**CORRECTION 2026-09-03, appended rather than edited into the block above,
which is preserved as C11a wrote it.** *"The other 18, on the dialogs and in
`MT_GUI`, are not pinned"* was true at C11a and is **false now** — C11d's
`metagui` scenario pinned eleven of them. Counted in
`guibehaviour-baseline.txt`: **44 `spin` lines, 40 carrying a committed value**
(29 on the five pages, 11 in the MetaGP block). The four that are not are three
survey-only lines at 214, 1347 and 1399, and one at 1404 reading
`commits=not-pressed(in a dialog)`.

**And the 18 itself never reproduced.** Counted in the generated forms: **46**
`QSpinBox`, of which **29** are precisely the three page forms
(`SIG_GPParameterBase` 18 + `SIG_SimulationParameterBase` 8 +
`SIG_LanguageParametersBase` 3 — so the 29 is exact), leaving **17** elsewhere in
the forms, or **20** counting `toleranceSpinBox`, `powerSpinBox` and
`lineProbSpinBox`, which are declared in code rather than in a `.ui`. Neither is
18, and a first version of this correction repeated the 18 while correcting the
arithmetic around it. *A figure written when a step closed keeps being read as
current after later steps move it; this one stood for a day in two places at
once, and its own correction had to be corrected.*

##### What was NOT exercised, and why

- **A real X-level click.** No tool on this machine can deliver one.
- **A running evolution over several generations — the largest gap.** What IS
  established: the three Start guards pass (`bodies=1`, `population=120`,
  `fitnessName=SimpleFitnessFunction`); `QSignalSpy` on
  `signalEvolutionNotRunning` catches **`false` then `true`**, so the slot really
  runs, where a silent decline would emit nothing; two real `sigel_slave`
  processes were seen under `ps`, so `pvm_spawn` dispatch works; Stop works and
  the population is intact afterwards. **Every shipped experiment terminates on a
  date in 2001** (`TERMINATIONUSESDATE 1`, `TERMINATIONTIME 2001`, all 14), so a
  correct Start finishes in under 100 ms — sampling at 5 s and at 100 ms both
  missed it, and only the signal spy distinguished it from a Start that did
  nothing. **Not seen: the generation counter advancing, the statistics, or the
  fitness curve.** With duration-based termination a real run does start, but the
  master then waits on slaves and `haveABreak()` only services the GUI between
  them, so no generation completed in budget at 120 individuals or at 8. Nothing
  suggests a defect — it is one machine's throughput, and **that was confirmed
  2026-09-02**: the oracle measured **≈ 4 minutes per generation** on the 2003
  i386 box from the timestamps of completed runs, so C10 was never going to see
  one inside its budget — but the `gui behaviour`
  gate does not cover the evolution path at all.
- **The visualisation window.** The payload was captured with a stub; the real
  GL window was not opened here. The oracle opened it on 1.3.
- **Native file dialogs.** Qt's own non-native dialog was used throughout.

##### What C11a did NOT exercise, and why

- **The Robot page has nothing in scope.** Six list boxes, four buttons, no
  spin box, slider, combo, checkbox or validated field. Its contents are
  surveyed — 2 links, 1 material, 1 body, 1 sensor, 1 joint, 1 drive, with
  icons on the last three — and the driver says so explicitly rather than
  printing an empty block that reads as a harness failure.
- **The four Robot-page buttons and the pages' Import/Export buttons.** They
  open file dialogs and a `SIG_TextView`; both are separate C11 items.
- **The 21st validator.** It is on `SIG_EditCommandDialog`, not on a page, and
  dialogs are a separate item. 20 of the 21 are driven here.
- **`listviewCommands` and `listviewHosts` beyond their contents.** Both are
  double-click entry points to dialogs.
- **A real X-level click.** Still impossible on this machine; C10 measured that
  and nothing has changed.

##### What C11b did NOT exercise

- **`Import > Population` beyond the round trip.** It replaces the pool; the
  round trip covers the format, not the replacement's effect on the view.
- **A malformed file into any importer.** Every import here was given a file
  the matching exporter had just written.
- **`Import > Robot` with a robot that is not the experiment's own**, and the
  `Robot import error!` path.
- **The six dialogs, `MT_GUI`, and a running evolution** — separate items.

##### What C11c did NOT exercise

- **The multi-selection branch of Edit Command**, which hides the command label
  and retitles to `Edit commands...`.
- **`slotAddHost`**, the Add path of the Edit Host dialog, and its `...` browse
  button — both open a further file dialog.
- **Anything that writes through these dialogs except the JMP case.** OK was
  pressed once, on the JMP allow; every other dialog was cancelled.
- **`MT_GUI`'s dialogs**, which are a separate item.

##### What C11d did NOT drive

- **The toolbar actions that do something**: Start and Stop evolution, Default,
  Load, Save, the population Add / Delete / Import / Export / Load / Save, and
  `update statistics`. All were surveyed — labels, enabled state, icons — and
  none was clicked. Start and Stop are disabled on arrival anyway.
- **`MT_AddConstantsWidget`**, which is a dialog behind the Individual page's
  Create button. Its four validators are fixed and its type-radio rebuild is
  fixed, but nothing has opened it.
- **`MT_Editor`**, the program editor on the Individual page.
- **A MetaGP evolution actually running**, which belongs with the main
  evolution path. *That path is now done (§9 item 2) and `SIGEL_SlaveGUI` was DRIVEN 2026-09-03. What is still
  undriven is the MetaGP variant of an evolution, `MT_Editor`, `MT_AddConstantsWidget` and
  `update statistics`.*

---

## 8. Steps and status

**Phase status is the table at the top of this file.** A second one lived here
and drifted: it had Phase B at "8 of 14", Phase C at 10 steps and Phase V at 5
where §7 defines C1–C10 plus C11a, C11b and C11c, and V1–V9, and it omitted Phases D and R entirely.

**No effort estimates in this file.** The column that held them carried six
invented figures. Step counts are counted and stay. **Do not put estimates
back.**

**A single fitness evaluation runs locally with no PVM** — that is `sigel_eval`
and all of Phase R. What has no local path is the **evolution loop**:
`SIG_GPFitnessTrainer` dispatches every evaluation through `pvm_spawn` of
`sigel_slave` with no in-process fallback, and the one method that looks like a
fallback, `SIG_GPExperiment::calculateFitness` (`SIG_GPExperiment.cpp:158`), is a
stub that returns 0. So without PVM the ported interface builds and shows its
windows, and nothing happens behind the Start button. **With the vendored PVM
up it now runs locally** — three generations driven end to end on 2026-09-02,
§9 item 2. Every clause above is still true *without* PVM; PVM is no longer
absent.

---

## 9. Open

**WHAT IS ACTUALLY OPEN, as of 2026-09-03.** The tables below are mostly struck
through; read this first and use them for detail only.

| still open | size |
|---|---|
| ~~**item 1** — `pagesave` and `roundtrip` have no gate~~ **BOTH DONE 2026-09-03.** `pagesave` against a reference captured from the 2003 binary; `roundtrip` after fixing a probe that could not fail | closed |
| **`SIGEL_SlaveGUI` has no runtime connect coverage** — 44 `SIGNAL(` and 44 `SLOT(` sites, and `check.sh` never runs the `slavegui` scenario | coverage; found 2026-09-03 |
| ~~**item 3** — `QHashSeed::setDeterministicGlobalSeed()` unowned in `sigel.cpp`~~ **DONE 2026-09-04** — and it was UI non-determinism, not bookkeeping: `~SIG_Experiment` iterates `widgetDict` to remove widgets from a **shared** stack | closed |
| **item 4** — six dropped size constraints | measured, cosmetic |
| ~~**undriven**: a MetaGP evolution, `MT_Editor`, `MT_AddConstantsWidget`, `update statistics`, MT_GUI's toolbar actions~~ **ALL BUT THE EVOLUTION DRIVEN 2026-09-04** — new `metadrive` scenario, in the gate | coverage |
| **a MetaGP evolution — probably UNREACHABLE, on both versions, and for a reason worth stating** | `Start` is created disabled and is enabled only by `separateEvolutionAllowed()`, i.e. `getPresentTSize() >= getResultArray()->size()` — the training set having FILLED — and that predicate is re-evaluated **only on a MetaGP page switch** (`MT_MainWindow.cpp:282,309,361,380`), so it never lights up on its own. Filling the trainer needs a running SIGEL evolution feeding the classifier and evaluator. **There is no shortcut in the shipped data**: `&Open` filters for `Setup(*.mcnf)` and **no `.mcnf` exists in the repo or any tarball**. **And the MetaGP window is application-modal**, measured on 1.3 — with it open the main window accepts no input (0 pixels changed, against a control of 141,023 for a page switch inside the MetaGP window), so the feeding evolution cannot run while the window showing `Start` is open. The only route is enable → **close** → run → reopen → switch pages. *1.3 then crashed in generation 4 of that run; see the `pvmTasks` hazard in §10. Attribution pending a control.* **If both versions never enable Start, that is an equivalence result and closes this item rather than leaving it open** |
| ~~D28's divergence reaches `MT_AddConstantsWidget`~~ **FIXED 2026-09-04** — it reached the generated constants tenfold, so Qt 2's out-of-range rule is restored there rather than accepted | closed; D28 stands for the spin boxes it covers |
| six forms declare a minimum smaller than Qt 6's layout needs — inherited from 1.3, `MT_StatisticsWidgetBase` unreadable if dragged small | usability |
| the evolution result is recorded as prose; no artefact is committed | reproducibility |

**Closed and kept only for their lessons:** items 2 and 5, every row of the
`never driven` table, V3 (satisfied) and V4 (dropped). **The most useful part of
this section is the hazard list near its end** — measured traps, all
forward-looking, and the thing most likely to save a future session time.

**A warning this section has earned.** Four claims were withdrawn in two days —
the item-5 reachability argument, the evolution headline, a slider divergence and
a seed. Each was inherited from a document and repeated before being checked
against source. **Verify before citing anything here.**


### C12 — the MetaGP window's last undriven corner, 2026-09-04

`metagui` (C11d) surveyed the MetaGP window and printed what it found; nothing
had ever **pressed** anything on it. §9 listed four such items — `MT_Editor`,
`MT_AddConstantsWidget`, `update statistics` and MT_GUI's toolbar actions — and
`metadrive` now drives all four here, and the oracle measured the first three
on 1.3. *The toolbar actions have NO 1.3-side measurement — neither pressed
action has a row in the table below, so for that one item this is coverage on
this side only, not an agreement.* **The question throughout was
core functionality, not appearance: can a user still do the thing, and does the
thing still do what it did.**

**Three results agree with 1.3, one diverges, and one prediction was falsified.**

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

**THE DIVERGENCE REACHED THE GENERATED DATA, AND IS THEREFORE RESTORED RATHER
THAN ACCEPTED.** Qt 2's `QIntValidator` returned *Intermediate* out of range, so
`QLineEdit` accepted every digit and the range bit only on commit; Qt 6 returns
*Invalid* and drops the keystroke, leaving a truncated prefix. §9's D28 accepted
exactly this for the **29 spin boxes on the five parameter pages**, explicitly
not the dialogs — and the reason it could be accepted there is that the
differing value is **visible in the box** before anything is saved.

**Here it is not.** The oracle measured the commit side on the running 1.3, with
min set equal to max so the result could not be a random draw: integer mode,
min = max = `-50000`, count 3, OK — the field keeps `-50000`, the count goes
30 → 33, and **the three new constants are `-50000`**. No clamping at any point.
A port holding `-5000` generates constants of `-5000`: **a tenfold difference in
data that reaches the MetaGP population and is invisible from then on.**

**THE FIRST FIX WAS INCOMPLETE IN THE WORST POSSIBLE WAY, and a review caught
it.** It restored the INTEGER validators of a dialog that **opens in float
mode**, and the float half carried a worse instance of the same class. Qt 6's
`QDoubleValidator` returns Invalid for any input beginning with `-` whenever
`bottom >= 0` — and 2003 built this dialog's float min validator as
`QDoubleValidator(100000.0, -100000.0, 4)`, bottom above top. Measured on
Qt 6.10.2 through the same typing route: **`-50000` becomes `50000`, `-5`
becomes `5`, `-1.5` becomes `1.5`. A sign flip, in the default mode, on the
value that becomes the generated constants.** Qt 2 had no such rule
(`qvalidator.cpp:362`) — out of range was always Intermediate. Qt 6 also
enforces `decimals`, so `1.23456` became `1.2345` where Qt 2 returned
Intermediate and kept typing.

**Both validators are now TRANSCRIBED from vendored Qt 2 rather than patched on
top of Qt 6's answers**, which is what let the first attempt miss two of the
three divergences: Qt 6 says Invalid in three places Qt 2 said Intermediate, and
post-processing one of them fixed one. `Qt2IntValidator` and
`Qt2DoubleValidator` reproduce `qvalidator.cpp:236` and `:362` including the
exponent handling, the `^ *-?\.? *$` empty form and the too-many-decimals rule.
*`toInt`, not `toLongLong`: Qt 2's `QString::toLong` capped at `INT_MAX/base`
whatever the platform's `long` width, so it failed past ±INT_MAX and the
first version was over-permissive there.*

**AND THE TRANSCRIPTION'S FIRST VERSION HANDED THE PRECISION LOSS BACK, ONE
FOCUS CHANGE LATER.** Neither class overrode `fixup()`, so Qt 6's was inherited
and live. **Qt 2's `QIntValidator` and `QDoubleValidator` do not override
`fixup` at all** — the only definition is `QValidator::fixup`, an empty body
(`qvalidator.cpp:175`) — and Qt 2's `QLineEdit` called it **only on Return**,
never from `focusOutEvent`. Qt 6 overrides it in both validators *and* calls it
on Return **and** focus-out. Measured with the float validator at
`(-10000, 10000, 4)`: typing `123.456789` and then clicking away rewrites the
field to **`1.2346e+02`**, and `accept()` stores `123.46`. `9999.12345` becomes
`9.9991e+03`. *So the commit that listed `1.23456 → 1.2345 (precision)` among
the three divergences it fixed had repaired the keystroke-drop half and
reproduced the loss through a different door, in the same data path.* Both
classes now override `fixup` as a no-op, which is Qt 2's own behaviour;
re-measured, all three values survive focus-out unchanged.

**The transcription was then checked against the binary rather than trusted**,
because reading the source is what produced the falsified narrowing prediction.
The oracle typed into a fresh dialog with the type radio never touched:
`-1.5` → `-1.5`, `1.23456` → `1.23456`, `9.87654321` → `9.87654321`,
`-0.0001` → `-0.0001`, `-50000` → `-50000`. **`decimals` is not enforced during
typing at all**, exactly as the transcription assumes. Its control, in the same
state: `abc` → empty, `1.2.3` → `1.23`, `--5` → `-5`. **The port reproduces
seven of the eight** — `-1.5` is the oracle's measurement only; the port side
types `50000` in its place and has no `-1.5` line. *So Qt 2's rule here is well-formedness only — one leading minus, one
decimal point — with no constraint on magnitude or decimal count, and that is
the shape now transcribed.* **That is preserving 1.3's behaviour, not
improving on it** — the same argument as pinning these validators to
`QLocale::c()`. *The control that this does not simply disable the validators: `abc` is still
rejected in both modes and `12.5` still becomes `125` in integer mode, both
matching 1.3 exactly.* **And the whole chain is now gated rather than just the
validator**: the scenario sets min = max = `-50000` in integer mode, which makes
the generator degenerate, and pins the produced values — `constants 30 → 33,
new constants=[-50000,-50000,-50000]`, exactly what the oracle measured on 1.3.
*Before that it pinned only the COUNT, and replacing `boss->minValue`/`maxValue`
with `0` in `accept()` — the typed bounds never reaching the generator at all —
left the output byte-identical.* Teeth-tested: reverting the validators gives
`[-5000,-5000,-5000]`. D28 stands unchanged for the spin boxes it covers; the two
decisions differ because the consequences differ, and both are measured.

**A PREDICTION MADE FROM THE SOURCE WAS FALSIFIED BY BOTH BINARIES.** The 2003
code builds the float validators with the bounds **swapped** —
`QDoubleValidator(100000.0, -100000.0, 4, …)`, bottom above top — and rebuilds
them at ±10000 when the type radio is clicked, which reads like a tenfold
narrowing. **There is no narrowing.** `50000` survives in float mode before and
after the switch, on 1.3 and here. The swapped bounds do nothing observable.
*Confirmed as 2003's own code against the pristine tarball first, so it was
never a porting slip — but "this odd-looking constructor must have an effect"
was an inference, and the binaries say it has none.*

**THE SCENARIO CLOSED THE WINDOW IT WAS TESTING, and that one defect explains
three separate wrong conclusions.** `MT_MainWindow` is itself modal and comes
back from `activeModalWidget()`, so the handler `whenModal([](QWidget *m) {
m->close(); })` closed the MetaGP window immediately after opening it —
**a trap `metagui` documents by name 150 lines earlier in the same file, having
been bitten by it in C11d.** Everything below it was then measuring a closed
window:

- `closed=1 mainWindowAlive=1` **could not fail** — deleting the final
  `mt->close()` outright still printed `closed=1`.
- `modalAppeared=0` in the statistics section was an artefact; with the window
  alive the handler sees `MT_MainWindow` itself and had to be narrowed to
  `QMessageBox` there too.
- **`MT_Editor`'s focus-out was declared undrivable and blamed on the platform.**
  This file said "the MetaGP window is never mapped under
  `QT_QPA_PLATFORM=offscreen`, and focus delivery is then racy". **That was
  wrong.** The window *is* mapped; this scenario had closed it. With the handler
  fixed the probe is stable across three runs — `discarded=1 editorHidden=1` —
  and **all three of `MT_Editor`'s contract points now agree with 1.3.** *A
  platform was blamed for a defect in the probe, and the retraction is recorded
  because the false explanation was the more plausible-sounding one.*

A `[window] visible=%d` line is printed right after the window is found, as the
control: if it ever reads 0 the handler is closing it again and everything after
is measuring a corpse.

**What the `update statistics` section can and cannot see.** It pins that the
action exists, is enabled, can be triggered without raising a box or changing
the page, and that the three Fitness fields read `ERR` exactly as 1.3's do —
which is a real agreement, because 1.3 does nothing visible there either. **It
cannot tell whether the slot behind the action ran**: gutting
`MT_StatisticsWidget::slotUpdateGUI()` to `return;` leaves the output
byte-identical. The `ERR` values are the `.ui`'s own static text and the two
fields that are not `ERR` are written by `onShow()` at page-raise. So this is an
agreement about **state**, not a test of the slot, and the section says so.

**A probe error of the usual shape, caught by a diagnostic rather than by a
failure.** The first run reported the editor never opening. `isVisible()` was
the wrong predicate: offscreen leaves the MetaGP window unmapped, so every
widget inside it reports invisible whatever the code did — while the editor
itself reported `isHidden()==0` and a real 84×17 geometry, i.e. `popup()` had
run correctly. **`isVisible()` would have recorded "the editor never opens" on a
port where it opens perfectly.** The predicate is `isHidden()`, which is exactly
what `MT_Editor`'s own `show()`/`hide()` pair sets.

**A claimed 1.3 behaviour that the Qt 2 source contradicted, and the source was
right.** The oracle first reported that **double-click does not open the editor**
on 1.3, only Return and the context menu. But 1.3 wires
`QListBox::selected(QListBoxItem*)`, and vendored `qlistbox.cpp` emits it from
**`mouseDoubleClickEvent`** (`:1840-1855`) *and* from Return/Enter
(`:2146-2155`) — both paths, unambiguously. Challenged on that basis and
re-measured: **the oracle's click helper sleeps 0.35 s after release and re-moves
the pointer, so two calls landed ~750 ms apart, past Qt 2's 400 ms
`doubleClickInterval`, and arrived as two single clicks.** With a real double
click the editor opens exactly as the source says. **Double-click, Return/Enter
and right-click → Edit all open it, and the port agrees.** *The control it
adopted is worth reusing: in the load-experiment dialog a genuine double click
accepts the dialog and loads the file with no Open click, which is
double-click-only behaviour and so discriminates a real double click from two
singles without depending on the widget under test.* The port reaches the same slot through
`itemActivated`, which fires on double-click or Return **only while the style
says activate-on-single-click is false** — so that style hint is pinned in the
gate, because a style saying true would make a SINGLE click open the editor
where 1.3 needs two.

**Two toolbar press blocks were DEAD CODE, and the baseline is how it hid.**
The action map is keyed `text + "\t" + objectName`, and every objectName on
these toolbars is empty — so `acts.value("&Default")` and
`acts.value("manual/timed stop")`, without the tab, matched nothing and both
blocks never ran. The baseline simply carried neither result line, so there was
nothing to notice. **Until this was fixed, nothing on the MetaGP toolbar was
ever pressed and "MT_GUI's toolbar actions DRIVEN" was false** — they were
surveyed, which `metagui` already did. Fixed and deterministic:
`manual/timed stop` toggles 0 → 1 → 0 and restores, `&Default` raises no box and
leaves the Strategy page's two spin boxes unchanged.

**Two more things the survey had been hiding.** `metagui`'s toolbar dump keyed
actions by text, which collapsed the evolution-control toolbar's eight
`addWidget` children — the status label, the hour/minute boxes — into **one
blank row**, and a later attempt keyed by text-plus-objectName still lost the
unnamed `QSpinBox` pair. They are listed individually now. And the Population
page's six actions are **not children of the page**: `MT_PopulationWidget` puts
them on a toolbar owned by the MetaGP window, so a `findChildren` on the page
returns almost nothing — which is why they appear in the window's toolbar map.

**A harness trap from the oracle, recorded because it cost it an X server.**
"Add Constants" is a separate top-level window **destroyed and recreated with
new X window ids on every open**; its cached ids went stale and
`import -window <stale id>` then hung *while holding an X server grab*, freezing
every client on that display. This driver resolves the dialog fresh from its
modal handler each time and caches nothing, so the trap has nothing to grab.

**Teeth-tested**, per §0: making `MT_Editor::hideEvent` see `acceptChange =
false` — the one-token change that stops Return committing — takes
`gui behaviour` from 1 pass to 0 pass 1 fail, and the diff lands on the editor
lines. Deterministic across four runs, no `!!` markers.

### C11 — the coverage gap C10 leaves — C11a–C11d DONE; the evolution path DONE 2026-09-02

C10 found two defects in **fifteen** driven scenarios. That density is the
argument for continuing: the defects were not in the parts anyone suspected,
they were in the parts anyone happened to drive.

**The gap as C11 found it, and what is left of it.** `guidump-baseline.txt`
lists **58** entries — 42 menu items and 16 toolbar buttons, separators
excluded. When this paragraph was first written `guidrive.cpp` clicked **13**
distinct actions and the estimate "roughly four fifths never used" followed
from that. It no longer holds: C11a drove all six View pages and C11b all
sixteen Import/Export children, so the two struck-through rows below are
closed. *The 68 in the earlier text was not derivable from the file it cited;
no slicing of `guidump-baseline.txt` gives it. Corrected by review.*

| never driven | what that leaves untested |
|---|---|
| ~~**5 of the 6 View pages**~~ **DONE in C11a** — all five driven: 29 spin boxes, 20 sliders, 7 combos, 4 checkboxes, 8 radios, 20 of the 21 validators. The 21st is on `SIG_EditCommandDialog`, so it belongs to the dialogs item | closed. The validator battery matches 1.3 character for character under two locales; a `QIntValidator` regression came out of it, accepted as a divergence by D28 and pinned in the gate |
| ~~**15 of 16 Import/Export children**~~ **DONE in C11b** — all eight exports driven and diffed against 1.3, the five parameter formats round-tripped, and Robot / Program / Individual import driven | closed. Seven of eight exports byte-identical across the two architectures; the eighth differs for a documented Phase D reason. One defect found and fixed, one preserved |
| ~~**6 dialogs**~~ **DONE in C11c** — EditCommand, EditHost, InfoBox, RobotInfo, IndividualView and AddIndividuals beyond its OK | closed. C7's 21st validator driven; a select-on-focus defect found and fixed at four sites; a command-insertion divergence measured and kept |
| ~~**The evolution path entirely**~~ **DONE 2026-09-02** — three generations driven on both machines from the same input, single-slave, terminating by Generation set through the Evolution control tab | closed, and three of the four named unknowns answered rather than merely exercised. **The enable/disable sweep**: Start GREYED and Stop ENABLED for the whole run, then back. **The generation counter DOES NOT MOVE during a run, and that is 1.3 behaviour to preserve** — the only two statements that would update it — `SIG_GUIGPManager.cpp:43` and `:92` in the working tree, `:40` and `:72` in the vendor commit `0516d62` — are **commented out already in the sources as released**, the sources as released 2003-04-30, one of them under `// update generations display (this line looks cool, doesn't it ?!)`. The only live update is `SIG_ExperimentView.cpp:64` — and it is **inside `putIntoExperiment()`** (which begins at `:59`), not inside any refresh hook, so the counter moves when the experiment is selected, when a page is switched, and at the top of `slotStartEvolution` — **never when a run ends.** `slotEvolutionStopped()` (`SIG_Experiment.cpp:642`) only re-enables widgets. Confirmed from behaviour on BOTH binaries: 187 samples here across 554 s and 172 on the oracle across two generation boundaries, every one reading the starting value. **And the post-run behaviour matches too — checked because the oracle reported a possible divergence and it turned out to be MY error, not the port's.** 1.3 still reads the old value after the run and only updates after a page switch; this port reads **136 at every sample for 30 s after `Start` returned**, and through Stop, in the 3-generation run. An earlier reading of *“139 once Start returned”* came from the 1-generation run, where `guidrive`'s legacy direct-invoke probe ran `slotStartEvolution` a second time and its `putAllIntoExperiment()` refreshed the LCD. That probe is now suppressed whenever `SIGEL_GENERATIONS` is set — it would also start a second evolution over the population being compared. **The fitness curve** is `experimentHistory`, one entry per generation, and it grew by exactly 3 to 139 entries, gen 1..139 contiguous, on both machines. **Statistics** were not driven — `update statistics` is still an unclicked toolbar action, as it was after C11d |
| ~~**`MT_GUI`**~~ **DONE in C11d** — the MetaGP window and its six pages driven; a C7-class locale defect found and fixed. ~~**`SIGEL_SlaveGUI`** is still undriven~~ **DRIVEN 2026-09-03** | MT_GUI's toolbar actions are still unclicked, but the slave's simulation window is now driven on both versions. **It is reachable with NO PVM and no master**: `sigel_slave -visualize <exp>` (`sigel_slave.cpp:143-152`) loads an experiment, takes individual 0 and opens the window; `guidrive`'s `slavegui` scenario does the same in process, since it already links `SIGEL_SlaveGUI`. **THE ROBOT MUST BE PREPARED FIRST** — `SIG_RobotRenderer`'s constructor walks every link's geometry, which is null until `instantiateGeometries()` has run, so passing the experiment's robot straight in SEGFAULTS; the slave copies it and calls `prepareDynaMechs()` first (`sigel_slave.cpp:245-269`) and the scenario mirrors that exactly. Matching 1.3 on: title, two tabs, all five checkbox states on arrival, the render combo's three items in order, `frameDelaySpinBox` range 0..1000, and the double-spaced `"0 hours  0 min.  0 secs."`. **A REAL USER-FACING DEFECT WAS FOUND HERE AND FIXED — see §9 item 4**: the `Navigation` group collapsed to 90x37 and clipped its seven buttons and the position readouts out of existence. It was invisible to every widget-level probe, which found the buttons present, correctly sized and at 1.3's exact offsets; only a SCREENSHOT of the running app showed the group was too small to contain them. **The other apparent defects were all the PROBE, not the port** — the ninth, tenth and eleventh of this phase: `showAncorPointsCheckBox` cycling 0→1→2 is `tristate=true` **in the pristine 2003 form**, byte-identical to the converted one; the empty X/Y/Z labels are `signalPosition` being emitted from `paintGL()` (`:777`), which never runs offscreen — **on a real display the view renders correctly and the labels read 0/1/0, matching 1.3 exactly** (verified 2026-09-03 on X11: GL initialises clean, 4511 distinct colours in the captured window). *`QT_QPA_PLATFORM=offscreen` gives no FBO, so ANY conclusion about the 3-D view from the offscreen harness is worthless; use `DISPLAY` + `QT_QPA_PLATFORM=xcb`. Under Wayland the menu popups fail and the scenario cannot even open an experiment.* (`QOpenGLWidget: No fbo, cannot render`); and Play appearing dead was a 250 ms `frameDelay` the probe itself had just set, giving ~0.16 simulated seconds in a display quantised to whole seconds — restored, Play reaches 9 min 46 s in 30 s. **NEITHER SIDE CAN SEE THE GL AREA**: it does not render here, and the oracle's capture of that region returns a constant image while the scene demonstrably moves. Anything about the rendered view is unobserved on both, and must not be read as agreement. **A CLAIMED DIVERGENCE HERE WAS WRONG AND IS WITHDRAWN.** `yawSlider` and `pitchSlider` were reported as ignoring groove clicks where 1.3 pages them. They do not. All four sliders page-step in both directions, exactly as 1.3 does. **The fault was the probe, for the fourth time in this module.** Every click used the middle of the slider's SHORT side. That axis was never varied. A slider with tick marks on one side has its groove OFF-CENTRE: `yawSlider` is 20 px tall with a groove at y 3..10, and the widget middle, y=10, sits at the edge of the band that responds. Clicking y=3..9 pages it +36 every time. `distanceSlider` and `ambientLightingSlider` worked only because their tick side, or absence of ticks, happens to put the groove under the middle. **The diagnostic that found it had the same bug one level down:** the `QStyleOptionSlider` handed to `subControlRect` did not set `tickPosition`, so the style returned a tickless groove — identical rects for two vertical sliders whose ticks are on opposite sides. With `tickPosition`, `tickInterval` and `upsideDown` set, the grooves come back correctly at x 3..10 for pitch and x 10..17 for distance, and all four move. **Lesson for the next probe: take the cross-axis from `SC_SliderGroove`, never from the widget's middle, and populate the style option fully before trusting any rect it returns.** **BUT A SMALL REAL DIFFERENCE FALLS OUT OF IT, and it is NOT a probe artefact.** Qt 2's Motif slider treats the WHOLE widget as clickable: the oracle scanned every cross-axis offset and got a clean page step at **all twenty** of `yawSlider`'s y values and all twenty of `pitchSlider`'s x values. Qt 6 honours the groove sub-rect only — measured here, clicking 80% along `yawSlider`: y=3,5,7,9 page it +36 and y=1,11,13,15,17,19 do nothing. So **clicking the outer edge of a ticked slider pages on 1.3 and does nothing on this port**. Nobody is likely to notice, and it is a Qt framework behaviour rather than anything the conversion did, but it is a fidelity difference and it is the reason the oracle's "all four move on 1.3" was true while saying nothing about where the probe was clicking. *This cost a commit that had to be reverted, and it was caught only because the oracle insisted a cheaper explanation be excluded first — it then pushed further and measured the write-back on 1.3, which is what showed the first lead could not be the cause.* |

**Reproducing the evolution comparison.** `guidrive.cpp`'s `evolution` scenario
takes **`SIGEL_GENERATIONS=N`**, which selects `Generation` in
`comboboxTerminationBy` and types N into `spinboxByGenerationNumber` on the
**Evolution control** tab, then reads the value back OUT of the experiment
(`model=1`) rather than trusting the widget — `slotStartEvolution` calls
`putAllIntoExperiment()` before it runs. It saves the evolved experiment through
`File > Save Experiment` at the end, because a run that leaves no artefact
cannot be diffed. The combo's items are User / Time / Generation / Time-or-
generation, which is **not** the enum order (`byTime`, `byGeneration`,
`byTimeGeneration`, `byUser`): index 2 maps to enum 1
(`SIGEL_MasterGUI/SIG_GPParameter.cpp:155-169`). The old `SIGEL_RUN_LONGER`
duration path is kept as the `else` branch.

*The run input is BUILT, not shipped: `data/Experiments/twoBasesSimpleFitness1.exp`
with the GP `RANDOMSEED` — the **second** of the two, the first being the
simulation's (`SIG_GPParameter.cpp:387` against
`SIG_SimulationParameters.cpp:102`) — set to 12345, the eight 2003 PVMHOST lines
replaced by one local host at max-processes 1, and the paths rewritten. The
oracle proved its own reference inputs derive from that same shipped file, and
`expstruct.py` confirmed the reconstruction independently: identical
`ORDER`/`PROGRAMS` and the same 21185 structural lines.*

**`expstruct.py` is the comparison tool**, and it is in the repo because two
machines had to run identical logic rather than two readings of a spec. It
prints the counts, a per-individual line in file order, and `ORDER`/`PROGRAMS`/
`SHAPE`/`HISTORY` hashes, and it **drops every float by construction** — `--audit`
prints every line it refused to look at. `--selfcheck` asserts both halves of
the property everything rests on (blind to fitness, sighted on structure) and is
a `check.sh` section costing 0.34 s; teeth-tested by disabling both fitness
filters and by blinding the program matcher. **Read `ORDER`/`PROGRAMS` with
care: they are all-or-nothing and hid an 87%-identical population behind a
single mismatched hash.** When they differ, get the per-individual names before
concluding anything.

**WHAT THE FINGERPRINT CANNOT SEE, and it is not a small surface.** The `FLOAT`
net that keeps fitness out also drops **every live input that contains a float**:
the 13 `<CMD> CommandParameters <f>` mutation step sizes, the `Link` /
`Geometry` / `Material` morphology lines, and the environment constants. None of
those reach `ORDER`, `PROGRAMS` or `SHAPE`. Twenty-two such lines in
`twoBasesSimpleFitness1.exp`, forty-two in `octopusSimpleFitness.exp`. The
concrete danger, found by the oracle rather than reasoned about: **`MOVE
CommandParameters` is `0.01` for twoBases and `0.1` for octopus** — a tenfold
difference in mutation magnitude on the one command with large integer operands.
A reconstructed input that got that wrong would evolve differently and **every
hash exchanged between the two machines would still agree**.

*How that was closed for the runs recorded here, and how to close it again:* not
by hashing the surface but by an exact diff of it, between the shipped experiment
and the built input — identical for both models on both machines, so neither
side's transformation touched any of it. **The cross-machine half needed no hash
exchange at all**, only that both sides derive from the same ancestor:
`twoBasesSimpleFitness1.exp` is md5 `35bcdb3a7a2bb6c2af7ccf964761e87e` and
`octopusSimpleFitness.exp` md5 `d6a73c806137f492f285914e0e131c49` on both. Same
ancestor plus surface-preserving on each side means the surfaces are identical —
two `md5sum`s and a `diff`, instead of writing and cross-validating a
normalisation. **The oracle's diff came back identical UNSORTED, which is the
load-bearing part**: had its inputs been GUI load-and-save products the block
order would have permuted and only a sorted comparison could have passed. That works only because these inputs are built by
rewriting seed/PVMHOST/path lines TEXTUALLY. **Anything rebuilt through the GUI's
load-and-save must be normalised first**: 1.3 permutes the `CommandParameters`
block, the `Link`/`Geometry` order, and even the named attribute groups WITHIN a
single `Link` line (`...Normal x y z ...Dir x y z` comes back `...Dir` first),
all values preserved. A naive hash over that surface mismatches for pure
`QDict`-bucket reasons, and Qt 6 permutes differently again — expected, and not
a divergence.

**Do it as C11 with the same method, not a rewrite.** `guidrive.cpp` already has
the machinery — modal interception, context-menu posting, a watchdog, PVM
teardown, the master-object assertion. New scenarios are additions to it, and
each one that stabilises earns lines in `guibehaviour-baseline.txt`.

**Order, most-likely-to-find-something first**, on the evidence of what C10
actually caught: ~~(1) the five parameter pages~~ — **done, and the guess was
right: the regression it found is in exactly the spin-box/validator shape C6
and C7 had already been bitten by**; ~~(2) the Import/Export round trips~~ —
**done, and the guess was right again: byte-comparable files caught a defect
that had corrupted a committed baseline**; ~~(3) the remaining dialogs~~ — **done, and the guess held a third
time: the defect it found needed no invalid input at all**; ~~(4) `MT_GUI`~~ — **done, and it found the same defect class C7 had
fixed everywhere else**; ~~(5) the evolution path, last~~ — **done, and it was the one place the guess did NOT hold: the defect-hunt found no port defect, and what it found instead was that this section's own comparison premise was wrong.** *This said it "needs a
throughput answer before it can be observed at all".* **THE ANSWER ARRIVED
2026-09-02 and it is measured, not estimated** — see the open-items table.

**THE PORT EVOLVES. Measured 2026-09-03, and it is the first measurement of OUTPUT this project has made.**
Population 100, 30 generations, twoBases with `SimpleFitnessFunction` (fitness = distance / simulated seconds,
i.e. speed). **Best fitness 0.063794 → 0.141625, never decreasing at any generation.** Mean 0.011711 → 0.046059.
Every individual was evaluated on this machine — the pool's stored fitness was cleared first through
`Individuals > Reset`, because survivors otherwise keep their 2003 i386 values and a curve drawn from a mix of
two architectures means nothing. 4 slaves, 61.6 s/generation, 1,849,022 ms total; `POOLGENERATION` 136 → 166.

**No conclusion is drawn from those numbers beyond the one question that is answerable:** does best fitness
improve for a reasonably large population over a reasonable horizon. It does. GP is a randomised process with
many parameters; anything further — whether 0.14 is a plausible value, whether the parameters are well chosen,
what a flat stretch would imply — is not answerable from one run and must not be asserted here.
*For future runs the starting pool should be RANDOMISED rather than a fitness-reset of the shipped programs.*

**V4 and the cross-machine evolution comparison are DROPPED.** Both are exact replication across an x87/IEEE
boundary that this section already says cannot be compared, and the attempt confirmed it: the matching counts
are forced by `createTours` and the contents cannot agree. V3 stays and is SATISFIED — it is same-box
determinism, not replication, and the oracle demonstrated it twice (serA≡serB, octGateA≡octGateB).

**Open after C11c — five items, of which items 2 and 5 are now closed.** The `QSpinBox` divergence is closed: **D28**
accepted it. The Import/Export item is closed by C11b.

| # | open item | blocked on |
|---|---|---|
| **1** | ~~**`pagesave` and `roundtrip` have no gate.**~~ **BOTH GATED 2026-09-03 — `pagesave vs 1.3` and `roundtrip` inside `gui behaviour`. This item is CLOSED.** The blocker was never the code — it was that a baseline captured from the port is only the port compared against itself (§7). **The oracle supplied the reference**: load `twoBasesSimpleFitness2.exp`, visit no page, change nothing, save once, and send everything above `POPULATION BEGIN{`. **The port writes those 192 lines byte for byte — 2069 bytes, sha256 `a327150c…` on both machines.** *A first version of this row called it "the project's third cross-machine anchor and the first that is parameter text rather than a byte count". **Both halves are wrong against this document's own text** and are withdrawn: C10's exported program was pinned by `sha256 f9407517…`, not a byte count, and C11b already had seven of eight exports byte-identical to 1.3's, `.gpp`/`.sip`/`.env` among them — parameter text, cross-machine. What is actually new here is only that the SAVE path is now covered, where C11b covered the EXPORT paths.* So a failure in the base half is a regression against 1.3. *The capture carried a positive control — a second save after one parameter change differed in exactly one line — and the oracle sent it as a readable `key / value` rendering rather than raw bytes, fearing the transport would reflow 192 short lines; expanding every ` / ` back to a newline reproduces its line count, byte count, sha256 and all seven blank-line positions, so the reconstruction is proven rather than assumed.* **`LanguageParameters` is checked separately and would otherwise have been missed**: it sits at line 133288 of the saved file, far below `POPULATION BEGIN{` at 193, so the block covers eleven of C11a's twelve edits. That was found because the oracle reported where `POPULATION BEGIN{` had moved to — not by reading the code | **`roundtrip` GATED 2026-09-03 too, and gating it found that IT COULD NOT FAIL.** It needs no 1.3 reference — it is export-import-export on one machine, and its point is that the *reader* undoes a mutation, so a no-op importer cannot pass. **It did pass.** With `gpExperiment.gpParameter.readFromFile()` commented out entirely, the scenario printed `export1 1299 bytes 88e851e2…`, `export2 1299 bytes 88e851e2…`, **`ROUND TRIP STABLE (and the import undid the change)`** — those exact words, on a gutted importer. **Cause:** `mutate` types into a *widget*, but `importFrom` ends with `getOutOfExperiment()`, which refreshes every widget *from* the experiment — and the experiment never saw the typed value, because only `putIntoExperiment` moves it there. So the change was undone before the second export whether the reader ran or not. **This is C11c's "the round trip could not fail" a second time, in a different mechanism, in the probe C11c had just fixed.** **Fixed by clicking a View page the mutation did not touch**, which runs `putAllIntoExperiment()` through the path a user actually takes — a tree selection change calls it (`SIG_ExperimentListView.cpp:323`) and every page switch goes through `selectItem()` → `setCurrentItem()`. *A direct call to the slot works too and was measured not to contaminate the exports, but this file already records one reading that was wrong BECAUSE a direct invoke ran that function at a moment no GUI action would; using the real path removes the question instead of answering it.* **Two claims made while fixing this were themselves wrong and are withdrawn:** that Language-Parameters' export slot calls `putIntoExperiment` zero times — `slotLanguageParameterExport` calls it once, and the earlier count came from a grep for the *plural* `slotLanguageParametersExport`, which matched nothing and reported 0; and that the aggregator "pushes all five pages" — it pushes `experimentView`, `gpParameter`, `simulationParameter`, `languageParameters`, `environmentView`, so **not** `robotView` and **not** the population. Population needed no push at all: `Individuals > Delete` changes the experiment directly, so that entry was falsifiable already. The real reason the four widget-typed entries needed it is **ordering**, not a missing call. *The fix changes NO output on a clean tree — the two runs are byte-identical — and makes the gutted reader report `ROUND TRIP *** CHANGED IN CONTENT ***`, export2 1301 bytes against 1299. Invisible when passing, decisive when failing, which is what a teeth test should look like.* Deterministic across two runs, 103 s, the most expensive scenario in the gate |
| **2** | ~~**One C11 item left — the evolution path**~~ **DONE 2026-09-02. The evolution path was driven end to end. ITS HEADLINE CLAIM WAS THEN LARGELY WITHDRAWN BY A THIRD REVIEW ON 2026-09-03 — read this row's second half before citing it.** Three generations were run through the GUI on both machines from the same input, single-slave, and diffed with `expstruct.py`. **What was claimed:** that every COUNT matched — `NEXTIDENTIFIER` 12552 (245 identifiers over three generations, per-generation deltas 80/84/81), `POOLGENERATION` 139, `INDIVIDUALS` 100, `HISTORY` 139 — while contents differed, and that this showed the GP drawing from the randomiser identically on i386/x87 and aarch64/IEEE. **WHAT IS ACTUALLY TRUE: THOSE COUNTS ARE VERIFIED HERE TO BE FORCED, AND CANNOT BE EVIDENCE OF ANYTHING.** Identifiers are taken in the tournament CONSTRUCTORS' initialiser lists — `SIG_GPSimpleTournament.cpp:36`, `SIG_GPMutationTournament.cpp:36`, and **two** at `SIG_GPCrossOverTournament.cpp:39-40` — all of which run inside `createTours` (`SIG_GPManager.cpp:232`) **before any tournament runs and before any fitness is read**. The count is constant: `:723` calls `createTours(getTournamentsPerGeneration() * getPopulation().getSize())` = 0.5 × 100 = **50 tournaments every generation**. So the per-generation delta is `50 + (number of crossover tournaments)`, chosen by one integer draw per tournament. A replay of `SIG_Randomizer`'s LCG from seed 12345 in forty lines of Python — no fitness, no physics, no machine — reproduces generation 1's **80** exactly, and octopus's **62**. *A number predictable from the seed alone cannot show that two runs agree.* The other three are worse: `POOLGENERATION` is 136+3, `HISTORY` is one append per generation, and pool size cannot change at all (`SIG_GPPopulation.cpp:189`'s resize is commented out). **And generation 1 cannot diverge in the first place** — `evalNewIndis` evaluates only individuals with fitness < 0 (`SIG_GPManager.cpp:429`) and every individual in the input has positive fitness, so no simulation runs before its tournaments. Estimated probability that all three deltas agreed even if the port were maximally divergent: **~98.6% for twoBases, ~60% for octopus.** About one bit of evidence, and the octopus 'replication' replicates nothing. **CONFIRMED INDEPENDENTLY BY THE ORACLE, from its own measurements and without this machine's replay or source reading.** Since the type of each of the 50 tournaments is one draw against the summed weights (`SIG_GPManager.cpp:267`, `getRandomInt(totalProbCount) + 1`), the delta is `50 + Binomial(50, XOVERPROB/1000)` — computable from the .exp alone: *twoBases* `XOVERPROB 670` predicts 83.5/generation, 250.5 ± 5.8 over three, **measured 245 (z = −0.96)**; *octopus* `XOVERPROB 300` predicts 65.0/generation, 195.0 ± 5.6, **measured 193 (z = −0.36)**. Two experiments with crossover rates 0.67 and 0.30, both inside one standard deviation, and the rates implied by the data (0.633, 0.287) recover the file's. **Nothing about the port, the architecture, the physics or the fitness enters that prediction** — the measured totals are fully explained without the port existing. | **What survives, and it is worth keeping.** (a) **Contents cannot be compared across these two architectures** — that part was always right, and the tournament decides on a bare `>=` between two physics-produced doubles (`SIG_GPSimpleTournament.cpp:78`, `SIG_GPMutationTournament.cpp:76`, `SIG_GPCrossOverTournament.cpp:101` and `:114`). (b) **`SIG_Randomizer` is architecture-independent by construction** — `next` is `unsigned long`, 32-bit on i386 and 64 on aarch64, but the LCG's low 32 bits are congruent at both widths and `getRandomInt` returns only bits 16-30. That is a stronger statement than the counts ever made, and it is why they could not have differed. (c) The two runs executed **the same 50 tournaments per generation on the same pool positions**, and 13 slots differ — a ~3-4% flip rate, which is what the pool's four-orders-of-magnitude fitness spread predicts under the ±45% perturbation §7 measures. **The 87 of 100 figure is also restated: 21 slots are untouched survivors of the input, so agreement over the slots actually REPLACED is 66 of 79 = 84%**, and the surviving 21 skew hard to high fitness (median 0.842 against 0.035), so survivorship does much of the work. **Not ruled out:** a different tournament EXECUTION ORDER across machines would give identical counts and different contents with no float comparison involved — and this project has already measured that mechanism at 64 of 100 names between two 8-slave runs on ONE machine. `runA==runB` and `serA==serB` show each machine is internally stable; they do not show both used the same order. **The measurement this should have made, and did not:** a draw counter in `SIG_Randomizer` dumped per generation on both sides. One counter and one `printf` would measure lockstep directly instead of inferring it from a quantity that was ~98% certain to agree. **But it can only ever be HALF a measurement: the oracle has no source tree** — its side is 2003 i386 binaries, pixmaps and support libraries, and every structural fact it has supplied came from the running program, the file formats or `strings`. So `SIG_Randomizer` is instrumentable here and not there, and any lockstep claim rests on this side's counter plus whatever is externally observable on that one. Design no check that assumes both halves can be instrumented. **Sample is one seed, two models, and effectively two generations of divergence on an already-converged 136-generation pool.** |
| **3** | ~~**`QHashSeed::setDeterministicGlobalSeed()` was never added to `sigel.cpp`.**~~ **DONE 2026-09-04, and it was NOT purely bookkeeping.** The mis-scoping half of this row was right — only `sigel` links the three `QHash`es (`SIG_Experiment`'s `widgetDict` and `menuDict`, `SIG_ExperimentListView`'s `experimentDict`), and `sigel_slave` links `GUI_SLAVE`, which has none, so the call goes to `sigel.cpp` alone and §10's "and `sigel_slave.cpp`" stays withdrawn. **The "one iteration site is destruction, which is order-insensitive" half was wrong on both counts.** There are **two** iteration sites, both in `~SIG_Experiment`, and the first is `for ( QWidget *w : widgetDict ) widgetStack->removeWidget( w )` — **and that stack is SHARED**: `SIG_ExperimentListView` hands its own `widgetStack` to every experiment it constructs (`SIG_ExperimentListView.cpp:77`). So with a second experiment still loaded, the order in which one experiment's widgets leave the stack can decide **which page is current afterwards**, and an unpinned seed makes that differ between runs of the same binary. *Not data, and not something a gate would see — D26 measured that no hashed container's order reaches a file — but it is run-to-run UI non-determinism, which is more than bookkeeping.* One line, at the top of `main()`, removes the question | closed |
| **4** | **Six dropped size constraints from C2 were never restored — MEASURED 2026-09-03 AND THEY ARE COSMETIC, as §9 originally said.** *This row was briefly reclassified as user-facing after a sibling defect hid real controls; an independent review measured all six and the reclassification was WRONG. Reverted here.* The six are `Layout32/33/28` in `MT_IndividualWidgetBase.ui` (each a **maximumSize** width ceiling of 130 — a dropped ceiling can only let a widget grow WIDER, it cannot clip anything), `Layout60` in `MT_PopulationWidgetBase.ui` (a 200 px minimum, of which 190 is covered by `individualListView`'s own kept minimum — net loss 10 px), and `Layout22`/`Layout22_2` in `MT_StatisticsWidgetBase.ui` (which carried Qt's literal defaults, `minimumSize 0,0` and `maximumSize 32767,32767` — **no-ops**). All three forms were rendered at design size and at their declared minimum: not one widget is clipped by any ancestor. | open, and cosmetic. **The user-facing class is a DIFFERENT one and it is now closed**: a container whose children are placed by ABSOLUTE geometry, which Qt 2 sized from those children and Qt 6 sizes from a sizeHint that ignores them. **Exactly two such containers exist in all 20 converted forms**, and both are fixed: `GroupBox6` ("Navigation", 90x37 against a needed 220x331 — seven buttons and three readouts hidden) and `groupboxDirectory` ("File conventions" in the movie dialog, 465x37 against a needed 401x99 — file prefix, leading-zeros, image format and JPEG quality **fully hidden**, the directory row 83% visible). Neither was a dropped value: the pristine forms carry no size either, so both needed a NEW minimum. Gated by `no clipped controls`, teeth-tested by reverting each fix |
| **5** | ~~**A use-after-free in `SIG_SimulationVisualisationWidget.cpp:414` is now reachable.**~~ **FIXED 2026-09-02.** `visualisation = nullptr;` now sits between the `delete` and the `new`. The premise was re-verified rather than inherited: `SIG_Simulation.cpp`'s `default:` case throws for `SIMULATIONLIBRARY 0`, the widget has **21** `visualisation->` dereferences behind **14** `if (visualisation)` guards, and the throw unwinds out of `visualizeThis()` between the two statements. **The dereferences are not reached WITH A DANGLING POINTER today — and the first version of this paragraph got the reasons wrong in three ways, corrected here by a fresh-eyes review.** (a) `sigel_slave.cpp:293` is **not** the sole caller: `SIG_SimulationVisualisationWidget.cpp:478` calls `visualizeThis()` from `slotStopSimulation()`, a live slot wired to the viewer's Stop action (`SIG_SimulationWindow.cpp:81-83` → `SIG_SimulationWidget.cpp:264`), and that path runs **inside** `a.exec()` with no try/catch. (b) The widget's destructor is empty only in the DERIVED class — `visualisation` is a **base-class member** (`SIG_VisualisationWidget.h:233`) and `SIG_VisualisationWidget::~SIG_VisualisationWidget()` does `delete visualisation` (`SIG_VisualisationWidget.cpp:60`), so destruction after a throw **is** a double free; it does not bite on the slave path only because `simWindow` is leaked past the catch's `return 1`. (c) The 14 guards sit in `paintGL()`, `makeTimeSteps()` and the navigate/checkbox slots, and `paintGL()` runs every frame of any successful run — so they are reached constantly; what they never hold today is a FREED pointer. **Why it is still unreachable, on the corrected argument:** every other caller is downstream of a first `visualizeThis()` that must have SUCCEEDED (a throw there exits before `a.exec()`), and nothing in the slave calls `setSimulationLibrary`, so the same parameters cannot start throwing later. **The line is kept and gated** — one line, and it is now the thing standing between a throw and a double free. *`physics_backends.md` cited lines 376-381; the site is at 414 — line drift, not a second site.* | closed. Gated: `check.sh`'s **`freed-pointer null`** flattens comments and newlines and requires every `delete visualisation;` to be followed by a null assignment — teeth-tested by deleting the line (1 pass → 1 fail). **Deliberately NOT generalised.** The tree has many `delete x; ... x = new ...` pairs with no intervening null, and the others (validators, program parts) are benign, so a blanket rule would be noise rather than a gate. *A count of "19" stood here and is withdrawn: it depended on an unrecorded lookahead window, and re-measuring gives **20** at 10 lines and **14** at 3. A number nobody can reproduce is worse than none.* |

**Font drift — six forms declare a minimum smaller than Qt 6's layout needs.** Inherited from 1.3 unchanged, so these are
not port regressions, but Qt 6's larger default font pushes the layouts past the old figures. Declared against measured
`minimumSizeHint`: `MT_StatisticsWidgetBase` **220x390 against 427x555** — the worst, and it sits in the MetaTrainer's
`QSplitter` (`MT_MainWindow.cpp:125,151`) so a user can actually drag it that small, where the labels compress to 3-8 px
tall and the buttons to 42x8. Nothing is hidden or unclickable, but it is unreadable. Then `SIG_SimulationWidgetBase`
780x640 against 373x752, `MT_EstimationWidgetBase` 230x260 against 323x274, `MT_SelectionWidgetBase` 410x240 against
472x301, `MT_IndividualsWidgetBase` 440x362 against 338x404, `MT_SearchWidgetBase` 240x400 against 228x420. Measured by
rendering each form under Qt 6.10.2, not derived.

**Everything else in the 20 forms is clean, and was checked rather than assumed:** 623 widgets, 55 group boxes, 15 tab
pages, 89 laid-out containers, 34 grid layouts. No negative coordinates, no zero-sized geometry, no `min > max`, no child
outside its parent's declared size, no two grid items sharing a cell, every tab page carries a layout. The nine
`enabled=false` widgets are all deliberate and re-enabled in code.

**Hazards a follow-up must inherit, all of which cost time in C10:**

- **Plain `grep` silently skips nine Latin-1 files** including
  `SIG_MainWindow.cpp` — see the TRAP below. Use `command grep`.
- **Four files are Latin-1 and some are mixed CRLF/LF.** Edit them in BINARY
  mode. A text-mode Python edit silently stripped 934 CRs from
  `MT_Controller.cpp`; `check.sh`'s `encodings` section caught it, but only
  because that gate exists.
- **Any harness must link `$(MASTER_OBJ)`**, or the Clean `SIG_GPExperiment`
  leaves `mtController` uninitialised and produces a convincing false crash.
- **`QTest` is not a mouse.** Whatever is driven, the record must keep saying so.
- **`slotStartEvolution` BLOCKS, so a sampling loop written after the click sees
  only the finished state.** `gpManager->start()` runs the whole evolution
  inline (`SIG_Experiment.cpp:282`, with `slotEvolutionStopped()` on the very
  next line), and the GUI survives only because
  `SIG_GUIGPManager::haveABreak()` calls `qApp->processEvents()`. Anything that
  must observe a RUNNING evolution has to be armed BEFORE the click — the
  `evolution` scenario uses a `QTimer` for exactly this. The oracle hit the
  same shape driving 1.3 with XTest and armed its capture first.
- **An unset `SIGEL_ROOT` segfaults every slave on spawn IN 1.3** — and PVM
  tasks inherit *pvmd's* environment, not the master's, so it can be set for the
  master and still absent for the slave. Set it for the daemon, not just for the
  GUI. *This first said `sigel_slave` builds the path from an unchecked
  `getenv()`. Wrong file and wrong tense: `sigel_slave.cpp` contains no `getenv`
  at all. The code is `SIG_Environment::generateTerrain()`
  (`SIGEL_Environment/SIG_Environment.cpp:441`), reached from
  `loadDynaMechsEnvironment()` at `:400-402`, and **this port already guards it**
  at `:450-455` — `v1.3-pristine` has the unguarded
  `std::string terrain(sigelRootCString)`, which is the oracle's crash. Corrected
  by review; the guard was observed firing during this step
  ("Generate terrain: SIGEL_ROOT is not set") and not recognised at the time.*
  Residual worth knowing: `:409` still builds a `QString` from the same unchecked
  `getenv`, harmless in Qt 6 but it silently yields `/Terrain.ter`.
- **`pvmd` resolves the slave executable relative to the directory in the
  PVMHOST line**, not `SIGEL_ROOT`. A wrong directory gives
  `pvm_spawn() failed ... (0/-7) - Executable ... is not found`, once per
  individual. Here the PVMHOST directory IS the build directory, so the two
  coincide; that is a property of this setup and not a general one.
- **`tearDownPvm()`'s `pvm_halt()` BLOCKS FOR EVER at exit, so every evolution
  run ends by being killed rather than by exiting**, and `timeout` reports 124
  or the shell reports 143/15. The scenario itself has completed and its files
  are written by then, so the artefacts are good — but the exit status of an
  evolution run means nothing and must not be read as pass/fail. Located with
  gdb rather than guessed: `guidrive.cpp:349` → `pvm_halt` → `msendrecv` →
  `mroute` → `mxfer` → `select()`, in `PvmGuard::~PvmGuard` after `main`
  returned. **Pre-existing, and deliberately not changed here** — `pvm_halt()`
  is what stops the daemon this process started, and the comment above it
  records that dropping it left `pvmd3` and its slaves running. Measured: no
  stray `pvmd3` survives the kill, so the current behaviour is safe if untidy.
- **A `printf` before an early `return` in `guidrive` is LOST unless flushed on
  that path.** stdout is block-buffered when redirected to a file, and because
  of the `pvm_halt` block above the process never reaches exit to flush it. An
  assertion added during this step fired correctly and its message vanished;
  it was only visible under gdb. Flush on the failing path, not after the block.
- **Above one slave there is no baseline at all.** Two same-seed 8-slave runs
  on the oracle's single machine differed in 64 of 100 individual names.
  Whether that is GP evaluation order or the PVM layer is UNMEASURED. Any
  evolution comparison must pin PVMHOST field 2 to 1.
- **Verify every finding before acting.** C10: three apparent defects were the
  probe, two oracle readings retracted. C11a: three more, all the probe.
- **`QTest` posts key events STRAIGHT AT THE WIDGET, focus or no focus**, so a
  keyboard probe says nothing about focus. On 1.3 a groove click does not
  focus a slider, so arrow keys never reach it. Drive the path both sides
  share — for a slider, a groove click or a handle drag.
- **A page remembers its tab.** The View toolbar does not reset the tab widget.
  Click the tab you want.
- **Each file a scenario saves shifts the load dialog's row positions for the
  next run**, so a multi-run sequence can silently re-load what it just
  wrote. Check the line count and the six-space count.
- **Order the probes so a toggle cannot grey a field before it is typed into.**
  One click on a radio greyed an integer field and turned five subsequent
  probes into false negatives.
- **`QFileDialog` navigates as a path is typed and strips the directory out of
  the field.** Use `acceptFileDialog()`; it types an absolute path and
  re-asserts it. Do not hand-roll a second one.

**The oracle is the reason this works.** Both C10 defects were confirmed against
the running 1.3 binary *before* anything was changed — in one case that
confirmation is what distinguished a Qt 6 regression from a 1.3 defect that had
to be preserved. A C11 without a working oracle would be much weaker, and should
say so rather than proceed as if not.

### Ownership hazards Phase C inherits (was: the Phase B audit)

**No `setAutoDelete` call remains anywhere — this said 11, and C6/C7 removed the last of them.** The 19 occurrences of the name left in the tree are all COMMENTS recording what the Qt 2 code used to free, and the same is true of every remaining mention of `QDict`, `QArray`, `QListViewItem`, `QPtrList` and `QCString`. The compiler is the proof: none of those types or methods exists in Qt 6, so a live one could not build, and the whole tree builds.

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

Measured on 6.10.2 with 1.3's own line `eiche 2 1 "/home/pg368b/ross/…"`: the
first read returns `' '` where Qt 2 returned `'"'`, the second returns `'"'`
where Qt 2 returned `'/'`, the loop therefore never executes, and **every PVM
host parsed from an `.exp` file gets an empty slave directory.**

**It also drops spaces inside the quoted path, and that is preserved.** Each
read skips whitespace, so Qt 2 turns `/tmp/with space` into `/tmp/withspace`. A
2003 defect; converted, not fixed.

**`SIGEL_GP` was converted in Phase D and has been green on all four gates ever
since.** Nothing could see this: it is not a compile error, and no gate reads a
`PVMHOST` line. **That is the actual lesson** — the gates cover what the gates
read, and a file format the program parses but no gate opens is covered by
nothing at all. `check.sh` now has a `parsers` section that round-trips a
`PVMHOST` line; it fails on the naive conversion.

*Found while answering an unrelated question about a checkbox two files away.
The x86 box reported that `Edit host`'s `Enable host` box was unchecked for a
host the file marks enabled; chasing that led to the parser. **The finding was
not in the thing being examined.***

*Sweep: three `>> buffer` sites, all in this one constructor. No other file in
the tree extracts a stream into a `char` or `QChar`.*

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
order `wickie, bube, birke, urobe, pappel, lithium, eiche, herz`, and
GP-Parameters → PVM draws them **exactly reversed**, `herz` first and `wickie`
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
form can pass every gate this project has while the code that fills it is what
holds the behaviour.

**`SIG_ExperimentItem.cpp:64` is a second, independent trap in the same file.**
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
`absoluteDriveNo` stays in `[0, 2^w)` and nothing overflows, so no gate sees it.
**Not fixed here**: the wrap decides which actuator a `MOVE` drives, so changing
it changes simulation results against 1.3. It predates the port.

### `QTextStream` default codec

232 sites. API unchanged, but Qt 2 defaulted to Latin-1 and Qt 6 to UTF-8. A
no-op over this corpus — the 2003 binary's own `.exp`, `.rrb` and `.wrl` output
is pure ASCII. Becomes live the moment a non-ASCII experiment file exists.

### ~~Toggling containers (Phase C)~~ — CLOSED BY C7

**Both are now `QHash`es owning nothing** (`SIG_ExperimentListView.cpp:63-65`,
`SIG_Experiment.cpp:183-189`), so there is no `autoDelete` to toggle and
nothing here needs per-site thought. The table is kept because it records
what the Qt 2 code did. *It said:* two containers flip `autoDelete` at
runtime — Qt 2's "remove without deleting" idiom, which a single destructor
free does not reproduce; both GUI, both needing per-site thought:

| Container | File | Toggles |
|---|---|---|
| `experimentDict` | `SIG_ExperimentListView.cpp` | 3 pairs (102/110, 229/236, 249/256) |
| `widgetDict` | `SIG_Experiment.cpp` | 1 pair (206/210) |

### Reference data and reference output

Experiment and robot files are **not** in any tarball — separate downloads,
verified live 2026-08-18:
`sigel.sourceforge.net/download/experimente/experiments.tar.gz` (12 `.exp`) and
`.../robotermodelle/robots.tar.gz` (7 models, `.rrb` + `.wrl`).

Both downloaded 2026-08-22 to `data/`, which is untracked: `data/Experiments/`
holds the 12 `.exp`, and one directory per robot holds its `.rrb` and `.wrl`.

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
dictorder gate is unaffected. **`build/pvm_link` lost it** — `nm -C` finds the
symbol once in `sigel_eval` and **zero** times in `pvm_link`, because
`SIG_GPFitnessTrainer.o` used to pull the shim's per-TU initialiser onto that
link line. Unobservable today: **0 `QHash`/`QSet`/`QMultiHash` in the converted
core**, measured, and a build with a genuinely random seed reproduces
`dictorder-baseline.txt` exactly (D26 review action).

~~**PHASE C MUST ADD `QHashSeed::setDeterministicGlobalSeed()`**~~ **DONE 2026-09-04, in `sigel.cpp` only — see §9 item 3, which also corrects the reason.**
Phase C closed without it, and the `sigel_slave` half is MIS-SCOPED: `GUI_SLAVE` links no `QHash`.
Do not act on the original sentence, kept below struck through:
~~PHASE C MUST ADD it TO `sigel.cpp` AND
`sigel_slave.cpp`**, as the first statement of each `main()`. Neither has it,
and both will link containers this port has not yet seen.

### `MT_Control` is not headless

A8 cut the header include correctly, but **`MT_Controller.cpp` is deeply coupled
to the GUI**: 23 `mainWindow->` accesses, it constructs `new MT_MainWindow`
itself, and it wires toolbar actions with `connect`/`disconnect`. It cannot
compile against Qt6Core alone until `MT_GUI` is ported.

This matters because `sigel.cpp:272` reaches `MT_Controller::startTimedEvolution`
on the headless `-me` path. Either the GUI wiring moves out into `MT_GUI`, or
`MT_Control` is reclassified as a GUI module. **Not decided.**

**AND ON THAT SAME PATH `qApp` IS A `static_cast` TO A TYPE THE OBJECT IS NOT.
STILL OPEN — C8 left it for C9 and C9 never closed it.** Measured 2026-09-03 by
preprocessing the real translation unit with the Makefile's own include set, not
by grepping a header:

- `sigel.cpp:178` handles `-mtevolve` / `-me`, and `:266` constructs a plain
  **`QCoreApplication`** on that path — deliberately, because it is the Qt 6
  spelling of Qt 2's `QApplication(argc, argv, false)`.
- `MT_Controller.cpp:125` calls `qApp->exit(0)`.
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
real reason nothing has bitten. It is UB either way. The fix is one line
(`QCoreApplication::instance()->exit(0)`), and it is left open rather than done
quietly because the `-me` path is undriven and this file's rule is that an
untested change to unrun code is not an improvement.

Same module: 4 of its 15 `QMessageBox` calls are **interactive** — the return
value drives a branch. Those cannot become console output.

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
| `Q2Array::sort()` | `memcmp` byte order | numeric | Three sites need ascending numeric order and break once any element reaches 256: `SIG_GPManager.cpp:304,311`, `SIG_AllIndividualsView.cpp:240`. Qt 2's own source says *"Qt 3.0: Add a virtual compareItems()"* |
| out-of-range array access | ~~warn, clamp to 0~~ **BOTH HALVES OF THIS ROW ARE WRONG, corrected 2026-09-03.** "2003 warned and clamped" is true of **`QGArray::at`** (`qgarray.h:108-117`, `msg_index(index); index = 0;`) and **false of `QGVector::at`** (`qgvector.h:85-92`, which warns and then *reads out of range*) and of **`QGList::at`** (`qglist.h:172-176`, which returns **null**). Three behaviours, and this row is the **fourth** recorded instance of fusing them — in the very table §10's corrected semantics row points at | ~~same, in the shim~~ **nothing clamps today**: `q2compat.h` went with D27, and `shim/` now holds only `fstream.h`, `iomanip.h`, `iostream.h`, `minmax.h`, `new.h`, `strstream.h`, `vector.h`. The port uses `value()`, which yields null — **safer than Qt 2, not equal to it** | The original reason still stands for why a clamp was not replaced by `Q_ASSERT`: it compiles to nothing under `QT_NO_DEBUG`, so a release build would corrupt memory silently where 2003 returned a wrong value. *Neither `QT_NO_DEBUG` nor `NDEBUG` is defined by the Makefile or `check.sh`, so `QList`'s assert is live in both build trees today* |
| ~~`SIG_ProgramLine.cpp:215-224`~~ | writes `element[no]` in the branch entered *because* `no >= size()` | **FIXED** — `:215-232` now guards `if( no >= 0 && no < int(element.size()) )` | Its own comment is `// ToDo: Exception!` |
| ~~`SIG_DynaSystem.cpp:266-268`~~ | deletes `dynaJoints[k]` while looping to `dynaDrives.size()` | **moot 2026-08-28** — the file is deleted with the Dynamo backend, `physics_backends.md` | The two vectors grew independently |
| `SIG_EarlyRunTermSimulation.cpp:97` | `QTime zeroHour;` | `QTime( 0, 0 )` | Same class as the other 11 `QTime()` sites but a declaration, so the first sweep's pattern missed it. `getMaxRecorderSteps` returned 2 instead of 182 — a factor of 91 on the denominator of three fitness functions. No shipped experiment selects them, so `replicate.sh` cannot see it |
| `sigel_slave`, `getenv("SIGEL_ROOT")` | dereferenced unchecked | to be fixed | Segfaults if unset; the SIGSEGV handler masks it with no core. Bites under PVM specifically — spawned tasks inherit *pvmd's* environment, not the master's |
| `SIG_GPPVMData.cpp:51` `sendQStringToPVM` | sends `str.length() + 1`, a **character** count, then sends `str.toUtf8()`, up to 4x longer in bytes | `qCStringBuffer.size() + 2` (D21; was `+ 1` on a `Q2CString`) | `getQStringFromPVM` sizes its receive buffer from that count and lets `pvm_upkstr` write the bytes in. 20 `ü` gives `heap-buffer-overflow ... in byteupk` under ASan; short strings survive only because `QList` over-allocates. Qt 2's `length()` was the Latin-1 byte count, so 2003 was right for its own data. **Changes the wire format for non-ASCII** — safe only because both ends are this file and no distributed run exists. Found by Phase P's P4, regression-tested by `pvm_link.cpp` |
| `SIG_GPIndividual.cpp:557-559` / `:647` | the writer emits `"\n      "` before `}HISTORY END;`; the reader takes everything up to that marker as content, so the separator becomes data | **preserved, not fixed** | Every save grows every `HISTORY` block by 7 bytes, linearly and without limit — 100 blocks is ~700 bytes per round trip. Measured on the 1.3 binary over three consecutive round trips (V8) and confirmed to be the same code here. Fixing it would change file bytes against 1.3. Any gate that diffs a round-tripped `.exp` must normalise trailing whitespace inside these blocks |
| `SIG_GPPVMData::sendQStringToPVM`, a **null** `QString` | `Q2CString`'s `const char *` conversion gave `nullptr`, and `pvm_pkstr` does `strlen(cp)` unguarded — a segfault | `constData()` gives `""`; an empty string is sent | Found by the D21 review, which showed the `+ 2` does not reproduce the old length for a null string. It never could: the old path died before the length was used. Unreachable today — the two live callers pass a string built by `savePVMDataTransfer` — but it is a crash removed, not a value preserved, and D21 first claimed otherwise |
| `SIG_GPForceFitnessFunction`'s cleanup loop | a `do`/`while` dereferencing `listForces.first()` **before** testing it | a range-for | `Q2PtrList::first()` returned null on an empty list, so an evaluation that recorded no frames took a null dereference **while freeing memory**. Identical with frames, a no-op without. Contrast D10, where the same shape's once-through was load-bearing and had to be kept — which side of the null the body is written for must be read each time, not pattern-matched |
| `SIG_Environment` terrain load | `getenv("SIGEL_ROOT")` unchecked | already checked, message on stderr | `sigel_eval` says "SIGEL_ROOT is not set, cannot locate Terrain.ter" instead of reading `/Terrain.ter` |
| `MT_GPSystem/MT_FitnessTrainer.cpp:88` | `loadSetup` sized `Result`/`ResultIst` from the **stale member** `TSetSize` while handing the training set the file's `NewTSetSize` | `TSetSize = NewTSetSize;` | Any setup file with a larger set made `calculateFitness` **write past both arrays**. `setSelektionValue` in the same file always did it correctly. Found by the D4–D6 review; **off the 42-evaluation path, so no gate saw it**. *Added to this table 2026-09-03 — it had been recorded only in a D-step paragraph, which the compression then removed* |
| `MT_Control/MT_Substitute.cpp:64` | `changeErrorInfo` looped to `CorrectFitness.size()`, a **high-water mark that only grows**, while indexing the caller's arrays | bounded by the smallest of the three | Those arrays shrink whenever the selection size is lowered. Same review, same table omission, same date |

~~**Open, from the R1 review:** SOLID is built without the `-DNDEBUG` its own
`Make-config` sets, so eight `assert(!eqz(x))` guards ahead of a division are
live that were not in 2003.~~ **Moot as of 2026-08-28** — SOLID is no longer
compiled at all. It went with the Dynamo backend, its only caller
(`physics_backends.md`).

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
  (`SIG_GPExperiment.cpp:133`, `SIG_GPExperimentClean.cpp:109,162`). All 14
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
  `MT_GUI/MT_ExperimentWidget.cpp:48` calls `prevSelectedItems.head()` with no
  emptiness guard, so `lastSelected()` before any selection returned 0 in 2003
  and will abort now. **STALE — Phase C closed this.** `MT_GUI` is in `check.sh`'s `MODULES` (`check.sh:94`) and the Makefile's `CORE` (`Makefile:312`), and C11c's `metagui` scenario drives the whole MetaGP window. *It said:* `MT_GUI` is in neither `check.sh`'s `MODULES` nor the
  Makefile's `CORE`, so nothing flags it.

### TRAP — plain `grep` in this environment SILENTLY SKIPS 9 SOURCE FILES

Found 2026-09-02, chasing a reported defect that turned out not to be one. In
this shell `grep` is a **function**, not the binary: it execs `ugrep` with
`-I`, which means *ignore binary files*. Nine files in the tree are ISO-8859
(Latin-1) rather than UTF-8, ugrep classifies them as binary, and `-I` drops
them **with no message and exit status 0** — indistinguishable from "no
matches".

    src/SIGEL_MasterGUI/SIG_MainWindow.cpp
    src/SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.cpp
    src/MT_GUI/MT_PopulationWidget.cpp
    src/MT_GUI/MT_StatisticsWidget.cpp
    include/MT_GUI/{MT_ExperimentItem,MT_PopListViewItem,MT_Editor,DoubleSpinBox,MT_WidgetBase}.h

`SIG_MainWindow.cpp` is on that list, so **"grep found nothing in the main
window" has meant nothing all along.** Use `command grep` (bypasses the
function) or `grep -a`. Both were verified against a file with a known match.

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
| Phase V item 4 targeted `SIG_DynaDrive::applyForce` | it would have gated the register-to-force path | that function is **DynaMo-only** and every shipped experiment selects DynaMechs. A clean "no divergence" would have meant nothing |
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
| **a running gate** | the `initial` rest angle, the `Q2Dict` ordering, the 42 fitness evaluations, the eight duplicate-key lookups | strongest available here — byte-identical across five phases |
| **the 1.3 binary, statically** | the randomiser's constants, `setAutoDelete` per constructor, destruction order, the truncated-π factor | strong, and the only route to code no gate reaches |
| **inspection and review alone** | the evolution-loop containers, the `±DBL_MAX` sentinel, the `-0` question, the tie class | weakest — and these are latent *precisely because* no gate reaches them, which is why they needed the binary rather than a test suite |

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
| ~~`SIG_GPManager.cpp:415` and `:1507`~~ **CLOSED by D25b** | `fitTaskList` is now `QList< QList<int> * >` (`:373`, `:1460`) and the walk is index-based | The hazard was real: `fitTaskList.first()` then `while (actFitTask)` — **the loop terminated on the null**, and Qt 6's `first()` is UB on empty. It became a cursor index (`isEmpty() ? -1 : 0`, then `at()`), not `value(0)` or a range-for, because the walk also needs `remove`/`current`/`next` semantics. This row is why the step was written the way it was |
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
referent error as Phase V item 4, and it recurred one message after the table
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

The live `sense` references **eight** constants, not the five first recorded
here: `2.0`, `-DBL_MAX`, `+DBL_MAX`, the truncated-π factor, `360.0`, `90.0`,
`-90.0`, `180.0`. *The first list was truncated by a `head` and reported as
complete — the same failure as the disassembly windows, in the same exchange
that tabulated it.*

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
values are read from the model file (`SIG_Joint.cpp:64`). Measured:

    DBL_MAX written by our stream : 1.79769e+308
    read back                     : 1.7976900000000001e+308
    DBL_MAX                       : 1.7976931348623157e+308
    round-trips to == DBL_MAX ?   : NO

At `%g` precision 6 the sentinel loses 11 significant digits, so **a joint saved
as unbounded and reloaded is no longer recognised as unbounded**. What follows is
worse than a wrong interval, and worse again than first recorded here:

| step | value |
|---|---|
| `posRange = maxPos - minPos` | **`+inf`** — the true difference `3.595e+308` exceeds `DBL_MAX`. *An earlier draft said "3.6e+308", a figure no `double` can hold* |
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

*Two corrections, both found by cross-checking against the x86 box.* The old
figures were **61 records and four distinct pairs** — 61 is right for the **12**
experiments in `experiments.tar.gz`, and this file has **14**: the two
`runner*.exp` come from `data/results/`. And the pairs are **seven**, not four:

    -45/45  26    0/35  12    45/80  11    -90/-50  10    -90/90  7    -85/85  6    0/90  1

The three that were missing — `45/80`, `-90/-50`, `0/90` — account for 18 of the
original 61. The 7 `.rrb` models agree independently: 53 rotational joints, the
same seven pairs, spelled `minimal`/`maximal`.

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
else in this section it is gate-covered** — fitness is distance-over-time from
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

*It also corrects Phase V item 4, which named the wrong function.*
`SIG_DynaDrive::applyForce` has exactly one caller —
`SIG_DynaMoCommandInterface::moveDrive` — and **every shipped experiment sets
`SIMULATIONLIBRARY 1`**, which is DynaMechs, so the DynaMo interface is never
constructed. A breakpoint on `applyForce` was watching a function that is dead
in the selected backend. The live `SIG_DynaMechsCommandInterface::moveDrive`
calls no `applyForce` at all; it computes the drive value inline.

**Verified locally against `xb/kdesigel/sigel_slave`** — a second unstripped 1.3
binary in this repository, alongside `xb/kdesigel/sigel`. `moveDrive` is at
`0x080b787c`:

| claim | measured |
|---|---|
| extended-precision ops | **14 `fstpt` + 22 `fldt` = 36**, over the function's true extent `0x080b787c`–`0x080b8094` (2072 bytes). *Reported to me as "seven pairs". I answered "nine ops" — also wrong, and wrong the same way: I had stopped at an arbitrary `0x080b7b80`, covering 772 of 2072 bytes. **I made the truncation error in the same message in which I corrected it.** The boundary is the next symbol, `__static_initialization_and_destruction_0`.* |
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
file-mediated gate; it matters only to someone demanding bit equality.

**How to compare it.** Treat `moveDrive` as a function of its inputs — feed a
known register value and width, capture the drive value on both sides — rather
than expecting bit equality. Given the 6-significant-digit finding above,
agreement to 6 figures is very likely and is the gate that matters. **Measure it
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
Qt 2 wrote through `sprintf("%.*lg")` (`qtextstream.cpp:1776-1805`, vendored
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
are exact binary fractions. That covers the V2 save path, `.pol` pool images and
the POV-Ray export. Not fixed here — `QTextStream` has no tie-breaking control,
so matching 1.3 would mean routing every real number through `snprintf("%.*lg")`,
which is a change to every writer and needs deciding, not assuming.
*Found by the C5 review. The audit was correct and its sampling method could not
see the failure — §9's "test with a representative value, not an extreme", in
the other direction.* *`-0` remains the one
known exception and appears in no shipped stream.*

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
| `SIG_GPParameter::slotAddHost`, `::slotDeleteHost` | **Phase C.** `slotDeleteHost` is safe by design — the 2003 comment at `:467` says *"we don't delete the host directly as the iterator would get confused"*, and it defers into a list. But it is a knot of three separate §9 hazards: an unconverted Qt 2 `QList<SIG_GPPVMHost>` used as a pointer list, a Qt 2 cursor `QListIterator` with `.current()`/`++`, and `hostList2.remove(host)` at `:520` |

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
./dictorder-dump.sh | diff -u dictorder-baseline.txt -
```

`dictorder-baseline.txt` is **2,189 lines over 21 blocks** — 14 experiments and 7 `.rrb`.
Every later Phase D step has to leave that diff empty.

**What the gate covers.** `SIG_Robot`'s **six** `Q2Dict`s (`SIG_Robot.h:60-65`)
— bodies, materials, links, joints, drives, sensors — all written in iteration
order by `writeToFileTransfer` and read back in that order by
`SIG_DynaMechsSimulationData`, plus `SIG_Link::points`, one per link.
**The order-carrying containers number ten, not seven or eight** — V1 measured
that against the 1.3 binary and found two this plan never enumerated.

**Verified to have teeth:** rebuilding the core with `h % vlen` perturbed to
`(h + 1u) % vlen` fires the gate on **8 of 14** experiments and **6 of 7**
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

**The script refuses to run** unless it finds exactly 14 `.exp` and 7 `.rrb`,
and it inspects `sigel_eval`'s stderr instead of discarding it. Both were real
holes: the previous version exited 0 on a missing build directory, a missing
`data/`, and a wrong `SIGEL_ROOT`, emitting a short file each time — and
Phase D re-captures this baseline at every step, so a silent short run would
have overwritten it and reported success.

**`data/` needs 14 `.exp`, and `experiments.tar.gz` carries 12.** The two
`runner*.exp` come from `data/results/runner*Experiment.tar.gz` (§7). Provision
`data/` the way §9 describes and the baseline is 6 robots, not 7.

### D2 — what the migration actually has to preserve

Names, so this stops being ambiguous: `dictorder-dump.sh` produces the order,
`dictorder-baseline.txt` is the committed reference, and **`data-reordered/`**
is the working copy. `data/` is never written to — it is the untracked download
and the only clean original we have.

**Only four of the six dicts are numbered.** The walk is, in this order,
**links → joints → sensors → drives**, and those four orders are the ones a
migration must reproduce exactly. **The citation here was wrong and is corrected
2026-08-28:** it named `SIG_DynaMoSimulationData.cpp:33-55`
(`dynaSystem.newLink/newJoint/newSensor/newDrive`), which is the **Dynamo**
class, deleted with that backend. Both backends walked the same four dicts in
the same order, so nothing this section concludes changes and
`dictorder-baseline.txt` did not move — but the live file is
`SIG_DynaMechsSimulationData.cpp`.

Bodies and materials are free of *numbering* — but an earlier draft said they
were "touched only by `loadGeometries`" and "only through `lookupMaterial`",
**and that was wrong**: both are also iterated by
`SIG_Robot::writeToFileTransfer`, so their order reaches `.exp` bytes, the PVM
stream (`SIG_GPPVMData.cpp:144`) and the POV-Ray export
(`SIG_RobotRenderer.cpp:218`). No index, number or DynaMechs registration comes
from either, so the conclusion holds and no fitness moves — but for a different
reason than the one given. One latent path: `SIG_Material.cpp:57` records a
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

The 7 `.rrb` do need it: `SIG_RobotCompilerObjects.cpp:89` inserts in
declaration order, which the same prepend then reverses, so the file has to be
written in today's `rrb` order for the flip to preserve it.

**Link numbers shift in the `.rrb`. An earlier draft here said that was safe
because "nothing indexes by that number". THAT WAS WRONG** — corrected by
review, and the correction contradicts nothing else in this file only because
the risk was already stated two paragraphs up.

`SIG_DynaSystem::getJoint(int)` really is a linear search rather than a
subscript, but the search *key is the number*, so that fact settles nothing.
Four containers are subscripted by it directly —
`SIG_DynaMechsSimulationData.cpp:148` `drives.insert(getNumber(), …)`, `:170`,
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
the number *is* the declaration position (`SIG_RobotCompilerObjects.cpp:89`,
`linknumber++`) and the file has nowhere to record a number independently. The
migration chose **iteration order**, which is what §10 asks for and what feeds
the DynaMechs body index. That choice was made silently and should not have
been; it is written down now, and it is **open to reversal** — leaving the 7
`.rrb` untouched would instead give declaration order = position = number =
iteration order, fully self-consistent, at the cost of changing the order the
1.3 binary would have used.

**Nothing shipped is affected either way.** The 14 `.exp` embed their own robot
with its own stored numbers, are never reconciled against a `.rrb`
(`SIG_GPExperiment.cpp:86-101`), and `SIG_Link.cpp:65` reads
`tx >> name >> number` straight back — every number stays as 2001 wrote it.
`dictorder-dump.sh` now prints the stored number next to the position (`#N`) so
this is visible to the gate instead of invisible to it.

### D3–D26 — the shim's users, one container at a time

**COMPRESSED 2026-09-03, from ~1,910 lines.** D27 deleted `q2compat.h`, so every
conversion below is enforced by a compiler that has no `Q2*` type left to accept.
What is kept is the semantics table the conversions were derived from, the
defects they found, the pre-existing hazards they wrote down, and the coverage
list — because *that* is the part that is still true and still load-bearing.
**"What the gates actually reach" below is verbatim.**

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
| `Q2PtrList::at(i)` | returned **nullptr** out of range, and 2003 code relies on it defensively (`MT_Statistics.cpp:79`) | **`value(i)`**, not `at()`. `QList::at()` out of range is UB and compiles silently |
| `Q2PtrVector::at(i)` / `operator[]` | **Qt 2 warned and then READ OUT OF RANGE** — `QGVector::at` is `if ( index >= len ) warningIndexRange( index ); return vec[index];` (`qgvector.h:85-92`), no clamp. **The clamp was the SHIM's own deliberate divergence**, and `q2compat.h`'s header comment said so | `value(i)`, which yields null. *Three distinct behaviours, and conflating them has now been done three times: `Q2PtrList::at` **returned null**, `Q2Array::at` **clamped** (`QGArray::at` does `msg_index(index); index = 0;`), and `Q2PtrVector::at` **read out of bounds**. §9's null-out-of-range row is the FIRST of the three. So where a D-step below says "the clamp went", what went was the **shim's safety net**, not 1.3 behaviour — and `value()` is safer than Qt 2 rather than equal to it.* Corrected 2026-09-03 by review; the compression had fused the distinction away |
| `Q2PtrList::insert(uint, const T*)` | returned false and did nothing when `i > size` | `QList::insert` is `Q_ASSERT_X(i <= size)` — abort in debug, UB in release |
| `Q2PtrList::take(i)` | returned nullptr out of range | `takeAt(i)` is out-of-range `operator[]` — abort or a heap read |
| `Q2Array::at()` | **clamped** an out-of-range index | dropped. It fired in none of the 42 evaluations, and both defects §9 said it masked are already fixed |
| `Q2Queue::dequeue()` / `head()` / `current()` | returned **0** on an empty queue (`qglist.cpp:438-439`, via `cfirst()`) | **`QQueue::dequeue()` is `QList::takeFirst()`, which is `Q_ASSERT(!isEmpty())`** — abort at `-O1 -g`, segfault under `-DQT_NO_DEBUG`. *The divergence runs the other way round from the obvious guess: the shim matched the reference and `QQueue` is the divergence* |
| `Q2ValueList::Iterator` | a **doubly-linked** list, so an iterator stays valid when the list is modified elsewhere, and `end()` is a fixed sentinel that appends splice in front of | `QList` is contiguous and an append can reallocate. **The faithful conversion is an index walk** — D25a |
| `Q2PtrList`'s internal cursor | `first()`/`next()` are real state; `remove()` returns the *next*; a dead cursor stays dead | an explicit index plus `cursorAfterRemoval`: stay on whatever slid in, step back if the removed one was last, die if the list emptied, and **do not advance on `next()` from dead** |
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
| **D11** | `friction`, `usedByLinks`, `successors` | **`SIG_Material::friction` and `SIG_Link::noCollide` are exercised by no gate**, so `-selfcheck` covers them through public API — and **the only assertion that catches the append bug is the count**, because `getFrictionValue` still returns the right value (a forward scan finds the correct entry before the stray duplicate). `fitness-check.sh` runs the self-check a **second** time under `detect_leaks=1`, which is what can see a dropped `qDeleteAll` at all |
| **D12** | `Q2CString` off the executed path | *a `grep` for shim **types** says nothing about shim **includes*** — six live `#include "compat/q2compat.h"` remained after the type sweep came back clean, every one reaching `QList` through the shim |
| **D13** | `hostList` | **a trap preserved rather than tidied**: an `if` with no braces whose next line is the cursor advance, so the loop advanced correctly and `-Wmisleading-indentation` was flagging 2003's layout. Verified against the pristine tarball bytes, not inferred from the indentation |
| **D14** | `experimentHistory`, in **both** variants of the class | the master's destructor is **not** byte-identical to Clean's — tab-indented, CRLF, and it also deletes `mtController`. See §9's "`SIG_GPExperiment` is defined twice". The file is one of the 46 mixed-encoding files §2 warns about |
| **D15** | `SIG_GPPopulation::pool` — five of §9's eight hidden frees | **`deleteIndividual` performed exactly one delete and nothing in the function said so**: it shifted with `insert(x, take(x+1))`, and `take` nulled each source slot, so only the *first* insert freed anything. **D15 then missed one of the five** — `readFromFile`'s `wasCanceled()` shrink, dead today only because `sigel_eval` has no `QApplication` and live the moment `slotPopulationImport` runs. Measured: **4,850 bytes in 45 allocations**. *`SIG_GPPopulation::sort()` has an EMPTY BODY despite a header comment saying it sorts the pool by fitness — a reader could implement it and silently renumber every individual and every stored `poolPos`* |
| **D16** | a self-check for the evolution loop's containers | **built because D15 proved the gates cannot see this code**: D15 shipped a real leak that `check.sh`, both diffs, the sanitized run, the self-check *and* the leak baseline all reported clean. Found **four pre-existing `SIG_GPPopulation` constructor defects**; two are fixed (missing initialiser lists) and **two are not**: `(int, SIG_Randomizer&)` and its four-argument sibling store `&r` and the destructor **deletes it** — a bad free on a borrowed randomizer. Repairing them means choosing an ownership policy, which is a design decision for whoever calls them. **Only the default constructor is used today** — exhaustive grep over the 1.3 tree and the 1.0 distribution |
| **D17** | the trainer's two host lists | **unexercised, not dead**, and the difference matters: `addDynHost` is reached only through a `pthread_create` **thread entry point** behind `-devolve`, and `flushAllDynHosts`' two call sites are both behind `if (serverIsUp)`, which only that thread sets — **so it is never entered under `-evolve`**. *And the modulus concern cannot be settled from the reference machine*: all four reference evolutions used exactly **one** `PVMHOST` (the `8` is slave slots on one host), so `nextHostNumber % 1` is 0 on every one of ~56,000 spawns. **Host rotation has never been exercised by anything** |
| **D18** | `toSpawnList` — the cursor walk | **the self-check tests a TRANSCRIPTION, not `sweepToSpawn`**, because the trainer is not linked into `sigel_eval`: dropping `cursorAfterRemoval`'s step-back **in production** leaves `check.sh`, `-selfcheck` and `pvm-check.sh` all passing, while the same mutation in the copy fails it. The evidence for the shipped rewrite is the review's differential run — **28,672 walks, 0 divergences**, mutation-tested at 8,256 and 12,544 divergences |
| **D19** | `pvmTasks`, `pvmHosts` | **D19 shipped a leak**: it removed both `setAutoDelete` flags and gave only `pvmTasks` a `qDeleteAll`, so every enabled `PVMHOST` leaked one `SIG_GPActivePVMHost` per trainer destruction, and **all 56 shipped `.exp` have at least one**. Found by a **19,500-scenario differential sweep**: 41 diverged, the smallest being one static host and no spawns; with the missing `delete`, all 19,500 agree. **It was the only behavioural difference in the entire conversion.** *Pre-existing and written down: `nextFreeNumber` is never reset — two writes in the 1.3 binary, `movl $0x0` in the constructor and `incl` in `spawnTask` — so `pvmTasks` grows without bound (~226 KB at 56,333 spawns, arithmetic not measurement), and `stopTrainersSlaves` uses it as its loop bound, making shutdown **O(total spawns ever)*** |
| **D20** | `SIG_GPFullDataRecorder`'s four lists, 10 files | **a null dereference fixed rather than reproduced**, per D13: `SIG_GPForceFitnessFunction`'s cleanup was a `do`/`while` that dereferenced `first()` before testing, so an evaluation recording no frames took a null dereference **while freeing**. *Same shape as D10's `do`/`while`, opposite conclusion — there the once-through was load-bearing and had to be preserved, here it is a crash. **The difference is which side of the null the body is written for, and it has to be read each time rather than pattern-matched.*** Coverage: `SIG_GPNiceWalkingFitnessFunction`'s walk runs on every gate, but **setting its index to 1 — the classic error for this conversion — leaves the fitness gate byte-identical across all 42 individuals.** The gate discriminates one bit per individual, not which frame or how many |
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
`SIG_Body::load()` (`src/SIGEL_Robot/SIG_Body.cpp:217`). It **aborts** under
AddressSanitizer, so `dictorder-dump.sh` cannot be run against `build/` at all
and the 7 `.rrb` — and everything the VRML/SceneGraph reader does — are
unreachable by ASan and UBSan. Vendored code, out of scope for the Qt port, but
**the blind spot is ours**: D9's write-up claimed "all 21 files load in
`dictorder-dump.sh`: 0 sanitizer reports", which is true only of the unsanitized
build. *Reproduced 2026-09-03 by review, unchanged.*

**`QT_NO_DEBUG` and `NDEBUG` are defined by neither the Makefile nor
`check.sh`**, so `QList::operator[]`'s `Q_ASSERT_X` is **live in both `build/`
and `build-fast/`** — confirmed to abort on a negative index. Several coverage
arguments above lean on this (an out-of-range index anywhere in a covered run
would have aborted rather than been absorbed), and the residual risk is a
release build alone.

**Two behaviour-changing fixes to 2003 code, both off the 42-evaluation path so
no gate saw either** — `MT_FitnessTrainer`'s stale-`TSetSize` array sizing and
`MT_Substitute`'s high-water-mark loop bound. **They are recorded in §9's
"Defects fixed rather than preserved (D13)" table**, which is where this class
belongs; they are not repeated here. *A first version of this section added them
to that table and then restated both in a duplicate table right here, in the
present tense, two paragraphs after a correction whose own moral was that a
figure "stood for a day in two places at once". Found by review the same day.*

**The trainer's host-rotation modulus is cast, and it is D9's defect a third and
fourth time.** `SIG_GPFitnessTrainer.cpp:593` and `:602` are
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
  three `resizeOwning` calls **still passes every gate, including the
  self-check** — the leak was behind `if (qApp)` and is unreachable headless.
  What the self-check does catch is `deleteIndividual`'s `delete`, and only under
  `detect_leaks=1`.
- **`sigel_eval`'s converted trace walk (D20) is exercised but unchecked.**
  `dictorder-dump.sh` filters stdout with `sed` and the only line that loop
  produces — `frames … height … last …` — is dropped by that filter, so patching
  the loop to skip its first element leaves the gate empty.
- **D20's `listForces` is not owned, and only ONE of six fitness functions frees
  the force vectors**, while `record()` allocates a `vector<double*>` plus a
  `new double[6]` per link on every recorded frame regardless of which fitness
  function runs. Measured on one `-v` run of `hammer`: **11,891,420 bytes in
  252,265 allocations**, the majority from `dmArticulation::getForces`. That is
  the leak D4 records as scaling with links and frames.

**Two pre-existing leaks D24 wrote down.** `TmpBuffer`'s **never-matched
remainder** leaks — a *matched* case is freed, because it is enqueued on
`TCaseBuffer` and `MT_Trainingset::updateTSet` deletes it at
`MT_Trainingset.cpp:120` and `:145`. And `StatisticsOfGeneration` leaks
**entirely**: its elements are `new`'d at `MT_Statistics.cpp:56` and
`MT_GPManager.cpp:557`, `~MT_Statistics` is empty, and **no `delete` of an
`MT_StatisticsElement` exists anywhere in the tree**.

**D25b's forward-looking note.** `delete fitTaskList.takeAt( fitCur )` is
unguarded where Qt 2's `remove()` was a silent **no-op on an invalid cursor**, so
a future edit that mutates the list mid-loop turns a no-op into UB.

##### Three things that were nearly wrong about the whole exercise

**`sigel_eval.cpp` is in the repository root, and `src/` + `include/` is not "all
code".** D26 declared the shim deletable on a grep over those two directories.
`sigel_eval.cpp:363` and `:401` instantiate `Q2PtrList<int>` — the D18
differential check — and **that file is the dictorder and fitness gate binary**.
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
Qt's own internal hashes. *§9 item 3 is the remaining bookkeeping: the call is
still unowned in `sigel.cpp`.*

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

### What the gates actually reach — measured 2026-08-28, not assumed

This plan has described the remaining shim work as a **31 / 52 split**: 31 sites
the gates can execute and 52 in the evolution loop that nothing can run until
Phase C. **That split is wrong**, and D9 and D10 both leaned on it. Found by the
D10 review, then measured directly: a counter was put in each shim class's
constructor, the gates were run, and the counter was read back.

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
`SIG_DynaMechsSimulationData.cpp:303`, which were the only `Q2CString` the gates
ever constructed. The 7 that survive are PVM code `sigel_eval` never runs. D12
took the row to zero and left it standing while citing the table as current;
corrected by review.

**Four of the six types execute under the gates, not three.** The single
`Q2PtrVector` is `SIG_GPPopulation::pool`, which the "evolution loop" label had
written off. `sigel_eval` builds a whole `SIG_GPExperiment`, so
`SIG_GPParameter::hostList` (the shipped `.exp` carry 20, 21 or **8** `PVMHOST`
lines), `SIG_GPExperiment::experimentHistory` and, on the `-v` path,
`SIG_GPFullDataRecorder`'s four lists are all live too.

**`Q2Queue` and `Q2ValueList` are the genuinely blind ones** — 0 constructions
in either gate. They are where the caveat at the top of this file actually
applies, and they are the two to leave for last and treat as the risk.

The probe was temporary and is not committed; the shim was restored from a copy
and both trees rebuilt before the gates were re-run.

**Converted code the gates do not exercise — keep this list growing.** Type
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
| `SIG_Material::friction` — three walks and the owning free, D11 | **0 friction declarations in any `.rrb`, and `nfric` is 0 on all 31 `Material` lines**. The list is empty on every gate run |
| `SIG_Body::usedByLinks`, D11 | appended on every `.rrb` load and **read nowhere in the tree** |
| both `SIG_GPFitnessTrainer` host walks, D13 | that object is **not linked into `sigel_eval` at all** |
| `SIG_GPParameter::writeToFile`'s `PVMHOST` loop, D13 | ~~linked, never called~~ **COVERED as of C11b** — `exportall` writes it and `guibehaviour-baseline.txt` pins the bytes |
| `readFromFile`'s `qDeleteAll` + `clear`, D13 | runs every load, always on an **empty** list |
| **all four sites in `SIG_GPExperiment.cpp`**, D14 | the master variant is compiled into `libSIGEL_GP.a` and **never linked** — `SIG_GPExperimentClean.o` satisfies the symbols first. `readelf --debug-dump=info` on `sigel_eval` has a CU for Clean and none for the master |
| `writeHistoryToFileTransfer`, D14 | linked, never called — no gate saves an `.exp` |
| `exportExperimentHistoryToGNUPlot`, D14 | linked; its only caller is `SIG_Experiment.cpp:567`, Phase C |
| **five of D15's eight `delete pool[…]`** | the three sized constructors, `importNewIndividual`, and `addRandomIndividuals`' — which is `delete nullptr` on every possible call, since `resize()` just made those slots. *D16 said six of nine; nine was a `grep` hit that counted a comment, and three of the eight are entered by the self-check as of D16* |
| both `wasCanceled()` shrinks, D15 | need a `QApplication`; `sigel_eval` has none, so `if (qApp)` is false |
| `readFromFile`'s shrink loop, D15 | the function runs on every load, but always on an **empty** pool, so the loop body never executes |
| `sort`, D15 | no caller anywhere |
| **the whole `TmpBuffer` loop, D24** | `MT_Evaluator` is **not in either gate binary** — `nm -C build/sigel_eval \| grep -c 'MT_Evaluator::'` is **0**, same for `build-fast`. Modules link as static archives and nothing references `MT_Evaluator.o`, so its 14 symbols never leave `libMT_Control.a`. The three green baselines carry **no** evidence about this conversion; the only mechanical check that touches it is `check.sh`'s `-fsyntax-only` |
| **`MT_Statistics`'s three converted functions, D24** | linked, never run. `gdb` breakpoints on `updateStatistics`, `getStatisticElement` and `writeToFileMT_Statistics` across **all 14** shipped experiments at individual 0: **zero hits** |
| `MT_GUI/MT_StatisticsWidget.cpp`'s nine `.count()` calls on the converted member, D24 | ~~`MT_GUI` is in neither `check.sh`'s `MODULES` nor the Makefile's `CORE`~~ — **STALE, it is in both, and `metagui` drives the widget (C11c)** |
| **the whole `taskCanDoList` index walk, D25a** | compiled and archived (21 `SIG_GPManager::` symbols in `libSIGEL_GP.a`) but **linked into nothing** — 0 in `sigel_eval`, `build-fast/sigel_eval` and `pvm_link`. The `removeAt` path, the append path and the `:122` reference never execute here. The 1.3 binary shows the path is live in a real run (80 appends in two generations); our gates cannot reach it |
| **four of D25a's six `at(canDoIdx)` sites** | `:127`, `:151`, `:1310`, `:1336` sit inside `#ifdef SIG_DEBUG`, and **`SIG_DEBUG` is defined nowhere in this build** — its only occurrence in the repo is `x/sigelSourceDistribution.1.0/sigel/makefile:72`, the abandoned 1.0 tree. Neither `make` nor `check.sh` parses them. Compiled explicitly with `-DSIG_DEBUG` by review: exit 0, no errors — so no latent defect, but only two of the six were checked by the build |
| the `maxTouchsPerLoop` break, D25a | **dead on every shipped configuration**: the value is persisted in none of the 28 `.exp`, and all five presets call `setMaxTouchsPerLoop(-1)` (`SIG_GPParameter.cpp:325,330,335,340,345`), so `(maxTouchsPerLoop != -1)` is always false |
| all **seven** D23 sites | linked and never called — *in `sigel_eval`, which is a test harness, not the program*. 1.3's master links the whole MT subsystem (`MT_Evaluator` 16 symbols, `MT_Classifier` 25, `MT_Statistics` 83, `MT_Substitute` 20, `MT_TrainingCase` 46, `MT_Trainingset` 14; all 0 in `sigel_slave`), so "never linked" is a fact about our harness and Phase C will link these. Detail: `MT_Substitute`, `MT_Trainingset` and `MT_FitnessTrainer` **are** in `sigel_eval` and `pvm_link` — 59 symbols, pulled in by `moc/MT_GPSystem/MT_GPManager.o` on the link line — but `gdb` breakpoints on all three converted functions and on `MT_GPManager::checkForNewTCase` were not hit across a full evaluation. The classes that never link are **`MT_Classifier` and `MT_Evaluator`**, the queue's two fillers, both evolution-loop. *This row previously said the first three were the unlinked ones: inverted, and asserted without running the `nm` the D22 row had already established for exactly this* |
| **all six D22 sites** | `nm -C build/sigel_eval` finds **0** `SIG_GPOperations::` and **0** `SIG_GPCrossOverTournament::`, and the same in `pvm_link`. The objects are archived in `libSIGEL_GP.a` and never pulled into a link. The evolution loop needs `sigel`, which Phase C blocks |
| all six `Q2CString` sites in `SIG_GPFitnessTrainer`, D21 | zero trainer symbols in `sigel_eval`; `pvm_link` links the object but never constructs a trainer, so they are **link-checked and never run** |
| `SIG_GPPVMData`'s `+ 2`, D21 | `pvm_link` runs the function, but `pvm-check.sh` passes with `+ 1` **and** `+ 0` — `QList` over-allocation hides a shortfall under about 8 bytes |
| all five **unlinked** fitness functions' walks, D20 — `Adaptive`, `Zorc`, `Stepper`, `RealSpeed`, `Force` — plus `SIG_EarlyRunTermSimulation` | `nm` finds 0 symbols for each in `sigel_eval`. `Stepper` is the **only reader of `touchdowns`** in the tree; `Force` the only reader of `listForces` and the only code that ever frees a force vector |
| `sigel_eval`'s trace walk, D20 | runs on all 21 dictorder inputs; its output is dropped by the gate's `sed`, so only a crash or a sanitizer report would show — **and there is no sanitizer report to be had.** `dictorder-dump.sh` defaults to `build-fast`, which has no sanitizer, and it **cannot be run against `build/` at all**: the `.rrb` load path aborts under ASan inside vendored cv97 (see the blind-spot section in §10). So this row's second half is empty and the walk is covered by a crash only. *Reconciled 2026-09-03; the restored blind spot falsified it* |
| `SIG_GPNiceWalkingFitnessFunction`'s walk, D20 | runs for 18 individuals, but the gate has **one bit** of discrimination — an off-by-one in the index is invisible to it |
| **everything D17, D18 and D19 changed** in `SIG_GPFitnessTrainer` — including all six `delete v[i]`, `resizeOwningHosts`, both `qDeleteAll` in the destructor and both `static_cast<uint>` moduli | `nm -C build/sigel_eval \| grep -c SIG_GPFitnessTrainer` is **0**. `pvm_link` links the object but never constructs a trainer, so it is link-checked and never run. The rewritten walk needs a live `pvm_spawn`; `flushAllDynHosts` is `-devolve`-only; the destructor's three `qDeleteAll` run for no gate |

### D27 — the compatibility layer is deleted

**`q2compat.h` and `q2compat_check.cpp` are gone.** `include/compat/` no longer
exists. **No `Q2*` shim type is used anywhere** — every remaining `Q2` mention is
a comment recording what the Qt 2 original did.

**That is NOT "no Qt 2 container exists".** The unported modules still declare
**71 lines** of them, across 22 files in `MT_GUI`, `SIGEL_MasterGUI`,
`SIGEL_CommonGUI`, `SIGEL_SlaveGUI`, `SIGEL_Visualisation` and
`SIGEL_RealInterface` — `QArray` 21, `QList` (pointer list) 15, `QDictIterator`
15, `QListIterator` 10, `QVector` 5, `QDict` 3, `QValueList` 2, `QQueue` 1.
**Phase C must convert every one.** *An earlier version of this section, of the
commit message and of the Phase D status row all said "no Qt 2 container type
exists anywhere in the tree" — false, and it reads as "Phase C has no containers
to port". Same too-narrow scope as §9's table, in the paragraph that names it.*
`sigel.cpp` and `sigel_slave.cpp` have none.

**The two blockers, cleared in order.**

**1. `sigel_eval.cpp` tested the D18 cursor rewrite against `Q2PtrList`.** That
check could not simply be deleted — nothing executes `sweepToSpawn` (it needs a
live PVM spawn), so the walk has no other verification at all. It is now
compared against **`Qt2CursorList`**, a **37**-line model of the Qt 2 cursor
(`sigel_eval.cpp:371-407`) written from the vendored source, with the line references kept:
`first`/`next`/`last`/`current` from `qglist.cpp:203-260`, `remove()` from
`:504-516`, the cursor-after-removal rule from `:436-473`, `autoDelete` from
`qlist.h:100`. **Backed by `std::vector`, not `QList`** — it shares no
implementation with either side, so agreement means agreement. A differential
test needs a reference independent of the code under test; that is what the
model is, and it is not a second shim.

**2. Twenty files got `<QList>` (and often `<QString>`, `<QTextStream>`) only
through the shim** — 16 headers and 3 `.cpp` under `src/`, **plus
`sigel_eval.cpp` at the repository root**. The D26 review found two headers.
Found by removing the include and letting the compiler answer: **175 error
diagnostics**, `check.sh` down to 94 pass / 15 fail. *An earlier draft said
"nineteen files, 30 errors" — the first omits the root file, the second is
reproducible under no scope I can construct.* Each now includes what it uses. *My first
sweep tested only for shim **types** and missed every file that used a plain Qt
6 type transitively — the same "too narrow a scope" shape as §9's table.*

**`check.sh` lost its shim self-check step.** That step built and **ran**
`q2compat_check.cpp` under ASan and UBSan, and was described in the script as
the only mechanical check that could see an ownership error. It only ever
tested the compatibility layer, so nothing remains for it to check. *It fed only
the grand `fail` counter, never `pass`, so 105/4 is unchanged — verified by
running the parent's `check.sh` on the parent's tree. An earlier version of the
replacement comment claimed the count would drop by one.*

*This said `check.sh` executes no code at all, which was true when D27 was
written.* **Phase C put three running binaries back into it** — `sigel_slave`,
a headless GUI probe, and `guidrive` over seven scenarios. What is still true,
and is the point of the paragraph, is that none of them reaches the evolution
loop. So **ownership of the evolution-loop containers (`fitTaskList`,
`toSpawnList`, `tours`), which no gate can reach, now has no mechanical coverage
of any kind.** `sigel_eval -selfcheck` still runs, but from `fitness-check.sh`,
not `check.sh`.

**Nothing that covered live converted code was lost.** All 19 assertion blocks
were classified against the recovered file: every one tested a `Q2*` type that
no longer exists, and the two behaviours the converted code reproduces by hand
are now covered *better* — `Q2CString::size()` counting the NUL is checked by
`pvm-check.sh` through **real PVM**, and the copy-constructor ownership hazard is
enforced at **compile time** (`= delete`) instead of by a runtime assertion.

**But it was also an executable specification for four Qt 2 semantics that
UNPORTED code still depends on**, and those consumers are still on the Qt 2 API.
Phase C now has to re-derive these from `qglist.cpp` instead of reading a
reviewed, runnable spec — **recover the file from git history rather than
guessing**:

| semantic | consumer still on the Qt 2 API |
|---|---|
| `QQueue::dequeue` must unlink on a **null head** — the self-check noted it "used to spin forever" otherwise | the `MT_GUI` queue |
| `QQueue::remove()` exists (`qqueue.h:60`), and `head()` after it | `MT_GUI/MT_ExperimentWidget.cpp:43,51` |
| **`remove(ptr)`: removes *that* pointer not the current one, reports whether it unlinked, never frees while non-owning, and a failed search KILLS the cursor.** The self-check called all three "load-bearing" | `SIGEL_MasterGUI/SIG_GPParameter.cpp:520` — `if ( hostList2.remove( host ) ) delete host;` |
| `QListIterator` stays dead off the end; `atFirst()`/`atLast()` both true on empty | the 10 remaining `QListIterator` lines |

**`Qt2CursorList` was verified, not assumed.** Review checked it line by line
against vendored `qglist.cpp`, ran it against the recovered `Q2PtrList` over
**20,000 randomised trials × 40 operations** comparing full state after every
step (0 divergences), repeated that against an independent node-for-node
`QGList` transcription so a *shared misreading* would surface (0 divergences),
and mutation-tested both the probe and the shipped check — perturbing D18's
index walk makes `-selfcheck` fail and exit 1. It covers 6 operations where the
self-check covered 11; the missing five (`prepend`, `at(i)`, `remove(idx)`,
`removeFirst`, `removeLast`) have no converted consumer.

**Gates unchanged: 105 pass, 4 fail, 309 warnings, both baselines empty,
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

### FLAKE — `pagesave` failed once, in a way that left no evidence, 2026-09-04

**Recorded rather than dismissed, because the interesting part is what the
failure did NOT say.** One `check.sh` run gave `pagesave vs 1.3  0 pass 1 fail`
with the message `a pagesave run did not finish:` **and nothing after it**. The
scenario run directly, immediately afterwards, was clean — exit 0, 9.9 s, the
expected 192 lines — and the next full `check.sh` was 846/0. It has not
recurred.

**What the absence of output narrows it to.** Both stdout files were empty and
the captured stderr was deleted unread, so the first version of this diagnostic
produced no evidence at all. guidrive's watchdog `fflush()`es stdout before
`_exit(3)`, so an empty stdout with no `!! WATCHDOG` line means **the watchdog
never fired**, and the process died without flushing its stdio buffer.

**THE FIRST DIAGNOSIS WRITTEN HERE WAS "blocked outside the event loop until
`timeout` killed it", AND THAT IS PROBABLY WRONG.** A later `check.sh` run on
this machine was **killed by the system for low memory**, with no stray
processes and 2.5 GiB free afterwards — the box has 7.2 GiB total and carries a
desktop session, an editor and two agent processes. **An OOM kill is SIGKILL: no
flush, no watchdog, no stderr, non-zero exit — every symptom, and a far simpler
explanation than a blocking call.** *The blocking-call reading was a real
deduction from real evidence and still fitted; it was just not the only thing
that fitted, and it was written as though it were. Check free memory before
chasing a blocking call.*

**The diagnostic now says all of that**, and prints the two files' sizes and the
captured stderr. *The `gui behaviour` section had already been fixed to print
its stderr on the failure path, by an earlier review; this section was written
afterwards and did not inherit it. A fix applied to one of two near-identical
blocks is half a fix.*

**This is the second flake this project has seen** — the first was a 844/1 run
followed by 845/0 on an identical tree, cause unknown and never reproduced.
That one left no evidence either. This one now would.

**And a THIRD appeared immediately after, in `gui behaviour`, which was
self-contradicting and IS fixed.** A `roundtrip` run printed

```
export1 0 bytes  383d1f4f...   <- the hash of a 299-byte file
export2 299 bytes 383d1f4f...
```

**Those two cannot both be true of one stable file**: sha256 of an empty file is
`e3b0c442…`, not `383d1f4f…`. The probe was taking the hash at one instant and
the size at another — `sha256Of(fa)` immediately after the export, then
`QFileInfo(fa).size()` some thirty lines and one whole import later — and
printing them as if they were simultaneous. `sha256Of` now returns the length of
**the exact bytes it hashed**, so the two cannot disagree. *This does not explain
what made a separate stat report 0; it removes the probe's ability to report a
contradiction, and leaves a real truncation to show up where it should — as a
hash mismatch and a failed round trip, loudly.*

### PRE-EXISTING — `pvmTasks` is READ without the growth check that WRITES it

**Found 2026-09-04 from the oracle's crash, and it is 1.3's own defect,
unchanged in the port.** `SIG_GPFitnessTrainer::spawnTask` grows the vector
before writing — `oldMaxIndex < nextFreeNumber + 1` → `resize( oldSize +
population )` — and that logic is **identical to the pristine 2003 source and
correct on its own terms**: it always grows before the index it is about to use.
But `checkTask( int taskId )` (`SIG_GPFitnessTrainer.cpp:368`) does

```cpp
SIG_GPPVMTask *pvmTask = pvmTasks[ taskId ];
```

with **no growth check and no bounds check at all**, so the read path can be
handed an id the vector has not been grown to cover.

**The evidence.** Driving a normal SIGEL evolution on the 2003 binary with
MetaGP enabled, the oracle got, in generation 4:

```
QGVector::operator[]: Index 272 out of range
Invalid storage access                       <- SIGEL's own SIGSEGV string
```

`QGVector` is the **pointer**-vector base, so `QArray` members are excluded
(they print `QGArray::at:`) — which rules out `MT_ResultBuffer`,
`instructionProb` and the `MT_TranslatedIndividual` arrays. Of the `QVector`
members in the evolution loop at population 100 — `pool` 100, `tours` 50,
`indis` and `pvmHosts` small, `individualItems` pool-sized — **only `pvmTasks`
can be near 272**: it is built at 100 and grows a whole population at a time, so
its sizes are 100, 200, 300, and **272 is out of range for exactly size 200**.

**THE EXPERIMENT WAS CONFOUNDED AND NO ATTRIBUTION IS AVAILABLE. Do not repeat
this as a MetaGP defect.** Two variables moved together: in both crashed runs
MetaGP was enabled **and** the GUI was touched during the run (an attempt to
open Configure System); in both controls MetaGP was off **and** nothing was
touched. So "MetaGP enabled" and "the GUI was poked mid-run" are perfectly
correlated in the data and the crash is attributable to either. *Caught by the
oracle itself, after it had already sent a MetaGP-flavoured reading — the
timing on the second run is documented to a 89-second window that contains the
interaction.* Two crashes, indices **272** and **497**.

| run | MetaGP | mid-run GUI interaction | outcome |
|---|---|---|---|
| 1 | on | yes | crash, generation 4, index 272 |
| 2 | on | yes | crash, index 497 |
| control 1 | off | none | generation 6, alive when killed |
| control 2 | off | none | running at generation 2 |

**The decisive run is MetaGP ON with no mid-run interaction**, and it had not
been done when this was written. *If it survives, the two "MetaGP" crashes were
self-inflicted and the real finding is much broader and more useful: **touching
the GUI during a running evolution can kill 1.3**.* That reading also fits the
mechanism above — an unchecked read racing the growth is exactly what a
re-entrant `processEvents()` from a GUI interaction would expose, and
`slotStartEvolution` only survives at all because `haveABreak()` pumps events.

*A MetaGP-specific mechanism does exist and is why the first reading was
plausible:* `MT_Evaluator::checkTask` **overrides** the unchecked function and
calls the base, and `MT_Evaluator` is one of the two classes only in the loop
when MetaGP is enabled. That remains a candidate, not a conclusion.

**THE PORT FAILS DIFFERENTLY HERE, AND MORE LOUDLY.** The unchecked read is
unchanged, but neither the Makefile nor `check.sh` defines `QT_NO_DEBUG`, so
`QList::operator[]`'s `Q_ASSERT_X` is live in both build trees. Where 1.3 warns
to stderr and then reads out of bounds into a segfault, the port aborts at the
index itself. That is the dropped-clamp consequence D6, D8, D9 and D25c each
recorded in the abstract; **this is the first evidence of a path that actually
reaches it.** No gate covers it — `SIG_GPFitnessTrainer` is linked into no gate
binary — so it is recorded, not tested.

### PRE-EXISTING LEAK — the simulation backend is never freed

`SIG_Simulation.cpp:65` allocates a `SIG_DynaMechsSimulationData` with `new`.
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
