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

**Status — 2026-08-28**

| phase | state |
|---|---|
| 0 — comments to English | done for the 9 core modules; 9 GUI files still hold Latin-1 |
| A — core onto Qt 6 | **done**, tags `step-A0`…`step-A9` |
| B — ownership explicit | **subsumed by Phase D**, which deletes the containers rather than converting them. **0 `setAutoDelete` calls left in core**, re-measured 2026-08-30 after D25c: D11 removed the last in `SIGEL_Robot`, D24 the last in `MT_Control`, D25b replaced the two `fitTaskList` calls with an RAII guard, and D25c wrote out `tours`' two real frees at their sites. Every remaining call in the tree is in `SIGEL_MasterGUI`, `SIGEL_Visualisation`, `SIGEL_CommonGUI` or `MT_GUI` — Phase C. *This row has been corrected five times, each time by review: it has read 13, 12, 11, 3 and 1. The recurring errors were counting comments as calls and quoting a `SIGEL_MasterGUI` figure as a core one.* **Not all of these were unreachable, and an earlier version of this row said they were** — `SIG_GPPopulation::pool` is owning, is constructed on every `sigel_eval` run, and takes 100 `insert()`s inside both gates; see "What the gates actually reach" in §10 |
| R — build and run | core builds and runs. **No longer checked only against itself** — Phase V has confirmed both the ordering and the arithmetic against the 1.3 binary, §7 |
| T — old-Qt tool container | **done 2026-08-27.** `tools/qtmig`, §4 |
| D — delete the shim, migrate the data | **DONE 2026-08-30.** `q2compat.h` and `q2compat_check.cpp` deleted; `include/compat/` gone; **no `Q2*` shim type is used anywhere**. D1–D27. *This is not "no Qt 2 container exists" — the unported GUI modules still declare **71 lines** of `QArray`, `QDict`, `QList`-as-pointer-list and friends, all of which Phase C must convert. See D27.* The shim's self-check step is gone from `check.sh`, which now runs no code. §10 |
| P — PVM | **DONE 2026-08-28.** Vendored 3.4.3 replaced by upstream 3.4.6; nine patches carry the four config lines and Debian's eight source fixes; `libpvm3.a` and `pvmd3` build; SIGEL's two PVM objects link against them and `SIG_GPPVMData` round-trips through real PVM. `sigel`/`sigel_slave` still need Phase C. §7 |
| C — GUI | **C1 DONE 2026-08-30**, AUTHORIZED 2026-08-27 per D24. **534 Qt 2 code sites** in the 5 GUI modules, re-measured at C1 with the pattern stated and with every row derived from a sweep of what Qt 6 no longer declares — **up** from 466, because 8 classes this plan had never named account for 89 of them, `QIconSet` alone for 47. **20 forms, all converted (C1, C2).** §7 |
| V — check against the 1.3 binary | **V1, V5's MDH probe, V6, V7 and V8 all done, all PASS.** Ordering: 10 of 10 container orders match. Arithmetic: `twoBases` exact bit for bit, `octopus` 9/9 with three joints exact and 5 ulp worst. **V6, V7 and V8 done 2026-08-29** — friction and no-collide negotiation, their four remaining rules, and the GP parameter blocks captured *before* their conversion. `verification-against-sigel-1.3/v6`, `v7`, `v8`. V2–V4 not started; V5's sensor and force probes are **invalid as specified** — both target Dynamo-only functions, deleted 2026-08-28. §7 |

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
8. The evolution-loop containers, still untestable — PVM runs as of Phase P, but Phase C blocks the loop (§7).

**A caveat that governs the order of what is left.** Everything after Phase D's
simulation-side work is in the **evolution loop**, which nothing can execute:
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
  Takes several minutes. **It runs no code** — D27 removed the only step that
  did. Execution happens in `./fitness-check.sh`, which runs
  `sigel_eval -selfcheck`.
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
silently skips the 46 Latin-1/CRLF files — it has produced wrong counts several
times.

### Qt API that must change

| Item | Sites | Notes |
|---|---|---|
| `QArray<T>` | 74 | Qt 2 value array |
| `QDict<T>` + `QDictIterator` | 61 | |
| `QVector<T>` | 48 | **array of pointers** in Qt 2 — not Qt 6's `QVector` |
| `QList<T>` | 37 | **list of pointers** in Qt 2, with `autoDelete()` |
| `QCString` / `QValueList` / `QQueue` / `QListIterator` | 16 | |
| `setAutoDelete` / `autoDelete` | 47 | 38 `TRUE`, 9 `FALSE`. *Pristine-tree figures.* Today: **0 in core, 16 in the GUI modules** |
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

**100 in the pristine tree; C4 repaired 10; 90 remain — 44 in
`SIGEL_MasterGUI` (C7), 31 in `MT_GUI` (C6), 15 in `MT_Control` (C8)**, and
zero in the **eleven other** modules `check.sh` compiles. *`MT_Control` is one
of the twelve it compiles, so "zero everywhere it looks" was false; its 15 are
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
| **D26** | What the 1.3 binary is asked for | **structure and arithmetic, not fitness equality.** Three tiers, in descending confidence: the container ordering and numbering, which compare exactly (V1, V2); per-individual fitness, which is chaotic across architectures and is therefore a judgement (V4); the non-integrating quantities, which compare exactly but need `gdb` (V5). Bit-exact agreement on an integrated trajectory is **not** a target and its absence proves nothing — §7. Recorded because this file repeatedly described the missing reference as "fitness numbers", which is the one thing that binary cannot usefully give |
| **D25** | What "done" means | **Plain modern Qt 6, nothing left over.** `q2compat.h` deleted, no Qt3Support class anywhere, no compatibility flag on SIGEL's own code. This moves §10's "drop the Qt 2 emulation" from optional debt into a **required phase**, and with it the data migration that section describes — the shim exists because `Q2Dict`'s hash order numbers the links, so the 7 `.rrb` and 12 `.exp` files must be rewritten before it can go. **Ordered before Phase C**, so the GUI sites (466 as counted then, **534** re-measured at C1 — §7) are ported once, to the final target, instead of twice. Vendored third-party code is out of scope for this rule: qhull, cv97, Dynamo and PVM keep `-w -fpermissive` |

## 5a. Decisions — signed off 2026-08-22, for Phase R

| # | Decision | Answer |
|---|---|---|
| **D15** | Where the robot models come from | downloaded from `sigel.sourceforge.net`, §9. Untracked, in `data/` |
| **D16** | The Dynamo branch in `SIG_Simulation.cpp` | ~~build Dynamo, SOLID and qhull~~ **superseded 2026-08-28.** `physics_backends.md` was decided and executed: the branch and its 13 file pairs are deleted, `SIMULATIONLIBRARY 0` now fails loudly, and the Dynamo archive is cut to the maths objects. SOLID is still built and is now dead weight |
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
- Commit messages: subject plus 3–5 lines. The code is the commit.

**Phase P departed from this, on instruction.** No `step-P*` tags exist — the
session owner asked for none, so the last tags are `step-R1` and `step-A9`. P1's
commit is not prefixed either. And each step has **two** commits, not one: the
step, then the fix for what its review found. Every round found something, so
folding the fix into the step would have hidden it. Eight commits, `ee981a1`
through `f0f2daa`.

---

## 7. Steps

**Exit criterion per step:** `./check.sh` at the repo root — **393 pass, 5 fail,
355 warnings** as of 2026-08-31, C4.

**The pass/fail basis changed at C4 and earlier figures are not comparable.**
The standalone header pass had always been reported and never added to the
totals, so the headline "fail" excluded a real header failure; it is now folded
in, along with the 33-row regex self-test and the parsers check.
**228 + 132 + 33 = 393, and 4 + 1 = 5.** Warnings are on the unchanged basis and
remain comparable throughout.

Earlier figures, on the old basis: 227/4/355 at C4 as first recorded, 203/4/309
at C2 (the rise being `SIGEL_Visualisation` 22 + `SIGEL_CommonGUI` 10, both
newly covered), 105/4/309 before Phase C, 322 warnings on
2026-08-28 — the drops are recorded per step and each is explained, because a
step that silently loses a warning has hidden something). The 4 failures are
exactly the files the Makefile excludes. Of the 112 passes added since Phase C began, **98 are the `forms (Phase C)`
section** — six checks over each of the 20 forms — and **14 are genuine new
module coverage**: `SIGEL_Visualisation` (12) joined `MODULES` at C5 and
`SIGEL_CommonGUI` (2) at C3. A GUI module joins only when every file in it
compiles. *This paragraph said "not new module coverage… a GUI module still
joins only at C3–C7" while the paragraph fifteen lines below already said the
opposite. It is the exit criterion, which is the first thing §0 sends a new
session to.* It was 118/4/338 until the Dynamo backend was deleted
(`physics_backends.md`); the pass count and "headers standalone" each fall by
exactly 13, one per deleted file pair, and the failing files are unchanged.

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
only when every file in it compiles. `sigel.cpp`, `sigel_slave.cpp` and the
three remaining GUI modules are still checked by nothing — a GUI module can only join
`MODULES` when every file in it compiles, which is C3–C7. What C1 added is a
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
through a live daemon. **`sigel` and `sigel_slave` still do not link** — they
need the Qt 2 GUI modules Phase C has not ported, so the evolution loop stays
unreachable. The blocker moved from PVM to Phase C; it did not lift.

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
./check.sh                                            393 pass, 5 fail
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

**4 files still fail**, and they are exactly the ones the Makefile excludes —
`MT_Controller.cpp`, `SIG_GUIGPManager.cpp` and both ZORC fitness files. Blocked
on Phase C; the site counts are in §7 C8. *This said 5 and listed
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

`make` at the repo root builds 205 vendored objects, 105 of SIGEL's 109 core
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

**Four core files still do not build.** `MT_Controller.cpp` constructs an
`MT_MainWindow`; `SIG_GUIGPManager.cpp` is its counterpart; both ZORC fitness
files ask the user for the distance walked through `QInputDialog` and are still
on the Qt 2 API. None is on the evaluation path.

**Four vendored patches**, applied by `make` against a stamp inside the
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

### Replication — nothing to compare against yet

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
| V3 | Determinism on the x86 box — one experiment run twice, both `RANDOMSEED`s pinned | gates everything numeric; never tested there |
| V4 | **REFERENCE RECEIVED 2026-08-30**, captured before this build can run it. Two whole-run digests — `twoBases` and `octopus` — each validated across two independent 1.3 runs on the reference machine. **Not yet a runnable gate here**: no capture file, and the prepared inputs are uncommitted, so the digests cannot be checked locally. Supersedes the single-individual fitness harvest | the interpreter, physics, genetic operators, selection and RNG **in composition**, over 300 evaluations, as an exact yes/no — *once the inputs are committed* |
| V5 | **MDH probe DONE 2026-08-27, PASS** — `verification-against-sigel-1.3/v5-1.3-mdh-compared.txt`. The sensor and force probes remain open | the port's **arithmetic**, which V1–V4 never touch |
| V6 | **DONE 2026-08-29, PASS, 5 of 5** — `verification-against-sigel-1.3/v6-1.3-friction-nocollide.txt` | the two Phase D paths **no shipped data exercises**: friction pairs and no-collide pairs, and whether both setters negotiate |
| V7 | **DONE 2026-08-29, 4 runs on `walker`** — `verification-against-sigel-1.3/v7-1.3-friction-nocollide-rules.txt` | the remaining rules for those two paths: multiple partners, unloaded partners, duplicates, and whether a dropped entry is resurrected |
| V8 | **DONE 2026-08-29, captured BEFORE the conversion** — `verification-against-sigel-1.3/v8-1.3-gp-blocks.txt` | `SIG_GPParameter::hostList` and `SIG_GPExperiment::experimentHistory`, the two `Q2PtrList` the gates run on every load and the next to convert |
| V9 | **DONE 2026-08-29, 3 of 3** — three function *bodies* disassembled, recorded below rather than as a capture file | whether a reworked body hides under an unchanged name. Symbol lookups cannot see that |

### V4 — the whole-run gate, captured 2026-08-30 before this build can run it

**Two reference captures, each validated across two independent 1.3 runs.** This
replaces the single-individual fitness check V4 originally proposed, and the
tolerance argument that came with it.

**PROVENANCE — read before relying on any of this.** These digests were produced
on the x86 reference machine and **cannot be verified in this repository**.
Unlike V1 and V5–V8 there is **no capture file** here, and the modified `.exp`
inputs are not committed — so the six hashes and the spot-check rows are
currently *unfalsifiable locally*. The prose spec below is the only route back to
the inputs, and it already needed one correction (the `PVMHOST` count differs
between the two experiments), which means a future mismatch could be a
reconstruction error rather than a port defect.

**Before this is used as a gate, commit the two prepared `.exp` files** — or the
diff that produces them — so a mismatch is interpretable. Until then treat V4 as
*reference data received*, not as a gate that can be run.

**Digest construction.** Per pool snapshot: one line per individual,
`NAME|FITNESS|sha256(program)`, **in pool order**; then sha256 over the whole
block.

**`twoBasesSimpleFitness1`** — 2 links, 1 joint, 100 individuals per snapshot:

    snapshot 1  eec007b386aa97cd977ce61c9d18138493cace77b3e74520eee21c01e8c07856
    snapshot 2  bc2ad371e65afac762f29e6d1bd5b5852994b74bee4bf70c24d3d55f1eccab51
    snapshot 3  1771c69f7e15b98401c735da204fa24f554b17da8937afe433b966f329c4618b

    spot check, first five of snapshot 1 (NAME|FITNESS):
    12354|0.0106312  12040|0.842208  11958|0.560132  12291|0.246616  12326|0.704262

**`octopusSimpleFitness`** — 10 links, 9 joints, 100 individuals per snapshot:

    snapshot 1  db5b486ba5ebb0e071dd07611b6e7889e93488de5fb75ba669b3169882038551
    snapshot 2  3895888944f94319ae5334beb4290dbe9342d7cdad89de013a647aaba9501378
    snapshot 3  632ec0847c3e2fd44203dd1095f1b8ba6d0c6e5fee83e26d041cef5b94950e53

    spot check, first five of snapshot 1:
    5367|0.829977  5192|0.789198  5220|0.701824  5438|1.08944  5437|0.624343

**Reproduction spec.** Start from the shipped `.exp` and change exactly these;
the population is unchanged. *Identical for both **except** `PVMHOST`: the
shipped host count is **8** for twoBases, **20** for octopus and 21 for walker.
An earlier draft said "identical for both" with "8 dead hosts" in the shipped
column, which is wrong for octopus.*

| key | shipped | gate |
|---|---|---|
| `TIMETOSIMULATE` | `0 3 0 0` | `0 0 5 0` (3 min → 5 s) |
| `RANDOMSEED`, **both keys** | `0`, `0` | `12345`, `12345` |
| `TERMINATIONUSESDATE` | 1 | 0 |
| `TERMINATIONMODEL` | 0 | 2 (generation) |
| `TERMINATIONGENERATIONNO` | 0 | 3 |
| `TERMINATIONTIME` year | 2001 | 2030 |
| `TERMINATIONDURATIONDAYS` | 0 | 30 — **must not be 0** |
| `POOLIMAGEGENERATION` | 0 | 1 |
| `PVMHOST` | 8 dead hosts (twoBases) / **20** (octopus) | one host, **slot count 1** |
| `GRAVEYARDDIRECTORY`, `POOLIMAGEDIRECTORY` | dead paths | writable local |

**The slot count of 1 is load-bearing.** At 8 the run is not reproducible and the
digests mean nothing.

**Octopus has TWO dead 2003 roots, not one.** `/home/pg368/sawitzki/octopus/` on
the `Body` lines and `/home/pg368/sawitzki/sigel` for the directories — where
twoBases has only `/home/pg368b/ross/projects/sigel`. A repointing tool that
knows one root silently fixes nothing. Both were normalised onto the twoBases
root before capture.

**Why two gates rather than one — they discriminate.** twoBases has one MDH
call, one drive, one sensor, and no container-ordering effects at all: nothing
can collide in its hash, which is why its robot block round-trips byte-identical
(V1). Octopus is where V1 found Joint, Drive **and** Sensor orders permuting and
where V5 found nine MDH triples including the one-ULP near-misses. Nine joints
means nine MDH sets and nine sensor readings per step — and **the truncated-π
constant feeds every one of them**, compounded over 500 steps and 300
evaluations. So **twoBases green with octopus red points at geometry and
container ordering, not at the interpreter, RNG or selection.** One gate cannot
say that.

**What a match proves:** the interpreter executed 300 programs identically, the
physics integrated them identically, the genetic operators picked the same
parents, crossover and mutation points, selection ranked identically, and the
RNG produced the same stream — *in composition*. With the 6-significant-digit
format result, a match means the `.pol` files are byte-identical bar timestamps.

**What a mismatch gives:** the three snapshots localise in time, and the
per-individual rows are held on the reference side, so a failure narrows to
which individuals differ and whether it is the program, the fitness or the
ordering. Ask for the diff at whatever granularity helps.

**Limits, stated rather than implied.** Two experiments, two robots, three
generations, a **5-second** window chosen for runtime — not the shipped 180.
Neither exercises friction or no-collide (every shipped robot has `nfric 0`).
`SLAVES=1` only: nothing here bears on the concurrent path, where two runs
genuinely differ. **A strong gate on the machinery, not a certificate on the
robot corpus.** `walker` (19 links, 0.002 step ⇒ 2,500 steps per individual) is
deliberately uncaptured — it is the expensive one and is better taken once this
build can be compared against something.

**This also strengthens V3.** The determinism result no longer rests on one
case: **two experiments, two robots (2 links and 10), two fitness functions**,
all exactly reproducible at `SLAVES=1` with both seeds pinned.

**SCOPE — the 1.3 source and the 1.3 binary are not the same revision.**
Everything above treats `x/kdesigelSources.1.3/` and the frozen binary as one
reference. **They are not.** `nm sigel | grep -c MT_FitnessTranier` returns
**24**; `MT_FitnessTrainer` returns **0**. Our source has **38 lines** carrying the correct
spelling (43 occurrences, over `src/` and `include/`) and **zero**
misspellings, with the typo surviving only as a config
token (`stdConf.mt:71`) and four string literals in `MT_FitnessTrainer.cpp`,
where `:15` says it is kept deliberately. `SIG_GPFitnessTrainer` is spelled
correctly in the same binary, so this is not a general mangling artifact.

A 30-name survey found no other divergence: 28 present as expected, and the
two that were not are both mine to own — `SIG_GPEnergyFitnessFunction` is
absent from **both** trees, so the friend-line debris I called decisive
supports neither direction, and `SIG_GPExperimentClean` was a category error,
a file name that was never a class. **"The gap is confined to `MT_`" is an
inference, not a measurement**, and a reworked body under an unchanged name is
invisible to every symbol lookup. That is what V9 exists to probe.

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

**T2 EXERCISED ONE OF `uic3`'s FIVE MODES.** Everything below, and the residue
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
| C6 | `MT_GUI` — **31 dead connects** (§2) and **9 prepending item sites** (§9), six of them ordering the MetaGP page list | 3,911 LOC, 14 files |
| C7 | `SIGEL_MasterGUI` — **44 dead connects** (§2) and **28 prepending item sites** (§9) | 7,717 LOC, 20 files |
| C8 | `sigel.cpp`, `sigel_slave.cpp`; **`MT_Control`'s 15 dead connects** (§2) | 15 sites |
| C9 | All five modules and both programs build, link and run | — |

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
| C7 | `SIGEL_MasterGUI` | none |

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

#### C1 — one form end to end, and the four `uic3` modes Phase T never ran

**The form is `SIGEL_MasterUI/SIG_GPParameterBase.ui`**, chosen by measurement
rather than by size: of the 20 it is the **only one carrying both an embedded
image and dropped slot declarations**, which are the two categories §7 names for
this step. 3,149 lines, 26 connections, 10 custom slots, 9 `QGroupBox`, 2
`QButtonGroup`, 1 `QListView`, 1 image. It is also the form behind
`SIG_GPParameter.cpp`, the file §9 calls a knot of three separate hazards.

**PHASE T MEASURED ONE OF `uic3`'s MODES, AND §4 AND THE RESIDUE TABLE ARE
BUILT ON THAT ONE.** `-convert` is not the whole tool. `uic3 -help` lists
convert, declaration (`-decl`, `-wrap`), implementation, image collection
(`-embed`), subclass declaration and subclass implementation — with `-extract`
an **option**, not a mode. *An earlier draft here said "five modes", promoted
`-extract` to one, merged the two subclass modes and omitted `-embed`; the
count was wrong in three ways at once for no gain, since what matters is which
ones do work this plan was about to do by hand.* Three residue rows were being
planned as hand work that Qt already does:

| mode | what it produces |
|---|---|
| `uic3 <form>.ui` | **the QWidget-derived base class.** Qt 6's `uic` emits only `Ui_<Form>`, a struct with `setupUi()`, so the class the 20 hand-written subclasses inherit from had no source. This plan had not noticed the gap at all |
| `uic3 -impl <hdr> <form>.ui` | its constructor, `languageChange()` → `retranslateUi`, and a `qWarning("Not implemented yet")` stub per custom slot |
| `uic3 -convert -extract <f>.qrc` | the embedded images as real files plus a `.qrc`, and `<images>` rewritten to `<resources>` |
| `uic3 -subdecl` / `-subimpl` | the *subclass* skeleton. Not used — all 20 subclasses exist and are the port's actual work |

Qt 6's `uic` adds a fifth answer: **`--connections string`** emits string-based
`SIGNAL`/`SLOT` for every connection, which is what 2003 had. **Not used**, and
the reason is typing: restoring the `<slots>` block instead leaves `uic` free to
emit pointer-to-member for the 16 connections it *can* resolve and string-based
for only the 10 it cannot. Compile-time checking on 16 beats a flag.

**The base class — the decision this step actually settled.**
`uic3`'s declaration mode picks the shape, so it is not an invention of this
port:

```cpp
class SIG_GPParameterBase : public QWidget, public Ui::SIG_GPParameterBase
{ Q_OBJECT
public:
  SIG_GPParameterBase( QWidget *parent = 0, const char *name = 0,
                       Qt::WindowFlags fl = 0 );
public slots:
  virtual void slotAddHost();          // …all 10
protected slots:
  virtual void languageChange();
};
```

It keeps the **three-argument constructor**, so `SIG_GPParameter.cpp`'s
`: SIG_GPParameterBase( parent, name, fl )` needs no change and C3–C7 stay about
Qt 2 API sites instead of also rewriting 20 class hierarchies. The alternative —
each subclass becoming `public QWidget, private Ui::<Form>` with `setupUi(this)`
— is the modern idiom and adds no files, but it is a hierarchy change Qt's own
converter does not ask for.

It is **committed**, per form, as `include/<Module>/<Form>.h` and
`src/<Module>/<Form>.cpp` — exactly where 2003's build generated them. It cannot
be generated at build time: Qt 6's `uic` cannot produce it and requiring docker
for every build is not acceptable. Three Qt 6 adjustments to `uic3`'s output,
all marked in the files:

| `uic3` emits | Qt 6 |
|---|---|
| `Qt::WindowFlags fl = 0` | `= Qt::WindowFlags()` — `QFlags` has no converting constructor from `0` |
| `: QWidget(parent, name, fl)` | `: QWidget(parent, fl)` plus `setObjectName(name)`. The name argument was `QT3_SUPPORT` and did exactly this |
| `protected slots: languageChange()` | kept, and driven from a `changeEvent` override. Qt 3 called it itself; Qt 6 has no such hook, so without the override the slot would be dead code rather than a port |

