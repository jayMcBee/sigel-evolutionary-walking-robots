# To do — SIGEL 2.0

Independent of the port, which is done. Do not combine commits across the two.

**Protocol:** one commit per item, each reviewed and approved.

**Status:** `[ ]` open · `[~]` in review. **Finished items are
not kept here.** They move to `PORTING.md`, which is the only log of what was
done; their numbers stay with them, because other items cite them.

Paths are relative to `sigel/`, the source tree.

---

## 2 · Let the compiler hunt bugs

- [ ] **5. Add `override`**, one commit per module. Highest value on this
  list: a method meant to override with a subtly wrong signature silently
  becomes a new function. **Read every failure; do not fix mechanically.**
  Needs tooling rather than an editor, because base headers must parse. The
  failure shape is a missing `const`, or `int` against `long`.

---

## 3 · Ownership

- [ ] **8. Smart pointers.** Correct and broken both compile clean, and no
  flag verifies it. Prerequisites in order: one experiment running end to end,
  a fixed seed, a recorded fitness trajectory, then one class at a time with
  the trajectory bit-identical. Do not start before that exists.
  Leaks found by item 101's review: `SIG_Simulation`'s constructor members;
  the `SIG_Simulation` that `evalFitness` allocates in the Simple, RealSpeed,
  NiceWalking and Force fitness functions, once per evaluation; the elements
  of `MT_Statistics`; the list in `MT_PopulationWidget::slotExpInd`.

- [ ] **36. Delete `SIG_Body::usedByLinks`; `SIG_Material::FrictionValue`
  could be a value type.** `usedByLinks` is written and never read.
  `FrictionValue` values would drop the `new` and the
  `qDeleteAll`, as D8 did for `SIG_Register`; tidiness only. D11 left both as
  they were, by decision, because the port moved the Qt API and nothing else; that was a
  port-scope rule, not a refusal.

---

## 4 · Program display

- [ ] **9. Syntax-highlight the program view.** New
  `programToHtml(const SIG_Program &, const SIG_LanguageParameters &)` returning
  a `QString`, fed to `QTextBrowser::setHtml()` wherever a program is shown.
  `printToString()` stays as the ZORC serial format and must keep serving
  `SIG_GPRemoteZORCFitnessFunction::evalFitness`.
  `<pre>` wrapper, one `<span>` per token, a line number per line. Colour
  opcodes by group: arithmetic `ADD SUB MUL DIV MOD MIN MAX`, data
  `COPY LOAD`, control `CMP JMP`, robot `MOVE SENSE DELAY`. Registers print as
  `R0`–`R7`, computed as `element % getMemorySize()`; `LOAD` operand 2 and
  `JMP` operand 1 print as literals. Callers: `SIG_SimulationWidget::visualizeThis`,
  `SIG_IndividualView::SIG_IndividualView`,
  `SIG_AllIndividualsView::slotVisualize`.

---

## 5 · Renames and translation

- [ ] **10. Rename the two `SIG_GPExperiment` variants.**
  `SIG_GPExperiment.cpp` builds `sigel`, `SIG_GPExperimentClean.cpp` builds
  `sigel_slave`; both define the same class and include guard. It has already
  been misread as an accidental duplicate. **The master keeps
  `SIG_GPExperiment`; the slave's gets the new name,** with its own guard.
  The Makefile globs `src/<module>/*.cpp` and compiles both into
  `libSIGEL_GP.a`, where 2003 built separate targets.
  **Move `$(MASTER_OBJ)` with it:** the Makefile defines it as
  `SIG_GPExperiment.o`, and `guidrive` names it and asserts on it.
  Linking the Clean variant once produced a convincing false crash.

- [ ] **11. Rename `SIG_GPManager::tours` to `tournaments`,** with the doxygen
  comments that name it. Crosses both `SIG_GPExperiment` variants, so do it
  with item 10.

- [ ] **12. Translate the German comments,** including the `NEU NEU NEU`
  banners and the MSVC German file headers. Nothing executes.

