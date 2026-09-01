# C++ modernization — working list

**Scope: C++ language level (§1–§4), plus the deferred renames, the German
translation and the version bump below.** Independent of the Qt port; do not
combine commits across the two.

**Protocol:** one commit per item. Jan reviews and approves each. Items 1–2 are
purely mechanical; everything from 3 on gets reviewed and discussed before it
lands.

Status: `[ ]` open · `[~]` in review · `[x]` done · `[!]` blocked

Counts measured 2026-08-19 against the extracted 1.3 tree. Paths are relative to
`x/kdesigelSources.1.3/kdesigel/kdesigel/`.

---

## 1 · Make it legal C++

Constructs the language removed. Not style — a current compiler rejects them.
Review is reading the diff.

- [ ] **1. Pre-standard headers** — 7 sites
  `<iostream.h>` → `<iostream>`, `<vector.h>` → `<vector>`
  ```
  src/SIGEL_GP/SIG_GPIndividual.cpp:25
  src/MT_GPSystem/MT_GPManager.cpp:6
  src/MT_GPSystem/MT_Randomizer.cpp:6
  src/SIGEL_RobotIO/SIG_RobotScanner.cpp:26
  include/SIGEL_Simulation/SIG_SimulationQueries.h:34
  include/SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h:34
  (was SIG_DynaMoSimulationQueries.h:34 -- file deleted 2026-08-28,
   physics_backends.md; re-measure this count)
  ```
  Also retires `shim/`, which exists only to fake these.

- [ ] **2. `register` keyword** — 3 sites, all `src/SIGEL_Visualisation/SIG_EnvironmentRenderer.cpp`
  lines 119, 500, 602. Removed in C++17. Delete the keyword.

- [ ] **3. Loop-variable scope leaks** — 13 files in `src/MT_GPSystem/`
  `for (int i=…){…}` then `i` used after the loop. Hoist the declaration.
  Files: `MT_FitnessTrainer`, `MT_GPManager`, `MT_Interpreter`, `MT_Population`,
  `MT_Program`, `MT_Randomizer`, `MT_Search`, `MT_Statistics`,
  `MT_StatisticsElement`, `MT_Tournament`, `MT_TournamentManager`,
  `MT_TranslatedIndividual`, `MT_Trainingset`
  *Confined to one module — the VC6-era half of the tree.*

- [ ] **4. Dynamic exception specifications** — 6 file pairs (.h + .cpp)
  ```
  SIGEL_Simulation   SIG_Simulation, SIG_Register, SIG_Recorder
                     (SIG_DynaSystem was here; deleted 2026-08-28)
  SIGEL_GP           SIG_GPSimpleRecorder, SIG_GPFullDataRecorder
  SIGEL_Visualisation SIG_RenderRecorder, SIG_SimulationVisualisation
  ```
  **Not purely cosmetic.** Empty `throw()` means "terminate if anything
  escapes" → convert those to `noexcept`. Non-empty `throw(X, Y)` → delete.
  Removing a non-empty spec lets exceptions propagate that previously
  terminated. Discuss per site.

**After 1–4 the code is valid modern C++.** Stopping here is a real result.

---

## 2 · Let the compiler hunt bugs

Additive only — nothing changes at runtime. The value is that failures are
*discoveries*: code that has been wrong since 2003.

- [ ] **5. Add `override`** — 313 `virtual` sites, **one commit per module**

  | module | sites |
  |---|---|
  | SIGEL_RobotIO | 164 |
  | SIGEL_Robot | 40 |
  | SIGEL_GP | 32 |
  | SIGEL_Simulation | 28 |
  | MT_GPSystem | 17 |
  | SIGEL_CommonGUI | 12 |
  | MT_Control | 8 |
  | SIGEL_Visualisation | 7 |
  | MT_GUI | 6 |
  | SIGEL_MasterGUI | 5 |
  | SIGEL_Program, SIGEL_SlaveGUI | 3 |

  Highest-value item on this list. A method meant to override with a subtly
  wrong signature (missing `const`, `int` vs `long`) silently becomes a *new*
  function and the wrong one gets called. `override` turns each into a compile
  error. **Read every failure — do not fix mechanically.**
  Needs base headers to parse, so this is the first item requiring tooling
  rather than an editor.

- [ ] **6. `NULL` → `nullptr`** — 75 sites. Mechanical, one commit.

- [ ] **7. `explicit` on single-argument constructors** — per module.
  Every break is a place an implicit conversion was silently happening.

---

## 3 · Ownership — BLOCKED

