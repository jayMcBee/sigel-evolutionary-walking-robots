# To do — SIGEL 2.0

Independent of the port, which is done. Do not combine commits across the two.

**Protocol:** one commit per item, each reviewed and approved.

**Status:** `[ ]` open · `[~]` in review. **Finished items are
not kept here.** They move to `PORTING.md`, which is the only log of what was
done; their numbers stay with them, because other items cite them.

Paths are relative to `sigel/`, the source tree.

---

## 1 · Ownership

- [ ] **140. Assess `DynaMechsLinkGuard`.** It is a hand-written struct in
  `SIG_DynaMechsSimulationData.cpp` that frees the links when the
  constructor throws. Review its design, and decide whether a standard
  C++20 construct does the same job.

- [ ] **36. `SIG_Material::FrictionValue` could be a value type.**
  `FrictionValue` values would drop the `new` and the `qDeleteAll`, as D8 did
  for `SIG_Register`; tidiness only. D11 left it as it was, by decision,
  because the port moved the Qt API and nothing else; that was a port-scope
  rule, not a refusal.

---

## 2 · Renames and translation

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

- [ ] **14. Translate the German symbols.** Every miss is a compile error,
  so `check.sh` verifies it.

  | symbol | where |
  |---|---|
  | `getRandomInstruktion`, `ProbInstruktion` | `MT_Randomizer.h` |
  | `T_Instruktion`, `T_Instruk` | `MT_TranslatedIndividual.h` |
  | `Instruktion` | `MT_Classifier.cpp, createDoubleTransIndi`; `MT_Substitute.cpp, translatedSIGProg` |
  | `set`/`getSelektionValue` | `MT_FitnessTrainer.h`, `MT_GPManager.h` |

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
  is time.
  `pixmaps/altLogo.png`, with a `Sigel v1.0` caption, is kept but unused.

- [ ] **144. Put the 4x4 identity and translation matrix in one helper
  class.** `SIG_Body::load`, `SIG_Body::readVRMLNode`, the long function in
  `IFunctions.cpp` and `SIG_DynaMechsLink::buildTranslationMatrix` each write
  these `NEWMAT::Matrix` values out by hand. A small class in `SIGEL_Tools`
  with static functions defines them once. The three `build...` methods of
  `SIG_DynaMechsLink` move there: they build general matrices and use no
  member of a link. The fitness rows must stay identical.
  Review `SIG_Body::createRotationMatrix` with it. Its sine terms have the
  opposite sign from `rotationMatrix` in `IFunctions.cpp`, so one is the
  transpose of the other; do not merge the two. If it does not build the
  usual rotation matrix, the least change is a name that says what it
  builds.

- [ ] **145. Review `IFunctions.cpp`.** It holds free functions, among them
  `rotationMatrix` and
  `phatRockingUpStylinVectorBendingAngleSwingingMasterFunction`, which is
  very long. Behaviour belongs to a class. Decide which class each function
  belongs to, and give the long one a name that says what it does.

---

## 3 · Defects preserved by the port

All present in 1.3, none introduced here. Each needs a decision before it is
touched, because changing one changes behaviour against the reference binary.

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
  `SIG_Program::clampLengthToLimits` and recorded with `addLengthIncreasedInfo`;
  unlikely edge cases, such as a stored fitness after padding, would not be
  handled.

- [ ] **53. DynaMechs returns uninitialised forces for end links.**
  `dmArticulation::getForces` returns `f_star`, which is never written for a
  link without children, and `SIG_GPForceFitnessFunction` uses it. Only
  experiments that select `ForceFitnessFunction` are affected; no shipped
  experiment does. No patch to the library touches this method.

- [ ] **73. Fix the latent sensor bugs in
  `SIG_DynaMechsSimulationQueries::sense`:** a joint with equal limits, or
  limits that normalise to the same angle (0/360, -180/180), reads NaN; pitch
  at 90° or more reads as 1, and `acos`/`asin` get no clamp; with no contact
  model, or on the `default:` branch, an uninitialised value is loaded. No shipped robot triggers
  any of them.

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
  A negative step reaches `SIG_Interpreter::interprete` as a negative time
  account: no command runs, and the amount is taken off the time of the
  next call.
  - **On load:** throw `SIG_UnstreamingError` for a `STEPSIZE` of 0 or less,
    as item 71 does for register widths. File > Open shows it (item 88);
    `sigel -e` and `coredrive` refuse the file.
  - **When typed:** `lineeditStepSize` becomes a `QDoubleSpinBox` with a
    smallest value above 0.
  - **Open:** the smallest step and its decimals. The shipped experiments use
    0.01 and 0.002; 0.0001 with 4 decimals would keep both.

