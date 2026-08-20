# SIGEL — Qt 2.3 → Qt 6 migration plan

**Scope: Qt API migration only.** Not toolchain modernization, not build-system
work, not reproducibility testing, not PVM. Those are separate jobs; see §3.

**Status: §5 decisions signed off 2026-08-18.** Phases A and B are authorized.
Phase C is deferred per **D3(b)** and is *not* authorized. §9 lists what is
deliberately still open.

---

## 1. Repo state

```
/home/jan/Downloads/sigel/
├── PORTING.md                              this file
├── kdesigelSources.1.3.tar.gz              upstream source (2003-04-30)
├── sigelSourceDistribution.1.0.tar.gz      older 1.0 release, not used
├── supportingLibs.tar.gz                   vendored deps
├── kbin.tar.gz                             2003 i386 binary, reference only
├── x/kdesigelSources.1.3/kdesigel/         ← extracted source root
│   ├── configure.in, Makefile.am           autotools (see §3)
│   └── kdesigel/
│       ├── src/       19 module dirs       ~40k LOC
│       ├── include/   16 module dirs       ~25k LOC
│       └── ui/        20 .ui files         Qt 2 Designer format
├── x/supportingLibs/supportingLibs/        Qt 2.3, dynamechs, SOLID, cv97,
│                                           newmat09, qhull, fparser, Dynamo, pvm3
├── shim/                                   throwaway header shims from analysis
└── xb/                                     extracted 2003 binary
```

Git repo initialized; pristine source tagged `v1.3-pristine`. See §6.

Upstream: `sourceforge.net/projects/sigel` — SIGEL, Uni Dortmund LS11,
2001–2003, GPLv2. Genetic programming evolves walking gaits for simulated
robots. Despite the name "KDESIGEL", **there is no KDE dependency** — zero
`k*.h` includes. It is a plain Qt 2.3 application.

---

## 2. Facts — measured, do not re-derive

All counts from the extracted 1.3 tree. A fresh session should trust these
rather than re-scanning.

### Qt API that must change

| Item | Sites | Notes |
|---|---|---|
| `QArray<T>` | 74 | Qt 2 value array |
| `QDict<T>` + `QDictIterator` | 61 | |
| `QVector<T>` | 48 | **array of pointers** in Qt 2 — not Qt 6's `QVector` |
| `QList<T>` | 37 | **list of pointers** in Qt 2, with `autoDelete()` |
| `QCString` / `QValueList` / `QQueue` / `QListIterator` | 16 | |
| `setAutoDelete` / `autoDelete` | 42 | 41 calls (34 `TRUE`, 7 `FALSE`) + 1 comment — see §5 |
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
`QTimer` 2 — all carry into Qt 6 essentially as-is.

One qualifier on `QTextStream`: the *API* is unchanged, but the default codec
is not — Qt 2 defaulted to Latin-1, Qt 6 defaults to UTF-8. A no-op over this
corpus — non-ASCII appears only in comments (§5) — but it is the path experiment files are read
and written through. Tracked in §9, not scheduled.

**Absent, and these are usually the worst part of a Qt port:**

- **0 `QPainter`, 0 `paintEvent`** — no custom painting anywhere.
- **444 `SIGNAL()`/`SLOT()` macros in 174 `connect()` calls are all still valid
  in Qt 6.** String-based connect was never removed. No-ops unless a signature
  names a dead type.

### Structure

| Metric | Value |
|---|---|
| Core files needing **no** container work | **210 of 266** |
| Core files touching dead Qt 2 containers | 56 — worst is `src/SIGEL_Robot/SIG_Robot.cpp` (30) |
| `Q_OBJECT` in core | 3 |
| Core files touching dialogs | 5 |
| Core → GUI back-edges | 4 edges, **5 includes total** |

Module dependency DAG (measured from includes):

```
SIGEL_Tools, SIGEL_Environment, MT_GPSystem     no deps
SIGEL_Robot      ← Tools                        (+1 back-edge → Simulation)
SIGEL_Program    ← Tools, Robot                 (+1 back-edge → GP)
SIGEL_RobotIO    ← Tools, Robot
SIGEL_Simulation ← Tools, Program, Environment, Robot
MT_Control       ← MT_GPSystem                  (+1 back-edge → MT_GUI)
SIGEL_GP         ← all of the above             (+2 back-edges → MasterGUI)
SIGEL_Visualisation, then SIGEL_CommonGUI, MT_GUI, SIGEL_MasterGUI, SIGEL_SlaveGUI
```

