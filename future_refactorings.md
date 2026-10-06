# To do — SIGEL 2.0

Independent of the port, which is done. Do not combine commits across the two.

**Protocol:** one commit per item, each reviewed and approved.

**Status:** `[ ]` open · `[~]` in review. **Finished items are
not kept here.** They move to `PORTING.md`, which is the only log of what was
done; their numbers stay with them, because other items cite them.

Paths are relative to `sigel/`, the source tree.

---

## 1 · Let the compiler hunt bugs

- [ ] **5. Add `override`**, one commit per module. Highest value on this
  list: a method meant to override with a subtly wrong signature silently
  becomes a new function. **Read every failure; do not fix mechanically.**
  Needs tooling rather than an editor, because base headers must parse. The
  failure shape is a missing `const`, or `int` against `long`.

---

## 2 · Ownership

- [ ] **8. One owner for each object.** Replace a `new` and its `delete`
  by a local object, a value member or a `std::unique_ptr`, where no other
  class has to change. One place per commit. Gate for each: the fitness rows
  are identical on both builds, and the AddressSanitizer run has no report.

  *Local pointer that can be a local object:*
  - [ ] `MT_Search::crossover`: `NextProgPartForChildOne`,
    `NextProgPartForChildTwo`.
  - [ ] `SIG_Program::readFromFile`: `prgLine`. It leaks when a line does
    not parse. It fits when `SIG_Program::lines` no longer holds raw
    pointers.
  - [ ] `SIG_Body::load`: `bodyScene`. Never deleted; `~SceneGraph` has
    never run in SIGEL.
  - [ ] `SIG_GPPopulation::addRandomIndividuals` and `readFromFile`: the
    `progress` dialog.
  - [ ] `SIG_SimulationVisualisation::initShadowMapping`: `program`.
  - [ ] `SIG_ExperimentListView::openExperimentFile`: `theNewExperiment`.
  - [ ] `main` in `sigel_slave.cpp`: `robot`, `environment`,
    `simulationParameters`, `program`, `modifiedRobot`. They wait for
    item 141.
  - [ ] `main` in `sigel.cpp`: `mainWindow`. Never deleted, so
    `~SIG_MainWindow` and the destructors of the open experiments have never
    run at exit. Read that chain first, and test a quit with an open
    experiment on the AddressSanitizer build.

  *Pointer member with one owner:*
  - [ ] `SIG_DynaMechsSimulationData::dynaMechsIntegrator`. Never deleted;
    the DynaMechs integrator destructors have never run in SIGEL.
  - [ ] `SIG_DynaMechsLink::screwLink`. Never deleted.
  - [ ] `SIG_Robot::language`.
  - [ ] `SIG_Body::geometry`.
  - [ ] `SIG_Link::geometry`, `mirtich`.
  - [ ] `SIG_GPExperiment::mtController`.
  - [ ] `SIG_SimulationVisualisation::simulation`, `renderRecorder`,
    `shadowProgram`. The simulation must be destroyed before the recorder.
    This also does item 20.
  - [ ] `SIG_VisualisationWidget::visualisation`. The old one must be
    destroyed before the new one is built.
  - [ ] `SIG_GUIGPExperiment::guiGPManager`.
  - [ ] `MT_GPManager`: `BestIndividual`, `Randi`, `Statistics`, `Offspring`,
    `Parent`, `Seeker`, `Selector`, `FitnessTrainer`. Used by the MetaGP
    thread and the GUI thread.
  - [ ] `MT_Individual::Program`.
  - [ ] `MT_Program::Program`, a `new[]` array.
  - [ ] `MT_Programline::OperandA`, `OperandB`.
  - [ ] `MT_TrainingCase::TranslateIndividual`.
  - [ ] `MT_TranslatedIndividual`: `T_Instruktion`, `T_Operand1`,
    `T_Operand2`, `MetaData`.
  - [ ] `MT_FitnessTrainer::TSet`.
  - [ ] `MT_Controller`: `gpManager`, `substitution`, `cacheStrm`. The first
    two are used by the MetaGP thread; `cacheStrm` leaks.
  - [ ] `MT_Substitute`: `Interpreter`, `BestMETAProgram`.

  *A `new` that nothing deletes:*
  - [ ] `MT_Classifier::classifier`: the `MT_TranslatedIndividual` from
    `createDoubleTransIndi`.
  - [ ] `MT_Evaluator::spawnTask`: the `MT_TranslatedIndividual` from
    `translatedSIGProg`.
  - [ ] `MT_PopulationWidget::getSelectedItems`: the list, used in
    `slotExpInd`.
  - [ ] `SIG_GPParameter::slotDeleteHost`: the tree items it takes out.
  - [ ] `SIG_Robot::readFromFileTransfer`: the language parameters, when the
    stream has none. Done by `SIG_Robot::language` above.
  - [ ] `SIG_DynaMechsLink`: the contact model and the DynaMechs link bodies.
  - [ ] `SIG_EnvironmentRenderer::loadPNMTexture`: the texture image, from
    `malloc`.

  *Not in this item:* lists of raw pointers, among them the elements of
  `MT_Statistics`, which are never deleted. `SIG_GPPopulation::randomizer`
  and `SIG_GPManager::trainer` own their object in one mode and borrow it in
  another.

