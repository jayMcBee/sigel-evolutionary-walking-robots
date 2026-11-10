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
#ifndef SIGEL_ENVIRONMENT_TST_SIG_ENVIRONMENT_H
#define SIGEL_ENVIRONMENT_TST_SIG_ENVIRONMENT_H

#include <QByteArray>
#include <QObject>
#include <QTemporaryDir>

namespace SIGEL_Environment
{

  /**
   * The unit tests of SIG_Environment.
   */
  class TST_SIG_Environment : public QObject
  {
    Q_OBJECT

  private slots:

    void newEnvironmentHasAGravityOf9Point81Downwards();

    void newEnvironmentStartsTheRobotAboveTheFloorOnAGroundThatHoldsIt();

    void newEnvironmentHasNoTextureAtFullAlphaAndNoAutosave();

    void newEnvironmentHasAFlatFloor();

    void makeTerrainGivesTheHeightsOfTheFloorFunction();

    void makeTerrainGivesTheHeightsOfThePictureFile();

    void makeTerrainRefusesAFloorFunctionThatCannotBeParsed();

    void makeTerrainRefusesAPictureFileThatCannotBeRead_data();
    void makeTerrainRefusesAPictureFileThatCannotBeRead();

    void getTerrainGivesTheTerrainOfMakeTerrain();

    void getTerrainFollowsAChangeOfTheFloorFunction();

    void getTerrainGivesTheTerrainOfThePictureFile();

    void getTerrainGivesTheTerrainOfSetTerrainWhateverTheFloorSettings();

    void getTerrainGivesNoQuietFlatFloorForAPictureFileThatCannotBeRead_data();
    void getTerrainGivesNoQuietFlatFloorForAPictureFileThatCannotBeRead();

    void getTerrainGivesNoQuietFlatFloorForAFloorFunctionThatCannotBeParsed();

    void writeToFileThenReadFromFileGivesTheSameEnvironment_data();
    void writeToFileThenReadFromFileGivesTheSameEnvironment();

    void writeToFileWritesTheBlockInTheFormOfTheExperimentFiles();

    void readFromFileOfABlockWithoutTheFloorKeysKeepsTheirDefaults();

    void readFromFileReadsEachKeyOfABlockOfTheCurrentForm();

    void readFromFileOfABlockWithoutTheFloorKeysGivesTheirDefaultsToAnEnvironmentInUse();

  private:

    static QString pictureFile( const QTemporaryDir &folder, bool fileExists, const QByteArray &contents );
    static void addUnreadablePictureRows();
  };

}

#endif // SIGEL_ENVIRONMENT_TST_SIG_ENVIRONMENT_H
