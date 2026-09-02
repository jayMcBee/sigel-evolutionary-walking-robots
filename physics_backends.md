# SIGEL physics backends — Dynamo removed, DynaMechs kept

**DONE.** Analysed 2026-08-20, executed 2026-08-28, independently of the Qt 6
port. SIGEL shipped two physics engines and chose one at run time. Dynamo
crashed on most shipped robots, its own authors labelled it "not recommended",
and all 14 shipped experiments selected DynaMechs. It is gone.

This file records what was removed, what was deliberately kept and why, and the
three follow-ups that were not taken. It is not a proposal — the decision has
shipped.

---

## The two engines

- **Dynamo** — "Dynamic Motion library", Bart Barenbrug, TU Eindhoven,
  1996–1999. Constraint-based. LGPL. **Deleted.**
- **DynaMechs** — Scott McMillan. Articulated-body (Featherstone). **Kept.**

Both implemented `SIG_SimulationData`, `SIG_SimulationQueries` and
`SIG_CommandInterface`; `SIG_Simulation.cpp` switched between them at run time
on the `SIMULATIONLIBRARY` key, which is `1` in 14 of 14 shipped `.exp`.

---

## What was actually done — 2026-08-28

### SOLID and qhull went too

The Dynamo backend was deleted in `5addd66`. **SOLID and qhull followed in a
separate commit**, because their only caller was the deleted code:

| library | files | lines no longer compiled |
|---|---|---|
| SOLID | 16 | 1,999 |
| qhull | 11 | 20,705 |
| **total** | **27** | **22,704** |

Measured before removing, not assumed: **zero `dt*` SOLID API calls anywhere in
SIGEL, zero in vendored DynaMechs, and zero `qh_*` references outside qhull
itself.** qhull existed only to give SOLID its convex hulls — that is what the
`-DQHULL` flag selected — so it could not outlive it.

The one surviving mention is `maximalSOLIDIterations`, a simulation parameter
still parsed, stored and written back but now read by nothing. It joins the six
others the Dynamo removal left in that state.

`libdynalib.a` is the one that could **not** go: see the deletion commit. Its
maths half is not separable from its physics half.

### Deleted

**SIGEL's 13 Dynamo file pairs, 26 files, 3,226 lines.** `SIG_Dyna`,
`SIG_DynaCallbacks`, `SIG_DynaDrive`, `SIG_DynaJoint`, `SIG_DynaLink`,
`SIG_DynaMoCommandInterface`, `SIG_DynaMoSimulationData`,
`SIG_DynaMoSimulationQueries`, `SIG_DynaSensor`, `SIG_DynaSystem`,
`SIG_DynaSystemWrongNumberException`, `SIG_RotationalController`,
`SIG_TranslationalController` — every one of them `.cpp` and `.h`.

**3,226, not the 3,247 counted above.** The list of files was right; four of
them changed size between 2026-08-20 and the deletion, all in Phase D:
`SIG_DynaSystem` −8, `SIG_DynaMoSimulationData` −15,
`SIG_DynaMoSimulationQueries` +1, `SIG_DynaMoCommandInterface` +1.

**The backend switch.** `SIG_Simulation.cpp` keeps its `DynaMechs` case and
gains a `default:` that prints to `std::cerr` and throws. It does not fall
through — the previous switch had no `default` at all, so an unknown library
would have left `simulationData`, `simulationQueries` and `commandInterface`
uninitialised. Verified by building a copy of `sigel_eval` without its own
`SIMULATIONLIBRARY` guard (`sigel_eval.cpp:242`) and running an `.exp` edited to
`SIMULATIONLIBRARY 0`: one line of diagnostic, then abort, exit 134. It never
falls through and never runs DynaMechs.

**"Loud" is true of `sigel_eval` only.** The throw clears all six fitness
functions, which construct `SIG_Simulation` outside their own `try` — but one
frame further out, `sigel_slave.cpp:361-367` wraps `evalFitness()` in
`catch (SIG_Exception &) { fitnessValue = 0; }`. So under PVM the exception is
swallowed and the individual is scored 0.0 as though evaluated, the failure
`SIG_GPSimpleRecorder.cpp:42` documents. The printed line is therefore the only
evidence anyone gets, which is why it goes to `std::cerr` and not the buffered
`SIG_IO::cerr`. Hardening that `catch` is pre-existing and out of scope.

