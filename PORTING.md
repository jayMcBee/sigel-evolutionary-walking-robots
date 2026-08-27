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

**Status — 2026-08-27**

| phase | state |
|---|---|
| 0 — comments to English | done for the 9 core modules; 9 GUI files still hold Latin-1 |
| A — core onto Qt 6 | **done**, tags `step-A0`…`step-A9`. `./check.sh`: 117 pass, 5 fail (all need a GUI) |
| B — ownership explicit | **8 of 14 containers**. 5 still on `setAutoDelete` — open, §7 |
| R — build and run | core builds and runs, faithful to 1.3. **No way to check it yet** — needs fitness numbers from the 1.3 binary on the x86 box, §7 |
| T — old-Qt tool container | **done 2026-08-27.** `tools/qtmig`, §4 |
| D — delete the shim, migrate the data | **D1–D6 done 2026-08-27. `Q2Dict`, `Q2DictIterator` and `Q2Array` deleted** — shim 806 → 546 lines, `./check.sh` 117 pass / 5 fail / 337 warnings. Remaining: `Q2PtrVector` 68, `Q2PtrList` 61, `Q2Queue` 16, `Q2ListIterator` 14. §10 |
| C — GUI | **not started, AUTHORIZED 2026-08-27 per D24.** ~450 Qt 2 sites + 20 forms |

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
   AddressSanitizer.~~ **Built and running**, faithful to 1.3. Validating it
   needs reference numbers from the 1.3 binary — §7.
2. ~~**Stand up the old-Qt container**~~ — **done**, `tools/qtmig`. §4.
3. **Phase D — delete the shim.** Migrate the 7 `.rrb` and 14 `.exp` so
   declaration order *is* simulation order, drop `q2compat.h`, put core on plain
   Qt 6 containers per D6. Ordered here by **D25** so Phase C ports the
   interface once, to the final target. §10.
4. **Phase C — the interface**, one module or one form at a time. §7.
5. Fix PVM — 23 of 39 files fail because glibc dropped `rpc/types.h`, the rest
   on gcc 14's promoted C errors and two real defects. §3.
6. Full headless run, compared against the captured 2003 run.
7. Convert the last 5 containers, now testable (§7).

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
- Run the published experiments: `./replicate.sh build-fast`. Read the scope
  note at the top of this file first — those experiments are from SIGEL 1.0 and
  do not test this port.
- **After every step, an independent agent reviews the diff with fresh eyes.**
  Not optional — every round so far has found a real defect, and a compile check
  proves nothing about ownership.
- When adding a shim assertion, break the shim and confirm the check aborts.
  Nine assertions have passed on broken code.
- Sibling docs, both independent of this port: `future_refactorings.md` (C++
  language level) and `physics_backends.md` (whether to delete the Dynamo path).
  Do not mix their commits with this work. Third sibling:
  `regression_1.0_to_1.3.md`, deferred until the port is done.

---

## 1. Repo

