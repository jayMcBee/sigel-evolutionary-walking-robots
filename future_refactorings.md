# C++ modernization — working list

**Scope: C++ language level (§1–§4), plus the deferred renames, the German
translation and the version bump below.** Independent of the Qt port; do not
combine commits across the two.

**Protocol:** one commit per item. Jan reviews and approves each. Items 1–2 are
purely mechanical; everything from 3 on gets reviewed and discussed before it
lands.

Status: `[ ]` open · `[~]` in review · `[x]` done · `[!]` blocked

Counts measured 2026-08-19 against the extracted 1.3 tree. Paths are relative to
`x/kdesigelSources.1.3/kdesigel/kdesigel/`.

---

## 1 · Make it legal C++

Constructs the language removed. Not style — a current compiler rejects them.
Review is reading the diff.

- [ ] **1. Pre-standard headers** — 7 sites
  `<iostream.h>` → `<iostream>`, `<vector.h>` → `<vector>`
  ```
  src/SIGEL_GP/SIG_GPIndividual.cpp:25
  src/MT_GPSystem/MT_GPManager.cpp:6
  src/MT_GPSystem/MT_Randomizer.cpp:6
  src/SIGEL_RobotIO/SIG_RobotScanner.cpp:26
  include/SIGEL_Simulation/SIG_SimulationQueries.h:34
  include/SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h:34
  (was SIG_DynaMoSimulationQueries.h:34 -- file deleted 2026-08-28,
   physics_backends.md; re-measure this count)
  ```
  *This said it "also retires `shim/`".* **It does not**: `Makefile:208` puts
`-I$(SHIM)` on **cv97**'s include path, and 32 files across the vendored tree
need `<iostream.h>` or `<fstream.h>`. Fixing SIGEL's own sites cannot retire it.

- [ ] **2. `register` keyword** — 3 sites, all `src/SIGEL_Visualisation/SIG_EnvironmentRenderer.cpp`
  lines 119, 500, 602. Removed in C++17. Delete the keyword.

- [ ] **3. Loop-variable scope leaks** — 13 files in `src/MT_GPSystem/`
  `for (int i=…){…}` then `i` used after the loop. Hoist the declaration.
  Files: `MT_FitnessTrainer`, `MT_GPManager`, `MT_Interpreter`, `MT_Population`,
  `MT_Program`, `MT_Randomizer`, `MT_Search`, `MT_Statistics`,
  `MT_StatisticsElement`, `MT_Tournament`, `MT_TournamentManager`,
  `MT_TranslatedIndividual`, `MT_Trainingset`
  *Confined to one module — the VC6-era half of the tree.*

- [x] **4. Dynamic exception specifications** — **DONE BY THE PORT, Phase A7.**
  Zero remain: `) throw (` matches nothing tree-wide and all seven named headers
  are clean. Three sites keep them only as `// NOTE: in 2003 this carried…`
  comments. Listed below as it stood — 6 file pairs (.h + .cpp)
  ```
  SIGEL_Simulation   SIG_Simulation, SIG_Register, SIG_Recorder
                     (SIG_DynaSystem was here; deleted 2026-08-28)
  SIGEL_GP           SIG_GPSimpleRecorder, SIG_GPFullDataRecorder
  SIGEL_Visualisation SIG_RenderRecorder, SIG_SimulationVisualisation
  ```
  **Not purely cosmetic.** Empty `throw()` means "terminate if anything
  escapes" → convert those to `noexcept`. Non-empty `throw(X, Y)` → delete.
  Removing a non-empty spec lets exceptions propagate that previously
  terminated. Discuss per site.

**After 1–4 the code is valid modern C++.** Stopping here is a real result.

---

## 2 · Let the compiler hunt bugs

Additive only — nothing changes at runtime. The value is that failures are
*discoveries*: code that has been wrong since 2003.

- [ ] **5. Add `override`** — 313 `virtual` sites, **one commit per module**

  | module | sites |
  |---|---|
  | SIGEL_RobotIO | 164 |
  | SIGEL_Robot | 40 |
  | SIGEL_GP | 32 |
  | SIGEL_Simulation | 28 |
  | MT_GPSystem | 17 |
  | SIGEL_CommonGUI | 12 |
  | MT_Control | 8 |
  | SIGEL_Visualisation | 7 |
  | MT_GUI | 6 |
  | SIGEL_MasterGUI | 5 |
  | SIGEL_Program, SIGEL_SlaveGUI | 3 |

  Highest-value item on this list. A method meant to override with a subtly
  wrong signature (missing `const`, `int` vs `long`) silently becomes a *new*
  function and the wrong one gets called. `override` turns each into a compile
  error. **Read every failure — do not fix mechanically.**
  Needs base headers to parse, so this is the first item requiring tooling
  rather than an editor.

- [ ] **6. `NULL` → `nullptr`** — 75 sites. Mechanical, one commit.

- [ ] **7. `explicit` on single-argument constructors** — per module.
  Every break is a place an implicit conversion was silently happening.

---

## 3 · Ownership — BLOCKED

- [!] **8. Smart pointers** — 554 `new` vs 127 `delete`

  **Blocked on: a runnable program and a reference trajectory.**

  This is the class of change where the correct and the broken version *both
  compile clean*. No compiler flag verifies it.

  Prerequisite, in order:
  1. one experiment running end to end
  2. fixed random seed
  3. fitness trajectory recorded to a file as the reference
  4. then convert one class at a time, trajectory must stay bit-identical

  Until that exists this is guesswork with nice syntax. Do not start.

---

## 4 · Program display

- [ ] **9. Syntax-highlight the program view** — the GUI builds and runs as of
  C9; the bar is now "after the port is validated", not "after it compiles"

  New function `programToHtml(const SIG_Program&, const SIG_LanguageParameters&)`
  returning a `QString`. `printToString()` stays as the ZORC serial format
  (`SIG_GPRemoteZORCFitnessFunction.cpp:69`).

  - `<pre>` wrapper, one `<span>` per token.
  - Colour opcodes by group: arithmetic `ADD SUB MUL DIV MOD MIN MAX`,
    data `COPY LOAD`, control `CMP JMP`, robot `MOVE SENSE DELAY`.
  - Register operands print as `R0`–`R7`, computed as `element % getMemorySize()`.
    `LOAD` operand 2 and `JMP` operand 1 print as literals.
  - Line number in a leading `<span>` per line.

  Feed it to `QTextBrowser::setHtml()` at `SIG_SimulationWidget.cpp:234`,
  `SIG_IndividualView.cpp:51`, `SIG_AllIndividualsView.cpp:394`.