---

## 3. Precondition — out of scope, stated once

The Qt work cannot be *compiled or verified* until the app builds at all. That
is a separate job, not planned here and not to be started as part of it:
vendored libs on gcc 15, PVM (40/40 files fail — glibc dropped `rpc/types.h`),
a build system to replace the `configure.in` that detects the OS by grepping
`/proc/version` for `SuSE`, and pre-standard `for`-scope in `MT_*`.

Checked and clean, nothing to do: **LP64 portability** — 2 pointer-to-`int`
casts, no long↔pointer casts, no `sizeof(long)` assumptions, no format-string
mismatches.

---

## 4. Why there is no "start on old Qt" step

| Candidate | Verdict |
|---|---|
| Qt 2.3 (bundled) | Won't build on gcc 15. |
| Qt 3.3.8 | Won't build on gcc 15. |
| **TQt3** — Trinity's fork, R14.1.4 / Apr 2025 | Builds on modern gcc, but renames every `Q*` → `TQ*`, no compat layer. Candidate as a one-shot tool for the `.ui` files only. |
| Qt 4 + Qt3Support | Historically exactly this step — `Q3PtrList`, `Q3ListView`, `Q3PopupMenu`. No maintained Qt 4 on a current toolchain. |
| Qt 5.15 | Qt3Support already removed. Nearly as far from Qt 2 as Qt 6. |

This is background for decision **D1**, not a conclusion.

---

## 5. Decisions — SIGNED OFF 2026-08-18

| # | Decision | Answer |
|---|---|---|
| **D1** | Migration strategy | **(a)** compat shim over Qt 6, ported onto then deleted |
| **D2** | Target Qt version | **6.9.2** from Ubuntu `qt6-base-dev`; recorded here as the tested-against version, no pinning machinery |
| **D3** | GUI scope | **(b)** core only, GUI deferred — Phase C not authorized |
| **D4** | GUI toolkit | **(a)** Qt 6 Widgets *(applies when Phase C starts)* |
| **D5** | `.ui` handling | **(a)** converter script *(applies when Phase C starts)* |
| **D6** | Container targets | **(a)** `QList<T*>` |
| **D7** | The `autoDelete` sites | **(a)** blanket rule from D6; exceptions logged in §9 |
| **D8** | `QString`→`const char*` | **(b)** `.toUtf8()` |
| **D9** | `QListView` → | **(a)** `QTreeWidget` *(applies when Phase C starts)* |
| **D10** | Back-edge cutting | **(a)** forward-declare where possible — measured: callback needed nowhere |
| **D11** | Verification depth | **(a)** per-file `g++ -fsyntax-only` against Qt 6; Phase B additionally requires the §9 ownership audit |
| **D12** | Step granularity | as listed — **15 steps** (A0–A9, B1–B5), plus Phase 0 per D14; A9 splits in-flight only if it proves unwieldy |
| **D13** | Qt 2 behaviour that is itself a defect | **fix it, and fix the cause.** Applies to the `memcmp` sort and the out-of-range clamp — see §9. The port is no longer strictly behaviour-preserving; that is deliberate |
| **D14** | Comment language | translate German → English as **Phase 0**, before A1, in its own commits. Also makes the tree pure ASCII |

Signed off 2026-08-18; **D13/D14 added 2026-08-18** after the A0 review.

### Measured during sign-off

Facts established while answering the above, not present in §2. These are
measured; trust them as you would §2.

- **`autoDelete` splits 38 owning / 9 non-owning — 47 calls.** All take a
  literal `TRUE`/`FALSE`, which is what makes D7(a) a lookup rather than a
  judgement. (An earlier count of 34/7 here was wrong: `grep` in this
  environment is a wrapper around `ugrep -I`, which classifies the 46
  Latin-1/CRLF files as binary and skips them silently. Any measurement in this
  document not taken with `command grep` or Python is suspect for that reason;
  §2's own figures were spot-checked and hold.)
- **The back-edges are nearly free.** Of the 5 includes, 2 are dead text and 3
  forward-declare cleanly; none needs a callback. Detail in §7.
- **Non-ASCII exists, but only in comments.** 46 of 380 source files carry
  Latin-1 German (`ä ö ü ß Ä`) — 119 lines, **every one a comment, none a string
  literal**, verified byte by byte. So D8's conclusion stands (no runtime string
  carries a byte ≥ 0x80, making `.toLatin1()` and `.toUtf8()` identical over all
  real data, and the three explicit `.latin1()` sites feed POSIX paths where
  UTF-8 is right), but the earlier claim that the corpus was *pure ASCII* was
  wrong. Phase 0 removes the Latin-1 entirely.
