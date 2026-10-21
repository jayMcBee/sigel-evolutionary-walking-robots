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
#ifndef SIGEL_VISUALISATION_SIG_POVRAYEXPORTER_H
#define SIGEL_VISUALISATION_SIG_POVRAYEXPORTER_H

#include <qstring.h>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QVector3D>

#include <newmat.h>

#include <GL/gl.h>

class QTextStream;

namespace SIGEL_Visualisation
{

	class SIG_Renderer;

	class SIG_SimulationVisualisation;

	/**
	 * How the POV-Ray floor is covered: one texture image per tile, laid over the floor colour.
	 */
	struct SIG_PovrayFloor
	{
		/** The texture image, or empty for a floor with a grain in its colour. */
		QString textureFile;

		/** From 0 to 1: how much the texture covers the floor colour. */
		double textureOpacity = 1;

		/** Every other column of tiles is mirrored, as the OpenGL renderer draws it. */
		bool mirrored = true;

		/** The side of one tile, in robot diameters. */
		double tileSize = 1;
	};

	/**
	 * Triangles that share their vertices, as a POV-Ray mesh2 lists them. Rays slip through
	 * the seam between two triangles unless both use the very same vertices; vertices that
	 * rounding has moved apart leave a gap.
	 */
	struct SIG_PovrayMesh
	{
		/** Each vertex once, as POV-Ray text at full precision. */
		QStringList vertices;

		/** The index in vertices of each vertex text. */
		QHash< QString, int > vertexIndices;

		/** The triangles as vertex indices, each as POV-Ray text. */
		QStringList faces;
	};

	/**
	 * Writes a SIG_SimulationVisualisation as POV-Ray scene files: one include file with
	 * what all frames share, and one scene file per frame. The text that is the same for
	 * every scene comes unchanged from the .inc resources in povray/. Where that text needs
	 * a value from SIGEL, it reads a name that this class declares before it.
	 */
	class SIG_PovrayExporter
	{
	public:
		SIG_PovrayExporter( SIG_SimulationVisualisation const &visualisation );

		/**
		 * Writes the include file that every frame includes. With a floor texture, it also
		 * writes the floor image beside it.
		 *
		 * @return false if a file cannot be written, the texture cannot be read or a POV-Ray
		 * text is missing.
		 */
		bool writeIncludeFile( QString fileName, SIG_PovrayFloor const &floor ) const;

		/**
		 * Writes one frame: the scene as it is now.
		 *
		 * @return false if the file cannot be written or a POV-Ray text is missing.
		 */
		bool writeFrame( QString includeFileName, QString fileName ) const;

	private:
		/** @param floorImageName the floor's image file, or empty for a floor with a grain in its colour. */
		QString environmentDeclarations( QString floorImageName, SIG_PovrayFloor const &floor ) const;

		/**
		 * Writes the floor image: the texture over the floor colour, at textureOpacity. A
		 * mirrored floor gets the texture and a mirrored copy; otherwise only the mirrored copy.
		 * False if the texture cannot be read or the image cannot be written.
		 */
		bool writeFloorImage( QString fileName, SIG_PovrayFloor const &floor ) const;

		/** One layer of grid lines, across the axis "x" or "z": a pattern, so that it has no end. */
		QString gridLines( QString axis ) const;

		/** The vertices and triangles of a terrain that is not flat and of the ground past its edges. */
		QString terrainMesh() const;

		/**
		 * Adds the ground past the edges of the terrain, to the horizon: as in DynaMechs, each
		 * edge cell continues outward at the edge heights, and each corner at the corner height.
		 */
		void terrainBeyondEdges( SIG_PovrayMesh &mesh ) const;

		/** The SIGEL point of the terrain at column x and row z. */
		QVector3D terrainVector( int x, int z ) const;

		/**
		 * How far the ground goes past a terrain that is not flat: 2^16. From a camera below
		 * 30 units, its end is less than a pixel below the horizon. From 2^19 on, rays slip
		 * through between the grid and the ground.
		 */
		static constexpr float groundReach = 65536;

		/** The distances from an edge at which the ground past it is cut into cells. */
		static QList< float > outwardSteps();

		/** Adds the ground past the edge from 'from' to 'to', in the direction 'outward'. */
		static void edgeStrip( SIG_PovrayMesh &mesh, QVector3D from, QVector3D to, QVector3D outward );

		/** Adds the ground at the corner's height, outward in both directions. */
		static void edgeCorner( SIG_PovrayMesh &mesh, QVector3D corner, QVector3D outward1, QVector3D outward2 );

		/** Adds two triangles: a, b, c and a, c, d. */
		static void quadrangle( SIG_PovrayMesh &mesh, QVector3D a, QVector3D b, QVector3D c, QVector3D d );

		static void triangle( SIG_PovrayMesh &mesh, QVector3D a, QVector3D b, QVector3D c );

		/** The index of vertex in mesh, which adds it if it is new. */
		static int vertexIndex( SIG_PovrayMesh &mesh, QVector3D vertex );

		QString robotDeclarations() const;

		QString environmentObjects() const;

		/**
		 * The robot radius, or 1 for a robot without vertices. Patterns scale with it, so a
		 * small and a large robot look alike.
		 */
		double patternSize() const;

		/** The radius of the grid lines and the robot's path: half a percent of patternSize. */
		double lineRadius() const;

		/** The radius of the anchor points' spheres: 2.5 percent of patternSize. */
		double anchorPointRadius() const;

		static QString sceneObjects( SIG_Renderer const &renderer, GLuint first, GLuint end );

		static QString vectorToPovray( NEWMAT::ColumnVector input );

		static QString vectorToPovray( QVector3D input );

		/** Copies a POV-Ray text compiled into the program; false if it is missing. */
		static bool writeResource( QTextStream &stream, QString fileName );

		SIG_SimulationVisualisation const &visualisation;
	};

}

#endif // SIGEL_VISUALISATION_SIG_POVRAYEXPORTER_H
