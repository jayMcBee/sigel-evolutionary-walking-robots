# To do — after the Qt 6 port

Independent of the port. Do not combine commits across the two.

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

- [ ] **2. `register` keyword** — 3 sites in `SIG_EnvironmentRenderer.cpp`.
  Removed in C++17. Delete the keyword.

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

- [ ] **13. Translate the German — strings.** 9 lines in 4 files:
  `MT_GPManager.cpp`, `SIG_GPIndividual.cpp`, `SIGEL_GP/SIG_GPManager.cpp`,
  `SIG_GPOperations.cpp, reproduction`. The strings: "beste Fitness vor
  Berechnung", "Sigel Fitness/Sieger", "Meta Vorhersage", "durch.Fitness",
  "Fitness (Elter 1)", "(Elter 2)", "SIG_GPManager::run() wurde mehr als einmal
  aufgerufen!" — **the same string twice** — and "reproduction: Konnte kein neues
  Individuum erzeugen".
  **No compiler and no check covers this phase.** All three persisted paths are
  write-only: `SIG_GPIndividual::readFromFile` parses only `NAME='`, `POOLPOS=`,
  `FITNESS=`, `AGE=` and `PROGRAM BEGIN{`; the `MT_GPManager` block sits after
  that file's own "is not loaded" marker; the ZORC format carries no German.
  **Phase 0 could not have caught these** — they are pure ASCII, and Phase 0
  swept for bytes above 127. A check for German must look for words, not bytes.

- [ ] **14. Translate the German — symbols.** 217 lines in 39 files, not
  user-visible. Every miss is a compile error, so `check.sh` verifies it.

  | symbol | where |
  |---|---|
  | `liesdas` | `SIG_Scanner`, `SIG_RobotScanner`, `SIG_UnstreamerScanner`, + 3 `.cpp` |
  | `getRandomInstruktion`, `ProbInstruktion` | `MT_Randomizer.h` |
  | `T_Instruktion`, `T_Instruk` | `MT_TranslatedIndividual.h` |
  | `Instruktion` | `MT_Classifier.cpp, createDoubleTransIndi` |
  | `set`/`getSelektionValue` | `MT_FitnessTrainer.h`, `MT_GPManager.h` |
  | `Varianz` | `MT_StatisticsElement.h` |
  | `winkel`, `verschiebung`, `schiebung`, `drehmatrix`, `hilf`, `stflorianhilf` | `IFunctions.h`, `IFunctions.cpp` |
  | `masse` | `SIG_Mirtich.h`, `.cpp` |
  | `dichte`, `konstante`, `anderes_material`, `rot`/`gruen`/`blau` | `SIG_RobotCompiler.cpp` |
  | `betrag`, `betraege`, `varianz`, `durchschnittProGelenk` | `SIG_GPForceFitnessFunction.cpp` |
  | `ausgabeTerrain` | `SIG_Environment.cpp, generateTerrain` |
  | `zeiger` | `SIG_EnvironmentRenderer.cpp` |
  | `zahl` | `SIG_Geometry.cpp` |

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

- [ ] **16. Set the version to 2.0 — the last commit of the port.**
  The tree disagrees with itself: `README` says `KDESIGEL v1.1 Readme File`, the
  About box in `SIG_InfoBox` prints `Sigel v1.1`, and `pixmaps/altLogo.png`
  carries a `Sigel v1.0` caption. "1.3" exists only in the tarball name.
  **Rewritten 2026-09-20:** `configure.in` used to be the only real declaration
  and this item said to set it; it went with the 2003 autotools build, so the
  three above are now the whole story. Move all three or 2.0 ships the same
  mismatch. **Decide first whether a version is declared anywhere at all** —
  nothing prints one today, `sigel --version` does not exist, and the honest
  alternative is that the About box is the only place a user ever sees it.

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

---

## 7 · The interface