- **Per-file syntax checking needs nothing from §3.** `SIGEL_Tools` and
  `MT_GPSystem` have zero non-Qt/non-stdlib includes; `SIGEL_Environment` has
  one (`dmEnvironment.hpp`), `SIGEL_Robot` one (`CyberVRML97.h`). Vendored
  headers need only *parse*, not link. This is what makes D11(a) reachable
  while §3 remains out of scope.

**What D11(a) does and does not buy.** Phase A steps are pure renames under the
shim, so a syntax check covers the entire error class they can produce. Phase B
is where ownership becomes hand-written code, and there a syntax check proves
nothing — both the correct and the double-freeing version compile. The §9 audit
is the substitute, and it is an inspection, not a test. Phase B ships
correct-by-inspection; that is a known and accepted limit of this plan.

## 6. Git protocol

Done — the repo exists, the pristine 2003 source is committed and tagged:

```
0516d62  vendor: kdesigel 1.3 sources as released 2003-04-30   (tag: v1.3-pristine)
685f04c  docs: Qt 2.3 -> Qt 6 migration plan
```

Every later change diffs against `v1.3-pristine`.

- One branch per phase: `qt6/phase-a-core`, `qt6/phase-b-shim-removal`.
  No `qt6/phase-c-gui` unless D3 is revisited.
- **One commit per step**, message prefixed with the step ID: `A4: SIGEL_Robot
  onto q2compat`
- Tag each completed step: `step-A4`
- No squashing — the per-step history *is* the progressive record, and is what
  makes a bad step bisectable

**15 steps** across the two authorized phases (D12).

## 7. Steps — AUTHORIZED (Phases A and B)

Per **D1(a)**. 15 steps, per **D12**.

**Exit criterion per step (D11):** run `./check.sh` at the repo root. It is the
D11 criterion made reproducible — before it existed, the flags lived only in
this session's shell history and could not be re-run by anyone else.

It compiles every converted module, compiles every converted header standalone,
and runs the shim self-check. Vendored headers are `-isystem`, so the ~12,979
warnings they generate do not bury the ~300 in code we are responsible for.

**Warnings are part of the criterion.** They were not read up to A8, and that
cost: the `Qt::endl`-on-`std::cerr` regression in A3 was reported by this very
command at the step that introduced it, and the step recorded "0 errors" and
shipped. A reviewer found it four commits later.

Phase B steps additionally require the §9 ownership audit.

### Phase 0 — comments to English (D14)

Before A1. German comments → English, which also removes the Latin-1 bytes from
all 46 affected files. No code change whatsoever.

Exit criterion, stronger than D11's and specific to this phase: **strip comments
from before and after, and diff the remainder — it must be byte-identical.** That
mechanically proves the phase touched nothing but comments.

It must not be interleaved with A1–A9. The whole value of D1(a) is that each
Phase A diff is reviewable as a pure rename; mixing comment rewrites into those
diffs destroys that property.

### Phase A — core onto Qt 6 (10 steps)

| # | Work | LOC |
|---|---|---|
| A0 | Add `compat/q2compat.h`: `Q2Array`, `Q2Dict`, `Q2PtrVector`, `Q2PtrList`, `Q2Queue`, `Q2ValueList`, `Q2CString` + iterators, with Qt 2 semantics over Qt 6. Nothing uses it yet. **Done, tagged `step-A0`.** | 541 new |
| A1 | `SIGEL_Tools` | 666 |
| A2 | `SIGEL_Environment` | 1,028 |
| A3 | `MT_GPSystem` | 6,880 |
| A4 | `SIGEL_Robot` + cut back-edge → `SIGEL_Simulation` (1 include) | 7,469 |
| A5 | `SIGEL_Program` + cut back-edge → `SIGEL_GP` (1 include) | 1,932 |
| A6 | `SIGEL_RobotIO` | 3,017 |
| A7 | `SIGEL_Simulation` | 8,715 |
| A8 | `MT_Control` + cut back-edge → `MT_GUI` (1 include) | 2,724 |
| A9 | `SIGEL_GP` + cut back-edges → `SIGEL_MasterGUI` (2 includes) | 14,066 |