- [!] **8. Smart pointers** — 554 `new` vs 127 `delete`

  **Blocked on: a runnable program and a reference trajectory.**

  This is the class of change where the correct and the broken version *both
  compile clean*. No compiler flag verifies it.

  Prerequisite, in order:
  1. one experiment running end to end
  2. fixed random seed
  3. fitness trajectory recorded to a file as the reference
  4. then convert one class at a time, trajectory must stay bit-identical

  Until that exists this is guesswork with nice syntax. Do not start.

---

## 4 · Program display

- [ ] **9. Syntax-highlight the program view** — needs Phase C, GUI not built yet

  New function `programToHtml(const SIG_Program&, const SIG_LanguageParameters&)`
  returning a `QString`. `printToString()` stays as the ZORC serial format
  (`SIG_GPRemoteZORCFitnessFunction.cpp:69`).

  - `<pre>` wrapper, one `<span>` per token.
  - Colour opcodes by group: arithmetic `ADD SUB MUL DIV MOD MIN MAX`,
    data `COPY LOAD`, control `CMP JMP`, robot `MOVE SENSE DELAY`.
  - Register operands print as `R0`–`R7`, computed as `element % getMemorySize()`.
    `LOAD` operand 2 and `JMP` operand 1 print as literals.
  - Line number in a leading `<span>` per line.

  Feed it to `QTextBrowser::setHtml()` at `SIG_SimulationWidget.cpp:234`,
  `SIG_IndividualView.cpp:51`, `SIG_AllIndividualsView.cpp:394`.

---

## What is measured vs. proposed

- **Measured** (trust as fact): every site count, file path and line number above.
- **Proposed** (yours to approve): the three-section ordering, the per-module
  split of item 5, and item 8's blocked status.

## Left by the Qt port, deliberately — Phase D11, 2026-08-29

Both were converted rather than changed, because the port's rule is to move the
Qt API and nothing else.

Both are reached by the gates and neither is observable by them. `usedByLinks`
is appended on every one of the 7 `.rrb` loads and read nowhere in the tree.
`friction` is walked by `writeToFileTransfer` on every run, always empty.

- **`SIG_Material::FrictionValue` could be a value type.** It is two words —
  `SIG_Material *otherSide` and a `DL_Scalar` — held as `QList<FrictionValue *>`
  with a `new` per entry and a `qDeleteAll` in `~SIG_Material`. Values would
  delete both. D8 made exactly this move for `SIG_Register` and it removed a
  real leak; here the destructor already frees, so there is nothing to fix and
  the change would only be tidier.
- **`SIG_Body::usedByLinks` is dead.** `addUsingLink` appends to it from
  `SIG_RobotCompilerObjects.cpp:112` on every model load, and nothing in the
  tree — GUI included — ever reads it back. The member, the method and the one
  call could all go. Asked directly during D11, the answer was convert, not
  delete.

## Rename the two `SIG_GPExperiment` variants — after the Qt 6 port

Two files define `SIGEL_GP::SIG_GPExperiment` with different bodies, one per
binary: `SIG_GPExperiment.cpp` for `sigel`, `SIG_GPExperimentClean.cpp` for
`sigel_slave`. Their headers share the include guard
`SIGEL_GP_SIG_GPEXPERIMENT_H`. See PORTING.md §9.

The arrangement works and is deliberate, but nothing in the source says so.
It has already been misread once as an accidental duplicate, with deletion of
one of the pair proposed as the fix. The name is the whole problem: "Clean"
says nothing about `MT_Controller`, and the identical class name hides that
there are two.

**Do:** give the two classes distinct names — the master's keeps
`SIG_GPExperiment`, the slave's becomes something that says what it is, and the
headers get matching distinct guards. Then the compiler enforces what the build
files currently only imply.

**Not before Phase C.** The master variant is the one that constructs
`MT_Controller`, and `MT_Controller.cpp` does not compile yet, so the master
half cannot be built or tested until the interface is ported. Renaming a class
nothing can compile is how a rename goes wrong.

**Also fix while there:** our `Makefile` globs `src/<module>/*.cpp`, so it
compiles both variants into `libSIGEL_GP.a` where 2003 compiled them into
separate targets. The unused object is harmless today only because the linker
cannot extract it — `SIG_GPExperiment.o` needs `MT_Controller`, which the build
excludes. That is luck, not design.

## Rename `tours` to `tournaments` — after the Qt 6 port is complete and validated

`SIG_GPManager::tours` holds every `SIG_GPTournament` for one generation. The
name reads as travel, or as a shortening of nothing in particular. The type it
holds already says the word.

