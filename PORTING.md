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
corpus, which is pure ASCII (§5), but it is the path experiment files are read
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
| **D12** | Step granularity | as listed — **15 steps** (A0–A9, B1–B5); A9 splits in-flight only if it proves unwieldy |

### Measured during sign-off

Facts established while answering the above, not present in §2. These are
measured; trust them as you would §2.

- **`autoDelete` splits 34 owning / 7 non-owning.** §2's count of 42 includes
  one *comment* (`src/SIGEL_MasterGUI/SIG_GPParameter.cpp:467`), not a call.
  All 41 real calls take a literal `TRUE`/`FALSE`, which is what makes D7(a) a
  lookup rather than a judgement.
- **The back-edges are nearly free.** Of the 5 includes, 2 are dead text and 3
  forward-declare cleanly; none needs a callback. Detail in §7.
- **The corpus is pure ASCII.** Sources, `.ui` files and the 2003 binary
  distribution contain zero bytes ≥ 0x80, and no experiment data ships with
  either. Latin-1 and UTF-8 are byte-identical over everything present — which
  is why D8 went to `.toUtf8()`: the risk §5 originally warned about is empty
  for existing data, and all three explicit `.latin1()` sites feed POSIX file
  paths, where UTF-8 is correct on a modern system.
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

**Exit criterion per step (D11):** every file touched by the step passes
`g++ -fsyntax-only` against Qt 6 headers. Phase B steps additionally require
the §9 ownership audit.

### Phase A — core onto Qt 6 (10 steps)

| # | Work | LOC |
|---|---|---|
| A0 | Add `compat/q2compat.h`: `Q2Array`, `Q2Dict`, `Q2PtrVector`, `Q2PtrList` with Qt 2 semantics over Qt 6. Nothing uses it yet. | ~400 new |
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

- each of the **34** `setAutoDelete(TRUE)` containers has **exactly one**
  `qDeleteAll()` on the owning path
- none of the **7** `setAutoDelete(FALSE)` containers has one
- no `qDeleteAll()` exists that does not trace to a `TRUE` site

Grep-checkable against `v1.3-pristine`. It verifies the *transformation*, not
the behaviour — that distinction is the accepted limit of D11(a).

### `QTextStream` default codec

232 sites. API unchanged, but Qt 2 defaulted to Latin-1 and Qt 6 defaults to
UTF-8. A no-op over this corpus (pure ASCII, §5), and it is the path experiment
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

They also **confirm the D8 basis independently**: every `.exp`, `.rrb` and `.wrl`
is pure ASCII, so the corpus-wide claim in §5 holds against this data too, and the
`QTextStream` codec change above stays a no-op.

### Name collisions that survive into Qt 6

Both are traps where a Qt 2 name still exists in Qt 6 with different meaning:

- `QVector`/`QList` — pointers in Qt 2, values in Qt 6. **Silent**: compiles
  clean, then double-frees. This is the trap D1(a)'s shim exists to defuse.
- `QListView` — a multi-column tree in Qt 2, a flat model-view list in Qt 6.
  Fails loudly at compile time, but will mislead anyone reading the diff.
