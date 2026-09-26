# To do — SIGEL 2.0

Independent of the port, which is done. Do not combine commits across the two.

**Protocol:** one commit per item, each reviewed and approved. Items 1 and 2
are mechanical; everything from 3 on is discussed before it lands.

**Status:** `[ ]` open · `[~]` in review · `[!]` blocked. **Finished items are
not kept here.** They move to `PORTING.md`, which is the only log of what was
done; their numbers stay with them, because other items cite them.

**Counts are stale. Re-measure before acting on any item.** A 2026-09-02 audit
found item 1 is 6 sites not 7, item 2's lines are 125/505/607, item 5's table
sums to 322 not 313 and its per-module split has moved hard since Phase C, and
item 6 is 63 sites not 75. Paths are relative to `sigel/`, the source tree,
renamed and hoisted 2026-09-20.

**Three things here are proposed, not approved:** the section ordering, item 5's
per-module split, and item 8's blocked status.

---

## 1 · Make it legal C++

Constructs the language removed. A current compiler rejects them.

- [ ] **1. Pre-standard headers** — `<iostream.h>` → `<iostream>`,
  `<vector.h>` → `<vector>`. `SIG_GPIndividual.cpp`, `MT_GPManager.cpp`,
  `MT_Randomizer.cpp`, `SIG_RobotScanner.cpp`, `SIG_SimulationQueries.h`,
  `SIG_DynaMechsSimulationQueries.h`.
  Does **not** retire `shim/`: the Makefile puts it on every vendored library's
  include path, and 25 files in the libraries we build include one of the four
  headers — cv97 16, Dynamo 6, dynamechs 2, newmat09 1. *Re-measured 2026-09-20;
  this said 32.*

- [ ] **3. Loop-variable scope leaks** — 13 files in `src/MT_GPSystem/`:
  `MT_FitnessTrainer`, `MT_GPManager`, `MT_Interpreter`, `MT_Population`,
  `MT_Program`, `MT_Randomizer`, `MT_Search`, `MT_Statistics`,
  `MT_StatisticsElement`, `MT_Tournament`, `MT_TournamentManager`,
  `MT_TranslatedIndividual`, `MT_Trainingset`. Hoist the declaration.
  The pattern is `for (int i=…){…}` then `i` used after the loop.

---

## 2 · Let the compiler hunt bugs

- [ ] **5. Add `override`** — one commit per module. Highest value on this
  list: a method meant to override with a subtly wrong signature silently
  becomes a new function. **Read every failure; do not fix mechanically.**
  Needs base headers to parse, so it needs tooling rather than an editor. The
  failure shape is a missing `const`, or `int` against `long`.
  **The table below is pre-Phase C and the split has moved hard:** `MT_GUI` 6 to
  24 and `SIGEL_MasterGUI` 5 to 38, because Phase C's uic3-derived base classes
  declare `virtual` slots. Re-measure before planning the commits.

  | module | sites | module | sites |
  |---|---|---|---|
  | SIGEL_RobotIO | 164 | MT_Control | 8 |
  | SIGEL_Robot | 40 | SIGEL_Visualisation | 7 |
  | SIGEL_GP | 32 | MT_GUI | 6 |
  | SIGEL_Simulation | 28 | SIGEL_MasterGUI | 5 |
  | MT_GPSystem | 17 | SIGEL_Program, SIGEL_SlaveGUI | 3 |
  | SIGEL_CommonGUI | 12 | | |

- [ ] **6. `NULL` → `nullptr`** — mechanical, one commit.

- [ ] **7. `explicit` on single-argument constructors** — per module. Every
  break is an implicit conversion that was happening silently.

- [ ] **92. Remove the `const_cast`s where the API allows it, and the `(void)`
  parameter lists.** Counted 2026-09-25.
  - **25 `const_cast`s in six modules.** None is undefined behaviour today:
    each object was created non-const and is only read through the cast. But
    a later write through one would be, and the compiler would not say so.
    - **Keep, 10:** PVM's C functions take `char*` for strings they only
      read: `SIG_GPFitnessTrainer` (8), `SIG_GPPVMData::sendQStringToPVM`
      (1), `SIG_AllIndividualsView::slotVisualize` (1).
    - **Meta-GP, 9:** the three tournament classes and `MT_Evaluator` cast
      programs and an individual because `MT_Classifier::createNewTCase` and
      `classifier` take non-const pointers. Make those take `const` if they
      only read.
    - **`getLine`, 2:** `SIG_Interpreter::interprete` and
      `SIG_ProgramLine::operator=`; item 91.
    - **Robot and environment, 4:** `SIG_DynaMechsCommandInterface::moveDrive`
      (`SIG_Joint` from a const drive), `SIG_DynaMechsLink` (`SIG_Geometry`),
      `SIG_EarlyRunTermSimulation` (keeps a mutable robot pointer),
      `SIG_EnvironmentRenderer` (keeps a mutable environment reference).
      Needs const getters on `SIG_Joint` and `SIG_Geometry`, and the two
      stored references made const.
  - **289 `(void)` parameter lists in 61 files**, a C habit: `SIGEL_Robot`
    (40 files), `SIGEL_RobotIO` (14), `SIGEL_GP` (4), `SIGEL_Simulation` (3).
    `f(void)` becomes `f()`; mechanical, one commit. No `(void)x` casts were
    found.

---

## 3 · Ownership

- [!] **8. Smart pointers** — 554 `new` against 127 `delete`.
  **Blocked.** Correct and broken both compile clean, and no flag verifies it.
  Prerequisites in order: one experiment running end to end, a fixed seed, a
  recorded fitness trajectory, then one class at a time with the trajectory
  bit-identical. Do not start before that exists.

---

## 4 · Program display

