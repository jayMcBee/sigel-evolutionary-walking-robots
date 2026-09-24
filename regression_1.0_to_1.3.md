# SIGEL 1.0 → 1.3 — a regression that predates the Qt port

**Status: open work since 2026-09-23.** Written 2026-08-23 and deferred until
the Qt 6 port was done; the port is done.

---

## Why this document exists

While validating the Qt 6 port I compared our numbers against the 14 published
experiments. That comparison is not valid. **Those files were produced in August
2001 by SIGEL 1.0. The source being ported is 1.3, dated 2003-04-30.** Every 1.0 → 1.3 change therefore
shows up as a "port failure" that has nothing to do with Qt.

Dates, verified:

| | |
|---|---|
| the 14 `.exp` files | 2001-08-10 … 2001-09-06 10:04 |
| `sigelSourceDistribution.1.0.tar.gz` rolled | 2001-09-06 11:17 — one hour later |
| KDE-SIGEL 1.3 `ChangeLog` starts | 2001-12-18 |
| 1.3 binaries | 2003-04-30 |

The port's own reference is 1.3 and the 1.3 binary. This document is what was
found along the way.

---

## The regression

`SIG_DynaMechsSimulationQueries::sense()`, joint-sensor branch. 1.3 has:

```cpp
scaledState  = (q - minPos);
scaledState *= 360.0 / (2.0*3.14159265);   // numerator now degrees
scaledState /= posRange;                   // denominator still radians
```

`q`, `minPos` and `posRange` are all radians. `getMechsMinPos`/`getMechsMaxPos`
are built in radians at `SIG_Joint.cpp, calculateMDH` as `2*pi - (deg/360)*2*pi`, because
`dmRevoluteLink` wants radians. So `scaledState`, which must be a 0..1 fraction
of the joint's travel, comes out **57.3 times too large**.

SIGEL 1.0, `sigel/src/SIGEL_Simulation/SIG_DynaMechsSimulationQueries.cpp, sense`:

```cpp
double scaledState = (q - minPos) / posRange;
```

The constant `360.0/(2.0*3.14159265)` appears **nowhere** in 1.0's source. The
multiply arrived with the pitch/roll and contact sensor branches — the 1.3
source's own comment reads *"Changed to handle different types of sensors (jb,
12/2001)"* — and was evidently copied from the `tPitchRollSensor` pattern, where
converting to degrees is correct because that branch then divides by 180. The
file grew from 251 lines to 479 in the same change.

### It is genuinely in the shipped 1.3 binary

`vendor/kdesigel/sigel_slave` on the x86 box (2003-04-30, unstripped i386,
md5 `5e20d30cc43b4aaea63b729d3627e446`) at
`0x080b1f08`–`0x080b1f3a`: `fld [ebp-0xc8]` / `fsub [ebp-0xd8]` for
`q - minPos`, then `fld QWORD PTR ds:0x81ec320` / `fmulp`, then
`fdiv [ebp-0xe8]` for `posRange`. The constant decodes to **57.29577957855229**
— the folded value of `360.0/(2.0*3.14159265)`, distinguishable from exact
`180/pi` (57.29577951308232) because the source truncates pi. One occurrence;
the exact value has none. The oracle confirmed all of this again on
2026-09-23, in raw `objdump` output.

So 1.3 really did behave this way. It simply postdates the published record.

### What it does

Measured on `twoBases`, whose hinge is limited to ±85°: `posRange` = 2.967 rad
(170°), `q` ∈ [2.73, 3.77], and the correct `scaledState` ∈ [0.362, 0.712]. With
the multiply it is 20.7–40.8, so `registerValue` reaches 161–322 and wraps
modulo 8 in `SIG_Register::makeValid`. Over 6,650 `SENSE` calls the register
histogram is `{0: 5918, 1: 713, -1: 19}`.

**Every joint sensor reports a sawtooth of about 57 cycles across the joint's
travel instead of the joint angle.** Any evolved program that senses is flying
blind.

### What removing it does

Restoring 1.0's line, measured with `checks/replicate.sh` over the 14
published experiments:

| | with the 1.3 multiply | with 1.0's line |
|---|---|---|
| distinct experiments within 10% | 7 of 13 | **11 of 13** |
| `runnerNiceWalkingFitness`, per individual | — | **100 of 100 exact** |
| `twoBasesHighMutationRate` | 0.050 | 1.016 |
| `twoBasesHardlyReducedIS` | 0.094 | 1.007 |
| `twoBasesSimpleFitness2` | 0.088 | 0.989 |

Nothing that already passed changed. The failures correlate perfectly with a
3-bit register width — 6 of 6 fail, 8 of 8 pass — because at 3 bits the wrap
destroys the reading entirely.