**Three dead includes and one wrong one.** `SIG_SimulationQueries.cpp` included
`SIG_DynaLink.h`, `SIG_Dyna.h` and `SIG_DynaSensor.h` and used none of them.
`SIG_Environment.h:26` included `<constraint.h>`, a Dynamo *physics* header,
only to reach `DL_vector`; it is `<pointvector.h>` now, as this document asked.

**One `moc` target.** `SIG_DynaSystem.h` is off `MOC_HDRS`.

**`SIG_Simulation` stays a `QObject`, and that is now vestigial — recorded
rather than glossed.** The deleted `connect()` was the only thing that wired
`slotDynamoMessage`, which is the class's only slot; it declares no signals. So
`Q_OBJECT`, the `QObject` base and the surviving `moc` target exist for a slot
nothing can invoke. Two consequences follow and are annotated in the source:
`stopSimulation` had no other writer, so `makeTimeSteps`' `if (stopSimulation)`
is permanently false — and that is the **only** throw site of
`SIG_SimulationCannotSolveException` in the tree, which makes the 2003-behaviour
boundary around `start()` and `makeTimeSteps()` guard a type that can no longer
arrive. All of it is left in place: removing a slot changes the Qt surface of a
class Phase C still has to port, which is a different decision from deleting a
physics backend.

**46 of the 60 vendored Dynamo `.cpp` stop being compiled**, 10,084 of 13,567
lines. Nothing vendored is *deleted* — that tree is untracked and comes out of
a tarball — only the `dynamo_SRC` list the build feeds `ar`.

**Four build files, and the fourth is the one that matters.** Both `Makefile.am`
in `SIGEL_Simulation` lose the 39 filenames that no longer exist, and
`SIGELCommon.dsp` — the 2003 Visual Studio project — loses 27 `Source File`
blocks: the 26 files plus `moc_SIG_DynaSystem.cpp` and its two custom-build
rules.

**`kdesigel.kdevprj` is the fourth, and it is not a leftover.** Both `Makefile.am` carry
the marker `####### kdevelop will overwrite this part!!! (begin)`, so the
`.kdevprj` is what KDevelop 2 **regenerates them from**. Left alone, the next
regeneration would put `SIG_DynaSystem.cpp` and its twelve siblings back into
`libSIGEL_Simulation_a_SOURCES` and the autotools build would fail on missing
files. It named the 26 deleted files in 28 places — two `files=` manifests and
26 per-file sections, all now removed. The other four `.dsp` projects were
checked and never named them.

### Kept, and why

**The maths library, and more of Dynamo than expected.**

- `pointvector.cpp` and `list.cpp` are genuinely empty — "no non-inline
  methods". `DL_vector` and `DL_point` really are header-only.
- **`matrix.cpp` is not.** It carries 27 out-of-line `DL_matrix` members, all 28
  of its symbols link into `sigel_eval`, so `libdynalib.a` **cannot** be dropped
  from the link line. Tested, not assumed.
- **`matrix.cpp` is not pure maths either.** It `#include`s `dyna_system.h` so
  that `DL_matrix::invert` can report a singular matrix through the physics
  engine's global callback: `matrix.cpp:233`,
  `DL_dsystem->get_companion()->Msg("singular matrix can't be inverted\n")`.
  That call, plus the `DL_geo` vtable `matrix.o` emits, is an undefined
  reference to `dyna_system.o` and `geo.o`, whose closure is nine more physics
  translation units.

So **the maths and the physics are not cleanly separable**. The archive holds
14 objects and **the linker pulls 12 of them**: `pointvector.o` and `list.o`
define no symbols at all and are never extracted, so they are compiled only to
keep the maths half of the library named rather than implied. The 11 physics
objects below are all pulled. Measured with `nm` over all 60 objects and
confirmed against a linker map; each entry names the symbol that pulled it in:

| object | pulled in by |
|---|---|
| `dyna_system` | `DL_dsystem` |
| `geo` | `DL_geo::move` |
| `dyna` | `DL_dyna::newkinenergy` |
| `constraint` | `DL_constraint::reset_undo` |
| `constraint_manager` | `DL_constraints` |
| `euler` | `DL_euler::DL_euler` |
| `m_integrator` | `DL_m_integrator::stepsize` |
| `largematrix` | `DL_largematrix::prep_for_solve` |
| `supvec` | `DL_supvec::A2q` |
| `force_drawable` | `DL_force_drawable::get_fd_info` |
| `vector4` | `DL_vector4::assign` |

