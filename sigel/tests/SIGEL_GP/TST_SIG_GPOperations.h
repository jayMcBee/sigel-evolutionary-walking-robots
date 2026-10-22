/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef SIGEL_GP_TST_SIG_GPOPERATIONS_H
#define SIGEL_GP_TST_SIG_GPOPERATIONS_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

namespace SIGEL_Program
{
  class SIG_Program;
}

namespace SIGEL_Tools
{
  class SIG_Randomizer;
}

namespace SIGEL_GP
{

  class SIG_GPIndividual;

  /**
   * The unit tests of SIG_GPOperations and SIG_Program::checkLength.
   *
   * The operators take their random numbers from a seeded SIG_Randomizer. Each test runs
   * the seeds 1 to seeds and checks rules that hold for every seed, not exact programs: one
   * more randomizer call in an operator would change every child.
   */
  class TST_SIG_GPOperations : public QObject
  {
    Q_OBJECT

  private slots:

    void mutationKeepsTheLengthInsideTheLimits();

    void mutationChangesAddsOrDeletesOneLine();

    void mutationChangesAddsAndDeletesALine();

    void mutationChangesAnOperandOrTheInstruction();

    void mutationAtTheMaximumDoesNotAddALine();

    void mutationAtTheMinimumDoesNotDeleteALine();

    void mutationWithEqualLimitsOnlyChangesALine();

    void mutationBringsAParentOutsideTheLimitsInside();

    void mutationLeavesTheParentAlone();

    void mutationGivesTheNameAndPoolPositionAndRecordsTheFather();

    void crossoverJoinsRunsOfTheParents();

    void crossoverCutsALongChildAtTheEnd();

    void crossoverUsesBothVariants();

    void crossoverKeepsTheChildrenInsideTheLimits();

    void crossoverLeavesTheParentsAlone();

    void crossoverGivesTheNamesAndPoolPositionsAndRecordsBothParents();

    void reproductionCopiesTheParent();

    void reproductionPadsAShortParentWithNops();

    void reproductionCutsALongParentAtTheEnd();

    void checkLengthLeavesAProgramInsideTheLimits();

    void checkLengthPadsAShortProgramWithNops();

    void checkLengthCutsALongProgramAtTheEnd();

    void checkLengthWithMaximumZeroHasNoMaximum();

  private:

    static constexpr int seeds = 100;

    /** A parent of length lines, named "father", with fitness 0.75 and pool position 1. */
    std::unique_ptr<SIG_GPIndividual> parent( SIGEL_Tools::SIG_Randomizer &randomizer, long length, QString const &name = "father" );

    /** A random program of length lines. */
    SIGEL_Program::SIG_Program program( SIGEL_Tools::SIG_Randomizer &randomizer, long length );

    QStringList lineTexts( SIGEL_Program::SIG_Program const &program );

    /**
     * True if child is first[0, a) + second[b, c) + first[d, end) with 1 <= a <= d, which covers
     * the one-point and the two-point crossover. With mayBeCut, child may also be the start
     * of such a program.
     */
    bool joinsRunsOfTheParents( QStringList const &child, QStringList const &first, QStringList const &second, bool mayBeCut );

    QString nopText();

    /**
     * How child differs from parent: "unchanged", "changed" if one line differs, "added" if
     * child is parent with one line more, "deleted" if one line less, else empty.
     */
    QString mutationKind( QStringList const &parent, QStringList const &child );
  };

}

#endif // SIGEL_GP_TST_SIG_GPOPERATIONS_H