- [ ] **13. Translate the German strings.** What is left is **kept, by
  decision:** the history text of `SIG_GPIndividual`, which is saved in `.exp`
  files: "Fitness (Elter 1)" and "(Elter 2)" in `addCrossOverInfo`, "CREATED
  NEW INDIVIDUUM" in the constructor, "INDIVIDUUM IS GENERATED RANDOMLY" in
  `generateRandomIndividual`. The shipped experiments hold the "Elter" lines.
  **No compiler and no check covers German strings:** every persisted path is
  write-only. `SIG_GPIndividual::readFromFile` parses only `NAME='`,
  `POOLPOS=`, `FITNESS=`, `AGE=` and `PROGRAM BEGIN{`; the `MT_GPManager`
  block sits after that file's own "is not loaded" marker; the ZORC format
  carries no German.
  These strings are pure ASCII: a check for German must look for words, not
  bytes above 127.

- [ ] **14. Translate the German symbols.** Every miss is a compile error,
  so `check.sh` verifies it.

  | symbol | where |
  |---|---|
  | `getRandomInstruktion`, `ProbInstruktion` | `MT_Randomizer.h` |
  | `T_Instruktion`, `T_Instruk` | `MT_TranslatedIndividual.h` |
  | `Instruktion` | `MT_Classifier.cpp, createDoubleTransIndi` |
  | `set`/`getSelektionValue` | `MT_FitnessTrainer.h`, `MT_GPManager.h` |
  | `Varianz` | `MT_StatisticsElement.h` |
  | `winkel`, `verschiebung`, `schiebung`, `drehmatrix`, `hilf`, `stflorianhilf` | `IFunctions.h`, `IFunctions.cpp` |
  | `masse` | `SIG_Mirtich.h`, `.cpp` |

  **Kept, by decision:** `sliderIntervall` and `slotIntervallChanged`. The GUI
  baselines record the widget by name, and the form connects the slot by name.
  The robot grammar is English, so the German names in `SIG_RobotCompiler.cpp`
  take the name of the keyword they hold. `rot` in `getMinRot` and `rotMin` is
  rotation, not the colour — leave it.
  **Umlauts are Latin-1 bytes;** a UTF-8 grep misses them. Comments first,
  because that phase cannot move a baseline.
  **Check after each phase:** `./checks/check.sh`, then
  `./checks/dictorder-dump.sh | diff -u checks/baselines/dictorder-baseline.txt -`
  empty, then `./checks/fitness-check.sh` clean.

- [ ] **15. Rename the `act` prefix to `current`.** German `aktuell`; reads as
  the verb "act". `actExperiment`, `actExpChanged` and `slotActExpChanged` are
  done.
  **Settle the scope first.** Most of it is in `SIGEL_GP`, which D33 keeps
  untouched for behaviour; a rename is not behaviour, but it is a large diff
  in a frozen module, so it needs sign-off first.
  A signal or slot breaks its string-based connect if only one side moves;
  `check.sh` catches that. No reference file holds these names.

- [ ] **16. Set the version to 2.0** in `SIGEL_Tools/SIG_Version.h` when it
  is time. To discuss: what to do with `sigel/README`, which still says
  `KDESIGEL v1.1 Readme File`. `pixmaps/altLogo.png`, with a `Sigel v1.0`
  caption, is kept but unused.

- [ ] **30. Cut comments over two lines that do not earn their place.** A
  longer comment must carry something the code cannot say. **Comments the port
  itself wrote come first.** File by file, each pass signed off first. The
  named instance: the comment above `setIconSize( QSize( 25, 25 ) )` in
  `SIG_MainWindow::SIG_MainWindow`. Two lines carry the whole fact — Qt 6 has
  one icon size per toolbar, and 25 is the largest of the small pixmaps, so
  nothing is scaled past what 1.3 drew.

---

## 6 · Defects preserved by the port

All present in 1.3, none introduced here. Each needs a decision before it is
touched, because changing one changes behaviour against the reference binary.

- [ ] **121. Make the dynamic-client handshake share one mutex.**
  `SIG_GPManager::run`, both overloads, waits on the member `cond` with its
  own local `mutex`; `SIG_GPManager::RegisterDynPVMClients` broadcasts under
  its own local `servMutex`. The two threads never lock the same mutex, so the
  flags `disconnectClients` and `allDisconnected` race, and a broadcast that
  lands between the check and the wait is lost, which leaves the master
  waiting for good. Latent: only the dynamic-client thread reaches it, which
  only `sigel.cpp` starts. Item 19 is the same kind of fault elsewhere.

- [ ] **122. Destroy MetaGP's mutexes instead of unlocking them.**
  `MT_Substitute::~MT_Substitute` and `MT_GPManager::~MT_GPManager` call
  `pthread_mutex_unlock` on mutexes the destroying thread does not hold, which
  POSIX leaves undefined, and never call `pthread_mutex_destroy`.

