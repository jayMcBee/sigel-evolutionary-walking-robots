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
| B — ownership explicit | **subsumed by Phase D**, which deletes the containers rather than converting them. 13 `setAutoDelete` left in core — 10 in `SIGEL_GP`, 2 in `SIGEL_Robot`, 1 in `MT_Control`, all in the evolution loop, which Phase C still blocks even though PVM now runs |
| R — build and run | core builds and runs. **No longer checked only against itself** — Phase V has confirmed both the ordering and the arithmetic against the 1.3 binary, §7 |
| T — old-Qt tool container | **done 2026-08-27.** `tools/qtmig`, §4 |
| D — delete the shim, migrate the data | **D1–D8 done.** `Q2Dict`, `Q2DictIterator` and `Q2Array` gone from all code; `Q2PtrVector` off `SIG_Geometry`, `SIG_Body` and the `SIG_Register` cluster. Shim 806 → **530** lines. Remaining, measured 2026-08-28: `Q2PtrList` 62, `Q2PtrVector` 53, `Q2CString` 21, `Q2Queue` 16, `Q2ListIterator` 14, `Q2ValueList` 12. §10 |
| P — PVM | **DONE 2026-08-28.** Vendored 3.4.3 replaced by upstream 3.4.6; nine patches carry the four config lines and Debian's eight source fixes; `libpvm3.a` and `pvmd3` build; SIGEL's two PVM objects link against them and `SIG_GPPVMData` round-trips through real PVM. `sigel`/`sigel_slave` still need Phase C. §7 |
| C — GUI | **not started, AUTHORIZED 2026-08-27 per D24.** ~450 Qt 2 sites + 20 forms |
| V — check against the 1.3 binary | **V1 and V5's MDH probe both done and both PASS.** Ordering: 10 of 10 container orders match. Arithmetic: `twoBases` exact bit for bit, `octopus` 9/9 with three joints exact and 5 ulp worst. V2–V4 not started; V5's sensor and force probes are **invalid as specified** — both target Dynamo-only functions, deleted 2026-08-28. §7 |

**SCOPE — DECIDED 2026-08-23. Read this before changing anything.**

**The reference for this port is SIGEL 1.3 and nothing else.** The 1.3 source in
`x/kdesigelSources.1.3/` and the 1.3 binary running on the x86 box, reachable
through the `sigel-x86` Claude session. A port must not change results, so the
target is that our build reproduces what the 1.3 binary does.

**The 14 published experiments cannot check this port.** They were produced in
August 2001 by SIGEL 1.0. Validating a port of 1.3 against them measures every
1.0 → 1.3 change as though it were ours. That mistake cost most of 2026-08-22
and produced a second project by accident.

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

**Order of work, revised 2026-08-27.** Reordered around the goal above: the
interface is the work, so it goes first, and PVM follows because without it the
ported interface has nothing to drive.

1. ~~Build core plus a small program that runs one fitness evaluation, under
   AddressSanitizer.~~ **Done.**
2. ~~**Stand up the old-Qt container**~~ — **done**, `tools/qtmig`. §4.
3. ~~**Get a reference from the 1.3 binary**~~ — **V1 and V5's MDH probe done,
   both pass.** This was item 1's missing half and is no longer outstanding.
4. **Phase D — delete the shim.** *In progress, D1–D8.* The data migration is
   done and only the 7 `.rrb` needed it. Remaining: `Q2PtrList`, the rest of
   `Q2PtrVector`, `Q2CString`, `Q2Queue`, `Q2ListIterator`, `Q2ValueList`, then
   the header. Ordered before C by **D25** so the interface is ported once, to
   the final target. §10.
5. **Phase C — the interface**, one module or one form at a time. §7.
6. Fix PVM — **superseded 2026-08-28.** That count measured the vendored 3.4.3,
   now replaced by upstream 3.4.6: four config lines, no source edits. Phase P.
7. Full headless run, compared against the captured 2003 run.
8. The evolution-loop containers, still untestable — PVM runs as of Phase P, but Phase C blocks the loop (§7).

**A caveat that governs the order of what is left.** Everything after Phase D's
simulation-side work is in the **evolution loop**, which nothing can execute:
the 13 remaining `setAutoDelete` sites, `Q2PtrList`'s `fitTaskList` and
`toSpawnList`, and the rest of `Q2PtrVector`. Both gates and AddressSanitizer
reach none of it. Converting those blind is the largest remaining risk in this
plan. **Phase P was expected to retire it and did not.** PVM builds and runs as
of 2026-08-28, but `SIG_GPFitnessTrainer` dispatches through `pvm_spawn` of
`sigel_slave`, and Phase C has to build `sigel_slave` first. The blocker moved;
it did not lift.