- [ ] **38. Does the poll interval in `evolutionLoop` earn its length?**
  The wait before `checkTask` carried the comment *"experiments show gain in
  performance when we add some minor delay"* from 2003, and nobody has measured
  it here. It is a poll interval: `checkTask` only asks whether a result has
  arrived, so a longer wait leaves every finished result sitting, and the
  successor tournament waiting behind it. At 120 individuals a generation that
  is not a small number.
  **What little is measured:** cutting it from 300 ms to 200 ms moved one
  two-generation run from 73.3 s to 69.8 s per generation. One run each, a
  randomised pool, one machine — that is not a result, only a reason to think
  the claim is worth testing.
  **How:** the `evolution` scenario already prints ms per generation and the
  worst gap between pumps. Sweep the value, hold everything else, repeat enough
  runs to see past the noise.

- [ ] **23. The window stops answering during a run.**
  `SIG_GUIGPExperiment::slotStartEvolution` runs the whole evolution on the GUI
  thread. Input is handled in `SIG_GUIGPManager::haveABreak`, which pumps events
  for at most `getPassiveTime()` milliseconds. `SIG_GPManager::evolutionLoop`
  calls it once per outer pass, and the inner loop over `taskCanDoList` calls
  `usleep(300000)` per entry, so the gap between pumps grows with the entries a
  pass visits. `evalNewIndis` and `evalNeededIndis` also call `haveABreak`, and
  `SIG_GPPopulation::writeToFile` calls `processEvents` of its own — a
  measurement needs all of them. **Not measured:** how long that gap is.
  **Constraint:** the pump and the sleep are in `SIGEL_GP`, which D33 keeps
  untouched. Needs a decision first.

- [ ] **24. Show progress during a run.** Partly done. D38 drives
  `generationProgBar` from the count of individuals holding a fitness value,
  which steps back when a tournament makes offspring. D37 writes the pool
  generation into `lcdnumberGenerations` from
  `SIG_GUIGPManager::updateIndividualView`, which also calls
  `SIG_IndividualListItem::setTo` as results come in.
  A true per-generation figure needs `tours`, `taskCanDoList` and
  `currentGenerationNo`, all private to `SIG_GPManager`, so it needs new state
  there and D33 forbids it.
  **The matching call in the `SIG_GUIGPManager` constructor stays commented out**
  — `slotStartEvolution` already refreshes the counter through
  `putAllIntoExperiment`. 1.3 has both commented out.
  **No check covers the counter during a run, because no check starts a run.**
  Only the unchecked `evolution` scenario samples it; `runlock` covers the write
  in `slotEvolutionStopped`. **Untried:** a check without PVM may be possible
  with a run that has no tournaments.

- [ ] **25. Dialogs with no parent can end up out of sight.** D36. A modal
  prompt with no parent blocks the window, Quit included — seen when
  `SIG_ExperimentListView::slotSaveExperiment`'s overwrite prompt sent
  `set_modal` and no `set_parent` on Wayland. D35 covered the eight save and
  export file dialogs; D39 finished the interface, all 32 sites in
  `SIGEL_MasterGUI`. **What is left, counted after D35:** 24 of 100
  `QMessageBox` calls pass parent `0`, and 13 of 43 `QFileDialog` calls pass
  `nullptr`. D39's list has the detail, part of it
  in `MT_Controller`, `SIG_GPPopulation` and `SIG_GPRemoteZORCFitnessFunction`,
  which D33 keeps untouched. **Not measured:** whether a parent attaches GTK's
  own file dialog on Wayland.