- [ ] **9. Syntax-highlight the program view.** New
  `programToHtml(const SIG_Program &, const SIG_LanguageParameters &)` returning
  a `QString`. `printToString()` stays as the ZORC serial format.
  `printToString()` must keep serving `SIG_GPRemoteZORCFitnessFunction.cpp,
  evalFitness`.
  `<pre>` wrapper, one `<span>` per token, line number per line. Colour opcodes
  by group: arithmetic `ADD SUB MUL DIV MOD MIN MAX`, data `COPY LOAD`, control
  `CMP JMP`, robot `MOVE SENSE DELAY`. Registers print as `R0`–`R7`, computed as
  `element % getMemorySize()`; `LOAD` operand 2 and `JMP` operand 1 print as
  literals. Feed it to
  `QTextBrowser::setHtml()` in `SIG_SimulationWidget::visualizeThis`,
  `SIG_IndividualView::SIG_IndividualView`,
  `SIG_AllIndividualsView::slotVisualize`.

---

## 5 · Renames and translation — after the port is validated

- [ ] **10. Rename the two `SIG_GPExperiment` variants.**
  `SIG_GPExperiment.cpp` builds `sigel`, `SIG_GPExperimentClean.cpp` builds
  `sigel_slave`, both define the same class name and share the include guard
  `SIGEL_GP_SIG_GPEXPERIMENT_H`. It has already been misread as an accidental
  duplicate. **The master keeps `SIG_GPExperiment`; the slave's gets the new
  name.** Distinct guards too, so the compiler enforces what the build files only
  imply. While there: the Makefile globs `src/<module>/*.cpp` and compiles both
  into `libSIGEL_GP.a`, where 2003 built separate targets.
  **Move `$(MASTER_OBJ)` with it.** The Makefile defines it as
  `SIG_GPExperiment.o` and `guidrive` names it and asserts on it. Linking the
  Clean variant produced a convincing false crash in C10.

- [ ] **11. Rename `tours` to `tournaments`.** `SIG_GPManager::tours` holds
  every `SIG_GPTournament` of one generation — the type already says the word.
  It is private and used in one file. **Three** doxygen comments in
  `SIG_GPManager.h` name it and go with it. Crosses both `SIG_GPExperiment`
  variants, so do it with item 10.

- [ ] **12. Translate the German — comments.** 118 lines in 46 files, including
  15 `NEU NEU NEU` banners and 6 MSVC German file headers. Nothing executes.

- [ ] **13. Translate the German — strings.** 4 lines in 1 file are left, all
  **kept, by decision:** the history text of `SIG_GPIndividual`, which is saved
  in `.exp` files. "Fitness (Elter 1)" and "(Elter 2)" in `addCrossOverInfo`,
  "CREATED NEW INDIVIDUUM" in the constructor and "INDIVIDUUM IS GENERATED
  RANDOMLY" in `generateRandomIndividual`. The shipped experiments hold the
  "Elter" lines.
  **No compiler and no check covers this phase.** All three persisted paths are
  write-only: `SIG_GPIndividual::readFromFile` parses only `NAME='`, `POOLPOS=`,
  `FITNESS=`, `AGE=` and `PROGRAM BEGIN{`; the `MT_GPManager` block sits after
  that file's own "is not loaded" marker; the ZORC format carries no German.
  **Phase 0 could not have caught these** — they are pure ASCII, and Phase 0
  swept for bytes above 127. A check for German must look for words, not bytes.

- [ ] **14. Translate the German — symbols.** 214 lines in 38 files, not
  user-visible. Every miss is a compile error, so `check.sh` verifies it.

  | symbol | where |
  |---|---|
  | `getRandomInstruktion`, `ProbInstruktion` | `MT_Randomizer.h` |
  | `T_Instruktion`, `T_Instruk` | `MT_TranslatedIndividual.h` |
  | `Instruktion` | `MT_Classifier.cpp, createDoubleTransIndi` |
  | `set`/`getSelektionValue` | `MT_FitnessTrainer.h`, `MT_GPManager.h` |
  | `Varianz` | `MT_StatisticsElement.h` |
  | `winkel`, `verschiebung`, `schiebung`, `drehmatrix`, `hilf`, `stflorianhilf` | `IFunctions.h`, `IFunctions.cpp` |
  | `masse` | `SIG_Mirtich.h`, `.cpp` |

  **Kept, by decision:** `sliderIntervall` and `slotIntervallChanged`, German
  spelling of "interval". The GUI baselines record the widget by name, and the
  form connects the slot by name.

  The robot grammar is entirely English — `density`, `red`, `green`, `blue`,
  `friction`, `minimal_rot` — so the German names in `SIG_RobotCompiler.cpp`
  hold the value of an English keyword and the target name is already written in
  the grammar. `rot` in `getMinRot` and `rotMin` is rotation, not the colour —
  leave it.

  **Umlauts are Latin-1 bytes.** A UTF-8 grep misses them. Comments first,
  because that phase cannot move a baseline.
  **Do not translate `Sigel.mak`, `sigel_slave.mak`, `manage_dyn_slave.mak` or
  `Sigel.dsw`** — MSVC-generated German, not built here. Item 35 deletes them.
  Do not hand-edit generated files.
  **Check after each phase:** `./checks/check.sh`, then
  `./checks/dictorder-dump.sh | diff -u checks/baselines/dictorder-baseline.txt -`
  empty, then `./checks/fitness-check.sh` clean.

- [ ] **15. Rename the `act` prefix to `current`.** German `aktuell`; reads as
  the verb "act". **`actExperiment` is already done** — 115 sites, renamed
  2026-09-16 along with `actExpChanged` and `slotActExpChanged`, a signal and a
  slot whose string-based connects went with them. **636 occurrences over 49
  distinct names remain**, measured over `src/` and `include/`: the largest are
  `actTour`, `actRealPosition`, `actInd`, `actSuccessor`, `actFitness` and
  `actFitTask`. Re-measure before starting.
  **Settle the scope first.** Most of what is left is in `SIGEL_GP`, which D33
  keeps untouched for behaviour; a rename is not behaviour, but it is still a
  large diff in a frozen module, so it needs sign-off before it is done at all.
  Any signal or slot in the set breaks its string-based connect if only one side
  moves — `check.sh` has a check for exactly that failure. No reference file
  holds these names.