**Still needs a decision:** whether
`QTextStream` no longer printing `-0` matters (§9); the order of remaining
Phase B work.

Seven independent review rounds have run. They found 2 leaks, 1 double free, 1
free lost on the exception path, 9 gaps where the self-check passed on broken
code, and 17 false statements in the code and in this file. Round 6 found a
missing `#include <cstddef>` that `-fpermissive` was hiding, and that `-lGL` is
not optional. Round 7 found a twelfth `QTime()` site, a race in `replicate.sh`
that scored crashed evaluations as zero, and the 2001 date of the published
experiments — which is what produced the scope note above. All fixed.

## 0. Working on this

- Source root: `x/kdesigelSources.1.3/kdesigel/kdesigel/`
- The shim: `include/compat/q2compat.h`; its self-check:
  `include/compat/q2compat_check.cpp`
- Build: `make` at the repo root gives `build/sigel_eval` under ASan and UBSan.
  `make B=build-fast SAN= SIGSAN=` gives an unsanitised build about 15x faster,
  in its own directory.
- Verify: `./check.sh` from the repo root compiles every module and header and
  runs the shim self-check. Takes several minutes.
- PVM: `make pvm && make pvm-link`, then `./pvm-check.sh` starts a daemon and
  runs both round trips. Not one of the three checks below — it has no baseline,
  it is PASS/FAIL.
- Run the published experiments: `./replicate.sh build-fast`. Read the scope
  note at the top of this file first — those experiments are from SIGEL 1.0 and
  do not test this port.
- **After every step, an independent agent reviews the diff with fresh eyes.**
  Not optional — every round so far has found a real defect, and a compile check
  proves nothing about ownership.
- When adding a shim assertion, break the shim and confirm the check aborts.
  Nine assertions have passed on broken code.
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
| `setAutoDelete` / `autoDelete` | 47 | 38 `TRUE`, 9 `FALSE`; 28 remain after B1–B5 |
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
`paintEvent`. The 444 `SIGNAL()`/`SLOT()` macros in 174 `connect()` calls are
all still valid — string-based connect was never removed.

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
- PVM — **HISTORICAL, 2026-08-28. This whole item measured the vendored 3.4.3,
  which Phase P deleted; its line numbers no longer resolve.** Kept because it
  is what the 3.4.6 decision was argued against. Where it cites a line, 3.4.6
  reads: `global.h:321`, not 314; `pvmlog.c:503-504`, not 421-422, and there
  now behind `#ifndef USESTRERROR`, which `conf/LINUX64.def` defines — so that
  defect is gone. The 23 on `rpc/types.h` still holds in 3.4.6. See Phase P.

  All 39 `.c` in `pvm3/src` fail. **Measured 2026-08-27, corrected by
  review. Four causes, partitioning exactly: 23 + 8 + 7 + 1 = 39.**
  - **23** on `rpc/types.h`, which glibc dropped. `libtirpc` is the answer;
    its `-dev` package is not installed here.
  - **8** on errors gcc 14 promoted from warnings — `imalloc`, `lmsg`,
    `nmdclass`, `pkt`, `pvmalloc`, `pvmdabuf`, `pvmfrag`, `pvmlog`.
    `-std=gnu17 -fpermissive` clears all 8.
  - **7** on `global.h:314`, which declares
    `extern struct Pvmtevdid pvmtevdidlist[]` and relies on the includer having
    pulled in `pvmtev.h` first. **5 of the 7 are MPP or shared-memory files in
    neither Linux object list**; only `pvmcruft.c` and `pvmerr.c` matter.
  - **1** — `pvmwin.c` wants Win32's `<process.h>`. In no Unix object list.

  `pvmlog.c:421-422` additionally uses `sys_nerr` / `sys_errlist`, removed in
  glibc 2.32 — a second, real defect inside the 8. A socket build needs 15
  objects for `libpvm3.a` and ~24 for `pvmd3`
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

**This section previously said "No 'start on old Qt' step" and was wrong.** Its
reasoning was that Qt 2.3 and Qt 3.3.8 do not build on gcc 15, and it concluded
that the staged migration was therefore unavailable. That does not follow. Qt 3
and Qt 4 do not have to build on gcc 15 — they have to **run**, once, as
converters. An old toolchain in a container is where they run.

The project already has a precedent: the 2003 i386 binary runs against Debian
woody libraries on the x86 box (§9).

