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
#ifndef SIGEL_GP_SIG_GPPOPULATIONTEST_H
#define SIGEL_GP_SIG_GPPOPULATIONTEST_H

#include <QObject>
#include <QString>

namespace SIGEL_GP
{

  class SIG_GPPopulation;

  /**
   * The unit tests of SIG_GPPopulation.
   */
  class SIG_GPPopulationTest : public QObject
  {
    Q_OBJECT

  private slots:

    void defaultConstructorGivesAnEmptyPopulation();

    void getNextIdentifierCountsUp();

    void addRandomIndividualsGivesNamesAndPoolPositions();

    void addRandomIndividualsKeepsTheIndividualsThatAreThere();

    void addRandomIndividualsTakesTheNamesFromTheIdentifier();

    void addRandomIndividualsUsesTheLengthLimits();

    void programLengthsAreInsideTheLimits();
    void programLengthsReachBothLimits();

    void sameSeedGivesTheSameIndividuals();

    void differentSeedsGiveDifferentIndividuals();

    void getIndividualGivesTheIndividualAtThePosition();

    void deleteIndividualMovesTheRestDown();

    void deleteIndividualOfTheLastLeavesTheOthers();

    void deleteIndividualDownToAnEmptyPopulation();

    void setIndividualReplacesOneIndividual();

    void resetAllFitnessValuesSetsMinusOne();

    void bestWorstAndAverageUseOnlySimulatedFitnessValues();

    void bestWorstAndAverageDoNotDependOnThePositionWithoutFitness();

    void fitnessZeroCountsAsAFitness();

    void bestWorstAndAverageAreZeroWithoutAnyFitness();

    void writtenTextIsReadBack();

    void writtenTextWithoutHistoryHasNoHistoryBlock();

    void individualsWithHistoryAreReadBack();

    void textWithoutHeaderIsRead();

    void emptyPopulationIsReadBack();

    void savePoolWritesTheTextOfWriteToFile();

    void readFromFileReplacesTheIndividualsThatAreThere();

  private:

    // Sets the seed first: without it the programs depend on the time of day.
    int addIndividuals( SIG_GPPopulation &population, int quantity, int seed = 1 );

    QString writtenText( SIG_GPPopulation &population );
  };

}

#endif // SIGEL_GP_SIG_GPPOPULATIONTEST_H
