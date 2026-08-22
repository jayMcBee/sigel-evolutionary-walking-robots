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
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Simulation/SIG_Simulation.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Tools/SIG_Exception.h"

using SIGEL_Simulation::SIG_SimulationParameters;

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

  SIGEL_Robot::SIG_Robot robot(experiment.robot);
  try {
    robot.prepareDynaMechs();
  }
  catch (SIGEL_Tools::SIG_Exception &e) {
    fprintf(stderr, "preparing the robot: %s\n", qPrintable(e.getMessage()));
    return 1;
  }

  if (verbose) {
    // The robot as the simulation sees it: Q2Dict iteration order is what
    // numbers the DynaMechs links -- PORTING.md §9, §10.
    Q2DictIterator<SIGEL_Robot::SIG_Link> li = robot.getLinkIter();
    int n = 0;
    for (; li.current(); ++li)
      printf("  link  %2d  %s\n", n++, qPrintable(li.currentKey()));
    Q2DictIterator<SIGEL_Robot::SIG_Joint> ji = robot.getJointIter();
    n = 0;
    for (; ji.current(); ++ji)
      printf("  joint %2d  %s\n", n++, qPrintable(ji.currentKey()));
  }

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
