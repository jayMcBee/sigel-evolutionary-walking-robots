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
#include "SIGEL_Environment/SIG_Environment.h"

namespace SIGEL_Environment {

  SIG_Environment::SIG_Environment()
    : gravity(0,-9.81,0),
    veloDamping(0.995),
    startPosition(0,1,0),
    floorDimensionX(50),
    floorDimensionZ(50),
    floorFunction("0"),
    floorFuncSelected(true),
    withTexture(false),
    texAlpha(0xFF),
    groundPlanarSpringConstant(5500),
    groundNormalSpringConstant(7500),
    groundPlanarDamperConstant(50),
    groundNormalDamperConstant(50),
    frictionCoeff_u_s(1.5),
    frictionCoeff_u_k(1),
    autosaveIntervalGenerations(0)
  {
  }

  SIG_Environment::SIG_Environment(SIG_Vector newGravity, 
				   double newVeloDamping, SIG_Vector newStartPosition, 
				   double newGroundPlanarSpringConstant, 
				   double newGroundNormalSpringConstant, double newGroundPlanarDamperConstant, 
				   double newGroundNormalDamperConstant, double newFrictionCoeff_u_s, 
				   double newFrictionCoeff_u_k)
    : gravity(newGravity),
      veloDamping(newVeloDamping),
      startPosition(newStartPosition),
      floorFuncSelected(true),
      withTexture(false),
      texAlpha(0xFF),
      groundPlanarSpringConstant(newGroundPlanarSpringConstant),
      groundNormalSpringConstant(newGroundNormalSpringConstant),
      groundPlanarDamperConstant(newGroundPlanarDamperConstant),
      groundNormalDamperConstant(newGroundNormalDamperConstant),
      frictionCoeff_u_s(newFrictionCoeff_u_s),
      frictionCoeff_u_k(newFrictionCoeff_u_k),
      autosaveIntervalGenerations(0)
  {
  }

  void SIG_Environment::readFromFile(QTextStream& file)
  {
    QString s;
    while (!file.atEnd()) {
      s=file.readLine();

  		if (s == "VELODAMPING") {
      	s=file.readLine();
      	veloDamping=s.toDouble();
      }

      if (s == "GRAVITY") {
      	s=file.readLine();
      	gravity.x=s.toDouble();
      	s=file.readLine();
      	gravity.y=s.toDouble();
      	s=file.readLine();
      	gravity.z=s.toDouble();
      }

      if (s == "STARTPOSITION") {
      	s=file.readLine();
      	startPosition.x=s.toDouble();
      	s=file.readLine();
      	startPosition.y=s.toDouble();
      	s=file.readLine();
      	startPosition.z=s.toDouble();
  		}

  		if ( s == "FLOORDIMENSION") {
  			s=file.readLine();
  			floorDimensionX=s.toInt();
  			s=file.readLine();
  			floorDimensionZ=s.toInt();
  		}
  		
  		if (s == "FLOORFUNCTION") floorFunction=file.readLine();
  		
  		if (s == "FLOORPICTUREFILE") floorPictureFile=file.readLine();
  		
  		if (s == "TEXTUREFILE") textureFile=file.readLine();

     	if (s == "FLOORFUNCSELECTED") {
  			s=file.readLine();
  			int tmp = s.toInt();
  			if (tmp == 0) {
  				floorFuncSelected = false;
  			}
  			else floorFuncSelected = true;
  		}
      		
  		if (s == "TEXALPHA") {
      	s=file.readLine();
      	texAlpha=s.toInt();
  		}
  		
  		if (s == "WITHTEXTURE") {
  			s=file.readLine();
  			int tmp = s.toInt();
  			if (tmp == 0) {
  				withTexture = false;
  			}
  			else withTexture = true;
  		}
  		
    	if (s == "AUTOSAVETIME") {
  			s = file.readLine();
     		autosaveIntervalGenerations = s.toInt();
      }

      if (s == "GPSconst") {
      	s=file.readLine();
      	groundPlanarSpringConstant=s.toDouble();
      }

      if (s == "GNSconst") {
      	s=file.readLine();
      	groundNormalSpringConstant=s.toDouble();
      }

      if (s == "GPDconst") {
      	s=file.readLine();
      	groundPlanarDamperConstant=s.toDouble();
  		}

      if (s == "GNDconst") {
      	s=file.readLine();
      	groundNormalDamperConstant=s.toDouble();
      }

      if (s == "USFcoeff") {
      	s=file.readLine();
      	frictionCoeff_u_s=s.toDouble();
  		}

      if (s == "UKFcoeff") {
      	s=file.readLine();
      	frictionCoeff_u_k=s.toDouble();
      }
		}
  }
  