```
/home/jan/Downloads/sigel/
├── PORTING.md                              this file
├── check.sh                                per-file compile check, §7
├── replicate.sh                            runs the published experiments, §7
├── Makefile                                the build, §7 Phase R
├── sigel_eval.cpp                          one fitness evaluation, §7 Phase R
├── future_refactorings.md                  sibling doc, independent of the port
├── physics_backends.md                     sibling doc, independent of the port
├── regression_1.0_to_1.3.md                sibling doc, DEFERRED
├── patches/                                5 patches to the vendored tree
├── shim/                                   pre-standard C++ headers
├── build/                                  untracked, `make clean` removes it
├── data/                                   untracked, 7 robots and 14 experiments
├── kdesigelSources.1.3.tar.gz              upstream source (2003-04-30)
├── supportingLibs.tar.gz                   vendored deps
├── kbin.tar.gz                             2003 i386 binary, reference only
├── x/kdesigelSources.1.3/kdesigel/kdesigel/
│   ├── src/       19 module dirs           ~40k LOC
│   ├── include/   16 module dirs           ~25k LOC
│   └── ui/        20 .ui files             Qt 2 Designer format
├── x/supportingLibs/supportingLibs/        Qt 2.3, dynamechs, SOLID, cv97,
│                                           newmat09, qhull, fparser, Dynamo, pvm3
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
| `Q_OBJECT` in core | 4 — `SIG_Simulation`, `SIG_DynaSystem`, `MT_GPManager`, `MT_Controller` |
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
- PVM — all 39 `.c` in `pvm3/src` fail. **Measured 2026-08-27, corrected by
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
| **D25** | What "done" means | **Plain modern Qt 6, nothing left over.** `q2compat.h` deleted, no Qt3Support class anywhere, no compatibility flag on SIGEL's own code. This moves §10's "drop the Qt 2 emulation" from optional debt into a **required phase**, and with it the data migration that section describes — the shim exists because `Q2Dict`'s hash order numbers the links, so the 7 `.rrb` and 12 `.exp` files must be rewritten before it can go. **Ordered before Phase C**, so the 466 GUI sites are ported once, to the final target, instead of twice. Vendored third-party code is out of scope for this rule: qhull, cv97, Dynamo and PVM keep `-w -fpermissive` |

## 5a. Decisions — signed off 2026-08-22, for Phase R

| # | Decision | Answer |
|---|---|---|
| **D15** | Where the robot models come from | downloaded from `sigel.sourceforge.net`, §9. Untracked, in `data/` |
| **D16** | The Dynamo branch in `SIG_Simulation.cpp` | **build Dynamo, SOLID and qhull**. No source change, so `physics_backends.md` stays a separate decision |
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

---

## 7. Steps

**Exit criterion per step:** `./check.sh` at the repo root. It compiles every
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

`make` at the repo root builds 251 vendored objects, 118 of SIGEL's 122 core
sources, three moc outputs and `build/sigel_eval`. 93 s from clean at `-j4`.
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
| `Dynamo/Src/Inc/containerlist.h` | `NULL` with no `#include <cstddef>`. `-fpermissive` was hiding this, which is why it is a patch and not a flag |
| `SOLID-2.0/include/3D/Basic.h:40,43` | `INFINITY` is a C99 macro from `<math.h>`; `abs(double)` is now declared in the global namespace, so SOLID's own conflicts with it |

Two shim headers went with them: `new.h` is new (5 SOLID sources include it),
and `iomanip.h` now includes `<iostream>`, which the pre-standard header did.

**OpenGL is on the link line and is never called.** `dmLink::draw()` is pure
virtual and every override lives in `gldraw.cpp`, so every `dm*` vtable
references it and the linker pulls it in. `-lGL`, no `-lGLU` — nothing
references GLU.

**Sanitizer split.** SIGEL's own code gets `-Wall -Wextra`, no `-fpermissive`
and the full AddressSanitizer plus UndefinedBehaviorSanitizer. The vendored
libraries get `-w -fpermissive` and UndefinedBehaviorSanitizer minus three
checks they trip by construction: alignment and signed overflow throughout
qhull and the f2c translation of `ssvdc`, and `vptr` in cv97.

**AddressSanitizer.** Clean, but only with one `SIGEL_ROOT` per worker.
`SIG_Environment::generateTerrain` rewrites `$SIGEL_ROOT/Terrain.ter` on **every
evaluation** and reads it straight back, so workers sharing a root read it
half-written, get a zero-size grid, and take a real heap-buffer-overflow in
`dmEnvironment::getGroundElevation`. `replicate.sh` gives each worker its own
root and treats a non-zero exit as an error; an earlier version scored crashes
as a fitness of 0, which made the headline number load-dependent.

**Leak baseline (D18): 41,374 bytes in 117 allocations** per evaluation, from
§10's pre-existing leak — `SIG_Simulation` is `new`ed and never deleted, and
its destructor is empty. Gate on ASan and UBSan errors, not on this.

### Replication — nothing to compare against yet

`./replicate.sh` runs every individual of every published experiment. It is
**not currently a test of this port**, because the 14 published `.exp` files
were produced in August 2001 by SIGEL 1.0 and the source being ported is 1.3.
See the scope note at the top and `regression_1.0_to_1.3.md`.

Against those 2001 files the current 1.3-faithful build gets 7 of 13 distinct
experiments within 10%. That number measures the 1.0 → 1.3 regression, not us.

**What is needed to make this a port test:** the fitness the 1.3 binary on the
x86 box computes for a set of individuals of one published `.exp`. Then
`replicate.sh` compares our build against 1.3 instead of against 1.0, which is
the only comparison that says anything about the port. Everything else is in
place — the harness, the data and the driver.

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

**Leak baseline (D18): 41,374 bytes in 117 allocations** per evaluation, from
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

## 8. Effort