- [ ] **119. Make MetaGP able to start on Linux.**
  `MT_GPManager::startEvolution` waits for enough training cases with
  `sleep(10000000)` on POSIX, about 115 days, where Windows waits
  `Sleep(10000)`, 10 s. The first check always finds too few cases, so on
  Linux the meta evolution never starts, in 1.3 as well. MetaGP was published
  with results (Ziegler and Banzhaf, CLAWAR 2003), so it presumably ran on
  Windows only.

- [ ] **118. Check the GP parameters when a file loads.**
  `SIG_GPParameter::readFromFile` accepts any value, including ones the
  dialog does not allow, such as a minimum length below 5; the setters check
  nothing either, and `TERMINATIONMODEL` and `PRIORITY` are cast from any
  integer to their enum. Hand-edited files
  are not the concern. To decide: the valid range of each parameter, and
  what a load does with a value outside it.
  **Loaded programs** are not checked either
  (`SIG_GPPopulation::readFromFile`, `SIG_GPIndividual::readFromFile`): a
  program shorter than the minimum or longer than the maximum loads
  unchanged. A short one could be padded with the NOP logic of
  `SIG_Program::checkLength` and recorded with `addLengthIncreasedInfo`;
  unlikely edge cases, such as a stored fitness after padding, would not be
  handled.

- [ ] **18. Stop `SIG_GPPVMTask` from using a deleted host.**
  `SIG_GPFitnessTrainer::flushAllDynHosts` can delete the host a task holds a
  reference to. Latent: only the dynamic-client thread reaches it, which only
  `sigel.cpp` starts, and `guidrive` does not.

- [ ] **19. Make `SIG_GPFitnessTrainer::getNextHost`'s mutex lock
  something.** It is a function local, so it excludes nothing, and the section
  it guards races the dynamic-client thread. Its comment "now we make ourself
  running exclusively" is false and goes with it.

- [ ] **20. Stop `renderRecorder` leaking when
  `SIG_SimulationVisualisation`'s constructor throws.** The other half was
  fixed by nulling `visualisation`. Latent: the only caller that catches the
  throw, `sigel_slave`'s `main`, returns at once; it becomes live the moment
  any caller catches the throw and continues. Three fixes, none changes
  behaviour: a `std::unique_ptr` (the destructor must still delete
  `simulation` first, because it holds a reference to the recorder); a
  try/catch that deletes and throws again; or a value member.
  *Larger and related:* `SIG_Simulation::~SIG_Simulation` is empty, so every
  Stop in the viewer leaks the whole simulation — the §10 leak in PORTING.md.

- [ ] **53. DynaMechs returns uninitialised forces for end links.**
  `dmArticulation::getForces` returns `f_star`, which is never written for a
  link without children, and `SIG_GPForceFitnessFunction` uses it. Only
  experiments that select `ForceFitnessFunction` are affected; no shipped
  experiment does. The library is unpatched, as SIGEL's `supportingLibs`
  ships it.

- [ ] **34. Fix `tearDownPvm()`: `pvm_halt()` never returns.** The daemon
  SIGTERMs this process instead. `guidrive` survives that with
  `keepExitCodeThroughPvmShutdown`.
  **Open:** once that handler holds a status of 0, the process reports 0 for
  any SIGTERM, including a person killing a wedged `guidrive`. A teardown
  flag, or preserving only a non-zero status, closes it.
  **Do not simply delete the call:** it is what stops the daemon this process
  started; dropping it left `pvmd3` and its slaves running. With a daemon
  already up there is no halt and no problem. No stray `pvmd3` survives PVM's
  own shutdown, so today's behaviour is safe, only untidy. Matters before `check.sh` ever
  runs a PVM scenario.
  *Any printf on an early-return path here is lost unless it flushes itself.*
  **Closing `sigel` with SIGTERM does not end it either:** in the one
  measured run, `sigelStandardSignalHandler` called `pvm_halt()` twice and
  the process stayed until SIGKILL.

- [ ] **61. Decide whether to un-transpose the terrain.**
  `SIG_Environment::generateTerrain` and DynaMechs'
  `dmEnvironment::loadTerrainData` disagree on row order, so a floor whose X
  and Z sizes differ is misread. Physics and drawing agree, and no shipped
  experiment sets `FLOORDIMENSION`. A fix changes physics for asymmetric or
  non-square floors.