**Two cases stayed outside 10% even with 1.0's line.** Both experiments were
later removed from the repo by decision, so they are closed and not
investigated further:
`twoBasesHighCrossOverRate` 0.819 and `twoBasesReducedInstructionSet` 0.721.
Both stable and crash-free when run serially, so they are a deterministic
difference, not noise. Ranked by comment-stripped 1.0 → 1.3 delta on the
evaluation path, the places that were to be looked at:

| file | changed lines | what changed |
|---|---|---|
| `SIG_DynaMechsCommandInterface.cpp` | 116 | the entire `tServoSimpleMode` block is new in 1.3; `SIG_Drive` had no mode at all in 1.0 |
| `SIG_DynaMechsSimulationData.cpp` | 83 | pitch/roll and contact sensors added |
| `SIG_Robot.cpp` | 46 | |
| `SIG_Joint.cpp` | 23 | |
| `SIG_GPSimpleFitnessFunction.cpp` | 19 | |
| `SIG_Simulation.cpp` | 16 | `prematureTermination()` added — a no-op here |

`SIG_Interpreter`, `SIG_Register`, `SIG_ProgramLine` and
`SIG_GPNiceWalkingFitnessFunction` are functionally identical 1.0 → 1.3.

### The data today

The measurements above used all 14 published experiments. **The repo now
holds 7 of them**, in `experiments/`: `hammer`, `insect`, `octopus`, `runner`,
`shortHammer`, `twoBases` and `walker`. Each is its robot's
`…NiceWalkingFitness` file, except `twoBases`, which is
`twoBasesHardlyReducedIS`. Items 40, 44 and 45 in `PORTING.md`, "Finished to-do
items", removed the other seven: five `twoBases` variants,
`octopusSimpleFitness` and `runnerSimpleFitness`. Both cases that stayed outside
10% above were among them, so they are closed. Item 46 renamed the kept seven. The originals are on
sourceforge; see "Reference material" in `PORTING.md`. The oracle's machine
keeps all 14 as downloaded, at `/home/debian/sigel-shipped-original-2026-09-19/`.

Before the joint-sensor fix, with the 1.3 multiply, `checks/replicate.sh`, 2026-09-23:

| experiment | 2001 best | ours | best | match |
|---|---|---|---|---|
| hammer | 0.45972 | 0.45668 | 0.993 | 10/100 |
| insect | 0.63896 | 0.61079 | 0.956 | 5/100 |
| octopus | 0.52013 | 0.51471 | 0.990 | 38/100 |
| runner | 0.91951 | 0.083121 | **0.090** | 26/100 |
| shortHammer | 0.49015 | 0.49085 | 1.001 | 30/100 |
| twoBases | 0.56965 | 0.053302 | **0.094** | 1/100 |
| walker | 0.27548 | 0.25485 | 0.925 | 12/100 |

5 of 7 within 10% on best-of-population. The two failures are `runner` and
`twoBases`, the two that 1.0's line restores in the table above.

**The joint-sensor fix is applied, 2026-09-23:** `sense` has 1.0's
`(q - minPos) / posRange`.
The same run after it:

| experiment | 2001 best | ours | best | match |
|---|---|---|---|---|
| hammer | 0.45972 | 0.45668 | 0.993 | 10/100 |
| insect | 0.63896 | 0.61079 | 0.956 | 5/100 |
| octopus | 0.52013 | 0.51471 | 0.990 | 38/100 |
| runner | 0.91951 | 0.91951 | **1.000** | **100/100** |
| shortHammer | 0.49015 | 0.49085 | 1.001 | 30/100 |
| twoBases | 0.56965 | 0.57356 | **1.007** | 16/100 |
| walker | 0.27548 | 0.25485 | 0.925 | 12/100 |

Then 7 of 7 within 10%. Only `runner` and `twoBases` moved. No insect
program holds a `SENSE` instruction; the octopus population holds one and
walker's holds 11 `SENSE` lines, and neither best nor match moved. The next
section has the data after the second fix.

### The top of the register range, fixed 2026-09-23

Every sensor branch of `sense` maps its reading, 0 to 1, onto the register as
`int(scaledState * 2^n - 2^(n-1))`. A reading of exactly 1 gives `2^(n-1)`, one
past the register's top, and `SIG_Register::makeValid` wraps it to the bottom.
So a joint pressed against its max stop read as its min, and a contact sensor
read the same value with and without contact. 1.0 had this too. `sense` now
caps the value at `SIG_Register::getMaxValue`. This holds for register widths
up to 31. At 32, the default width for a new experiment, the cast to `int` in `sense`
and `makeValid` itself both overflow; that is a separate bug.

The same run after this fix:

| experiment | 2001 best | ours | best | match |
|---|---|---|---|---|
| hammer | 0.45972 | 0.45668 | 0.993 | 10/100 |
| insect | 0.63896 | 0.61079 | 0.956 | 5/100 |
| octopus | 0.52013 | 0.51471 | 0.990 | 38/100 |
| runner | 0.91951 | 0.12227 | **0.133** | **37/100** |
| shortHammer | 0.49015 | 0.49085 | 1.001 | 30/100 |
| twoBases | 0.56965 | 0.57356 | 1.007 | **13/100** |
| walker | 0.27548 | 0.25485 | 0.925 | 12/100 |

