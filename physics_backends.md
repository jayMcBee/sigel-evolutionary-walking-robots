# SIGEL physics backends — Dynamo or DynaMechs?

**Status: analysis only. Nothing decided, nothing deleted.**
Written 2026-08-20 during the Qt 6 port. Independent of that port — but the
decision changes how much code the port has to carry.

---

## The question

SIGEL ships two physics engines and picks one at run time:

- **Dynamo** — "Dynamic Motion library", Bart Barenbrug, TU Eindhoven, 1996–1999.
  Constraint-based. LGPL. Vendored at `x/supportingLibs/supportingLibs/Dynamo/`.
- **DynaMechs** — Scott McMillan. Articulated-body (Featherstone).
  Vendored at `x/supportingLibs/supportingLibs/dynamechs/`.

Both implement the same three interfaces — `SIG_SimulationData`,
`SIG_SimulationQueries`, `SIG_CommandInterface` — and `SIG_Simulation.cpp`
switches between them.

Do we keep both?

---

## Recommendation

**Delete the Dynamo physics code. Keep DynaMechs. Keep Dynamo's maths headers.**

Three reasons, in order of weight:

1. **The Dynamo path crashes on most of the shipped robots.**
2. The original authors marked it "not recommended" in the GUI.
3. All 12 shipped experiments select DynaMechs.

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
- All 12 experiments would point at a backend that crashes on 5 of 7 robots.
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
