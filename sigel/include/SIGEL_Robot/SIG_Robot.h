/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

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
#ifndef SIGEL_ROBOT_SIG_ROBOT_H
#define SIGEL_ROBOT_SIG_ROBOT_H

namespace SIGEL_Robot { class SIG_Robot; }

#include <QList>
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_Sensor.h"
#include <qtextstream.h>

/**
 * In this namespace all classes involved in the modelling
 * of robots can be found.
 */
namespace SIGEL_Robot
{

  /**
   * SIG_Robot is the root class of the robot model.
   *
   * Having access to the SIG_Robot object one is
   * able to obtain any information necessary to work
   * with the robot model.
   *
   * Note that this is a static model. No values will
   * changed during the simulation. Therefore, a copy
   * consisting of DynaMechs objects, the dynamic
   * model, has to be made.
   */
  class SIG_Robot {
  private:
    SIG_LanguageParameters *language;
    SIG_Link *rootlink;
    // The order is the file's; see SIG_LanguageParameters.
    QList<SIG_Body *> bodies;
    QList<SIG_Material *> materials;
    QList<SIG_Link *> links;
    QList<SIG_Joint *> joints;
    QList<SIG_Drive *> drives;
    QList<SIG_Sensor *> sensors;

  public:
    /**
     * Standard constructor.
     *
     * The standard constructor creates an empty robot model with
     * no links and joints.
     */
    SIG_Robot ();
    /**
     * File or network constructor.
     *
     * This constructor reads the whole robot from a stream, which
     * may e.g. be linked to a file or to a network device. The
     * information stored to and read from the stream includes
     * the surface description.
     * @param tx The stream with the position set immediately
     *           in front of the robot information. Should not
     *           be write-only.
     */
    SIG_Robot (QTextStream & tx);
    /**
     * copy constructor. internally, it works via text streams.
     */
    SIG_Robot (const SIG_Robot & rob);
    /**
     * Destructor.
     */
    ~SIG_Robot ();

    SIG_Vector initialLocation;
    SIG_Matrix initialOrientation;

    /**
     * Makes this a fresh and empty robot object.
     */
    void clear ();
    /**
     * Adds the geometry information.
     *
     * This adds a geometry information object of class SIG_Body. It is
     * stored under its DXF filename.
     */
    void addBody (SIG_Body *b);
    /**
     * Adds a SIG_Material object.
     */
    void addMaterial (SIG_Material *m);
    /**
     * Adds a SIG_Link object.
     */
    void addLink (SIG_Link *l);
    /**
     * Adds a SIG_Joint object (or one of its descendants).
     */
    void addJoint (SIG_Joint *j);
    /**
     * Adds a SIG_Drive object.
     */
    void addDrive (SIG_Drive *d);
    /**
     * Adds a SIG_Sensor object (or one of its descendants).
     */
    void addSensor (SIG_Sensor *s);

    /**
     * Sets a new language parameters object. If delprev
     * is set to zero, the previous one will not be deleted.
     */
    void setLangParam (SIG_LanguageParameters *languageParameters, int delprev = 1);
    /**
     * Returns the language parameter object.
     */
    SIG_LanguageParameters *getLangParam () const;
    /**
     * Returns a SIG_Body object.
     *
     * @param n This is DXF FILENAME of the body that is to be returned.
     */
    SIG_Body *lookupBody (QString n) const;
    /**
     * Returns a SIG_Material object.
     *
     * @param n This is SYMBOLIC NAME of the material that is to be returned.
     */
    SIG_Material *lookupMaterial (QString n) const;
    /**
     * Returns a SIG_Link object.
     *
     * @param n This is SYMBOLIC NAME of the link that is to be returned.
     */
    SIG_Link *lookupLink (QString n) const;
    /**
     * Returns a SIG_Joint object.
     *
     * @param n This is SYMBOLIC NAME of the joint that is to be returned.
     */
    SIG_Joint *lookupJoint (QString n) const;
    /**
     * Returns a SIG_Drive object.
     *
     * @param n This is SYMBOLIC NAME of the drive that is to be returned.
     */
    SIG_Drive *lookupDrive (QString n) const;
    /**
     * Returns a SIG_Sensor object.
     *
     * @param n This is SYMBOLIC NAME of the sensor that is to be returned.
     */
    SIG_Sensor *lookupSensor (QString n) const;