- [ ] **73. Fix the latent sensor bugs in
  `SIG_DynaMechsSimulationQueries::sense`:** a joint with equal limits, or
  limits that normalise to the same angle (0/360, -180/180), reads NaN; pitch
  at 90° or more reads as 1, and `acos`/`asin` get no clamp; with no contact
  model, or on the `default:` branch, an uninitialised value is loaded. No shipped robot triggers
  any of them.

- [ ] **74. Decide when to give up on a timed-out individual.**
  `SIG_GPFitnessTrainer::checkTask` re-spawns it for ever, as 1.0 did, and
  the probe-error branch does the same; on one
  machine it will likely time out every time, and the generation does not
  end. The shipped experiments set 0, 10 or 30 minutes. To decide: when to
  give up, and what score it then gets.

- [ ] **75. Decide whether the fitness functions should reject physics
  blow-ups.** `SIG_GPFitnessFunction::isValid` rejects only Inf and NaN, so a
  finite blow-up that throws the robot far scores high. Any bound is a
  heuristic and needs more thought. The high 1.3 scores the `sigel-x86`
  session reported are no evidence: those runs had several slaves on one
  host, where the `Terrain.ter` race, fixed in 2.0, produced at least one
  false score (PORTING.md has the measurement).
  `SIG_GPAdaptiveWalkingFitnessFunction`'s height warnings go to
  `SIG_IO::cerr` only and do not change the score.

- [ ] **76. Warn when a mesh has negative volume.** Inverted face winding
  gives negative mass and inertia in `SIG_Mirtich::computePhysics`; the robot
  loads and the simulation runs into NaN without a message. A warning at load,
  naming the link and the mesh file, is one option.

- [ ] **90. Decide whether "stop at generation N" counts pool generations.**
  By reading the code, `SIG_GPManager::checkTerminationConditions` counts from
  each start, not in the pool generation the Experiment page and `.exp` file
  show. Changing it changes when a run stops. The text in
  `SIG_GUIGPExperiment::terminationAlreadyMet` changes with it.

- [ ] **89. Refuse bad simulation parameters.** Needs more thought.
  - **Time to simulate under 1 s:** the simulated fitness functions divide by
    the run time in whole seconds, which gives `inf`, or `NaN` for a robot
    that does not move. What the GP does with that is not measured.
  - **Step size of 0 or less:** item 104. What a negative step does is not
    measured.
  **Where to refuse, not decided:** (1) a range on the field, which stops
  typing only; (2) on load, in `SIG_SimulationParameters`, like item 71, but
  any load error kills the interface until item 88 is done; (3) when a run
  starts, with a message box, which catches typed and loaded values and kills
  nothing.

- [ ] **104. Refuse a step size of 0 or less.** The step-size half of item
  89. A step of 0 gives `int(inf)`, undefined behaviour, in Real Speed, and
  loops `SIG_Simulation::start` for ever in the other fitness functions.
  - **On load:** throw `SIG_UnstreamingError` for a `STEPSIZE` of 0 or less,
    as item 71 does for register widths. File > Open shows it (item 88);
    `sigel -e` and `sigel_eval` refuse the file.
  - **When typed:** `lineeditStepSize` becomes a `QDoubleSpinBox` with a
    smallest value above 0.
  - **Open:** the smallest step and its decimals. The shipped experiments use
    0.01 and 0.002; 0.0001 with 4 decimals would keep both.

- [ ] **37. Give each `generateTerrain` call its own partial file name.** The
  name is unique per process, but `MT_Controller` runs an evolution on its own
  thread.

- [ ] **52. `SIG_Drive`'s stream constructor can leave `mode` unset.** For
  an unknown word it is left unset, and `writeToFileTransfer` then writes
  `invalid_mode`, unless the unset value happens to equal a known one. Only
  transfer text can bring an unknown word in; the robot
  compiler rejects it.

- [ ] **55. Review the marker checks that do nothing.** The
  stream constructors of `SIG_Geometry`, `SIG_Polygon` and
  `SIG_CommandParameters` hold only `// ERROR` and read on; `SIG_Robot` and
  `SIG_LanguageParameters` already throw `SIG_UnstreamingError`. Throwing
  would make a malformed transfer text fail at once; valid files are not
  affected. The other empty bodies go in the same round:
  `if (running) {} else {}` in the `MT_*Widget.cpp` files, empty `else {}` in
  `SIG_GPIndividual.cpp` and `SIG_GPPopulation.cpp`, and `SIG_Robot`'s
  `if (isroot)`, "Something seems to be missing here".

