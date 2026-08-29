/*
  One fitness evaluation from an experiment file -- PORTING.md §3.

  This does what the master does before it ships work to a slave
  (SIG_GPFitnessTrainer.cpp:41-68): copy the experiment's robot, call
  prepareDynaMechs on the copy, then hand robot, environment, simulation
  parameters and one individual's program to a fitness function.

  No PVM and no QApplication. SIG_GPPopulation guards every dialog with
  if (qApp), so loading an experiment runs headless.

  The experiment file already records the fitness each individual scored in
  2003, so `sigel_eval <exp> <n>` prints the new value next to the old one.
*/
#include <cstdio>
#include <cstdlib>

#include <QFile>
#include <QHashSeed>
#include <QTextStream>

#include "SIGEL_GP/SIG_GPExperiment.h"
#include "SIGEL_GP/SIG_GPNiceWalkingFitnessFunction.h"
#include "SIGEL_GP/SIG_GPSimpleFitnessFunction.h"
#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_Robot/SIG_CommandParameters.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_RobotIO/SIG_RobotBuilder.h"
#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"
#include "SIGEL_Robot/SIG_GlueJoint.h"
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Sensor.h"
#include "SIGEL_Simulation/SIG_Simulation.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Tools/SIG_Exception.h"

using SIGEL_Simulation::SIG_SimulationParameters;

// Every Q2Dict whose iteration order reaches the simulation -- PORTING.md
// Phase D. SIG_Robot holds six (SIG_Robot.h:60-65); all six are written in
// iteration order by SIG_Robot::writeToFileTransfer and read back in that
// order by SIG_DynaMechsSimulationData, which is what numbers the DynaMechs
// bodies. (This named SIG_DynaMoSimulationData until 2026-08-28 -- that was
// the Dynamo class, deleted with that backend. Both walked the same four
// dicts in the same order, so the ordering this gate protects is unchanged.)
// SIG_Link::points is a seventh, one per link.
//
// D1 first dumped only links and joints. A review rebuilt the core with a
// perturbed Q2Dict::hash and found 6 of 14 experiments whose order changed
// while the diff stayed empty -- the four unwatched dicts were carrying it.
// SIG_Body and SIG_Material carry no number; the other four do.
template <typename T> static int SIG_NUMBER_OF(const T *e) { return e->getNumber(); }
static int SIG_NUMBER_OF(const SIGEL_Robot::SIG_Body *)     { return -1; }
static int SIG_NUMBER_OF(const SIGEL_Robot::SIG_Material *) { return -1; }

