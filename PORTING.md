# SIGEL — Qt 2.3 → Qt 6 migration plan

**Scope widened 2026-08-22.** Was Qt API only. Now also covers getting SIGEL to
build and run, because nothing else can be verified without it — see §3. The
interface migration (Phase C) follows.

**Status — 2026-08-23**

| phase | state |
|---|---|
| 0 — comments to English | done for the 9 core modules; 9 GUI files still hold Latin-1 |
| A — core onto Qt 6 | **done**, tags `step-A0`…`step-A9`. `./check.sh`: 117 pass, 5 fail (all need a GUI) |
| B — ownership explicit | **8 of 14 containers**. 5 still on `setAutoDelete` — open, §7 |
| R — build and run | core builds and runs, faithful to 1.3. **No way to check it yet** — needs fitness numbers from the 1.3 binary on the x86 box, §7 |
| C — GUI | not started, not authorized |

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

1. **The Qt port** — PVM, then Phase C, the interface. This is the work.
2. **The 1.0 → 1.3 regression** — after, if wanted.
3. **The diagnostics wishlist** from the `sigel-x86` session — 9 items on
   validating robot models at load. Recorded in `regression_1.0_to_1.3.md`.
   Not part of either job above.

**Order of work, agreed 2026-08-22:**

1. ~~Build core plus a small program that runs one fitness evaluation, under
   AddressSanitizer.~~ **Built and running**, faithful to 1.3. Validating it
   needs reference numbers from the 1.3 binary — §7.
2. Fix PVM — 40/40 files fail because glibc dropped `rpc/types.h`.
3. Full headless run, compared against the captured 2003 run.
4. Convert the last 5 containers, now testable (§7).
5. Phase C — the interface.

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
- Verify: `./check.sh` from the repo root. Takes several minutes.
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
├── check.sh                                the exit criterion, §7
├── replicate.sh                            the 14 experiments vs 2003, §7
├── Makefile                                the build, §7 Phase R
├── sigel_eval.cpp                          one fitness evaluation, §7 Phase R
├── patches/                                5 patches to the vendored tree
├── shim/                                   pre-standard C++ headers
├── build/                                  untracked, `make clean` removes it
├── data/                                   untracked, 7 robots and 12 experiments
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
- PVM — 40/40 files fail, glibc dropped `rpc/types.h`; `libtirpc` is the likely
  answer
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

## 4. No "start on old Qt" step

Qt 2.3 and Qt 3.3.8 do not build on gcc 15. Qt 4 + Qt3Support was historically
this step but has no maintained build on a current toolchain. Qt 5.15 already
dropped Qt3Support. TQt3 (Trinity, R14.1.4) builds but renames every `Q*` →
`TQ*` — possible one-shot tool for the `.ui` files only.

---

## 5. Decisions — signed off 2026-08-18

| # | Decision | Answer |
|---|---|---|
| **D1** | Migration strategy | **(a)** compat shim over Qt 6 |
| **D2** | Target Qt version | **6.9.2**, Ubuntu `qt6-base-dev`; recorded, not pinned |
| **D3** | GUI scope | **(b)** core only; Phase C not authorized |
| **D4** | GUI toolkit | **(a)** Qt 6 Widgets *(Phase C)* |
| **D5** | `.ui` handling | **(a)** converter script *(Phase C)* |
| **D6** | Container targets | **(a)** `QList<T*>` |
| **D7** | The `autoDelete` sites | **(a)** blanket rule from D6; exceptions in §9 |
| **D8** | `QString`→`const char*` | **(b)** `.toUtf8()` |
| **D9** | `QListView` → | **(a)** `QTreeWidget` *(Phase C)* |
| **D10** | Back-edge cutting | **(a)** forward-declare; no callback needed anywhere |
| **D11** | Verification depth | **(a)** per-file syntax check; Phase B adds the §9 audit |
| **D12** | Step granularity | **15 steps** — A0–A9, B1–B5, plus Phase 0 per D14 |
| **D13** | Qt 2 behaviour that is itself a defect | **fix it, and fix the cause** — §9 |
| **D14** | Comment language | German → English as **Phase 0**, before A1 |

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

One evaluation is 0.2 s. **All 12 shipped experiments run clean under
AddressSanitizer and UndefinedBehaviorSanitizer**, with identical results
sanitized and not.