---

## What is measured vs. proposed

- **Measured when written, and SOME HAVE DRIFTED.** A 2026-09-02 audit
  re-derived them: item 1 is 6 sites not 7 (`MT_GPManager.cpp:10`, not `:6`);
  item 2's `register` is at 125/505/607, not 119/500/602; item 5's "313
  `virtual` sites" does not match its own table, which sums to 322, and the
  per-module split has moved hard — `MT_GUI` 6 to 24 and `SIGEL_MasterGUI` 5 to
  38 — because Phase C's uic3-derived base classes declare `virtual` slots, so
  the one-commit-per-module plan needs re-measuring; item 6 is 63 `NULL` sites
  not 75; item 9's `SIG_AllIndividualsView.cpp:394` is `:448` and
  `SIG_SimulationWidget.cpp:234` is `:235`; the `tours` section cites two
  doxygen comments where there are three, at `:225`, `:259` and `:262`. Item 3
  still reproduces exactly. **Re-measure before acting on any of them.**
- **Proposed** (yours to approve): the three-section ordering, the per-module
  split of item 5, and item 8's blocked status.

## Left by the Qt port, deliberately — Phase D11, 2026-08-29

Both were converted rather than changed, because the port's rule is to move the
Qt API and nothing else.

Both are reached by the gates and neither is observable by them. `usedByLinks`
is appended on every one of the 7 `.rrb` loads and read nowhere in the tree.
`friction` is walked by `writeToFileTransfer` on every run, always empty.

- **`SIG_Material::FrictionValue` could be a value type.** It is two words —
  `SIG_Material *otherSide` and a `DL_Scalar` — held as `QList<FrictionValue *>`
  with a `new` per entry and a `qDeleteAll` in `~SIG_Material`. Values would
  delete both. D8 made exactly this move for `SIG_Register` and it removed a
  real leak; here the destructor already frees, so there is nothing to fix and
  the change would only be tidier.
- **`SIG_Body::usedByLinks` is dead.** `addUsingLink` appends to it from
  `SIG_RobotCompilerObjects.cpp:112` on every model load, and nothing in the
  tree — GUI included — ever reads it back. The member, the method and the one
  call could all go. Asked directly during D11, the answer was convert, not
  delete.

## Rename the two `SIG_GPExperiment` variants — after the Qt 6 port

Two files define `SIGEL_GP::SIG_GPExperiment` with different bodies, one per
binary: `SIG_GPExperiment.cpp` for `sigel`, `SIG_GPExperimentClean.cpp` for
`sigel_slave`. Their headers share the include guard
`SIGEL_GP_SIG_GPEXPERIMENT_H`. See PORTING.md §9.

The arrangement works and is deliberate, but nothing in the source says so.
It has already been misread once as an accidental duplicate, with deletion of
one of the pair proposed as the fix. The name is the whole problem: "Clean"
says nothing about `MT_Controller`, and the identical class name hides that
there are two.

**Do:** give the two classes distinct names — the master's keeps
`SIG_GPExperiment`, the slave's becomes something that says what it is, and the
headers get matching distinct guards. Then the compiler enforces what the build
files currently only imply.

**Not before the port is validated.** *This said `MT_Controller.cpp` "does not
compile yet", which was true before C6–C9.* It compiles, links into `sigel` and
into `guidrive`, and is driven by the `metagui` scenario — so the master half
CAN now be built and tested, and the bar is validation rather than compilation.

**Also fix while there:** our `Makefile` globs `src/<module>/*.cpp`, so it
compiles both variants into `libSIGEL_GP.a` where 2003 compiled them into
separate targets. *This said the unused object is harmless "only because the linker cannot
extract it — `SIG_GPExperiment.o` needs `MT_Controller`, which the build
excludes".* **The build no longer excludes it**: `MT_Controller` compiles and
links, and the Makefile now excludes exactly one file,
`WIN_SIG_GPRemoteZORCFitnessFunction.cpp`. Which variant the linker picks is
now settled deliberately — `guidrive` names `$(MASTER_OBJ)` and asserts it,
after linking the Clean variant produced a convincing false crash in C10.

## Rename `tours` to `tournaments` — after the Qt 6 port is complete and validated

`SIG_GPManager::tours` holds every `SIG_GPTournament` for one generation. The
name reads as travel, or as a shortening of nothing in particular. The type it
holds already says the word.

**Do:** rename the member to `tournaments`. It is private
(`SIG_GPManager.h:140`) and used in one file, so the change is contained. The
two doxygen comments that name it (`SIG_GPManager.h:213`, `:247`) go with it — and they are wrong twice over, since it is not a `QArray`
and has not been one for some time.

**Not before the port is validated.** *This said `SIG_GPManager` "cannot be
compiled today" because it reads `actExperiment.mtController`, a member of only
the master variant, which needed `MT_Controller`, which did not build.* All of
that builds now. The bar is validation, not compilation — but the reason to
wait is unchanged: this rename crosses the two `SIG_GPExperiment` variants.

Same reason as the `SIG_GPExperiment` rename above, and worth doing in the same
pass.

## Translate the German — after the Qt 6 port is complete and validated

SIGEL was written at Uni Dortmund and parts of it are in German, against the
English of the rest of the tree. This is 2003 upstream, not debt the port
created. Counts measured 2026-08-30 by grep over the extracted 1.3 tree.

**The umlauts are Latin-1 bytes, not UTF-8.** A UTF-8 grep misses them.

**Three phases, one commit per phase, in this order.**

- [ ] **10. Comments** — 118 lines in 46 files
  Includes 15 `NEU NEU NEU…` banner lines (`MT_Classifier.cpp`,
  `MT_Substitute.cpp`, `SIG_GPManager.cpp`) and 6 MSVC German-locale file
  headers (`Schnittstelle für die Klasse` / `Implementierung der Klasse` /
  `Konstruktion/Destruktion`, all five `MT_GUI` pairs plus `MT_Tournament.cpp`).
  Nothing to verify — nothing executes.