- [ ] **140. Assess `DynaMechsLinkGuard`.** It is a hand-written struct in
  `SIG_DynaMechsSimulationData.cpp` that frees the links when the
  constructor throws. Review its design, and decide whether a standard
  C++20 construct does the same job.

- [ ] **141. Untangle `main` in `sigel_slave.cpp`.** One function does three
  jobs: it shows an experiment file, it shows an individual that PVM sent,
  and it computes a fitness for PVM. The pointers `robot`, `environment`,
  `simulationParameters`, `program` and `modifiedRobot` own their object in
  one mode and point at another owner's object in the other mode, so nothing
  deletes them. Give each job its own code and each object one owner.

- [ ] **36. `SIG_Material::FrictionValue` could be a value type.**
  `FrictionValue` values would drop the `new` and the `qDeleteAll`, as D8 did
  for `SIG_Register`; tidiness only. D11 left it as it was, by decision,
  because the port moved the Qt API and nothing else; that was a port-scope
  rule, not a refusal.

---

## 3 · Renames and translation

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

- [ ] **11. Write `tour` out as `tournament` in every name,** in SIGEL and
  MetaGP.

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
  | `Instruktion` | `MT_Classifier.cpp, createDoubleTransIndi`; `MT_Substitute.cpp, translatedSIGProg` |
  | `set`/`getSelektionValue` | `MT_FitnessTrainer.h`, `MT_GPManager.h` |
  | `winkel`, `verschiebung`, `schiebung`, `drehmatrix`, `hilf`, `stflorianhilf` | `IFunctions.h`, `IFunctions.cpp` |

  **Kept, by decision:** `sliderIntervall` and `slotIntervallChanged`. The GUI
  baselines record the widget by name, and the form connects the slot by name.
  `rot` in `getMinRot` and `rotMin` is rotation, not the colour — leave it.
  **Check after each phase:** `./checks/check.sh`, then
  `./checks/dictorder-dump.sh | diff -u checks/baselines/dictorder-baseline.txt -`
  empty, then `./checks/fitness-check.sh` clean.

- [ ] **15. Rename the `act` prefix to `current`.** German `aktuell`; reads as
  the verb "act". `actExperiment`, `actExpChanged` and `slotActExpChanged` are
  done.
  A signal or slot breaks its string-based connect if only one side moves;
  `check.sh` catches that. No reference file holds these names.

- [ ] **16. Set the version to 2.0** in `SIGEL_Tools/SIG_Version.h` when it
  is time. To discuss: what to do with `sigel/README`, which still says
  `KDESIGEL v1.1 Readme File`. `pixmaps/altLogo.png`, with a `Sigel v1.0`
  caption, is kept but unused.

- [ ] **30. Cut comments over two lines that do not earn their place.** A
  longer comment must carry something the code cannot say. **Comments the port
  itself wrote come first.** File by file, each pass signed off first.

---

## 4 · Defects preserved by the port

All present in 1.3, none introduced here. Each needs a decision before it is
touched, because changing one changes behaviour against the reference binary.

- [ ] **136. Keep the terrain in `SIG_Environment` as SIGEL data.**
  `SIG_Environment` has a `dmEnvironment` member only to load `Terrain.ter`
  for `SIG_EnvironmentRenderer`. Without it, no class outside
  `SIGEL_Simulation` uses a DynaMechs environment.
  `loadDynaMechsEnvironment` also runs `generateTerrain`, which writes the
  `Terrain.ter` that every simulation reads. That step stays.