- [ ] **16. Set the version to 2.0.** Since 2026-09-25 the version SIGEL
  shows is declared once, in `SIGEL_Tools/SIG_Version.h`, as the
  work-in-progress "1.4". The About box reads it, and its logo is
  `newLogoNoVersion.png`, with no version drawn in. Left: `sigel/README`
  still says `KDESIGEL v1.1 Readme File`; its options are still to be
  discussed. `pixmaps/altLogo.png`, with a `Sigel v1.0` caption, is kept
  but unused. Set "2.0" in `SIG_Version.h` when it is time.

---

## 6 · Defects preserved by the port

All present in 1.3, none introduced here. Each needs a decision before it is
touched, because changing one changes behaviour against the reference binary.

- [ ] **18. `SIG_GPPVMTask` holds a reference to a host that can be deleted
  under it.** `SIG_GPPVMTask` declares `SIG_GPActivePVMHost &host`;
  `SIG_GPFitnessTrainer::flushAllDynHosts` calls `resizeOwningHosts`, which
  deletes host objects. An outstanding task then decrements a freed object. Latent: `addDynHost` is
  called only from `SIG_GPManager::RegisterDynPVMClients`, on the dynamic-client
  thread that only `sigel.cpp` starts, and `guidrive` starts no such thread.

- [ ] **19. `getNextHost`'s mutex is a function local and locks nothing.**
  `SIG_GPFitnessTrainer::getNextHost` initialises, locks and unlocks a local
  `pthread_mutex_t`, so two threads exclude each other from nothing. The section
  it guards moves entries out of `freshDynHosts`, written by the dynamic-client
  thread — the one place in the trainer where a lock was wanted. The comment
  above it, "now we make ourself running exclusively", is false and goes with it.
  Arrived with 1.3; 1.0 has no dynamic-host feature.

- [ ] **20. `renderRecorder` leaks when the visualisation constructor throws.**
  `SIG_SimulationVisualisation::SIG_SimulationVisualisation` allocates it, then
  constructs `SIG_Simulation`, which throws for the removed Dynamo backend. The
  destructor is the only thing that deletes it and never runs, so each attempt
  leaks one `SIG_RenderRecorder`. The other half was fixed by nulling
  `visualisation`. Becomes live the moment any caller catches that throw and
  continues.
  **Assessed 2026-09-23, read-only.** Latent today: the only caller that
  catches the throw, `sigel_slave`'s `main`, returns at once. Three fixes, none
  changes behaviour: `renderRecorder` as a `std::unique_ptr` (the destructor
  must still delete `simulation` first, because `SIG_Simulation` holds a
  reference to the recorder); a try/catch in the constructor that deletes it and
  throws again; or a value member. *Larger and related:*
  `SIG_Simulation::~SIG_Simulation` is empty, in 1.3 too, so every Stop in the
  viewer leaks the whole simulation — the §10 leak in PORTING.md.

- [ ] **53. DynaMechs returns uninitialised forces for end links.**
  `dmArticulation::getForces` copies the `f_star` of each movable link. But
  `dmArticulation::ABBackwardDynamics` never writes the `f_star` of a link
  without children: `ABBackwardDynamicsN` writes into the parent's `f_star`, or
  into a temporary. `dmArticulation::addNode` creates the struct with
  `new LinkInfoStruct` and does not zero it. So the values for end links are
  uninitialised memory, and `SIG_GPForceFitnessFunction::evalFitness` uses them.
  It matters only for experiments that select `ForceFitnessFunction`; none of
  the 7 shipped experiments does. The library code is unpatched, as SIGEL's
  `supportingLibs` ships it. Found 2026-09-22.

- [ ] **59. Investigate the pool limit of 32768.** Tournament selection draws
  each player's pool position with `SIG_Randomizer::getRandomInt`, which
  returns 0 to 32767 before the modulo. `SIG_GPManager::createTours` draws with
  `getSize()`, `getSize()-1`, `-2` and `-3` and shifts the later draws past the
  earlier ones, so it reaches index 32770 at most: only the first 32771
  individuals can ever play. The pool itself can grow past that, through
  repeated Adds or a loaded file, whose `POPULATIONSIZE` and `INDIVIDUAL(x)`
  indexes both grow it. An individual further down is evaluated — every
  generation, because `resetAllFitnessValues` clears its fitness — but never enters a
  tournament.
  **The modulo also favours low positions** at every pool size that does not
  divide 32768: 4 draws against 3 for the first 2471 positions of a pool of
  10099, and 2 against 1 for any pool of 16385 to 32767.
  The generator is 1.3's, and its bits are the same on a 32-bit machine, so
  the limit is 1.3's too; confirm that on the oracle. All seven shipped
  experiments hold 100 individuals, so none comes near it. To decide: leave it
  as 1.3 has it, refuse a larger pool, or draw differently — and the last
  makes every run differ from 1.3's. Found by review 2026-09-22.
  **Pencilled in 2026-09-24:** limit the pool to 30,000 in the interface, and
  add a check with a warning in the code for any pool above 32,767.

---

## 7 · The interface

- [ ] **94. Adding many individuals is slow, and slower as the pool grows.**
  Observed 2026-09-26: Individuals > Add with 1000 individuals takes a long
  time, and the rate drops as the pool grows. Creating random programs should
  be quick. Not measured yet. What the code does: after
  `SIG_GPPopulation::addRandomIndividuals`,
  `SIG_AllIndividualsView::slotAddIndividuals` clears the whole list and
  builds every row again, with a progress dialog update and a
  `processEvents` call per row. Measure first, with a profiler or gdb,
  where the time goes: the random programs, the list rebuild, or the
  sorted inserts into the list.

---