static void dumpOrder(const SIGEL_Robot::SIG_Robot &r, const char *which)
{
// Position AND stored number. The two are independent: position becomes the
// DynaMechs body index, while the stored number is what an evolved program's
// SENSE and MOVE operands resolve through (SIG_DynaMechsSimulationQueries.cpp:
// 93-95, SIG_DynaMechsCommandInterface.cpp:74). Reordering a .rrb changes the
// number; reordering an .exp would change the position. A gate that watched
// only position could not see the first.
#define SIG_DUMP(label, Type, accessor)                                    \
  do {                                                                     \
    int n = 0;                                                             \
    for (Type *e : r.accessor())                                           \
      printf("  %-8s %-9s %2d #%-3d %s\n", which, label, n++,               \
             SIG_NUMBER_OF(e), qPrintable(e->getName()));                  \
  } while (0)

  SIG_DUMP("body",     SIGEL_Robot::SIG_Body,     getBodies);
  SIG_DUMP("material", SIGEL_Robot::SIG_Material, getMaterials);
  SIG_DUMP("link",     SIGEL_Robot::SIG_Link,     getLinks);
  SIG_DUMP("joint",    SIGEL_Robot::SIG_Joint,    getJoints);
  SIG_DUMP("drive",    SIGEL_Robot::SIG_Drive,    getDrives);
  SIG_DUMP("sensor",   SIGEL_Robot::SIG_Sensor,   getSensors);
#undef SIG_DUMP

  // Geometry digest. Review showed that doubling every VRML vertex left both
  // baselines byte-identical: the .exp path never runs the VRML reader, and the
  // dump recorded only names and numbers, nothing derived from a coordinate.
  // A count and a checksum per body close that, and cost one line each.
  for (SIGEL_Robot::SIG_Body *b : r.getBodies()) {
    const SIGEL_Robot::SIG_Geometry *g = b->getGeometry();
    if (!g) { printf("  %-8s geom      -  %s  (none)\n", which, qPrintable(b->getName())); continue; }
    double sum = 0.0;
    for (int i = 0; i < g->getNumVertices(); ++i) {
      DL_vector v = g->getVertex(i);
      sum += v.get(0) * 1.0 + v.get(1) * 2.0 + v.get(2) * 3.0;
    }
    printf("  %-8s geom   %4d v %4d p  %+.9e  %s\n", which,
           g->getNumVertices(), g->getNumPolygons(), sum, qPrintable(b->getName()));
  }

  // The eighth ordered container, missed by the first enumeration: it rides
  // inside every .exp and every PVM transfer through writeToFileTransfer.
  // Nothing numbers commands -- they are looked up by name -- so like bodies
  // and materials its order reaches only the serialised bytes.
  {
    int n = 0;
    for (const SIGEL_Robot::SIG_LanguageParameters::NamedCommand &c :
         r.getLangParam()->getCommands())
      printf("  %-8s command   %2d      %s\n", which, n++, qPrintable(c.name));
  }

  // Each link carries its own dict of significant points, in its own order.
  for (SIGEL_Robot::SIG_Link *l : r.getLinks()) {
    int n = 0;
    for (const SIGEL_Robot::SIG_Link::NamedPoint &p : l->getPoints())
      printf("  %-8s point %s %2d  %s\n", which,
             qPrintable(l->getName()), n++, qPrintable(p.name));
  }
}