- [ ] **139. Assess the report that a simple servo drive goes to the mirrored
  angle.** A robot built with `simpleservo` drives was reported to move its
  joint to minimum + maximum - command instead of to the command. One test
  on one joint agrees with the report. It is not known whether this is a
  defect, which joints it concerns, or whether 1.3 does the same. The place
  to read is the `simpleservo` branch of
  `SIG_DynaMechsCommandInterface::moveDrive`. The shipped robots use force
  drives, so no shipped experiment shows it.
  **WARNING: change nothing here before the original ZORC experiment is
  tested.** It may use servo drives, and a naive change may break existing
  experiments.

- [ ] **125. Give `accept()` a buffer size in
  `SIG_GPManager::RegisterDynPVMClients`.** `alen` is passed to `accept()`
  without being set, so `accept()` reads an arbitrary buffer size. If that
  value is invalid, `accept()` fails, and the server prints "accept() failed"
  and exits the program. Latent: only the dynamic-client thread reaches it,
  like item 121.

- [ ] **121. Make the dynamic-client handshake share one mutex.**
  `SIG_GPManager::run`, both overloads, waits on the member `cond` with its
  own local `mutex`; `SIG_GPManager::RegisterDynPVMClients` broadcasts under
  its own local `servMutex`. The two threads never lock the same mutex, so the
  flags `disconnectClients` and `allDisconnected` race, and a broadcast that
  lands between the check and the wait is lost, which leaves the master
  waiting for good. Latent: only the dynamic-client thread reaches it, which
  only `sigel.cpp` starts. Item 19 is the same kind of fault elsewhere.

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

- [ ] **53. DynaMechs returns uninitialised forces for end links.**
  `dmArticulation::getForces` returns `f_star`, which is never written for a
  link without children, and `SIG_GPForceFitnessFunction` uses it. Only
  experiments that select `ForceFitnessFunction` are affected; no shipped
  experiment does. No patch to the library touches this method.

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
  experiment has such a floor: only `runner.exp` sets `FLOORDIMENSION`, to a
  square floor. A fix changes physics for asymmetric or
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

- [ ] **76. Warn when a mesh has negative volume.** Inverted face winding
  gives negative mass and inertia in `SIG_Mirtich::computePhysics`; the robot
  loads and the simulation runs into NaN. The robot check reports it as
  "Link without mass" and names the link, but only when Check is pressed. A
  warning at load, naming the link and the mesh file, is one option.

- [ ] **89. Refuse bad simulation parameters.** Needs more thought.
  - **Time to simulate under 1 s:** the simulated fitness functions divide by
    the run time in whole seconds, which gives `inf`, or `NaN` for a robot
    that does not move. What the GP does with that is not measured.
  - **Step size of 0 or less:** item 104. What a negative step does is not
    measured.
  **Where to refuse, not decided:** (1) a range on the field, which stops
  typing only; (2) on load, in `SIG_SimulationParameters`, like item 71;
  (3) when a run
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

- [ ] **52. `SIG_Drive`'s stream constructor can leave `mode` unset.** For
  an unknown word it is left unset, and `writeToFileTransfer` then writes
  `invalid_mode`, unless the unset value happens to equal a known one. Only
  transfer text can bring an unknown word in; the robot
  compiler rejects it.

- [ ] **55. Review the marker checks that do nothing.** The
  stream constructor of `SIG_CommandParameters` holds only `// ERROR` and
  reads on; those of `SIG_Geometry` and `SIG_Polygon` print a message and
  read on; `SIG_Robot` and
  `SIG_LanguageParameters` already throw `SIG_UnstreamingError`. Throwing
  would make a malformed transfer text fail at once; valid files are not
  affected. The other empty bodies go in the same round:
  `if (running) {} else {}` in the `MT_*Widget.cpp` files, empty `else {}` in
  `SIG_GPIndividual.cpp` and `SIG_GPPopulation.cpp`, and `SIG_Robot`'s
  `if (isroot)`, "Something seems to be missing here".

---

## 5 · The interface

- [ ] **128. Evaluate a Stop that waits for the end of the generation.**
  Stop ends a run at once, in the middle of a generation: the tournaments
  played so far have already changed the pool, but the generation is not
  counted and no history entry is written. A first click could stop after the
  current generation and a second click at once. Suggested: replace the
  `userTerminated` flag with one stop request that has three values, no stop,
  after the generation, and now, rather than add a second flag.

- [ ] **107. Review `SIG_GPPopulation::readFromFile` with the maintainer,**
  deciding each change before it is made. The method is long and hard to
  read.

