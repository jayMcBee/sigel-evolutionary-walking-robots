# SIGEL physics backends — Dynamo removed, DynaMechs kept

**DONE.** Analysed 2026-08-20, executed 2026-08-28, independently of the Qt 6
port. SIGEL shipped two physics engines and chose one at run time. Dynamo
crashed on most shipped robots, its own authors labelled it "not recommended",
and all 14 shipped experiments selected DynaMechs. It is gone.

**Read "What was actually done" and stop.** Everything from "Why the Dynamo path
is not usable" onward is the frozen 2026-08-20 case for the decision, kept so it
can be re-read, corrected in place where execution proved it wrong.

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

**But "loud" is only true of `sigel_eval`, and the first draft of this entry
and of the code comment both had the reason backwards.** The throw clears all
six fitness functions, which construct `SIG_Simulation` outside their own `try`
— but one frame further out `sigel_slave.cpp:361-367` wraps `evalFitness()` in
`catch (SIG_Exception &) { fitnessValue = 0; }`. So under PVM the exception is
swallowed and the individual is scored 0.0 as though it had been evaluated,
which is precisely the failure `SIG_GPSimpleRecorder.cpp:42` documents. That
makes the printed line the only evidence that reaches anyone, and it is why the
message goes to `std::cerr` rather than the buffered `SIG_IO::cerr` — a reason
this change got right by accident and now states correctly. Hardening that
swallow is a pre-existing defect and out of scope here.

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

**Three build files.** Both `Makefile.am` in `SIGEL_Simulation` lose the 39
filenames that no longer exist, and `SIGELCommon.dsp` — the 2003 Visual Studio
project — loses 27 `Source File` blocks: the 26 files plus
`moc_SIG_DynaSystem.cpp` and its two custom-build rules.

### Kept, and why

**The maths library, and more of Dynamo than this document predicted.** The
prediction was `Cpp/{pointvector,matrix,list}.cpp`. That is wrong, and the
measurement is the useful part of this entry:

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

**SOLID is still built and still linked.** This document claimed SOLID would go
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
| `check.sh` | **105 pass, 4 fail**, 322 warnings |

`check.sh` was 118 pass, 4 fail, 338 warnings. The pass count falls by exactly
13 because 13 fewer `.cpp` exist, and "headers standalone" by exactly 13 for the
same reason. **The 4 failures are the same 4 files** — `MT_Controller.cpp`,
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
| `SIG_Robot::prepareDynaMo` and `SIG_Link::transformToDynaMo` | `SIG_Robot.cpp:274`, `SIG_Link.cpp:200` (~45 lines). Their only callers are the three surviving `case DynaMo:` arms below |
| three `case DynaMo:` arms | `SIG_GPFitnessTrainer.cpp:52`, `sigel_slave.cpp:252`, `SIG_AllIndividualsView.cpp:311`. Each transforms the robot for a simulation that now always throws. Left because the `SimulationLibrary` enum has to survive — the parser, the GUI and four other switches name it |
| `SIG_Simulation::slotDynamoMessage`, `stopSimulation`, and the only throw of `SIG_SimulationCannotSolveException` | see "One `moc` target" above |

### Follow-up this change deliberately did not take

1. ~~**`libsolid.a`.**~~ **DONE** — see "SOLID and qhull went too" above. qhull
   went with it, because it existed only to give SOLID its convex hulls.
2. **The GUI can still author an experiment that now aborts.**
   `SIG_SimulationParameter.cpp:121` calls `setSimulationLibrary(DynaMo)` and
   `SIG_SimulationParameterBase.ui:143` still offers "Dynamo  (not
   recommended)". Nothing is broken today — Phase C has not started — but the
   radio button has to go with the backend.
3. **`SIG_SimulationQueries.cpp` has four more dead includes** — `matrix.h`,
   `pointvector.h`, `NaN.h` and `SIG_SimulationCannotSolveException.h`. The
   file is a licence header, seven includes and an empty constructor; it
   compiles clean without any of them. This change removed only the three that
   named deleted files.
4. ~~**`future_refactorings.md` cites deleted code**~~ — corrected 2026-08-28.

5. **Extract the maths into a small local header, and drop `libdynalib.a`.**
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

### Corrections to the frozen analysis below

The analysis from 2026-08-20 is kept verbatim so the decision can be re-read.
Five of its claims did not survive execution, and one of its section headings
is wrong; all six are measured, and none of them changes the decision.

