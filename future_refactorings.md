# To do — after the Qt 6 port

Independent of the port. Do not combine commits across the two.

**Protocol:** one commit per item. Jan reviews and approves each. Items 1 and 2
are mechanical; everything from 3 on is discussed before it lands.

**Status:** `[ ]` open · `[~]` in review · `[x]` done · `[!]` blocked

**Counts are stale. Re-measure before acting on any item.** A 2026-09-02 audit
found item 1 is 6 sites not 7, item 2's lines are 125/505/607, item 5's table
sums to 322 not 313 and its per-module split has moved hard since Phase C, and
item 6 is 63 sites not 75. Paths are relative to
`x/kdesigelSources.1.3/kdesigel/kdesigel/`.

**Three things here are proposed, not approved:** the section ordering, item 5's
per-module split, and item 8's blocked status.

---

## 1 · Make it legal C++

Constructs the language removed. A current compiler rejects them.

- [ ] **1. Pre-standard headers** — `<iostream.h>` → `<iostream>`,
  `<vector.h>` → `<vector>`. `SIG_GPIndividual.cpp`, `MT_GPManager.cpp`,
  `MT_Randomizer.cpp`, `SIG_RobotScanner.cpp`, `SIG_SimulationQueries.h`,
  `SIG_DynaMechsSimulationQueries.h`.
  Does **not** retire `shim/`: the Makefile puts it on cv97's include path and
  32 vendored files need it.

- [ ] **2. `register` keyword** — 3 sites in `SIG_EnvironmentRenderer.cpp`.
  Removed in C++17. Delete the keyword.

- [ ] **3. Loop-variable scope leaks** — 13 files in `src/MT_GPSystem/`:
  `MT_FitnessTrainer`, `MT_GPManager`, `MT_Interpreter`, `MT_Population`,
  `MT_Program`, `MT_Randomizer`, `MT_Search`, `MT_Statistics`,
  `MT_StatisticsElement`, `MT_Tournament`, `MT_TournamentManager`,
  `MT_TranslatedIndividual`, `MT_Trainingset`. Hoist the declaration.
  The pattern is `for (int i=…){…}` then `i` used after the loop.

- [x] **4. Dynamic exception specifications** — done by the port, Phase A7.
  Three sites keep them only as `// NOTE: in 2003 this carried…` comments,
  which item 30 covers.

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
  | `schlussJetzt` | `SIG_GPManager.h` + 5 uses |
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
  **Check after each phase:** `./check.sh`, then
  `./dictorder-dump.sh | diff -u dictorder-baseline.txt -` empty, then
  `fitness-check.sh` clean.

- [ ] **15. Rename the `act` prefix to `current`.** German `aktuell`; reads as
  the verb "act". **`actExperiment` is already done** — 115 sites, renamed
  2026-09-16 along with `actExpChanged` and `slotActExpChanged`, a signal and a
  slot whose string-based connects went with them. **636 occurrences over 49
  distinct names remain**, measured over `src/` and `include/`: the largest are
  `actTour`, `actRealPosition`, `actInd`, `actSuccessor`, `actFitness` and
  `actFitTask`. Re-measure before starting.
  **Settle the scope first.** Most of what is left is in `SIGEL_GP`, which D33
  keeps untouched for behaviour; a rename is not behaviour, but it is still a
  large diff in a frozen module. Ask Jan whether to do it at all.
  Any signal or slot in the set breaks its string-based connect if only one side
  moves — `check.sh` has a check for exactly that failure. No reference file
  holds these names.

- [ ] **16. Set the version to 2.0 — the last commit of the port.**
  The tree disagrees with itself: `configure.in` says
  `AM_INIT_AUTOMAKE(kdesigel,1.0)` and is the only real declaration, `README`
  says `KDESIGEL v1.1 Readme File`, the About box in `SIG_InfoBox` prints
  `Sigel v1.1` and `pixmaps/altLogo.png` carries a `Sigel v1.0` caption. "1.3" exists only in the tarball
  and directory names. Set `configure.in` and the README, and move both About
  box and logo or 2.0 ships the same mismatch.
  **Do not touch `kdevprj_version`** — that is KDevelop's file format version.
  Nothing prints a version today — `sigel --version` does not exist. Whether it
  should is a separate decision.