  SIG_Environment::SIG_Environment(QTextStream& file)
  {
    readFromFile(file);
  }

  void SIG_Environment::writeToFile(QTextStream& file)
  {
    // Nothing reads YPLANELEVEL or FLOORMATERIALNAME; fixed values keep the file format.
    file << "YPLANELEVEL\n";
    file << 0 << "\n";
    file << "VELODAMPING\n";
    file << veloDamping << "\n";
    file << "GRAVITY\n";
    file << gravity.x << "\n";
    file << gravity.y << "\n";
    file << gravity.z << "\n";
    file << "STARTPOSITION\n";
    file << startPosition.x << "\n";
    file << startPosition.y << "\n";
    file << startPosition.z << "\n";
    file << "FLOORDIMENSION\n";
		file << floorDimensionX << "\n";
		file << floorDimensionZ << "\n";
		file << "FLOORFUNCTION\n";
		file << floorFunction << "\n";
		file << "FLOORPICTUREFILE\n";
		file << floorPictureFile << "\n";
  	file << "FLOORFUNCSELECTED\n";
   	file << floorFuncSelected << Qt::endl;
		file << "TEXTUREFILE" << Qt::endl;
		file << textureFile << Qt::endl;
		file << "TEXALPHA" << Qt::endl;
		file << texAlpha << Qt::endl;
		file << "WITHTEXTURE" << Qt::endl;
		file << withTexture << Qt::endl;
    file << "FLOORMATERIALNAME\n";
    file << "floor\n";
    file << "AUTOSAVETIME\n";
    file << autosaveIntervalGenerations << "\n";

    file << "GPSconst\n";
    file << groundPlanarSpringConstant << "\n";
    file << "GNSconst\n";
    file << groundNormalSpringConstant << "\n";
    file << "GPDconst\n";
    file << groundPlanarDamperConstant << "\n";
    file << "GNDconst\n";
    file << groundNormalDamperConstant << "\n";
    file << "USFcoeff\n";
    file << frictionCoeff_u_s << "\n";
    file << "UKFcoeff\n";
    file << frictionCoeff_u_k << "\n";
  }

  void SIG_Environment::setGravity(SIG_Vector newGravity)
  {
    gravity=newGravity;
  }

  SIG_Vector SIG_Environment::getGravity() const
  {
    return gravity;
  }

  void SIG_Environment::setVeloDamping(double newVeloDamping)
  {
    veloDamping=newVeloDamping;
  }

  double SIG_Environment::getVeloDamping() const
  {
    return veloDamping;
  }

  void SIG_Environment::setStartPosition(SIG_Vector newStartPosition)
  {
    startPosition=newStartPosition;
  }

  SIG_Vector SIG_Environment::getStartPosition() const
  {
    return startPosition;
  }

  void SIG_Environment::setFloorDimensionX(int newFloorDimensionX) {
		floorDimensionX = newFloorDimensionX;
  }

  int SIG_Environment::getFloorDimensionX() const {
		return floorDimensionX;
  }

  void SIG_Environment::setFloorDimensionZ(int newFloorDimensionZ) {
		floorDimensionZ = newFloorDimensionZ;
  }

  int SIG_Environment::getFloorDimensionZ() const {
		return floorDimensionZ;
  }

  void SIG_Environment::setFloorFunction(QString newFloorFunction)
  {
    floorFunction=newFloorFunction;
  }

  QString SIG_Environment::getFloorFunction() const
  {
    return floorFunction;
  }