- [ ] **26. The 3D camera never sizes the view to the robot.** Not a port
  defect — every camera file matches pristine 1.3. Two 2003 faults:
  the distance is a constant (`SIG_SimulationWidget::slotSetDistance` divides
  `distanceSlider` by 10, the default 10 gives 1.0 world unit, and the 100 degree
  field of view shows about 2.4 units, against robots 3 to 13 units long —
  hammer 12.9, shortHammer 10.9, octopus 7, twoBases 5, walker 4 plus legs,
  insect 3); and both the tracking block in
  `SIG_SimulationVisualisationWidget::makeTimeSteps` and `slotNavigateCenter`
  aim at the **root link's model origin**, not the robot's centre.
  **A fix needs a measurement that does not exist:** walk `SIG_Robot::getLinks`,
  place each `SIG_Geometry::getVertices` with `SIG_Link::getInitialLocation`,
  reduce to a box. That belongs next to `SIG_Robot`, not the interface. Then set
  the **slider**, not the widget, from `margin * halfExtent / tan(fovy / 2)`, or
  the next slider touch snaps the view back — the slider's maximum of 200,
  distance 20, is enough for every shipped robot. Add the box centre, rotated by
  `SIG_SimulationVisualisation::getRobotRotation`, to the look point. `fovy` is a local constant in
  `SIG_Visualisation::updateAspectRatio` and needs one shared home.
  *Found alongside:* `SIG_SimulationVisualisationWidget::visualizeThis` calls
  `resizeGL` with logical pixels where Qt uses device pixels. Not measurable
  here; the harness pins the ratio to 1.

- [ ] **27. Name the three signals in the wildcard disconnect.**
  `SIG_AllIndividualsView::slotEvolutionNotRunning` calls
  `QObject::disconnect(individualList->listviewIndividuals, 0, 0, 0)`,
  byte-identical to 2003, and Qt 6 warns on
  stderr each time an evolution starts. The code below it reconnects the same
  three signals. **Not before the `gui behaviour` check covers an actual
  evolution** — that path is unguarded today.

- [ ] **28. Check every UI label for grammar and typos.** One pass over every
  label, button, menu entry, tooltip and dialog title. Most are in `ui/`; the
  rest are literals in `src/`. `pushbuttonShowFitnessCurve` is one already
  spotted — "Fitnesscurve" also appears in
  `SIG_ExperimentView::slotShowFitnesscurve`.

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
  `MT_Controller::slotSaveSetup` — the last in a module D33 keeps untouched.
  They pass no parent, so they are item 25 sites too — but **delete rather than
  re-parent them.**

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

- [ ] **41. The simulation viewer starts too close, and follows the robot.** Seen
  2026-09-19 on both machines, in the port and in 1.3: trace robot is on by
  default, and the view starts far too close. Both come from the form and
  are the same in 1.3's: `distanceSlider` in `SIG_SimulationWidgetBase.ui` starts
  at 10 of 2 to 200, page step 20; trace robot is on — `traceRobotCheckBox`
  starts ticked, `SIG_SimulationVisualisationWidget`'s constructor sets
  `traceRobot` true, and `visualizeThis` calls `slotSetTraceRobot( true )` again.
  To see the twoBases and hammer robots on 1.3, the oracle moved the slider 3
  page steps out, from 10 to 70. A new default is a deliberate divergence from
  1.3; the value, and whether trace stays on, need sign-off.

- [ ] **42. The 3-D view puts the robot at a corner of the grid, not in its
  middle.** Seen 2026-09-19: in the 3-D view the robot stands at a corner of the
  grid, not at its centre. What the code does: `SIG_EnvironmentRenderer::
  buildGrid` draws the terrain grid from the origin out to the terrain's size,
  with the grid spacing forced to 1 because, by its own note, `getTerrainData`
  does not set it; the robot starts at the experiment's `STARTPOSITION`, which is
  `0 1 0` in `twoBasesSimpleFitness1`, so at the grid's corner. The same code is
  in 1.3. Not yet compared with 1.3 by eye.

- [ ] **43. The ambient light slider seems to do nothing in the 3-D view.** Seen
  2026-09-19: moving the ambient slider in the 3-D view shows no change.
  What the code does: `SIG_VisualisationWidget::setAmbientLighting` calls
  `SIG_Visualisation::setAmbientSceneColor`, which keeps the value for the
  POV-Ray export and calls `glLightModelfv( GL_LIGHT_MODEL_AMBIENT, … )` straight
  from the slider's slot, outside `paintGL`. Two things to check first, neither
  measured: a Qt 6 `QOpenGLWidget` has its context current only inside
  `initializeGL`, `paintGL` and `resizeGL` unless `makeCurrent()` is called, so
  the call may reach no context; and ambient light has no effect while lighting
  is off, which may be so in wireframe mode. Not yet compared with 1.3.

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