---

## 6 · Defects preserved by the port

All present in 1.3, none introduced here. Each needs a decision before it is
touched, because changing one changes behaviour against the reference binary.

- [x] **17. `pvm_probe`'s error return is read as "a message is ready"** —
  done 2026-09-17, D41. `checkTask` now splits the three returns: above zero
  receive, zero wait, below zero give up on the task, kill it and re-queue the
  individual. The old `if (info != 0)` sent an error into the receive branch,
  where `pvm_recv` blocks for a message that cannot come — and **no
  `TIMEOUTMINUTES` value rescued it**, because the timeout lives in the `else`
  branch — or fails and leaves the task record destroyed while every caller in
  `SIG_GPManager` reads -1 as "not ready yet" and waits for ever.

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

- [x] **21. A run does not notice when the PVM daemon goes away** — done
  2026-09-17, D41. `pvm_probe` and `pvm_spawn` both report `PvmSysErr`, the
  trainer records it, and the interface ends the run and says why.
  **Two ways of detecting it were measured and rejected.** `pvm_mytid` cannot:
  `BEATASK` is `( pvmmytid == -1 ? pvmbeatask() : 0 )`, so an enrolled task gets
  its cached tid back without touching the daemon. Watching
  `$PVM_TMP/pvmd.<uid>` disappear cannot either: a run completed three further
  generations with that file removed, because an enrolled task keeps its socket.
  *Signature of the fault: main thread in `hrtimer_nanosleep`, seconds of CPU
  over hours, no `sigel_slave` at all.*

- [x] **22. Say why a run ended at once** — done 2026-09-17, D42.
  `SIG_GUIGPExperiment::slotEvolutionStopped` says so when the run completed no
  generation, nobody stopped it, and `terminationAlreadyMet` finds the condition
  already true. It names the setting that caused it and the tab to change it on.
  **Two cases it deliberately does not cover.** MetaGP with `SAVEEXIT` set —
  which all 14 shipped experiments carry — completes its first generation, so
  the branch never runs and only the counter moves. And a run that ends for any
  other silent reason says nothing rather than guess.

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
  stopgap. **Start at `SIG_IO::cerr`** — Jan's instruction; it is the hook.

- [ ] **30. Comments over two lines must earn their place.** A comment longer
  than two lines must carry something the code cannot say. **Comments the port
  itself wrote come first.** File by file, each pass shown to Jan first.
  **The named instance:** the nine lines above `setIconSize( QSize( 25, 25 ) )`
  in `SIG_MainWindow::SIG_MainWindow`, all nine describing what Qt 2 did. Two
  lines carry the whole fact — Qt 6 has one icon size per toolbar, and 25 is the
  largest of the small pixmaps, so nothing is scaled past what 1.3 drew.

- [x] **31. Confirm the run lock is finished** — done 2026-09-18.
  **No leftovers:** `g_runningEvolutions`, `SIG_GPManager::running()` and
  `evolutionRunningActionGroup` have no match in `src/`, `include/`, `ui/`,
  `guidrive.cpp` or `check.sh`. Of the other names holding "running",
  `evolRunning` and `metaEvolutionRunning` are MetaGP's own run state,
  `slotEvolutionRunning` is `MT_Controller`'s and `simulationRunning` is the
  slave's 3D playback — none is a second mechanism.
  **One mechanism:** `SIG_GUIGPExperiment::evolutionRunning`, read through
  `SIG_ExperimentListView::isRunning`, and one signal `evolutionNotRunning`
  driving `SIG_MainWindow::slotEnableEvolutionRunningActions` over 29 actions
  and the two `slotEvolutionNotRunning` slots.
  **The one gap is closed.** `startEvolutionAction` and `stopEvolutionAction`
  had no check; `runlock` now reads both at rest, during a run on the running
  experiment, and during a run on another experiment. Teeth-tested both ways.
  That is also what makes the right-click `Stop` crash unreachable — PORTING.md
  section 9.

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