- [ ] **106. Review `SIGEL_MasterGUI::SIG_GPParameter` with the maintainer,**
  the GP Parameters page, method by method, deciding each change before it is
  made. Its long methods, `slotItemDoubleClicked` and `slotDeleteHost` above
  all, have deep nesting and uneven indentation that invite bugs. Split them
  into smaller methods; make the iterator loops C++11 range-for loops with
  `auto`. Commented-out code and porting comments hide the logic of the
  methods they sit in.
  The model class of the same name, `SIGEL_GP::SIG_GPParameter`, was reviewed
  first. Still open there: `maxAge`, which the page shows and the file stores
  but nothing reads.

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

- [ ] **9. Syntax-highlight the program view.** New
  `programToHtml(const SIG_Program &, const SIG_LanguageParameters &)` returning
  a `QString`, fed to `QTextEdit::setHtml()` wherever a program is shown.
  `printToString()` stays as the ZORC serial format and must keep serving
  `SIG_GPRemoteZORCFitnessFunction::evalFitness`.
  `<pre>` wrapper, one `<span>` per token, a line number per line. Colour
  opcodes by group: arithmetic `ADD SUB MUL DIV MOD MIN MAX`, data
  `COPY LOAD`, control `CMP JMP`, robot `MOVE SENSE DELAY`; `NOP` has no
  group yet. Registers print as
  `R0`–`R7`, computed as `element % getMemorySize()`; `LOAD` operand 2 and
  `JMP` operand 1 print as literals. Callers: `SIG_SimulationWidget::visualizeThis`,
  `SIG_IndividualView::SIG_IndividualView`,
  `SIG_AllIndividualsView::slotSelectionChanged`.

---

## 6 · MetaGP

- [ ] **132. Make the MetaGP autosave rotate.** In `MT_Controller`,
  `autoSaveCnt = autoSaveCnt++ % 3` writes the old value back, so the
  counter stays at 0. The `saveName` it builds is also not used: the next
  line saves to `name`. The planned rotation over three autosave files
  never happens. GCC warns (`-Wsequence-point`).

- [ ] **122. Destroy MetaGP's mutexes instead of unlocking them.**
  `MT_Substitute::~MT_Substitute` and `MT_GPManager::~MT_GPManager` call
  `pthread_mutex_unlock` on mutexes the destroying thread does not hold, which
  POSIX leaves undefined, and never call `pthread_mutex_destroy`.

- [ ] **119. Make MetaGP able to start on Linux.**
  `MT_GPManager::startEvolution` waits for enough training cases with
  `sleep(10000000)` on POSIX, about 115 days, where 1.3's Windows build
  waited `Sleep(10000)`, 10 s. The first check always finds too few cases, so on
  Linux the meta evolution never starts, in 1.3 as well. MetaGP was published
  with results (Ziegler and Banzhaf, CLAWAR 2003), so it presumably ran on
  Windows only.

- [ ] **37. Give each `generateTerrain` call its own partial file name.** The
  name is unique per process, but `MT_Controller` runs an evolution on its own
  thread.

- [ ] **120. Make MetaGP's Add work.** On the MetaGP window's population
  page, Add does not add individuals, so the population can only be filled by
  loading one. Found in use; the cause is not known yet
  (`MT_PopulationWidget::slotAddInd` and its dialog look complete).

---

## 7 · GP engine

How programs control a robot, and how evolution changes programs. Every item
here changes evolution results, so each is judged only by whether the best
fitness improves. Each starts with the published GP approaches to the
problem; the choice is made before any code is written.

- [ ] **127. Review `SIG_GPManager.cpp` with the maintainer,** method by
  method, deciding each change before it is made. Several of its methods are
  very long and hard to read and maintain.

- [ ] **124. Find a modern replacement for pthreads.** The MetaGP thread,
  the dynamic-client server thread and their locks use `pthread_create`,
  `pthread_mutex_*` and `pthread_cond_*` directly. Research what should
  replace them, for example what C++20 offers natively, before any code
  changes. Items 19, 121 and 122 are faults in this code.

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
  `SIG_GPFitnessFunctionRegistry::fitnessFunctions()`. No file format
  changes.

---

## 8 · Removals

- [ ] **83. Put ZORC support behind a compile-time switch, off by default.**
  Decided. ZORC is a real robot driven over a serial line; the simulation does
  not need it. One global `#define` removes `SIG_GPRemoteZORCFitnessFunction`, its
  branches in `sigel_slave.cpp`, and its entry in
  `SIG_GPFitnessFunctionRegistry::fitnessFunctions`, from which the fitness
  function list in the interface is filled. Not ZORC:
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