---

## 7 · The interface

- [ ] **107. Review `SIG_GPPopulation::readFromFile` with the maintainer,**
  deciding each change before it is made. The method is long and hard to
  read.

- [ ] **120. Make MetaGP's Add work.** On the MetaGP window's population
  page, Add does not add individuals, so the population can only be filled by
  loading one. Found in use; the cause is not known yet
  (`MT_PopulationWidget::slotAddInd` and its dialog look complete).

- [ ] **106. Review `SIG_GPParameter.cpp` with the maintainer,** method by
  method, deciding each change before it is made. The GP Parameters page is
  hard to read: long methods, commented-out code, and porting comments that
  hide the logic of the method they sit in.

- [ ] **29. Give SIGEL a real logging system.** Qt 6's `QTextStream` does not
  flush on a newline, so diagnostics written through `SIG_IO` are lost when
  the process dies, which is exactly when they are wanted.
  **Do not fix this by adding `Qt::endl` everywhere.** It needs levels, one
  place that decides where output goes and when it flushes, and something the
  GUI can display. It replaces both `SIG_IO` and the console-warning stopgap.
  **Start at `SIG_IO::cerr`** — decided; it is the hook.
  **Review every dialog outside the interface modules as part of this,** for
  example in `SIG_EnvironmentRenderer`, `MT_Controller`, `SIG_GPPopulation`
  and `SIG_GPRemoteZORCFitnessFunction`. Decide for each whether it belongs in
  the interface or goes through the new error reporter.

- [ ] **105. Offer an MP4 when recording stops,** if `ffmpeg` is present, at
  the frame rate the frames were taken at, so the movie plays at simulation
  speed. Image formats only; POV-Ray writes scene files. Hook it into
  `SIG_SimulationVisualisationWidget::reportRecordedFrames`; its "frames
  written" message could become one combined message and question.

- [ ] **77. A robot checker.** Idea; the name is open. It reads a robot model
  and its Language Parameters and warns about what will make evolution fail
  or mislead:
  - **Joint-limit stability per joint,** `timestep * sqrt(K / I)` and
    `timestep * damper / I`. From the x86 session on 1.3, not re-measured: a
    `JOINTLIMITSK_SPRING` of 25000 moved from a one-joint robot onto a
    three-joint chain put the limit-spring torque above half the drive torque.
  - **Drive strength against weight,**
    `maximalforce / (mass * g * half-length)`. Same source: 0.60 to 0.92 in
    the shipped models, whose masses span 1.2 to 49.
  - **Drives MOVE cannot reach,** or reaches unevenly. MOVE picks drive
    `(register value + 2^(w-1)) % number of drives`, w the register width.
    From reading 1.3's `SIG_Interpreter::interprete` and
    `SIG_DynaMechsCommandInterface::moveDrive`, as are the rest of this list;
    the port is expected to match, not checked yet.
  - **A register width of 1,** which divides a force drive's torque by 0.
  - **Too few torque levels** for a small register width: a force drive
    gives `maximalforce * R0 / (2^(w-1) - 1)`.
  - **`minimalforce` as a dead band:** above the maximum, or large against
    one torque step.
  - **Servo drives** that a narrow register cannot turn through their range:
    the register value is an angle in degrees, so 8 bits reach -128 to 127.
  - **Reordered drives:** drive numbers are the order in the model, so a
    change silently changes which joint an evolved program moves.
  - **Sensors:** SENSE picks from a register value too; the same checks
    likely apply. Not read yet.
  To show beside it: every MOVE takes its torque from R0, and a negative
  register operand wraps (`MOVE -128` with 24 registers reads R8).

---

## 8 · GP engine

How programs control a robot, and how evolution changes programs. Every item
here changes evolution results, so each is judged only by whether the best
fitness improves. Each starts with the published GP approaches to the
problem; the choice is made before any code is written.