**The one defect that stood between building and running.** Qt 2's `QTime()`
was 00:00:00.000 and valid. Qt 6's is null: `addSecs` returns another null
`QTime`, `secsTo` returns 0, and a null `QTime` holds -1 ms, which is less than
every real time. So `SIG_DynaMechsSimulationQueries::getActualSimulationTime`
returned null, `SIG_Simulation::start`'s `while (act < max)` never ended, and
`fitness = distance / simulatedSeconds` was a division by zero. **11 sites**,
all now `QTime( 0, 0 )` — 9 fitness functions, the queries object and one
`SIGEL_SlaveGUI` signal. This is the shape §9 warns about: same API, same
compile, different behaviour.

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

### Replication — no oracle yet

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
the only independent numeric oracle available. All in `data/`, untracked.

### Phase C — GUI — DEFERRED per D3(b), NOT AUTHORIZED

| # | Work | Sites |
|---|---|---|
| C1 | `WFlags` / `WType_*` / `WStyle_*` → `Qt::WindowFlags`, `Qt::WA_*` | 60 |
| C2 | `setCaption` → `setWindowTitle`; `QApplication`, `qApp` | 44 |
| C3 | `QGLWidget` → `QOpenGLWidget` | 6 |
| C4 | `QMultiLineEdit` → `QTextEdit` | 6 |
| C5 | `QPopupMenu` → `QMenu`; `insertItem` → `addAction`/`addItem` | 79 |
| C6 | `QListView` → per **D9** | 105 |
| C7 | `.ui` conversion per **D5**, prototyped on one form | 1 |
| C8 | Remaining 19 forms | 19 |
| C9 | The four GUI modules build and run | 14.6k |

**C7 context:** the files are Qt **2** format, one generation below what Qt 4's
`uic3 -convert` accepts, and Qt 6's `uic` requires `version="4.0"`. Small
vocabulary — ~30 XML elements, 20 widget classes, 19 of them standard. Two
transforms: Qt 2 nests properties as children where Qt 4/6 uses attributes, and
108 `QLayoutWidget` pseudo-widgets flatten into real `<layout>` elements.
`uic3 -convert` discards custom signals and slots — there are 49 connections.

---

## 8. Effort

| Phase | Steps | Effort | Status |
|---|---|---|---|
| A | 10 | 1.5 wk | done |
| B | 5 | 1 wk | 8 of 14 containers |
| C | 9 | 2.5–3 wk | deferred |

Excludes the §3 precondition (~3 days, separate job), which nothing here
depends on.

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
| `SIG_DynaMechsSimulationQueries::sense` | 1.3 scaled the joint sensor by `360/2pi` | restored 1.0's plain ratio | A 1.3 regression that postdates the published record: the sensor reported a ~57-cycle sawtooth instead of the joint angle. Replication 7/13 -> 11/13, `runnerNiceWalkingFitness` to 100/100. **HEAD therefore reproduces SIGEL 1.0, not the 1.3 binary** |
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
  11 sites; found only by running, which is the argument for §3.

---

## 10. Debt — after the port is trusted

### Drop the Qt 2 emulation

**After the port is confirmed to produce valid results, not before.** What the
shim currently carries, and why:

| | why it exists |
|---|---|
| Qt 2's hash order in `Q2Dict` | iteration order numbers the links, so the shipped experiments depend on it |
| `size()` ≠ `count()` on `Q2PtrVector` | allocated vs occupied slots, with null holes |
| the internal cursor on `Q2PtrList` | `first()`/`next()` walks are real state |
| `insert()` deletes the occupant, shrinking `resize()` deletes the tail | the only free path at 8 sites |
| clamp-on-out-of-range | matches `QGArray::at`, and hides several real defects |

The clean-up, in this order:

1. Replace the emulation with straightforward containers — insertion-ordered
   maps, plain `QList<T*>`, explicit deletes, indexed loops instead of a cursor.
2. **Migrate the data files at the same time** so the experiments keep working:
   7 `.rrb` and 12 `.exp` (robots are embedded in the experiments too). Rewrite
   each so declaration order *is* the order the simulation uses. `Q2Dict::hash`
   is the function that generates that ordering — keep it until the migration is
   done, then delete it.
3. Re-verify against the captured 2003 run.

Sequenced this way the ordering stops being a hidden property of a hash function
and becomes visible in the data.

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