**CORRECTED 2026-08-27 by review.** An earlier draft of this section had a
three-leg path starting `Qt 2.3 --qt20fix--> Qt 3`. **`qt20fix` is not a
Qt 2 → Qt 3 tool.** It is Qt 2's own Qt **1.x** → Qt **2.x** script, and the
proof is vendored in this repo:
`x/supportingLibs/supportingLibs/qt/src/doc/porting.doc` is headed *"Help with
porting from Qt 1.x to Qt 2.x"* and line 156 points at `qt/bin/qt20fix`. The
"20" is "2.0". **Qt never shipped a Qt 2 → Qt 3 converter at all** — Qt 3's
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
| **D2** | Target Qt version | **6.9.2**, Ubuntu `qt6-base-dev`; recorded, not pinned |
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
| **D19a** | The core/GUI boundary | **SETTLED 2026-08-27 by D25: delete the shim from core first, then port the GUI once, straight to clean Qt 6.** Measured, not assumed: `qt3to4` leaves Qt 2's `QList`/`QDict`/`QArray`/`QVector` **completely untouched** — 9 `QList<>` in `SIG_GPParameter.cpp` before, 9 after, 0 `Q3PtrList`. Qt 3 had already renamed `QList` to `QPtrList`, so `q3porting.xml` maps *that*; our Qt 2 spelling reads to it as a Qt 4 class. So the boundary fails loudly at compile time (Qt 6's `QList` is a value list) rather than silently, and the choice is real: spread the shim to 466 more GUI sites and remove it twice, or remove it once, first |
| **D20** | `.ui` handling | **`uic3 -convert` in the container**, straight from the Qt 2 form — measured in Phase T, no Qt 3 uplift needed. Supersedes D5(a). No hand-written form parser |
| **D21** | Interface fidelity | **ported, not redesigned.** The 2003 interface arrives at Qt 6 as itself. A widget with no Qt 6 successor gets the nearest equivalent, recorded here — not a redesign |
| **D22** | The Qt 2 style classes | **`QStyleFactory::create("Fusion")` for the `#else` (Motif) branch.** `QMotifPlusStyle` has no successor in Qt 6; Fusion is the closest it offers. Chosen 2026-08-27 after comparing the two styles Qt 6.9 offers here. **The `#ifdef _WINDOWS` branch keeps Windows** — `QWindowsStyle` is no longer a public class but Qt 6 still creates that style by name, so under D21 its nearest equivalent is `QStyleFactory::create("Windows")`, not Fusion |
| **D23** | Phase C granularity | **one module or one form at a time**, each its own commit, each independently reviewable. No API-wide sweeps across modules |
| **D24** | GUI scope | **Phase C is authorized.** Supersedes D3(b), which scoped the interface out. Named separately because D19–D23 did not carry it and the status table cited a struck-through row |
| **D26** | What the 1.3 binary is asked for | **structure and arithmetic, not fitness equality.** Three tiers, in descending confidence: the container ordering and numbering, which compare exactly (V1, V2); per-individual fitness, which is chaotic across architectures and is therefore a judgement (V4); the non-integrating quantities, which compare exactly but need `gdb` (V5). Bit-exact agreement on an integrated trajectory is **not** a target and its absence proves nothing — §7. Recorded because this file repeatedly described the missing reference as "fitness numbers", which is the one thing that binary cannot usefully give |
| **D25** | What "done" means | **Plain modern Qt 6, nothing left over.** `q2compat.h` deleted, no Qt3Support class anywhere, no compatibility flag on SIGEL's own code. This moves §10's "drop the Qt 2 emulation" from optional debt into a **required phase**, and with it the data migration that section describes — the shim exists because `Q2Dict`'s hash order numbers the links, so the 7 `.rrb` and 12 `.exp` files must be rewritten before it can go. **Ordered before Phase C**, so the 466 GUI sites are ported once, to the final target, instead of twice. Vendored third-party code is out of scope for this rule: qhull, cv97, Dynamo and PVM keep `-w -fpermissive` |

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

**Exit criterion per step:** `./check.sh` at the repo root — **105 pass, 4 fail,
322 warnings** as of 2026-08-28. The 4 failures are exactly the files the
Makefile excludes. It was 118/4/338 until the Dynamo backend was deleted
(`physics_backends.md`); the pass count and "headers standalone" each fall by
exactly 13, one per deleted file pair, and the failing files are unchanged.