    /**
     * Returns, in order, all SIG_Body objects.
     */
    const QList<SIG_Body *> &getBodies () const;
    /**
     * Returns, in order, all SIG_Material objects.
     */
    const QList<SIG_Material *> &getMaterials () const;
    /**
     * Returns, in order, all SIG_Link objects.
     */
    const QList<SIG_Link *> &getLinks () const;
    /**
     * Returns, in order, all SIG_Joint objects.
     */
    const QList<SIG_Joint *> &getJoints () const;
    /**
     * Returns, in order, all SIG_Drive objects.
     */
    const QList<SIG_Drive *> &getDrives () const;
    /**
     * Returns, in order, all SIG_Sensor objects.
     */
    const QList<SIG_Sensor *> &getSensors () const;
    /**
     * Returns the root link.
     *
     * The root link is also contained within the
     * dictionary of links.
     */
    SIG_Link const *getRootLink () const;
    /**
     * Returns the number of points that are annotated
     * to links by the user.
     */
    int getNrOfPoints () const;
    /**
     * Marks a link as the root link.
     *
     * The root link has to be within the dictionary
     * of links already.
     */
    void setRootLink (SIG_Link *l);

    /**
     * initiate calculates the initial positions and orientations
     * of the links.
     *
     * WARNING! If there is a link loop within the robot, there
     * may be constraints that are not solved. This depends on the
     * soundness of the values the user entered.
     *
     * CORRECTION! If the conditions that are described in the
     * warning above are true, an exception will be thrown.
     */
    void initiate ();

    /**
     * Makes all body objects load its geometr
     * descriptions from file.
     */
    void loadGeometries ();

    /**
     * Duplicates the geometry descriptions loaded
     * within a previous step. Afterwards, different
     * links do not share a common geometry object.
     */
    void instantiateGeometries ();

    /**
     * Transforms the links' coordinate system to a form
     * which is suitable for simulation with DynaMechs.
     * Also performs calculation of initial positions and
     * modified Denavit-Hartenberg parameters.
     */
    void prepareDynaMechs ();

    /**
     * Robot to text stream transformation.
     *
     * This method writes the whole robot including the
     * surface information into a stream. When it is connected
     * to a file, the robot can be saved to disk. When it is
     * connected to a network socket, the robot can be propagated
     * through a network.
     */
    void writeToFileTransfer (QTextStream & tx) const;

    /**
     * Helper functions for ill-built programs.
     */
    void readFromFileTransfer (QTextStream & tx);

    /**
     * INTERNAL.
     *
     * Writes a vector to a stream. This is a convenience function.
     */
    static void vectorToStream (QTextStream & tx, SIG_Vector vec);
    /**
     * INTERNAL.
     *
     * Reads a vector from a stream. This is a convenience function.
     */
    static SIG_Vector streamToVector (QTextStream & tx);
    /**
     * INTERNAL.
     *
     * Writes a matrix to a stream. This is a convenience function.
     */
    static void matrixToStream (QTextStream & tx, SIG_Matrix mat);
    /**
     * INTERNAL.
     *
     * Reads a matrix from a stream. This is a convenience function.
     */
    static SIG_Matrix streamToMatrix (QTextStream & tx);

    /**
     * The mass of each link, in the order of getLinks(). Throws a
     * SIG_Exception for a link whose mass cannot be computed.
     */
    QList<double> getLinkMasses () const;
  };

}

#endif // SIGEL_ROBOT_SIG_ROBOT_H