**Three corrections to the Phase T residue table, all found by doing it.**

- **The embedded image is worse than recorded.** The table says Qt 6's `uic`
  "emits `setIcon(QPixmap("image0"))`, which compiles clean and renders a blank
  button". Here the pixmap is on a **`QComboBox` item**, and `uic` emits
  `addItem(QString())` — no icon, no `QPixmap`, no trace. There is nothing in the
  generated code to grep for, which is why the check below tests the `.qrc` in
  **both** directions.
- **`Q3ButtonGroup` does not always need a `QButtonGroup`.** The table says
  "`QGroupBox` + a `QButtonGroup` for the exclusivity". Both uses in this form
  are plain `QGroupBox`: `buttongroupByTime` holds the form's only two
  `QRadioButton`s as direct children, and Qt's auto-exclusivity is per parent
  widget; `buttongroupGraveyard` holds a `QCheckBox` and no exclusive buttons at
  all. **The other 5 uses are in other forms and each needs its children read** —
  this is not a blanket rule either way.
- **Restoring the `<slots>` block verbatim is not enough**, and **the 2003 form
  is not to blame** — a first draft here said it was. Qt 6's `uic` matches a
  declaration against a `<connection>`'s `<slot>` **as a string**. The Qt 2 form
  is internally *consistent*: `slotCrossoverChanged( int )` in the declaration
  **and** in the connection, spaces in both. `uic3 -convert` **normalises the
  connection** to `slotCrossoverChanged(int)` and drops the declaration, so
  pasting the Qt 2 declaration back verbatim is what creates the mismatch —
  and Qt 6's `uic` then silently falls back to
  `&QWidget::slotCrossoverChanged`, a hard compile error. Two of ten here. The
  declarations are re-spelled from the connection that uses them. **C2 must
  expect this wherever the 2003 author put spaces in a signature**, not where
  the form disagrees with itself. Found by review.

- **`uic3 -convert` drops a `Line`'s `orientation`, and the separator then
  paints nothing.** Found by the C1 review, in C1's own form. Qt 2 spelled a
  Designer separator `<class>Line</class>` with `orientation = Horizontal`;
  `uic3` emits `<widget class="Line" name="Line2"/>` with **no properties at
  all**, and that property is the only thing Qt 6's `uic` reads to choose a
  frame shape. Without it the widget is a bare `QFrame`, which is `NoFrame`:
  it compiles, it lays out, and it draws nothing. Restored here as
  `Qt::Horizontal`, which makes `uic` emit `setFrameShape(HLine)` and
  `setFrameShadow(Sunken)`. **This is a sixth silent-loss category** — Phase T's
  residue table lists only the 4 dropped size constraints. *C2 measured the
  scope and this paragraph first overstated it: **3 of the 6 `Line` widgets are
  affected, not 6.** `uic3` keeps an explicit `frameShape` and drops only the
  then-redundant `orientation`, so only a Line whose Qt 2 form set
  `orientation` **alone** loses everything — C1's `Line2`,
  `MT_StatisticsWidgetBase`'s `Line20` and `SIG_LanguageParametersBase`'s
  `Line1`.* `check.sh` counts `Line` widgets against those carrying **either**
  property.

**A sorting divergence, measured on both sides.** `listviewHosts`'s four columns
are all `clickable` in Q3Header, which has no per-column Qt 6 spelling; the
equivalent is `QTreeView::sortingEnabled`, now set in the form. But Qt 2's
`QListView` **also sorted by column 0 ascending by default** —
`qlistview.cpp:1836-1837` in the vendored source sets `sortcolumn = 0;
ascending = TRUE`. Qt 6's `setSortingEnabled(true)` leaves the indicator on
column 0 **descending**: measured with a `QTreeWidget` probe, section 0 and
`Qt::DescendingOrder` before and after the call. Restored with an explicit
`sortByColumn( 0, Qt::AscendingOrder )` in the base class, because nothing in
the `.ui` can express it and nothing in SIGEL calls `setSorting` on this view.
*Row order is unaffected on shipped data* — column 0 holds a pixmap and no text,
so every sort key is empty. What differs is the indicator arrow and therefore
which way the user's first header click sorts. `resizable` needed nothing:
`QHeaderView`'s default resize mode is already Interactive.

*Two other views do call it —* `SIG_ExperimentListView.cpp:51` and
`MT_GUI/MT_ExperimentWidget.cpp:11` both `setSorting( -1 )`, which is Qt 2's
`Unsorted`, i.e. `setSortingEnabled( false )`. C6 and C7.

**The conversion was checked for content loss, not just for compiling.**
Comparing the committed Qt 6 form against `HEAD`'s Qt 2 form:

| | |
|---|---|
| widget and object names | 163, all present, after writing `uic`'s four duplicate-`tab` renames into the form |
| visible `<string>`s | **98, all present.** *This row first reported `sliderMOD`'s empty `caption` as dropped. It is not: `uic3` writes it as the self-closing `<string/>`, which the comparison regex did not match. 97 + 1 = 98. A loss reported that did not happen — the wrong-referent shape again, in the table written to catch losses. Found by review.* |
| tab stops | 47 = 47 |
| connections | 26 = 26 |
| combo and list items | 16 = 16 |
| list view columns | 4 = 4 |
| widget classes | identical after the two documented remaps, with 7 `QLayoutWidget` flattened to `<layout>` |

**None of those 7 `QLayoutWidget`s carried a property besides its name**, so the
size-constraint row of the residue table (4 as recorded then, **6** as
re-measured at C2) is entirely
in other forms and stands. *The first version of the item comparison read 16
against 82 and looked like a catastrophe; Qt 4's `<item>` is also a layout item,
so it was counting two different things. §9's wrong-referent shape, caught by
looking rather than by reporting it.*

**The duplicate `tab` names are written into the form rather than left to
`uic`.** Five widgets are named `tab`; `uic` renames the last four to
`tab1`…`tab4`, variable **and** `setObjectName`, and warns four times. Writing
those names in produces **byte-identical** generated code — verified by `cmp` —
and takes `uic`'s warning count on this form to **zero**, which is what lets the
check below treat any `uic` output as a failure.

**Build wiring.** `make forms` runs Qt 6's `uic` over `FORMS`, which lists the
**converted** forms only, so an unconverted one cannot be fed to it by accident.
`rcc` compiles the extracted image **into the binary**, which is what Qt 2 did
and what a runtime file path would not do — verified by loading
`:/SIG_GPParameterBase/images/image0.xpm` from the linked resource: present, 688
bytes, and a non-null 16×16 `QPixmap`. The extracted XPM's **image payload is
identical to the embedded one** — all 37 XPM rows compared, and the file is
byte-identical to what `uic3 -extract` produces. *Not* byte-identical to the
inflated `XPM.GZ` itself: `uic3` rewrites the C declaration around it
(`static const char* const image0_data[]` for `static char *dummy[]`). An
earlier draft said "byte-faithful" without that qualifier. It is *not* interchangeable
with `pixmaps/ZORC_connected.xpm`, which is the same icon re-quantised and
differs in **94 of 256 pixels**; using the on-disk file would have been a
redesign under D21.

**`check.sh` now covers Phase C, per §7's requirement that this is part of the
first step.** A `forms (Phase C)` section, six checks per form, because they
break independently: `uic` accepts the form and says nothing; the generated
`ui_<Form>.h` compiles standalone; the committed base class compiles; `moc`
accepts it and its output compiles; the `.qrc` agrees with the header in
**both** directions; and every `Line` carries an `orientation`. Generation is delegated to the Makefile rather than
repeated, because `check.sh` disagreeing with the Makefile about flags has
already produced one phantom failure.

**The reverse resource direction is the one that matters**, and the first draft
of the check did not have it. Checking only that a referenced `:/…` is backed by
the `.qrc` passes trivially on a form that **lost** its images — there is then no
path to check, which is precisely the failure the check exists for.

**Two defects in the check itself, both found by review, both fixed.**
`pfx=$(sed ... "$qrc")` takes `sed`'s exit status, and `sed` on a missing file
exits 2 — so under `set -e` the script died **before** the `[ -f "$qrc" ]` guard
three lines below that exists for exactly that case. **19 of the 20 forms have
no `.qrc`**, so C2's first added form would have aborted the whole gate with no
`forms` line and no `total:` line printed. And a `make forms` failure was
labelled `uic FAIL` when that target also runs `rcc` and a compile, sending the
reader after the wrong tool while five passes vanished from the total; it now
says which checks did not run.

**Verified to have teeth, one mutation at a time**, as §0 requires. Baseline
`6 pass 0 fail`:

| mutation | result |
|---|---|
| `<slots>` block removed | **2 pass 3 fail** |
| iconset path typo | **3 pass 2 fail** |
| `icon` property dropped — the silent one | **3 pass 1 fail** |
| header and implementation disagree on a slot name | **4 pass 1 fail** |
| `Q_OBJECT` **corrupted** (`Q_OBJEC`) | **3 pass 2 fail** |
| `Q_OBJECT` cleanly **removed** | **4 pass 1 fail** — `moc` alone catches it. *This row was recorded as 3/2, which is the corrupted variant; the label and the mutation did not match. Found by review, which re-ran all seven.* |
| wrong `Ui::` base class | **3 pass 2 fail** |
| a duplicate widget name reintroduced | **5 pass 1 fail** |

*A sixth mutation did **not** fire and is recorded because it was nearly reported
as a pass: deleting `#include <QEvent>` from the base header changes nothing,
because `ui_<Form>.h` pulls `<QtWidgets/QApplication>` and `QEvent` arrives
transitively. The include is kept — a header should include what it uses — but
no check can see it.*

**What C1 does NOT cover, and C9 must.** Nothing renders this form. The gates
prove the form generates, compiles, mocs and that its resource path resolves;
they cannot see layout, tab order, the sort indicator, or whether the icon
reaches the widget on screen. `qrc_SIG_GPParameterBase.o` also has to be named
explicitly on the link line at C7 — a resource object that ends up inside a
static archive with nothing referencing it is dropped, and the icon then
silently disappears again.

**Gates: `./check.sh` 111 pass, 4 fail, 309 warnings** — the 4 failures are the
same Makefile-excluded files, the warning count has not moved, and the 6 new
passes are the forms section. Both baselines byte-identical, and
`ASAN_OPTIONS=detect_leaks=0 ./fitness-check.sh build` reproduces
`fitness-baseline.txt`. C1 touches no core code; the Makefile change that could
have reached it is one extra `-I` on `SIGINC`.

#### C2 — the remaining 19 forms, and a property class no check could see

**All 20 forms are Qt 6.** C1's recipe held with no change: `uic3 -convert`
(plus `-extract` for the one form with images), restore `<slots>` re-spelled to
the connections, remap the Qt3Support classes and enums, write `uic`'s duplicate
renames in, then the committed base-class pair per form from `uic3`'s own
declaration and implementation modes. **Writing `uic`'s duplicate renames into the forms and dropping the vestigial
`<pixmapfunction>` changed no generated code at all** — 17 of 19 headers
byte-identical across those two edits, and the two that moved are the signal and
the icons. *That is a much narrower claim than "17 of 19 byte-identical before
and after the hand fixes", which this section first made: restoring a `<slots>`
block, remapping a class, adding `sortingEnabled` and removing a dead property
all change the output by design, and at least 10 of the 19 moved for those
reasons. Found by review.*

**THE FINDING: `stdset="0"` decides whether a lost property is loud or silent,
and 493 property sets were riding on it.** Qt 6's `uic` emits a real setter for
a property spelled `<property name="x">` and
`setProperty("x", …)` for one spelled `<property name="x" stdset="0">`. On a
property Qt 6 does not have, the first is a **compile error** and the second
**compiles clean, sets a dynamic property nothing reads, and does nothing**.
The same Qt 2 property proved it both ways in one commit:
`showSortIndicator` on a `QTreeWidget` is a hard error in
`SIG_IndividualListBase` and a silent no-op in `MT_PopulationWidgetBase`,
differing only in that attribute.

**So the attribute is gone: 616 `stdset="0"` removed across the 19 forms C2
converted** — C1's carries none — and with it every unchecked `setProperty`
call. *The count of those was given here as 493 and does not reproduce: review
measures **492** against `uic3`'s raw output and **491** against the committed
forms with the attribute re-inserted. 616 is the reproducible figure; the
derived one depends on which tree you generate from, and should not have been
stated without saying which.* Every property is now
compile-checked. That is not a behaviour change for the ones that resolve
(`setProperty("text", v)` reaches `setText` through the meta-object), and it
turned the whole silent class loud in one edit. **It immediately found one more
loss no one had seen: `QLCDNumber::intValue`.**

**One property has to keep `stdset="0"`, and it is the exception that explains
the rule.** `intValue` **is** a Qt 6 property — `Q_PROPERTY(int intValue READ
intValue WRITE display)` — but its setter is named `display`, not `setIntValue`,
so `uic`'s set-plus-capital convention cannot reach it. Measured:
`setProperty("intValue", 42)` returns **true** and sets it. The rule is
therefore *"stdset only where the Qt 6 setter is not named `set<Property>`"*,
not *"stdset is wrong"*.

**Five properties genuinely have no Qt 6 equivalent**, all now removed from the
forms with the reason recorded here:

| property | on | resolution |
|---|---|---|
| `showSortIndicator` ×2 | `Q3ListView` → `QTreeWidget` | **redundant.** Measured: `setSortingEnabled(true)` takes `header()->isSortIndicatorShown()` from 0 to 1, and both views already get it from their clickable columns |
| `frameShape` | `Q3ProgressBar` → `QProgressBar` | Q3ProgressBar was a `QFrame`; Qt 6's `QProgressBar` is not, so it has no frame at all. *This row also listed `frameShadow`, which the widget never had — an invented entry in the table whose job is to enumerate real losses. Found by review* |
| `margin` ×3 | `Q3MultiLineEdit` → `QTextEdit`, **and** a `Line` and a `QLCDNumber` | no widget-level equivalent; Qt 6 puts it on the document. *The two extra sites are the ones `uic3` warns about — `MT_StatisticsWidgetBase/Line1` (`margin 1`) and `SIG_ExperimentViewBase/lcdnumberGenerations` (`margin 0`). Both inert — a `Line` has no contents and 0 is the default — but this table said `Q3MultiLineEdit` only while the prose quoted the warnings, without joining them* |
| `undoDepth` | `Q3MultiLineEdit` → `QTextEdit` | gone in Qt 6; the nearest survivor is `undoRedoEnabled`, which is not the same setting |

**A dead signal, found by sweeping rather than by compiling.**
`SIG_EnvironmentBase` connects `radiobuttonFunction`'s **`stateChanged(int)`** —
Qt 2's `QButton` had it, Qt 6's `QAbstractButton` does not. String-based connect
means this **compiles and fails at runtime** with "No such signal", and the
floor-selection UI would silently stop responding. Now `toggled(bool)`, which
carries the same information to a no-argument slot. *Every signal and slot in
all 49 connections was swept against Qt 6 for exactly this; it is the only one.*

**The 7 button icons did NOT need reattaching, and a first version of this step
attached them twice.** `uic3 -extract` emits the property itself, as
`<property name="icon"><pixmap resource="…">`. *This step reported it "dropped
outright" on the strength of a grep for `name="pixmap"` returning 0 — but Qt 4
renames the **property** to `icon` and keeps `<pixmap>` only as the child
element, so that grep measured a name the tool had already changed. §9's
wrong-referent shape.* The seven hand-added `<iconset>` duplicates are removed;
`uic3`'s are what ship. **The eighth image is genuinely not a form image**:
`image7` is the
Designer palette icon of the custom widget `SIG_SimulationVisualisationWidget`,
with no runtime effect; it is out of the `.qrc` and deleted, and `check.sh`'s
reverse resource direction is what flagged it.

**`QDialog` forms take a fourth constructor argument.** `uic3` emits
`(QWidget*, const char*, bool modal, Qt::WindowFlags)` for the 4 dialog forms,
and `MT_PopulationWidget.cpp:242` really does call
`MT_AddIndividualsWidgetBase( this, 0, true )`. Qt 6's `QDialog` has neither
`name` nor `modal` in its constructor, so the base does
`QDialog( parent, fl )` then `setObjectName( name )` and `setModal( modal )`.

**Two forms name a class that is not their filename**, which the build has to
respect: `MT_IndividualWidgetBase.ui` declares `MT_Individual**s**WidgetBase`,
and `MT_AddIndividualsWidget.ui` declares `MT_AddIndividualsWidget**Base**`. The
header file is named after the **form**, the class after `<class>` — that is
what the 2003 includes expect, and `include/MT_GUI/MT_AddIndividualsWidget.h`
was a **committed 2003 uic output** that this step regenerates for Qt 6.

**Two forms have no consumer, and that is recorded rather than acted on.**
`MT_ExperimentWidgetBase` is included by nothing — `MT_ExperimentWidget` derives
straight from `QListView`. It is converted anyway, per D21. *A grep that missed
it initially used plain `grep`, which is `ugrep -I` and skips the Latin-1/CRLF
files — §2's trap, hit again inside a script.*

**One form is blocked on C3 and `check.sh` says so instead of failing.**
`SIG_SimulationWidgetBase` embeds `SIG_SimulationVisualisationWidget`, whose
header chain reaches `SIGEL_CommonGUI/SIG_VisualisationWidget.h` and its Qt 2
`#include <qgl.h>`. The form is converted and its `uic`, resource and `Line`
checks run; the compile and `moc` checks are skipped with the reason printed.
**A `FORM_LIST` entry can now carry a third field naming the blocking step** —
without that, C2 would have had to either ship a fifth "known failure" or hide
the dependency.

**THE SORT DIRECTION WAS RE-CREATED IN FOUR MORE VIEWS AND CAUGHT BY REVIEW.**
C1 measured that `sortingEnabled` is only half the conversion of Q3Header's
clickable columns — Qt 2's `QListView` also sorted **column 0 ascending**
(`qlistview.cpp:1836-1837`) where Qt 6 leaves the indicator **descending** — and
pinned it with `sortByColumn( 0, Qt::AscendingOrder )`. C2 added
`sortingEnabled` to four more views and pinned none of them. **C1's note that
row order was unaffected did not carry**: it held because that view's column 0
holds a pixmap and no text, and `MT_PopListViewItem.cpp:19` and
`SIG_IndividualListItem.cpp:54` both `setText(0, …)`, so those two lists would
have displayed **reversed**. All five sorted views now pin the direction, and
**`check.sh` check 7 requires it**: any view the form calls
`setSortingEnabled(true)` on must have a matching `sortByColumn` in its base
class. *A bounded observation reported as a general one — §9's shape, and the
reason the fix is now a check rather than a habit.*

**A layout margin was deleted by hand and nothing recorded it, also caught by
review.** Stripping `margin` from the `Q3MultiLineEdit` used a `count=1` regex
that removed the **first** `margin` in the file — the top-level `QVBoxLayout`'s
`11` — leaving `SIG_ExperimentViewBase` the only one of 20 forms whose top
layout set no margin, and silently falsifying Phase T's verified
"layout margins and spacing 196/196". Restored; the count is 196 again.