## 8 · Cleanup

- [ ] **29. SIGEL needs a real logging system.** Qt 2's `QTextStream` wrote
  straight through to unbuffered `stderr`; Qt 6 buffers and flushes only on
  `flush()`, `Qt::endl`, overflow or destruction. **A trailing newline does not
  flush.** `SIGEL_Tools::SIG_IO` still declares `cin`, `cout` and `cerr` as
  plain `QTextStream`, so every diagnostic inherits it. 475 mentions, 35 lines
  that flush, **232 that end in a bare newline** — those are lost when the
  process dies, which is exactly when they are wanted.
  **Do not fix this by adding `Qt::endl` to 232 call sites.** It needs levels,
  one place that decides where output goes and when it flushes, and something
  the GUI can display. It replaces both `SIG_IO` and the console-warning
  stopgap. **Start at `SIG_IO::cerr`** — decided; it is the hook.
  **Review every dialog outside the interface modules as part of this.** The
  layering is not clean: classes that should have no user interface open
  dialogs themselves. `SIG_EnvironmentRenderer` shows three texture warnings;
  `MT_Controller`, `SIG_GPPopulation`, `SIG_GPFitnessTrainer` (one, commented
  out) and `SIG_GPRemoteZORCFitnessFunction` do the same. Decide for each
  whether it belongs in the interface or goes through the new error reporter.

- [ ] **30. Comments over two lines must earn their place.** A comment longer
  than two lines must carry something the code cannot say. **Comments the port
  itself wrote come first.** File by file, each pass signed off first.
  **The named instance:** the nine lines above `setIconSize( QSize( 25, 25 ) )`
  in `SIG_MainWindow::SIG_MainWindow`, all nine describing what Qt 2 did. Two
  lines carry the whole fact — Qt 6 has one icon size per toolbar, and 25 is the
  largest of the small pixmaps, so nothing is scaled past what 1.3 drew.

- [ ] **32. Twelve redundant `setEnabled` lines.**
  `SIG_GUIGPExperiment::slotStartEvolution` and `slotEvolutionStopped` each set
  six widgets for their own experiment; `slotEvolutionNotRunning` now sets the
  same widgets for every experiment from the same signal with the same value.
  `pushbuttonStop` is **not** one of them and must stay — nothing else enables
  it. Each of the two slots becomes one call plus its own `pushbuttonStop` line.

- [ ] **33. Six overwrite prompts D35 wants deleted.**
  `MT_StatisticsWidget` prompts at six export sites after a `getSaveFileName`
  that already asks. Same family: `MT_PopulationWidget::slotExpInd` and
  `slotSavePop`, `MT_IndividualsWidget::slotExportConstants`,
  `MT_Controller::slotSaveSetup`. The six in `MT_StatisticsWidget` pass no
  parent; item 25 left them for this item. `slotSaveSetup`'s prompt got a
  parent in item 25 and is still to go; it is in `MT_Controller`, and D33 says
  only that a dialog's parent is outside it, not whether deleting a prompt is.
  **Delete rather than re-parent them.**

- [ ] **34. `tearDownPvm()`'s `pvm_halt()` never returns.** It sends `TM_HALT`
  and waits for a reply the daemon never sends; the daemon's `pvmbailout()` then
  SIGTERMs every local task, this process included, because it enrolled with
  `pvm_mytid()`. `guidrive` now survives that with a SIGTERM handler
  (`keepExitCodeThroughPvmShutdown`), so a scenario's status is readable.
  **The other half is open:** once that handler holds a status of 0 the process
  reports 0 for any SIGTERM, including a person killing a wedged `guidrive`.
  A teardown flag, or preserving only a non-zero status, closes it.
  **Do not simply delete the call.** `pvm_halt()` is what stops the daemon this
  process started; dropping it left `pvmd3` and its slaves running. It does not
  run at all when `g_pvmOurDaemon` is false — with a daemon already up there is
  no halt, no SIGTERM, and the status was always readable. No stray `pvmd3`
  survives PVM's own shutdown, so today's behaviour is safe, only untidy.
  Matters before `check.sh` ever runs one of the three PVM scenarios.
  *Any printf on an early-return path here is lost unless it flushes itself.*
  **Closing `sigel` with SIGTERM does not end it either.** Measured 2026-09-21:
  `sigelStandardSignalHandler` calls `pvm_halt()`, Qt then reports a `QThread`
  destroyed while running, the handler's SIGABRT branch prints `Abort` and calls
  `pvm_halt()` again, and the process stayed until SIGKILL.

- [ ] **61. The terrain is the transpose of its floor function.** Found
  2026-09-23 in the item 42 assessment, read-only.
  `SIG_Environment::generateTerrain` writes the rows z-major; DynaMechs'
  `dmEnvironment::loadTerrainData` reads them x-major. So the floor is f(z, x),
  not f(x, z), and a floor whose X and Z sizes differ is misread. Physics and
  drawing read the same array, so the view stays true to the physics. In 1.3
  too; no shipped experiment sets `FLOORDIMENSION`, so none reaches it. A fix
  changes physics for asymmetric or non-square floors, so it needs a decision.