- [ ] **39. Clear the 2003 Dortmund PVM hosts out of the experiments.**
  All 7 kept `.exp` carry them: 22 distinct host names — `wickie`, `bube`,
  `eiche`, `pappel`, `urobe` and the rest — in 130 `PVMHOST` lines, with slave
  directories under `/home/pg368/sawitzki/sigel` and
  `/home/pg368b/ross/projects/sigel`, and 14 `GRAVEYARDDIRECTORY` and
  `POOLIMAGEDIRECTORY` values under the same roots. None has existed since 2003,
  so every one of them is a spawn that fails.
  **The robot block carries them too.** Each of the 20 `Body` entries names its
  2001 directory, as in `Body base.wrl /home/pg368/sawitzki/hammer/ y`.
  `check.sh`'s `v2 round trip` section pins an md5 over hammer's whole robot
  block and an `expstruct` shape hash that covers those entries, so changing the
  paths moves both pins. The 2026-09-18 attempt changed them without saying so.
  **Where they are expected output:**
  - `pagesave-baseline.txt`: 16 host lines and 4 directory values — the port's
    own save of `twoBases.exp`. Regenerable here.
  - `v8-1.3-gp-blocks.txt`: the 20 hammer host names, in the order the
    `v2 round trip` section compares against.
  - `check.sh`: the three pinned `pvmhost` lines of that section, and its
    host-parser test.
  - `guibehaviour-baseline.txt`: 19 lines — 10 from the `pages` scenario, 2 from
    `exportall`, 7 from `dialogs`. Regenerable here.
  `v6-1.3-friction-nocollide.txt` names two such paths in its method note; no
  check reads them. `dictorder`, `fitness`, `guidump` and `xtest` carry none.
  Every scenario behind `guibehaviour-baseline.txt` and `pagesave-baseline.txt`
  loads `twoBases.exp`, so their host lines move with that file, and both are
  the port's own output. Map distinct placeholders consistently across the
  experiments, those two files and `check.sh`'s host-parser test.
  **Copy both data trees before the first write.** The 2026-09-18 attempt did
  not, and lost the only copy of `data-reordered/`.
  **Decide first what replaces them** — this machine, or an empty list. An empty
  list makes `slotStartEvolution` run with nowhere to spawn, which is a
  different silent failure from the one D41 and D42 just closed.

- [x] **40. Keep one two-bases experiment; remove the other five, here and on the
  x86 machine.** Jan judged all six side by side on 1.3 and the port, 2026-09-19,
  each with its best individual by the port's own scoring: **keep
  `twoBasesHardlyReducedIS`**; remove `twoBasesHighMutationRate` (a close second),
  `twoBasesSimpleFitness1`, `twoBasesReducedInstructionSet`,
  `twoBasesHighCrossOverRate` (the same file as `twoBasesSimpleFitness1`) and
  `twoBasesSimpleFitness2`. Jan on the last: *"it's broken everywhere and
  provides no value"* — its individual 0 scores 3.4e-05 on both, and its best
  (number 78) moved only slightly.
  **Every one of the five is in `dictorder-baseline.txt` and
  `fitness-baseline.txt`**, so both move. `twoBasesSimpleFitness1` is also the
  input of `check.sh`'s `expstruct selfcheck`, and `replicate.sh` names it and
  `twoBasesHighCrossOverRate` as the identical pair.
  **`twoBasesSimpleFitness2` is the hard one.** It is the experiment most checks
  load, so removing it moves them all:
  in `check.sh` it drives six sections — `gui behaviour`, `real clicks` and
  `pagesave` through `BEXP`, and `no clipped controls`, `form minimums` and
  `slave gui`, which name the file directly; `guidrive.cpp` loads it by default;
  `guibehaviour-baseline.txt` names it 24 times, `xtest-baseline.txt` 2,
  `fitness-baseline.txt` 3, and `dictorder-baseline.txt` has its section.
  **`pagesave-baseline.txt` was 1.3's own output for this file**, and the port
  matched it. It now holds the port's own save of `twoBases.exp`; no new
  capture from the oracle — see item 44.
  Removing it also ends item 39's conflict over this file.
  **Done on both machines 2026-09-19.** On the x86 machine with Jan's approval
  there; its seven files hash as ours. Its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.