- [ ] **35. Remove the Windows and Visual Studio support.** Decided
  2026-09-09. It does not build here and nothing tests it.
  **It could not build in 2003 either:** `Sigel.dsw` lists 13 projects and only
  5 exist; the project files are Visual C++ 6 and link `msvcirt.lib`, which
  Microsoft removed, and `qt-mt230nc.lib`, Qt 2.3.0 for Windows, which is not
  obtainable; Release and Debug link PVM from different paths.
  **What is there:** 9 Visual Studio project files at the source root, 7,962
  lines — `.dsp` for `Sigel`, `SIGELCommon`, `MetaSIGEL`, `sigel_slave` and
  `manage_dyn_slave`, plus `Sigel.dsw`, `Sigel.mak`, `sigel_slave.mak` and
  `manage_dyn_slave.mak`; the 2 `WIN_SIG_GPRemoteZORCFitnessFunction` sources,
  451 lines, already excluded by `EXCLUDE_SIGEL_GP`; **206 `_WINDOWS`
  occurrences across 57 files** — 195 `#ifdef`, 9 `#ifndef` and **2 inside
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
  **The two `WIN_` files are the `SIG_GPExperiment` trap again:** they declare
  the same class, with the same include guard, as
  `SIG_GPRemoteZORCFitnessFunction`, and only the project file chose between
  them. `sigel_slave.cpp` includes the `WIN_` header inside its `_WINDOWS`
  branch, so delete that branch and the two files in the same commit.
  **The gates cannot see deleted code**: nothing ever compiled it. Read every
  diff.
  **How:** delete the project files and the `WIN_` sources first, with
  `EXCLUDE_SIGEL_GP` and the four `winskip` sites in `check.sh`. Then take the
  `#ifdef _WINDOWS` blocks one module at a time, keeping the `#else` half; the
  object file must not change.
  **Watch:** the 9 `#ifndef _WINDOWS` blocks are reverse polarity — the body is
  kept and only the guard goes. **12 headers carry `_WINDOWS`**, and a mistake
  in one changes what every including translation unit sees: `MT_Controller.h`,
  `MT_Substitute.h`, `MT_GPManager.h`, `SIG_GPManager.h`, `SIG_Program.h`,
  `SIG_DynaMechsSimulationQueries.h`, `SIG_Recorder.h`, `SIG_Register.h`,
  `SIG_SimulationQueries.h`, `SIG_EnvironmentRenderer.h`,
  `SIG_EnvironmentVisualisation.h`, `SIG_Renderer.h`.
  **It supersedes D22**, a signed decision: Fusion for the `#else` half, and the
  `#ifdef` half keeps Windows by name because Qt 6 still creates that style.
  Three live call sites — `sigel.cpp, main` and two in `sigel_slave.cpp`.
  Keeping the `#else` half deletes them, which is right once Windows is gone, but
  say that a decision is being overturned.
  **It overturns three "permanent" statements about the `WIN_*` files,** all to
  be edited in the same move: PORTING.md's Phase C exclusions ("an explicit
  exclusion rather than a standing gap"), its C7/C8 row ("one remains and always
  will"), and `check.sh`'s "Windows-only WIN_* file(s) excluded -- permanent".
  The `winskip` counter behind that line goes with them, and its line
  disappearing will look like a lost check unless it is done knowingly.
  **It moves a pinned check total:** the 11 deleted files are counted by the
  `encodings` check, so `check.sh` goes from 1097 pass to 1086. PORTING.md pins
  that number in three places — the per-step exit criterion in §7, the
  Handover's note on uncommitted work and the check list — and its trail in §7
  records each step. Move all of them in the same commit.
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