- [ ] **116. Let drives hold a torque until the program changes it.**
  `SIG_DynaMechsCommandInterface::moveDrive` applies the torque from R0 for
  the `MOVE` duration, then the drive goes limp. Tristar sets 0.001 s, so
  every `MOVE` acts for a single 10 ms step, and few joints carry load at the
  same time. Either two commands, start and stop, or one command with a duration operand. New
  commands change `LanguageParameters` and the experiment files.

- [ ] **117. Let mutation tune operands in steps.**
  `SIG_GPOperations::mutation` only replaces an operand with a new value over
  the whole range; torque, drive number, jump distance and register number
  are never adjusted.
  Steps of plus or minus 1 need far too many generations. **Start with a
  research phase:** there is plenty of GP literature on operand mutation, so
  look at what others have done or proposed before writing anything, starting
  from the links at https://www.genetic-programming.org/. Candidates so far:
  step sizes scaled to the operand's range, and step sizes that adapt during
  the run.

- [ ] **68. Decide whether the robot starts in the middle of the terrain.**
  Every shipped experiment starts it at the terrain's corner. Moving it means
  either `STARTPOSITION` in the `.exp` files, a file change, or the terrain's
  place in DynaMechs, a vendor patch. Positions and fitness move with it.

- [ ] **81. Add a fitness function that rewards steady walking.** No existing
  function rewards an even pace. Decided so far, preliminary name "Steady
  Walking", `SteadyWalkingFitnessFunction`, class
  `SIG_GPSteadyWalkingFitnessFunction`:
  - split the run into 1 s windows; drop a partial last window;
  - pᵢ = horizontal progress in window i along the line from the run's start
    to its end, so no direction is fixed;
  - v̄ = mean of pᵢ per second; cv = std(pᵢ) / v̄;
  - h = std(body height) / start height;
  - score = v̄ / (1 + cv) / (1 + h), the height weight being 1;
  - no settling time at the start; a full run, never the early-stop
    simulation;
  - 0 when v̄ is 0 or below, or a position is Inf or NaN.
  Records every frame with `SIG_GPFullDataRecorder`. Register it in
  `SIG_GPFitnessFunctionRegistry::fitnessFunctions()` and in `sigel_slave`'s
  name mapping. No file format changes.

---

## 9 · Removals

- [ ] **123. Remove the second `#include "MT_GPSystem/MT_TrainingCase.h"`
  from `MT_Substitute.h`.** The same line appears twice in a row.

- [ ] **64. Remove what is left of Dynamo.** Decided: it goes completely. It
  was hardly ever used (https://sigel.sourceforge.net/seiten/links_en.html).
  PORTING.md, "Dynamo removed, DynaMechs kept", has the background. Three
  parts, one round each, in this order.
  - **The choice of Dynamo in the interface and the model:** the
    "Dynamo  (not recommended)" radio button and the `DynaMo` tabs in
    `SIG_SimulationParameterBase.ui` and `SIG_EnvironmentBase.ui`;
    `SIG_SimulationParameter::putIntoExperiment` and `getOutOfExperiment`; the
    `DynaMo` value of `SIG_SimulationParameters::SimulationLibrary` and every
    case that handles it, with `SIG_Robot::prepareDynaMo`,
    `SIG_Link::transformToDynaMo` and `SIG_Simulation::slotDynamoMessage`; the
    DynaMechs check in `SIG_GUIGPExperiment::slotRobotInfo`;
    `SIG_SimulationCannotSolveException` and the `stopSimulation` flag that
    only Dynamo sets. Today the interface can make
    an experiment the simulation refuses, which is one way into item 20.
  - **Dynamo's settings in the `.exp` files** (`MAXIMALERROR`,
    `MAXIMALITERATIONS`, `SKIPFRAMES`, `ANALYTICAL`, `MAXIMALCOLLISIONLOOPS`,
    `SOLVEMODE`, `INTEGRATOR`, `MAXIMALSOLIDITERATIONS`) are in every shipped
    experiment. Decided: a load reads them and ignores them; a save no longer
    writes them. `SIMULATIONLIBRARY` loses its Dynamo value only. `STEPSIZE`
    stays: DynaMechs uses it. **Open:** what a load does with a file whose
    `SIMULATIONLIBRARY` is Dynamo.
  - **The maths library `libdynalib.a`,** whose `DL_vector` and `DL_matrix`
    SIGEL is built on. PORTING.md, "Follow-up this change deliberately did not
    take", point 3, has the plan: a small local header in its place. Assess
    first; the replacement is expected to be 1:1. The fitness gates prove it.
  Doc comments that name Dynamo go with the code they describe. The comment on
  the guard in `SIG_SimulationVisualisationWidget::visualizeThis` names Dynamo
  too; the guard stays, for `SIG_CannotMirtich`.

