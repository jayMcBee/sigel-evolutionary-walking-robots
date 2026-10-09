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
#ifndef MT_GPSYSTEM_MT_SEARCHTEST_H
#define MT_GPSYSTEM_MT_SEARCHTEST_H

#include <QList>
#include <QObject>
#include <QStringList>

class MT_Program;

/**
 * What one mating of two parents gives.
 */
struct MT_MatingResult
{
  QStringList parentLines[2];
  QStringList parentLinesAfter[2];
  QList<QStringList> childLines;
  QList<int> childGenesis;
  // 0 or 1
  QList<int> childParent;
  int maxProgramLength = 0;
  // Each offspring place is filled, and both parents are among the offspring.
  bool complete = false;
};

/**
 * The unit tests of MT_Search: startMatingProcess with its crossover, mutation
 * and reproduction.
 *
 * Each rule holds for each random seed. The thresholds of MT_Randomizer select
 * one operator for a mating: a draw is below 1000, so the thresholds
 * 1000/1000/1000 always select the first entry, 0/1000/1000 the second and
 * 0/0/1000 the third.
 */
class MT_SearchTest : public QObject
{
  Q_OBJECT

private slots:

  void crossoverKeepsTheLinesOfTheParents_data();

  void crossoverKeepsTheLinesOfTheParents();

  void crossoverWithNoRoomCutsTheChildren_data();

  void crossoverWithNoRoomCutsTheChildren();

  void mutationKeepsTheLengthOfTheParent();

  void mutationWithRateZeroCopiesTheParent();

  void reproductionCopiesTheParent();

private:

  static constexpr unsigned seeds = 200;

  // A random program gets 0.66 of this, 13 lines.
  static constexpr int startLength = 20;

  static constexpr int always[2] = { 1000, 1000 };
  static constexpr int never[2] = { 0, 0 };
  static constexpr int onePoint[3] = { 1000, 1000, 1000 };
  static constexpr int crossoverOnly[3] = { 1000, 1000, 1000 };
  static constexpr int mutationOnly[3] = { 0, 1000, 1000 };
  static constexpr int reproductionOnly[3] = { 0, 0, 1000 };

  void addCrossoverPointRows();

  QStringList programLines( MT_Program *program );

  QString randomizerText( const int searchOperator[3], const int mutationPower[2],
                          const int crossoverPoints[3], int maxProgramLength );

  // Two random parents and four offspring places: the two parents and two children.
  MT_MatingResult mate( unsigned seed, const int searchOperator[3], const int mutationPower[2],
                        const int crossoverPoints[3], int startLength, int maxProgramLength );
};

#endif // MT_GPSYSTEM_MT_SEARCHTEST_H