- [ ] **152. A geometry file with `DEF` and `USE` aborts SIGEL.** A VRML
  file may give a shape a name with `DEF` and place it again with `USE`.
  `SIG_Body` loads the file with the `cv97` library, and the library stops
  the program with a failed assertion in `Node::createInstanceNode`. The
  import of such a robot closes SIGEL with no message. Handle it gracefully:
  the robot is loaded, or the user gets a message that names the file.

---

## 4 · The interface

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
  group yet. Callers: `SIG_SimulationWidget::visualizeThis`,
  `SIG_IndividualView::SIG_IndividualView`,
  `SIG_AllIndividualsView::slotSelectionChanged`.

---

## 5 · MetaGP

- [ ] **142. Name the crossover points in `MT_Search::crossover`.** The list
  `XPoints` from `MT_Randomizer::getRandomXPoints` is read by bare index:
  `[0]` is the number of crossover points, `[1]` to `[6]` are the first,
  second and third point in the first and the second individual. Only a
  comment says so. Use an enum for the indices.

- [ ] **143. Name the types of genesis of a MetaGP individual.**
  `MT_Individual::setTypOfGenesis` takes a bare number: 1, 2 and 3 for a
  crossover with that many points, 4 for reproduction, 5 for a parent that
  is carried over, and 100 plus the number of mutated elements for a
  mutation. `MT_Search` sets it and the statistics in `MT_GPManager` read
  it. Use a scoped enum. The count of mutated elements is a second value
  and needs its own member.

- [ ] **148. Name the estimation strategies.** `EstimationStrategy` in
  `MT_Substitute` is a bare `int`. `MT_Evaluator::evaluationTactic` and
  `MT_Classifier::evaluationTactic` switch on its numbers, and only the
  list in `MT_EstimationWidget` gives them names. Use a scoped enum. The
  number is written into the saved MetaGP file, so each enumerator keeps
  its value. The kind of substitute has the same fault: `MT_Controller.h`
  names it with three preprocessor defines, `NOMETA_SUBST`,
  `EVALUATOR_SUBST` and `CLASSIFIER_SUBST`, and `MT_Substitute::Typ` holds
  it as 1 or 2. Use one scoped enum for both.

- [ ] **132. Make the MetaGP autosave rotate.** In `MT_Controller`,
  `autoSaveCnt = autoSaveCnt++ % 3` writes the old value back, so the
  counter stays at 0. The `saveName` it builds is also not used: the next
  line saves to `name`. The planned rotation over three autosave files
  never happens. GCC warns (`-Wsequence-point`).

- [ ] **122. Destroy MetaGP's mutexes instead of unlocking them.**
  `MT_Substitute::~MT_Substitute` and `MT_GPManager::~MT_GPManager` call
  `pthread_mutex_unlock` on mutexes the destroying thread does not hold, which
  POSIX leaves undefined, and never call `pthread_mutex_destroy`.
  There may be more to it: nothing waits for the evolution thread to end
  before these objects are deleted, so check the locking of the MetaGP thread
  as a whole before changing this.

- [ ] **119. Make a fresh MetaGP able to start beside a SIGEL evolution on
  Linux.** `MT_GPManager::startEvolution` waits for enough training cases
  with `sleep(10000000)` on POSIX, about 115 days, where 1.3's Windows build
  waited `Sleep(10000)`, 10 s. The wait is reached only when MetaGP runs
  with a substituter, and only when the training set has fewer than 10
  cases or not more than a tenth of the result array. A MetaGP that starts
  with an empty training set beside a new SIGEL evolution therefore sleeps
  at its first test, in 1.3 as well. MetaGP on its own, and a MetaGP that
  is loaded with enough training cases, do not reach the wait. MetaGP was
  published with results (Ziegler and Banzhaf, CLAWAR 2003), so the fresh
  start presumably ran on Windows only.

- [ ] **120. Make MetaGP's Add work.** On the MetaGP window's population
  page, Add does not add individuals, so the population can only be filled by
  loading one. Found in use; the cause is not known yet
  (`MT_PopulationWidget::slotAddInd` and its dialog look complete).

---

## 6 · GP engine

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
  changes. Item 122 is a fault in this code.

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
  - score = the smallest pᵢ, the progress of the slowest second; 0 when it
    is below 0, or a position is Inf or NaN;
  - no settling time at the start; a full run, never the early-stop
    simulation.
  A robot that lunges and then rests scores 0. Height is not scored; add the
  height band of `SIG_GPNiceWalkingFitnessFunction` only if bobbing shows up
  in runs. Records one position per second with `SIG_GPFullDataRecorder`.
  Register it in `SIG_GPFitnessFunctionRegistry::fitnessFunctions()`. No
  file format changes.