The shim exists so each of A1–A9 is a **pure rename**, deferring every
ownership decision to Phase B. The trap it defuses: Qt 2's `QVector`/`QList`
hold pointers, Qt 6's hold values, **and the names are identical** — a naive
rename compiles clean and then double-frees.

**The 4 back-edges, measured (D10).** Cheaper than the table above implies —
2 are dead includes and 3 forward-declare. No callback anywhere:

| Step | Include | Use | Fix |
|---|---|---|---|
| A4 | `src/SIGEL_Robot/SIG_Link.cpp:36` → `SIG_DynaSystem.h` | symbol appears nowhere else in the file | delete the include |
| A5 | `include/SIGEL_Program/SIG_Program.h:45` → `SIG_GPParameter.h` | `&param` in 2 signatures | forward-declare, include in `.cpp` |
| A8 | `include/MT_Control/MT_Controller.h:4` → `MT_MainWindow.h` | `MT_MainWindow *mainWindow` member | forward-declare |
| A9 | `include/SIGEL_GP/SIG_GUIGPManager.h:27` → `SIG_Experiment.h` | `&guiExperiment` param + member | forward-declare, include in `.cpp` |
| A9 | `include/SIGEL_GP/SIG_GPManager.h:29` → `SIG_IndividualListItem.h` | symbol appears nowhere else in the header | delete the include |

The two "dead include" findings are *textual* — no symbol reference in the
file. Since nothing compiles yet (§3), that is the strongest available claim.
If one turns out to be load-bearing transitively (e.g. `SIG_DynaSystem.h`
pulling in dynamechs declarations), the fix is a direct include of whatever it
was really providing — not a callback.

A8 and A9 point at deferred GUI modules. A forward declaration is exactly the
seam Phase C would reconnect to, so cutting them now costs Phase C nothing.

Phase A exit: core passes `-fsyntax-only` against Qt6Core, headless.

### Phase B — delete the shim (5 steps)

| # | Class | Sites |
|---|---|---|
| B1 | `Q2Array` — value semantics, easiest first | 74 |
| B2 | `Q2Dict` | 61 |
| B3 | `Q2PtrVector` | 48 |
| B4 | `Q2PtrList` — the 41 `autoDelete` calls resolve here per **D7**: 34 `TRUE` → `qDeleteAll()` in the owner's destructor, 7 `FALSE` → nothing | 37 |
| B5 | `Q2CString`, `Q2ValueList`, `Q2Queue`, iterators | 16 |

Phase B exit: `q2compat.h` deleted, and the §9 ownership audit passes.

### Phase C — GUI — DEFERRED per D3(b), NOT AUTHORIZED

Mechanical and independent items first, to shrink the surface before the
structural ones.

| # | Work | Sites |
|---|---|---|
| C1 | `WFlags` / `WType_*` / `WStyle_*` → `Qt::WindowFlags`, `Qt::WA_*` | 60 |
| C2 | `setCaption` → `setWindowTitle`; `QApplication`, `qApp` | 44 |
| C3 | `QGLWidget` → `QOpenGLWidget` | 6 |
| C4 | `QMultiLineEdit` → `QTextEdit` | 6 |
| C5 | `QPopupMenu` → `QMenu`; `insertItem` → `addAction`/`addItem` | 79 |
| C6 | `QListView` → per **D9** | 105 |
| C7 | `.ui` conversion per **D5**, prototyped on `SIG_EditHostDialogBase.ui` | 1 form |
| C8 | Remaining 19 forms | |
| C9 | `SIGEL_CommonGUI`, `MT_GUI`, `SIGEL_MasterGUI`, `SIGEL_SlaveGUI` build and run | 14.6k |

**C7 context for whichever option D5 selects:** the files are Qt **2** format,
one generation below what Qt 4's `uic3 -convert` accepts, and Qt 6's `uic`
requires `version="4.0"`. The vocabulary is small — ~30 distinct XML elements,
20 widget classes of which 19 are standard (247 `QLabel`, 108 `QLayoutWidget`,
54 `QPushButton`, 52 `QLineEdit`, 51 `QSlider`, 46 `QSpinBox`). Two transforms:
Qt 2 nests properties as children where Qt 4/6 uses attributes, and 108
`QLayoutWidget` pseudo-widgets flatten into real `<layout>` elements. `uic3
-convert` is documented to discard custom signals and slots — there are 49
connections in these files.