Two gates run alongside it, both committed and both required to stay empty: `./dictorder-dump.sh | diff -u dictorder-baseline.txt -` and
`./fitness-check.sh | diff -u fitness-baseline.txt -`, the second of which runs
`sigel_eval -selfcheck` first. It compiles every
converted module, compiles every converted header standalone, and builds and
runs the shim self-check under ASan and UBSan. Vendored headers are `-isystem`,
so their ~12,979 warnings do not bury the ~373 in our own code.

**Warnings count.** They were not read up to A8, and the `Qt::endl`-on-
`std::cerr` regression in A3 was reported by this very command at the step that
introduced it, then shipped as "0 errors".

**`check.sh` does not yet cover Phase C.** Its module list is the 9 core
modules, and it compiles nothing under `src/` at top level — so
`sigel.cpp`, `sigel_slave.cpp` and all 5 GUI modules are checked by nothing
today. Extending it is part of the first Phase C step, not an afterthought.
The repo-root programs are in the same hole: `sigel_eval.cpp`, `pvm_smoke.c`
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

Found by P4 once it ran SIGEL's own code. `SIG_GPPVMData.cpp` sent
`finalLength = str.length() + 1` — a **character** count — then sent
`str.toUtf8()`, up to four times longer in bytes. `getQStringFromPVM` sized its
receive buffer from that count and let `pvm_upkstr` write the bytes into it.
Measured under ASan: 20 `ü` gave `heap-buffer-overflow ... in byteupk`; short
strings survived only because `QList` over-allocates.

**Upstream's, not a port regression.** `v1.3-pristine` has the identical defect
with `str.utf8()` and `QArray<char>`. Qt 2's `QString::length()` was the Latin-1
byte count, so the 2003 code was right for its own data.

**Fixed 2026-08-28** — `qCStringBuffer.size() + 1`, taken from the same
`Q2CString` the call already builds, so no extra conversion and no temporary.
`pvm_link.cpp` round-trips 200 `ü` plus 50 `€` (250 characters, 550 bytes) as
the regression test; reverting the line reproduces the overflow and fails the
check. This changes the wire format for non-ASCII, which is safe only because
both ends are this same file and no distributed run exists yet.

---

#### Phase P research — 2026-08-28, superseded by the record above

Kept because the decision it argued for is the one that was taken, and because
two of its claims turned out to be wrong.

**Decision: upstream PVM 3.4.6, four config lines, no source edits.**

```
conf/LINUX64.def   ARCHCFLAGS += -I/usr/include/tirpc -std=gnu17 \
                                 -Wno-implicit-function-declaration -Wno-implicit-int
                   ARCHDLIB   = -ltirpc
                   ARCHLIB    = -ltirpc
lib/pvmgetarch     Linux,aarch64 )  ARCH=LINUX64 ;;
```

**The flag choice matters, and this reasoning holds.** `getcwd` is used without
its header, so its return truncates to `int` on a 64-bit machine.
`-fpermissive` would also demote `int-conversion`,
`incompatible-pointer-types` and `return-mismatch` — the gcc-14 error classes
that catch exactly that. The two targeted `-Wno-` flags silence only the K&R-era
classes and leave truncation a hard error.

**Wrong: "one upstream patch line."** Eight Debian source patches touch what we
compile; all eight are applied. See P2.

**Wrong: "verified by building and running it."** That build did not carry patch
24, so it still had the `getcwd` truncation the research itself identified.

**Also stated there:** vendored 3.4.3 would work with two patches Fedora shipped
before retiring the package in 2015. 3.4.6 contains one of the two — see P2.
No distribution maintains PVM: Fedora retired 2015, Debian removed 2024. An AUR
`PKGBUILD` for 3.4.6 is current at `pkgrel=10`; not an upstream, but not nothing.

---

### Handover — one owner at a time

**Never two sessions on this repository at once.** On 2026-08-27 three sessions
were commissioning the x86 reference box, two of them describing themselves in
identical words, and one was this session under a display name it could not see.
It cost hours and nearly corrupted a reference capture. Sequential sessions are
fine; concurrent ones are not.

This document is the handover. A new session should read §0, this section, and
the phase it is taking on.

**The `sigel-x86` channel** reaches the machine holding the 1.3 reference binary
and a working PVM. **That PVM is no longer the only one** — this machine has had
one since Phase P — so the channel's value is now the 1.3 binary alone. It is currently owned by the Qt 6 session. If it transfers,
identify yourself to it by **verifiable facts** — repo path, recent commit
hashes, a reference file you authored — never by a session name, because names
are assigned per side and neither end sees the other's.

**Gates any session must keep green**, all committed:

```
./check.sh                                            105 pass, 4 fail
./dictorder-dump.sh | diff -u dictorder-baseline.txt -    empty
./fitness-check.sh  | diff -u fitness-baseline.txt -      empty
```

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

**5 files still fail**, all needing a `QApplication`: `MT_Controller.cpp`,
`SIG_GPPopulation.cpp`, `SIG_GUIGPManager.cpp`, and both ZORC fitness files.
Blocked on Phase C.

### Phase B — make ownership explicit

Phase B does **not** delete `q2compat.h`. Three of its behaviours are
load-bearing until the data migration in §10: the `Q2Dict` hash order that
numbers the links, the `Q2PtrVector` `size()`/`count()` split with null slots,
and `insert()`/shrinking `resize()` being the only free path at 8 sites.

Goal: every owning container frees its items explicitly at the owner, via
`deleteContents()`, and `setAutoDelete` disappears container by container.

**8 of 14 done. Five containers are an open decision:**

| container | file | why left |
|---|---|---|
| `pool` | `SIG_GPPopulation.cpp:37,50,96,116` | 9 sites; `insert()` frees the losing individual |
| `pvmTasks` | `SIG_GPFitnessTrainer.cpp:48` | 7 sites; `insert(id, 0)` frees a slot |
| `pvmHosts` | `SIG_GPFitnessTrainer.cpp:49` | 5 sites |
| `toSpawnList` | `SIG_GPFitnessTrainer.cpp:47` | `remove()` at `:489` is the free |
| `tours` | `SIG_GPManager.cpp:63` | 7 sites, compacted by `MT_Classifier` across a module boundary |

28 hand-written frees in the core GP loop, each compiling whether right or
wrong, with no run to check against. Every review round has found a real defect
in the *easy* conversions.

**`fitTaskList` (`SIG_GPManager.cpp:357,1434`) keeps `setAutoDelete`
permanently.** On a **local** container, letting the container delete its own
items is the right answer: it frees at scope exit, including
the early `return` at `:404` and anything thrown out of `checkTask`. Writing
those frees by hand loses the unwinding path. `SIG_Body.cpp`'s local `vertices`
was converted in B2 before this was understood, and uses `setAutoDelete` again.

**Exit criterion per step:** the self-check builds and runs clean under ASan and
UBSan, with an assertion covering the free path each converted container
actually uses — owner frees exactly once and is idempotent, non-owner frees
nothing. Verified to have teeth: every assertion added has been checked by
breaking the shim and confirming the check aborts.

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

**The defect that stood between building and running.** Qt 2's `QTime()`
was 00:00:00.000 and valid. Qt 6's is null: `addSecs` returns another null
`QTime`, `secsTo` returns 0, and a null `QTime` holds -1 ms, which is less than
every real time. So `SIG_DynaMechsSimulationQueries::getActualSimulationTime`
returned null, `SIG_Simulation::start`'s `while (act < max)` never ended, and
`fitness = distance / simulatedSeconds` was a division by zero. **12 sites**,
all now `QTime( 0, 0 )` — 9 fitness functions, the queries object and one
`SIGEL_SlaveGUI` signal, plus a twelfth found later by review —
`SIG_EarlyRunTermSimulation.cpp:97` declared `QTime zeroHour;`, which the first
sweep's pattern missed because it is a declaration rather than a call. This is
the shape §9 warns about: same API, same compile, different behaviour.

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
references GLU.

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

**AddressSanitizer.** Clean, but only with one `SIGEL_ROOT` per worker.
`SIG_Environment::generateTerrain` rewrites `$SIGEL_ROOT/Terrain.ter` on **every
evaluation** and reads it straight back, so workers sharing a root read it
half-written, get a zero-size grid, and take a real heap-buffer-overflow in
`dmEnvironment::getGroundElevation`. `replicate.sh` gives each worker its own
root and treats a non-zero exit as an error.

**Leak baseline (D18): 41,254 bytes in 109 allocations** per evaluation, from
§10's pre-existing leak — `SIG_Simulation` is `new`ed and never deleted, and its
destructor is empty. Gate on ASan and UBSan errors, not on this.

**Verified faithful to Qt 2 by independent audit**, each by measurement rather
than inspection:

| | |
|---|---|
| `Q2Dict` hash order | the shim reproduces Qt 2's ELF hash, seed, shift, mask and ascending bucket walk exactly; link and joint order checked against an independent model of Qt 2's table for all 7 robots |
| `SIG_Randomizer` | identical sequence — the LCG's extracted bits 16..30 are unaffected by `unsigned long` widening |
| `QTextStream` double formatting | byte-identical to Qt 2's `%.6lg` over 200,000 random bit patterns, except `-0`, which appears in no shipped robot stream |
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
`reference/`, and every later step diffs against it locally. The x86 box is
needed **once per quantity, not once per step** — after V1 this is another line
in `check.sh`, not a remote call.