- [ ] **64. Remove what is left of Dynamo.** Asked for 2026-09-23. The Dynamo
  backend was deleted on 2026-08-28; PORTING.md, "Dynamo removed, DynaMechs
  kept", has the details. The original project dropped Dynamo as its physics
  engine early and kept only some of its classes
  (https://sigel.sourceforge.net/seiten/links_en.html). Two parts are left:
  - **The choice of Dynamo in the interface and the model.** The "Dynamo  (not
    recommended)" radio button and the `DynaMo` tab in
    `SIG_SimulationParameterBase.ui`, a second `DynaMo` tab in
    `SIG_EnvironmentBase.ui`, the write in
    `SIG_SimulationParameter::putIntoExperiment` and the read-back in
    `getOutOfExperiment`, the `DynaMo`
    value of `SIG_SimulationParameters::SimulationLibrary`, the `DynaMo` cases in
    `SIG_GPFitnessTrainer`, `SIG_AllIndividualsView` and `sigel_slave.cpp` that call
    `SIG_Robot::prepareDynaMo`, `SIG_Link::transformToDynaMo`, and
    `SIG_Simulation::slotDynamoMessage`, and the DynaMechs check in
    `SIG_GUIGPExperiment::slotRobotInfo`. Today the interface can still make an
    experiment that the simulation refuses. `SIG_Simulation` throws for it,
    which is one way into item 20; the other, a `SIG_CannotMirtich` from a
    bad mass, stays after Dynamo goes. Also Dynamo's: the `stopSimulation`
    flag, which only `slotDynamoMessage` sets, and so the throw in
    `SIG_Simulation::makeTimeSteps`, the class
    `SIG_SimulationCannotSolveException` and its two catch sites in
    `SIG_Simulation.cpp`; four files include its header, so they change
    with it.
    `SIG_SimulationParameters::writeToFile` saves Dynamo's settings, which only the
    parameter dialog reads: `MAXIMALERROR`, `MAXIMALITERATIONS`, `SKIPFRAMES`,
    `ANALYTICAL`, `MAXIMALCOLLISIONLOOPS`, `SOLVEMODE`, `INTEGRATOR` and
    `MAXIMALSOLIDITERATIONS`. `SIMULATIONLIBRARY` is read by the simulation
    too, and loses its Dynamo value only. `STEPSIZE` stays: DynaMechs uses
    it. All 7 shipped experiments hold these lines, so removing
    them changes the file format, and the maintainer decides.
  - **The maths library `libdynalib.a`.** SIGEL is built on its `DL_vector`
    and `DL_matrix`. PORTING.md, "Follow-up this change deliberately did not
    take", point 3, has the plan: a small local header in its place.
  Doc comments that name Dynamo, in `SIG_SimulationParameters.h`,
  `SIG_Simulation.h`, `SIG_SimulationCannotSolveException.h`, `SIG_Robot.h`,
  `SIG_Link.h`, `SIG_Material.h` and `SIG_Body.h`, go with the code they
  describe. The comment on the guard in
  `SIG_SimulationVisualisationWidget::visualizeThis` names Dynamo too; the
  guard stays, for `SIG_CannotMirtich`.

- [ ] **68. Start the robot in the middle of the terrain?** Raised
  2026-09-23 with item 42, for discussion later. The robot starts at the
  experiment's `STARTPOSITION`; x and z are 0 in every shipped experiment,
  which is the terrain's corner. Moving it is a change to the experiment setup: either
  `STARTPOSITION` in the `.exp` files, a file change, or the terrain's place
  in DynaMechs, a vendor patch. Positions and fitness move with it.

- [ ] **72. A slave result of exactly -1.0 hangs the run.** Found 2026-09-23
  by reading `SIG_GPFitnessTrainer::checkTask` and its callers in
  `SIG_GPManager`: `checkTask` receives the result and deletes the task, but
  -1 is also its "not ready" value, so every caller keeps polling a task that
  no longer exists, for ever. Never observed.

- [ ] **73. Latent sensor bugs in `SIG_DynaMechsSimulationQueries::sense`.**
  Found 2026-09-23 with the two sensor fixes. No shipped robot or experiment
  triggers any of them: every shipped sensor is a joint sensor on a joint with
  two different limits.
  - **A joint with equal limits reads NaN.** `SIG_Joint::calculateMDH` gives
    `mechsMinPos == mechsMaxPos` when the limits are equal or normalise to the
    same angle (0/360, -180/180), so `posRange` is 0 and `scaledState` is 0/0.
    The cast of NaN to `int` is undefined behaviour. The `±DBL_MAX` test in
    `sense` never matches, because `SIG_Joint` never stores those values.
  - **Pitch at 90° or more reads as exactly 1.** Pitch comes from `acos`, 0° to
    180°, and the branch clamps to ±90°, so the upper half of the range reads
    the same. `acos` and `asin` get no clamp against rounding past ±1, which
    gives NaN.
  - **An uninitialised value is loaded.** With no contact model
    (`dmRigidBody::getForce(0)` is NULL), or on the `default:` branch,
    `registerValue` is never set and `loadValue` still reads it.

- [ ] **74. A timed-out individual is evaluated again, for ever.** Found
  2026-09-24. When `TIMEOUTMINUTES` expires, `SIG_GPFitnessTrainer::checkTask`
  kills the slave and puts the same individual back on `toSpawnList`; the
  probe-error branch does the same. It never gives up. On one machine the same
  individual gives the same simulation, so an individual that is too slow is
  likely to time out again at every attempt, and the generation does not end.
  1.0 has the same timeout and the same re-spawn. The shipped experiments set
  0, 10 or 30 minutes. No rule exists for when to give up on an individual, or
  what score it then gets.

- [ ] **75. The position check in the fitness functions rejects only Inf and
  NaN.** Observation, 2026-09-24; no fix decided.
  `SIG_GPFitnessFunction::isValid`, applied to each coordinate of a robot
  position, accepts any finite value. `SIG_GPSimpleFitnessFunction` scores the straight distance
  from start to end over the simulated time, so a finite physics blow-up that
  throws the robot far scores high. `SIG_GPNiceWalkingFitnessFunction` checks
  every recorded position against a minimal and maximal height, but not
  against horizontal jumps. The `sigel-x86` session reported two fitness
  functions, not named in its report, defeated by individuals scoring 268 and
  292 on 1.3; not re-measured here. Those runs used several slaves on one
  host, and a 249.503 found the same way turned out to be the shared
  `Terrain.ter` race, fixed in 2.0: the program scored 0.0327516 with one
  slave. PORTING.md has the measurement with the `Terrain.ter` fix. So the
  1.3 scores are no evidence for this item; it rests on the code alone.
  Any bound is a heuristic and needs more thought.
  **Related, found 2026-09-24:** `SIG_GPAdaptiveWalkingFitnessFunction`
  already tests each recorded height. Above 1.5 times the start height it logs
  "o quite high", above 2 times "r u ready 2 fly ?". Both are only written to
  `SIG_IO::cerr`, once per recorded sample, and do not change the score.