- [ ] **83. Put ZORC support behind a compile-time switch, off by default.**
  Decided. ZORC is a real robot driven over a serial line; the simulation does
  not need it. One global `#define` removes `SIG_GPRemoteZORCFitnessFunction`, its
  branches in `sigel_slave.cpp`, and the "Remote ZORC" combo box entry,
  handled like item 82's index shift. Not ZORC:
  `SIG_GPAdaptiveWalkingFitnessFunction`, stored as
  "ZorcWalkingFitnessFunction"; it stays. To decide: where the switch lives
  (the `Makefile` or a header), and what an experiment file naming
  "RemoteZORCFitnessFunction" does when the switch is off.

- [ ] **47. `sigelDynClient` and `manage_dyn_slave`.**
  `sigelDynClient` makes a second machine a dynamic slave of `sigel -de`. It
  is still 1.3's Solaris `tcsh` script with placeholder paths, and it runs
  `manage_dyn_slave`, which 1.3 built and the port does not.
  **Modernise in place, do not replace.** It needs a second machine to prove
  it on; the 1.3 reference machine is not ours to use for tooling, so it waits
  until there is one.
  - **`manage_dyn_slave.c`, `main`,** tests `getprotobyname` through an
    `(int)` cast, which keeps only the low 32 bits of the pointer, so a valid
    pointer can read as a failure.

- [ ] **35. Remove the Windows and Visual Studio support.** Decided. It does
  not build here, nothing tests it, and it could not build in 2003 either.
  **Going now, by decision, in six commits** rather than one, so each can be
  reviewed:
  1. the nine Visual Studio files, with the `encodings` totals PORTING.md pins
     and the docs that named the files — done 2026-09-29;
  2. the `_WINDOWS` groups in the headers;
  3. the `_WINDOWS` groups in the MetaGP sources: `MT_Control`,
     `MT_GPSystem`, `MT_GUI`;
  4. the `_WINDOWS` groups in the `SIGEL_*` sources, `sigel.cpp`,
     `sigel_slave.cpp` and `manage_dyn_slave.c`;
  5. the `AFX_…_INCLUDED_` include guards, renamed to the tree's `DIR_FILE_H`
     form, and the "Added from the class view" comments;
  6. the MSVC class-wizard comments: the `// X.h: interface for class X.`
     banners and the `Construction/destruction` blocks.

  A trial run on 2026-09-28 resolved every group with a script and passed
  `check.sh`.
  - **Resolving a group:** for `#ifdef _WINDOWS` keep the `#else` half, or
    nothing when there is none; for `#ifndef _WINDOWS` keep the body and drop
    the guard.
  - **The proof is two builds,** because removed lines move `__LINE__` in
    `throw`s and the `"file:line"` text Qt's `SIGNAL` and `SLOT` store. First
    with removed lines left blank: every object must match outside the debug
    information. Then with the blanks gone: only line numbers may differ.
  - **`src/manage_dyn_slave.c` is easy to miss,** the only `.c` file. Keep
    the file: `sigelDynClient` needs it (item 47). Only its Windows branches
    go.
  - **The gates cannot see deleted code.** Read every diff: a Windows-only
    group can hold a line the other half needs. `_WINDOWS` also sits in
    commented-out code in `MT_GPManager.cpp`, which a script skips.
  - **Headers carry `_WINDOWS` too,** and a mistake in one changes every
    including translation unit: `MT_Controller.h`, `MT_Substitute.h`,
    `MT_GPManager.h`, `SIG_GPManager.h`, `SIG_Program.h`,
    `SIG_DynaMechsSimulationQueries.h`, `SIG_Recorder.h`, `SIG_Register.h`,
    `SIG_SimulationQueries.h`, `SIG_EnvironmentRenderer.h`, `SIG_Renderer.h`.
    `MT_Controller.h` declares `meta_thread` and the thread entry point twice,
    once as `HANDLE`, once for `pthread`.
  - **D22 loses its Windows half.** Fusion stays, as the only style; its call
    sites are in `sigel.cpp, main` and `sigel_slave.cpp`. Commit 4 notes it in
    D22.
  **Do not mix it with any other change.**

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
