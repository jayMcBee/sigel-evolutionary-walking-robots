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
#include <QList>
#include <QString>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <QFile>
#include <QHashSeed>
#include <QTextStream>


#include "SIGEL_GP/SIG_GPExperiment.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
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
  // Line-buffer stdout. SIG_WANT records and continues, but a wrong size can
  // abort a later index, and a block-buffered stdout then discards the
  // "selfcheck FAILED:" line that says which assertion went. The gate still
  // fails; the operator just cannot see why.
  setvbuf( stdout, 0, _IOLBF, 0 );

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

    // The partner must be updated too, not left at 0.25. setFrictionValue's
    // negotiate call sits OUTSIDE its "if (!found)" block for this reason.
    // Move it inside -- which reads like a tidy -- and a keeps 0.75 while b
    // keeps 0.25. Confirmed against the 1.3 binary: verification-against-sigel-1.3/v7.
    //
    // This is the ONLY assertion here that can see that failure. The count
    // and the name both still pass, because the entry is present, correctly
    // named and symmetric -- only its value is wrong.
    SIG_WANT(b.getFrictionValue(&a) == 0.75);

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
    SIG_WANT(l1.getNoCollides().value(0) == &l2);   // value(), not at():
    SIG_WANT(l2.getNoCollides().value(0) == &l1);   // SIG_WANT continues after
                                                   // a failure, so at() would
                                                   // abort the block instead
                                                   // of reporting the rest.

    l1.addNoCollide(&l2);                         // the duplicate guard
    SIG_WANT(l1.getNoCollides().count() == 1);
    SIG_WANT(l2.getNoCollides().count() == 1);
  }
  {   // The parser DROPS a friction or no-collide partner that is not loaded
      // yet, silently. Confirmed against the 1.3 binary (verification-against-sigel-1.3/v7): it is
      // upstream behaviour, not ours, and nothing warns. Both parsers register
      // the object only AFTER constructing it, so a name can only refer
      // backwards. No shipped file exercises this -- none declares either.
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
  {   // THE EVOLUTION LOOP'S OWNING CONTAINER, D15.
      //
      // Six of SIG_GPPopulation's nine frees are in functions no gate enters:
      // setIndividual (12 tournament call sites) and deleteIndividual.
      // addRandomIndividuals is driven for its allocations, not its frees --
      // its delete pool[x] is always delete nullptr, because resize() just
      // created those slots. D15 shipped a real leak in
      // readFromFile's cancel path and every gate reported it clean -- both
      // diffs, the sanitized run, the self-check and the 41,254-byte leak
      // baseline. That baseline comes from the default constructor,
      // readFromFile on an EMPTY pool, and the destructor, so nothing here
      // moves it. fitness-check.sh runs the self-check a second time under
      // LeakSanitizer, and that is what judges these blocks.
      //
      // Built on the DEFAULT constructor. The two sized ones are unused and
      // both defective -- see PORTING.md section 9.
    SIGEL_GP::SIG_GPParameter param;
    SIGEL_Robot::SIG_LanguageParameters langParams;

    SIGEL_GP::SIG_GPPopulation pop;
    pop.addRandomIndividuals( 4, param, langParams );
    SIG_WANT(pop.getSize() == 4);

    // deleteIndividual frees the victim and shifts the rest down. The free
    // is invisible to any assertion, which is why the leak-checked run is
    // the real judge; these pin the shift, which is what a careless rewrite
    // breaks.
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

    // Every SIG_GPIndividual constructor already sets fitness to -1, so
    // asserting -1 on a fresh pool cannot fail. Move one off it first, and
    // check BOTH slots -- a resetPool that reset only the first would pass.
    pop.getIndividualPointer( 0 )->setFitness( 3.5 );
    pop.getIndividualPointer( 1 )->setFitness( 7.5 );
    pop.resetPool();
    SIG_WANT(pop.getIndividualPointer( 0 )->getFitness() == -1);
    SIG_WANT(pop.getIndividualPointer( 1 )->getFitness() == -1);

    // Leave the pool NON-EMPTY. Draining it made ~SIG_GPPopulation's
    // qDeleteAll run on an empty list, so removing that free -- the largest
    // in the class, 4,398,620 bytes per evaluation -- was invisible to every
    // gate including this one. Found by review of D16 itself.
    pop.deleteIndividual( 0 );
    SIG_WANT(pop.getSize() == 1);
  }
  // ---------------------------------------------------------------------
  // Qt2CursorList -- a model of Qt 2's QList cursor, for the test below.
  //
  // The test compares D18's rewritten cursor walk against the semantics it
  // replaced. Those semantics used to come from Q2PtrList, but the shim is
  // being deleted, so the reference has to outlive it.
  //
  // Written from the vendored Qt 2.3 source, not from the shim:
  //   first/next/last/current   qglist.cpp:203-260, qglist.h:186-196
  //   remove()                  qglist.cpp:504-516 -- removes CURRENT
  //   the cursor after removal  qglist.cpp:436-473 -- unlink() sets
  //                             curNode = n->next, or n->prev with
  //                             curIndex-- when the last node goes
  //   autoDelete                qlist.h:100 -- deleteItem frees if del_item
  //
  // Backed by std::vector on purpose: it shares no implementation with
  // either QList or the shim, so agreement means agreement.
  // ---------------------------------------------------------------------
  struct Qt2CursorList
  {
    std::vector< int * > v;
    long                 cur = -1;
    bool                 del = false;

    ~Qt2CursorList() { if (del) for (int *p : v) delete p; }

    void setAutoDelete(bool e) { del = e; }
    unsigned count() const     { return unsigned(v.size()); }
    bool valid() const         { return cur >= 0 && cur < long(v.size()); }

    void append(int *p) { v.push_back(p); cur = long(v.size()) - 1; }

    int *current() const { return valid() ? v[size_t(cur)] : nullptr; }
    int *first()   { cur = v.empty() ? -1 : 0;             return current(); }
    int *last()    { cur = long(v.size()) - 1;             return current(); }

    int *next()
    {
      if (cur < 0) return nullptr;                 // a dead cursor stays dead
      if (++cur >= long(v.size())) { cur = -1; return nullptr; }
      return v[size_t(cur)];
    }

    bool remove()                                  // removes the CURRENT item
    {
      if (!valid()) return false;
      const long i = cur;
      if (del) delete v[size_t(i)];
      v.erase(v.begin() + i);
      if (i < long(v.size()))        cur = i;      // whatever slid in
      else if (!v.empty())           cur = long(v.size()) - 1;   // stepped back
      else                           cur = -1;
      return true;
    }
  };

  {   // THE CURSOR WALK, D18 -- checked against the shim it replaced.
      //
      // SIG_GPFitnessTrainer::sweepToSpawn walked toSpawnList with
      // Q2PtrList's internal cursor and removed the CURRENT element while
      // iterating. D18 rewrote that as an explicit index. Nothing executes
      // sweepToSpawn -- it needs a live PVM spawn -- so the rewrite would
      // otherwise ship unverified.
      //
      // Run the same sequence against the rewrite and against Qt2CursorList
      // above -- a model of the Qt 2 semantics taken from the vendored
      // source -- and require they agree at every step. This used to compare
      // against the shim; the model replaces it so the check outlives the
      // shim's deletion.
    const int script[] = { 0, 1, 1, 0, 0, 1, 0, 1, 1, 1 };   // 1 = remove
    Qt2CursorList  shim;
    QList<int *>   mine;
    for (int i = 0; i < 6; i++) { shim.append(new int(i)); mine.append(new int(i)); }
    shim.setAutoDelete(true);

    int *sJob = shim.first();
    qsizetype cur = mine.isEmpty() ? -1 : 0;
    int *mJob = (cur < 0) ? 0 : mine.at(cur);

    for (int step = 0; step < 10; step++) {
      // the values must match, not just the null-ness
      SIG_WANT((sJob == 0) == (mJob == 0));
      if (!sJob || !mJob) break;
      SIG_WANT(*sJob == *mJob);
      SIG_WANT(shim.count() == uint(mine.size()));

      if (script[step]) {                       // the success branch
        shim.remove();
        sJob = shim.current();

        delete mine.takeAt(cur);
        if (cur >= mine.size())
          cur = mine.isEmpty() ? -1 : mine.size() - 1;
        mJob = (cur < 0) ? 0 : mine.at(cur);
      } else {                                  // the failure branch
        sJob = shim.next();

        if (cur < 0 || ++cur >= mine.size()) { cur = -1; mJob = 0; }
        else mJob = mine.at(cur);
      }
    }
    SIG_WANT(shim.count() == uint(mine.size()));

    // Drain a short list to EMPTY. The script above breaks at step 7 on a
    // null cursor, so its last three removals never run and the list is
    // never emptied -- which is the one case D18's own prose singles out,
    // "dies if the list emptied". Found by review. Two elements, both
    // removed, so cursorAfterRemoval takes its cur = -1 branch.
    Qt2CursorList  dshim;
    QList<int *>   dmine;
    for (int i = 0; i < 2; i++) { dshim.append(new int(i)); dmine.append(new int(i)); }
    dshim.setAutoDelete(true);

    int *dS = dshim.first();
    qsizetype dcur = dmine.isEmpty() ? -1 : 0;
    int *dM = (dcur < 0) ? 0 : dmine.at(dcur);

    for (int k = 0; k < 3; k++) {
      SIG_WANT((dS == 0) == (dM == 0));
      if (!dS || !dM) break;
      SIG_WANT(*dS == *dM);
      dshim.remove();
      dS = dshim.current();
      delete dmine.takeAt(dcur);
      if (dcur >= dmine.size())
        dcur = dmine.isEmpty() ? -1 : dmine.size() - 1;
      dM = (dcur < 0) ? 0 : dmine.at(dcur);
    }
    SIG_WANT(dshim.count() == 0);
    SIG_WANT(dmine.isEmpty());
    SIG_WANT(dcur == -1);

    // Drive the cursor off the end and keep going. Qt 2's next() leaves a
    // DEAD cursor dead and does not advance it, so a second call must also
    // give null rather than wrapping or walking off. The loop above exits at
    // the first null and never reaches this, which is the same shape of gap
    // the D16 review found -- so it is exercised deliberately here.
    shim.last();
    cur = mine.isEmpty() ? -1 : mine.size() - 1;
    for (int k = 0; k < 3; k++) {
      int *sN = shim.next();
      int *mN;
      if (cur < 0 || ++cur >= mine.size()) { cur = -1; mN = 0; }
      else mN = mine.at(cur);
      SIG_WANT((sN == 0) == (mN == 0));
      if (sN && mN) SIG_WANT(*sN == *mN);
    }
    qDeleteAll(mine);
  }
#undef SIG_WANT
  printf(bad ? "selfcheck: %d FAILED\n" : "selfcheck: ok\n", bad);
  return bad ? 1 : 0;
}

int main(int argc, char *argv[])
{
  // PORTING.md §9. The deleted shim called this during static initialisation;
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
    for (const DL_vector *p : trace.positions) {
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
