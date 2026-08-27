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
// order by SIG_DynaMoSimulationData, which is what numbers the DynaMechs
// bodies. SIG_Link::points is a seventh, one per link.
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

  // Each link carries its own dict of significant points, in its own order.
  for (SIGEL_Robot::SIG_Link *l : r.getLinks()) {
    int n = 0;
    for (const SIGEL_Robot::SIG_Link::NamedPoint &p : l->getPoints())
      printf("  %-8s point %s %2d  %s\n", which,
             qPrintable(l->getName()), n++, qPrintable(p.name));
  }
}

int main(int argc, char *argv[])
{
  // PORTING.md §9. q2compat.h calls this during static initialisation, but
  // static initialisation order across translation units is unspecified.
  QHashSeed::setDeterministicGlobalSeed();

  bool verbose = false;
  if (argc > 1 && QString(argv[1]) == "-v") { verbose = true; argv++; argc--; }

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