- [ ] **76. A mesh with negative volume loads with no message.** Found
  2026-09-24. `SIG_Mirtich::computePhysics` computes a link's mass as density
  times the mesh volume, and throws `SIG_CannotMirtich` only when the centre
  of mass, the inertia tensor or the mass is NaN. A mesh with inverted face
  winding has a negative volume, so its mass and inertia are negative; the
  robot loads and the simulation runs into NaN without a message. A warning
  at load, naming the link and the mesh file, is one option.

- [ ] **77. A robot checker.** Idea, 2026-09-24; the name is still open:
  checker, advisor or sanity check. It reads a robot model and its Language
  Parameters and warns about what will make evolution fail or mislead.
  - **Joint-limit stability per joint**, `timestep * sqrt(K / I)` and
    `timestep * damper / I`. Reported by the x86 session on 1.3, not
    re-measured here. The shipped models sit in a narrow band. A
    `JOINTLIMITSK_SPRING` of 25000, taken from a one-joint robot onto a
    three-joint chain, put the limit-spring torque above half the full drive
    torque, with no warning.
  - **Drive strength against weight per drive**,
    `maximalforce / (mass * g * half-length)`. Same source. It is 0.60 to
    0.92 in all seven shipped models, whose masses span 1.2 to 49; nothing
    documents it, so a new model can be badly under- or over-powered.
  - **The following come from reading 1.3's `SIG_Interpreter::interprete` and
    `SIG_DynaMechsCommandInterface::moveDrive`, 2026-09-25.** The port is
    expected to match; not checked yet.
    - **Unreachable drives.** MOVE picks drive
      `(register value + 2^(w-1)) % number of drives`, w the register width.
      With more drives than 2^w values, some drives can never move.
    - **Unevenly chosen drives.** With 2^w not a multiple of the number of
      drives, the first drives get one register value more: 43 against 42
      for 6 drives at 8 bits. It grows as w shrinks.
    - **A register width of 1 breaks force drives:** the force divides by
      2 * (2^(w-1) - 1), which is 0.
    - **Torque resolution.** A force drive gives
      `maximalforce * R0 / (2^(w-1) - 1)`; a small w gives few torque levels.
    - **`minimalforce` is a dead band.** Torques below it are not applied. A
      minimum above the maximum means the drive never acts; a minimum large
      against one torque step makes most register values do nothing.
    - **Servo drives read the register value as an angle in degrees**,
      clamped to the joint's limits. With 8 bits only -128 to 127 degrees
      can be reached.
    - **Drive numbers are the order in the model.** Reordering, adding or
      removing a drive silently changes which joint an evolved program moves,
      so an existing population stops meaning what it did.
    - **SENSE picks its sensor from a register value too;** the same two
      reach and balance checks likely apply to sensors. Not read yet.
  - **Program facts to show beside it, not model defects:** every MOVE takes
    its torque from R0, so a program that moves several drives must reload
    R0; and a negative register operand is taken modulo an unsigned register
    count, so `MOVE -128` with 24 registers reads R8, not a register near
    R0.

- [ ] **81. A fitness function that rewards steady walking.** Idea, 2026-09-24.
  No existing function rewards an even pace: `SIG_GPNiceWalkingFitnessFunction`
  only tests a fixed height band, pass or fail, and all but
  `SIG_GPRealSpeedFitnessFunction` score net distance from start to end.
  Decided so far, preliminary name "Steady Walking",
  `SteadyWalkingFitnessFunction`, class `SIG_GPSteadyWalkingFitnessFunction`:
  - split the run into 1 s windows; drop a partial last window;
  - pᵢ = horizontal progress in window i along the line from the run's start
    to its end, so no direction is fixed, as in the existing functions;
  - v̄ = mean of pᵢ per second; cv = std(pᵢ) / v̄;
  - h = std(body height) / start height;
  - score = v̄ / (1 + cv) / (1 + h), the height weight being 1;
  - no settling time at the start; a full run, never the early-stop
    simulation;
  - 0 when v̄ is 0 or below, or a position is Inf or NaN.
  It records every frame with `SIG_GPFullDataRecorder`, as Nice Walking does,
  and needs one entry in the GP parameter combo box and one in
  `sigel_slave`'s name mapping. No file format changes.

- [ ] **83. ZORC support behind a compile-time switch, off by default.**
  Decided 2026-09-24. ZORC is a real robot driven over a serial line; the
  simulation does not need it. One global `#define`, off by default, removes
  everything ZORC-related from the build: `SIG_GPRemoteZORCFitnessFunction`,
  its branches in `sigel_slave.cpp`'s `main` (the fitness function and the
  visualise-mode program transfer), and the "Remote ZORC" entry in the combo
  box, handled in `SIG_GPParameter` like item 82's index shift, which is
  in PORTING.md's Done entry for item 82. Not ZORC:
  `SIG_GPAdaptiveWalkingFitnessFunction` is a simulated function whose stored
  name happens to be "ZorcWalkingFitnessFunction"; it stays. To decide: where
  the switch lives (the `Makefile` or a header), and what an experiment file
  naming "RemoteZORCFitnessFunction" does when the switch is off.

- [ ] **84. Two defects found reading the fitness functions.** 2026-09-24.
  - **An unknown fitness-function name crashes `sigel_slave`.** In
    `sigel_slave.cpp`'s `main`, the last `else` prints "Error: Unknown fitness
    function!" and leaves `fitnessFunction` null, but `setActGeneration` and
    `setResetEveryGeneration` are called on it before the
    `if (fitnessFunction)` test.
  - **A friend class that does not exist.** `SIG_GPFullDataRecorder.h`
    declares `friend class SIG_GPEnergyFitnessFunction`; no such class is in
    the tree. Delete the line.