3,056 lines of physics survive for one error message. Nothing is stubbed and no
symbol is defined away: breaking the coupling means patching a diagnostic out of
a vendored file, which is a separate decision and was not taken.

**The whole Dynamo include path.** `-isystem .../Dynamo/Src/Inc` stays on both
the build and `check.sh`: the maths headers live in the same directory as the
physics ones.

**`patches/dynamo-containerlist-null.patch` is now dead but kept.**
`containerlist.h` is included by `containerlist.cpp` and by nothing else, and
that is one of the 46 sources no longer compiled. The patch still applies
cleanly against the untracked tree, so it was left alone rather than removed in
this changeset.

~~**SOLID is still built and still linked.**~~ **NO LONGER TRUE, and this
paragraph contradicted the section 70 lines above it.** SOLID and qhull went
with the Dynamo backend on 2026-08-28; no `libsolid.a` exists in either build
tree. Only the `-isystem .../SOLID-2.0/include` path survives. This document claimed SOLID would go
with Dynamo. Its 15 API references were indeed all in deleted files, but
removing `libsolid.a` was outside the brief for this change and was not
attempted. It is now dead weight — ~4,800 lines and a vendored patch — and
dropping it is a one-line follow-up, to be measured the way `libdynalib.a` was.

### Gates — all clean, which is the point

| gate | result |
|---|---|
| `dictorder-dump.sh` vs baseline | **empty diff** |
| `fitness-check.sh` vs baseline | **empty diff**, 42 of 42 |
| `sigel_eval -selfcheck` | pass |
| sanitized `fitness-check.sh build` | **empty diff**, no ASan or UBSan report |
| `check.sh` **as of 2026-08-28** | **105 pass, 4 fail** — see PORTING.md for the current figure, 842/0 |

`check.sh` was 118 pass, 4 fail before this change. The pass count falls by
exactly 13 because 13 fewer `.cpp` exist, and "headers standalone" by exactly 13
for the same reason. The warning count is not quoted here because Phase D keeps
moving it; §7 of PORTING.md has the current figure. **The 4 failures are the same 4 files** — `MT_Controller.cpp`,
`SIG_GUIGPManager.cpp` and the two ZORC files — and the one header failure is
the same one. No fitness value and no line of container ordering moved, which
is what "dead code" was supposed to mean.

### Dead but not deleted — the predictions, re-measured

| predicted | measured 2026-08-28 |
|---|---|
| `SIG_GlueJoint`, `SIG_CylindricalJoint` | **holds.** Both still parsed (`SIG_Robot.cpp:367,369`), still compiled by `SIGEL_RobotIO`, still listed by `SIG_RobotView.cpp:110`. No simulator reads either |
| `getFrictionValue`, `getElasticity`, `getVeloDamping` | **holds, exactly.** Zero call sites anywhere in the tree; only the definition and the declaration remain |
| `getYPlaneLevel` | survives, as predicted — `SIG_EnvironmentRenderer.cpp:58` and `SIG_EnvironmentView.cpp:148` |
| the six simulation parameters | **holds, with a correction.** `getAnalytical`, `getIntegrator`, `getMaximalIterations`, `getMaximalCollisionLoops`, `getSkipFrames` and `getSolveMode` lose their only *simulation* reader, but each is still read by `SIG_SimulationParameter.cpp` to fill its dialog. They are now exactly as dead as `getMaximalError` and `getMaximalSOLIDIterations` already were: parsed, displayed, editable, simulated by nothing |

**Three things the prediction missed, found by review of the change:**

| now dead | where |
|---|---|
| `SIG_Robot::prepareDynaMo` and `SIG_Link::transformToDynaMo` | `SIG_Robot.cpp:274-282` and `SIG_Link.cpp:200-206` — **16 lines of definition, 23 with the declarations and their doxygen blocks.** An earlier draft said "~45", which was a guess; this project's rule is that only measured figures go in a planning document. Their only callers are the three surviving `case DynaMo:` arms below |
| three `case DynaMo:` arms | `SIG_GPFitnessTrainer.cpp:52`, `sigel_slave.cpp:252`, `SIG_AllIndividualsView.cpp:311`. Each transforms the robot for a simulation that now always throws. Left because the `SimulationLibrary` enum has to survive — the parser, the GUI and four other switches name it |
| `SIG_Simulation::slotDynamoMessage`, `stopSimulation`, and the only throw of `SIG_SimulationCannotSolveException` | see "One `moc` target" above |