- [ ] **164. The simulated fitness functions share their setup.** Each
  `evalFitness` copies the same code by hand: build the recorder and the
  simulation, catch the exception from `start()`, normalise the recorded
  positions, append the end position, and walk the list with the Inf and NaN
  test. `SIG_GPForceFitnessFunction` also copies the height band of
  `SIG_GPNiceWalkingFitnessFunction`. The few lines that score are hard to
  find. Put the shared part in one place, so that each `evalFitness` holds
  only its score: either a class that runs the simulation and hands back the
  normalised positions, or methods in `SIG_GPFitnessFunction`. The fitness
  values must stay identical.

- [ ] **169. The master sends the floor picture to the slave.**
  The job carries only the path of the floor picture, and each slave opens
  that path on its own host. `SIG_Environment::getTerrain` falls back to a
  flat floor when `SIG_Terrain::setFromPictureFile` fails, and the slave
  only prints a warning. The master is not told and records the fitness on
  the flat floor. An absolute picture path that exists on one host only made
  a run evolve for the wrong floor this way. `SIG_GPPVMData::savePVMDataTransfer`
  sends the picture's contents instead, so the slave opens no file. A
  picture the master cannot read stops the run with an error. The `.exp`
  keeps the path; its format does not change.

- [ ] **170. A dynamic client finds the master also by an IPv6 name.**
  `manage_dyn_slave` looks up the master's name with `gethostbyname`, which
  gives IPv4 addresses only. Where the name resolves only to IPv6,
  `sigelDynClient <master name>` stops with "invalid host", and only the
  master's IPv4 address works. The master's registration socket in
  `SIG_GPPVMDynamicClientServer` also takes IPv4 only. Both sides need IPv6:
  the master listens on both address families, and the client looks up the
  name with `getaddrinfo` and tries each address it gives.

- [ ] **172. The master is found on a host whose name maps to 127.0.1.1.**
  Debian and Ubuntu map the host's own name to 127.0.1.1 in `/etc/hosts`.
  PVM on the master then gives the other hosts 127.0.1.1 as the master's
  address, and they cannot reach it. Today GETTING_STARTED section 2.2 tells
  the user to edit `/etc/hosts`, which needs root and changes the system for
  every program. The x86 machine got around it without root by giving PVM the
  master's network address in a host file (`PVMHOSTFILE` with `ip=`).

---

## 7 · The robot language

- [ ] **160. The conditional instructions are `CMP`, `JLE` and `JMP`.**
  Today `JMP n` jumps only when `CMP` has set the compare flag, and no
  instruction jumps always. The set becomes:

  | Instruction | Meaning |
  |---|---|
  | `CMP a,b` | Sets the compare flag when register a <= register b. Unchanged. |
  | `JLE n` | Jumps n lines when the compare flag is set. Today's `JMP`. |
  | `JMP n` | Jumps n lines always. |

  Do item 166 first. Item 162 waits for this item.

- [ ] **162. Two buttons for two sets of default instruction probabilities.**
  On the GP parameters page: one sets all probabilities equal, one sets
  the conditional instructions `CMP`, `JLE` and `JMP` (item 160) lower than
  the others. Do item 160 first.

- [ ] **168. Consider one description for each robot instruction.** The name
  of an instruction and its number of operands are written out for each
  instruction in `SIG_ProgramLine` (`setRobotInstruction`, `printToString`,
  `writeToFile`, `readFromFile`, `randomRobotInstruction`), in the command
  list of `SIGEL_MasterGUI::SIG_LanguageParameters`, and by number of
  operands in `MT_Classifier::createDoubleTransIndi` and
  `MT_Substitute::translatedSIGProg`. A new instruction needs an edit in each
  place. Assess first if one table can serve these places; the call sites
  differ, so the gain is not certain.

---

## 8 · Experiments and robot models

- [ ] **166. Ship a second set of experiments that fits today's instructions.**
  The fixes to the robot instructions change how a saved program runs, so the
  stored fitness values in the shipped experiments no longer match, and some
  programs depend on a result that is now fixed. Keep the shipped files as the
  original 1.3 set. Add a second set: each experiment loaded and evolved for
  10 to 25 generations, so that the fitness values come from today's code and
  such programs are selected away. In the same pass, give the robot models
  less saturated colours. To decide: the name of the second set and where the two sets live.
  Item 160 waits for this item.

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
