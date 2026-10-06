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
    coredrive -selfcheck             rules of small classes
    coredrive -metamating            rules of MetaGP's mating code

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
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPFitnessFunctionRegistry.h"
#include "SIGEL_GP/SIG_GPRemoteZORCFitnessFunction.h"
#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_RobotCheck/SIG_RobotChecker.h"
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
#include "MT_GPSystem/MT_Individual.h"
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Programline.h"
#include "MT_GPSystem/MT_Randomizer.h"
#include "MT_GPSystem/MT_Search.h"

#include <cstdlib>


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

// Rules of small classes that no shipped experiment or robot file reaches.
// fitness-check.sh runs this before the evaluations, and once more under
// LeakSanitizer: every object made here must be freed by its owner.
static int selfcheck()
{
  // Line-buffer stdout. SIG_WANT records and continues, but a wrong size can
  // abort a later index, and a block-buffered stdout then discards the
  // "selfcheck FAILED:" line that says which assertion went. The gate still
  // fails; the operator just cannot see why.
  setvbuf( stdout, 0, _IOLBF, 0 );

  int bad = 0;
#define SIG_WANT(cond)                                                     \
  do { if (!(cond)) { printf("selfcheck FAILED: %s\n", #cond); ++bad; } }  \
  while (0)

  {   // SIG_LanguageParameters owns its commands; removeCommand frees one.
    SIGEL_Robot::SIG_LanguageParameters lp;
    SIGEL_Robot::SIG_CommandParameters *command = new SIGEL_Robot::SIG_CommandParameters();
    lp.addCommand("CHECKED", command);
    SIG_WANT(lp.getCommand("CHECKED") == command);
    lp.removeCommand("CHECKED");
    SIG_WANT(lp.getCommand("CHECKED") == 0);
  }
  {   // A robot file may declare a point of a link twice. The last one wins.
    SIGEL_Robot::SIG_Link link(0, "L", 0);
    link.addPoint("P", SIG_Vector(1, 0, 0));
    link.addPoint("P", SIG_Vector(2, 0, 0));
    SIG_WANT(link.getPoint("P").x == 2);
  }
  {   // SIG_Robot finds each kind of part by name, and owns and frees the parts.
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Body     *body     = new SIGEL_Robot::SIG_Body(&robot, "body", "d");
    SIGEL_Robot::SIG_Material *material = new SIGEL_Robot::SIG_Material(&robot, "material");
    SIGEL_Robot::SIG_Link     *link     = new SIGEL_Robot::SIG_Link(&robot, "link", 0);
    SIGEL_Robot::SIG_Joint    *joint    = new SIGEL_Robot::SIG_GlueJoint(&robot, "joint", 0);
    SIGEL_Robot::SIG_Drive    *drive    = new SIGEL_Robot::SIG_Drive(&robot, "drive", 0);
    SIGEL_Robot::SIG_Sensor   *sensor   = new SIGEL_Robot::SIG_ContactSensor(&robot, "sensor", 0);
    robot.addBody(body);
    robot.addMaterial(material);
    robot.addLink(link);
    robot.addJoint(joint);
    robot.addDrive(drive);
    robot.addSensor(sensor);
    SIG_WANT(robot.lookupBody("body")         == body);
    SIG_WANT(robot.lookupMaterial("material") == material);
    SIG_WANT(robot.lookupLink("link")         == link);
    SIG_WANT(robot.lookupJoint("joint")       == joint);
    SIG_WANT(robot.lookupDrive("drive")       == drive);
    SIG_WANT(robot.lookupSensor("sensor")     == sensor);
    SIG_WANT(robot.lookupLink("MISSING") == 0);
  }
  {   // SIG_Material::setFrictionValue. No shipped robot declares friction,
      // so no other check runs this code.
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

    // The partner is updated too. This is the only check that fails when
    // setFrictionValue updates one side of a pair that already exists.
    SIG_WANT(b.getFrictionValue(&a) == 0.75);

    // The written record shows the length of the list: an update must not
    // append a second entry.
    // "Material a <elasticity> <density> <nfric> b 0.75 c 0.5 <colour>"
    QString written;
    { QTextStream ts(&written); a.writeToFileTransfer(ts); }
    SIG_WANT(written.split(' ').value(4) == "2");   // 3 would be the append bug
    SIG_WANT(written.contains("b 0.75"));
    SIG_WANT(written.contains("c 0.5"));
  }
  {   // SIG_Link::addNoCollide registers the pair on both links, once. No
      // shipped robot declares a no-collide pair.
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Link l1(&robot, "l1", 0);
    SIGEL_Robot::SIG_Link l2(&robot, "l2", 1);

    l1.addNoCollide(&l2);                         // negotiates both directions
    SIG_WANT(l1.getNoCollides().count() == 1);
    SIG_WANT(l2.getNoCollides().count() == 1);
    SIG_WANT(l1.getNoCollides().value(0) == &l2);   // value(), not at():
    SIG_WANT(l2.getNoCollides().value(0) == &l1);   // SIG_WANT continues after
                                                   // a failure, so at() would
                                                   // abort the block instead
                                                   // of reporting the rest.

    l1.addNoCollide(&l2);                         // the duplicate guard
    SIG_WANT(l1.getNoCollides().count() == 1);
    SIG_WANT(l2.getNoCollides().count() == 1);
  }
  {   // A material read from a stream keeps a friction partner that is loaded
      // already, and drops, without a message, one that is not loaded yet.
    SIGEL_Robot::SIG_Robot robot;
    SIGEL_Robot::SIG_Material *known = new SIGEL_Robot::SIG_Material(&robot, "known");
    robot.addMaterial(known);

    QString text = "later 1 1 1 known 0.25 0 0 0 ";
    { QTextStream ts(&text, QIODevice::ReadOnly);
      SIGEL_Robot::SIG_Material *later = new SIGEL_Robot::SIG_Material(&robot, ts);
      robot.addMaterial(later);
      SIG_WANT(later->getFrictionValue(known) == 0.25);   // backward ref kept
      SIG_WANT(known->getFrictionValue(later) == 0.25);   // and negotiated
    }

    QString fwd = "early 1 1 1 notYetLoaded 0.9 0 0 0 ";
    { QTextStream ts(&fwd, QIODevice::ReadOnly);
      SIGEL_Robot::SIG_Material *early = new SIGEL_Robot::SIG_Material(&robot, ts);
      robot.addMaterial(early);
      // 0.6 is the not-found default: the pair was dropped, not stored.
      SIG_WANT(early->getFrictionValue(known) == 0.6);
    }
  }
  {   // SIG_GPPopulation owns its individuals. The run under LeakSanitizer
      // judges every free in setIndividual, deleteIndividual and the
      // destructor; the checks below pin positions and values.
    SIGEL_GP::SIG_GPParameter param;
    SIGEL_Robot::SIG_LanguageParameters langParams;

    SIGEL_GP::SIG_GPPopulation pop;
    pop.addRandomIndividuals( 4, param, langParams );
    SIG_WANT(pop.getSize() == 4);

    // deleteIndividual frees one individual and shifts the rest down.
    SIGEL_GP::SIG_GPIndividual *third = pop.getIndividualPointer( 2 );
    SIGEL_GP::SIG_GPIndividual *last  = pop.getIndividualPointer( 3 );
    pop.deleteIndividual( 1 );
    SIG_WANT(pop.getSize() == 3);
    SIG_WANT(pop.getIndividualPointer( 1 ) == third);
    SIG_WANT(pop.getIndividualPointer( 2 ) == last);
    SIG_WANT(pop.getIndividualPointer( 1 )->getPoolPos() == 1);
    SIG_WANT(pop.getIndividualPointer( 2 )->getPoolPos() == 2);

    pop.deleteIndividual( pop.getSize() - 1 );   // last: the shift loop is empty
    SIG_WANT(pop.getSize() == 2);

    // setIndividual frees the loser and takes ownership of the caller's
    // object. Deliberately not deleted here -- the pool must free it.
    SIGEL_GP::SIG_GPIndividual *winner = new SIGEL_GP::SIG_GPIndividual();
    pop.setIndividual( *winner, 0 );
    SIG_WANT(pop.getIndividualPointer( 0 ) == winner);
    SIG_WANT(pop.getSize() == 2);

    // A new individual has fitness -1 already, so set other values first,
    // on two slots.
    pop.getIndividualPointer( 0 )->setFitness( 3.5 );
    pop.getIndividualPointer( 1 )->setFitness( 7.5 );
    pop.resetAllFitnessValues();
    SIG_WANT(pop.getIndividualPointer( 0 )->getFitness() == -1);
    SIG_WANT(pop.getIndividualPointer( 1 )->getFitness() == -1);

    // The pool is left with one individual, so that the destructor has
    // something to free under LeakSanitizer.
    pop.deleteIndividual( 0 );
    SIG_WANT(pop.getSize() == 1);
  }
#undef SIG_WANT
  printf(bad ? "selfcheck: %d FAILED\n" : "selfcheck: ok\n", bad);
  return bad ? 1 : 0;
}

// ---------------------------------------------------------------------------
// -metamating: the mating code of MetaGP, MT_Search::startMatingProcess with
// its private crossover, mutate and reproduce, and MT_Program::insertProg.
//
// Every rule here must hold for every random seed, so there is no baseline
// file. One operator is forced per run through MT_Randomizer's thresholds: a
// draw is below 1000, so thresholds 1000/1000/1000 always choose the first
// entry, 0/1000/1000 the second and 0/0/1000 the third.
//
// MT_Randomizer seeds rand() from the clock in its constructor. The srand()
// after it makes a failure repeat.

static QStringList metaLines(MT_Program *program)
{
  QStringList lines;
  for (int i = 0; i < program->getLength(); i++) {
    QString text;
    QTextStream stream(&text);
    program->getProgramLine(i)->writeToFileProgramLine(stream);
    lines << text.trimmed();
  }
  return lines;
}

static QString metaRandomizerText(const int searchOperator[3], const int mutationPower[2],
                                  const int crossoverPoints[3], int maxProgramLength)
{
  QString text;
  QTextStream stream(&text);
  // Parents, offspring, registers, maximum program length.
  stream << "Randomizer:\n2\n4\n10\n" << maxProgramLength << "\n";
  for (int i = 0; i < 3; i++) stream << searchOperator[i] << "\n";
  for (int i = 0; i < 2; i++) stream << mutationPower[i] << "\n";
  for (int i = 0; i < 3; i++) stream << crossoverPoints[i] << "\n";
  // The 18 instruction thresholds of stdConf.mt.
  for (int i = 0; i < 18; i++) stream << i * 1000 << "\n";
  stream << "\nConstant:\n3\n1\n2\n3\n\n";
  return text;
}

struct MetaMating
{
  QStringList parentLines[2];       // before the mating
  QStringList parentLinesAfter[2];
  QList<QStringList> childLines;
  QList<int> childGenesis;
  QList<int> childParent;           // 0 or 1, from getFitnessOfParent
  int maxProgramLength = 0;
  bool complete = false;            // every offspring slot is filled, both parents are in it
};

// Two random parents, four offspring slots: the two parents and two children.
static MetaMating metaMate(unsigned seed, const int searchOperator[3], const int mutationPower[2],
                           const int crossoverPoints[3], int startLength, int maxProgramLength)
{
  MetaMating result;
  result.maxProgramLength = maxProgramLength;

  QString text = metaRandomizerText(searchOperator, mutationPower, crossoverPoints, startLength);
  QTextStream stream(&text);
  MT_Randomizer randomizer(stream);
  srand(seed);

  MT_Population parents(&randomizer, 2);
  parents.setMaxProgLen(maxProgramLength);
  MT_Individual *parent[2] = { parents.getIndividual(0), parents.getIndividual(1) };
  for (int i = 0; i < 2; i++) {
    parent[i]->setFitness(i + 1.0);
    result.parentLines[i] = metaLines(parent[i]->getProgram());
  }

  MT_Population offspring;
  offspring.changePopSize(4);
  MT_Search search(&parents, &offspring, &randomizer);
  const int error = search.startMatingProcess();

  int parentsFound = 0;
  bool allFilled = true;
  for (int i = 0; i < offspring.getSize(); i++) {
    MT_Individual *individual = offspring.getIndividual(i);
    if (!individual) { allFilled = false; continue; }
    if (individual == parent[0] || individual == parent[1]) { parentsFound++; continue; }
    result.childLines << metaLines(individual->getProgram());
    result.childGenesis << individual->getTypOfGenesis();
    result.childParent << (individual->getFitnessOfParent() == 1.0 ? 0 : 1);
  }
  for (int i = 0; i < 2; i++)
    result.parentLinesAfter[i] = metaLines(parent[i]->getProgram());
  result.complete = (error == 0) && allFilled && (parentsFound == 2) && (result.childLines.size() == 2);
  return result;
}

static int metamating()
{
  int bad = 0;
#define META_CHECK(cond, what, seed)                                          \
  do { if (!(cond)) { printf("metamating FAILED: %s, seed %u: %s\n", what, seed, #cond); ++bad; } } \
  while (0)

  const unsigned seeds = 200;
  const int startLength = 20;       // random programs get 0.66 of this, 13 lines
  const int always[2] = { 1000, 1000 };
  const int never[2] = { 0, 0 };
  const int onePoint[3] = { 1000, 1000, 1000 };

  // Crossover with 1, 2 and 3 points.
  const int crossoverOnly[3] = { 1000, 1000, 1000 };
  const int points[3][3] = { { 1000, 1000, 1000 }, { 0, 1000, 1000 }, { 0, 0, 1000 } };
  const char *pointNames[3] = { "crossover, 1 point", "crossover, 2 points", "crossover, 3 points" };
  for (int p = 0; p < 3; p++) {
    for (unsigned seed = 1; seed <= seeds; seed++) {
      // Room for every line: the children together hold exactly the parents' lines.
      MetaMating roomy = metaMate(seed, crossoverOnly, never, points[p], startLength, 100);
      META_CHECK(roomy.complete, pointNames[p], seed);
      if (!roomy.complete) continue;
      QStringList fromParents = roomy.parentLines[0] + roomy.parentLines[1];
      QStringList fromChildren = roomy.childLines[0] + roomy.childLines[1];
      fromParents.sort();
      fromChildren.sort();
      META_CHECK(fromChildren == fromParents, pointNames[p], seed);
      META_CHECK(roomy.childGenesis[0] == p + 1 && roomy.childGenesis[1] == p + 1, pointNames[p], seed);
      META_CHECK(roomy.parentLinesAfter[0] == roomy.parentLines[0], pointNames[p], seed);
      META_CHECK(roomy.parentLinesAfter[1] == roomy.parentLines[1], pointNames[p], seed);

      // No room: a child is cut at the maximum length and is never empty.
      MetaMating tight = metaMate(seed, crossoverOnly, never, points[p], startLength, startLength);
      META_CHECK(tight.complete, pointNames[p], seed);
      if (!tight.complete) continue;
      for (const QStringList &child : tight.childLines)
        META_CHECK(child.size() >= 1 && child.size() <= tight.maxProgramLength, pointNames[p], seed);
      META_CHECK(tight.parentLinesAfter[0] == tight.parentLines[0], pointNames[p], seed);
      META_CHECK(tight.parentLinesAfter[1] == tight.parentLines[1], pointNames[p], seed);
    }
    printf("metamating %s: %u seeds\n", pointNames[p], seeds);
  }

  // Mutation.
  const int mutationOnly[3] = { 0, 1000, 1000 };
  int mutatedChildren = 0;
  for (unsigned seed = 1; seed <= seeds; seed++) {
    MetaMating changed = metaMate(seed, mutationOnly, always, onePoint, startLength, startLength);
    META_CHECK(changed.complete, "mutation", seed);
    if (changed.complete)
      for (int c = 0; c < 2; c++) {
        META_CHECK(changed.childLines[c].size() == changed.parentLines[changed.childParent[c]].size(), "mutation", seed);
        META_CHECK(changed.childGenesis[c] >= 100, "mutation", seed);
        if (changed.childLines[c] != changed.parentLines[changed.childParent[c]]) mutatedChildren++;
        META_CHECK(changed.parentLinesAfter[c] == changed.parentLines[c], "mutation", seed);
      }

    MetaMating unchanged = metaMate(seed, mutationOnly, never, onePoint, startLength, startLength);
    META_CHECK(unchanged.complete, "mutation with rate 0", seed);
    if (unchanged.complete)
      for (int c = 0; c < 2; c++) {
        META_CHECK(unchanged.childLines[c] == unchanged.parentLines[unchanged.childParent[c]], "mutation with rate 0", seed);
        META_CHECK(unchanged.childGenesis[c] == 100, "mutation with rate 0", seed);
      }
  }
  // A mutated element can get its old value again, so one child may be equal
  // to its parent. All of them equal means that mutate changes nothing.
  META_CHECK(mutatedChildren > 0, "mutation, all seeds", seeds);
  printf("metamating mutation: %u seeds\n", seeds);

  // Reproduction.
  const int reproductionOnly[3] = { 0, 0, 1000 };
  for (unsigned seed = 1; seed <= seeds; seed++) {
    MetaMating copied = metaMate(seed, reproductionOnly, never, onePoint, startLength, startLength);
    META_CHECK(copied.complete, "reproduction", seed);
    if (copied.complete)
      for (int c = 0; c < 2; c++) {
        META_CHECK(copied.childLines[c] == copied.parentLines[copied.childParent[c]], "reproduction", seed);
        META_CHECK(copied.childGenesis[c] == 4, "reproduction", seed);
        META_CHECK(copied.parentLinesAfter[c] == copied.parentLines[c], "reproduction", seed);
      }
  }
  printf("metamating reproduction: %u seeds\n", seeds);

#undef META_CHECK
  printf(bad ? "metamating: %d FAILED\n" : "metamating: ok\n", bad);
  return bad ? 1 : 0;
}

int main(int argc, char *argv[])
{
  // No result may depend on Qt's random hash seed.
  QHashSeed::setDeterministicGlobalSeed();

  bool verbose = false;
  if (argc > 1 && QString(argv[1]) == "-v") { verbose = true; argv++; argc--; }
  if (argc > 1 && QString(argv[1]) == "-selfcheck") return selfcheck();
  if (argc > 1 && QString(argv[1]) == "-metamating") return metamating();
  bool robotCheck = false;
  if (argc > 1 && QString(argv[1]) == "-check") { robotCheck = true; argv++; argc--; }
  bool order = false;
  if (argc > 1 && QString(argv[1]) == "-order") { order = true; argv++; argc--; }

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "usage: %s [-v] [-check] <experiment.exp> [individual, default 0]\n       %s -order <experiment.exp | robot.rrb>\n       %s -selfcheck | -metamating\n", argv[0], argv[0], argv[0]);
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