### Follow-up this change deliberately did not take

1. **The GUI can still author an experiment that now aborts.**
   **Four surviving surfaces**: `SIG_SimulationParameter.cpp:121` calls
   `setSimulationLibrary(DynaMo)`; `:224-225` reads the value back to re-check
   that button; `SIG_SimulationParameterBase.ui:143` offers "Dynamo  (not
   recommended)" and `:566-568` is a whole tab titled `DynaMo`; and
   `SIG_EnvironmentBase.ui:916-918` is a second such tab. Removing only the
   radio button leaves two dead tabs and a read-back for a value nothing can
   set.

   **The throw has a second consequence.**
   `SIG_SimulationVisualisationWidget.cpp:376-381` does
   `delete visualisation;` and then assigns the result of a constructor that
   now throws — so `visualisation` keeps a freed pointer. Fourteen sites in that
   widget test `if (visualisation)` and then dereference it, so the guard
   passes and each is a use-after-free; `renderRecorder` leaks with it. The
   path pre-existed — Dynamo's own constructor could throw — but this change
   turns a conditional hazard into a certain one for every robot.
   **THIS SAID IT WAS "unreachable today only because `SIGEL_SlaveGUI` does
   not compile". THAT EXPIRED.** `libSIGEL_SlaveGUI.a` builds and
   `build-fast/sigel_slave` links it, and the bare `delete` is still there.
   The fix is still `visualisation = nullptr;` between the delete and the new.
   Tracked as open item 5 in PORTING.md section 9.
2. **`SIG_SimulationQueries.cpp`: all seven non-self includes are dead.**
   Verified by compiling a translation unit
   holding only the class's own header and the empty constructor, under
   `check.sh`'s full flags: it passes. `<qdatetime.h>` and `SIGEL_Tools/SIG_IO.h`
   are dead too. This change removed only the three that named deleted files.
3. **Extract the maths into a small local header, and drop `libdynalib.a`.**
   *Decided 2026-08-28, deliberately not now.* The archive survives at 14
   objects — ~3,000 lines of physics — for **one line**: `DL_matrix::invert`
   reports a singular matrix through the physics engine's global system object
   (`matrix.cpp:233`), and that call plus the `DL_geo` vtable pulls in ten more
   translation units.

   The API is tiny: `DL_vector` is 18 methods in 296 lines, `DL_matrix` 81
   lines, and the whole vocabulary is get/set a component, add and subtract in
   place, scale, negate, norm, normalise, inner and cross product, multiply,
   invert. A 3-vector and a 3×3 matrix.

   **Pragmatic and minimal: a local header, not a dependency.** Boost's real
   candidate would be QVM rather than uBLAS, and Eigen would be the better
   library — but the port's direction is removing 2003 dependencies, not
   swapping them, and NEWMAT is already compiled and linked and used in 13
   files, so the tree already carries two matrix libraries. Replacing both with
   ~250 lines we own removes the last of Dynamo and adds nothing.

   **After Phase C, not before.** It is mechanical across ~1,100 references in
   93 files, and the payoff today is zero: the surviving objects link and
   nothing calls them. Doing it during the interface port would collide two
   large mechanical diffs in the same files.

   One thing to check when it happens: `matrix.cpp:233` is live code on a real
   error path, now calling into an engine with no running system behind it. It
   should be established whether that is a null dereference or something
   quieter.

### Two corrections this deletion forces on PORTING.md

**`SIG_DynaMoSimulationData` is not "the site that numbers the DynaMechs
bodies"** (§10 D1, §10 D2, `sigel_eval.cpp:46`). It was the **Dynamo** site.
Both backends walked links → joints → sensors → drives in that order, so the
orders the gate protects are unchanged and `dictorder-baseline.txt` is
untouched — but the file those sections cite is the wrong one, and it no longer
exists. The live site is `SIG_DynaMechsSimulationData.cpp`.