- [ ] **85. A short description of the selected fitness function.** Idea,
  2026-09-24. Below the fitness-function combo box on the GP Parameters page,
  a label of at most two lines says what the selected function rewards, in
  the style of the Edit host dialog's line "For best speed, use one process
  per CPU core". The text belongs to the fitness function, not to the
  interface: each class gets a const getter returning it (name to decide,
  for example `uiDescription()`). It must work without an object, because the
  master never creates a fitness function; only `sigel_slave` does. So a
  static member, or one lookup by the stored name next to the one
  `sigel_slave`'s `main` does. `SIG_GPParameter` updates the label when the
  selection changes. Write the texts after items 82 and 83, so the dropped
  functions get none.

- [ ] **90. "Stop at generation N" counts from each start.** Found
  2026-09-24 with item 86. `SIG_GPManager::checkTerminationConditions`,
  called from `stopIfNecessary`, compares the termination generation with
  `currentGenerationNo`, which counts from 0 at
  every start, not with the pool generation that the Experiment page and the
  `.exp` file show. A run restarted at pool generation 1034 with a
  termination generation of 1100 would stop at pool generation 2134,
  by reading the code. Changing it changes when a run stops; not decided.
  `SIG_GUIGPExperiment::terminationAlreadyMet` tells the user the run
  "terminates after N generations", a count per run; its text changes with
  it.

- [ ] **91. Make `SIG_ProgramLine`'s getters const, then `getLine`.** Found
  2026-09-25 by the review of the `SIG_Program` const step. None of these
  change the line: `getRobotinstructionType`, `getInstructionElement`,
  `getElement`, `getNumberOfElements`, `getElementsArray`, `printToString`,
  `print`, `writeToFile`. With them const, `SIG_ProgramLine::operator=` loses
  its `const_cast`, and `SIG_Program` can offer
  `const SIG_ProgramLine *getLine( long no ) const` beside the non-const one
  that `SIG_GPOperations`' mutation needs; `SIG_Interpreter::interprete` then
  loses its last `const_cast`. `SIG_GPIndividual::exportProgram` and `print`
  can use `getProgram()` instead of `getProgramVar()`. `SIG_Program::print`'s
  comment says stdout; it writes to `SIG_IO::cerr`.

- [ ] **89. Refuse bad simulation parameters.** Found 2026-09-24 in item 87;
  needs more thought. Two values reach divisions with nothing to stop them:
  - **Time to simulate of 0:** every simulated fitness function divides the
    distance by `QTime(0,0).secsTo(getTimeToSimulate())`, the run time in
    whole seconds. The three time spin boxes each go down to 0, and a loaded
    file can carry less than 1 s. The result is `inf`, or `NaN` for a robot
    that does not move. What the GP does with that fitness is not measured.
  - **Step size of 0 or less:** Real Speed and `SIG_SimulationVisualisation`
    divide 0.5 by it, `SIG_SimulationVisualisationWidget` divides 5 by it,
    `SIG_EarlyRunTermSimulation` divides the run time by it, and DynaMechs
    integrates with it. A step of 0 makes `int(inf)`, undefined behaviour, in
    Real Speed. For the other functions the run never ends:
    `SIG_DynaMechsSimulationQueries::getActualSimulationTime` is frames times
    the step, so the time stays at 0 and `SIG_Simulation::start` loops. What a
    negative step does is not measured. `lineeditStepSize` has a
    `QDoubleValidator` with no range, and `SIG_SimulationParameters` loads any
    number.

  **Where to refuse, not decided:** (1) a validator range on the field, which
  stops typing only and needs a smallest step; (2) on load, in
  `SIG_SimulationParameters`, like item 71, but any load error kills the
  interface until item 88 is done; (3) when a run starts, with a message box,
  which catches typed and loaded values and kills nothing.

- [ ] **47. `sigelDynClient` and `manage_dyn_slave`.** `sigelDynClient` makes a
  second machine a dynamic slave of a master started with `sigel -de`, which
  `sigel.cpp` still accepts. It is still 1.3's Solaris `tcsh` script, its home
  folder paths are placeholders, and it runs `manage_dyn_slave`, which 1.3 built
  and the port does not. **Modernise in place, do not replace.** It needs a
  second machine to prove it on; the 1.3 reference machine is not ours to use
  for tooling, so it waits until there is one.

- [ ] **36. `SIG_Body::usedByLinks` is dead, and
  `SIG_Material::FrictionValue` could be a value type.** Left converted rather
  than changed during D11, by decision, because the port moves the Qt API and
  nothing else — which is a port-scope rule, not a refusal, so both belong here.
  `addUsingLink` appends to `usedByLinks` from
  `SIG_RobotCompilerObjects.cpp, linkGeometryFile` on every model load and
  nothing in the tree reads it back; the member, the method and the one call can
  all go. `FrictionValue` is two words held as `QList<FrictionValue *>` with a
  `new` per entry and a `qDeleteAll` in `~SIG_Material`; values would delete
  both, as D8 did for `SIG_Register`. The destructor already frees, so this one
  is tidiness.

- [ ] **37. Two `generateTerrain` calls in one process share the partial file
  name.** The name carries host and process id, so it is unique per process, but
  `MT_Controller` runs an evolution on its own thread. Not new — they interleaved
  in `Terrain.ter` itself before the atomic write — and closing it needs per-call
  state, which the port may not add.

- [ ] **52. `SIG_Drive`'s stream constructor can leave `mode` unset.**
  `SIG_Drive(SIG_Robot*, QTextStream&)` prints `Unsupported Drive Mode <…> !!`
  when it reads an unknown word, and does not set `mode`.
  `SIG_Drive::writeToFileTransfer` then writes `invalid_mode` as the mode, unless
  the unset value happens to equal one of the four known ones. The robot
  compiler rejects unknown modes, so only transfer text can bring an unknown
  word in.