**Do:** rename the member to `tournaments`. It is private
(`SIG_GPManager.h:152`) and used in one file, so the change is contained. The
two doxygen comments that call it "the QArray tours" (`SIG_GPManager.h:216`,
`:250`) go with it — and they are wrong twice over, since it is not a `QArray`
and has not been one for some time.

**Not before the port is complete AND validated.** `SIG_GPManager` cannot be
compiled today: its constructor reads `actExperiment.mtController`, a member of
only the master variant of `SIG_GPExperiment`, which needs `MT_Controller`,
which does not build. A rename inside a file nothing can compile is a rename
nobody can check.

Same reason as the `SIG_GPExperiment` rename above, and worth doing in the same
pass.

## Translate the German — after the Qt 6 port is complete and validated

SIGEL was written at Uni Dortmund and parts of it are in German, against the
English of the rest of the tree. This is 2003 upstream, not debt the port
created. Counts measured 2026-08-30 by grep over the extracted 1.3 tree.

**The umlauts are Latin-1 bytes, not UTF-8.** A UTF-8 grep misses them.

**Three phases, one commit per phase, in this order.**

- [ ] **10. Comments** — 118 lines in 46 files
  Includes 15 `NEU NEU NEU…` banner lines (`MT_Classifier.cpp`,
  `MT_Substitute.cpp`, `SIG_GPManager.cpp`) and 6 MSVC German-locale file
  headers (`Schnittstelle für die Klasse` / `Implementierung der Klasse` /
  `Konstruktion/Destruktion`, all five `MT_GUI` pairs plus `MT_Tournament.cpp`).
  Nothing to verify — nothing executes.

- [ ] **11. Strings** — 11 lines in 4 files
  `MT_GPManager.cpp:277,332,333,341`, `SIG_GPIndividual.cpp:384,385`,
  `SIG_GPManager.cpp:627,1053`, `SIG_GPOperations.cpp:697`.

  **No compiler and no gate checks this phase.** All three persisted paths were
  checked and all three are safe:
  - `SIG_GPIndividual::writeToFile` writes history; `readFromFile` parses only
    `NAME='`, `POOLPOS=`, `FITNESS=`, `AGE=`, `PROGRAM BEGIN{`. History is never
    read back.
  - The `MT_GPManager` block sits after that file's own marker
    *"Additional information about the fitness computation ... is not loaded"*.
    Write-only.
  - The ZORC serial format (`printToString()`) carries no German.

- [ ] **12. Symbols** — 217 lines in 39 files, not user-visible
  Every miss is a compile error, so `./check.sh` verifies this phase in full.

  | symbol | where |
  |---|---|
  | `schlussJetzt` | `SIG_GPManager.h:136` + 5 uses |
  | `liesdas` (ctor param) | `SIG_Scanner.h:43`, `SIG_RobotScanner.h:48`, `SIG_UnstreamerScanner.h:34` + 3 `.cpp` |
  | `getRandomInstruktion`, `ProbInstruktion` | `MT_Randomizer.h:54,165` |
  | `T_Instruktion`, `T_Instruk` | `MT_TranslatedIndividual.h:38,74` |
  | `Instruktion` | `MT_Classifier.cpp:319` |
  | `set`/`getSelektionValue` | `MT_FitnessTrainer.h:109,115`, `MT_GPManager.h:68` |
  | `Varianz` | `MT_StatisticsElement.h:31` |
  | `winkel`, `verschiebung`, `schiebung`, `drehmatrix`, `hilf`, `stflorianhilf` | `IFunctions.h:38`, `IFunctions.cpp:321,380–397` |
  | `masse` | `SIG_Mirtich.h:80`, `.cpp:265,330` |
  | `dichte`, `konstante`, `anderes_material`, `rot`/`gruen`/`blau` | `SIG_RobotCompiler.cpp:167–236` |
  | `betrag`, `betraege`, `varianz`, `durchschnittProGelenk` | `SIG_GPForceFitnessFunction.cpp:105–122` |
  | `ausgabeTerrain` | `SIG_Environment.cpp:460–536` |
  | `zeiger` | `SIG_EnvironmentRenderer.cpp:447–455` |
  | `zahl` | `SIG_Geometry.cpp:53` |

  **The robot description grammar is entirely English** — `density`, `red`,
  `green`, `blue`, `friction`, `minimal_rot`. So the German names in
  `SIG_RobotCompiler.cpp` are locals holding the value of an English keyword and
  the target name is already written in the grammar. `rot` elsewhere
  (`getMinRot`, `rotMin`) is rotation, not the colour — leave it.

