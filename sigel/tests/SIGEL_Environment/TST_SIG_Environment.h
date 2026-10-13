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

class dmEnvironment;

namespace SIGEL_Environment
{

  class SIG_Environment;

  /**
   * The unit tests of SIG_Environment.
   */
  class TST_SIG_Environment : public QObject
  {
    Q_OBJECT

  private slots:

    void loadTerrainGivesTheHeightsOfTheFloorFunction();

    void loadTerrainGivesTheHeightsOfThePictureFile();

    void loadTerrainGivesTheDimensionsOfTheFloor();

    void anotherEnvironmentDoesNotChangeTheTerrain();

    void eachLoadHasATerrainFileOfItsOwnAndLeavesNoneBehind();

    void floorFunctionThatCannotBeParsedGivesAFlatFloor();

    void pictureFileThatDoesNotExistGivesAFlatFloor();

    void loadTerrainNeedsNoSigelRoot();

    void loadTerrainUsesTheSettingsAtTheCall();

    void readFromFileGivesTheTerrainOfTheFile();

    void loadTerrainGivesHeightsWithSixSignificantDigits();

    void loadTerrainGivesEachHeightOfASymmetricFloorFunction();

    void loadTerrainGivesTheDimensionsOfAFloorThatIsNotSquare();

    void floorFunctionThatCannotBeParsedGivesAFlatFloorThatIsNotSquare();

    void groundElevationIsInterpolatedInTrianglesAndKeepsTheEdgeHeightPastTheFloor();

    void pictureFileWithAnotherMagicKeyGivesAFlatFloor();

    void emptyPictureFileGivesAFlatFloor();

    void commentsInThePictureFileAreSkipped();

    void pictureFileNeedsNoFinalNewline();

  private:

    /** The height that DynaMechs holds at one grid point after loadTerrain. */
    double loadedHeight( const SIG_Environment &environment, int first, int second ) const;

    /** Loads the terrain of a picture file with this text into target; false if the file cannot be written. The floor settings are 3 by 5. */
    bool loadPictureTerrain( const QByteArray &picture, dmEnvironment &target ) const;

    /** Fails the test unless the terrain has these dimensions and this height at each grid point. */
    void verifyFloorOfOneHeight( dmEnvironment &dynaMechsEnvironment, int firstDimension, int secondDimension, double height ) const;
  };

}

#endif // SIGEL_ENVIRONMENT_TST_SIG_ENVIRONMENT_H