**V5's zero-hit `applyForce` probe is explained.** §7 records the `applyForce`
breakpoint arming and taking no hits "in a session where the MDH breakpoint
fired 18 times ... real but unexplained". `SIG_DynaDrive::applyForce` is called
from exactly one place, `SIG_DynaMoCommandInterface.cpp:54`, on the **Dynamo**
path. Every shipped experiment selects DynaMechs, so that breakpoint could
never fire. The same holds for the other open probe:
`SIG_DynaSensor::senseJoint1`/`senseJoint2` are called only from
`SIG_DynaMoSimulationQueries.cpp:45,47`. **Both remaining V5 probes were aimed
at code no shipped experiment executes**, and have to be re-pointed at
`SIG_DynaMechsSimulationQueries` / `SIG_DynaMechsCommandInterface` before they
can say anything about this port.

### Two repo-state findings from checking this work

**Nothing vendored was deleted.** That tree is untracked and is left exactly as
it extracts; 10,084 lines merely stopped being compiled. `diff -rq` against a
fresh extract shows five differences across the whole vendored tree: the four
recorded patches, plus the `.sigel-patched` stamp — ~~**and three stale `.rej` files**~~ *(this listed `cv97/JVector.h.rej`,
`dynamechs/dm/svd_linpack.cpp.rej` and `SOLID-2.0/include/3D/Basic.h.rej`, and
concluded the re-extract cycle was not as clean as PORTING.md describes)* —
**none remains; `find . -name "*.rej"` is empty, so the cycle is clean after
all.**

**The DynaMechs adapters are 2,077 lines**, not the 2,013 measured on
2026-08-20. They grew with Phase V5's MDH probe.

### Still not fixed, deliberately

The `exit(1)` a glue joint would reach under DynaMechs is at
**`SIG_DynaMechsSimulationData.cpp:424`**, not `:394` as recorded above — the
line moved with Phase V5's probe. It is still an `exit(1)` and should still
become a thrown exception. Left alone to keep this changeset single-purpose.

---

## What was given up

Measured 2026-08-20, before the decision, and unchanged by it.

**Two joint types become dead data.** `SIG_GlueJoint` (186 lines) is still
parsed and still editable, and no simulator reads it. `SIG_CylindricalJoint`
(219) was already dead — both backends dropped it to `default:`. No shipped
robot uses either: all joints in all 7 `.rrb` are rotational — 18, 12, 9, 6, 4,
3, 1.

**Four properties lose their only reader.** `SIG_Material::getFrictionValue`,
`getElasticity` and `SIG_Environment::getVeloDamping` were read by
`SIG_DynaSystem.cpp` alone and are now write-only: parsed, editable, read by
nothing. `getYPlaneLevel` survives for the two renderers. DynaMechs uses global
friction constants rather than a per-material-pair table, so **per-material
friction is the one genuine capability loss.**

**Six simulation parameters become dead**: `getAnalytical`, `getIntegrator`,
`getMaximalIterations`, `getMaximalCollisionLoops`, `getSkipFrames`,
`getSolveMode`. They join `getMaximalError` and `getMaximalSOLIDIterations`,
which were already parsed, displayed, editable and simulated by nothing.

**In full:** glue joints, closed kinematic loops, link-to-link self-collision,
mesh-accurate collision geometry, per-material-pair friction. **None of it is
used by any shipped robot or experiment.**

---

## Why Dynamo was the one to go

**It crashes on any robot with a joint sensor.** `SIG_DynaSystem.cpp:801` sets
a new sensor's joint pointer to zero and nothing ever assigns it. Then
`SIG_DynaSensor.cpp:28` does `switch (joint->joint->getJointType())`, and
`SIG_DynaMoSimulationQueries.cpp:46` dereferences `sensor.joint->joint`
directly. A null dereference on the first `SENSE` instruction.

Joint sensors per shipped robot: walker 18, insect 12, octopus 9, runner 6,
twoBases 1, hammer 0, shortHammer 0. **5 of 7 robots could not run on Dynamo at
all.**

Two more defects in the same files: `SIG_DynaSystem.cpp:335` inverts the
no-collision test, permitting collision precisely when the pair is on the
must-not-collide list; and `SIG_Simulation.cpp:120-149` throws on every Dynamo
diagnostic, including plain `"Warning:"` lines.

**The authors agreed.** `SIG_SimulationParameterBase.ui:143,154` labels the two
choices `"Dynamo  (not recommended)"` and `"DynaMechs   (preferred)"`, and
`SIG_Experiment.cpp:630` refuses to open the Robot Information dialog unless
DynaMechs is selected. All 14 shipped experiments carry `SIMULATIONLIBRARY 1`.