- [ ] **11. Strings** — **9** lines in 4 files
  `MT_GPSystem/MT_GPManager.cpp:277,332,333,341` ("beste Fitness vor
  Berechnung", "Sigel Fitness/Sieger", "Meta Vorhersage", "durch.Fitness"),
  `SIG_GPIndividual.cpp:384,385` ("Fitness (Elter 1)" / "(Elter 2)"),
  `SIGEL_GP/SIG_GPManager.cpp:687,1116` ("SIG_GPManager::run() wurde mehr als
  einmal aufgerufen!", the same string twice),
  `SIG_GPOperations.cpp:697` ("reproduction: Konnte kein neues Individuum
  erzeugen").

  *Corrected 2026-09-09. This said **11** lines and cited
  `SIG_GPManager.cpp:627,1053`. Both were wrong: the count is 9 in the working
  tree **and 9 in the pristine 1.3 tarball**, so 11 never matched anything; and
  those two line numbers point at unrelated code, while the strings they mean
  are at `:687` and `:1116`. Jan read `SIG_GPManager` on 2026-09-09, found the
  German, and reasonably concluded it was untracked — the entry covered it, but
  its citations pointed away from it.*

  **Why Phase 0 could not have caught these.** Phase 0 swept for bytes above
  127. Every string here is pure ASCII — German without umlauts is invisible to
  an encoding sweep. Any future check for German has to look for words, not
  bytes.

  **No compiler and no gate checks this phase.** All three persisted paths were
  checked and all three are safe:
  - `SIG_GPIndividual::writeToFile` writes history; `readFromFile` parses only
    `NAME='`, `POOLPOS=`, `FITNESS=`, `AGE=`, `PROGRAM BEGIN{`. History is never
    read back.
  - The `MT_GPManager` block sits after that file's own marker
    *"Additional information about the fitness computation ... is not loaded"*.
    Write-only.
  - The ZORC serial format (`printToString()`) carries no German.

- [ ] **12. Symbols** — 217 lines in 39 files, not user-visible
  Every miss is a compile error, so `./check.sh` verifies this phase in full.

  | symbol | where |
  |---|---|
  | `schlussJetzt` | `SIG_GPManager.h:123` + 5 uses |
  | `liesdas` (ctor param) | `SIG_Scanner.h:43`, `SIG_RobotScanner.h:48`, `SIG_UnstreamerScanner.h:34` + 3 `.cpp` |
  | `getRandomInstruktion`, `ProbInstruktion` | `MT_Randomizer.h:54,165` |
  | `T_Instruktion`, `T_Instruk` | `MT_TranslatedIndividual.h:38,74` |
  | `Instruktion` | `MT_Classifier.cpp:319` |
  | `set`/`getSelektionValue` | `MT_FitnessTrainer.h:109,115`, `MT_GPManager.h:68` |
  | `Varianz` | `MT_StatisticsElement.h:31` |
  | `winkel`, `verschiebung`, `schiebung`, `drehmatrix`, `hilf`, `stflorianhilf` | `IFunctions.h:38`, `IFunctions.cpp:321,380–397` |
  | `masse` | `SIG_Mirtich.h:80`, `.cpp:265,330` |
  | `dichte`, `konstante`, `anderes_material`, `rot`/`gruen`/`blau` | `SIG_RobotCompiler.cpp:167–236` |
  | `betrag`, `betraege`, `varianz`, `durchschnittProGelenk` | `SIG_GPForceFitnessFunction.cpp:105–122` |
  | `ausgabeTerrain` | `SIG_Environment.cpp:460–536` |
  | `zeiger` | `SIG_EnvironmentRenderer.cpp:447–455` |
  | `zahl` | `SIG_Geometry.cpp:53` |

  **The robot description grammar is entirely English** — `density`, `red`,
  `green`, `blue`, `friction`, `minimal_rot`. So the German names in
  `SIG_RobotCompiler.cpp` are locals holding the value of an English keyword and
  the target name is already written in the grammar. `rot` elsewhere
  (`getMinRot`, `rotMin`) is rotation, not the colour — leave it.

**Do not translate:** `Sigel.mak`, `sigel_slave.mak`, `manage_dyn_slave.mak`,
`Sigel.dsw`. MSVC-generated German, not built by this port. Delete them or leave
them; do not hand-edit generated files.

**Gate:** `./check.sh` after each phase, `./dictorder-dump.sh | diff -u
dictorder-baseline.txt -` empty, `fitness-check.sh` clean. Phase 10 cannot move
any of them, which is why it goes first.

**Not before validation.** `MT_GPManager`, `MT_Classifier` and `SIG_GPManager`
carry most of the German. *This said none of them compiles today*; all three do,
and all three link into `sigel`. Same bar as the two renames above.

## Set the version to 2.0 — the LAST step of the port

Do this only once the port is complete, running and validated against the 1.3
binary. It is the marker that the Qt 6 result is a different thing from what
2003 shipped, and it should be the final commit, not an early one.

**Where the version actually lives, measured — it is not where anyone would
look, and the tree disagrees with itself:**

| place | says | note |
|---|---|---|
| `kdesigel/configure.in:2` | `AM_INIT_AUTOMAKE(kdesigel,1.0)` | **the only real version declaration in the whole tree.** The 1.3 release still calls itself 1.0 here |
| `kdesigel/README:1` | `KDESIGEL v1.1 Readme File` | a third number, in the file a user reads first |
| `kdesigel/kdesigel.kdevprj:36` | `kdevprj_version=1.3` | **not SIGEL's version.** This is KDevelop's own project-file format version, which happens to also be 1.3. **Do not touch it** |
| the source | `SIG_InfoBox.cpp:60` | **the About box prints `Sigel v1.1`**, and `pixmaps/altLogo.png` carries a `Sigel v1.0` caption — 1.3 already ships that mismatch. BOTH have to move or 2.0 ships it again with new numbers. Found by C11c |

So "1.3" exists only in the tarball name and the directory name. Three files
carry three different numbers and none of them is 1.3.

**Do:** set `configure.in` to 2.0, and the README's heading with it. Then
decide whether the code should carry a version at all — today nothing prints
one, so `sigel --version` does not exist and a user cannot tell which build
they are running. Adding one is a small job and would be worth doing in the
same commit, but it is a new feature rather than a rename, so it is a separate
decision.

**Do not** change `kdevprj_version`. It describes the file's own format and
KDevelop reads it.

## Two Qt 6 noises the port keeps on purpose — found by C10, 2026-09-02

Both are 1.3 code preserved verbatim. Neither changes what the program does,
and neither should be "cleaned up" while the port is still being trusted
against the 1.3 binary.

**The wildcard disconnect.** `SIG_AllIndividualsView::slotEvolutionNotRunning`
begins each branch with `QObject::disconnect( individualList->
listviewIndividuals, 0, 0, 0 )`, which is byte-identical to the 2003 source.
Qt 6 prints `QObject::disconnect: wildcard call disconnects from destroyed
signal of QTreeWidget::listviewIndividuals` on stderr each time an evolution
starts. The disconnect still does what it is meant to; Qt 2 simply did not
warn about the pattern.

**Do:** name the three signals being disconnected, which is what the code
below the call immediately reconnects anyway. **Do not** do it before the
`gui behaviour` gate has a run under an actual evolution — that path is the
one C10 could not exercise, so a change there is currently unguarded.

**The history block grows by one line per individual per save.**
`SIG_GPIndividual::readFromFile` takes everything between `HISTORY BEGIN{` and
`}HISTORY END` as a SINGLE string — `history.clear()` is commented out in both
versions — and `writeToFile` emits `history.join("\n")` followed by a fresh
`"\n      }HISTORY END;"`. The chunk already ends with the previous save's
indent, so each round trip adds one `      ` line to every individual: measured
here as 1 line becoming 2 becoming 3, 120 extra lines per cycle on
`twoBasesSimpleFitness2.exp`, and the code is identical in the 1.3 tarball.

**CONFIRMED ON THE 1.3 BINARY, so preserve it.** The oracle ran the same two
saves: the six-space line count goes 360 -> 480 -> 600 while `}HISTORY END;`
stays at 120, i.e. +840 bytes = 120 x 7 per save, one `"      \n"` per
individual. (The first save of a shipped file is +141 lines rather than +120,
because the writer also emits five keys the 2003 file lacks — `WITHHISTORY`,
`WITHTEXTURE`, `TEXTUREFILE`, `TEXALPHA`, `RESEVGEN`.)

**Do:** nothing. This is 1.3 behaviour and the port reproduces it exactly, which
is the correct outcome. It is recorded here only so that a future reader who
notices experiment files growing does not "fix" it and silently diverge from
1.3. If it is ever changed deliberately, that is a product decision and needs a
note in PORTING.md saying the port stopped matching 1.3 on purpose.

## Restore Qt 2's spin-box editing — C11a, 2026-09-02, NOT DOING IT (D28)

**D28 accepted this divergence rather than fixing it**, because it is reachable
only by typing a number outside a box's own range, the differing value is
visible in the box before anything is saved, and the real cost is owning a
custom widget forever rather than writing it. The current behaviour is pinned
in `guibehaviour-baseline.txt`. This section is the worked-out change, kept so
that revisiting the decision is a lookup and not a re-derivation.

**What differs.** Qt 2's `QIntValidator::validate` returned **Intermediate**
for an out-of-range number (`qvalidator.cpp:236`), so `QLineEdit` accepted
every digit; `QSpinBox::interpretText()` then mapped the whole text and
`QRangeControl::directSetValue` clamped it into range. Qt 6's returns
**Invalid** once the typed prefix passes the top, so the keystroke is refused
and the box keeps the truncated prefix. Measured on four fields on both
architectures: type 32001 into a [2..32000] box and 1.3 commits **32000**, the
port commits **3200**; type 100 into a [1..99] box and 1.3 commits **99**, the
port commits **10**. Both valid, different, and `putAllIntoExperiment()` writes
whichever the widget holds.

**The fix, measured rather than sketched.** This subclass — 33 lines, 28
without comments — reproduces all four 1.3 readings exactly — 32001→32000, 8001→8000, 24→23, 100→99 — with in-range
typing unchanged:

```cpp
class SIG_SpinBox : public QSpinBox {           // Qt 2's editing semantics
public:
    using QSpinBox::QSpinBox;
    QValidator::State validate( QString &input, int &pos ) const override {
        QString t = stripFix( input );
        if ( t.isEmpty() || t == "-" ) return QValidator::Intermediate;
        bool ok = false;
        const long long v = t.toLongLong( &ok );
        if ( !ok ) return QSpinBox::validate( input, pos );
        // Qt 2 said Intermediate here; Qt 6 says Invalid. This is the change.
        if ( v < minimum() || v > maximum() ) return QValidator::Intermediate;
        return QValidator::Acceptable;
    }
    void fixup( QString &input ) const override {   // QRangeControl's clamp
        bool ok = false;
        const long long v = stripFix( input ).toLongLong( &ok );
        // Qt 2 only called setValue() when mapTextToValue() succeeded, then
        // updateDisplay() put the CURRENT value back -- so unparseable text
        // keeps the old value. Falling through to QSpinBox::fixup() instead
        // corrects to the MINIMUM, which 1.3 does not do. Measured.
        if ( !ok ) { input = prefix() + textFromValue( value() ) + suffix(); return; }
        input = prefix()
              + QString::number( qBound<long long>( minimum(), v, maximum() ) )
              + suffix();
    }
private:
    QString stripFix( const QString &s ) const {
        QString t = s;
        if ( !prefix().isEmpty() && t.startsWith( prefix() ) ) t = t.mid( prefix().size() );
        if ( !suffix().isEmpty() && t.endsWith( suffix() ) )   t.chop( suffix().size() );
        return t.trimmed();
    }
};
```

Each instance also needs
`setCorrectionMode( QAbstractSpinBox::CorrectToNearestValue )`, which is what
routes an Intermediate commit through `fixup()`.

**The promotion is smaller than "47 widgets" sounds.** Per form it is one
`<customwidgets>` block — the existing one in `SIG_SimulationWidgetBase.ui` is
12 lines, 8 without its `<sizehint>` — plus
`sed 's/class="QSpinBox"/class="SIG_SpinBox"/g'`. **Twelve** forms hold a
`QSpinBox`, not eight, plus one line in `SIG_AddIndividualsDialog.cpp` where
the widget is built in code.

**Do:** put it in `SIGEL_CommonGUI`, add a `<customwidget>` block to the three
forms the five View pages are built from and promote **29 widgets** —
`SIG_GPParameterBase.ui` (18), `SIG_SimulationParameterBase.ui` (8) and
`SIG_LanguageParametersBase.ui` (3). Promotion is already an established
pattern in this tree: `SIG_SimulationWidgetBase.ui` carries the one existing
`<customwidget>`.

**Then the other 18, counted:** `SIG_AddIndividualsDialog.cpp:77` builds its
spin box in code rather than in a form, so that one is a one-line type change;
`SIG_EditHostDialogBase.ui` (1), `SIG_MovieSettingsDialogBase.ui` (5) and
`SIG_SimulationWidgetBase.ui` (1) are forms; and `MT_GUI` owns 10 across six
forms. **47 spin boxes tree-wide** — 46 in the 20 forms plus the one built in
code — of which the five View pages hold the 29 above.
Regenerate `guibehaviour-baseline.txt`; every spin box's `commits=` column
should move to the clamped value, and **nothing else in the baseline should
move**. That diff is the check.

**Do NOT** do it piecemeal. Fixing the five View pages and leaving the dialogs
on stock `QSpinBox` would make the application inconsistent with itself, which
is worse than being consistently different from 1.3.

**Why a subclass, when C7 did its validator work in a runtime loop.** Three
cheaper routes were tried and rejected on measurement, not on taste:

1. **Swap the validator on the internal `QLineEdit`** (public API, no
   subclass). The digits go through, but `QAbstractSpinBox`'s own interpret
   path still calls the stock `validate()`, and 100 into a [1..99] box
   commits to **1** — worse than the divergence it was meant to remove.
2. **Widen the range.** Changes what the pages display and what is written.
3. **Validator swap PLUS an event filter that pre-clamps on every commit
   trigger.** This is the closest thing to a runtime-only fix and it very
   nearly works: 100→99, 24→23, 8001→8000, 32001→32000 all correct on
   Return. **It gets Hide wrong — 100 then hide gives 1, where 1.3 gives
   99** — and Hide is the trigger that matters here, because switching View
   pages hides the spin box and that is the ordinary way to leave a field in
   SIGEL. Qt 2 commits on FocusOut, Leave, Hide, Return, Up and Down
   (`qspinbox.cpp:575`); an event filter has to enumerate them and this one
   silently missed one.

**That is the argument for the subclass in one line: every Qt 6 commit path
funnels through the virtual `validate()`/`fixup()`, so overriding them covers
triggers you never thought to enumerate.** The subclass gets Hide and
FocusOut right without either being mentioned in it. It is also the
documented extension point — `validate()`, `fixup()`, `valueFromText()` and
`textFromValue()` are the virtuals `QAbstractSpinBox` and `QSpinBox` expose
for exactly this — so it is the idiomatic answer as well as the working one.

**One part is NOT restored by this and is left alone deliberately.** 1.3 also
drops the suffix while editing — `3 bit` at rest, plain `100` while typing,
`99 bit` after commit — because Qt 2's `updateDisplay()` writes prefix + text +
suffix into the line edit and does nothing to protect it, so select-all +
Delete takes the suffix with everything else. Qt 6's `QAbstractSpinBox`
actively keeps them in the editor. It is transient and cosmetic; chasing it
means fighting `QAbstractSpinBox`'s editor management for no change to any
value that is ever written.

**Until it is decided, the current behaviour is pinned.** Every spin box's
`commits=` value is in `guibehaviour-baseline.txt`, so the port cannot drift
further without the gate saying so.

---

## `tearDownPvm()`'s `pvm_halt()` never returns

`guidrive.cpp:424`, inside `tearDownPvm()`. Located with gdb, not guessed:
`pvm_halt` → `msendrecv` → `mroute` → `mxfer` → `select()`. Because stdout is
block-buffered to a file and the process never reaches exit, **any `printf` on
an early-return path is lost unless it flushes itself** — an assertion added
during §9 item 2 fired correctly and its message vanished, visible only under
gdb.

*Two things this entry used to say are corrected. It cited `guidrive.cpp:349`
and `PvmGuard::~PvmGuard` "after `main` has returned": the guard was a local of
the function that is now `guidriveMain`, so it ran before `main` returned, and
it has since been deleted outright. And it said such runs "are killed by their
timeout" — they are killed by PVM, well before any timeout, as measured below.*

**Deliberately not changed during the port.** `pvm_halt()` is what stops the
daemon this process started, and the comment above it records that dropping it
left `pvmd3` and its slaves running. Measured: no stray `pvmd3` survives PVM's
own shutdown, so today's behaviour is safe, only untidy. *It does not run at
all when `g_pvmOurDaemon` is false — with a daemon already up there is no halt,
no SIGTERM, and the exit status was always readable.* Connected: `PORTING.md`'s note
that SIGEL's own SIGTERM handler calls `pvm_halt()` from signal context, which
is why a SIGTERM cannot shut it down cleanly either.

**The reading side is now fixed; the halt is not.** 2026-09-06 measured what
actually kills the process, and it is not the timeout this entry assumed:
`pvm_halt()` sends `TM_HALT` and waits for a reply the daemon never sends
(`tdpro.c:1507-1516`), the daemon's `pvmbailout()` then `kill`s every local task
with SIGTERM on its way out (`pvmd.c:1485-1517`), and this process is one of
them because it enrolled with `pvm_mytid()`. Demonstrated: a `pvmcrash` run with
a 25-second watchdog under a 90-second `timeout` exited **143**, far too early
for the timeout to have fired. `guidrive` now installs a SIGTERM handler that
re-exits with the status the scenario decided (`keepExitCodeThroughPvmShutdown`,
called from `main` and from the watchdog), so `return 1` from an assertion
survives. **Before that, an assertion in `evolution`, `visualize` or `pvmcrash`
returned into a status nobody could read whenever this process had started the
daemon itself** — a failed assertion and a clean pass both left 143. Confirmed
from both sides afterwards: an `evolution` run that passed exited **0**, and a
watchdog abort exited **3**. Found by review.

**STILL OPEN, and it is the other half of the same knot.** Once the handler is
installed with a status of 0, this process reports 0 for *any* SIGTERM for the
rest of its life — including a person killing a `guidrive` wedged in the
`pvm_halt()` above. The scenario really did pass by then, so the status is not a
lie about the scenario; it is a lie about the cleanup. A "we are in teardown"
flag, or preserving only a non-zero status, would close it. Nothing gates on it
today because none of the three PVM scenarios is in `check.sh`. Found by
review.

**When to do it:** the remaining half is `pvm_halt()` itself, which still never
returns. It matters before `check.sh` ever runs one of the three PVM scenarios,
since the handler makes the status readable but does nothing about the process
sitting in `select()` until PVM kills it.

## `pvm_probe`'s error return is read as "a message is ready"

`SIG_GPFitnessTrainer.cpp:372-382`. `pvm_probe` returns a buffer id above zero
when a message is waiting, zero when none is, and a **negative error code**
otherwise. The test is `if (info != 0)`, so an error takes the branch meant for
a delivered result. Two shapes follow, and the second is worse than a hang:

- `pvm_recv` blocks and the evolution stops dead. **No `TIMEOUTMINUTES` value
  rescues it**: the timeout lives in the `else` branch (`:384-386`), so once
  control enters the `if` and blocks at `:376` it never reaches the timeout at
  all. Setting a timeout would change nothing.
- `pvm_recv` fails immediately, `pvm_upkdouble` leaves `result` at `-1`, and the
  code still decrements `noOfSlaves` and destroys the task record. The four callers
  (`SIG_GPManager.cpp:202`, `:469`, `:1461`, `:1577`) read `-1` as "not ready
  yet" and wait forever for a task that no longer exists. The individual is
  lost.

**Preserved, not introduced.** The same code in 1.0
(`sigelSourceDistribution.1.0/.../SIG_GPFitnessTrainer.cpp:216-218`) and in the
pristine `kdesigelSources.1.3.tar.gz` (line 345) — *identical statements, not
identical bytes: 1.0 indents with six spaces where both the tarball and the port
use tabs.* Found by review 2026-09-06.

**When to do it:** before any claim rests on `noOfSlaves` accounting. A
2026-09-06 argument that counted spawns to prove results had been harvested was
sound only because this branch did not fire; the argument itself could not tell.

## `SIG_GPPVMTask` holds a reference to a host that can be deleted under it

`include/SIGEL_GP/SIG_GPPVMTask.h:43` declares `SIG_GPActivePVMHost &host`, and
`SIG_GPFitnessTrainer::flushAllDynHosts` (from `:182`) calls
`resizeOwningHosts`, which deletes host objects. Any task still outstanding then
decrements a freed object at one of the two decrement sites, `:379` or `:397`.
Latent today because `addDynHost` has exactly one call site,
`SIG_GPManager.cpp:1005`, on the dynamic-client server thread that only
`sigel.cpp:267-274` starts — and `guidrive` starts no such thread, so no host is
ever flushed mid-run.

**Preserved, not introduced** — the same reference member is at
`sigelSourceDistribution.1.0/.../SIG_GPPVMTask.h:43`, the same line. Found by review
2026-09-06.

## `getNextHost`'s mutex is a function local and therefore locks nothing

`SIG_GPFitnessTrainer.cpp:550` declares `pthread_mutex_t mutex;` as a local,
`:558-559` `pthread_mutex_init`s and locks it, and `:588` unlocks it. *An
earlier version of this entry cited 552-556 and 585-587, which are the `#ifdef
_WINDOWS` halves of the same two blocks — tarball offsets applied to the port
file, 45 lines out. Found by review.* Every call gets
its own mutex, so the "now we make ourself running exclusively" comment above it
is false — two threads in this function exclude each other from nothing. The
section it guards is the one that moves entries out of `freshDynHosts`, which is
written by the dynamic client server thread, so this is the one place in the
trainer where a lock was actually wanted.

**Preserved, not introduced.** Present in the pristine `kdesigelSources.1.3`
tarball at its lines 505-514; **absent from 1.0**, which has no dynamic-host
feature at all, so it arrived with 1.3. Found by review 2026-09-06.

## `renderRecorder` leaks whenever the visualisation constructor throws

`SIG_SimulationVisualisation.cpp:53` allocates `renderRecorder`; `:55` then
constructs `SIG_Simulation`, which throws for the removed Dynamo backend
(`SIG_Simulation.cpp`, default case). The destructor at `:74-78` — the only
thing that deletes it — never runs, so each attempt leaks one
`SIG_RenderRecorder`.

**Not fixed with §9 item 5.** That step nulled `visualisation` so the freed
pointer could not be dereferenced or double-freed; this is the other half and
needs the constructor to clean up after itself. Not reachable today for the
same reason the null is not: 1.3's Dynamo path segfaults on Play (oracle,
2026-09-02) and nothing in the slave can switch libraries mid-run. It becomes
live the moment any caller catches that throw and continues.

## ~~`MT_Controller` should refuse a mid-run `configureSystem` ITSELF~~ — SUPERSEDED

**Dead as of D33, 2026-09-09.** The protection goes in the UI and the model is not
to be touched. `MT_Control` is a core module, so a refusal inside `MT_Controller`
is not available. The entry is kept because the DEFECT it describes is real and
still needs covering — from the UI side.

**The menu greying is not to be removed.** D30 greys the MetaGP actions for the
duration of a run and that stays. This entry PROPOSED a second check inside
`MT_Controller`, so the guard would also sit with the code that does the damage.
D33 rules that out — see the strike above.
Jan's instruction at the time, verbatim: *"do not REMOVE the greyed out! In
ADDITION MT_Controller should refuse, multiple layers of checks"*. The first half
stands; the second is what D33 overturned.

**Why it is worth a second layer.** `MT_Controller::configureSystem`
(`:402-404`) does `mainWindow->show(); delete substitution; substitution = 0;`,
and `substitution` is the object `SIG_GPManager` is holding as its `trainer`
whenever the meta system is the Evaluator. Deleting it mid-run is a
use-after-free that aborts the process. D30 shuts every route the GUI currently
offers; it cannot shut a route nobody has found yet. A refusal inside
`configureSystem` is refused twice rather than not at all.

**WHAT STANDS IN THE WAY — measured 2026-09-07, not assumed:**

- **`MT_Control` has no dependency on `SIGEL_MasterGUI` today.** Zero includes,
  either direction, across `src/MT_Control` and `include/MT_Control`.
- `SIG_Experiment::anyEvolutionRunning()` is a static on `SIG_Experiment.h:267`,
  which lives in `SIGEL_MasterGUI`. Calling it from `MT_Controller` adds a new
  module edge — the kind Phase A spent effort cutting.
- `MT_Controller` holds `SIGEL_GP::SIG_GPExperiment &sigExp`
  (`MT_Controller.h:67`), **not** the GUI `SIG_Experiment`, so the flag is not
  reachable through what it already has.
- The counter itself, `g_runningEvolutions`, is a file-static in
  `SIG_Experiment.cpp:212`.

**THAT ROUTE IS DEAD — REJECTED BY JAN 2026-09-09.** It proposed moving the run
counter down into `SIGEL_GP`. His objection: the counter is ours — added by D29
in `e998b76`, 2026-09-05, 28 commits before this was written — and pushing it
into the core model to serve a menu guard is the wrong direction. His words: *"I strongly reject changes to the core model
just to hot-fix a UI enablement issue."* **The counter is to be removed, not
relocated.** What replaces it is undecided.

**When to do it:** the LAYER is dead too, not only the route — D33 puts the
protection in the UI and `MT_Control` is a core module. What survives is the
defect, and it has to be covered from the UI side.

**Two facts measured 2026-09-07 that change the options above:**

- **`SIG_GPManager::running()` IS GONE — deleted 2026-09-09, with the empty
  `wait()` and `msleep()` that sat beside it.** It was a 2003 stub returning
  `false` unconditionally, overridden nowhere, so a guard written against it
  could never fire. It was a leftover from a `QThread` base dropped before
  release 1.0, not an unfinished feature — PORTING.md's D29 passage has the
  evidence. **So "just ask the manager" is not available and is not coming
  back**: D33 puts this problem in the UI, and the model is not to be touched.
- **`MT_Control` already depends on `SIGEL_GP`** — `MT_Evaluator.h:11` and
  `MT_Classifier.h:11-14` include `SIGEL_GP/` headers today, so the dependency
  objection above applies only to `SIGEL_MasterGUI`. *This was the argument that
  moving the counter into `SIGEL_GP` costs nothing. It is kept only as a fact
  about the dependencies — **it no longer supports anything**, because that move
  is rejected.*

---

## `pvm-check.sh` captures exit status the way `check.sh` did before it was fixed

`pvm-check.sh:124` and `:128` are `"$PVM_TMP/pvm_smoke"; p3=$?` and
`"$LINK"; p4=$?` — a bare command followed by `$?`, which is exactly the shape
that made `check.sh`'s form-minimums gate unable to report a failure (PORTING.md,
form minimums). It is **not** a defect today, and that is the whole point of
recording it: `pvm-check.sh:49` is `set -u` alone, the only one of the four gate
scripts without `-e`, so nothing aborts and `p3`/`p4` are read correctly.

**The risk is that somebody adds `-e` to that line.** It is the obvious
tightening, three of the four scripts already have it, and the moment it lands
both captures become dead code and both halves of the PVM check start exiting
the script instead of printing PASS/FAIL. The same line would also leave
`pvm-check.sh:79-80` (`mkdir -p`, `rm -f`), `:98` (`cat > lsan.supp`) and `:105`
(`pvmd3 &`) newly unguarded.

**What to do:** if `-e` is ever added, convert both captures to `p3=0; cmd || p3=$?`
in the same move, and check the four sites above. Doing it the other way round —
adding `-e` first and fixing the fallout after — is how the form-minimums gate
lost its teeth for two days without anyone noticing.

*Found by review 2026-09-07, while auditing check.sh for the same defect class.*

---

## SIGEL needs a real logging system

**RESTORED 2026-09-09, and the restoration is the point.** This item was opened
on 2026-08-20 at the review of A1–A6, written into PORTING.md by `1734ab3`
("note the logging debt"), and **removed the same day** by `2e23cc3` ("fix the
regressions and shim defects found by review") — a doc restructure, not a
decision. It was gone for twenty days before Jan noticed it missing. Nothing else
in either document mentions logging.

**The defect it records.** Qt 2's `QTextStream` wrote straight through to
unbuffered `stderr` on every `<<`. Qt 6 buffers and flushes only on `flush()`,
`Qt::endl`, overflow, or destruction. **A trailing `"\n"` does not flush.**
`SIGEL_Tools::SIG_IO` still declares `cin`, `cout` and `cerr` as plain
`QTextStream` (`SIG_IO.h:49-59`), so every diagnostic in the program inherits
that behaviour.

**Re-counted 2026-09-09, over `.cpp` and `.h` under the source root:**

| | 2026-08-20 | 2026-09-09 |
|---|---|---|
| `SIG_IO::cerr` / `cout` mentions | 509 | **475** |
| lines that flush (`Qt::endl` or `flush`) | 9 | **35** |
| lines ending in a bare `"\n"` | not counted then | **232** |

So the port has flushed 26 more sites than it had, and **232 statements still end
in a newline that does not flush**. Those sit in a buffer and are lost if the
process dies — including on the SIGSEGV path, which is exactly when they are
wanted.

**What is required, after the Qt migration.** A proper logging system: levels,
one place that decides where output goes and when it is flushed, and something
the GUI can display. It replaces both `SIG_IO` and the console-warning stopgap.

**Do NOT fix this by adding `Qt::endl` to 232 call sites.** That was the original
entry's closing instruction and it still holds. It would bury the real change
under a mechanical diff and leave the design untouched.

**WHERE TO START — Jan, 2026-09-09.** The code base uses
`SIGEL_Tools::SIG_IO::cerr` extensively. That is the hook: start there, and
assess the pattern from it when the time comes.

---

## Remove the Windows and Visual Studio support

**Decided 2026-09-09 by Jan.** The Windows half of this tree is dead. It does
not build here, nothing tests it, and it has not been built by anybody since
2003. It is to go. Recorded as a work item, not started.

**WHAT IS THERE — counted 2026-09-09, over every `.c`, `.cpp` and `.h` under
`x/kdesigelSources.1.3`:**

- **9 Visual Studio project files**, all at the source root: 5 `.dsp`
  (`Sigel`, `SIGELCommon`, `MetaSIGEL`, `sigel_slave`, `manage_dyn_slave`),
  1 `.dsw` (`Sigel.dsw`) and 3 `.mak` (`Sigel.mak`, `sigel_slave.mak`,
  `manage_dyn_slave.mak`). Together 7,962 lines. They name Qt 2 paths, MSVC 6
  switches and a `uic` that is not the one this port runs. **Four of them are
  already named at `future_refactorings.md:292`, under *Translate the German*:** *"Do not
  translate ... MSVC-generated German, not built by this port. Delete them or
  leave them."* This item decides it — they go.
- **2 `WIN_`-prefixed sources** — `WIN_SIG_GPRemoteZORCFitnessFunction.h` and
  `.cpp`, 451 lines. The `.cpp` is already excluded from the build by name
  (`Makefile:315`, `EXCLUDE_SIGEL_GP`), so it compiles nowhere.
- **206 `_WINDOWS` occurrences across 57 source files**: 195 `#ifdef _WINDOWS`,
  9 `#ifndef _WINDOWS`, and 2 inside commented-out code
  (`MT_GPManager.cpp:469` and `:594`).
- **4 `#include <windows.h>`** — `MT_Controller.h:16`, `MT_Substitute.h:21`,
  `MT_GPSystem/MT_GPManager.h:22`, `MT_GPSystem/MT_GPManager.cpp:12`.
- The Windows branches carry their own thread and mutex types — `HANDLE`,
  `DWORD WINAPI`, `LPVOID` — against `pthread_t` and `pthread_mutex_t` on the
  side that is built.

**`src/manage_dyn_slave.c` IS IN THIS LIST and is easy to miss.** It is the only
`.c` file in the tree, it holds 9 of the 206 occurrences, and a sweep written as
`--include=*.cpp --include=*.h` does not see it. The first version of this entry
made exactly that mistake and reported 197 across 56 files; found by review
2026-09-09. It has a `.dsp` and a `.mak` of its own in the list above.

**WHY IT IS WORTH DOING, beyond tidiness.** Every `#ifdef _WINDOWS` is a second
version of a function that no compiler here ever reads. It cannot be tested and
it cannot be trusted, but it is read by anybody working on the file.
`MT_Controller.h` is the clearest case: the meta thread is declared twice, at
`:73` as `HANDLE meta_thread` and at `:75` as `pthread_t meta_thread`, and the
thread entry point is declared twice as well at `:34` and `:36`.

**THIS ITEM SUPERSEDES D22, WHICH IS A SIGNED DECISION — say so when doing it.**
D22 in PORTING.md's decision table chose what the two style branches do: Fusion for the
`#else`, and **the `#ifdef _WINDOWS` branch keeps Windows**, by name, because
Qt 6 still creates that style. There are three such call sites —
`sigel.cpp:212`, `sigel_slave.cpp:277` and `:344` — and they are live, ported
Qt 6 code, not 2003 leftovers. Keeping the `#else` half deletes them, which is
the right outcome once Windows is gone, but it is a decision being overturned
and not a mechanical edit.

**IT ALSO OVERTURNS THREE "PERMANENT" STATEMENTS about the `WIN_*` files.**
PORTING.md's Phase C exclusions call them *"an explicit exclusion rather than a standing"*
gap; PORTING.md's C7/C8 row says *"one remains and always will"*; `check.sh:2389`
prints *"Windows-only WIN_* file(s) excluded -- permanent, §7"*. All three must
be edited in the same move. **And the counter behind that line goes with them:**
`check.sh:95` initialises `winskip`, `:174` and `:219` increment it on a `WIN_*`
basename, and `:2364` only prints when it is non-zero — delete the files and the
line disappears, which will look like a lost check unless it is done knowingly.

**HOW TO DO IT — the shape, not a plan.** Delete the 9 project files and the 2
`WIN_` sources first; nothing includes them, and `Makefile:315` and the four
`winskip` sites go with them. Then take the `#ifdef _WINDOWS` blocks one module
at a time, keeping the `#else` half and deleting the conditional. That half is
the one the build already uses, so each edit is checkable: the object file must
not change. Do not mix it with any other change.

**WHAT TO WATCH.** The 9 `#ifndef _WINDOWS` blocks are the reverse polarity —
their body is KEPT and only the guard goes. Reading them as `#ifdef` and
deleting the body would remove live code. **And 12 headers carry `_WINDOWS`, not
the three an earlier version of this entry listed** — a mistake there changes
what every including translation unit sees:

```
MT_Control/MT_Controller.h              SIGEL_Simulation/SIG_Recorder.h
MT_Control/MT_Substitute.h              SIGEL_Simulation/SIG_Register.h
MT_GPSystem/MT_GPManager.h              SIGEL_Simulation/SIG_SimulationQueries.h
SIGEL_GP/SIG_GPManager.h                SIGEL_Visualisation/SIG_EnvironmentRenderer.h
SIGEL_Program/SIG_Program.h             SIGEL_Visualisation/SIG_EnvironmentVisualisation.h
SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h
SIGEL_Visualisation/SIG_Renderer.h
```

**IT ALSO MOVES A PINNED GATE TOTAL.** The 9 project files and the 2 `WIN_`
sources are 11 of the 611 files the `encodings` gate counts, so deleting them
takes `./check.sh` from 1136 pass to 1125. PORTING.md pins that number twice — the per-step exit criterion in §7, and the
gate list — both say. Move them in the same commit, or the next session reads a green tree as a
regression. This is the same class of coupling the rest of this entry lists.

**When to do it:** after the MetaGP guard step and its review. Not before.
