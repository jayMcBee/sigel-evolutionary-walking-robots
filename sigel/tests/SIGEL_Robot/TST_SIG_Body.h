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
#ifndef SIGEL_ROBOT_TST_SIG_BODY_H
#define SIGEL_ROBOT_TST_SIG_BODY_H

#include "SIGEL_Tools/SIG_Vector.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTextStream>

#include <memory>

namespace SIGEL_Robot
{

  class SIG_Body;
  class SIG_Geometry;
  class SIG_Robot;

  /**
   * The unit tests of SIG_Body.
   */
  class TST_SIG_Body : public QObject
  {
    Q_OBJECT

  private slots:

    void constructorStoresTheNameAndLeavesTheBodyWithoutGeometry();
    void bodyCannotBeCopyConstructed();
    void bodyCannotBeCopyAssigned();
    void getGeometryFileGivesTheRelativeNameInTheDirectory();
    void getGeometryFileReturnsAnAbsoluteNameWithoutTheDirectory();
    void setGeometryGivesTheBodyThatGeometry();
    void setGeometryFreesTheGeometryItReplaces();
    void destructorFreesTheGeometry();
    void setGeometryOfTheGeometryItHoldsKeepsIt();
    void writeToFileTransferWritesNForABodyWithoutGeometry();
    void writeToFileTransferWritesYAndThenTheGeometry();
    void unstreamingConstructorReadsWhatWriteToFileTransferWroteAndStopsAfterIt();
    void unstreamingConstructorReadsABodyWithoutGeometryAndStopsAfterIt();
    void unstreamingConstructorThrowsForAFlagOtherThanYOrN();
    void unstreamingConstructorThrowsForAYFollowedByAMalformedGeometry();
    void unstreamingConstructorThrowsForAnEndedStream();
    void writeToFileTransferQuotesADirectoryWithWhitespace();
    void unstreamingConstructorReadsADirectoryWithWhitespaceAndStopsAfterTheBody();
    void writeToFileTransferQuotesAGeometryFileNameWithWhitespace();
    void unstreamingConstructorReadsAGeometryFileNameWithWhitespaceAndStopsAfterTheBody();

    void loadReadsTheVerticesAndFacesOfAnIndexedFaceSet();
    void loadReadsFacesOfMoreThanThreeVertices();
    void loadBuildsTheFacesFromTheCoordIndexOfAShapeWithAppearanceNormalsAndTextureCoordinates();
    void loadMergesEqualPointsIntoOneVertex();
    void loadRefusesANegativeIndexOtherThanMinusOne();
    void loadRefusesAnIndexBeyondTheCoordinates();
    void loadRefusesAFaceSetWithIndicesButWithoutCoordinates();
    void loadRefusesAFaceOfFewerThanThreeVertices_data();
    void loadRefusesAFaceOfFewerThanThreeVertices();
    void loadEndsAFaceAtTheEndOfItsShape();
    void loadAddsTheFacesOfTheShapesInFileOrderEachOverItsOwnCoordinates();
    void loadAddsNoFacesForShapesThatHaveNoFaces();
    void loadMakesTheFacesOfABox();
    void loadMakesFacesOnTheSurfaceOfACurvedShape_data();
    void loadMakesFacesOnTheSurfaceOfACurvedShape();
    void loadWarnsThatASphereOrConeIsNotRead_data();
    void loadWarnsThatASphereOrConeIsNotRead();
    void loadSkipsNodesOtherThanShapesAndGroupsAndReadsTheShapesAfterThem();
    void loadRefusesAFileWithoutFaces_data();
    void loadRefusesAFileWithoutFaces();
    void loadWalksIntoNestedGroups();
    void loadReadsTheFacesInEveryGroupingNode_data();
    void loadReadsTheFacesInEveryGroupingNode();
    void loadAppliesTheTransformTranslationToEachOfItsChildren();
    void loadAppliesATransformOnlyWithinItsOwnChildren();
    void loadAppliesTheTransformScaleToTheVertices();
    void loadAppliesTheTransformRotationByTheRightHandRule();
    void loadScalesBeforeRotating();
    void loadScalesAboutTheTransformCenter();
    void loadRotatesAboutTheTransformCenterBeforeTranslating();
    void loadScalesAlongTheTransformScaleOrientation();
    void loadRotatesAfterScalingAlongTheScaleOrientation();
    void loadScalesAlongTheScaleOrientationAboutTheTransformCenter();
    void loadScalesAlongAnEighthTurnedScaleOrientation();
    void loadAppliesNestedTransformsInnermostFirst();
    void loadAppliesATransformToTheShapesInGroupsWithinIt();
    void loadTransformsEveryVertexOfAShape();
    void loadKeepsTheFacesFacingOutwards_data();
    void loadKeepsTheFacesFacingOutwards();
    void loadDoesNothingWhenTheBodyHasAGeometry();
    void loadReadsTheFileRelativeToTheDirectory();
    void loadReadsAFileWithANonAsciiName();
    void loadRefusesAMissingFile();
    void loadRefusesAFileThatIsNotVRML();
    void loadRefusesAFileWithASyntaxErrorAfterValidShapes();
    void readVRMLNodeAppliesTheGivenTransformation();

    void loadReadsTheFaceSetsOfAFileWithUse();
    void loadReadsAShapeWhoseGeometryIsAUse();
    void loadReadsAUseDirectlyAfterANodeEndAsACopyBesideThatNode();

  private:

    static QString faceSetShape( const QString &points, const QString &indices );
    static QString triangleShape();
    static QString cornerShape( double x, double y, double z );
    static QString transformed( const QString &fields, const QString &children );
    static QString writeFile( const QTemporaryDir &folder, const QString &contents, const QString &name = "body.wrl" );
    static QString writeVRMLFile( const QTemporaryDir &folder, const QString &body, const QString &name = "body.wrl" );
    static QString thrownByLoad( SIGEL_Robot::SIG_Body &body );
    static std::unique_ptr< SIGEL_Robot::SIG_Body > unstreamedBody( SIGEL_Robot::SIG_Robot &robot, QTextStream &in, QString &thrown );
    static QString useDirectlyAfterTransformEnd();
    static bool isNear( SIG_Vector v, double x, double y, double z );
    static QString toString( SIG_Vector v );
    static QString toString( const QList< SIG_Vector > &vertices );
    static QList< SIG_Vector > verticesOf( const SIGEL_Robot::SIG_Geometry *geometry );
    static QList< QList< SIG_Vector > > loadedFaces( const QString &body );
    static QString toString( const QList< QList< SIG_Vector > > &faces );
    static bool isTheFirstCornerOfTheOnlyFaceNear( const QList< QList< SIG_Vector > > &faces, double x, double y, double z );
    static bool hasAFaceThatStartsNear( const QList< QList< SIG_Vector > > &faces, double x, double y, double z );
    static QList< SIG_Vector > withoutTheBesideShape( const QList< SIG_Vector > &vertices );
    static bool isOnTheSurface( SIG_Vector v );
    static double surfaceArea( const SIGEL_Robot::SIG_Geometry *geometry );
    static QList< QStringList > facePoints( const SIGEL_Robot::SIG_Geometry *geometry );
    static QList< QList< int > > faceIndices( const SIGEL_Robot::SIG_Geometry *geometry );
  };

}

#endif // SIGEL_ROBOT_TST_SIG_BODY_H