| claim below | measured 2026-08-28 |
|---|---|
| "**~21,100 lines of Dynamo deleted**", and the "Deleted" heading above covering the vendored `.cpp` | **Nothing vendored was deleted.** The tarball tree is untracked and is left exactly as it extracts; 10,084 lines merely stopped being compiled. `diff -rq` against a fresh extract shows one difference, the recorded `containerlist.h` patch |
| "**SOLID deleted entirely**, ~4,800 lines" | **not done.** `libsolid.a` is still built and still linked. Its 15 API references really were all in deleted files, so it is dead weight — see the follow-up list above |
| "**8 old-style exception specifications gone**" | **this deletion gained none.** All 8 were on `SIG_DynaSystem`, and the port had already removed every one of them before 2026-08-28. The statement was true of the 2003 tarball, not of `HEAD` |
| "`SIG_DynaSystem.cpp:268` … Double free" | **already fixed** by D13 before the deletion — the line read `delete dynaDrives[k];` with the 2003 behaviour in a comment. It was not a live defect being removed |
| "the DynaMechs adapters we keep total **2,013** lines" | **2,077.** They grew with Phase V5's MDH probe. The Dynamo side of the comparison was re-measured for this entry and the DynaMechs side was not |
| `SIG_DynaSystem.cpp:800`, `SIG_DynaMoSimulationQueries.cpp:45` | off by one — the null assignment was at `:801` and the dereference of `sensor.joint->joint` at `:46`. The defects are real and were read correctly; only the line numbers drifted |

### Still not fixed, deliberately

The `exit(1)` a glue joint would reach under DynaMechs is at
**`SIG_DynaMechsSimulationData.cpp:424`**, not `:394` as recorded above — the
line moved with Phase V5's probe. It is still an `exit(1)` and should still
become a thrown exception. Left alone to keep this changeset single-purpose.

---

## Why the Dynamo path is not usable

### It crashes on any robot with a joint sensor

`SIG_DynaSystem.cpp:800` creates a sensor and sets its joint pointer to zero:

```cpp
dynaSensors[sz]->joint = 0;
```

Nothing ever assigns it. Then `SIG_DynaSensor.cpp:28` does:

```cpp
switch (joint->joint->getJointType())
```

and `SIG_DynaMoSimulationQueries.cpp:45` dereferences `sensor.joint->joint`
directly. **Null pointer dereference on the first SENSE instruction.**

Joint sensors per shipped robot:

| robot | joint sensors |
|---|---|
| walker | 18 |
| insect | 12 |
| octopus | 9 |
| runner | 6 |
| twoBases | 1 |
| hammer | 0 |
| shortHammer | 0 |

**5 of 7 robots cannot run on Dynamo at all.**

### Other defects in the same files

- `SIG_DynaSystem.cpp:268` — `clearAllDynamics` deletes `dynaJoints[k]` inside
  the loop over *drives*. The two arrays grow independently. Double free.
- `SIG_DynaSystem.cpp:335` — the no-collision test is inverted:
  `if (getNoCollides().find(other) != -1)` permits collision precisely when the
  pair is on the must-not-collide list.
- `SIG_Simulation.cpp:120-149` — every Dynamo diagnostic, including plain
  `"Warning:"` lines, sets `stopSimulation = true` and throws.

### The authors' own verdict

`ui/SIGEL_MasterUI/SIG_SimulationParameterBase.ui:143,154`:

```
"Dynamo  (not recommended)"
"DynaMechs   (preferred)"
```

`SIG_Experiment.cpp:630` refuses to open the Robot Information dialog unless
DynaMechs is selected.

---

## WHAT WE THROW AWAY ON THE SIGEL SIDE

This is the part that matters. Deleting a vendored library is easy; deleting our
own code is the decision.

### 1. Thirteen file pairs — 3,247 lines, verified

All in `SIGEL_Simulation`. These exist only to drive Dynamo:

| file pair | lines |
|---|---|
| `SIG_DynaSystem` | **1,092** |
| `SIG_DynaMoSimulationQueries` | 329 |
| `SIG_DynaLink` | 282 |
| `SIG_RotationalController` | 247 |
| `SIG_DynaDrive` | 183 |
| `SIG_DynaSensor` | 175 |
| `SIG_TranslationalController` | 172 |
| `SIG_DynaCallbacks` | 163 |
| `SIG_DynaMoSimulationData` | 145 |
| `SIG_DynaMoCommandInterface` | 136 |
| `SIG_Dyna` | 122 |
| `SIG_DynaSystemWrongNumberException` | 102 |
| `SIG_DynaJoint` | 99 |
| **total** | **3,247** |

For comparison, the DynaMechs adapters we keep total 2,013 lines.

### 2. Two robot joint types become dead data

| class | lines | status after |
|---|---|---|
| `SIG_GlueJoint` | 186 | still parsed, still editable, no simulator reads it |
| `SIG_CylindricalJoint` | 219 | already dead — both backends drop it to `default:` |

**No shipped robot uses either.** All joints in all 7 `.rrb` models are
rotational: 18, 12, 9, 6, 4, 3, 1 — zero glue, zero cylindrical, zero
translational.

The robot compiler still accepts glue joints (`SIG_RobotCompiler.cpp:535`) and
would reach DynaMechs' `exit(1)` at
`SIG_DynaMechsSimulationData.cpp:394`. **That `exit(1)` should become a thrown
exception first, whatever we decide.**