6 of 7 within 10%. The runner programs were evolved in 2001 with the wrap, and
they depend on it: with correct readings they no longer walk as they did.
`twoBases` keeps its best, and 3 fewer of its individuals match (16 to 13).
The other five do not move. The fix causes this change; it is not a defect.
Done 2026-09-24: `experiments/runner.exp` is a new population evolved under
the fixed sensors. With it, 7 of 7 within 10%; runner 1.000, 187 of 250
matching, and the other 63 were left unscored by the run.

---

## Measuring any of this

Fitness is a **chaotic** metric. A 1-ULP change to the robot's start height
moves an individual's fitness by 45% and best-of-100 by up to 18%. Use it only
in aggregate, and never to compare across machines.

`checks/replicate.sh` reports two numbers. `best` compares best-of-population and is
loose — the more individuals share the recorded best, the more chances `max()`
has to hit it, and the count of distinct programs carrying that value tracks
which experiments "pass" (`runnerNiceWalkingFitness` 45, `twoBasesSimpleFitness1`
2). `match` counts individuals reproducing their own value to 0.1% and is far
sharper, but the stored per-individual `FITNESS` fields are partly inherited
from parents rather than measured, so a low count is not by itself a defect.

Two of the 14 files are byte-identical — `twoBasesSimpleFitness1` and
`twoBasesHighCrossOverRate`, md5 `35bcdb3a…` — so there are **13 distinct
experiments**. All five `twoBases` experiments embed an identical 280-token
robot stream, so nothing about the robot distinguishes pass from fail.

---

## Model-validation wishlist

From the `sigel-x86` session, which authored a new robot model against the 2003
binary. Each item cost it hours. **Not part of the port and not part of the
regression work** — recorded so it is not lost.

1. **Reject a mesh with non-positive volume at load, naming the file.** Inverted
   face winding gives negative volume, hence negative mass and inertia. The
   simulation runs, produces NaN link positions and says nothing. The volume
   integral is already computed for the inertia tensor, so the check is nearly
   free.
2. **Report axis-to-nearest-vertex distance per joint at load.** Every shipped
   model places each joint axis exactly on a knife edge of both meshes it
   connects (0.000–0.001). Put the axis in the middle of a flat face and the
   links interpenetrate by 0.5·sin(theta) per degree and the robot silently
   jams. The rule is invisible in the file format and undocumented.
3. **Per-evaluation summary of joint travel** — min/max angle reached per joint,
   and net displacement of the body. A robot whose joints never leave their
   start angle is almost always a modelling error, and a jammed robot is
   currently indistinguishable from a working one from outside.
4. **Bound per-sample displacement in the fitness guard.**
   `SIG_GPFitnessFunction::isValid` rejects only Inf and NaN, so a finite but
   enormous single-sample jump is accepted and accumulated. Two different
   fitness functions were defeated identically by individuals scoring 268 and
   292. The functions share the guard, so switching function does not help.
5. **Label `TIMETOSIMULATE`'s units, or log the resolved duration at run
   start.** The four fields are hours/minutes/seconds/ms; the GUI labels them,
   the file does not. Writing 5 into the minutes field intending seconds gave
   300 s per individual and an 87% timeout rate that looked like a physics bug.
6. **On timeout, report the simulated time reached.** `TIMEOUTMINUTES` is whole
   minutes, 0 disables it, and on expiry the slave is killed returning nothing.
   That one number separates "too slow" from "diverged and grinding".
7. **Report joint-limit stability groups at load** — `timestep*sqrt(K/I)` and
   `timestep*damper/I` per joint. The shipped models sit in a narrow band;
   `JOINTLIMITSK_SPRING` 25000 inherited from a one-joint robot onto a
   three-joint chain put limit-spring torque above half full drive torque, with
   no warning.
8. **Report `maximalforce/(mass*g*half-length)` per drive at load.** Across all
   seven shipped models it sits between 0.60 and 0.92, spanning 1.2 to 49 mass
   units. Clearly deliberate, entirely undocumented, and a new model can be
   wildly mis-powered with no indication.
9. **Record the source path and a checksum beside geometry compiled into the
   `.exp`, and warn when the file on disk no longer matches.** Importing a robot
   copies the meshes into the experiment; editing the `.wrl` afterwards changes
   nothing and nothing says so.

The common thread: SIGEL validates almost nothing at load and reports almost
nothing per evaluation. A geometrically impossible model, an individual that
never moves, and a fitness value that violates conservation of energy all pass
in silence.

Same session's measurement advice, from experience: net displacement and path
length taken from exported POV-Ray frames caught three problems that fitness
hid. For a cross-architecture check, body position over the first few hundred
steps is far more informative than any fitness number.
