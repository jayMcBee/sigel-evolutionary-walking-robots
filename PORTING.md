# SIGEL — Qt 2.3 → Qt 6 migration plan

**Scope: Qt API migration only.** Not toolchain modernization, not build-system
work, not reproducibility testing, not PVM. Those are separate jobs; see §3.

**Status: NOT APPROVED. No work starts until §5 decisions are signed off.**
Everything in §7 is contingent on those answers. A fresh session must not treat
§7 as authorized.

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

Not a git repository yet. git 2.51.0 available. See §6.

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
| `setAutoDelete` / `autoDelete` | 42 | ownership decision points |
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

## 4. Why there is no "start on old Qt" rung

| Candidate | Verdict |
|---|---|
| Qt 2.3 (bundled) | Won't build on gcc 15. |
| Qt 3.3.8 | Won't build on gcc 15. |
| **TQt3** — Trinity's fork, R14.1.4 / Apr 2025 | Builds on modern gcc, but renames every `Q*` → `TQ*`, no compat layer. Candidate as a one-shot tool for the `.ui` files only. |
| Qt 4 + Qt3Support | Historically exactly this rung — `Q3PtrList`, `Q3ListView`, `Q3PopupMenu`. No maintained Qt 4 on a current toolchain. |
| Qt 5.15 | Qt3Support already removed. Nearly as far from Qt 2 as Qt 6. |

This is background for decision **D1**, not a conclusion.

---

## 5. Decisions required before any work — YOUR CALL

Nothing in §7 is authorized until these are answered. Recommendations are
marked, not applied.

| # | Decision | Options | Rec. |
|---|---|---|---|
| **D1** | Migration strategy | (a) compat shim over Qt 6, ported onto then deleted (b) direct per-module rewrite, no shim (c) TQt3 as an intermediate rung | a |
| **D2** | Target Qt version | 6.x — pin a minor, or track distro | — |
| **D3** | GUI scope | (a) core + all 4 GUI modules (b) core only, GUI deferred (c) core only, GUI dropped permanently | — |
| **D4** | GUI toolkit *(if D3 ≠ c)* | (a) Qt 6 Widgets, preserves existing forms (b) QML, forms rewritten | a |
| **D5** | `.ui` handling *(if D3 ≠ c)* | (a) write a converter script (b) TQt3 Designer open-and-resave (c) hand-rebuild in Designer 6 | a |
| **D6** | Container targets | (a) `QList<T*>` — stays in Qt idiom (b) `std::vector<std::unique_ptr<T>>` — moves ownership to the type system | — |
| **D7** | The 42 `autoDelete` sites | (a) blanket rule from D6 (b) case-by-case, each needs your approval | — |
| **D8** | `QString`→`const char*`, 74 sites | (a) `.toLatin1()` — preserves Qt 2 behaviour exactly (b) `.toUtf8()` — modernizes; **data files and German strings may change meaning** | — |
| **D9** | `QListView` → | (a) `QTreeWidget` — item-based, closest to Qt 2 (b) `QTreeView` + model — more work, modern idiom | a |
| **D10** | Back-edge cutting | (a) forward-declare where possible, callback where not (b) callback/observer everywhere | a |
| **D11** | Verification depth per rung | (a) compiles and links (b) + smoke run (c) + output comparison — needs test scaffolding you have not scoped | — |
| **D12** | Rung granularity | one commit per rung as listed (24), or finer/coarser | — |

**D11 is the one I would not skip.** With (a), a rung being "green" means only
that it compiles — a rename that silently changes pointer ownership passes.
That is precisely the failure mode the shim in D1(a) is designed to postpone
rather than prevent.

---

## 6. Git protocol

Not yet a repo. Proposed, pending **D12**:

```
git init
git add x/kdesigelSources.1.3/          # pristine 2003 source
git commit -m "vendor: kdesigel 1.3 sources as released 2003-04-30"
git tag v1.3-pristine
```

Every later change diffs against `v1.3-pristine`.

- One branch per phase: `qt6/phase-a-core`, `qt6/phase-b-shim-removal`,
  `qt6/phase-c-gui`
- **One commit per rung**, message prefixed with the rung ID: `A4: SIGEL_Robot
  onto q2compat`
- Tag each completed rung: `rung-A4`
- No squashing — the per-rung history *is* the progressive record, and is what
  makes a bad rung bisectable

24 checkpoints across the three phases as listed below.

---

## 7. Rungs — CONTINGENT ON §5

Written assuming **D1(a)**. If D1 changes, this section is rewritten.

Exit criterion per rung is set by **D11** and currently unanswered.

### Phase A — core onto Qt 6 (10 checkpoints)

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

Phase A exit: core builds against Qt6Core, headless.

### Phase B — delete the shim (5 checkpoints)

| # | Class | Sites |
|---|---|---|
| B1 | `Q2Array` — value semantics, easiest first | 74 |
| B2 | `Q2Dict` | 61 |
| B3 | `Q2PtrVector` | 48 |
| B4 | `Q2PtrList` — the 42 `autoDelete` sites resolve here per **D7** | 37 |
| B5 | `Q2CString`, `Q2ValueList`, `Q2Queue`, iterators | 16 |

Phase B exit: `q2compat.h` deleted.

### Phase C — GUI (9 checkpoints, only if D3 ≠ c)

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

## 8. Effort, contingent

| Phase | Checkpoints | Effort |
|---|---|---|
| A | 10 | 1.5 wk |
| B | 5 | 1 wk |
| C | 9 | 2.5–3 wk |

**~2.5 weeks headless, ~5.5 weeks complete.** Excludes §3 precondition (~3 days,
separate job) and any test scaffolding chosen under D11.
