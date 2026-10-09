/*
  coredrive: the check program for SIGEL's core. It links SIGEL's libraries
  and has no window and no PVM.

    coredrive <experiment.exp> [n]   evaluates individual n as the master
                                     prepares it for a slave: it copies the
                                     robot, calls prepareDynaMechs on the
                                     copy, and gives robot, environment,
                                     simulation parameters and the program
                                     to the experiment's fitness function
    coredrive -v ...                 the same, with a trace of the simulation
    coredrive -order <file>          prints the order of a robot's parts
    coredrive -check <experiment>    lists the issues of the robot check

  SIG_GPPopulation shows a dialog only when a QApplication exists, so an
  experiment loads here without one.
*/
#include <QList>
#include <QString>
#include <cstdio>
#include <cstdlib>

#include <QFile>
#include <QHashSeed>
#include <QTextStream>


#include "SIGEL_GP/SIG_GPExperiment.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPFitnessFunctionRegistry.h"
#include "SIGEL_GP/SIG_GPRemoteZORCFitnessFunction.h"
#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_RobotCheck/SIG_RobotChecker.h"
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


// The order of SIG_Robot's lists. SIG_Robot::writeToFileTransfer writes all of
// them in list order, so that is the order the next reader sees: the copy
// constructor, and a PVM slave. The reader appends each joint to its links'
// lists in that order, and SIG_DynaMechsSimulationData numbers the DynaMechs
// bodies by a walk over SIG_Link::getJoints(). That walk is not printed here.
// SIG_Body and SIG_Material carry no number; the other four do.
template <typename T> static int SIG_NUMBER_OF(const T *e) { return e->getNumber(); }
static int SIG_NUMBER_OF(const SIGEL_Robot::SIG_Body *)     { return -1; }
static int SIG_NUMBER_OF(const SIGEL_Robot::SIG_Material *) { return -1; }

static void dumpOrder(const SIGEL_Robot::SIG_Robot &r, const char *which)
{
// Position AND stored number. The two are independent: position is the order
// the next reader of the robot's stream sees, while the stored number is what
// an evolved program's SENSE and MOVE operands resolve through
// (SIG_DynaMechsSimulationQueries.cpp, sense; SIG_DynaMechsCommandInterface.cpp,
// moveDrive). Reordering a .rrb changes the number; reordering an .exp would
// change the position. A check that watched only position could not see the first.
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

  // Geometry digest: a count and a sum of coordinates per body. Without it
  // nothing printed here depends on a coordinate, and a fault in the VRML
  // reader would not show.
  // A count and a checksum per body close that, and cost one line each.
  for (SIGEL_Robot::SIG_Body *b : r.getBodies()) {
    const SIGEL_Robot::SIG_Geometry *g = b->getGeometry();
    if (!g) { printf("  %-8s geom      -  %s  (none)\n", which, qPrintable(b->getName())); continue; }
    double sum = 0.0;
    for (int i = 0; i < g->getNumVertices(); ++i) {
      SIG_Vector v = g->getVertex(i);
      sum += v.get(0) * 1.0 + v.get(1) * 2.0 + v.get(2) * 3.0;
    }
    printf("  %-8s geom   %4d v %4d p  %+.9e  %s\n", which,
           g->getNumVertices(), g->getNumPolygons(), sum, qPrintable(b->getName()));
  }

  // The commands are written with every robot. They are found by name, so
  // like bodies and materials their order reaches only the written file.
  // and materials its order reaches only the serialised bytes.
  {
    int n = 0;
    for (const SIGEL_Robot::SIG_LanguageParameters::NamedCommand &c :
         r.getLangParam()->getCommands())
      printf("  %-8s command   %2d      %s\n", which, n++, qPrintable(c.name));
  }

  // Each link has its own list of points.
  for (SIGEL_Robot::SIG_Link *l : r.getLinks()) {
    int n = 0;
    for (const SIGEL_Robot::SIG_Link::NamedPoint &p : l->getPoints())
      printf("  %-8s point %s %2d  %s\n", which,
             qPrintable(l->getName()), n++, qPrintable(p.name));
  }
}