// Duplicate-key tie-breaking, which neither gate can see.
//
// Qt 2's QDict returned the NEWEST binding for a duplicate key, and the Q2Dict
// that replaced it did the same. Phase D replaced Q2Dict with plain QList and
// eight hand-written lookups, and D4's six scanned FORWARD -- oldest wins, a
// silent flip found by review. An independent rebuild confirmed that flipping
// them back leaves dictorder-baseline.txt and fitness-baseline.txt both empty:
// no shipped robot has a duplicate name, so nothing in the data can catch it.
// Hence this, run by fitness-check.sh before the evaluations.
static int selfcheck()
{
  int bad = 0;
#define SIG_WANT(cond)                                                     \
  do { if (!(cond)) { printf("selfcheck FAILED: %s\n", #cond); ++bad; } }  \
  while (0)

  {   // SIG_LanguageParameters: newest wins, and removeCommand frees THAT one.
    SIGEL_Robot::SIG_LanguageParameters lp;
    SIGEL_Robot::SIG_CommandParameters *first  = new SIGEL_Robot::SIG_CommandParameters();
    SIGEL_Robot::SIG_CommandParameters *second = new SIGEL_Robot::SIG_CommandParameters();
    lp.addCommand("DUP", first);
    lp.addCommand("DUP", second);
    SIG_WANT(lp.getCommand("DUP") == second);
    lp.removeCommand("DUP");
    SIG_WANT(lp.getCommand("DUP") == first);
    lp.removeCommand("DUP");
    SIG_WANT(lp.getCommand("DUP") == 0);
  }
  {   // SIG_Link::getPoint, same rule.
    SIGEL_Robot::SIG_Link link(0, "L", 0);
    link.addPoint("P", DL_vector(1, 0, 0));
    link.addPoint("P", DL_vector(2, 0, 0));
    SIG_WANT(link.getPoint("P").x == 2);
  }
  {   // All six of SIG_Robot's lookups. Checking only one left the other five
      // able to flip back to oldest-wins with every check silent -- which is
      // precisely the hole this whole self-check exists to close.
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Body     *b1 = new SIGEL_Robot::SIG_Body(&robot, "S", "d");
    SIGEL_Robot::SIG_Body     *b2 = new SIGEL_Robot::SIG_Body(&robot, "S", "d");
    SIGEL_Robot::SIG_Material *m1 = new SIGEL_Robot::SIG_Material(&robot, "S");
    SIGEL_Robot::SIG_Material *m2 = new SIGEL_Robot::SIG_Material(&robot, "S");
    SIGEL_Robot::SIG_Link     *l1 = new SIGEL_Robot::SIG_Link(&robot, "S", 0);
    SIGEL_Robot::SIG_Link     *l2 = new SIGEL_Robot::SIG_Link(&robot, "S", 1);
    SIGEL_Robot::SIG_Joint    *j1 = new SIGEL_Robot::SIG_GlueJoint(&robot, "S", 0);
    SIGEL_Robot::SIG_Joint    *j2 = new SIGEL_Robot::SIG_GlueJoint(&robot, "S", 1);
    SIGEL_Robot::SIG_Drive    *d1 = new SIGEL_Robot::SIG_Drive(&robot, "S", 0);
    SIGEL_Robot::SIG_Drive    *d2 = new SIGEL_Robot::SIG_Drive(&robot, "S", 1);
    SIGEL_Robot::SIG_Sensor   *s1 = new SIGEL_Robot::SIG_ContactSensor(&robot, "S", 0);
    SIGEL_Robot::SIG_Sensor   *s2 = new SIGEL_Robot::SIG_ContactSensor(&robot, "S", 1);
    robot.addBody(b1);     robot.addBody(b2);
    robot.addMaterial(m1); robot.addMaterial(m2);
    robot.addLink(l1);     robot.addLink(l2);
    robot.addJoint(j1);    robot.addJoint(j2);
    robot.addDrive(d1);    robot.addDrive(d2);
    robot.addSensor(s1);   robot.addSensor(s2);
    SIG_WANT(robot.lookupBody("S")     == b2);
    SIG_WANT(robot.lookupMaterial("S") == m2);
    SIG_WANT(robot.lookupLink("S")     == l2);
    SIG_WANT(robot.lookupJoint("S")    == j2);
    SIG_WANT(robot.lookupDrive("S")    == d2);
    SIG_WANT(robot.lookupSensor("S")   == s2);
    SIG_WANT(robot.lookupLink("MISSING") == 0);
  }
  {   // SIG_Material::friction, D11.  No shipped robot declares friction at
      // all -- 0 in all 7 .rrb, and nfric is 0 on all 31 Material lines -- so
      // both gates run this list empty and neither can see the conversion.
      // setFrictionValue's walk decides whether to append, which is exactly
      // the semantic a careless rewrite drops.
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Material a(&robot, "a");
    SIGEL_Robot::SIG_Material b(&robot, "b");
    SIGEL_Robot::SIG_Material c(&robot, "c");

    SIG_WANT(a.getFrictionValue(&b) == 0.6);      // the not-found default

    a.setFrictionValue(&b, 0.25);                 // negotiates by default
    SIG_WANT(a.getFrictionValue(&b) == 0.25);
    SIG_WANT(b.getFrictionValue(&a) == 0.25);

    a.setFrictionValue(&c, 0.5);
    SIG_WANT(a.getFrictionValue(&b) == 0.25);     // still there, not overwritten
    SIG_WANT(a.getFrictionValue(&c) == 0.5);

    a.setFrictionValue(&b, 0.75);                 // UPDATE, must not append
    SIG_WANT(a.getFrictionValue(&b) == 0.75);
    SIG_WANT(a.getFrictionValue(&c) == 0.5);

    // The serialiser is the only public window on the list's length, and
    // its walk is itself converted code no gate reaches.
    // "Material a <elasticity> <density> <nfric> b 0.75 c 0.5 <colour>"
    QString written;
    { QTextStream ts(&written); a.writeToFileTransfer(ts); }
    SIG_WANT(written.split(' ').value(4) == "2");   // 3 would be the append bug
    SIG_WANT(written.contains("b 0.75"));
    SIG_WANT(written.contains("c 0.5"));

    a.setFrictionValue(&a, 0.1);                  // self: must not recurse
    SIG_WANT(a.getFrictionValue(&a) == 0.1);
  }
  {   // SIG_Link::noCollide, D10.  Also unreachable from the data: 0
      // 'nocollide' in all 7 .rrb and noCollideCount 0 in all 261 .exp Link
      // records.  addNoCollide's !contains guard is what stops the pair being
      // added twice, and nothing else tests it.
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Link l1(&robot, "l1", 0);
    SIGEL_Robot::SIG_Link l2(&robot, "l2", 1);

    SIG_WANT(l1.getNoCollides().count() == 0);
    l1.addNoCollide(&l2);                         // negotiates both directions
    SIG_WANT(l1.getNoCollides().count() == 1);
    SIG_WANT(l2.getNoCollides().count() == 1);
    SIG_WANT(l1.getNoCollides().at(0) == &l2);

    l1.addNoCollide(&l2);                         // the duplicate guard
    SIG_WANT(l1.getNoCollides().count() == 1);
    SIG_WANT(l2.getNoCollides().count() == 1);
  }
#undef SIG_WANT
  printf(bad ? "selfcheck: %d FAILED\n" : "selfcheck: ok\n", bad);
  return bad ? 1 : 0;
}

int main(int argc, char *argv[])
{
  // PORTING.md §9. q2compat.h calls this during static initialisation, but
  // static initialisation order across translation units is unspecified.
  QHashSeed::setDeterministicGlobalSeed();

  bool verbose = false;
  if (argc > 1 && QString(argv[1]) == "-v") { verbose = true; argv++; argc--; }
  if (argc > 1 && QString(argv[1]) == "-selfcheck") return selfcheck();

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "usage: %s [-v] <experiment.exp> [individual, default 0]\n", argv[0]);
    return 2;
  }

  // A .rrb is the robot model as SIGEL_RobotIO reads it -- scanner, compiler,
  // builder, inserting in declaration order, which is not the order the
  // serialised copy inside an .exp comes back in. Phase D has to migrate both,
  // so the baseline has to watch both.
  if (QString(argv[1]).endsWith(".rrb")) {
    try {
      SIGEL_RobotIO::SIG_RobotBuilder builder(argv[1]);
      SIGEL_Robot::SIG_Robot *r = builder.build();
      dumpOrder(*r, "rrb");
      delete r;
    } catch (SIGEL_Tools::SIG_Exception &e) {
      fprintf(stderr, "loading %s: %s\n", argv[1], qPrintable(e.getMessage()));
      return 1;
    }
    return 0;
  }

  QFile file(argv[1]);
  if (!file.open(QIODevice::ReadOnly)) {
    fprintf(stderr, "cannot open %s\n", argv[1]);
    return 1;
  }

  SIGEL_GP::SIG_GPExperiment experiment;
  try {
    QTextStream stream(&file);
    experiment.loadExperiment(stream);
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    fprintf(stderr, "loading %s: %s\n", argv[1], qPrintable(e.getMessage()));
    return 1;
  }
  file.close();

  const int index = (argc == 3) ? atoi(argv[2]) : 0;
  if (index < 0 || index >= experiment.population.getSize()) {
    fprintf(stderr, "individual %d is outside 0..%d\n",
            index, experiment.population.getSize() - 1);
    return 1;
  }

  if (experiment.simulationParameter.getSimulationLibrary()
      != SIG_SimulationParameters::DynaMechs) {
    fprintf(stderr, "%s selects Dynamo; this program runs DynaMechs only\n", argv[1]);
    return 1;
  }

  // Both orders, because they are not the same one. SIG_Robot's copy ctor
  // round-trips through writeToFileTransfer/readFromFileTransfer, and
  // Q2Dict::insert prepends, so re-inserting in iteration order reverses every
  // colliding chain. They differ for 4 of the 14 experiments. After the shim
  // goes, load and save become order-preserving and the two collapse into one,
  // so a baseline that records only the copy cannot see experiment.robot move.
  if (verbose)
    dumpOrder(experiment.robot, "loaded");

  SIGEL_Robot::SIG_Robot robot(experiment.robot);
  try {
    robot.prepareDynaMechs();
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    fprintf(stderr, "preparing the robot: %s\n", qPrintable(e.getMessage()));
    return 1;
  }

  if (verbose) {
    const char *cmds[] = { "MOVE", "COPY", "ADD", "SENSE", "JUMP", "DELAY", 0 };
    for (int c = 0; cmds[c]; c++)
      if (robot.getLangParam()->hasCommand(cmds[c]))
        printf("  command %-6s duration %g\n", cmds[c],
               robot.getLangParam()->getCommand(cmds[c])->getDuration());
    printf("  maximalDelayTime %d  registerWidth %d\n",
           robot.getLangParam()->getMaximalDelayTime(),
           robot.getLangParam()->getRegisterWidth());
  }

  if (verbose)
    dumpOrder(robot, "copy");

  SIGEL_GP::SIG_GPIndividual &individual = experiment.population.getIndividual(index);
  const QString name = experiment.gpParameter.getFitnessName();

  SIGEL_GP::SIG_GPFitnessFunction *fitnessFunction = 0;
  if (name == "SimpleFitnessFunction")
    fitnessFunction = new SIGEL_GP::SIG_GPSimpleFitnessFunction(
        individual.getProgramVar(), robot,
        experiment.environment, experiment.simulationParameter);
  else if (name == "NiceWalkingFitnessFunction")
    fitnessFunction = new SIGEL_GP::SIG_GPNiceWalkingFitnessFunction(
        individual.getProgramVar(), robot,
        experiment.environment, experiment.simulationParameter);
  else {
    fprintf(stderr, "%s names %s, which this program does not build\n",
            argv[1], qPrintable(name));
    return 1;
  }

  const double recorded = individual.getFitness();
  const double fitness = fitnessFunction->evalFitness();

  if (verbose) {
    // Re-run with a recorder we can read, to see the trajectory the fitness
    // function saw. NiceWalking returns 0 the moment height leaves its band.
    SIGEL_GP::SIG_GPFullDataRecorder trace(1);
    SIGEL_Simulation::SIG_Simulation sim(robot, experiment.environment,
                                         individual.getProgramVar(),
                                         experiment.simulationParameter, trace);
    try { sim.start(); }
    catch (SIGEL_Tools::SIG_Exception &e) {
      printf("  simulation threw: %s\n", qPrintable(e.getMessage()));
    }
    double lo = 1e300, hi = -1e300;
    int frames = 0;
    DL_vector last;
    for (DL_vector *p = trace.positions.first(); p; p = trace.positions.next()) {
      if (p->y < lo) lo = p->y;
      if (p->y > hi) hi = p->y;
      last = *p;
      frames++;
    }
    printf("  frames %d  height %.6g .. %.6g  last (%.6g, %.6g, %.6g)\n",
           frames, lo, hi, last.x, last.y, last.z);
  }
  delete fitnessFunction;

  printf("%s individual %d  %s\n", argv[1], index, qPrintable(name));
  printf("  2003 recorded  %.17g\n", recorded);
  printf("  this run       %.17g\n", fitness);
  return 0;
}