**Corrections to Phase T's residue table**, both measured here: the 11 duplicate
renames are across **6** forms, not 5 (C1's plus five more); and `uic3` reports
**four** things that are not slot declarations — two `margin` properties it
cannot map, the `stateChanged` signal, and a `WordBreak` label flag.

**Gates: `./check.sh` 203 pass, 4 fail, 309 warnings.** The 4 failures are still
exactly the Makefile-excluded files and the warning count has not moved; the
forms section is **98 pass, 0 fail, 1 documented skip** — seven checks a form. Both baselines
byte-identical, sanitized run clean. Teeth re-verified after the check changed:
dropping a `Line`'s orientation gives 92/1, and a `.qrc` naming a missing file
stops `make forms` and reports which checks did not run.


#### C5 — `SIGEL_Visualisation` — DONE 2026-08-30

**Done first, not third, because the step order was wrong** — see the dependency
table above. 23 Qt 2 code sites converted; **12 of 12 sources and 12 of 12
headers compile**, once `libglu1-mesa-dev` was installed. The module is in
`check.sh`'s `MODULES` and contributes 22 warnings.

**`-lGLU` IS REQUIRED, AND §7 SAYS IT IS NOT.** Phase R records "`-lGL`, no
`-lGLU` — nothing references GLU", which was measured on the **core** build and
is false for the interface: `SIG_Visualisation.cpp` calls **`gluPerspective`**
and **`gluLookAt`**. `libglu1-mesa-dev` had to be installed — it was not, which is what blocked that
one file — and **C9's link line needs `-lGLU`**, which the Makefile now carries. *A measurement of core
quoted forward as a fact about the tree — §9's shape.*

**Ownership settled from the 1.3 binary before writing anything**, which is what
`xb/kdesigel/sigel_slave` is for:

| container | 1.3 evidence | conversion |
|---|---|---|
| `SIG_Renderer::sceneObjects`, `::floatingTexts` | ctor at `0x080cf640` makes **2** `setAutoDelete(true)` calls | owning → `qDeleteAll` in `~SIG_Renderer`, which freed **nothing** before |
| `SIG_Visualisation::floatingTexts` | ctor at `0x080ca020` makes **0** | **non-owning** — it aliases the renderer's pointers, filled from `robotRenderer.floatingTexts`. Making it owning would double-free |
| `SIG_RenderRecorder::robotLinks` | `setAutoDelete(true)`, and the class has **no destructor at all** | owning → a destructor added, declared and defined |
| `SIG_EnvironmentRenderer::robotPathPoints` | `setAutoDelete(true)`, empty destructor | owning → `qDeleteAll`. §9 flags this member by name as "the central pointer-versus-value trap, sitting unconverted" |

**All three classes that gained an owning destructor are now non-copyable.**
Qt 2's `QGVector` copy constructor cleared `del_item` (`qcollection.h`), so a
copy freed nothing; a `QList` copy shares the raw pointers and **both**
destructors `qDeleteAll`. `SIG_Renderer`, `SIG_EnvironmentRenderer` and
`SIG_RenderRecorder` get `= delete` on copy and assignment, as **D7**
(`SIG_Geometry`), **D15** (`SIG_GPPopulation`) and **D25c** (`SIG_GPManager`)
each did. *C5 first added the destructors and left the classes copyable —
the fourth time this file has recorded that precedent and the third time it was
not applied. Found by review; no copy site exists today, but C3 and C4 write new
code against exactly these classes.*

**THE ONE `insert` IN THE MODULE WAS MISSED, AND REVIEW CAUGHT IT.**
`SIG_SimulationVisualisation.cpp:63` fills `SIG_Visualisation::floatingTexts`
with `insert( i, … )`. Qt 2's `QVector::insert` **overwrites** slot `i`; Qt 6's
`QList::insert` **shifts** — so the list came out **twice its size with a null
tail**, and C3's `paintGL` indexes it against a widget list sized from
`floatingTexts.size()` and dereferences without a null check. Live code: the
x86 box measured `paintGL` at 1785 hits. Now a slot assignment. *The sweep was
keyed on the container **type** and not on the operations performed on it —
§9's "a sweep keyed on container type name" failure, verbatim, in the step whose
own table cites D25c's insert/shift row.*

**A range check was lost at a file-driven index, and D6, D8, D9 and D25c all say
to record that.** `sceneObjects.insert( number, … )` became
`delete sceneObjects[ number ]; sceneObjects[ number ] = …` at
`SIG_RobotRenderer.cpp:46`, where `number = actLink.getNumber()` is **read from
the model file** (`SIG_Link.cpp:65`). Qt 2's `QGVector::insert` was
range-checked and returned false; `QList::operator[]` asserts in these builds
and is an out-of-bounds write under `-DQT_NO_DEBUG`. Latent — the compiler
numbers links contiguously from 0 — but it is a file-data index, not a derived
one.

The two `robotPathPoints` cursor walks became index walks; both were guarded by
`count() >= 2`, which is what kept `first()` off an empty list — UB in Qt 6.

**Seven `QDictIterator` walks became range-for** over D4's `getLinks()`,
`getMaterials()` and D5's `getPoints()`. The dict **key** is the point's name,
so `pointIter.currentKey()` is `NamedPoint::name` and `*pointIter.current()` is
`*NamedPoint::value`. `robot.getLinkIter().count()` → `getLinks().size()`, **8**
sites — 4 in `SIG_RobotRenderer.cpp` and 4 in `SIG_SimulationVisualisation.cpp`,
plus a ninth `iter.count()` in `SIG_RobotVisualisation.cpp`. *Recorded as 6;
all were converted, only the figure was wrong.* `QDictIterator::count()` is the
dict size and is position-independent (`qdict.h`), so the exhausted-iterator
call maps correctly.

**`QArray<T>` → `QList<T>` by value**, 7 sites feeding GL through `.data()`.
*Qt 2's `QArray(n)` left the elements **uninitialised**; `QList(n)`
value-initialises. Every element here is written before use, so this only
removes undefined reads.*

**Four Qt 2 API classes the container sweep could not see**, all found by
compiling: `IO_WriteOnly` → `QIODevice::WriteOnly` (9), `QString::utf8()` →
`toUtf8()`, `QTextStream::precision()` → `setRealNumberPrecision()`, and **8
dynamic exception specifications**, which C++17 removed.

*§9's fourth-family note says "nothing in the tree sets `setRealNumberPrecision`
… on a file stream — only `SIG_GPPVMData` does, at 50, for the wire". That is
true as worded but narrower than it reads: `SIG_Renderer::vectorToPovray` sets
precision **5** on a `QString` stream, and that is the POV-Ray export path — the
one the x86 box confirms actually works and produced every published film.*

**Four core getters gained `const`** — `SIG_Environment::getWithTexture`,
`getDMEnvironment`, `getTexAlpha`, `getTextureFile` — because a `const
SIG_Environment &` cannot call a non-const getter. Adding `const` to a getter
cannot change behaviour, and the three gates stayed green.
*But "all three gates confirm it moved nothing", as this said, is a wrong
referent: **no gate calls any of the four.** Their only callers are
`SIG_EnvironmentRenderer.cpp` and `SIG_EnvironmentView.cpp`, neither of which is
built. What the gates confirm is that the header change broke nothing that IS
built. The `const` is safe by inspection — header and definition agree at all
four, `SIG_Environment` has no derived class and no const/non-const overload
pair — not by test. Found by review.*

#### The build was choosing between the two `SIG_GPExperiment` variants by luck

**Found while running C5's gates, and it is not a C5 defect.** §9 records that
the class is defined twice on purpose — `SIG_GPExperiment.cpp` for `sigel`,
`SIG_GPExperimentClean.cpp` for the slave, differing in whether the constructor
builds an `MT_Controller` — and asserts that `sigel_eval` links Clean. **Nothing
enforced that.** Both objects go into `libSIGEL_GP.a` and the linker takes the
first member defining the symbol.

**THE VARIABLE IS THE LOCALE.** `$(wildcard)` *does* sort — by `strcoll`, which
is locale-dependent — and the two names order differently either side of the
`Clean` suffix:

```
LC_ALL=C          SIG_GPExperiment.cpp, SIG_GPExperimentClean.cpp   -> master first -> LINK FAILS
LC_ALL=en_US.UTF-8  SIG_GPExperimentClean.cpp, SIG_GPExperiment.cpp -> Clean first  -> links
```

*This section first blamed clean-versus-incremental builds and said
"`$(wildcard)` does not sort" and "sorting would be worse — alphabetically the
master wins". All three are wrong, and the archive recipe cannot produce that
flip at all: it does `$(RM) $@` then `ar crs $@ $^` every time, from a fixed
order. What actually differed between my two builds was `LC_ALL=C`, exported in
one command and not the other — I attributed the result to the variable I
changed deliberately rather than the one I changed by accident. Found by
review.* A future reader chasing a recurrence should look at the environment,
not at build freshness. `SIG_GPExperimentClean.o`
is now named explicitly ahead of the archives, as 2003 compiled it into the
slave target, with an assertion after the link. Today the linker fails first on
the master's undefined `MT_Controller`; **the assertion becomes the only guard
once C6 makes `MT_Controller` link**, at which point the master would link
silently and `sigel_eval` would start constructing one per experiment.

#### What the 1.3 oracle could and could not answer for C3

Asked because C3 converts `QGLWidget` → `QOpenGLWidget`, and the floating text
labels are child `QLabel`s over the GL view — the case where the two differ
most: `QGLWidget` is a native child window, `QOpenGLWidget` renders to an FBO
inside the normal widget stack.

**Answered.** The state is a checkbox, "Show anchor points", at the bottom of the
Control tab below Show plane / Show grid / Show robot path, wired to
`setShowAncorPoints(int)`. It behaves as a **two**-state box; the box could not
establish a third state, so which of our 0/1/2 the middle value maps to is
unknown. The label code is **live, not dead**: with gdb on the running slave,
`SIG_SimulationVisualisationWidget::paintGL` 1783 hits,
`SIG_VisualisationWidget::paintGL` 1785, `initFloatingTextWidgets` 1 — it fired
on ticking the box. And `SIG_VisualisationWidget` is never constructed alone;
its constructor has exactly one caller, the subclass.

**NOT answered, and the reason is worth more than the answer.** The 3D view on
that machine renders **nothing at all** — not the labels and not the robot —
with no GL errors logged, while `paintGL` ran 1785 times. So the absence of
labels is equally consistent with "they do not composite over a native GL
window" and "nothing renders in that environment", and the robot's absence makes
the second the likely reading. The box declined to report the first, which would
have been measuring one thing and concluding about another.

**Consequence, recorded rather than assumed: the labels are *unverified*, not
unexercised.** The code demonstrably runs; whether a 2003 user ever saw them is
not something that machine can currently show. **So "port faithfully" and "make
it work" cannot be separated here**, and C3 should convert the code as written
and record that Qt 6 may well make the labels *more* visible than Qt 2 did —
that would be a behaviour change nobody can currently detect either way. *Also
learned: that box's SIGEL visualisation has always meant the **POV-Ray export**,
which computes geometry and writes files without the GL view displaying. The
on-screen GL side of the 1.3 binary is effectively untested there.*


#### C3 — `SIGEL_CommonGUI`: `QGLWidget` → `QOpenGLWidget`

Two sources, 665 LOC, done after C5 because it reads C5's `floatingTexts`.

| was | now |
|---|---|
| `class … : public QGLWidget`, `#include <qgl.h>` | `QOpenGLWidget`, `<QOpenGLWidget>`. The three virtuals `initializeGL`/`resizeGL`/`paintGL` are unchanged, and raw GL calls still work because the context is current inside them |
| `QGLWidget(parent, name, 0, f)` | `QOpenGLWidget(parent, f)` + `setObjectName` |
| `updateGL()` ×4 | `update()` — **and these are not equivalent.** Qt 2's `updateGL()` **is** `glDraw()`: `makeCurrent(); paintGL(); swapBuffers();`, all before the call returns (`qgl.cpp:1394,1600-1618`). Qt 6's `update()` posts a paint event. Safe at C3's four sites — none reads back anything `paintGL` produces — but **C4 has 16 more and one is on the movie path**: `SIG_SimulationVisualisationWidget::slotSimulationProgress` does `makeTimeSteps(1); updateGL();` where `makeTimeSteps` grabs the frame *before* the redraw. C4 must decide that deliberately |
| `WFlags` ×2 | `Qt::WindowFlags` |
| `QVector<SIG_FloatingTextLabel>` + `setAutoDelete(true)` | `QList<SIG_FloatingTextLabel *>` with the frees written out |
| `QLabel( parent, name )` | `QLabel( parent )` + `setObjectName` |
| `LeftButton`, `RightButton` | `Qt::LeftButton`, `Qt::RightButton` — Qt 2's button constants were global |
| `QMouseEvent::state()` | **`buttons()`**. Safe only because both uses are in `mouseMoveEvent`, where the two agree; on press and release Qt 2's `state()` was the state *before* the event |

**The double ownership is real and was safe by destruction order.** The labels
are Qt **children** of the widget *and* were owned by the vector's
`autoDelete`. Not a double free: the member is destroyed before the `QWidget`
base, so each label unparented itself and `~QWidget` found none left —
disassembly of the 1.3 slave shows exactly that — **`~QVector` at `0x0806edec`,
then `~QGLWidget` at `0x0806edfc`**. *This cited `0x0806edd7`, in three places
at once including a permanent source comment. That address is the `call *%esi`
that dispatches `delete visualisation` and has nothing to do with either
destructor: the claim was true and its only evidence pointed elsewhere, so
anyone re-deriving it would have landed on the wrong instruction and concluded
the record was invented. Found by review.* The `qDeleteAll` is in the destructor **body**, which runs
before the base, so the order is preserved. `initFloatingTextWidgets`'
`clear()` was likewise the free for the previous labels; without an explicit
`qDeleteAll` they would survive as children and **stay on screen**.

*The fill loop uses slot assignment, not `insert` — C5's review found exactly
that mistake one file away.*

**The x86 box's open question is answered here, locally.** It could not
establish whether the anchor-points checkbox has a third state. The form says
so: `showAncorPointsCheckBox` carries `tristate=true`, so the 0/1/2 our source
branches on are `QCheckBox`'s Unchecked / PartiallyChecked / Checked, and
**only the fully-checked state shows the labels** (`showAncorPointsState == 2`).
The box observed a two-state toggle, which is one click on a tristate box.

**What is still unverified, and it is the honest limit of this step.** Whether
those labels were ever *visible* over the GL view in 2003 is unknown — see the
oracle exchange under C5. `QOpenGLWidget` composites children correctly where
`QGLWidget`, a native child window, often did not, so this port may well make
them **more** visible than Qt 2 did. Nobody can currently detect that in either
direction, and C9 is the first step that could.

**C3 introduces a new Qt module dependency, and C9 needs it on the link line.**
`<QOpenGLWidget>` resolves only via `QtOpenGLWidgets`. That went into `check.sh`
and, until review caught it, nowhere else — the Makefile's `SIGINC` still named
only `QtCore QtGui QtWidgets` and neither link line had `-lQt6OpenGLWidgets`.
Both now carry the include paths and `-lQt6OpenGLWidgets -lQt6OpenGL … -lGLU`,
verified by a from-scratch build of both trees. *`check.sh` carries a comment
warning that disagreeing with the Makefile about flags has already produced one
phantom failure; C5 wrote a whole paragraph for exactly this shape of forward
requirement and C3 wrote none.*

**The module's 10 new warnings, named, because §7 says warnings are part of the
criterion.** Six are `QMouseEvent::x()`/`y()`, kept verbatim — correct and
widget-relative in Qt 6.10, but `QT_DEPRECATED_VERSION_X_6_0("Use position()")`.
Three are one pre-existing `-Wreorder` from the 2003 initialiser list, and one
is an unused parameter in `SIG_FloatingTextLabel::mousePressEvent`. So C3 owns
7 of the 10, and the `x()`/`y()` six are a **deferred conversion**, not a clean
port.

**A build churn that could have made a gate lie, fixed.** `make` treats the
generated `moc/*.cpp` as **intermediate** and deletes them after linking, so the
next `make` regenerates them, recompiles the objects and relinks — leaving
`sigel_eval` newer than nothing and older than its own prerequisites. The three
gate scripts run `make -q` first and refuse a stale binary (the D13 trap), so
this shows up as "is out of date" on a tree where nothing changed. `.SECONDARY`
keeps them; `make -q` is now clean after a second `make`.

**`SIG_SimulationWidgetBase`'s form is still blocked, but on C4 now, not C3** —
its custom widget's own header (`SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.h`)
still declares `WFlags`. `check.sh` names the new blocking step.

**Gates: `./check.sh` 217 pass, 4 fail, 341 warnings.** The 4 failures are the
same Makefile-excluded files. **Warnings rise 309 → 341, and the arithmetic is
exact**: `SIGEL_Visualisation` 22 + `SIGEL_CommonGUI` 10, both newly covered;
nothing in previously covered code moved. Both baselines byte-identical,
sanitized run clean.


#### C4 — `SIGEL_SlaveGUI`: the toolbar, the movie path, and a toolbar that did nothing

16 files, 2,344 LOC, **30 Qt 2 code sites** plus 27 Qt 2 `<q*.h>` includes. It
unblocks the last form: `SIG_SimulationWidgetBase` now compiles, so the
`forms` section runs all seven checks on all 20.

**THE FINDING THIS STEP WAS NAMED FOR DOES NOT EXIST.** It claimed that Qt 2's
`QAction::init()` self-inserted into a `QActionGroup` parent, that **Qt 6 does
not**, and that without added `addAction` calls the toolbar would render empty.
The first half is true; **the second is false, and has been since Qt 4**. Qt 6's
`QAction(QObject *parent)` does

```cpp
d->group = qobject_cast<QActionGroup *>(parent);
if (d->group) d->group->addAction(this);
```

Measured on 6.10.2 with 1.3's exact shape and no `addAction`: `actions()`
returns the action and `actionGroup()` is the group. The six calls this step
added were idempotent no-ops; they and their comments are removed. *Found by
review. The error mattered beyond this step — C7 builds three more groups and
would have inherited the wrong model — and the real hazard there is the
opposite one: two of those three pass Qt 2's `exclusive = false` third
argument, which Qt 6's constructor has no place for and which defaults to
**true**.*

**THE REAL "COMPILES, LINKS, RUNS, DOES NOTHING" DEFECT WAS IN THIS MODULE ALL
ALONG, AND THIS STEP WALKED PAST IT.** `QAction::activated()` does not exist in
Qt 6 — the signal is `triggered()`. A string-based connect to a signal that is
not there compiles, links, and silently never fires. C4's own two files carried
**nine** of them, plus `QComboBox::activated(const QString &)`, which Qt 6
renamed `textActivated`. So after C4 as first committed, the toolbar drew six
buttons and **not one did anything** — play, stop, step, fast-forward,
movie-settings, quit — and the render-mode combo was dead. All ten repaired.

**AND §2 IS WHAT MISLED ME**, with "The 444 `SIGNAL()`/`SLOT()` macros … are
**all still valid**". Rather than fix the two signals this step tripped over, I
resolved **every distinct signal signature in the tree** — 37 of them, once
whitespace-only spelling variants are normalised the way `connect()` does —
through `QMetaObject::indexOfSignal` on the real Qt 6 meta-objects. **Nine are dead, not
two**, and 90 connects to them remain in C6, C7 and C8. §2 carries the table,
the deliberate non-entries and the per-module counts, and `check.sh` gates them.
*The narrow fix would have left seven kinds of silent breakage for the later
steps to walk into exactly as this one did.*

**The sweep was right to run and still got two entries wrong, in opposite
directions.** `lostFocus()` was listed as dead and is not — it is `MT_Editor`'s
own signal — and `selectionChanged` was recorded only in its no-argument form
when Qt 2's `QListView` declared an item-argument overload as well. Nine kinds
is the corrected figure and 100 the corrected total; the two errors cancelled,
which is exactly why a total that reproduces is not evidence that its parts do.
§2 has both.

**The movie path: `QPixmap::grabWindow` is gone, and its replacement changes
which frame is recorded.** Qt 6 removed it; for a `QOpenGLWidget` it was wrong
anyway, since it read the window's on-screen pixels while the widget now
renders into an FBO. The equivalent is `grabFramebuffer()`, so the pixmap
becomes a `QImage` and `QWMatrix`/`xForm` become `QTransform`/`transformed`.

**The behaviour change is forced, not chosen, and is recorded because nothing
can see it.** Every caller runs `makeTimeSteps(n); update();` and the grab
happens **inside** `makeTimeSteps` — so 1.3 captured the *last presented* frame
while the simulation already stood one step further, i.e. frame N−1.
`grabFramebuffer()` renders current content, so Qt 6 records frame N. It removes
a one-frame lag that no like-for-like port could have kept. **This is the raster
path only** — `fileFormat == "pov"` takes a different branch, and per the x86
box the POV export is what produced every published film.

*`updateGL()` → `update()` is safe here for the same reason: the grab precedes
the repaint in every caller, so making the repaint deferred does not change
which frame is grabbed.* 15 sites.

**Two further forced divergences on this path, which the step made and did not
record.** *Found by review — they were in the diff and in the source comments,
but in neither the commit message nor this file, and the rule is that every
forced divergence is written down.*

**A negative width or height reached `QImage::copy` and produced a null image.**
Qt 2's `QPixmap::grabWindow` treated a negative `w`/`h` as "to the window edge"
(`qpixmap_x11.cpp`: `if ( w < 0 ) w = a.width - x;`). Qt 6's `QImage::copy` does
not — it returns a **null image**, `save()` fails, and `callRenderPixMap`'s
unconditional `return true` reports the failure as success, so no file is
written and nothing complains. Restored explicitly:
`if ( pW < 0 ) pW = grabbed.width() - pX;` and the same for height. The three
callers are exhaustive: `cropImage` sets both, `keepRatio` sets neither, and the
third branch's two arms cover `a>=1||b>=1` and its negation, each setting one.

**`grabFramebuffer()` returns device pixels, so on a HiDPI screen the movie
frames come out at the wrong size.** Qt 2 had no device-pixel-ratio concept and
1.3's frames are logical pixels. The grab is normalised back —
`scaled(size(), IgnoreAspectRatio, SmoothTransformation)` then
`setDevicePixelRatio(1.0)` — which introduces a resample that 1.3 never did, on
HiDPI only; at ratio 1.0 the branch does not run and the bytes are unchanged.

**And the status bar is adopted, not merely constructed.** 1.3 wrote
`new QStatusBar( this )` and relied on `QMainWindow` adopting any `QStatusBar`
child through `ChildInserted` (`qmainwindow.cpp:2335`). Qt 6 has no such
adoption, so it is now `setStatusBar( new QStatusBar( this ) )`.

| other Qt 2 API | Qt 6 |
|---|---|
| `QIconSet` ×18, `setPixmap(file, QIconSet::Large)` ×8, `setIconSet` ×13 | `QIcon`, `addFile(file)`, `setIcon` |
| `QMainWindow::setUsesBigPixmaps(true)` | `setIconSize( QSize(25,25) )` — **measured from the XPMs**, seven at 25×25 and `quitApplicationSmall` at 24×24, not chosen |
| `setUsesTextLabel(false)` | `setToolButtonStyle( Qt::ToolButtonIconOnly )` |
| `setDockEnabled( Left/Right/Bottom, false )` | no per-window equivalent; it is per toolbar now — `setAllowedAreas( Qt::TopToolBarArea )`, since only Top was left enabled |
| `QAction`/`QToolBar`/`QTimer`/`QMainWindow` `(parent, name)` ctors | `(parent)` + `setObjectName` |
| `QFileDialog::getExistingDirectory( dir, parent, name, caption, dirOnly )` | `( parent, caption, dir, ShowDirsOnly )` — every argument survives but the widget `name` |
| `QComboBox::setCurrentItem` ×6, `QString::lower`, `QTimer::changeInterval` | `setCurrentIndex`, `toLower`, `setInterval` |
| `QButton::Off` / `NoChange` / `On`, `state()` | `Qt::Unchecked` / `PartiallyChecked` / `Checked`, `checkState()` |

**That last row independently confirms C3's tristate reading.** The handler
switches on the checkbox and maps the three states to 0/1/2 explicitly, so the
enum's numeric values never mattered; and `showAncorPointsState == 2`, which is
what shows the floating labels, is `QButton::On` — fully checked. C3 read that
off the form's `tristate` property; this reads it off the handler.

**A defect preserved, and it is why the module gains a warning.**
`callRenderPixMap` assigns `res` from `save()` and then `return true`
unconditionally, so a failed frame write is reported as success and the caller's
`QMessageBox` and recording-stop are dead code. `-Wunused-but-set-variable` is
the compiler saying exactly that. Not fixed: it changes behaviour against 1.3 on
a path nothing here can exercise.

**A mistake of mine worth recording, because it is the same shape as the ones
review keeps finding.** The regex that rewrote `new QAction( this, "x" )` into
three lines matched a **commented-out** line and expanded it into one commented
line plus two live ones — dereferencing an uninitialised `recordAction`. Caught
by reading the output rather than by the compiler, which would have accepted it.
*A pattern applied to source that does not know what a comment is.*

**`check.sh` had to be restructured, and the bug it fixes was real.** GUI module
headers include the generated `ui_<Form>.h`, but `make forms` and the
`-I build/ui` flag lived **inside** the forms section, which runs *after* the
module and header passes. Five `SIGEL_SlaveGUI` headers failed the standalone
pass for want of the flag. Generation and the include path are now hoisted above
the module loop.

**Gates: `./check.sh` 393 pass, 5 fail, 355 warnings** — on a changed basis,
so it is not comparable with the 227/4 recorded through C3. The standalone
header pass (132/1) had never been folded into the totals, so the headline
"fail" had been silently excluding a real header failure; folding it in, plus
the 33-row regex self-test, accounts for the whole move: 228 + 132 + 33 = 393
and 4 + 1 = 5. The 5 failures are the same Makefile-excluded files. **Warnings 341 → 355, and the arithmetic is
exact**: all 14 are `SIGEL_SlaveGUI`'s — 12 `-Wreorder` from the 2003
initialiser lists (8 in `SIG_SimulationVisualisationWidget.h`, 4 in its
`.cpp`), 1 `-Wunused-but-set-variable` (the preserved `res` defect above) and 1
`-Wunused-parameter` in `SIG_SimulationControls.cpp`. Headers 132/1, forms
101/0 now that the last form is unblocked. Both baselines byte-identical,
sanitized run clean.

**`check.sh` gained three gates in this step, all proved by injection rather
than asserted.** `$DEAD_SIGNALS` fails a module on any connect to a signal Qt 6
does not have, against a per-module baseline that is **zero for every converted
module** — so C6, C7 and C8 can only reduce it. And the uic-warning check was
found *dead*: hoisting `make forms` above the module loop left a second,
no-op `make forms` in the forms section whose empty output overwrote the real
run's log, so the check C1 added to catch a dropped `<images>` block could not
fire. It now reuses the first run's log via a `FORMS_FAILED` flag, and
re-injecting an `<images>` block makes it fail again. *Two gates that reported
green while seeing nothing.* `MODULES="${1:-…}"` also became `${*:-…}`: it had
silently ignored every module argument after the first.

**The third gate reads a file format, which is a kind of check this project did
not have.** Everything else here is a compile check, and the four behaviour
gates only exercise what a fitness evaluation touches — so a format the program
parses but no gate opens is covered by nothing. `check.sh` now builds and runs a
`PVMHOST` round-trip against `SIG_GPPVMHost`, pinning 1.3's own line from
`twoBasesSimpleFitness1.exp`. It fails on the naive conversion and passes on the
faithful one. *Written because a defect of exactly that shape was found in
`SIGEL_GP` — see §9 — in a module converted long ago and green throughout.*


---

## 8. Steps and status

**Phase status is the table at the top of this file.** A second one lived here
and drifted: it had Phase B at "8 of 14", Phase C at 10 steps and Phase V at 5
where §7 defines C1–C9 and V1–V9, and it omitted Phases D and R entirely.

**No effort estimates in this file.** The column that held them carried six
invented figures. Step counts are counted and stay. **Do not put estimates
back.**

**A single fitness evaluation runs locally with no PVM** — that is `sigel_eval`
and all of Phase R. What has no local path is the **evolution loop**:
`SIG_GPFitnessTrainer` dispatches every evaluation through `pvm_spawn` of
`sigel_slave` with no in-process fallback, and the one method that looks like a
fallback, `SIG_GPExperiment::calculateFitness` (`SIG_GPExperiment.cpp:158`), is a
stub that returns 0. So without PVM the ported interface builds and shows its
windows, and nothing happens behind the Start button.

---

## 9. Open

### Ownership hazards Phase C inherits (was: the Phase B audit)

**11 `setAutoDelete` calls remain, all in GUI modules** — core is 0, and C5
removed 4. *Counted as calls, not lines: a `command grep` for the word returns
13, two of which are comments.* Two of them flip the flag at runtime; §9 "Toggling containers" names both.
**21 pointer containers own with no flag at all**, measured on the pristine
tree and **not re-measured for the GUI** — D7's blanket rule says nothing about
that class, so each GUI container needs its ownership read rather than inferred
from a flag.

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

### Toggling containers (Phase C)

Two containers flip `autoDelete` at runtime — Qt 2's "remove without deleting"
idiom, which a single destructor free does not reproduce. Both GUI, both need
per-site thought:

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
| the master's output | one line per generation, and nothing else. **No fitness value is ever printed.** Fitness lives only in the `.pol` pool images and the rewritten `.exp`, as `FITNESS=<value>`; `-1` means unevaluated, which is what V4 exploits |
| per-step trajectories | POV-Ray export exists only in `sigel_slave`'s visualiser widget and is driven through its GUI. **Not reachable headlessly** |
| how the binaries run | natively on x86-64 — woody's own loader invoked explicitly against unpacked woody `.deb`s. No chroot, no container, no root |

**`twoTri` on that box is not reference material.** It is not in
`robots.tar.gz`, which holds exactly the 7 models named above, and it appears
nowhere in `data/`; verified 2026-08-27. That box's own experiments, modified
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

**PHASE C MUST ADD `QHashSeed::setDeterministicGlobalSeed()` TO `sigel.cpp` AND
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
| out-of-range array access | warn, clamp to 0 | same, in the shim | Not via `Q_ASSERT`, which compiles to nothing under `QT_NO_DEBUG` — a release build would corrupt memory silently where 2003 returned a wrong value |
| `SIG_ProgramLine.cpp:215-224` | writes `element[no]` in the branch entered *because* `no >= size()` | to be fixed | Its own comment is `// ToDo: Exception!` |
| ~~`SIG_DynaSystem.cpp:266-268`~~ | deletes `dynaJoints[k]` while looping to `dynaDrives.size()` | **moot 2026-08-28** — the file is deleted with the Dynamo backend, `physics_backends.md` | The two vectors grew independently |
| `SIG_EarlyRunTermSimulation.cpp:97` | `QTime zeroHour;` | `QTime( 0, 0 )` | Same class as the other 11 `QTime()` sites but a declaration, so the first sweep's pattern missed it. `getMaxRecorderSteps` returned 2 instead of 182 — a factor of 91 on the denominator of three fitness functions. No shipped experiment selects them, so `replicate.sh` cannot see it |
| `sigel_slave`, `getenv("SIGEL_ROOT")` | dereferenced unchecked | to be fixed | Segfaults if unset; the SIGSEGV handler masks it with no core. Bites under PVM specifically — spawned tasks inherit *pvmd's* environment, not the master's |
| `SIG_GPPVMData.cpp:51` `sendQStringToPVM` | sends `str.length() + 1`, a **character** count, then sends `str.toUtf8()`, up to 4x longer in bytes | `qCStringBuffer.size() + 2` (D21; was `+ 1` on a `Q2CString`) | `getQStringFromPVM` sizes its receive buffer from that count and lets `pvm_upkstr` write the bytes in. 20 `ü` gives `heap-buffer-overflow ... in byteupk` under ASan; short strings survive only because `QList` over-allocates. Qt 2's `length()` was the Latin-1 byte count, so 2003 was right for its own data. **Changes the wire format for non-ASCII** — safe only because both ends are this file and no distributed run exists. Found by Phase P's P4, regression-tested by `pvm_link.cpp` |
| `SIG_GPIndividual.cpp:557-559` / `:647` | the writer emits `"\n      "` before `}HISTORY END;`; the reader takes everything up to that marker as content, so the separator becomes data | **preserved, not fixed** | Every save grows every `HISTORY` block by 7 bytes, linearly and without limit — 100 blocks is ~700 bytes per round trip. Measured on the 1.3 binary over three consecutive round trips (V8) and confirmed to be the same code here. Fixing it would change file bytes against 1.3. Any gate that diffs a round-tripped `.exp` must normalise trailing whitespace inside these blocks |
| `SIG_GPPVMData::sendQStringToPVM`, a **null** `QString` | `Q2CString`'s `const char *` conversion gave `nullptr`, and `pvm_pkstr` does `strlen(cp)` unguarded — a segfault | `constData()` gives `""`; an empty string is sent | Found by the D21 review, which showed the `+ 2` does not reproduce the old length for a null string. It never could: the old path died before the length was used. Unreachable today — the two live callers pass a string built by `savePVMDataTransfer` — but it is a crash removed, not a value preserved, and D21 first claimed otherwise |
| `SIG_GPForceFitnessFunction`'s cleanup loop | a `do`/`while` dereferencing `listForces.first()` **before** testing it | a range-for | `Q2PtrList::first()` returned null on an empty list, so an evaluation that recorded no frames took a null dereference **while freeing memory**. Identical with frames, a no-op without. Contrast D10, where the same shape's once-through was load-bearing and had to be kept — which side of the null the body is written for must be read each time, not pattern-matched |
| `SIG_Environment` terrain load | `getenv("SIGEL_ROOT")` unchecked | already checked, message on stderr | `sigel_eval` says "SIGEL_ROOT is not set, cannot locate Terrain.ter" instead of reading `/Terrain.ter` |

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
  and will abort now. `MT_GUI` is in neither `check.sh`'s `MODULES` nor the
  Makefile's `CORE`, so nothing flags it.

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

**Our source really does use `long double`** — the only `long double` in the
whole tree, at `SIG_DynaMechsCommandInterface.cpp:49,65-69,102-111`.

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

**A consequence that sharpens V4.** The file format carries only 6 significant
digits, so **any fitness comparison mediated by a file has 6-digit
granularity** — architecture differences below the sixth significant figure
cannot appear in a `.pol` at all. That gives V4 a natural form: *do the two
builds agree to 6 significant figures on a fixed program?* If yes the files are
byte-identical and there is nothing left to argue about tolerance. The gate
falls out of the format instead of being chosen. **This does not touch the
chaotic-amplification question**, which is about the simulation diverging over
time, not about representation.

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

**And it sharpens V4 in a direction that was missed.** The
"6-significant-digit granularity" argument is about **storage**. Fitness crosses
the PVM wire at **precision 50**, so the master compares full-precision values
and truncates only when writing: two builds can agree to six figures in the
`.pol` and still make different selection decisions. V4's digests survive that —
they cover names and programs, so a selection difference surfaces as a different
population at the next snapshot — but "agree to six figures and the files are
byte-identical" is not a statement about what the algorithm consumed.

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

### D3 — the flip, done 2026-08-27

`Q2Dict` is now **insertion-ordered**: one chain, appended to, walked in order.
`find` searches it backwards so newest still wins, as Qt 2 did. Nothing places
by hash any more.

**Two gates, both committed, both clean:**

```
./dictorder-dump.sh   | diff -u dictorder-baseline.txt -
./fitness-check.sh    | diff -u fitness-baseline.txt -
```

| check | result |
|---|---|
| `copy` order, the order the simulation runs on | **0 of 14 blocks changed** |
| `rrb` order, the four numbered kinds and all 60 point lists | **0 of 7 changed** |
| fitness, 3 individuals × 14 experiments | **identical on all 42** |
| `loaded` order | **8** of 14 changed — **correct**, see below |
| `rrb` body and material order | 6 of 7 changed — free, see above |

`loaded` had to move. It was `hash(file)` and is now file order, which is
exactly what collapses it onto `copy` — the three load paths becoming one order
is the point of the exercise, not a regression. `body` and `material` order
moved with it, and nothing numbers either.

`fitness-baseline.txt` pins this machine against itself and is **not** a
cross-machine reference: §7 measures a 1-ULP change in start height moving an
individual by 45%, and the 2003 build was i386 x87.

`data/` stays the pristine download and is never written; `data-reordered/` is
what the build reads, and both scripts default to it.

### D4 — `SIG_Robot`'s six dicts are plain Qt 6

`Q2Dict<T>` → **`QList<T *>`** for all six. No new container type, nothing that
emulates Qt 2, and the header shrinks by that much.

What made it a rename rather than a redesign: **every insert site keyed on the
object's own name** — `bodies.insert(b->getName(), b)` and the five like it
(`SIG_Robot.cpp:105-134`), so the key was derivable and the dictionary was
carrying nothing the list does not. `lookupX(name)` became a linear scan, over
at most 19 links.

`getXIter()` is replaced by `getXs()` returning `const QList<T *> &`. That breaks
**22 call sites in `SIGEL_Visualisation` and `SIGEL_MasterGUI`** — both already
fail to build, both are rewritten in Phase C, and per D25 they should meet the
plain accessor rather than a shim one.

`clear()`'s six hand-written delete loops became `qDeleteAll`, in the same
reverse-of-construction order.

Counts: `Q2Dict` 42 → 37, `Q2DictIterator` 57 → 16. What is left is
`SIG_Link::points`, which maps a name to a `DL_vector` that has no name field,
so it needs an actual key and is its own step.

**Checked under AddressSanitizer**, because this changed a free path: no
use-after-free, no double-free. `octopus` and `twoBases` leak exactly the
documented 41,374 bytes in 117 allocations.

**Correction to that baseline, found here.** §7 states 41,374 bytes / 117
allocations as *the* per-evaluation leak. That holds only for small robots.
`walker` leaks **35,802,566 bytes in 630,138 allocations** — 866× — and it is
pre-existing, not from this change: byte-for-byte identical on the commit
before. The leak scales with links and simulated frames, which §10's "one
simulation object per fitness evaluation" does not convey.

### D5 — the dictionary type is gone

All eight users are plain Qt 6 now. `SIG_Robot`'s six were D4; this step did the
other two, both of which needed the key carried explicitly because the value has
no name of its own:

| | |
|---|---|
| `SIG_Link::points` | `QList<NamedPoint>`, `struct NamedPoint { QString name; DL_vector *value; }` |
| `SIG_LanguageParameters::allowedCommands` | `QList<NamedCommand>`, same shape |

`allowedCommands` is another order-carrying dictionary — its order rides inside
every `.exp` and every PVM transfer through `writeToFileTransfer`. **The total
is ten; V1 has the count, measured against the binary.**

Both lookups scan **backwards**, because Qt 2's `QDict` returned the newest
binding for a duplicate key and `removeCommand` has to free that same one. This
is the semantic that broke in D3 and was caught by the shim's self-check.

`Q2Dict` and `Q2DictIterator` are deleted from `q2compat.h`, and the 15 test
blocks that exercised them are deleted from the self-check, which still passes.

### D6 — `Q2Array` is gone, all 180 sites are plain `QList`

It was a value array over `QList` the whole time, so 40 files renamed and only
**two** compile errors came out — both the one real divergence: Qt 2 handed out
a **writable** `T &` from a *const* array, so `MT_Classifier.cpp:617,648` assign
through `at()`. `QList::at()` is const; those two are now `(*p)[i]`.

Two behaviours went with the type:

| | |
|---|---|
| `sort()` was numeric, per D13 | `std::sort` at `SIG_GPManager.cpp:304,311`, the only two callers |
| `at()` **clamped** an out-of-range index instead of failing | dropped |

Dropping the clamp is safe here and was checked rather than assumed. Both
defects §9 lists it as masking are already fixed —
`SIG_ProgramLine.cpp:215` drops the out-of-range write, and
`SIG_DynaSystem.cpp:266` deleted the right vector before that whole file went
with the Dynamo backend on 2026-08-28 (`physics_backends.md`). And the clamp **fired in none of the 42 evaluations**;
`Q2Array::at: index … out of range` appears nowhere in their output.

The residual risk is stated plainly: an out-of-range index is now undefined
rather than silently wrong, which AddressSanitizer catches in `build/` but a
release build would not. Nothing in the evaluation path reaches it. The
evolution loop cannot be exercised — as written, "until PVM builds"; **still
true after Phase P**, because the loop spawns `sigel_slave`, which Phase C
blocks.

### What review found in D4–D6, and what it changed

**A silent semantic flip, now fixed.** `Q2Dict::find()` returned the **newest**
binding for a duplicate key. D4's six replacements —
`SIG_Robot::lookupBody/Material/Link/Joint/Drive/Sensor` — scanned forward and
returned the **first**, and the D5 commit claimed all lookups scanned backwards
when only the two in `SIG_LanguageParameters` did. All eight now scan backwards.
No shipped robot has a duplicate name (0 across all 438 name-groups), so nothing
in the data could have caught it.

**Which is exactly why there is now a check that can.** `sigel_eval -selfcheck`
asserts newest-wins for **all eight** lookups — `SIG_LanguageParameters`,
`SIG_Link::points` and each of `SIG_Robot`'s six — and that `removeCommand`
frees the newest. `fitness-check.sh` runs it before the evaluations.

**Verified to have teeth, one lookup at a time.** Flipping any single one back
to first-wins makes the self-check fail, while `dictorder-baseline.txt` and
`fitness-baseline.txt` both stay empty. The first version of this check asserted
only `lookupLink`, leaving five of the six able to flip silently — the same hole
it was written to close, found by review.

**Two real out-of-range accesses, previously absorbed by the clamp `Q2Array`
provided and now fixed.** Both are off the 42-evaluation path, so no gate saw
them:

| | |
|---|---|
| `MT_FitnessTrainer.cpp:88` | `loadSetup` sized `Result`/`ResultIst` from the stale member `TSetSize` while giving the training set the file's `NewTSetSize`. Any setup file with a larger set made `calculateFitness` **write past both arrays**. `setSelektionValue` in the same file always did it correctly |
| `MT_Substitute.cpp:64` | `changeErrorInfo` looped to `CorrectFitness.size()`, a high-water mark that only grows, while indexing the caller's arrays — which shrink whenever the selection size is lowered. Now bounded by the smallest of the three |

**`allowedCommands` is in the gate now**, dumped alongside the others — 469 lines added to the baseline. Nothing *numbers*
commands (they are looked up by name), so like bodies and materials its order
reaches only the serialised bytes.

**`check.sh` was reporting a failure the real build does not have.** Its include
path stopped at `QtCore` while the Makefile adds `QtGui` and `QtWidgets`, so
`SIG_GPPopulation.cpp` failed on `<QApplication>`. Fixed; the 4 remaining
failures are exactly the files the Makefile excludes. *The pass count quoted
here was 118, which the Dynamo deletion made stale — see §7 for the current
figure, and do not quote a count from a step write-up.*

**Coverage lost with the deleted types, restored.** The block asserting that
`clear()` on a *non-owning* container frees nothing was shared with the `Q2Dict`
tests and went out with them, although `Q2PtrList` and `Q2PtrVector` still rely
on it — `SIG_Robot::clear()` depends on the property. Re-added for both.

**Pre-existing, not ours, recorded:** the `.rrb` load path takes a
**heap-buffer-overflow inside vendored cv97** — `JString::regionMatches`, from
`SceneGraph::SceneGraph()` via `SIG_Body::load()`. It aborts under
AddressSanitizer, so `.rrb` loading has no sanitized coverage. Vendored code,
out of scope for the Qt port.

**But the fitness gate does run sanitized, and an earlier draft here said no
gate could.** Measured:

```
ASAN_OPTIONS=detect_leaks=0 ./fitness-check.sh build
```

exits 0, runs the self-check, and reproduces `fitness-baseline.txt` byte for
byte with no ASan or UBSan report. So all 42 evaluations plus the duplicate-key
self-check have full sanitized coverage today. Only `dictorder-dump.sh` is
blocked, and only because it also loads the 7 `.rrb`.

### D7 — `Q2PtrVector`: `SIG_Geometry` and `SIG_Body` only

**The commit subject for this step overstated it.** `Q2PtrVector` is *not* off
the simulation path. Still on it, and executing on every fitness evaluation:
`SIG_DynaMechsSimulationData.h:88,92,96` — `dynaMechsLinks`, `drives`,
`sensors`, together with the `DynaMechsLinkGuard` whose whole purpose is
`deleteContents()` on the unwinding path, which is the most ownership-sensitive
one in executed code — and `SIG_Interpreter.h:125`'s `registers`, indexed on
every interpreted instruction. `SIG_SimulationQueries.h:67` and
`SIG_CommandInterface.h:66` are pure-virtual signatures taking
`Q2PtrVector<SIG_Register> &`, three implementations each, so that one has to
flip in a single commit. `SIG_DynaSystem.h:193-202` had four more on the Dynamo
backend — **gone 2026-08-28 with that backend, which is the whole of
`Q2PtrVector` 57 → 53** — and `SIG_GPOperations.h:73` and
`SIG_GPTournament.h:81` mean the four evolution-loop users named in the commit
message are not the whole remainder either.

`Q2PtrVector` splits in two. The **simulation side** is covered by both gates
and by AddressSanitizer; the **evolution loop** — `SIG_GPPopulation`,
`SIG_GPFitnessTrainer`, `SIG_GPManager`, `MT_Classifier` — is the 8 sites §9
lists where `insert()` or a shrinking `resize()` *is* the only free, and nothing
can run it — as written, "until PVM builds"; **still true after Phase P**, the
blocker being Phase C. This step does the covered half only.

| | |
|---|---|
| `SIG_Geometry::polygons`, `::vertices` | `QList<T *>`. They were grow-by-doubling append buffers, where `size()` was capacity and `count()` was fill; `QList::append` is that natively, so the doubling dance is deleted outright |
| `SIG_Body`'s local `vertices` | **`QList<DL_vector>` — values, not pointers.** There is no polymorphism, so the ownership question disappears rather than moving |

**A latent null dereference, removed rather than preserved.**
`SIG_DynaMechsLink.cpp:118` iterates `getVertices()` to **`size()`** — the
capacity — and dereferences every slot, so any unfilled capacity would have
crashed it. With `QList` the two are the same number by construction. The same
line also took the whole vector **by value** on every link construction; it is a
const reference now.

**The appending method is `getOrAddVertex`, it is public, and it *is* called
from outside**
`SIG_Geometry` (`SIG_Polygon.cpp:66`, reached from `SIG_Body.cpp` and
`SIG_RobotCompilerObjects.cpp:374`). And it did **not** fill exactly: it grew
capacity to `max(16, 2·size)`, so `size() > count()` from the first vertex on —
measured at **289 of 298 calls** across the 7 `.rrb`.

The real reason the link site was safe is narrower: `SIG_Link.cpp:196`
deep-copies the body's geometry through `SIG_Geometry(const SIG_Geometry *)`,
which sized to `count()` and so **compacted the holes away**. The simulation only
ever sees compacted copies — 87 observations across all 14 experiments, all
`size() == count()`, and `getOrAddVertex` called 0 times on any `.exp`.

**`SIG_Body`'s local is *a* container §7 said should keep `setAutoDelete`** —
not *the* one, which was `fitTaskList` — **converted in D25b**, which replaced
the flag with an RAII guard rather than dropping the ownership. On a local the flag *is* the RAII, because the NEWMAT multiply and
the `SIG_Polygon` allocations below it can throw. That reasoning was right for
pointers; values retire it, since nothing is owned.

**The bounds check first added here made things worse, and is fixed.**
`SIG_Polygon` self-registers with the geometry in its constructor, so creating
one and then skipping every out-of-range vertex left a **0-vertex face** behind —
which the Qt 2 clamp could never produce, because it appended vertex 0 instead.
`SIG_Mirtich::compFaceNormal` reads `verts[0..2]` unconditionally out of a
`new int[numVerts]`. The check now runs **before** the polygon is created, and
`compFaceNormal` refuses a face with fewer than three vertices and a zero-length
normal rather than dividing by it. Inert on shipped data — the branch fires 0
times across all 7 `.rrb` — but the failure mode had moved the wrong way for a
step whose point was removing a latent dereference.

**`SIG_Geometry`'s copy constructor and assignment are now `= delete`.** Both
lists own raw pointers that the destructor `qDeleteAll`s, so a generated copy
would free twice. Not a regression — `Q2PtrVector`'s copy set `del = false` but
`~SIG_Geometry` called `deleteContents()` unconditionally, so the hazard was
identical — and nothing in the tree copies one by value.

**The geometry hunks were covered by nothing, and now are.** Review injected a
doubling of every VRML vertex and both baselines stayed byte-identical: the
`.exp` path never runs the VRML reader, and the dump recorded only names and
numbers. `dumpOrder` now emits a per-body vertex count, polygon count and
coordinate checksum. Re-run with the same injection, the order gate **fires**.

Verified: both gates clean, `./check.sh` 118 pass / 4 fail / 338 warnings, all
14 experiments clean under AddressSanitizer and UndefinedBehaviorSanitizer, and
the leak total **unchanged at 41,374 bytes in 117 allocations** for `twoBases`,
35,802,566 in 630,138 for `walker` — no free dropped, none doubled.

**Formats, both pure permutations.** `.rrb` is block-structured with exactly five
top-level kinds — `material`, `link`, `joint <subtype>`, `drive`, `sensor`, all
`<kind> [<subtype>] <name> { … }`, with `point` lines nested inside links and no
stray top-level lines in any of the 7 files. The `.exp` carries the robot as a
line-oriented `StreamedRobot` block: `Body`, `Material`, `Link`, `…Joint`,
`Drive`, `…Sensor`, one entity per line except joints and sensors, which take
two. So the tool reorders whole units and invents nothing, and the check with
teeth is that the multiset of lines in equals the multiset of lines out.

**The order is genuine hash order, not something simpler.** Checked before
trusting the baseline: **11 of the 28 link/joint groups are not in ascending
name order** — `octopus` links start `thirdFootLink, firstFootLink, base`, and
`walker` joints start `leg6Joint1, leg5Joint2, leg4Joint3`. So the migration
cannot be replaced by sorting; the order has to be written into the `.rrb` and
`.exp` files, which is what §10 step 2 says.

What the shim currently carries, and why:

| | why it exists |
|---|---|
| Qt 2's hash order in `Q2Dict` | iteration order numbers the links, so the shipped experiments depend on it |
| `size()` ≠ `count()` on `Q2PtrVector` | allocated vs occupied slots, with null holes |
| the internal cursor on `Q2PtrList` | `first()`/`next()` walks are real state |
| `insert()` deletes the occupant, shrinking `resize()` deletes the tail | the only free path at 8 sites |
| clamp-on-out-of-range | matches `QGArray::at`, and hides several real defects |

The clean-up, in this order:

1. ~~Replace the emulation with straightforward containers.~~ **In progress —
   D3–D12 done.** `Q2Dict`, `Q2DictIterator` and `Q2Array` are deleted; their
   users are plain `QList`. Left, measured 2026-08-29 after D12: `Q2PtrList` 47,
   `Q2PtrVector` 49, `Q2CString` 19, `Q2Queue` 16, `Q2ListIterator` 8,
   `Q2ValueList` 12. *Every count in this list has to name the commit it was
   taken at: comparing one to a count taken a day later, after two steps had
   legitimately removed sites, once overstated an error eightfold.*
2. ~~**Migrate the data files at the same time.**~~ **Done, D2.** Only the 7
   `.rrb` needed it — the 14 `.exp` already stored the order the simulation
   used. `Q2Dict::hash` generated that ordering and is deleted.
3. Re-verify against the captured 2003 run. **Not done**, and it still needs
   the x86 box's numbers (§7).

Sequenced this way the ordering stopped being a hidden property of a hash
function and became visible in the data.
### D8 — the `SIG_Register` cluster, and a leak that had no free path at all

`Q2PtrVector<SIG_Register>` crossed a module boundary — a member on
`SIG_Interpreter` plus two pure-virtual signatures on `SIG_SimulationQueries`
and `SIG_CommandInterface`, four implementations between them — so it had to
flip in one commit. 12 files.

**`SIG_Register` is two ints**, no destructor, no pointers. So it becomes
`QList<SIG_Register>` **by value**, the same move as `SIG_Body`'s local in D7:
the ownership question disappears rather than moving. It has no default
constructor, so the register file is built with `append`, not `resize`.

**That fixed a leak with no free path anywhere.** `SIG_Interpreter` has no
destructor, never called `setAutoDelete` and never called `deleteContents`, so
the `Q2PtrVector` default of `del = false` meant **every interpreter leaked its
entire register file** — one interpreter per fitness evaluation, for the life of
the project. Measured, and the accounting is exact:

| | before | after |
|---|---|---|
| `twoBases` | 41,374 B / 117 allocs | **41,254 B / 109** |
| `walker` | 35,802,566 B / 630,138 | **35,802,446 B / 630,130** |

Both lose exactly **8 allocations**, and both robots declare `memSize 8` in
their `LanguageParameters` line — one allocation per register, gone. The write
order is `bitsPerRegister memSize maximalDelayTime`, and `twoBases` has
`bitsPerRegister` 3 against `walker`'s 8 while both lose 8, which rules out the
8 being register width.

**The byte figure is not 8 registers.** From the leak records: of the 120 bytes, only **64** are a
leak removed — the 8 × `new SIG_Register`. The other 56 are the same memory
still leaking, smaller: the container's heap buffer went 128 B → 80 B because
`resize` took Qt's growth policy to 14 slots where `reserve(8)` allocates
exactly, and the `SIG_Interpreter` object went 88 B → 80 B because the member
shrank from 32 to 24 bytes.

**So the register file still leaks** — as one 80-byte buffer instead of nine
allocations — because `new SIG_Interpreter` is never deleted. That is §10's
pre-existing leak and is out of scope. What D8 fixed is the register *objects*,
which had no free path of their own; the *file* still rides the larger leak.

**The clamp went too, and D6's precedent says to say so.** `Q2PtrVector`'s
`at()` warned and clamped an out-of-range index — or a negative one, via the
`uint` wrap — to element 0 and returned a valid pointer. `QList::operator[]`
asserts in these builds and, under `-DQT_NO_DEBUG`, reads out of bounds
silently. Checked the way D6 was: **`Q2PtrVector::at` warns in none of the 42
evaluations**. At-risk sites, all latent: `registers[1]` in both DynaMo files,
which needs `memSize` ≥ 2, and `registers[reg]` where
`reg = getInstructionElement(n) % numberOfRegisters`, which is negative if any
program element is — the header documents elements as 0..MAXINT.

Fitness identical on all 42, both gates clean, `./check.sh` 118 pass / 4 fail.

**The seven headers now include `<QList>` directly.** They had been getting it
from `compat/q2compat.h`, and after this step none of them uses a `Q2*` type at
all — so the commit would have left an implicit dependency on the very header
Phase D exists to delete.

**Leak byte totals are sensitive to the repo path length**, because a copy of
`$SIGEL_ROOT` is among the leaked blocks. The figures above reproduce exactly at
a 25-character repo root; at a longer path they shift by the difference. The
allocation *counts* are stable.

### D9 — `Q2PtrVector` off the executed path, and the unwinding guard with it

D7 did the covered half of `Q2PtrVector` for `SIG_Geometry` and `SIG_Body`, and
its own text listed what it left behind on the simulation path:
`SIG_DynaMechsSimulationData`'s `dynaMechsLinks`, `drives` and `sensors`. This
step does those three, which are the last `Q2PtrVector` in `SIGEL_Simulation`.
Of the 24 lines `grep` matches at HEAD, 7 are prose and one
(`MT_Program.h:86`) is commented out, leaving 16 live sites; one of those, `SIG_GUIGPManager.h:40`, is
a GUI class Phase C owns.

**THIS STEP CLAIMED `Q2PtrVector` WAS NOW OUT OF EVERY PATH THE GATES RUN. THAT
WAS FALSE, TWICE OVER**, and D10 repeated it in the status table before review
caught it. `q2compat_check.cpp` exercises `Q2PtrVector` and `check.sh` builds
and *runs* it under ASan and UBSan. Worse, `SIG_GPPopulation::pool`
(`SIG_GPPopulation.h:55`) is a `Q2PtrVector` with `setAutoDelete(true)`, and
`SIG_GPExperiment::loadExperiment` calls `population.readFromFile`
(`SIG_GPExperiment.cpp:99`), whose `pool.insert` runs for every individual —
**on every `.exp` load, in both gates.** Measured, not argued: see "What the
gates actually reach" below. The step itself stands; the boast did not.

All three are **slot-indexed with null holes**, sized once from the robot and
never resized, so `QList<T *>` sized by `QList(qsizetype)` and filled with
`fill(0)` is the same container. `insert(i, p)` becomes `v[i] = p` and
`take(i)` disappears into the `delete` that already preceded it.

| | |
|---|---|
| `drives`, `sensors` | non-owning. `Q2PtrVector`'s `del` was never set, so `insert()` over an occupied slot never freed and the assignment is exact |
| `dynaMechsLinks` | owning, and its three free paths were **already explicit** before this step — `delete` before each of the two `insert`s, `deleteContents()` in the destructor, and `DynaMechsLinkGuard` on the unwinding path. So this step moved no ownership; it only changed the spelling |

**The guard is now the only free while unwinding, and that is a widening.**
`~Q2PtrVector` freed nothing either — `del` was false — so the guard was already
load-bearing. But `Q2PtrVector` at least *had* a flag someone could set;
`~QList` can never free, so the guard has no fallback of any kind. Its comment
says so now.

**And this step could not test the one line it most needed to.** An earlier
draft here said the guard "was verified reachable in Phase B and is not
redundant". That is true only of `sigel_slave`, which Phase C blocks. In
`sigel_eval` — the only binary that exists, and the whole of both gates — the
throw has **nowhere to land**: `evalFitness()` is called with no `try`
(`sigel_eval.cpp:303`), and inside it the `new SIG_Simulation` sits *before* the
`try`, which wraps only `simulation->start()`. `sigel_eval.cpp:309` is the same
shape. So `SIG_CannotMirtich` from the constructor reaches `std::terminate` with
no handler, and this toolchain does not unwind in that case — the guard's
destructor never runs. It is a converted free path with **zero coverage**, kept
because `sigel_slave` does wrap the call (`sigel_slave.cpp:253-266`) and will
need it. See the list below.

**The three `insert(i, 0)` null-fill loops in the constructor were already
no-ops** — `Q2PtrVector(uint)` null-filled on construction, as `QList(qsizetype)`
value-initialises now. They are `fill( 0 )`, one line each, matching the
`jointIndices.fill( 0 )` two lines below rather than relying on the subtlety.

**The clamp went, as in D6 and D8, and the gates cover it here.**
`Q2PtrVector::at()` warned and clamped an out-of-range index to element 0, and
`insert()`/`take()` were silent no-ops out of range; `QList::operator[]` is
neither. The indices are stored link, drive and sensor numbers read from the
data files, so a file could in principle carry one past the end. All 14 `.exp`
run under AddressSanitizer in `fitness-check.sh` and all 21 files load in
`dictorder-dump.sh`: **0 sanitizer reports, both gates byte-identical.** No
bounds check was added — D7's review is the precedent for not moving a failure
mode sideways on a step that has coverage.

**And that coverage is sharper than it looks.** Neither the `Makefile` nor
`check.sh` defines `QT_NO_DEBUG` or `NDEBUG`, so `QList::operator[]`'s
`Q_ASSERT_X` is **live** in both `build/` and `build-fast/` — established by
review, confirmed to abort on a negative index. An out-of-range index anywhere
in the covered runs would have aborted rather than been absorbed. The residual
risk is a release build alone, which is exactly the case `q2compat.h:24-26`
says the clamp existed for.

`SIG_DynaMechsSimulationData.h` now includes `<QList>` directly instead of
`compat/q2compat.h`. It uses no `Q2*` type after this step, and it already held
two `QList` members that were reaching the header through the shim.

Verified: `./check.sh` 105 pass / 4 fail, **317 warnings, down from 322** — the
5 are `-Wsign-compare` in `SIGEL_Simulation`, from `int i < size()` against
`Q2PtrVector`'s `uint size()`. Both gates empty, and
`ASAN_OPTIONS=detect_leaks=0 ./fitness-check.sh build` reproduces
`fitness-baseline.txt` with no ASan or UBSan report.


### D10 — `SIG_Link`'s two lists, and a `do`/`while` that must run on empty

`adjacentJoints` and `noCollide` become `QList<T *>`, with `getJoints()` and
`getNoCollides()`. Both accessors return **by value**, as they did in 2003;
Qt 6's `QList` is implicitly shared, so a caller still gets its own list and
the copy costs a refcount. Neither list ever owned anything — no
`setAutoDelete` on either, and `~SIG_Robot`'s `qDeleteAll` frees the joints and
the links — so no ownership moved.

The return types drag four caller sites into the same commit:
`SIG_Robot.cpp:292`, `SIG_Joint.cpp:755` and
`SIG_DynaMechsSimulationData.cpp:370,520`, all `first()`/`next()` walks over
their own copy, all now range-for. `getNoCollides()` has **no caller anywhere
in the tree**, GUI included; it is ported rather than deleted, per D21.

**The one site that is not mechanical, and it is load-bearing on every robot.**
`SIG_Link::transformToDynaMechs` builds a local `successors` list and walks it
with a `do`/`while`:

```cpp
SIG_Joint *actSuccessor = successors.first();      // null when empty
do { … if (actSuccessor) { …; actSuccessor = successors.next(); } }
while (actSuccessor);
```

On an **empty** list the body still runs **once**, with `actSuccessor` null —
the body has an explicit "without successor" branch, and `transformX` is false
there. A range-for rewrite drops that pass silently. It is now an index with
`QList::value()`, which yields null past the end exactly as `first()`/`next()`
did, and the reason sits in a comment above it.

**Verified to have teeth, not argued.** Replacing the `do`/`while` with a plain
`while` — precisely what a careless rewrite produces — makes **every** robot
abort: `terminate called after throwing an instance of
'SIGEL_Robot::SIG_CannotMirtich'`, `sigel_eval exited 134`, and both gates go
from empty to reporting every block missing. The empty-successors pass is what
transforms every leaf link; it is not an edge case.

**D9's lesson applied before building, not after.** D9 was caught out by
`int % qsizetype`, which `gcc` does not warn about, so this step grepped every
`count()` and `size()` on these two lists rather than trusting the warning
delta. There is exactly one, `noCollide.count()` streamed at
`SIG_Link.cpp:326`. `qsizetype` is `long long` here and `QTextStream` has a
`qlonglong` overload, so no conversion happens at all — confirmed by compiling
the file with `-Wconversion -Wsign-conversion`, which says nothing about that
line.

**But no gate sees it.** Measured: **no shipped robot declares
`nocollide`** — 0 matches across all 7 `.rrb`, and every `Link` record in all 14
`.exp` carries `noCollideCount = 0`. So `noCollide` is empty on every gate run.
The count only ever streams `0`, the loop below it never iterates, and
`addNoCollide` with its `!contains` is **executed by no gate at all**. The
conversion is right by inspection; it is not right by test.

`containsRef(link) == 0` became `!contains(link)` — Qt 2's default
`compareItems` is pointer identity, which is what `QList::contains` does on a
pointer, and the call was only ever tested against zero.

`SIG_Link.h` now includes `<QList>` rather than `compat/q2compat.h`.

Verified: `./check.sh` 105 pass / 4 fail, **315 warnings, down from 317**; both
gates byte-identical; sanitized fitness run clean.

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
| `SIG_GPParameter::writeToFile`'s `PVMHOST` loop, D13 | linked, never called — no gate saves an `.exp` |
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
| `MT_GUI/MT_StatisticsWidget.cpp`'s nine `.count()` calls on the converted member, D24 | `MT_GUI` is in neither `check.sh`'s `MODULES` nor the Makefile's `CORE`. They still compile — `QList::count()` exists — but no gate says so |
| **the whole `taskCanDoList` index walk, D25a** | compiled and archived (21 `SIG_GPManager::` symbols in `libSIGEL_GP.a`) but **linked into nothing** — 0 in `sigel_eval`, `build-fast/sigel_eval` and `pvm_link`. The `removeAt` path, the append path and the `:122` reference never execute here. The 1.3 binary shows the path is live in a real run (80 appends in two generations); our gates cannot reach it |
| **four of D25a's six `at(canDoIdx)` sites** | `:127`, `:151`, `:1310`, `:1336` sit inside `#ifdef SIG_DEBUG`, and **`SIG_DEBUG` is defined nowhere in this build** — its only occurrence in the repo is `x/sigelSourceDistribution.1.0/sigel/makefile:72`, the abandoned 1.0 tree. Neither `make` nor `check.sh` parses them. Compiled explicitly with `-DSIG_DEBUG` by review: exit 0, no errors — so no latent defect, but only two of the six were checked by the build |
| the `maxTouchsPerLoop` break, D25a | **dead on every shipped configuration**: the value is persisted in none of the 28 `.exp`, and all five presets call `setMaxTouchsPerLoop(-1)` (`SIG_GPParameter.cpp:325,330,335,340,345`), so `(maxTouchsPerLoop != -1)` is always false |
| all **seven** D23 sites | linked and never called — *in `sigel_eval`, which is a test harness, not the program*. 1.3's master links the whole MT subsystem (`MT_Evaluator` 16 symbols, `MT_Classifier` 25, `MT_Statistics` 83, `MT_Substitute` 20, `MT_TrainingCase` 46, `MT_Trainingset` 14; all 0 in `sigel_slave`), so "never linked" is a fact about our harness and Phase C will link these. Detail: `MT_Substitute`, `MT_Trainingset` and `MT_FitnessTrainer` **are** in `sigel_eval` and `pvm_link` — 59 symbols, pulled in by `moc/MT_GPSystem/MT_GPManager.o` on the link line — but `gdb` breakpoints on all three converted functions and on `MT_GPManager::checkForNewTCase` were not hit across a full evaluation. The classes that never link are **`MT_Classifier` and `MT_Evaluator`**, the queue's two fillers, both evolution-loop. *This row previously said the first three were the unlinked ones: inverted, and asserted without running the `nm` the D22 row had already established for exactly this* |
| **all six D22 sites** | `nm -C build/sigel_eval` finds **0** `SIG_GPOperations::` and **0** `SIG_GPCrossOverTournament::`, and the same in `pvm_link`. The objects are archived in `libSIGEL_GP.a` and never pulled into a link. The evolution loop needs `sigel`, which Phase C blocks |
| all six `Q2CString` sites in `SIG_GPFitnessTrainer`, D21 | zero trainer symbols in `sigel_eval`; `pvm_link` links the object but never constructs a trainer, so they are **link-checked and never run** |
| `SIG_GPPVMData`'s `+ 2`, D21 | `pvm_link` runs the function, but `pvm-check.sh` passes with `+ 1` **and** `+ 0` — `QList` over-allocation hides a shortfall under about 8 bytes |
| all five **unlinked** fitness functions' walks, D20 — `Adaptive`, `Zorc`, `Stepper`, `RealSpeed`, `Force` — plus `SIG_EarlyRunTermSimulation` | `nm` finds 0 symbols for each in `sigel_eval`. `Stepper` is the **only reader of `touchdowns`** in the tree; `Force` the only reader of `listForces` and the only code that ever frees a force vector |
| `sigel_eval`'s trace walk, D20 | runs on all 21 dictorder inputs; its output is dropped by the gate's `sed`, so only a crash or a sanitizer report would show |
| `SIG_GPNiceWalkingFitnessFunction`'s walk, D20 | runs for 18 individuals, but the gate has **one bit** of discrimination — an off-by-one in the index is invisible to it |
| **everything D17, D18 and D19 changed** in `SIG_GPFitnessTrainer` — including all six `delete v[i]`, `resizeOwningHosts`, both `qDeleteAll` in the destructor and both `static_cast<uint>` moduli | `nm -C build/sigel_eval \| grep -c SIG_GPFitnessTrainer` is **0**. `pvm_link` links the object but never constructs a trainer, so it is link-checked and never run. The rewritten walk needs a live `pvm_spawn`; `flushAllDynHosts` is `-devolve`-only; the destructor's three `qDeleteAll` run for no gate |

### D11 — the last three lists on the executed path, and a check that can see them

`SIG_Material::friction`, `SIG_Body::usedByLinks` and
`SIG_DynaMechsLink::successors` become `QList<T *>`. `Q2ListIterator` is gone
from every module the gates run.

| container | ownership | shape |
|---|---|---|
| `SIG_Material::friction` | **owning** — `~SIG_Material` already freed it explicitly, so `deleteContents()` became `qDeleteAll` + `clear()`. No leak to fix and none introduced | 3 iterator walks → range-for |
| `SIG_Body::usedByLinks` | non-owning | its `setAutoDelete(false)` was a **no-op** — `Q2PtrList` and Qt 2's `QList` both default to false — and `QList` has no such method, so the line goes |
| `SIG_DynaMechsLink::successors` | non-owning; the links belong to `dynaMechsLinks`. No destructor, no flag | one `first()`/`next()` walk in `forwardKinematics` → range-for |

**`friction` stays a list of pointers.** D8 turned `SIG_Register` into values and
that fixed a real leak; here there is none, so a value type would move ownership
on a path nothing can test, for tidiness. Recorded in `future_refactorings.md`
instead. `usedByLinks` is **converted, not deleted**, though nothing in the tree
reads it — same reasoning, same place.

**Two of the three are exercised by no gate, so a check was written that can
see them.** `sigel_eval -selfcheck` now covers `SIG_Material::friction` and —
retrospectively — D10's `SIG_Link::noCollide`, using only public API:
the not-found default of 0.6, `negotiate` setting the reverse pair, an update
that must **not** append, and `addNoCollide`'s duplicate guard. It runs inside
`fitness-check.sh`, ahead of the evaluations.

**Verified to have teeth, both paths, the way this file requires.** Dropping
`setFrictionValue`'s "already present" test — what a careless rewrite of the
iterator walk produces — and dropping `addNoCollide`'s `!contains` guard makes
the self-check fail with 3 assertions and exit 1.

**Two holes in that check, both found by review and both closed.**

- It asserted `getNoCollides().at(0)`. `SIG_WANT` records and continues, so a
  regression that stopped `addNoCollide` appending reached `at(0)` on an **empty
  list** and the block died with `ASSERT failure in QList::at` and a core dump
  instead of reporting. Worse, it is the exact shape `q2compat.h:24-30` says not
  to leave to `Q_ASSERT`, which vanishes under `QT_NO_DEBUG`. Now `value(0)`,
  and with the guard broken the block reports **6** failures cleanly where it
  previously aborted after 2.
- **Nothing asserted that `~SIG_Material` still frees.** The self-check
  exercises the free but had no way to see it, and the prescribed sanitized gate
  runs `detect_leaks=0`, so a future rewrite that dropped the `qDeleteAll` would
  have passed `check.sh`, both baselines, the sanitized run *and* the
  self-check. `fitness-check.sh` now runs the self-check a **second** time under
  `detect_leaks=1` — it can afford to, because unlike the evaluations it frees
  everything it allocates. Broken deliberately: dropping the `qDeleteAll` gives
  `80 byte(s) leaked in 5 allocation(s)`, every frame named to
  `SIG_Material::setFrictionValue`, and exit 1, while the plain run still says
  `ok`.

**And one of those assertions is the only one that catches it.** With the
append bug in place, `getFrictionValue` still returns the *right value*: the
in-place update happens and the stray duplicate is appended after it, so a
forward scan finds the correct entry first. Only the serialised `nfric` count
moves, from 2 to 3. A self-check that asserted values alone would have passed
on broken code — which is the ninth time this project has hit that shape, and
the reason the count assertion is there.

Three headers now name `<QList>` directly, but only **two** of them dropped
`compat/q2compat.h` for it — `SIG_Body.h` and `SIG_DynaMechsLink.h`.
`SIG_Material.h` never included the shim; it had been getting `Q2PtrList`
transitively through `SIG_Robot.h`.

Verified: `./check.sh` 105 pass / 4 fail / 315 warnings, both gates
byte-identical, sanitized fitness run clean, self-check ok.

### D12 — `Q2CString`, and the executed path is clear

**D11's commit message is false in one sentence and cannot be amended.** It says
"only D12's two `Q2CString` sites are left there", and the status row said the
same until this step rewrote it. Both were wrong: `sigel_eval` constructs a
whole `SIG_GPExperiment`, so `SIG_GPParameter::hostList`,
`SIG_GPExperiment::experimentHistory` and `SIG_GPPopulation::pool` all run on
every gate — a `gdb` backtrace in the D11 review puts `hostList.append` at
`SIG_GPParameter.cpp:615` under `loadExperiment` under `main`. The sentence is
recorded here because git history is not editable and this file is where the
plan is read from.

Two sites, identical, both building the `Terrain.ter` path for DynaMechs:
`SIG_Environment.cpp:413` and `SIG_DynaMechsSimulationData.cpp:300`.

```cpp
const QByteArray b = terrainDataFileName.toUtf8();
char const *p = b.constData();          // was: Q2CString's implicit conversion
```

`Q2CString` existed for one reason — Qt 2's `QCString` converts implicitly to
`const char *` and `QByteArray` does not. Both sites already stored the byte
array in a **named local**, so neither was ever the §9 dangling-temporary trap;
the pointer stays valid for the local's lifetime, before and after.

**The one divergence cannot fire here.** `Q2CString::operator const char *`
returns `nullptr` for a null string, where `QByteArray::constData()` returns a
pointer to an empty string. `terrainDataFileName` is `sigelRootString +
"/Terrain.ter"`, so it is never empty and `toUtf8()` is never null. Both gates
load terrain on every evaluation, so this is covered rather than argued.

Both files drop `compat/q2compat.h` for `<QByteArray>`.

**With this, no shim TYPE is used in `SIGEL_Robot`, `SIGEL_Simulation` or
`SIGEL_Environment`.** Eight prose comments naming the old types are all that
`grep` finds there, and they are kept because they explain why the code reads as
it does.

*A `grep` for shim **types** says nothing about shim **includes**.* Six live
`#include "compat/q2compat.h"` remained after the type sweep came back clean —
`SIG_Geometry.h`, `SIG_LanguageParameters.h`, `SIG_Robot.h`,
`SIG_DynaMechsCommandInterface.cpp`, `SIG_DynaMechsLink.cpp` and
`SIG_DynaMechsSimulationQueries.cpp` — every one of them reaching `QList`
through the shim rather than any `Q2*` type. Found by the D12 review, which also
established all six were removable with an identical 105/4. They now include
`<QList>` directly. The comment count was also wrong: eight, not six. **The shim cannot be deleted yet**: `SIGEL_GP`, `MT_Control` and
`MT_GPSystem` still hold it, and, per "What the gates actually reach" above,
part of that is executed on every run rather than being unreachable evolution
loop as this plan long assumed.

Verified: `./check.sh` 105 pass / 4 fail / 315 warnings, both gates
byte-identical, sanitized fitness run clean, self-check ok.

### D13 — `hostList`, and the first conversion checked before it was made

`SIG_GPParameter::hostList` becomes `QList<SIG_GPPVMHost *>`. First container
in the evolution loop, and the first converted against a 1.3 reference captured
**before** the work: V8 measured `PVMHOST` order stable at 20 of 20 over three
round trips, which is what the write path has to preserve.

It owns its hosts, so both `deleteContents()` become `qDeleteAll` + `clear()`.
Three cursor walks become range-for — one in `writeToFile`, two in
`SIG_GPFitnessTrainer` that the accessor return type drags in. Precisely: the
trainer's `.first()` would have compiled, its `.next()` would not.

**A trap preserved rather than tidied.** This reads wrong and is not:

```cpp
while (actHost) {
  if (actHost->enabled)
    noOfActiveHosts++;
    actHost = ...next();
  };
```

The assignment is **not** inside the `if` — no braces, one governed statement —
so the loop advanced correctly. Verified against the pristine tarball bytes,
not inferred from the layout. `gcc` was flagging it: the 315 → 314 warning drop
is that `-Wmisleading-indentation`.

`getHostList()` now returns `QList<SIG_GPPVMHost *>&`, which affects 7 call
sites in `SIGEL_MasterGUI`. None was *broken* by this — that module did not
compile before it either — and D4 set the precedent that they meet the plain
accessor.

**No gate executes any of the three walks** — see "What the gates actually
reach". Coverage is `check.sh`'s syntax check plus a throwaway probe that
round-tripped four experiments and confirmed `PVMHOST` order matches V8's
recorded 1.3 order exactly. **Nothing committed checks it.**

**`pvm-check.sh` was not run for this step and should have been** — §7 says to
run it after touching `SIG_GPFitnessTrainer`. Run afterwards by review: both
halves PASS, so nothing was hidden, but the step's verification was incomplete.

### D14 — `experimentHistory`, in both variants of the class

`SIG_GPExperiment::experimentHistory` becomes
`QList<SIG_GPExperimentHistoryEntry *>`. Changed in **both** files and **both**
headers, per the rule above: `SIG_GPExperiment.cpp` for `sigel` and
`SIG_GPExperimentClean.cpp` for `sigel_slave`.

*This step claimed their history code was "byte-identical, so the edit is the
same in each". Three of the four sites were; **the destructor was not** — the
master's is tab-indented with CRLF endings and also does `delete mtController`.
That is precisely the line the first edit attempt failed on, so the claim was
wrong about the one line that mattered. The edits are right; the reason given
for them was not.*

It owns its entries, so both `deleteContents()` become `qDeleteAll` +
`clear()`. Two cursor walks become range-for — the file writer and the gnuplot
export.

**`first()` was not used**, because `QList::first()` on an empty list is
undefined where `Q2PtrList::first()` returned null, and it compiles either way
(§9). *The trap was theoretical here, and an earlier draft implied otherwise:
the old `first()`/`while` walk already emitted nothing on an empty list, so the
range-for preserves behaviour exactly rather than repairing it. It is still the
right spelling — the next walk of this shape may not be so lucky.*

**`SIG_GPExperiment.cpp:47` has CRLF line endings and a tab indent** where the
rest of the file has LF and spaces. The edit was applied with `newline=''` and
`latin-1` so the line ending survives. This is one of the 46 mixed-encoding
files §2 warns about, and it is why that section says to measure with
`command grep`.

**Confirmed against 1.3 before the change, not after** — `v8-…txt` result 4.
The 160 shipped entries come back byte-identical over three round trips,
generation numbers 1…160 in order, count unchanged. So the container is a
plain appending list.

**And the defect next door does not reach it.** The *per-individual* `HISTORY`
blocks in the same file grow **one whole line** per save — the correction to
V8's original "7 bytes", which understated it: the appended text begins with a
newline, so each block gains a whitespace-only line, +100 lines per pass across
100 individuals. A line-shaped defect is exactly what breaks a `readLine()`
loop, and `readHistoryFromFileTransfer` is such a loop. It is safe here on two
counts: the growth is in a different container, and the one blank line in the
experiment-history section is the last, present in the pristine file, and does
not multiply.

### D15 — `SIG_GPPopulation::pool`, where the container operation *was* the free

The most ownership-sensitive container in the plan. §9 lists eight sites where
`insert()` or a shrinking `resize()` is the only `delete` and the word appears
nowhere; five of them are this one. `pool` becomes
`QList<SIG_GPIndividual *>` and **every one of those frees is written out**.

| was | now | why |
|---|---|---|
| `insert(x, p)` ×**7** | `delete pool[x]; pool[x] = p;` | `Q2PtrVector::insert` freed the occupant first. Where the slot is known null the `delete` is a no-op, so one spelling is exact everywhere. *D15 said six. There are seven — the old file has eight `pool.insert`, one of which is the shift below. An off-by-one in a step whose whole premise is an exhaustive count of hidden frees* |
| `clear()` in `~SIG_GPPopulation` | `qDeleteAll` + `clear()` | `del` was true, so `clear()` freed all |
| `setAutoDelete(true)` ×4 | deleted | the flag *was* the ownership; the frees above are now it |
| `insert(x, take(x+1))` | `delete pool[poolpos]` once, then a plain shift | see below |
| all three shrinking `resize()` | `resizeOwning()`, a four-line static helper | `Q2PtrVector::resize` freed every truncated item. Written once rather than three times, so no shrink is a special case a later reader has to re-derive |

**`deleteIndividual` is the subtle one.** It shifted every later element down
with `pool.insert(x, pool.take(x+1))` and then shrank by one. `take` nulled
each source slot, so the shrink truncated a null and freed nothing; the *first*
`insert` freed the victim, because that slot still held it. Every later
`insert` saw a null and freed nothing. **Exactly one delete, and nothing in the
function says so.** It is now `delete pool[poolpos]` before the loop, a plain
assignment shift, and `removeLast()`.

**`setIndividual` is the 12-call-site one.** `pool.insert(poolpos, &indi)` freed
the tournament loser and then stored a pointer the *caller* allocated. Both
halves preserved; the mixed ownership is 2003's and is not this step's to fix.

**D15 MISSED ONE OF THE FIVE, AND REVIEW CAUGHT IT.** `readFromFile`'s
`wasCanceled()` branch shrinks with `pool.resize( x + 1 )` — §9's `:417`, listed
there precisely because that `resize` was the only `delete`. D15 converted the
other four and asserted the `POPULATIONSIZE` shrink was "the one not provably
null", silently reclassifying this one. It is not null: slots above `x` still
hold the individuals from **before** the load, because the replace loop only
reached `x`. Measured on a pool of 6 cancelled at the first individual —
pre-D15 no leak, post-D15 **4,850 bytes in 45 allocations**, every block traced
to the population constructor.

Reachable from `SIG_Experiment.cpp:505`, `slotPopulationImport`, which is the
one caller that reaches `readFromFile` with a **full** pool. Dead today only
because `sigel_eval` has no `QApplication`, so `if (qApp)` is false; live the
moment Phase C compiles `SIGEL_MasterGUI`. The comment D15 added — "reached
with an empty pool today" — was true only of code that currently compiles.

**The copy hazard `Q2PtrVector` carried is gone, so it is now a compile error.**
Its copy constructor cleared `del` on the copy, as Qt 2's `QCollection` did, so
a copied population freed nothing and could not double-free. A raw `QList` plus
an unconditional `qDeleteAll` removes that. `SIG_GPPopulation`'s copy
constructor and assignment are `= delete`, the same move D7 made for
`SIG_Geometry`. Nothing in the tree copies a population.

**Checked by measurement, not by reading.** The leak total is byte-identical
before and after: **41,254 bytes in 109 allocations** for `twoBases`, the
documented D18 baseline. Verified to have teeth — dropping the destructor's
`qDeleteAll` gives **4,398,620 bytes in 68,853 allocations**, 106× the
baseline.

**But no committed gate would have caught that.** D18 makes the leak figure a
recorded baseline rather than a gate, so a dropped free here fires nothing.
The 42 evaluations pass, both diffs stay empty, and the sanitized run is clean
because it sets `detect_leaks=0`. That policy is deliberate and out of scope
here; it is recorded because this is the step where it bites hardest.

**`SIG_GPPopulation::sort()` has an empty body.** Its header comment says it
sorts the pool by fitness so that the best individual is at position zero. It
does nothing at all. Good for this conversion — the pool is never reordered, so
V8's captured order covers it — but a reader could implement it and silently
renumber every individual, and every stored `poolPos` with them.

### D16 — a check the evolution loop's containers cannot pass by accident

**Built before converting anything else, because D15 proved the gates cannot
see this code.** D15 shipped a real 4,850-byte leak that `check.sh`, both
diffs, the sanitized run, the self-check *and* the leak baseline all reported
clean. Three review rounds have now found real defects by writing a throwaway
probe and deleting it. This commits one.

`sigel_eval -selfcheck` now drives `SIG_GPPopulation` through the functions no
gate enters: `addRandomIndividuals`, `deleteIndividual` at a middle position
and at the last, `setIndividual`, `resetPool`, and a drain to empty.
`fitness-check.sh` already runs the whole self-check a second time under
LeakSanitizer, and **that is what judges it** — the frees here are invisible to
any assertion, so the assertions pin the *shift* and the sanitizer pins the
*ownership*.

**Verified to have teeth — by an analogue, not by the defect D15 shipped.**
That one was `readFromFile`'s `wasCanceled()` shrink, behind `if (qApp)` and
unreachable headless: **reverting all three `resizeOwning` calls still passes
every gate including this one.** Removing
`deleteIndividual`'s `delete pool[poolpos]`:

| | |
|---|---|
| `sigel_eval -selfcheck` | **`selfcheck: ok`** |
| `./fitness-check.sh` and both diffs | **pass, empty** |
| the same self-check under `detect_leaks=1` | **fires**, naming `addRandomIndividuals` as the allocation site |

**Four pre-existing defects found by writing it**, all in `SIG_GPPopulation`
constructors. *D16 named two and said the other two were the ones in use.
**Only the default constructor is used** — an exhaustive grep over the whole
1.3 tree and the 1.0 distribution finds zero calls to any other.*

| constructor | defect | fixed? |
|---|---|---|
| `(int, SIG_Randomizer &r)` | stores `&r` via `setRandomizer`; `~SIG_GPPopulation` **deletes it**. A stack or borrowed randomizer is a bad free — ASan `attempting free on address which was not malloc()-ed`. Found on the test's first run | **no** |
| `(int, SIG_Randomizer&, SIG_GPParameter&, SIG_LanguageParameters&)` | identical, same `setRandomizer(&r)` | **no** |
| `(int)` | **no member initialiser list**, so `randomizer` is uninitialised and the `if (getRandomizerPointer()==0)` guard reads it | **yes**, `: randomizer( 0 )` |
| `(QString)` | same, and `~SIG_GPPopulation` then frees garbage | **yes**, `: randomizer( 0 )` |

The two borrowed-randomizer ones are **not fixed**: repairing them means
choosing an ownership policy, which is a design decision for whoever ports the
GUI that would call them. The two missing initialiser lists **are** fixed —
two tokens, no caller, and it turns undefined behaviour into the deterministic
path the guard was written for.

### D17 — the trainer's two host lists, and what the 1.3 binary says about them

`SIG_GPFitnessTrainer::dynHosts` and `freshDynHosts` become `QList<T *>`. Both
are plain index loops — `count()` and `at(i)`, no cursor — and their **five**
`deleteContents()` (`:107`, `:108`, `:195`, `:200`, `:529`) become
`qDeleteAll` + `clear()`.

*Provenance: pristine 1.3 sets `setAutoDelete` on **both** lists
(`SIG_GPFitnessTrainer.cpp:50-51`); commit `14bc134` removed the flags and made
the frees explicit. A statement true of the tree a step starts from can be false
of the code being ported — say which one you mean.*

**Established from the 1.3 binaries, by symbol table and disassembly**, because
none of these containers reaches a file and nothing here can be diffed:

| | |
|---|---|
| `addDynHost` | present and **referenced**, one call site. Reached only through `SIG_GPManager::RegisterDynPVMClients`, which is a **thread entry point** — its address is pushed to `pthread_create`, so a naive caller search misses it — behind the `-devolve` flag |
| `flushAllDynHosts` | called from both `SIG_GPManager::run` overloads — but **both calls sit behind `if (serverIsUp)`** (`SIG_GPManager.cpp:761`, `:1209`), and `serverIsUp` is `false` at `:47` and set true in exactly one place, `:822`, inside the `-devolve` thread. **So it is never entered under `-evolve`** |
| `getNextHost` | called from `spawnTask` and `sweepToSpawn` |

The binary's own usage text is the confirmation: `-devolve` is "Evolve with
dynamic clients", `-evolve` is "Evolve without GUI".

**So this code is unexercised, not dead, and the difference matters.** Under
`-evolve` — the four completed reference evolutions, and everything this port
can run — the dynamic-host thread never starts and `addDynHost` is never
called. Under `-devolve` it is live. Recording it as "dead" would be wrong.

*D17 recorded `flushAllDynHosts` as "not gated … entered on every run,
`-evolve` included", and that the function was "exercised constantly". **Both
are wrong**, found by review checking the source against the binary reading:
the two call sites are guarded by `serverIsUp`, which only the `-devolve`
thread sets. Under `-evolve` the function is not entered at all. The error is
in the safe direction — less exercised than claimed — but a coverage statement
this file carries for the next owner has to be right.* Its body is
additionally behind `if (dynHosts.count() > 0)`, so the
`pvmHosts.resize( size-1 )` inside it is doubly unreachable here.

**And the modulus concern cannot be settled from the reference machine.** All
four evolutions used exactly **one** `PVMHOST` — the `8` in those lines is the
slave-slot count on a single host, not eight hosts. So `pvmHosts.size()` was 1
throughout and `nextHostNumber % 1` is 0 on every one of the ~56,000 spawns in
the longest run. **Host rotation has never been exercised by anything**, and
the unsigned-wrap question D9 raised stays open for `getNextHost` until an
experiment with two live hosts exists. That box has one machine; the rest of
the 2003 cluster is gone.

### D18 — the cursor walk, checked against the shim it replaces

`toSpawnList` becomes `QList<QList<int> *>`. It is the hardest container in
the plan: `sweepToSpawn` walked it with `Q2PtrList`'s **internal cursor** and
removed the **current** element while iterating, which is the one shim
behaviour `QList` has no equivalent for.

The cursor is written out as an explicit index, reproducing
`cursorAfterRemoval` exactly: after a removal the cursor stays on whatever slid
into the slot, steps back to the new last element if the removed one was last,
and dies if the list emptied. `next()` on a dead cursor stays dead and does
**not** advance.

**`setAutoDelete(true)` was its only ownership** — there is no
`deleteContents()` anywhere in the file, so `~Q2PtrList` was the free. That is
§9 item 2, and the destructor now does it explicitly.

**Nothing executes `sweepToSpawn`** — it needs a live PVM spawn — so the
rewrite would have shipped unverified. `Q2PtrList` is still in the tree, so the
self-check now runs the **same sequence of operations against both** and
requires they agree at every step: same values, same null-ness, same count.
The block dies with the shim, by which time the conversion is proven.

**The self-check tests a TRANSCRIPTION, not `sweepToSpawn`.** It copies the
index logic rather than calling it, because `SIG_GPFitnessTrainer` is not
linked into `sigel_eval`. Demonstrated by review, not argued: dropping
`cursorAfterRemoval`'s step-back **in production `sweepToSpawn`** leaves
`check.sh`, `-selfcheck` and `pvm-check.sh` all passing, while the identical
mutation in the self-check's copy fails it. So "verified to have teeth" is true
of the copy and **false of the shipped function**. What the block is genuinely
worth is pinning the *shim's* semantics while the shim still exists.

**The evidence that the shipped rewrite is correct is the review's, not the
self-check's.** It transcribed the production statements verbatim and drove
them exhaustively against `Q2PtrList`: list sizes 0–6 × all 4096 twelve-step
success/failure scripts = **28,672 walks under ASan and UBSan, 0 divergences**
in visit order, free order, surviving list, iteration count and break
behaviour. That covers removal of the first, a middle and the last element, the
list emptied, the single-element case, the dead cursor, and the
`actJob == prevJob` break. Mutation-tested: dropping the step-back gives 8,256
divergences, advancing after removal 12,544.

Two gaps in the block itself, both found by that review and both now closed:
its 10-step script **breaks at step 7** so its last three removals never ran
and the list was never emptied — the one case this section's own prose singles
out — and `next()` on a dead cursor was unreachable. It now drains a
two-element list to empty and drives the cursor three steps past the end.

*One honest note on that second one: `sweepToSpawn`'s `while (actJob)` means
the dead-cursor branch cannot be reached from the walk at all, so that guard is
unreachable in production. It is kept because it documents the shim, not
because it covers anything.*

### D19 — `pvmTasks` and `pvmHosts`, the last containers in the trainer

Both become `QList<T *>`. The four traps §10 recorded before this step, and
what each became:

| trap | handling |
|---|---|
| `insert(i, d)` **deletes the occupant and does not shift** | **eight** sites, not the six this row first claimed. Six became `delete v[i]; v[i] = d;`. Two — the two null-fill loops — became `fill(0)` and a plain `v[i] = 0`, because `insert` deleted nothing there: the slots had just been value-initialised. Behaviourally exact either way, but "one spelling is exact everywhere" is not what the file does |
| `pvmHosts.resize( size-1 )` **is the free** | `resizeOwningHosts()`, the same four-line helper shape D15 used. The only shrink in the file |
| `nextHostNumber % pvmHosts.size()` was **unsigned** | `static_cast<uint>` at both sites, preserving the wrap. This is D9's defect exactly, and it is the second time this port has had to write that cast |
| `isEmpty()` meant `count()==0`, occupied slots | `pvmHosts` has no null slots — every slot is filled by the loop that sizes it — so `QList::isEmpty()` agrees. **`pvmTasks` is full of null slots**, but nothing calls `isEmpty()` or `count()` on it |

**Both were §9 item 2 sites**: `setAutoDelete(true)` was their only free, and
nothing in the file frees either. The destructor now does, explicitly.

**D19 SHIPPED A LEAK AND THIS SENTENCE WAS HALF FALSE.** It removed the flag
from both and gave **only `pvmTasks`** a `qDeleteAll`. `pvmHosts` was left with
no free anywhere — the destructor's loop tells PVM to drop each host but never
owned the object, and `resizeOwningHosts` frees only the dynamic tail. Every
enabled `PVMHOST` leaked one `SIG_GPActivePVMHost` per trainer destruction, and
all 56 shipped `.exp` have at least one. Found by review, over a
**19,500-scenario sweep** against the shim: as committed, 41 scenarios diverged,
the smallest being one static host and no spawns. With the one missing
`delete` added, **all 19,500 agree** on host-vector contents, `getNextHost`'s
full result sequence, `nextHostNumber` and free accounting. **It was the only
behavioural difference in the entire conversion.**

**`pvmTasks` grows without bound, and that is pre-existing.** It is indexed by
`nextFreeNumber`, which starts at 0, increments once per spawn, and is
**never reset** — confirmed against the 1.3 binary, which has exactly two
writes to that member: `movl $0x0` in the constructor and `incl` in
`spawnTask`. No reset is compiled from anywhere. The array grows by one
population's worth whenever the index catches up, and never shrinks.

For the reference machine's longest run — 56,333 spawns at `POPULATIONSIZE`
250 — that is about 225 growths ending near 56,500 slots, **~226 KB** on the
2003 i386 build. **Arithmetic, not measurement**: no RSS, `ps` line or memory
note survives in any of the four run logs, and the processes are long gone.
Recorded as inferred from source.

**A second consequence, found by the same binary work and not by us.**
`stopTrainersSlaves` uses `nextFreeNumber` as its loop bound, so shutdown walks
the whole grown array rather than the live tasks — **O(total spawns ever)**,
scanning tens of thousands of mostly-null slots on a long run. Harmless, and
the second place the unbounded index reaches behaviour.

*The warning figures recorded for D11, D12 and D13 are each one low, from a
mis-count propagated forward. The current count is in §7; do not quote one from
a step write-up.*

### D20 — the full-data recorder, and the first converted walk a gate runs

Four lists in `SIG_GPFullDataRecorder` become `QList<T *>`. The type change
forces **ten** files in one commit: the recorder's header and source, six
fitness functions, `SIG_EarlyRunTermSimulation`, and `sigel_eval`'s trace walk.

*`positions`, `rotations` and `touchdowns` are **public** — friendship is not
what grants access, and only `listForces` is private. Two readers are not
friends at all (`SIG_EarlyRunTermSimulation`, `sigel_eval`), and one listed
friend, `SIG_GPEnergyFitnessFunction`, **does not exist anywhere in the tree**.*

**Most** walks are the same shape — `positions.first()` and
`rotations.first()`, then `next()` on both in lockstep — so one index with
`QList::value()` is exact: `value()` yields null past the end exactly as
`first()`/`next()` did, and the two lists are appended together so they cannot
fall out of step. *Two are not that shape, and "every walk" was wrong:*
`SIG_GPForceFitnessFunction` walks a single list and deliberately skips its
first element, and `SIG_GPStepperFitnessFunction` carries a **third**,
`touchdowns`, on the same index — sound because `record()` appends all four
inside one block.

| list | ownership |
|---|---|
| `positions`, `rotations`, `touchdowns` | **owned.** 1.3 set `setAutoDelete(true)`; the port had already replaced that with `deleteContents()`, now `qDeleteAll` + `clear()` |
| `listForces` | **not owned, and never was** — no `setAutoDelete` even in 1.3, verified against the pristine tarball. The recorder only clears |

**But do not read that row as saying the memory is accounted for.** Only
**one of the six** fitness functions frees the force vectors, and `record()`
allocates a `vector<double*>` plus a `new double[6]` per link on **every**
recorded frame regardless of which fitness function is running. Measured on one
`-v` run of `hammer`: **11,891,420 bytes in 252,265 allocations**, the
overwhelming majority from `dmArticulation::getForces`. That is the leak §D4
already records as scaling with links and frames. D20 touched exactly the list
that causes it and left the attribution alone, which is right under the
governing rule — but the first version of this row read as though the vectors
were freed, and for five of six fitness functions they are not.

**A converted walk that runs — but the coverage is one bit wide.**
`SIG_GPNiceWalkingFitnessFunction` is linked and evaluates 18 of the 42 gate
individuals, so its rewritten walk executes on every gate run. *D20 called that
"real coverage rather than inspection". **It is not, and the review measured it
both ways.*** Setting the walk's index to 1 instead of 0 — dropping the first
recorded frame, the classic error for exactly this conversion — leaves the
fitness gate **byte-identical across all 42 individuals**. Setting it so the
loop never runs *does* move 9 rows. So the gate discriminates only "the walk
found an invalid frame" from "it did not", one bit per individual. It cannot
see which frame, how many, or in what order.

**And `sigel_eval`'s trace walk is exercised but unchecked.**
`dictorder-dump.sh` filters stdout with `sed`, and the only thing that loop
produces — the `frames … height … last …` line — is dropped by that filter.
Patching the loop to skip its first element leaves the dictorder gate empty.
The five other fitness functions and `SIG_EarlyRunTermSimulation` are not
linked at all.

**A null dereference fixed rather than reproduced, per D13.**
`SIG_GPForceFitnessFunction`'s cleanup loop was a `do`/`while` that
dereferenced before testing:

```cpp
vector<double*>* p = recorder.listForces.first();   // null if no frames
do { ...p->size()... } while ((p = recorder.listForces.next()) != 0);
```

`Q2PtrList::first()` returned null on an empty list, so an evaluation that
recorded no frames took a null dereference while freeing. It is a range-for
now: identical with frames present, a no-op with none. Same shape as
`SIG_Link`'s `do`/`while` in D10 — but there the once-through was
**load-bearing** and had to be preserved, and here it is a crash. The
difference is which side of the null the body is written for, and it has to be
read each time rather than pattern-matched.

### D21 — `Q2CString` is gone, and a `+ 1` that was really a `+ 2`

Seven sites, all in PVM code. Six in `SIG_GPFitnessTrainer` are the same
shape — a named local and an implicit conversion to `const char *`, which is
the only thing `Q2CString` ever provided — and become
`const QByteArray` + `.constData()`.

**The seventh is the PVM string-length fix and it is not trivial.**
`SIG_GPPVMData.cpp` computes the wire length that sizes the receiver's buffer:

```cpp
Q2CString qCStringBuffer = str.toUtf8();
int finalLength = qCStringBuffer.size() + 1;      // NOT byte length + 1
```

`Q2CString::size()` reported `QByteArray::size() + 1` **for a non-null
string**, because Qt 2's `QCString` counted the terminating NUL in its length.
So that line sent **byte length + 2**: one byte for the NUL `pvm_upkstr`
writes, and one spare. `QByteArray::size()` is the plain byte count, so
reproducing that needs `+ 2` — and a mechanical rename with the `+ 1` left
alone would have quietly shortened every message by a byte.

**One input does not follow that rule, and D21 stated it flatly.** `size()`
**special-cases a null string to 0**, not to `size()+1` — the shim's own
self-check asserts it (`q2compat_check.cpp:151`). So for a null `QString` the
old value was 1 and the new one is 2. *It never reached the wire:*
`Q2CString`'s `const char *` conversion returned **nullptr** for a null string
and `pvm_pkstr` does `strlen(cp)` unguarded, so the old code **segfaulted**.
The new code sends an empty string. That is a crash removed, not a value
changed — logged in §9's D13 table.

**Preserved rather than tightened.** `+ 1` would fit exactly and is arguably
what the Phase P fix meant; `+ 2` is what has been on the wire. Dropping the
margin is a behaviour change and not this step's to make. The `+ 2` carries a
comment saying it is not a typo.

**NOTHING VERIFIES THE `+ 2`, and D21 claimed `pvm-check.sh` did.** Measured
by review: with `+ 1` it passes, and with `+ 0` — one byte *shorter* than
`pvm_upkstr` writes — it **also passes, with no sanitizer report**. `QList`
over-allocates, which §7 already records, so the check is blind to a shortfall
of one to seven bytes; it only fails at about eight. It proves the file links
and round-trips. It says nothing about the constant this step is named for.

No gate touches this file — see "What the gates actually reach".

`Q2CString` now appears in no code outside the shim.

### D22 — `crossOver`, an owning container returned by value that owned nothing

`SIG_GPOperations::crossOver` returned `Q2PtrVector<SIG_GPIndividual>` **by
value**, and three tournament sites received the copy. That is the shape the
shim exists to defuse — Qt 2's copy constructor cleared `del` on the copy, so a
copy freed nothing while the original might.

**Here neither end ever owned anything.** No `setAutoDelete` on the local
`crossedInds`, none on the caller's `cinds`, so `del` was false throughout and
both destructors freed nothing. The two individuals are handed to
`SIG_GPPopulation::setIndividual`, which takes ownership — the same call whose
free of the losing individual D15 had to write out. `QList` never frees a
pointer, so this conversion moves **no ownership at all**.

*D22 called that "the first time in Phase D". It is not — **D9 already did
exactly this** for `drives` and `sensors`, same type, same `del`-never-set
finding, same `insert`→assignment rewrite, and its own text says "this step
moved no ownership; it only changed the spelling". D11 adds two more. What is
actually new here is **returned by value**, not "owned nothing".*

`insert(0, x)` and `insert(1, x)` go into slots a `QList(2)` has just
value-initialised to null, so `insert` deleted nothing and a plain assignment
is exact.

Six sites, three files — and **all three** dropped `compat/q2compat.h`, including `SIG_GPOperations.h`.

**Nothing executes any of it**, and the accurate statement is one notch
stronger than D22's "syntax pass": both files are fully compiled under ASan and
UBSan into `libSIGEL_GP.a`, archived, and never pulled into a link.

*D22 also pointed at the wrong binary.* `SIG_GPCrossOverTournament` is
constructed by `SIG_GPManager.cpp:336`, which is **master-side** — the binary
that would run `crossOver` is `sigel`, not `sigel_slave`. The conclusion holds,
since neither links, but the reasoning was borrowed from D21's trainer and does
not transfer.

### D23 — `Q2Queue` becomes `QQueue`, and the API used is three methods

`MT_Substitute::TCaseBuffer` and the two parameter types become
`QQueue<MT_TrainingCase *>`. Qt 6 has a real `QQueue`, so `enqueue`, `dequeue`
and `count` keep their names and only the type spelling changes: **seven** type
sites across six files, no call site touched — `MT_Substitute.h` carries two,
the `changeTCases()` return type at `:66` and the member at `:110`. *Count
sites, not files.*

**The shim's `Q2Queue` had eleven methods; `TCaseBuffer` uses three.**
`current` and the implicit `operator T *` have no caller anywhere in the tree.
**`clear`, `head` and `remove` do** — `MT_GUI/MT_ExperimentWidget.cpp` calls
`remove()` at `:43` and `:51` and `head()` at `:48`, which is what makes the
`head()`-on-empty hazard below live for Phase C.

**The divergence runs the other way round from the obvious guess.**
Qt 2's `dequeue()` on an empty queue returned 0 and carried on:
`QQueue::dequeue` is `QGList::takeFirst` (`qqueue.h:59`), which calls `unlink()`,
which opens `if ( curNode == 0 ) return 0;` (`qglist.cpp:438-439`; :436 is the
signature). `takeFirst` is null-safe twice over — `Item d = n ? n->data : 0;`.
The shim
reproduced that faithfully. **Qt 6's `QQueue::dequeue()` is `QList::takeFirst()`,
which is `Q_ASSERT(!isEmpty())`** — measured here: abort at `-O1 -g`, segfault
under `-DQT_NO_DEBUG`. So the shim matched the reference and *`QQueue` is the
divergence*, against the rule in SCOPE.

It cannot fire through the only drain today. `MT_Trainingset::updateTSet`
dequeues `count() - TSize` items, recounts, then dequeues the remainder:
exactly the whole queue, never one more. **Confirmed by differential probe**, not by
reading alone — `updateTSet`, `insertTCase` and `~MT_Trainingset` transcribed
verbatim, templated over the queue type, and run against `Q2Queue<Case>` and
`QQueue<Case *>` side by side under ASan and UBSan with a live-object counter:
`TSize` in {0,1,2,3,5,8,100} × `N` in 0..2·`TSize`+3, prefilled and empty ring.
**0 mismatches** on slot contents, `PresentTSize`, `FreePosition`, `TSetName`,
residual count and `Case::live == 0` after teardown.

**The same reversal has a live consequence for Phase C.** Qt 2's `head()`,
`current()` and `operator type *()` all route through `cfirst()`
(`qglist.h:188`), which returns 0 on an empty list. Qt 6's `QQueue::head()` is
`QList::first()`, which asserts. `MT_ExperimentWidget::lastSelected()` calls
`prevSelectedItems.head()` with no emptiness guard, so before any selection it
returned 0 in 2003 and will abort under Qt 6. `MT_GUI` is in neither
`check.sh`'s `MODULES` nor the Makefile's `CORE`, which is why nothing flagged
it. Recorded in §9's name-collision table.

**The queue never owned its contents and still does not.** No `setAutoDelete`
on `TCaseBuffer` — the self-check does call it, at `q2compat_check.cpp:171`, so
"anywhere" was wrong — hence `~Q2Queue` freed nothing and `~QQueue` frees
nothing.
`dequeue()` transfers ownership out, and `updateTSet` either `delete`s the
excess explicitly or hands each case to `insertTCase`. `~MT_Substitute`
(`MT_Substitute.cpp:26`) touches only its three mutexes. **Anything left in the
queue at destruction leaks** — pre-existing, unchanged, and now written down.

**The drain runs under a real lock, and the conversion is neutral on it.**
`MT_Substitute::tCaseBufferMutex` is a `pthread_mutex_t` (`MT_Substitute.h:61`),
initialised at `MT_Substitute.cpp:21`, taken by both producers
(`MT_Classifier.cpp:137/151`, `MT_Evaluator.cpp:498/507`) and by the drain
(`MT_GPManager.cpp:705/713`). `Q2Queue` was already implemented over
`Q2PtrList`, whose storage is `QList<T *>`, so storage, reallocation and
implicit sharing are unchanged; the refcount is always 1 because no copy is
ever made. The conversion drops the `Q2PtrList` cursor — one fewer non-atomic
field written under the lock. *The "exactly the whole queue" argument above
does not mention that a concurrent producer exists; it survives either way,
since a producer only grows the queue and so neither `count()` read can
over-report.*

Four of the six files dropped `compat/q2compat.h`. `Q2Queue` appears in no code outside the shim.

**Coverage: compiled, archived, linked into both binaries, never executed** —
the figures and the classes that do and do not link are in "What the gates
actually reach", which is also where an inverted version of them was corrected.

### D24 — `MT_Evaluator::TmpBuffer` and `MT_Statistics::StatisticsOfGeneration`

Both are `Q2PtrList`; both become `QList<T *>`. Two headers, two bodies. **The
site count depends on what you count**, so state it: the diff changes **8**
call-site lines (7 conversions plus the `setAutoDelete` deletion), the table
below enumerates **11** uses, and the tree holds **20** — because
`MT_GUI/MT_StatisticsWidget.cpp` makes nine further `.count()` calls on
`StatisticsOfGeneration` that this step does not touch and nothing compiles.
**Checked against the 1.3 binary before writing**, which is a first
for a `Q2PtrList` step.

**`TmpBuffer` — the loop is not what the plan said it was.** The note carried
from D23 described "a `take` and a conditional re-`insert`, in a loop that
changes its own bound while indices shift underneath". Reading it
(`MT_Evaluator.cpp:478-514`), both halves are wrong:

```cpp
int TmpBufferSize = TmpBuffer.count();     // snapshot, never re-read
for (int i = 0; i < TmpBufferSize; i++) {
    TCases = TmpBuffer.take(i);            // removes, shifts down
    if (TCases->getName() == taskId) {
        TmpBufferSize--;                   // dead: break follows
        ...enqueue on TCaseBuffer...
        break;
    } else
        TmpBuffer.insert(i, TCases);       // puts it straight back at i
}
```

The bound is a snapshot and the index advances normally. It is safe because the
non-matching branch reinserts at the same position, so `take`-then-`insert` is a
round trip and the indices never actually shift. Only the matching element stays
removed, and the loop breaks on it. It is a linear search written as
remove-and-maybe-replace. `TmpBufferSize--` is dead — `break` is **nineteen**
lines later (`:489` to `:508`, with four statements between: `setFitness`, the
mutex lock, the `enqueue`, the unlock) and the variable is never read again; it
appears only at `:478`, `:480` and `:489`. *An earlier draft of this section said
"three lines later". The conclusion is right, the distance was not.*

**The 1.3 binary confirms all of it, including the dead code.** In
`MT_Evaluator::checkTask` at `0x08110e58`: the back-edge `8111023: jmp 8110f78`
targets the comparison, and `count` is called once at `8110f66` **outside** the
loop, so the bound is snapshotted. And `8110fb6: decl -0x10(%ebp)` — the dead
decrement, on the same frame slot `count` wrote — sits on the matching side of
`jne`, before `call setFitness` at `8110fc9`. *A dead store surviving in both is
stronger evidence of a common source than any live path, because no behaviour
forces it to agree.* **Do not read this as a compiler result**: the 1.3 build is
plainly unoptimised — `mov %eax,%eax`, every local reloaded through the frame —
so nothing was eliminated anywhere — the evidence is about the source text, not
about a compiler decision that was never made. That matters here beyond D24: it is the first
hard evidence that the `MT_` half of the tree, where the only confirmed
source/binary divergence lives, is otherwise common. **It does not license
generalising to all of `MT_`** — one function is one function.

**`setAutoDelete(false)` is dropped, and that is safe because it was a no-op.**
It is the only such call on `TmpBuffer`, nothing ever sets it true, and Qt 2's
default is already false. `QList` has no equivalent. The 1.3 binary calls it
with `pushw $0x0`, confirming the argument. So the container never owned its
items before or after.

**`StatisticsOfGeneration` carries the `at()` trap.** `Q2PtrList::at(i)` returns
**nullptr** out of range (`q2compat.h:270`); `QList::at(i)` is undefined
behaviour. `MT_Statistics.cpp:79` writes
`if (getStatisticElement(i) != NULL)`, and `getStatisticElement` is nothing but
`return StatisticsOfGeneration.at(...)` — so the 2003 author relied on that null
defensively, and it is a public accessor any caller can pass anything. **Both
`at()` sites become `value()`**, which returns a default-constructed `T *`, i.e.
nullptr, out of range. `at()` would have compiled silently and been UB on the
path the original guards.

| site | was | now |
|---|---|---|
| `MT_Statistics.cpp:66,78,112` | `count()` | `size()` |
| `MT_Statistics.cpp:87` | `append` | unchanged |
| `MT_Statistics.cpp:94,115` | `at(i)` | **`value(i)`** |
| `MT_Evaluator.cpp:447` | `append` | unchanged (the `new` is at `:134`) |
| `MT_Evaluator.cpp:478` *(pre-commit)* | `setAutoDelete(false)` | **deleted**, no-op |
| `MT_Evaluator.cpp:478` | `count()` | `int( size() )` |
| `MT_Evaluator.cpp:484` | `take(i)` | `takeAt(i)` |
| `MT_Evaluator.cpp:511` | `insert(i, p)` | textually unchanged, **semantically not** — see below |

**Two more silent null-to-UB upgrades in this same step.** Both are unreachable
today; both belong on the §9 list, because the rule is the hazard class, not the
individual site.

- **`insert` is not "unchanged".** `Q2PtrList::insert(uint, const T *)`
  (`q2compat.h:261-268`) **returns false and does nothing** when `i > size`.
  `QList::insert` is `Q_ASSERT_X(size_t(i) <= size_t(d->size))` — abort in
  debug, UB in release. Same shape as `at()`, in a line the table called
  unchanged because the *text* did not change.
- **`takeAt` differs from `take` out of range.** `Q2PtrList::take(i)` returns
  nullptr, so the original would have null-dereferenced deterministically at
  `TCases->getName()`. `QList::takeAt(i)` is out-of-range `operator[]` — abort
  or a heap read. **This is the one input where converted and original differ.**
  It needs `i >= TmpBuffer.size()` at the top of an iteration, which needs
  another thread to shrink `TmpBuffer` between the snapshot and the `takeAt`.
  There is **no mutex on `TmpBuffer`** — unlike `TCaseBuffer` — so two
  concurrent `checkTask` calls can do it. The append path only grows, so it
  cannot. A pre-existing race with a differently-flavoured crash; not a port
  step's business to fix, but it is now written down.

**Warnings fall 315 → 313, and the two lost are accounted for.** Both were
`-Wsign-compare` at `MT_Statistics.cpp:78` and `:112` — `int i < uint count()`.
Qt 6's `size()` is signed `qsizetype`, so the comparison is signed-vs-signed and
the warning is correct to disappear. No behaviour change: for `i >= 0` the
Qt 2 form converted `i` to unsigned and compared equal. *A step that loses a
warning without naming it has hidden something; these two are named.*

**Ownership is unchanged. Neither container owns its items — but only one of
the two leaks everything.** `TmpBuffer`'s cases are `new`'d in `createNewTCase`
(`MT_Evaluator.cpp:134`) and appended at `:447`; `~MT_Evaluator` (`:73`) deletes
only `Interpreter` and `BestMETAProgram`. **A *matched* case does get freed**,
though: it is enqueued on `TCaseBuffer`, which reaches
`MT_Trainingset::updateTSet`, and that deletes at `MT_Trainingset.cpp:120`
(`delete (NewTCases->dequeue());`) and `:145` (`delete TCases[FreePosition];`).
So what leaks is the **never-matched** remainder left sitting in `TmpBuffer`, not
every case.

`StatisticsOfGeneration`'s elements are `new`'d at `MT_Statistics.cpp:56` on the
load-from-file path **and at `MT_GPManager.cpp:557` on the live path** (appended
`:575`); `~MT_Statistics` (`:18`) is empty and no `delete` of an
`MT_StatisticsElement` exists anywhere in the tree, so that one does leak
entirely. All of it pre-existing, unreachable while the subsystem does not run,
and recorded rather than fixed — a leak fix is not a port change.

`MT_Statistics.cpp` is a **CRLF** file. Verified byte-exact: every changed line
carries `^M` on both sides of the diff, and no line I did not edit moved.

`Q2PtrList` 39 → **37** (lines, source tree only).

### D25a — `taskCanDoList`, and why the iterator had to go

`SIG_GPManager::taskCanDoList` is `Q2ValueList<int>` → `QList<int>`. Header
+2/−1 (the declaration, plus an explicit `<QList>`), fourteen body lines, and
two walks with an **identical 14-line skeleton** — in `evolutionLoop()`
(`:80`, walk at `:107`) and `evolutionLoop(MT_Classifier *)` (`:1250`, walk at
`:1289`). *An earlier draft named these `evalNewIndis` and `evalNeededIndis`.
Those are at `:367` and `:1458` after D25b inserted a guard above the first of
them (`:351` and `:1432` before it), they hold `fitTaskList`, and neither
contains a `taskCanDoList` walk at all. The 1.3 binary settles it: the sweep's
`usleep(300000)` appears at exactly two call sites, both inside
`evolutionLoop`.* The **bodies** differ — `:156 actTour.run()` against
`:1342 actTour.run(MetaClassifier)`, and the second carries an extra
`successor != -1` guard the first does not; only the walk is common.

**A direct iterator-for-iterator conversion here would have been a
use-after-free.** Qt 2's `QValueList` is a **doubly-linked** list
(`qvaluelist.h:51-62`) — the shim models it as `std::list<T>` for exactly this
reason — so an iterator stays valid when the list is modified elsewhere. The
loop relies on that:

```cpp
Q2ValueList<int>::Iterator canDoIter = taskCanDoList.begin();
while (canDoIter != taskCanDoList.end()) {
    ...
    taskCanDoList << actSuccessor;                    // :171 and :209 -- APPEND
    ...                                               // while canDoIter is live
    if (!actTour.justWaiting)
        canDoIter = taskCanDoList.remove( canDoIter );  // returns the next
    else
        ++canDoIter;
}
```

`QList` is contiguous. That append can reallocate, and then `canDoIter`
dangles. This is not a null-versus-UB hazard like the ones in §9 — it is
**iterator invalidation**, a different member of the same family: *Qt 2 defined
it, Qt 6 does not.*

**The faithful conversion is an index walk**, because the container is
contiguous and every append goes to the end:

| was | now | why it matches |
|---|---|---|
| `Iterator canDoIter = begin()` | `qsizetype canDoIdx = 0` | |
| `while (canDoIter != end())` | `while (canDoIdx < size())` | growth extends the loop in both — but **not** because `end()` moves. Qt 2's `end()` is a **fixed sentinel**, `Iterator(sh->node)` (`qvaluelist.h:363`), allocated once. `insert(end(), x)` splices the new node in *front of* the sentinel (`:215-223`), i.e. behind the live iterator, so the walk reaches it. *An earlier draft said `end()` was re-read and moved; right conclusion, wrong mechanism.* `size()` genuinely is re-read |
| `*canDoIter` | `taskCanDoList.at( canDoIdx )` | six sites |
| `canDoIter = remove(canDoIter)` | `removeAt( canDoIdx )`, **no advance** | Qt 2's `remove` returned the *next* iterator; after `removeAt` the next element slides into the same index |
| `++canDoIter` | `++canDoIdx` | |

Appends stay correct: a forward iterator over a linked list eventually reaches
an element appended behind it, and so does an index walk over a growing array.

**Compiled under the sanitizers and archived, linked into nothing** — see
"What the gates actually reach". A full sanitized compile is not nothing; it
just is not execution. **The append does fire, and the trap was live rather than latent.** Measured on
the 1.3 binary under gdb, two generations of `twoBasesSimpleFitness1`, 100
individuals, `SLAVES=8`:

| counter | count | what it establishes |
|---|---|---|
| `begin()` | 33 | the sweep genuinely ran, 33 times |
| append at `0x80bf7a5` | 33 | first `taskCanDoList << actSuccessor` site |
| append at `0x80bf9e4` | 47 | second site — **80 appends inside the live loop** |
| `remove(iterator)` | 94 | 94 removals across 33 sweeps, so these are multi-iteration walks, not single passes |

Both append addresses fall inside the loop body: the condition is at
`0x80bf635`, the body runs `0x80bf648`–`0x80bfa58`, and the back-edge at
`0x80bfa58` returns to `0x80bf614`. **So a direct iterator conversion would have
been a use-after-free in the evolution loop on a path the shipped corpus takes
80 times in two generations.** The index walk was load-bearing, not defensive.

*A negative result here would have been corpus-dependent and would have had to
be recorded as such. A positive is not: the path fired, so it can fire.* The run
used the 5-second `TIMETOSIMULATE`, so the **rate** is this configuration's
number, not a universal one; whether it happens at all is settled.

This loop leans on **both** halves of the Qt 2 contract — stability under
append, and a valid successor after removal — 174 times between them in those
two generations.

**The index walk was checked against the node semantics it replaces, not just
argued.** A differential model — Qt 2 nodes (`std::list`, `erase` returns next,
append at tail) against the committed index walk, both driven by the *same*
randomised decisions, comparing the full visited sequence **and** the final
container contents — ran **200,000 cases with 0 divergences** under ASan and
UBSan: empty list, `maxTouchsPerLoop` 0 and −1, removal of the only element,
removal of the last element, appends mid-iteration, and appends landing on the
same iteration as a removal.

**One pathology is preserved deliberately.** With `maxTouchsPerLoop == 0` the
break fires on the first pass, nothing is ever removed, and the outer
`while (!taskCanDoList.isEmpty())` spins forever. That hang is identical in both
versions — preserved, not introduced, and unreachable on shipped data because
the value is always −1.

`Q2ValueList` now appears in **no user code at all** — only the shim and its
self-check (12 → 9 lines, narrow scope). The shim comment that justified keeping
it named the very code this step deleted; it now records the measurement
instead. The `q2compat.h` include stayed in
`SIG_GPManager.h` at this step because `tours` was still `Q2PtrVector` —
**D25c removed it**; `<QList>` was already included explicitly here rather than
arriving through the shim, which is why that removal was a one-line change.

**D25 is three commits, not one**, because it is three container types with
three different hazards: this one, then the two `fitTaskList` (`Q2PtrList` with
`setAutoDelete(true)` — an owning container, §7), then `tours`
(`Q2PtrVector`, whose `insert` and shrinking `resize` are **hidden frees** and
whose `size()` means allocated slots, not element count).

### D25b — the two `fitTaskList`, an owning container with four leak paths

Both are **locals**: `Q2PtrList< QList<int> >` with `setAutoDelete( true )`, in
`evalNewIndis` (`:373`) and `evalNeededIndis` (`:1450`). They become
`QList< QList<int> * >`.

**`setAutoDelete(true)` here is not the no-op it was in D24 — it is the only
free, and it runs on four paths.** `~Q2PtrList` fired at every exit from these
functions. Two of the four exits are **early returns that sit after the list is
populated**:

| function | populate | early returns inside the populated region |
|---|---|---|
| `evalNewIndis` | `:413` append | `:386` (inside the `poolSize` loop) and `:426` (inside the sweep `while`) |
| `evalNeededIndis` (`:1460`) | `:1518` append | `:1501` and `:1528`, same two shapes |

*`evalNeededIndis`'s line numbers are post-guard-block; `:1518` is the populate
line, not an early return.*

**The two `poolSize`-loop returns are *conditional* leak paths, not certain
ones.** The append at `:413`/`:1518` is gated on `!upToDate`, so if every
individual is already up to date the list is still empty when `:386`/`:1501` is
reached and nothing leaks. *An earlier draft said "non-empty from the second
iteration", which overstates it.* The two sweep-loop returns (`:426`, `:1528`)
sit inside `while (!fitTaskList.isEmpty())` and are unconditional.

A plain container swap loses all four. The fix is an RAII guard, following
`DynaMechsLinkGuard` from D9 rather than inventing a second pattern:

```cpp
struct FitTaskListGuard
  {
    QList< QList<int> * > *tasks;
    ~FitTaskListGuard() { if (tasks) qDeleteAll( *tasks ); }
  };
```

declared immediately after the list, so it destructs first and the list is still
alive. No double free: the walk's `takeAt` removes an item **before** deleting
it, so the guard never sees it.

**The cursor walk is the D18 pattern again**, and it carries the `first()`
hazard §9 records — this is the site the 1.3 sweep flagged and the one D25's
plan was written around:

| was | now |
|---|---|
| `fitTaskList.first()` | `fitCur = isEmpty() ? -1 : 0`, then `at(fitCur)` — **not** Qt 6's `first()`, which is UB on empty, and this loop *terminates on the null* |
| `fitTaskList.remove()` | `delete fitTaskList.takeAt( fitCur );` — `setAutoDelete` made `remove()` the free, so the `delete` is explicit now |
| `fitTaskList.current()` | `cursorAfterRemoval`: `if (fitCur >= size()) fitCur = isEmpty() ? -1 : size()-1;` then `at(fitCur)` |
| `fitTaskList.next()` | `if (fitCur < 0 \|\| ++fitCur >= size()) { fitCur = -1; actFitTask = 0; }` — a dead cursor stays dead and does **not** advance |

**A pathology that looks like it needs preserving does not exist.**
`prevFitTask` cannot hold a pointer whose object a later `delete` has freed, in
either version. `prevFitTask` is assigned only
in the else branch, to the item at index *p*, after which the cursor moves to
*p+1*; deletions happen only at the cursor; `cursorAfterRemoval` from an index
≥ *p+1* yields a cursor ≥ *p*; and the moment it reaches *p* the
`actFitTask == prevFitTask` test fires and breaks **before** any further delete.
Measured too: 0 dangling comparisons over 155,641 exhaustive schedules, in the
original as well as the conversion. Keeping the comparison is right — it is the
loop's termination condition — but the justification was invented.

`Q2PtrList` 37 → 36. **`setAutoDelete` in core falls 3 → 1**: only
`tours.setAutoDelete( true )` at `:64` remains, and that is D25c.

**Compiled twice, archived, linked into nothing** — same as D25a.
`-fsyntax-only` by `check.sh`, a real `-c` by the Makefile into both `build` and
`build-fast`, then archived; `nm -C` gives 0 `SIG_GPManager::` in
`build/sigel_eval`, `build-fast/sigel_eval` and `build/pvm_link`. Nothing
executes it. **The whole ownership change is checked by the type system and by
review alone.**

**Review supplied the execution the gates cannot.** A differential harness ran
the *real shim* `Q2PtrList` walk against the *verbatim converted* index walk over
the same oracle, exhaustively — list sizes 0–6 × every readiness bit-schedule,
**155,641 cases** under ASan and UBSan: `diffs=0`, `oobAt=0`, `danglingPrev=0`,
no leaks. A separate ASan test drove the guard through early return, a throw
mid-loop, and the `takeAt`-then-`delete` sequence: no leak, no double free.

Two residual notes, neither a present defect. `delete fitTaskList.takeAt(fitCur)`
is unguarded where Qt 2's `remove()` was a silent no-op on an invalid cursor, so
a future edit that mutates the list mid-loop turns a no-op into UB. And
`FitTaskListGuard` is copyable while holding a raw owning pointer — inherited
from `DynaMechsLinkGuard`, and worth fixing in both at once rather than
diverging them.

### D25c — `tours`, the sparse slot vector, and why its hidden frees are no-ops

`SIG_GPManager::tours` is `Q2PtrVector<SIG_GPTournament>` → `QList<SIG_GPTournament *>`,
with `MT_Classifier::preEvolution` and `::evalNeededTours` taking it by pointer.
**This is the `Q2PtrVector` bulk §10 said could not be linked until Phase C, and
the last `setAutoDelete` in core.**

*Line numbers in this section are **pre-commit** positions, so they match the
diff rather than the file at HEAD.*

**`Q2PtrVector` is not a list. It is a fixed-size array of nullable slots**, and
four of its operations mean something different from the `QList` method of the
same name:

| Qt 2 | what it does | `QList` equivalent |
|---|---|---|
| `size()` | **allocated slots** | `size()` — same only because the conversion keeps one element per slot, nulls included |
| `count()` / `isEmpty()` | **non-null** slots | *not* `size()`/`isEmpty()` |
| `insert(i, p)` | **overwrites slot `i`**, deleting the previous occupant; does **not** grow or shift; returns false if `i >= size` | `delete v[i]; v[i] = p;` — `QList::insert` *shifts*, which would be wrong |
| `remove(i)` | deletes the occupant and leaves a **null hole**; the slot stays | `delete v[i]; v[i] = 0;` — `QList::removeAt` *shifts* |
| `take(i)` | returns the occupant, empties the slot, **never deletes** | `p = v.value(i); v[i] = 0;` |
| shrinking `resize(n)` | **deletes the truncated tail** | delete `[n, size)` then `resize(n)` |

**All six hidden-free sites are provably no-ops — four in `MT_Classifier`, two
in `SIG_GPManager`.** This was
worth proving rather than assuming, because it is the difference between a
delete that must be reproduced and one that must not fire twice:

- `insert` at `:499` — slot `i` is always already empty. For `i < NumOfClassi`
  the first loop `take`s it; for `i >= NumOfClassi` this loop `take`s it at
  iteration `i - NumOfClassi`, which has already run.
- `insert` at `:566` — the branch is guarded by `if (Tourna == NULL)` where
  `Tourna = tours->at(i)`, so slot `i` is null by construction.
- shrinking `resize` at `:502` — the tail `[TourSize-NumOfClassi, TourSize)` is
  entirely inside the union of what the two loops emptied, whether or not
  `NumOfClassi` exceeds half of `TourSize`.
- shrinking `resize` at `:575` — `evalNeededTours` returns **exactly** the final
  count of `ToursWBestIndi[i] == 0` (it decrements `NumClassi` for each entry
  the calibration flips), so there are exactly `NumOfTour` survivors and the
  compaction packs all of them below `NumOfTour`.
- `resize` at `SIG_GPManager.cpp:240` grows from an empty vector, so it
  truncates nothing.
- `insert` at `SIG_GPManager.cpp:347` — `clear()` then `resize(quantity)` null
  every slot and the fill loop writes each exactly once, so the occupant is
  always null. **Two frees are real, not three.**

**The deletes are written out anyway.** Preserving the *semantics* rather than
the current behaviour: if a later edit breaks one of those proofs, the code
still frees what Qt 2 would have freed.

**Two frees were real and are now explicit**, since `setAutoDelete(true)`
(`:64`) is gone:

| site | was | now |
|---|---|---|
| `:239` `tours.clear()` | deleted every tournament | `qDeleteAll( tours ); tours.clear();` |
| `~SIG_GPManager` | `~Q2PtrVector` freed whatever was held; the destructor itself frees only `trainer` | `qDeleteAll( tours );` added |

**And that destructor addition created a defect, caught by review.**
`Q2PtrVector`'s copy constructor cleared `autoDelete` on the copy
(`q2compat.h:124`, matching `qcollection.h:64`), so a copied manager freed
nothing. A `QList` copy shares the raw pointers and **both** destructors would
run `qDeleteAll`. `SIG_GPManager` was copy-constructible; a `static_assert`
confirmed it. Closed with `= delete` on the copy constructor and assignment,
exactly as **D7** (`SIG_Geometry`) and **D15** (`SIG_GPPopulation`) did — and
`SIG_GUIGPManager` derives from this class, so Phase C is precisely the case
that would have found it. *The precedent was recorded twice in this file and I
still did not apply it when adding an owning destructor.*

**`isEmpty()` is the subtle one, and it is now reproduced rather than argued
away.** Qt 2's is `count() == 0` — *no non-null slots* — while
`QList::isEmpty()` is `size() == 0`. The four call sites use a file-local
`toursAreEmpty()` that counts non-null slots.

*Precisely, because a first reading of this got all four details wrong:* two of
the four sites (`:1057`, `:1078`) are in the overload `run(MT_Classifier *)`,
not `run()`; `run()` does **not** execute once, and the German message at one of
those very sites exists to detect a second call; the boundary is **`createTours`**,
not `calcInitTourSet`; and `createTours`' `!totalProbCount` early return
(`:262`) leaves the vector resized and entirely null **after the function
returns**, which the divergence window has to include.

**The conclusion survives all four.** `start()` has exactly two callers
in the 1.3 binary — `main`, which calls it once with no backward jump reaching
the call site, and `SIG_Experiment::slotStartEvolution`, which does
`delete gpManager; gpManager = new SIG_GUIGPManager(...)` before every
`start()`, so `tours` is default-constructed at each entry to `run()`. **The
"called more than once" guard is therefore dead code** — it tests
`!tours.isEmpty()` on an object allocated three lines earlier.

It is reproduced anyway. This step already writes out six provably-unnecessary
deletes rather than lean on their proofs; leaning on a reachability argument
here would be inconsistent.

`at()` becomes `value()` at three sites. *The first draft gave the §9 reason —
"Qt 2's returned null out of range" — and that is `Q2PtrList::at`, a different
container.* `QGVector::at` (`qgvector.h:85-92`) warns under `CHECK_RANGE` and
then reads out of bounds; the shim warns and **clamps to element 0**. So the
faithful-but-safer choice is `value()`, which yields null. All three sites are
in range, so nothing changes in practice. The four `take` sites keep an explicit
`i < size()` guard so an out-of-range index yields null and touches nothing, as
Qt 2 did — without it the failure mode changes from a null dereference to an
out-of-range write.

**`operator[]`'s clamp went, and D6, D8 and D9 all say to record that.**
`Q2PtrVector::operator[](int)` forwarded to the shim's `at()`, which warned and
clamped an out-of-range or negative index to element 0. `QList::operator[]`
asserts, or is UB under `-DQT_NO_DEBUG`. **13 pre-existing sites change
silently**: `SIG_GPManager.cpp:124, 171, 172, 209, 210, 517, 534, 1342, 1394,
1395, 1447, 1448, 1492`. Latent, not live — `calcInitTourSet` rebuilds
`successor`, `depNumber` and `taskCanDoList` over the post-shrink vector, so
every index is in range — but it is a semantic change on the path this step's
own code runs, and the first draft's mapping table omitted `operator[]`
entirely.

**Warnings fall 313 → 311, both accounted for and nothing new.** Each is the
same `-Wsign-compare` between `int` and `uint` on a `tours.size()` loop bound —
`SIG_GPManager.cpp:1479` and `MT_Classifier.cpp:578` — correct to disappear now
that `size()` is signed. Verified by diffing the normalised warning multiset of
both files before and after: one removal each, zero additions.

`Q2PtrVector` 46 → 43; both `SIG_GPManager.h` and `MT_Classifier.h` dropped the include entirely. **`setAutoDelete` in core is now
0.** Only `SIG_GUIGPManager::individualItems` and `SIG_GPTournament::indis`
still use the type — D26.

**No gate reaches this either.** `MT_Classifier` and `SIG_GPManager` are both
linked into nothing.

### D26 — the last two containers, and the shim has no users left

`SIG_GPTournament::indis` and `SIG_GUIGPManager::individualItems`, both
`Q2PtrVector`, become `QList<T *>`.

**Both are simpler than `tours` for one reason: neither is flag-owning *in this
tree*.** The shim's `insert` deletes the previous occupant only
`if (v.at(i) && del)`, and `del` is false for both. So **none of the nine
`insert`s and three `resize`s has a hidden free to reproduce**, and each
`insert` is a plain slot assignment.

*Two corrections to that reasoning.* **`indis` *was* flag-owning in 1.3** —
`indis.setAutoDelete( true )` in the constructor, confirmed in the binary at
`0x080cf9ee` (`pushw $0x1`). Phase B removed the flag and moved the free to
`deleteContents()`. The outcome is unchanged, because the flag freed nothing at
any converted site: all three `resize`s only grow, and every `insert` lands on a
null slot. But the justification as first written asserted the opposite of what
1.3 did, and the source comment said so too — both corrected.

And the count is **nine**, not eight: 2 + 2 + 4 in the tournament builders plus
`SIG_GUIGPManager.cpp:48`. **The ninth does not follow a `resize`** —
`individualItems` is sized in the member-init list and its insert sits in a
`while` loop, so two items sharing a `poolPosition` would land on a non-null
slot. Harmless, since `del` is false, but the stated reason did not cover it.

| container | ownership | conversion |
|---|---|---|
| `indis` | explicit — `~SIG_GPTournament` calls `deleteContents()` (`:44`), which *is* the whole ownership | `qDeleteAll( indis ); indis.clear();` |
| `individualItems` | **not owning** — the items belong to the list view | assignment only; nothing to free |

The three tournament builders (`SIG_GPSimpleTournament.cpp:38-40`,
`SIG_GPMutationTournament.cpp:38-40`, `SIG_GPCrossOverTournament.cpp:42-46`) all
`resize` then fill, so `indis.insert( i, new … )` becomes `indis[ i ] = new … ;`.
`MT_Classifier.cpp:631`'s `indis.at(k)` becomes `value(k)` for the §9 reason.
`SIG_GUIGPManager.cpp:48` gains a range guard — and the first version of it was
**not** the guard `Q2PtrVector::insert` had. The shim takes `uint i`, so a
negative index wraps huge and is **rejected**; a bare
`poolPosition < individualItems.size()` is `int < qsizetype`, signed, so a
negative **passes** and indexes out of range. Now `>= 0 &&` as well. Reachable
only through `SIG_GPIndividual.cpp:603`, which parses `POOLPOS` from a saved
`.exp` with `toLong()`.

**The read site had lost a clamp too, unrecorded.** `individualItems[ poolPos ]`
went through the shim's `operator[]` → `at()`, which warns and clamps to element
0; `QList::operator[]` out of range is UB. It is now `value( poolPos )` with a
null check, so an out-of-range index crashes cleanly instead of silently reading
the wrong item. *The first version of this step guarded the write and left the
read.*

*And `value(k)` at `MT_Classifier.cpp` is not the shim's `at()` either* — the
shim's `Q2PtrVector::at` **clamps to element 0 and warns**, which is a
deliberate divergence recorded in `q2compat.h`; §9's null-out-of-range row is
`Q2PtrList::at`, a different type. In range they agree and the loop bound
guarantees in range, so nothing changes; the cited reason was wrong.

**Warnings fall 311 → 309**, both the same `-Wsign-compare` between `int` and
`uint` on a `size()` loop bound — `MT_Classifier.cpp:629` and
`SIG_GPManager.cpp:1516` — correct to disappear now that `size()` is signed.

*A warning can move because a **header** changed, so a multiset diff scoped to
the touched `.cpp` files cannot see it.* Re-measured over the whole tree,
`LC_ALL=C`:

| scope | removed | added |
|---|---|---|
| six touched `.cpp`, raw | 3 | 2 |
| six touched `.cpp`, line numbers stripped | 1 | 0 |
| **whole tree, raw** | **4** | **2** |
| whole tree, line numbers stripped | 2 | 0 |

The total and the attribution are right. "0 added" holds only after stripping
line numbers — the raw multiset gains two because `SIG_GPTournament.cpp`'s
`-Wunused-parameter` warnings shift 50→53 and 53→56. *A verification has to
state its scope and its normalisation, or it is not reproducible.*

**`Q2PtrVector` is gone from every container member.** In `src/` and `include/`,
each remaining `Q2*` occurrence is a **comment** explaining what the Qt 2
original did — **20** such lines for `Q2PtrVector`, none a type use. *An earlier
draft said 44; that was the whole-tree count, and 25 of those lines are inside
`compat/`, where they are real type uses.*

**But the shim is NOT deletable, and the claim that it was is false.**
`sigel_eval.cpp:363` and `:401` instantiate `Q2PtrList<int>` — the D18
differential check, which walks the shim's cursor beside the rewritten one. That
file is built by `make` and **is the dictorder and fitness gate binary**.
*The false claim came from grepping `src/` and `include/` and reporting the
result as "all code"; the repository root was never in scope.* Another instance
of the failure tabulated in §9, and **the sharpest form of it**: the scope that
was too narrow happened to exclude *the file the gates run in*. Not merely
missed coverage — the omitted file was the one being certified.

**Three things must happen before the shim can go:**

1. **`sigel_eval.cpp:363,401`** — the two `Q2PtrList<int>` instantiations. The
   test they implement compares the converted cursor against the shim, so it
   cannot outlive the shim; it has to be retired or re-expressed.
2. **`MT_Randomizer.h` and `MT_StatisticsElement.h` need `#include <QList>`.**
   Both declare `QList` members and receive the header **only transitively
   through the shim**. Dropping the shim include from all 19 dead-include sites
   breaks **10** headers standalone (111/1 → 101/11); dropping it from only
   these two breaks 3. *An earlier draft said 8, which counts neither of the
   two headers themselves.* Adding `<QList>` to exactly those two restores
   **111 pass / 1 fail**. So the dead-include count is **19**, not 21 and not 20.
3. ~~**The shim installs deterministic hash seeding as a side effect**~~
   (`q2compat.h:88-91`, a per-TU `Q2DeterministicHashSeed`). **Measured, and it
   is vestigial** — see below. It should still move to `sigel.cpp` /
   `sigel_slave.cpp` as defence when they link, but it is not a correctness
   blocker.

**Why the seeding is vestigial, and how that was established without any
reference.** The worry was sharp: **V1 proved container iteration order is
serialised into the `.exp`** — Material, Link, Joint, Drive and Sensor orders
are all observable in a saved file — so on that path order is *output*, not an
internal detail. Qt 6 randomises `QHash` iteration per process unless seeded, so
losing the seed would not give a wrong-but-stable order; it would give **a
different order on every run of the same binary**.

Two measurements settle it:

| check | result |
|---|---|
| `QHash`/`QSet`/`QMultiHash` anywhere outside `compat/` | **none.** D3 replaced the six `Q2Dict`s with `QList<T *>`, which is insertion-ordered by construction |
| ~~the same binary under `QT_HASH_SEED` = 0, 1, 12345, 999999~~ | **A NULL EXPERIMENT — it could not have failed.** Qt 6 honours `QT_HASH_SEED` **only when it is 0**; any other value prints `forced seed value is not 0; ignored` and is coerced to 0. Confirmed: `QHashSeed::globalSeed()` is 0 for all four. And the shim pins the seed per translation unit before `main`, while `sigel_eval.cpp:451` calls `setDeterministicGlobalSeed()` as its first statement — the environment variable is never consulted |
| **the experiment that should have been run**: rebuild `sigel_eval` with `QHashSeed::resetRandomGlobalSeed()` after the deterministic call, so the seed is genuinely random per process | seeds `278889441371735583`, `12972567734183481017`, `2771707303525248410` — all three dictorder runs **byte-identical to `dictorder-baseline.txt`**. This is real support, and unlike the argument from "no `QHash` in our code" it also covers Qt's own internal hashes |

So no hashed container's order reaches a file, and the seed cannot affect
output. *The shim's own comment already anticipated the move to `main()`; what
it could not know is that D3 would remove every hash the seed protected.*

**Self-consistency before fidelity — the order matters and it is cheap.** The
V4 digests invite starting at the wrong end. The correct sequence:

1. **Run this build twice on the same input and compare it against itself.** No
   reference needed. Done: three consecutive `dictorder-dump.sh` runs and three
   `fitness-check.sh` runs, identical digests each.
2. **Only then compare against the 1.3 digests.** A mismatch at step 2 with a
   randomised seed underneath would look exactly like a fidelity failure and
   send someone hunting through the interpreter and the physics for what is
   actually one missing seed call.

*This is the same discipline that made the V4 captures gates rather than
fingerprints: the reference side validated each digest across two independent
runs before shipping it.*

**No gate reaches any of this** — the D25c section says so and D26's first draft
omitted it. `SIG_GUIGPManager.cpp` is not even syntax-checked: `Makefile:288`
excludes it and it is one of `check.sh`'s four known failures, aborting at
`SIG_Experiment.h:26` on `qwidgetstack.h` before the compiler sees the new
guard. The tournament builders, the destructor and the `MT_Classifier` line are
compiled into archives and linked into nothing — `nm -C` finds no
`SIG_GPTournament` or `MT_Classifier` symbol in `sigel_eval` or `pvm_link`.
**Every gate result in this step is a null result for the change.**

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

**`check.sh` now executes no code at all** — it is `-fsyntax-only` plus the
standalone-header pass. The deleted step was the only one that linked and ran
anything. So **ownership of the evolution-loop containers (`fitTaskList`,
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