int main(int argc, char *argv[])
{
  // No result may depend on Qt's random hash seed.
  QHashSeed::setDeterministicGlobalSeed();

  bool verbose = false;
  if (argc > 1 && QString(argv[1]) == "-v") { verbose = true; argv++; argc--; }
  bool robotCheck = false;
  if (argc > 1 && QString(argv[1]) == "-check") { robotCheck = true; argv++; argc--; }
  bool order = false;
  if (argc > 1 && QString(argv[1]) == "-order") { order = true; argv++; argc--; }

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "usage: %s [-v] [-check] <experiment.exp> [individual, default 0]\n       %s -order <experiment.exp | robot.rrb>\n", argv[0], argv[0]);
    return 2;
  }

  // A robot file is built by SIGEL_RobotIO: scanner, compiler, builder.
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

  // One line per issue, tab-separated: check, kind, part, title, analysis and
  // advice. Then one line that counts the issues.
  if (robotCheck) {
    SIGEL_RobotCheck::SIG_RobotChecker checker(experiment.robot, experiment.simulationParameter, experiment.environment);
    int count[3] = { 0, 0, 0 };
    for (const SIGEL_RobotCheck::SIG_RobotIssue &issue : checker.check()) {
      count[issue.kind]++;
      const char *check = "";
      switch (issue.check) {
        case SIGEL_RobotCheck::SIG_RobotIssue::tCannotBeSimulated: check = "CannotBeSimulated"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tLimitHoldsDrive:   check = "LimitHoldsDrive"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tDriveStrength:     check = "DriveStrength"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tJointAxis:         check = "JointAxis"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tLinkOverlap:       check = "LinkOverlap"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tJointStart:        check = "JointStart"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tStartHeight:       check = "StartHeight"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tGroundStepSize:    check = "GroundStepSize"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tJointStepSize:     check = "JointStepSize"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tIntegrator:        check = "Integrator"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tLinkMass:          check = "LinkMass"; break;
        case SIGEL_RobotCheck::SIG_RobotIssue::tStanding:          check = "Standing"; break;
      }
      printf("%s\t%s\t%s\t%s\t%s\t%s\n", check,
             issue.kind == SIGEL_RobotCheck::SIG_RobotIssue::tError ? "error" : issue.kind == SIGEL_RobotCheck::SIG_RobotIssue::tWarning ? "warning" : "suggestion", qPrintable(issue.part),
             qPrintable(issue.title), qPrintable(issue.analysis), qPrintable(issue.advice));
    }
    printf("check: errors %d, warnings %d, suggestions %d\n", count[0], count[1], count[2]);
    return 0;
  }

  const int index = (argc == 3) ? atoi(argv[2]) : 0;
  if (index < 0 || index >= experiment.population.getSize()) {
    fprintf(stderr, "individual %d is outside 0..%d\n",
            index, experiment.population.getSize() - 1);
    return 1;
  }

  // -order prints the robot's parts as the experiment file gives them, and
  // again for the copy that the simulation runs on, and evaluates nothing.
  if (order)
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

  if (order) {
    dumpOrder(robot, "copy");
    return 0;
  }

  SIGEL_GP::SIG_GPIndividual &individual = experiment.population.getIndividual(index);
  const QString name = experiment.gpParameter.getFitnessName();

  // Remote ZORC needs a serial line and a QApplication, which this program does not have.
  const std::optional<int> fitnessIndex =
      SIGEL_GP::SIG_GPFitnessFunctionRegistry::indexOf(name);
  const SIGEL_GP::SIG_GPRemoteZORCFitnessFunction remoteZORC;
  if (!fitnessIndex || name == remoteZORC.serializedId()) {
    fprintf(stderr, "%s names %s, which this program cannot evaluate\n",
            argv[1], qPrintable(name));
    return 1;
  }
  const SIGEL_GP::SIG_GPFitnessFunction &fitnessFunction =
      *SIGEL_GP::SIG_GPFitnessFunctionRegistry::fitnessFunctions()[*fitnessIndex];

  const double recorded = individual.getFitness();
  const double fitness = fitnessFunction.evalFitness(
      individual.getProgramVar(), robot,
      experiment.environment, experiment.simulationParameter);

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
    SIG_Vector last;
    for (const SIG_Vector *p : trace.positions) {
      if (p->y < lo) lo = p->y;
      if (p->y > hi) hi = p->y;
      last = *p;
      frames++;
    }
    printf("  frames %d  height %.6g .. %.6g  last (%.6g, %.6g, %.6g)\n",
           frames, lo, hi, last.x, last.y, last.z);
  }

  printf("%s individual %d  %s\n", argv[1], index, qPrintable(name));
  printf("  2003 recorded  %.17g\n", recorded);
  printf("  this run       %.17g\n", fitness);
  return 0;
}