| # | Step | What it checks |
|---|---|---|
| V1 | ~~Capture 1.3's load-and-save round trip for three shipped `.exp`~~ **DONE 2026-08-27** — `reference/v1-1.3-roundtrip.txt` | the `Q2Dict` hash, all order-carrying containers, the parser and the serialiser |
| V2 | Our half: a save path in `sigel_eval`, the same round trip locally, diffed against V1. Becomes a gate | equivalence instead of self-consistency |
| V3 | Determinism on the x86 box — one experiment run twice, both `RANDOMSEED`s pinned | gates everything numeric; never tested there |
| V4 | Force re-evaluation of a shipped population by setting its `FITNESS` fields to `-1`, harvest 1.3's per-individual fitness, compare against `sigel_eval` | the number this file has been asking for. **Judgement, not a gate** |
| V5 | **MDH probe DONE 2026-08-27, PASS** — `reference/v5-1.3-mdh-compared.txt`. The sensor and force probes remain open | the port's **arithmetic**, which V1–V4 never touch |

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

Build-time only. Nothing ships from it and nothing links against it.

**Debian jessie is the last release that packages Qt 4's migration tools *and*
has an arm64 port**, so this runs natively — no qemu, no x86 emulation, which
is not registered on this host anyway. `libqt4-dev-bin` carries `uic3` and
`qt3to4`; `libqt4-dev` carries `/usr/share/qt4/q3porting.xml`, the class-rename
rules. Without `QTDIR` pointing at it, `uic3` warns once and then leaves every
widget class unmapped. `/usr/bin/uic3` is a qtchooser stub — the image puts
`/usr/lib/aarch64-linux-gnu/qt4/bin` first on `PATH` to skip it.

**T2 result — `uic3 -convert` reads the Qt 2 forms directly.** All 20 convert,
0 failures, output `version="4.0"`, which Qt 6's `uic` accepts.

**"No warnings" was wrong** — corrected by review. `uic3` writes the form to
stdout and warnings to stderr; the first measurement kept only exit codes. There
are **28 warnings across 7 forms**. Redirecting stderr into the same file puts a
warning on line 1 and the output stops being XML.

**This corrects a false claim in the previous C7 paragraph**, which said the
files were "one generation below what `uic3 -convert` accepts" and concluded a
Qt 2 → Qt 3 leg was needed first. Measured 2026-08-27: it is not, for forms.
Whether `qt3to4` needs one for *sources* is untested and is the next question.

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
| 4 real size constraints, silently dropped | `QLayoutWidget` → `<layout>` discards them: `MT_IndividualWidgetBase` `Layout32`/`Layout33`/`Layout28` lose `maximumSize 130×32767`, `MT_PopulationWidgetBase` `Layout60` loses `minimumSize 200×0` |
| 11 widgets renamed by Qt 6's `uic` | duplicate names — `tab`→`tab1`… in 5 forms. Breaks any hand-written subclass referring to them |

Verified preserved: tab order 143/143, combo and list box items 47/47, list view
columns 15/15, layout margins and spacing 196/196, and all seven property
renames. No buddies exist in the source.

The 6 widget classes, across all 20 forms:

| class | uses | Qt 6 |
|---|---|---|
| `Q3GroupBox` | 48 | `QGroupBox` |
| `Q3ListBox` | 7 | `QListWidget` |
| `Q3ButtonGroup` | 7 | `QGroupBox` + a `QButtonGroup` for the exclusivity |
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

**Measured 2026-08-27** over the 5 GUI modules, 117 files, Latin-1 safe:
**466 Qt 2 sites.**

| category | sites | |
|---|---|---|
| `QListView` / `QListViewItem` | 128 | D9 → `QTreeWidget` |
| containers — `QArray` `QDict` `QVector` `QList` `QCString` `QValueList` `QListIterator` | 75 | **owned by no step until now** |
| `insertItem` | 66 | menus, combos, list boxes |
| `WFlags` / `WType_*` / `WStyle_*` | 63 | → `Qt::WindowFlags`, `Qt::WA_*` |
| `QString::null` | 42 | → `QString()` |
| `QPopupMenu` | 40 | → `QMenu` |
| `qApp` / `setCaption` / `QApplication` | 22 | |
| `setAutoDelete` / `autoDelete` | 17 | **owned by no step until now** |
| `QGLWidget` | 6 | → `QOpenGLWidget` |
| `sprintf` 4, string methods 3 | 7 | |
| `QMultiLineEdit` | **0** | all 6 are in `.ui` files, not in module code |