| Phase | Steps | Effort | Status |
|---|---|---|---|
| A | 10 | 1.5 wk | done |
| B | 5 | 1 wk | 8 of 14 containers |
| T | 2 | 2–3 days | **not started** — blocks C |
| C | 10 | 2.5–3 wk | **not started, authorized 2026-08-27** |
| PVM | — | ~3 days | not started, §3 |

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
| `SIG_DynaSystem.cpp:266-268` | deletes `dynaJoints[k]` while looping to `dynaDrives.size()` | to be fixed | The two vectors grow independently |
| `SIG_EarlyRunTermSimulation.cpp:97` | `QTime zeroHour;` | `QTime( 0, 0 )` | Same class as the other 11 `QTime()` sites but a declaration, so the first sweep's pattern missed it. `getMaxRecorderSteps` returned 2 instead of 182 — a factor of 91 on the denominator of three fitness functions. No shipped experiment selects them, so `replicate.sh` cannot see it |
| `sigel_slave`, `getenv("SIGEL_ROOT")` | dereferenced unchecked | to be fixed | Segfaults if unset; the SIGSEGV handler masks it with no core. Bites under PVM specifically — spawned tasks inherit *pvmd's* environment, not the master's |
| `SIG_Environment` terrain load | `getenv("SIGEL_ROOT")` unchecked | already checked, message on stderr | `sigel_eval` says "SIGEL_ROOT is not set, cannot locate Terrain.ter" instead of reading `/Terrain.ter` |

**Open, from the R1 review:** SOLID is built without the `-DNDEBUG` its own
`Make-config` sets, so eight `assert(!eqz(x))` guards ahead of a division are
live that were not in 2003. Right under a sanitizer, but it can abort where the
2003 binary divided by nearly zero. Not yet decided.

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
by `writeToFileTransfer` and read back in that order by
`SIG_DynaMoSimulationData`, which is the site that numbers the DynaMechs bodies.
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

**Only four of the six dicts are numbered.** `SIG_DynaMoSimulationData.cpp:33-55`
walks, in this order, **links → joints → sensors → drives**, calling
`dynaSystem.newLink/newJoint/newSensor/newDrive`. Those four orders are the ones
a migration must reproduce exactly.

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
`SIG_ProgramLine.cpp:215` drops the out-of-range write, `SIG_DynaSystem.cpp:266`
deletes the right vector. And the clamp **fired in none of the 42 evaluations**;
`Q2Array::at: index … out of range` appears nowhere in their output.

The residual risk is stated plainly: an out-of-range index is now undefined
rather than silently wrong, which AddressSanitizer catches in `build/` but a
release build would not. Nothing in the evaluation path reaches it. The
evolution loop cannot be exercised until PVM builds.

### What review found in D4–D6, and what it changed

**A silent semantic flip, now fixed.** `Q2Dict::find()` returned the **newest**
binding for a duplicate key. D4's six replacements —
`SIG_Robot::lookupBody/Material/Link/Joint/Drive/Sensor` — scanned forward and
returned the **first**, and the D5 commit claimed all lookups scanned backwards
when only the two in `SIG_LanguageParameters` did. All eight now scan backwards.
No shipped robot has a duplicate name (0 across all 438 name-groups), so nothing
in the data could have caught it.

**Which is exactly why there is now a check that can.** `sigel_eval -selfcheck`
asserts newest-wins for `SIG_LanguageParameters`, `SIG_Link::points` and
`SIG_Robot`'s lookups, and that `removeCommand` frees the newest.
`fitness-check.sh` runs it before the evaluations. **Verified to have teeth:**
flipping `lookupLink` back to first-wins makes it fail while
`dictorder-baseline.txt` and `fitness-baseline.txt` both stay empty.

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
AddressSanitizer, so `.rrb` loading has no sanitized coverage at all, and
neither gate can run against `build/` for that reason plus the known leak.
Vendored code, out of scope for the Qt port, but it is why the ASan claims in
D4–D6 mean "the 14 `.exp`, by hand, with leak detection off".

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
   users are plain `QList`. Left: `Q2PtrVector`, `Q2PtrList`, `Q2Queue`,
   `Q2ListIterator`, which are the pointer containers with ownership.
2. ~~**Migrate the data files at the same time.**~~ **Done, D2.** Only the 7
   `.rrb` needed it — the 14 `.exp` already stored the order the simulation
   used. `Q2Dict::hash` generated that ordering and is deleted.
3. Re-verify against the captured 2003 run. **Not done**, and it still needs
   the x86 box's numbers (§7).

Sequenced this way the ordering stopped being a hidden property of a hash
function and became visible in the data.

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