---

## 8. Effort

| Phase | Steps | Effort | Status |
|---|---|---|---|
| A | 10 | 1.5 wk | authorized |
| B | 5 | 1 wk | authorized |
| C | 9 | 2.5–3 wk | deferred, D3(b) |

**~2.5 weeks for the authorized work.** Excludes the §3 precondition (~3 days,
separate job), which nothing here depends on: D11(a) was chosen precisely so
Phases A and B can be verified without it.

---

## 9. Open, deliberately

Not decisions that were dodged — items measured and consciously left.

### Phase B ownership audit (D11)

The check that substitutes for being unable to run the code. After B4:

- each of the **38** `setAutoDelete(TRUE)` containers has **exactly one**
  `qDeleteAll()` on the owning path
- none of the **9** `setAutoDelete(FALSE)` containers has one
- no `qDeleteAll()` exists that does not trace to a `TRUE` site

Grep-checkable against `v1.3-pristine`. It verifies the *transformation*, not
the behaviour — that distinction is the accepted limit of D11(a).

### `QTextStream` default codec

232 sites. API unchanged, but Qt 2 defaulted to Latin-1 and Qt 6 defaults to
UTF-8. A no-op over this corpus (non-ASCII only in comments, §5), and it is the path experiment
files are read and written through. Explicitly **not** folded into D8. Becomes
a live question the moment a non-ASCII experiment file exists.

### Ownership handled without the flag

D7(a) assumes the `autoDelete` flag records every ownership decision. One
counter-example found, and it is a comment rather than a call:

```
src/SIGEL_MasterGUI/SIG_GPParameter.cpp:467
  * this is the list of SIG_GPPVMHosts which have to be deleted as
    setAutoDelete is NOT true
```

In `SIGEL_MasterGUI`, so outside authorized scope — but proof the pattern
exists. Phase B should watch for `take()`/`clear()` handoffs on containers whose
flag stays `TRUE`; those call sites get read during B4 anyway.

### Toggling containers (Phase C)

Two containers flip `autoDelete` at runtime — Qt 2's "remove without deleting"
idiom, which a single destructor `qDeleteAll` does **not** reproduce. Both are
in deferred GUI code; both need per-site thought when Phase C starts.

| Container | File | Toggles |
|---|---|---|
| `experimentDict` | `src/SIGEL_MasterGUI/SIG_ExperimentListView.cpp` | 3 `false`…`true` pairs (102/110, 229/236, 249/256) |
| `widgetDict` | `src/SIGEL_MasterGUI/SIG_Experiment.cpp` | 1 pair (206/210) |

The other four repeated targets (`pool`, `vertices`, `experimentHistory`,
`allowedCommands`) only re-set `true` in multiple constructors — blanket-safe.

### Reference data exists, if D11 is ever revisited

The experiment and robot-model files are **not** in any of the tarballs in §1.
They are separate downloads, verified live 2026-08-18:

- `sigel.sourceforge.net/download/experimente/experiments.tar.gz` — 12 `.exp` files
- `sigel.sourceforge.net/download/robotermodelle/robots.tar.gz` — 7 models, each
  `.rrb` + `.wrl` geometry (the `.wrl` files are what cv97 / `CyberVRML97.h` loads)

Not scheduled work, and D11(a) does not use them. They are noted because they are
the only known source of reference input should output comparison — D11 option
(c) — ever be reconsidered, and because until they were found there was no way to
run `-evolve` at all.

**Reference output now exists.** The 2003 i386 binary was brought up on Debian
woody libraries and all 12 experiments validated end to end. One run is captured
verbatim (input `.exp`, stdout/stderr, and the population SIGEL wrote back): 13
generations, 874 slave fitness evaluations, clean termination. Two limits on its
use as a comparison baseline, both important:

- the run is bounded by **wall-clock, not generation count** — `TERMINATIONMODEL`
  selects which criterion applies, so generation counts and timings in the log
  are machine-specific and not reproducible
- `SAVEEXIT=1` makes SIGEL **overwrite the experiment file it was given**, and
  re-emit config keys it had defaulted, so an evolved `.exp` is not
  byte-comparable with its input even ignoring the population