**The 92 container and `autoDelete` sites are the largest thing this plan has
been missing.** Phase B covers the 14 *core* containers; D6/D7 were decided for
core *with the shim*, which D19 does not extend to the interface. They are now
part of each module's step below, and D19a has to be settled first.

**Steps — one module or one form each, per D23.** Forms come first because the
modules include the headers `uic` generates from them.

| # | Work | Size |
|---|---|---|
| C1 | One form end to end — settles the residue table above, including the dropped slot declarations and the embedded images | 1 form |
| C2 | The remaining 19 forms | 19 forms |
| C3 | `SIGEL_CommonGUI` | 665 LOC, 2 files |
| C4 | `SIGEL_SlaveGUI` | 2,145 LOC, 5 files |
| C5 | `SIGEL_Visualisation` — carries all 6 `QGLWidget` sites | 3,544 LOC, 12 files |
| C6 | `MT_GUI` | 3,911 LOC, 14 files |
| C7 | `SIGEL_MasterGUI` | 7,717 LOC, 20 files |
| C8 | `sigel.cpp`, `sigel_slave.cpp` | 15 sites |
| C9 | All five modules and both programs build, link and run | — |

Each module step is the same shape: `qt3to4` in the container, hand-port off
Qt3Support, extend `check.sh` to cover the module, commit. Smallest first, which
also happens to respect the dependency order.

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

---

## 8. Steps and status

| Phase | Steps | Status |
|---|---|---|
| A | 10 | done |
| B | 5 | 8 of 14 containers |
| T | 2 | **done 2026-08-27** (§4) |
| C | 10 | **not started, authorized 2026-08-27** |
| V | 5 | **V1 done 2026-08-27**, V5 in progress — §7 |
| P | 4 | **done 2026-08-28** — §7 |

**The effort column is gone, 2026-08-27, and the section is no longer called
Effort.** It carried "1.5 wk", "1 wk", "2–3 days", "2.5–3 wk" and "~3 days".
None of those was measured or derived from anything — they were invented. A plan
whose every other number is counted from the tree should not carry six that are
guessed. Step counts are real and stay. **Do not put estimates back.** The same
row also claimed Phase T was "not started" while the status table at the top of
this file had it done; corrected here.

Phase T is new as of 2026-08-27 (§4). Phase C was rebuilt around modules and
forms. The PVM row was previously described as a separate job "which nothing
here depends on" — that is false and is corrected.

**Precisely, because an earlier draft overstated this.** A single fitness
evaluation *does* run locally with no PVM — that is what `sigel_eval` and all
of Phase R do. What has no local path is the **evolution loop**:
`SIG_GPFitnessTrainer` dispatches every evaluation through `pvm_spawn` of
`sigel_slave` with no in-process fallback, and the one method that looks like
one, `SIG_GPExperiment::calculateFitness` (`SIG_GPExperiment.cpp:158`), is a
stub that returns 0. So without PVM the ported interface builds and shows its
windows, and nothing happens behind the Start button.

---

## 9. Open

### Phase B ownership audit (D11)

| | |
|---|---|
| `setAutoDelete` calls | 47 — 38 `TRUE`, 9 `FALSE` |
| after B1–B5 | 28 — 21 `TRUE`, 7 `FALSE`; 12 in core |
| in deferred GUI modules | 16, which Phase B cannot touch |
| distinct owning containers in scope | 22 |
| pointer containers owning with **no flag at all** | 21 — D7 says nothing about these |

**Three things that cause a double free or a leak:**

1. **The free is hidden inside a container operation.** Eight `Q2PtrVector`
   sites where `insert()` or a shrinking `resize()` *is* the only delete and the
   word `delete` appears nowhere: `SIG_GPPopulation.cpp:168` (frees the losing
   individual, reached from 12 call sites), `:275`, `:278`, `:346`, `:417`, and
   `SIG_GPFitnessTrainer.cpp:194`, `:355`, `:378`.
   Three `Q2PtrList` sites belong here too: `SIG_GPManager.cpp:437`, `:1519`
   (`fitTaskList`) and `SIG_GPFitnessTrainer.cpp:489` (`toSpawnList`).
2. **Owning containers with no free path**, relying on `~Q2PtrList` /
   `~Q2PtrVector`. Fixed for the 8 converted; `~SIG_GPFitnessTrainer` still
   leaves `pvmHosts`, `pvmTasks` and `toSpawnList` to `setAutoDelete`.