  void SIG_Environment::setGroundPlanarSpringConstant(double newGroundPlanarSpringConstant) 
  {
    groundPlanarSpringConstant=newGroundPlanarSpringConstant;
  }

  double SIG_Environment::getGroundPlanarSpringConstant() const 
  {
    return groundPlanarSpringConstant;
  }

  void SIG_Environment::setGroundNormalSpringConstant(double newGroundNormalSpringConstant) 
  {
    groundNormalSpringConstant=newGroundNormalSpringConstant;
  }

  double SIG_Environment::getGroundNormalSpringConstant() const 
  {
    return groundNormalSpringConstant;
  }

  void SIG_Environment::setGroundPlanarDamperConstant(double newGroundPlanarDamperConstant) 
  {
    groundPlanarDamperConstant=newGroundPlanarDamperConstant;
  }

  double SIG_Environment::getGroundPlanarDamperConstant() const 
  {
    return groundPlanarDamperConstant;
  }

  void SIG_Environment::setGroundNormalDamperConstant(double newGroundNormalDamperConstant) 
  {
    groundNormalDamperConstant=newGroundNormalDamperConstant;
  }

  double SIG_Environment::getGroundNormalDamperConstant() const 
  {
    return groundNormalDamperConstant;
  }

  void SIG_Environment::setFrictionCoeff_u_s(double newFrictionCoeff_u_s) 
  {
    frictionCoeff_u_s=newFrictionCoeff_u_s;
  }

  double SIG_Environment::getFrictionCoeff_u_s() const 
  {
    return frictionCoeff_u_s;
  }

  void SIG_Environment::setFrictionCoeff_u_k(double newFrictionCoeff_u_k) 
  {
    frictionCoeff_u_k=newFrictionCoeff_u_k;
  }

  double SIG_Environment::getFrictionCoeff_u_k() const 
  {
    return frictionCoeff_u_k;
  }

	void SIG_Environment::setFloorFuncSelected(bool newFloorFuncSelected) {
		floorFuncSelected = newFloorFuncSelected;
	}
	
  bool SIG_Environment::getFloorFuncSelected() {
		return floorFuncSelected;
  }

  void SIG_Environment::setFloorPictureFile(QString newFloorPictureFile) {
		floorPictureFile = newFloorPictureFile;
  }

 	QString SIG_Environment::getFloorPictureFile() const {
 		return floorPictureFile;
 	}
 	
 	QString SIG_Environment::getTextureFile() const {
 		return textureFile;
 	}

  void SIG_Environment::setTextureFile(QString newTextureFile) {
		textureFile = newTextureFile;
  }

	SIG_Terrain SIG_Environment::getTerrain() const {
		if (givenTerrain)
			return *givenTerrain;

		SIG_Terrain terrain;

		if (makeTerrain( terrain ))
			return terrain;

		terrain.setFlat( floorDimensionX, floorDimensionZ );
		return terrain;
	}

	void SIG_Environment::setTerrain(SIG_Terrain const &newTerrain) {
		givenTerrain = newTerrain;
	}

	bool SIG_Environment::makeTerrain(SIG_Terrain &terrain) const {
		if (!floorFuncSelected)
			return terrain.setFromPictureFile( floorPictureFile );

		if (!terrain.setFromFunction( floorFunction, floorDimensionX, floorDimensionZ ))
			terrain.setFlat( floorDimensionX, floorDimensionZ );

		return true;
	}

	int SIG_Environment::getTexAlpha() const {
		return texAlpha;
	}

  void SIG_Environment::setTexAlpha(int newTexAlpha) {
		texAlpha = newTexAlpha;
  }

  bool SIG_Environment::getWithTexture() const {
		return withTexture;
  }

  void SIG_Environment::setWithTexture(bool newWithTexture) {
		withTexture = newWithTexture;
  }

	void SIG_Environment::setAutosaveIntervalGenerations(int newAutosaveIntervalGenerations) {
		autosaveIntervalGenerations = newAutosaveIntervalGenerations;
	}

	int SIG_Environment::getAutosaveIntervalGenerations() {
		return autosaveIntervalGenerations;
	}
	
}