They also **confirm the D8 basis independently**: every `.exp`, `.rrb` and `.wrl`
is pure ASCII, so no *data* file carries a byte ≥ 0x80 and the
`QTextStream` codec change above stays a no-op.

### DEBT — core error reporting (opened at A2)

`SIG_Environment.cpp` opened three `QMessageBox` warning dialogs on terrain
load failure. Core cannot do that: it must build against Qt6Core alone, and
`-evolve` has no `QApplication`, so the dialog could never have worked headless
anyway. A2 replaced them with `SIGEL_Tools::SIG_IO::cerr`.

**This is a stopgap, not a solution.** A GUI user now gets no dialog — the
message goes to a console they may not be watching. That is a regression in GUI
behaviour and it is accepted deliberately, for now.

**Required follow-up, after the core migration:** a real error-reporting
strategy for core — something core can call without knowing whether a GUI
exists, which the GUI can surface as a dialog and headless runs can print. A
callback, a signal, or an error sink handed in at construction. Do not settle
this per-site.

§2 counts 7 core files touching dialogs, so the same decision recurs. Known
sites: `SIG_Environment.cpp` (done), `MT_Controller.cpp`, `SIG_GPFitnessTrainer.cpp`,
`SIG_GPRemoteZORCFitnessFunction.cpp` (+ WIN variant, + both headers).

### DEBT — SIGEL needs a real logging system (opened at review of A1–A6)

Qt 2's `QTextStream` wrote through to unbuffered `stderr` on every `<<`. Qt 6
buffers 16 KB and flushes only on `flush()`, `Qt::endl`, overflow or
destruction. A trailing `"\n"` does **not** flush.

SIGEL has **509 `SIG_IO::cerr`/`cout` statements; 9 of them flush.** So most
diagnostics now sit in a buffer and are lost if the process dies — including on
the SIGSEGV path, which is exactly when they are wanted.

Fixed so far: only the three A2 terrain warnings, which now use `Qt::endl`.

**Required after the Qt migration:** a proper logging system. Levels, one place
that decides where output goes and when it is flushed, and something the GUI can
display. This replaces both `SIG_IO` and the console-warning stopgap recorded
below. Do not fix this by adding `Qt::endl` to 500 call sites.

### Determinism — fixed hash seed (resolved)

Qt 6 randomises the `QHash` seed per process, so `Q2Dict` iteration order varied
between runs. Qt 2's `QGDict` was deterministic for a given insertion sequence.

This mattered: `SIG_Robot`'s copy constructor round-trips through a text
serialisation that walks six dictionaries (`SIG_Robot.cpp:295-335`), and reading
back registers joints with their links in encounter order. Joint ordering, and
with it DynaMechs link numbering, would change run to run — a fixed `RANDOMSEED`
would no longer reproduce a run.

`compat/q2compat.h` now calls `QHashSeed::setDeterministicGlobalSeed()` during
static initialisation. Verified: identical iteration order across runs.
`main()` should call it explicitly too, once `sigel.cpp` and `sigel_slave.cpp`
are ported — static initialisation order is unspecified.

### PLAN CORRECTION — `MT_Control` is not headless (found at A8)

§7 lists A8 as "`MT_Control` + cut back-edge → `MT_GUI` (1 include)". Cutting the
header include is correct and done — `MT_Controller.h` used `MT_MainWindow` only
as a pointer, so it forward-declares.

But **`MT_Controller.cpp` is deeply coupled to the GUI**: 23 `mainWindow->`
member accesses, and it constructs `new MT_MainWindow` itself. It wires up
toolbar actions (`mtStartEvolutionAction`, `mtStopEvolutionAction`, …) with
`connect`/`disconnect`. That is not one include; it is a controller that owns
its window.

So `MT_Controller.cpp` cannot compile against Qt6Core alone, and cannot until
`MT_GUI` is ported in Phase C. The other three files in the module do.

This matters because `sigel.cpp:272` reaches `MT_Controller::startTimedEvolution`
on the headless `-me` path, so a headless build genuinely needs this class.
Either the GUI wiring moves out of `MT_Controller` into `MT_GUI`, or `MT_Control`
is reclassified as a GUI module. Not decided.

Related, same module: 4 of its 15 `QMessageBox` calls are **interactive** — the
return value drives a `switch` or an `if`, i.e. they ask the user a question.
The other 11 were converted to console output as at A2. The 4 cannot be, and are
listed in the error-reporting debt above.

### TRAP — `toUtf8()` returns a temporary (all 74 D8 sites)