- [ ] **54. Empty catch blocks. Review them in a round of their own.** 5 of the
  29 `catch` clauses in `sigel/` are empty, all
  `catch (SIGEL_Tools::SIG_Exception &e) { };` around `simulation->start()` in
  the fitness evaluation of five fitness functions: AdaptiveWalking, Force,
  NiceWalking, RealSpeed and Simple. A simulation that
  throws is ignored, and the fitness is computed from what was recorded until
  then. NiceWalking and Simple are the fitness functions of all 7 shipped
  experiments, so a change can move fitness values against 1.3. Each site needs
  its own decision. Measured 2026-09-22.

- [ ] **55. Marker checks that do nothing. Review them with item 54.** The
  transfer-text readers in `SIGEL_Robot` check a marker word first. Two throw
  `SIG_UnstreamingError` when it is wrong: `SIG_Robot` for `StreamedRobot` and
  `SIG_LanguageParameters` for `LanguageParameters`. Three hold only a
  `// ERROR` comment and go on reading the wrong data: the stream constructors
  of `SIG_Geometry`, `SIG_Polygon` and `SIG_CommandParameters`. Throwing would
  make a malformed transfer text fail at once; valid files are not affected.
  The same measurement found other empty bodies, for the same round:
  `if (running) {} else {}` in five `MT_*Widget.cpp`, seven empty `else {}` in
  `SIG_GPIndividual.cpp` and `SIG_GPPopulation.cpp`, and `SIG_Robot`'s
  `if (isroot)`, whose only content is "Something seems to be missing here".
  Measured 2026-09-22.

- [ ] **35. Remove the Windows and Visual Studio support.** Decided
  2026-09-09. It does not build here and nothing tests it.
  **It could not build in 2003 either:** `Sigel.dsw` lists 13 projects and only
  5 exist; the project files are Visual C++ 6 and link `msvcirt.lib`, which
  Microsoft removed, and `qt-mt230nc.lib`, Qt 2.3.0 for Windows, which is not
  obtainable; Release and Debug link PVM from different paths.
  **What is there:** 9 Visual Studio project files at the source root, 7,962
  lines — `.dsp` for `Sigel`, `SIGELCommon`, `MetaSIGEL`, `sigel_slave` and
  `manage_dyn_slave`, plus `Sigel.dsw`, `Sigel.mak`, `sigel_slave.mak` and
  `manage_dyn_slave.mak`; **200 `_WINDOWS`
  occurrences across 55 files** — 189 `#ifdef`, 9 `#ifndef` and **2 inside
  commented-out code in `MT_GPManager.cpp`, which no compile break will show**;
  4 `#include <windows.h>` in `MT_Controller.h`, `MT_Substitute.h`,
  `MT_GPManager.h` and `MT_GPManager.cpp`; and `HANDLE`, `DWORD WINAPI`,
  `LPVOID` against the `pthread` side that is built. The clearest case is
  `MT_Controller.h`, which declares `meta_thread` twice, as `HANDLE` and as
  `pthread_t`, and the thread entry point twice.
  **`src/manage_dyn_slave.c` is in this list and is easy to miss** — the only
  `.c` file in the tree, 9 occurrences, invisible to a `--include=*.cpp` sweep.
  **Keep the file itself**: 1.3 built it on Linux, and `sigelDynClient` needs it
  (item 47). Only its Windows branches go.
  **The gates cannot see deleted code**: nothing ever compiled it. Read every
  diff.
  **How:** delete the project files first. Then take the
  `#ifdef _WINDOWS` blocks one module at a time, keeping the `#else` half; the
  object file must not change.
  **Watch:** the 9 `#ifndef _WINDOWS` blocks are reverse polarity — the body is
  kept and only the guard goes. **11 headers carry `_WINDOWS`**, and a mistake
  in one changes what every including translation unit sees: `MT_Controller.h`,
  `MT_Substitute.h`, `MT_GPManager.h`, `SIG_GPManager.h`, `SIG_Program.h`,
  `SIG_DynaMechsSimulationQueries.h`, `SIG_Recorder.h`, `SIG_Register.h`,
  `SIG_SimulationQueries.h`, `SIG_EnvironmentRenderer.h`, `SIG_Renderer.h`.
  **It supersedes D22**, a signed decision: Fusion for the `#else` half, and the
  `#ifdef` half keeps Windows by name because Qt 6 still creates that style.
  Three live call sites — `sigel.cpp, main` and two in `sigel_slave.cpp`.
  Keeping the `#else` half deletes them, which is right once Windows is gone, but
  say that a decision is being overturned.
  **It moves a pinned check total:** the 9 deleted project files are counted
  by the `encodings` check, so `check.sh` loses 9 passes. PORTING.md pins
  that number in two places — the per-step exit criterion in §7 and the check
  list — and its trail in §7 records each step. Move all of them in the same
  commit.
  **Do not mix it with any other change. When:** after every other item in this
  section — moved to the back 2026-09-21, because it touches the whole
  codebase.

---

## Watch out

Not work. Traps that bite whoever edits these files next.

- **Do not add `-e` to `pvm-check.sh`.** It is the only check script with
  `set -u` alone, and both `p3=$?` and `p4=$?` depend on that. Adding `-e`
  makes both captures dead code and leaves `mkdir -p`, `rm -f`,
  `cat > lsan.supp` and `pvmd3 &` unguarded. If it is ever added, convert both
  captures to `p=0; cmd || p=$?` in the same move. Both halves of the PVM check
  would otherwise exit the script instead of printing PASS or FAIL.
- **Do not run `pvm-check.sh` while an evolution is live.** It removes the
  daemon socket from `PVM_TMP` and starts its own daemon. Give it its own
  `PVM_TMP`, or wait. Item 21 is what that fault looks like from the interface.
