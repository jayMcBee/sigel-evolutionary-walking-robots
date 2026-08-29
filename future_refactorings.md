# C++ modernization — working list

**Scope: C++ language level only.** Independent of the Qt port; do not combine
commits across the two.

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
  Files: `MT_FitnessTranier`, `MT_GPManager`, `MT_Interpreter`, `MT_Population`,
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