3. **`SIG_Robot::clear()`** hand-deletes six dictionaries' contents and calls
   `clear()` on them twelve lines later. Safe only because those dicts carry no
   flag. The self-check now asserts that `clear()` on a non-owning container
   frees nothing.

**A `getFoo()` returning a container by reference puts free sites in other
modules**, including modules that do not compile yet. Grep the accessor, not
just the member name. `SIG_GPParameter::getHostList()` is how B3 leaked into
`SIGEL_MasterGUI`.

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

`q2compat.h` calls `QHashSeed::setDeterministicGlobalSeed()` during static
initialisation. `main()` should call it explicitly too once `sigel.cpp` and
`sigel_slave.cpp` are ported — static initialisation order is unspecified.

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
| `SIG_GPPVMData.cpp:51` `sendQStringToPVM` | sends `str.length() + 1`, a **character** count, then sends `str.toUtf8()`, up to 4x longer in bytes | `qCStringBuffer.size() + 1` | `getQStringFromPVM` sizes its receive buffer from that count and lets `pvm_upkstr` write the bytes in. 20 `ü` gives `heap-buffer-overflow ... in byteupk` under ASan; short strings survive only because `QList` over-allocates. Qt 2's `length()` was the Latin-1 byte count, so 2003 was right for its own data. **Changes the wire format for non-ASCII** — safe only because both ends are this file and no distributed run exists. Found by Phase P's P4, regression-tested by `pvm_link.cpp` |
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

**The first version of D1 was blind, and review caught it.** It dumped links and
joints only. `SIG_Robot` holds **six** `Q2Dict`s (`SIG_Robot.h:60-65`) — bodies,
materials, links, joints, drives, sensors — all six written in iteration order
by `writeToFileTransfer` and read back in that order by the simulation-data
class — cited here as `SIG_DynaMoSimulationData` until 2026-08-28, which was
the **Dynamo** one and is deleted; the live site is
`SIG_DynaMechsSimulationData`.
`SIG_Link::points` is a seventh, one per link. Rebuilding the core with
`h % vlen` perturbed to `(h + 1u) % vlen` changed the ordering in 8 of 14
experiments and the old gate fired on **2**. It now fires on all 8, plus 6 of
the 7 `.rrb`. `twoBases` does not move, and should not: 2 links, 1 joint.

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

**Narrower than it first looks, and the first draft overstated it.** `hash ∘
hash` is not self-inverse in general — it is a stable sort by bucket, and is the
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

The first draft of this table said "`rrb` 0 of 7" and "`loaded` 7 of 14" without
qualification. Both were wrong as written: 6 of 7 `rrb` blocks changed in the
free dicts, and `loaded` moved for 8 blocks, not 7.

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

`allowedCommands` is the **eighth** order-carrying dictionary, and the earlier
"six dicts plus points is seven" missed it — found by review. Its order rides
inside every `.exp` and every PVM transfer through `writeToFileTransfer`.

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

**The eighth container is in the gate now.** `allowedCommands` order is dumped
alongside the other seven — 469 lines added to the baseline. Nothing *numbers*
commands (they are looked up by name), so like bodies and materials its order
reaches only the serialised bytes.

**`check.sh` was reporting a failure the real build does not have.** Its include
path stopped at `QtCore` while the Makefile adds `QtGui` and `QtWidgets`, so
`SIG_GPPopulation.cpp` failed on `<QApplication>`. Fixed: **118 pass, 4 fail**,
and the 4 are exactly the files the Makefile excludes. The long-quoted
"117 pass, 5 fail" baseline was always one part harness artifact.

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

**The first version of this paragraph justified that with two false claims**,
both corrected by review. There is no `addVertex` — the appending method is
`getOrAddVertex`, it is **public**, and it *is* called from outside
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
not *the* one, which is `fitTaskList` (`SIG_GPManager.cpp:357,1434`) and is
untouched. On a local the flag *is* the RAII, because the NEWMAT multiply and
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
   D3–D6 done.** `Q2Dict`, `Q2DictIterator` and `Q2Array` are deleted; their
   users are plain `QList`. Left, measured 2026-08-27: `Q2PtrVector` 69,
   `Q2PtrList` 62, `Q2CString` 21, `Q2Queue` 16, `Q2ListIterator` 14,
   `Q2ValueList` 12. An earlier version of this list omitted the last two.
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

**The byte figure is not 8 registers, and an earlier draft implied it was.**
Corrected by review, from the leak records: of the 120 bytes, only **64** are a
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