**Do not translate:** `Sigel.mak`, `sigel_slave.mak`, `manage_dyn_slave.mak`,
`Sigel.dsw`. MSVC-generated German, not built by this port. Delete them or leave
them; do not hand-edit generated files.

**Gate:** `./check.sh` after each phase, `./dictorder-dump.sh | diff -u
dictorder-baseline.txt -` empty, `fitness-check.sh` clean. Phase 10 cannot move
any of them, which is why it goes first.

**Not before Phase C and validation.** `MT_GPManager`, `MT_Classifier` and
`SIG_GPManager` carry most of the German and none of them compiles today — the
same bar as the two renames above.

## Set the version to 2.0 — the LAST step of the port

Do this only once the port is complete, running and validated against the 1.3
binary. It is the marker that the Qt 6 result is a different thing from what
2003 shipped, and it should be the final commit, not an early one.

**Where the version actually lives, measured — it is not where anyone would
look, and the tree disagrees with itself:**

| place | says | note |
|---|---|---|
| `kdesigel/configure.in:2` | `AM_INIT_AUTOMAKE(kdesigel,1.0)` | **the only real version declaration in the whole tree.** The 1.3 release still calls itself 1.0 here |
| `kdesigel/README:1` | `KDESIGEL v1.1 Readme File` | a third number, in the file a user reads first |
| `kdesigel/kdesigel.kdevprj:36` | `kdevprj_version=1.3` | **not SIGEL's version.** This is KDevelop's own project-file format version, which happens to also be 1.3. **Do not touch it** |
| the source | nothing | there is no version constant, and no binary prints one |

So "1.3" exists only in the tarball name and the directory name. Three files
carry three different numbers and none of them is 1.3.

**Do:** set `configure.in` to 2.0, and the README's heading with it. Then
decide whether the code should carry a version at all — today nothing prints
one, so `sigel --version` does not exist and a user cannot tell which build
they are running. Adding one is a small job and would be worth doing in the
same commit, but it is a new feature rather than a rename, so it is a separate
decision.

**Do not** change `kdevprj_version`. It describes the file's own format and
KDevelop reads it.

## Two Qt 6 noises the port keeps on purpose — found by C10, 2026-09-02

Both are 1.3 code preserved verbatim. Neither changes what the program does,
and neither should be "cleaned up" while the port is still being trusted
against the 1.3 binary.

**The wildcard disconnect.** `SIG_AllIndividualsView::slotEvolutionNotRunning`
begins each branch with `QObject::disconnect( individualList->
listviewIndividuals, 0, 0, 0 )`, which is byte-identical to the 2003 source.
Qt 6 prints `QObject::disconnect: wildcard call disconnects from destroyed
signal of QTreeWidget::listviewIndividuals` on stderr each time an evolution
starts. The disconnect still does what it is meant to; Qt 2 simply did not
warn about the pattern.

**Do:** name the three signals being disconnected, which is what the code
below the call immediately reconnects anyway. **Do not** do it before the
`gui behaviour` gate has a run under an actual evolution — that path is the
one C10 could not exercise, so a change there is currently unguarded.

**The history block grows by one line per individual per save.**
`SIG_GPIndividual::readFromFile` takes everything between `HISTORY BEGIN{` and
`}HISTORY END` as a SINGLE string — `history.clear()` is commented out in both
versions — and `writeToFile` emits `history.join("\n")` followed by a fresh
`"\n      }HISTORY END;"`. The chunk already ends with the previous save's
indent, so each round trip adds one `      ` line to every individual: measured
here as 1 line becoming 2 becoming 3, 120 extra lines per cycle on
`twoBasesSimpleFitness2.exp`, and the code is identical in the 1.3 tarball.

**CONFIRMED ON THE 1.3 BINARY, so preserve it.** The oracle ran the same two
saves: the six-space line count goes 360 -> 480 -> 600 while `}HISTORY END;`
stays at 120, i.e. +840 bytes = 120 x 7 per save, one `"      \n"` per
individual. (The first save of a shipped file is +141 lines rather than +120,
because the writer also emits five keys the 2003 file lacks — `WITHHISTORY`,
`WITHTEXTURE`, `TEXTUREFILE`, `TEXALPHA`, `RESEVGEN`.)

**Do:** nothing. This is 1.3 behaviour and the port reproduces it exactly, which
is the correct outcome. It is recorded here only so that a future reader who
notices experiment files growing does not "fix" it and silently diverge from
1.3. If it is ever changed deliberately, that is a product decision and needs a
note in PORTING.md saying the port stopped matching 1.3 on purpose.