Qt 2's `latin1()` returned a pointer into the QString's own buffer, so
`const char *p = s.latin1();` was valid for as long as the string lived. Qt 6's
`toUtf8()` returns a **temporary `QByteArray`**, so the same line dangles at
once:

```cpp
const char *p = s.toUtf8();                 // DANGLES
const QByteArray b = s.toUtf8();            // correct
const char *p = b.constData();
foo(s.toUtf8().constData());                // also fine: one full expression
```

`toLatin1()` behaves identically, so D8's choice does not affect this. Hit at
`SIG_Environment.cpp:459` in A2. Check every D8 site for a stored `const char *`.

### Defects being fixed rather than preserved (D13)

The port deliberately diverges from 2003 behaviour at these points. Each is a
defect, not a design choice, and two of the three were flagged by SIGEL's own
authors in comments they shipped.

| Where | 2003 behaviour | Now | Why |
|---|---|---|---|
| `Q2Array::sort()` | `memcmp` byte order (`qgarray.cpp:635-640`) | numeric | Three sites need ascending numeric order and break above 256: `SIG_GPManager.cpp:304,311` (distinct tournament indices) and `SIG_AllIndividualsView.cpp:240` (feeds `deleteIndividual(positions[n] - n)`). Cause is Qt 2's type erasure — its own source says *"Qt 3.0: Add a virtual compareItems()"* |
| out-of-range array access | warn, clamp index to 0 (`qgarray.h:108-117`) | **warn, clamp — same as 2003** | Done in the shim, not via `Q_ASSERT`: that compiles to nothing under `QT_NO_DEBUG`, so a release build would corrupt memory silently where 2003 returned a wrong value. Verified with `-DQT_NO_DEBUG`. The bad call sites are still being fixed |
| `SIG_ProgramLine.cpp:215-224` | writes `element[no]` in the branch entered *because* `no >= size()`; also compares `int` to `uint` | to be fixed | Its own comment is `// ToDo: Exception!` |
| `SIG_DynaSystem.cpp:266-268` | deletes `dynaJoints[k]` while looping to `dynaDrives.size()` | to be fixed | The two vectors grow independently (`:605`, `:807`) |
| `sigel_slave`, `getenv("SIGEL_ROOT")` | dereferenced unchecked on the `Terrain.ter` path | to be fixed | Segfaults instantly if unset, and the SIGSEGV handler masks it as "Invalid storage access" with no core. Found by running the 2003 binary |

The last one bites under PVM specifically: `pvm_spawn`'d tasks inherit *pvmd's*
environment, not the master's.

### The A0 review

A0 was reviewed independently against the vendored Qt 2.3 sources before any
step depended on it. It found six defects, three of them ownership or state
divergences that compiled cleanly:

- copy constructor and `operator=` propagated `autoDelete`; `qcollection.h:64` is
  `QCollection(const QCollection&) { del_item = FALSE; }`, so a Qt 2 copy is
  always non-owning. The original shim double-freed.
- the `Q2PtrList` cursor was not moved to the removal site, which `removeAt`
  does via `locate()` before unlinking.
- `Q2DictIterator` held a `QMultiHash::const_iterator`, invalidated by any erase
  or rehash, where Qt 2 repaired its registered iterators.

It also found that `Q2Queue`, `Q2ValueList` and `Q2CString` were missing, which
would have blocked A2, A3, A8 and A9 from being pure renames — 9 `QQueue`, 9
`QCString` and 1 `QValueList` site in core, against this document's earlier
assumption that all were deferrable to B5.

Because a double-free compiles perfectly, `q2compat_check.cpp` carries
assert-based verification of the ownership, cursor and slot-deletion semantics
in addition to forcing template instantiation. Reintroducing the copy-constructor
defect makes it abort, so the check is known to have teeth.

Review is scheduled again for each of B1–B5, where hand-written ownership
appears and a compile check stops meaning anything.

### Name collisions that survive into Qt 6

Both are traps where a Qt 2 name still exists in Qt 6 with different meaning:

- `QVector`/`QList` — pointers in Qt 2, values in Qt 6. **Silent**: compiles
  clean, then double-frees. This is the trap D1(a)'s shim exists to defuse.
- `QListView` — a multi-column tree in Qt 2, a flat model-view list in Qt 6.
  Fails loudly at compile time, but will mislead anyone reading the diff.