### 3. Four material and environment properties lose their only reader

| property | read today by |
|---|---|
| `SIG_Material::getFrictionValue` | `SIG_DynaSystem.cpp` only |
| `SIG_Material::getElasticity` | `SIG_DynaSystem.cpp` only |
| `SIG_Environment::getVeloDamping` | `SIG_DynaSystem.cpp` only |
| `SIG_Environment::getYPlaneLevel` | `SIG_DynaSystem.cpp` + the two renderers |

The first three become write-only: parsed from the robot file, editable in the
GUI, read by nothing. `getYPlaneLevel` survives for rendering.

DynaMechs uses global friction constants instead of a per-material-pair table,
so **per-material friction is the one genuine capability loss.**

### 4. Six simulation parameters become dead

`getAnalytical`, `getIntegrator`, `getMaximalIterations`,
`getMaximalCollisionLoops`, `getSkipFrames`, `getSolveMode`.

Two more — `getMaximalError`, `getMaximalSOLIDIterations` — are written by the
GUI and read by nobody **today**. They are already dead.

### 5. Summary of capability lost

Glue joints · closed kinematic loops · link-to-link self-collision ·
mesh-accurate collision geometry · per-material-pair friction.

**None of it is used by any shipped robot or experiment.**

---

## What we gain

- **3,247 lines of our own code deleted**, including the single largest file in
  `SIGEL_Simulation`.
- **~21,100 lines of Dynamo deleted** — only the maths headers stay.
- **SOLID deleted entirely**, ~4,800 lines. Verified: 15 references in exactly 3
  files, all Dynamo (`SIG_DynaSystem.cpp/.h`, `SIG_DynaLink.h`).
- **8 old-style exception specifications gone.** C++17 rejects these. This
  already bit the port: step A4 had to delete `SIG_Link.cpp`'s include of
  `SIG_DynaSystem.h` for exactly this reason. The DynaMechs adapters have zero.
- **One `moc` target gone** — `SIG_DynaSystem` is a `QObject`.

---

## What Dynamo does that DynaMechs cannot, and the reverse

| | Dynamo | DynaMechs |
|---|---|---|
| Joints | rotational, translational, **glue** | rotational, translational only |
| Topology | closed loops possible | tree only |
| Sensors | joint only — **and it crashes** | joint, **pitch/roll**, **contact** |
| Drives | force only | force **and positional servo** |
| Collision | SOLID, full mesh, link↔link and link↔ground | penalty springs, **ground only** |
| Ground | flat plane | **terrain heightfield** |
| Friction | **per material pair** | global constants |
| Integrators | Euler, DoubleEuler, RK2, RK4 | Euler, RK4, **RK45** |
| `getUsedForces()` | `return 0;` — never implemented | real |
| `getNumberOfTouchdowns()` | `return 0;` — never implemented | real |

Because of those last two, `StepperFitnessFunction` and `ForceFitnessFunction`
silently return zero under Dynamo.

---

## Why we cannot simply drop Dynamo

**Dynamo is also the maths library for the entire codebase.**

| use | references |
|---|---|
| `DL_vector` | 606 |
| `DL_Scalar` | 308 (it is `#define DL_Scalar double`) |
| `DL_matrix` | 138 |
| `DL_point` | 49 |
| **maths total** | **1,101 across 93 files** |
| physics total | 41 across 12 files |

The maths subset is self-contained and contains no physics:

```
Inc/{pointvector,matrix,scalar,boolean,list,NaN}.h
Cpp/{pointvector,matrix,list}.cpp          ~1,133 lines
```

Keep that. Delete the rest of `Dynamo/Src`.

One edit needed: `SIG_Environment.h:26` includes `<constraint.h>` only to reach
`DL_vector`. Change it to `<pointvector.h>`.

---

## If we dropped DynaMechs instead

Worse in every direction, and the survivor is the broken one.

- `SIG_Environment` **holds a `dmEnvironment` by value** (`SIG_Environment.h:29,447`).
  DynaMechs is in the core domain model, not just an adapter.
- `SIG_EnvironmentRenderer.cpp:129,507` renders terrain through it.
- All 14 experiments would point at a backend that crashes on 5 of 7 robots.
- 2,013 lines removed instead of 3,247 — **less code deleted, more capability
  lost.**

---

## Evidence quality

**Verified** — file contents, line numbers and all counts, measured with Python
and `command grep`. Note: plain `grep` in this environment wraps `ugrep -I`,
which silently skips Latin-1 and CRLF files; many SIGEL files are exactly that,
so plain `grep` counts are not trustworthy.

**Inferred, not observed** — the null dereference and the double free are read
from the source. Nothing was built or run. The 2003 binary does run on
`sigel-x86`, so both could be confirmed there if the decision needs it.