- [ ] **41. The simulation viewer starts too close, and follows the robot.** Jan,
  2026-09-19: *"On both machines and SIGEL versions we're defaulting to trace
  robot and are zooming in way way way too much."* Both come from the form and
  are the same in 1.3's: `distanceSlider` in `SIG_SimulationWidgetBase.ui` starts
  at 10 of 2 to 200, page step 20; trace robot is on — `traceRobotCheckBox`
  starts ticked, `SIG_SimulationVisualisationWidget`'s constructor sets
  `traceRobot` true, and `visualizeThis` calls `slotSetTraceRobot( true )` again.
  To see the twoBases and hammer robots on 1.3, the oracle moved the slider 3
  page steps out, from 10 to 70. A new default is a deliberate divergence from
  1.3; the value, and whether trace stays on, are Jan's to set.

- [ ] **42. The 3-D view puts the robot at a corner of the grid, not in its
  middle.** Jan, 2026-09-19: *"in 3D View we're placing the robot at the corner
  of the grid, not centered"*. What the code does: `SIG_EnvironmentRenderer::
  buildGrid` draws the terrain grid from the origin out to the terrain's size,
  with the grid spacing forced to 1 because, by its own note, `getTerrainData`
  does not set it; the robot starts at the experiment's `STARTPOSITION`, which is
  `0 1 0` in `twoBasesSimpleFitness1`, so at the grid's corner. The same code is
  in 1.3. Not yet compared with 1.3 by eye.

- [ ] **43. The ambient light slider seems to do nothing in the 3-D view.** Jan,
  2026-09-19: *"in 3D mode the ambient slider doesn't appear to DO anything?!"*
  What the code does: `SIG_VisualisationWidget::setAmbientLighting` calls
  `SIG_Visualisation::setAmbientSceneColor`, which keeps the value for the
  POV-Ray export and calls `glLightModelfv( GL_LIGHT_MODEL_AMBIENT, … )` straight
  from the slider's slot, outside `paintGL`. Two things to check first, neither
  measured: a Qt 6 `QOpenGLWidget` has its context current only inside
  `initializeGL`, `paintGL` and `resizeGL` unless `makeCurrent()` is called, so
  the call may reach no context; and ambient light has no effect while lighting
  is off, which may be so in wireframe mode. Not yet compared with 1.3.

- [x] **44. Keep one octopus experiment; remove `octopusSimpleFitness`, here and
  on the x86 machine.** Jan, 2026-09-19: *"once again two octopus experiments -
  ok, we'll keep only one! the first one, the current one is a failed/early
  run"*. Keep `octopusNiceWalkingFitness`, judged a keeper side by side with
  individual 13. The same wait as item 40: not before every experiment has been
  judged, and not unasked.
  **What names it:** `fitness-baseline.txt` (3 lines), `dictorder-baseline.txt`
  (its section) and `check.sh`'s v2 round trip, which loads it as `V2OCT` for the
  containers that collide.
  **That check compares against 1.3's own output for this file**, in
  `verification-against-sigel-1.3/v1-1.3-roundtrip.txt`, and the port matches it.
  The two octopus robot blocks differ — sha256 `2b56be22…a2d1` against
  `8934a27c…fc2d` for `octopusNiceWalkingFitness` — so the kept file has no 1.3
  capture. It needs none: moved to the kept file, the check takes the port's own
  output as its reference. Jan, 2026-09-19: *"since it passes why do we keep
  re-checking a known fact? Future refactorings only need to check against the
  now-proven Qt6 baseline"*. The same holds for item 40.
  The 2026-09-18 oracle measurements named "octopus" in PORTING.md were taken on
  this file; they stay as the record of what was measured.
  **Done on both machines 2026-09-19.** On the x86 machine with Jan's approval
  there; its seven files hash as ours. Its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.

- [x] **45. Keep one runner experiment; remove `runnerSimpleFitness`, here and on
  the x86 machine.** Jan, 2026-09-19, judging it side by side with individual 30:
  *"to be removed, the first one was the better experiment"*. Keep
  `runnerNiceWalkingFitness`. The same wait as item 40.
  **What names it:** `fitness-baseline.txt` (3 lines) and
  `dictorder-baseline.txt` (its section). No check loads it.
  `verification-against-sigel-1.3/` holds oracle measurements on it; they stay as
  the record of what was measured.
  **Done on both machines 2026-09-19.** On the x86 machine with Jan's approval
  there; its seven files hash as ours. Its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.

- [x] **46. Rename the kept experiments to their base names, here and on the x86
  machine.** Jan, 2026-09-19: the names carry notes added run by run as a kind of
  versioning, such as the fitness function used; *"when we're done with our
  review we need to strip all such versioning and just keep the base names! We'll
  do this interactively, you suggest and I approve."* One name at a time, after
  the review and after items 40, 44 and 45. Both machines use the same names, so
  the two stay comparable. Each kept experiment takes its robot's name.
  Approved: `twoBasesHardlyReducedIS` → `twoBases`; `hammerNiceWalkingFitness` →
  `hammer`; `insectNiceWalkingFitness` → `insect`; `octopusNiceWalkingFitness` →
  `octopus`; `runnerNiceWalkingFitness` → `runner`;
  `shortHammerNiceWalkingFitness` → `shortHammer`; `walkerNiceWalkingFitness` →
  `walker`. All seven approved 2026-09-19.
  **Done on both machines 2026-09-19.** On the x86 machine with Jan's approval
  there; its seven files hash as ours. Its copy of all 14 as downloaded is
  `/home/debian/sigel-shipped-original-2026-09-19/`.

- [ ] **35. Remove the Windows and Visual Studio support.** Decided by Jan
  2026-09-09. It does not build here and nothing tests it.
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
  `encodings` check, so `check.sh` goes from 1169 pass to 1158. PORTING.md pins
  that number twice — the per-step exit criterion in §7 and the check list. Move
  both in the same commit.
  **Do not mix it with any other change. When:** after the MetaGP guard step and
  its review.

- [ ] **36. `SIG_Body::usedByLinks` is dead, and
  `SIG_Material::FrictionValue` could be a value type.** Left converted rather
  than changed during D11, on instruction, because the port moves the Qt API and
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

---

## Not doing

Decisions, not work. Each is settled; reopen only with a reason.

- **Restore Qt 2's spin-box editing.** D28. Reachable only by typing a number
  outside a box's range, the value is visible before anything is saved, and the
  cost is owning a custom widget for ever. Current behaviour is pinned in
  `guibehaviour-baseline.txt`. The worked-out `SIG_SpinBox` subclass, the three
  cheaper routes that were measured and rejected, and the 47-widget promotion
  plan are in this file's history at `1dba5f4`.
- **The history block grows by one line per individual per save.** Confirmed on
  the 1.3 binary; the port reproduces it exactly, which is the correct outcome.
  Recorded so nobody "fixes" it and silently diverges. If it is ever changed
  deliberately, that is a product decision and needs a note in PORTING.md saying
  the port stopped matching 1.3 on purpose.
- **A lower bound on the terrain index in DynaMechs.** Vendored code is not
  touched.
- **Validating the terrain header in `SIG_Environment`.** After the atomic
  write, nothing in the tree produces a `Terrain.ter` that exists and does not
  parse, and the guard would have to be repeated in the second reader,
  `SIG_DynaMechsSimulationData`, which this port does not touch.
- **The malformed-picture-file branch of `generateTerrain`.** Reached only with
  `FLOORFUNCSELECTED 0`; `operator>>(istream &, string &)` leaves its string
  unchanged when the sentry fails, so the previous token is reused and the
  terrain header comes out malformed. No P2 file exists in the repository and all
  14 shipped experiments have an empty `FLOORPICTUREFILE`, so a fix would be code
  no check could exercise. Reopen only if a picture-file experiment ever exists.

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
