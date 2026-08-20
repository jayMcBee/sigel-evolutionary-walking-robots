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
#include "fparser.h"
#include "compat/q2compat.h"
#include "SIGEL_Tools/SIG_IO.h"

#ifdef _WINDOWS
using namespace std;
#else
#include "stdlib.h"
#endif

namespace SIGEL_Environment {

  SIG_Environment::SIG_Environment()
    : gravity(0,-9.81,0),
    yPlaneLevel(0),
    veloDamping(0.995),
    startPosition(0,1,0),
    floorDimensionX(50),
    floorDimensionZ(50),
    floorFunction("0"),
    floorFuncSelected(true),
    floorMaterialName("floor"),
    withTexture(false),
    texAlpha(0xFF),
    groundPlanarSpringConstant(5500),
    groundNormalSpringConstant(7500),
    groundPlanarDamperConstant(50),
    groundNormalDamperConstant(50),
    frictionCoeff_u_s(1.5),
    frictionCoeff_u_k(1),
    autosave(0)
  {
		loadDynaMechsEnvironment();
  };

  SIG_Environment::SIG_Environment(DL_vector newGravity, double newYPlaneLevel, 
				   double newVeloDamping, DL_vector newStartPosition, 
				   QString newFloorMaterialName, double newGroundPlanarSpringConstant, 
				   double newGroundNormalSpringConstant, double newGroundPlanarDamperConstant, 
				   double newGroundNormalDamperConstant, double newFrictionCoeff_u_s, 
				   double newFrictionCoeff_u_k)
    : gravity(newGravity),
      yPlaneLevel(newYPlaneLevel),
      veloDamping(newVeloDamping),
      startPosition(newStartPosition),
      floorFuncSelected(true),
      withTexture(false),
      texAlpha(0xFF),
      floorMaterialName(newFloorMaterialName),
      groundPlanarSpringConstant(newGroundPlanarSpringConstant),
      groundNormalSpringConstant(newGroundNormalSpringConstant),
      groundPlanarDamperConstant(newGroundPlanarDamperConstant),
      groundNormalDamperConstant(newGroundNormalDamperConstant),
      frictionCoeff_u_s(newFrictionCoeff_u_s),
      frictionCoeff_u_k(newFrictionCoeff_u_k),
      autosave(0)
  {
		loadDynaMechsEnvironment();
  };

  void SIG_Environment::readFromFile(QTextStream& file)
  {
    QString s;
    while (!file.atEnd()) {
      s=file.readLine();

      if ( s == "YPLANELEVEL") {
      	s=file.readLine();
      	yPlaneLevel=s.toDouble();
      }

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
  		
  		if (s == "FLOORMATERIALNAME") floorMaterialName=file.readLine();

    	if (s == "AUTOSAVETIME") {
  			s = file.readLine();
     		autosave = s.toInt();
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
    loadDynaMechsEnvironment();

  };
  
  SIG_Environment::SIG_Environment(QTextStream& file)
  {
    readFromFile(file);
  };

  void SIG_Environment::writeToFile(QTextStream& file)
  {
    file << "YPLANELEVEL\n";
    file << yPlaneLevel << "\n";
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
    file << floorMaterialName << "\n";
    file << "AUTOSAVETIME\n";
    file << autosave << "\n";

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
  };
  void SIG_Environment::setGravity(DL_vector newGravity)
  {
    gravity=newGravity;
  };
  DL_vector SIG_Environment::getGravity() const
  {
    return gravity;
  };
  void SIG_Environment::setYPlaneLevel(double newYPlaneLevel)
  {
    yPlaneLevel=newYPlaneLevel;
  };
  double SIG_Environment::getYPlaneLevel() const
  {
    return yPlaneLevel;
  };
  void SIG_Environment::setVeloDamping(double newVeloDamping)
  {
    veloDamping=newVeloDamping;
  };
  double SIG_Environment::getVeloDamping() const
  {
    return veloDamping;
  };
  void SIG_Environment::setStartPosition(DL_vector newStartPosition)
  {
    startPosition=newStartPosition;
  };
  DL_vector SIG_Environment::getStartPosition() const
  {
    return startPosition;
  };

  void SIG_Environment::setFloorDimensionX(int newFloorDimensionX) {
		floorDimensionX = newFloorDimensionX;
  };

  int SIG_Environment::getFloorDimensionX() const {
		return floorDimensionX;
  };

  void SIG_Environment::setFloorDimensionZ(int newFloorDimensionZ) {
		floorDimensionZ = newFloorDimensionZ;
  };

  int SIG_Environment::getFloorDimensionZ() const {
		return floorDimensionZ;
  };

  void SIG_Environment::setFloorFunction(QString newFloorFunction)
  {
    floorFunction=newFloorFunction;
  };

  QString SIG_Environment::getFloorFunction() const
  {
    return floorFunction;
  };

  void SIG_Environment::setFloorMaterialName(QString newFloorMaterialName)
  {
    floorMaterialName=newFloorMaterialName;
  };

  QString SIG_Environment::getFloorMaterialName() const
  {
    return floorMaterialName;
  };

  void SIG_Environment::setGroundPlanarSpringConstant(double newGroundPlanarSpringConstant) 
  {
    groundPlanarSpringConstant=newGroundPlanarSpringConstant;
  };

  double SIG_Environment::getGroundPlanarSpringConstant() const 
  {
    return groundPlanarSpringConstant;
  };

  void SIG_Environment::setGroundNormalSpringConstant(double newGroundNormalSpringConstant) 
  {
    groundNormalSpringConstant=newGroundNormalSpringConstant;
  };

  double SIG_Environment::getGroundNormalSpringConstant() const 
  {
    return groundNormalSpringConstant;
  };

  void SIG_Environment::setGroundPlanarDamperConstant(double newGroundPlanarDamperConstant) 
  {
    groundPlanarDamperConstant=newGroundPlanarDamperConstant;
  };

  double SIG_Environment::getGroundPlanarDamperConstant() const 
  {
    return groundPlanarDamperConstant;
  };

  void SIG_Environment::setGroundNormalDamperConstant(double newGroundNormalDamperConstant) 
  {
    groundNormalDamperConstant=newGroundNormalDamperConstant;
  };

  double SIG_Environment::getGroundNormalDamperConstant() const 
  {
    return groundNormalDamperConstant;
  };

  void SIG_Environment::setFrictionCoeff_u_s(double newFrictionCoeff_u_s) 
  {
    frictionCoeff_u_s=newFrictionCoeff_u_s;
  };

  double SIG_Environment::getFrictionCoeff_u_s() const 
  {
    return frictionCoeff_u_s;
  };

  void SIG_Environment::setFrictionCoeff_u_k(double newFrictionCoeff_u_k) 
  {
    frictionCoeff_u_k=newFrictionCoeff_u_k;
  };

  double SIG_Environment::getFrictionCoeff_u_k() const 
  {
    return frictionCoeff_u_k;
  };

	void SIG_Environment::setFloorFuncSelected(bool _floorFuncSelected) {
		floorFuncSelected = _floorFuncSelected;
	};
	
  bool SIG_Environment::getFloorFuncSelected() {
		return floorFuncSelected;
  };

  void SIG_Environment::setFloorPictureFile(QString newFloorPictureFile) {
		floorPictureFile = newFloorPictureFile;
  };

 	QString SIG_Environment::getFloorPictureFile() const {
 		return floorPictureFile;
 	};
 	
 	QString SIG_Environment::getTextureFile() {
 		return textureFile;
 	};

  void SIG_Environment::setTextureFile(QString _textureFile) {
		textureFile = _textureFile;
  };

	void SIG_Environment::loadDynaMechsEnvironment(){
		// before loading the DynaMechsEnvironment first generate an actual
   	generateTerrain();
#ifdef _WINDOWS	
	  char *sigelRootCString = ::getenv( "SIGEL_ROOT" );
#else	
	  char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
#endif

 	 	QString sigelRootString( sigelRootCString );

  	QString terrainDataFileName = sigelRootString + "/Terrain.ter";

  	Q2CString terrainDataFileNameQCString = terrainDataFileName.toUtf8();
	  char const *terrainDataFileNameCString = terrainDataFileNameQCString;

  	dynaMechsEnvironment.loadTerrainData( terrainDataFileNameCString );

  	dmEnvironment::setEnvironment( &dynaMechsEnvironment );
	};
	
	dmEnvironment *SIG_Environment::getDMEnvironment() {
		return dmEnvironment::getEnvironment();
	}
	
	int SIG_Environment::getTexAlpha() {
		return texAlpha;
	};

  void SIG_Environment::setTexAlpha(int _texAlpha) {
		texAlpha = _texAlpha;
  };

  bool SIG_Environment::getWithTexture() {
		return withTexture;
  };

  void SIG_Environment::setWithTexture(bool _withTexture) {
		withTexture = _withTexture;
  };

	bool SIG_Environment::generateTerrain() {
#ifdef _WINDOWS	
  	char *sigelRootCString = ::getenv( "SIGEL_ROOT" );
#else  	
  	char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
#endif  	
  	string terrain(sigelRootCString);
  	terrain += "/Terrain.ter";
  	
  	
  	std::ofstream ausgabeTerrain(terrain.c_str(), std::ios::trunc | std::ios::out);
  	
  	if (floorFuncSelected) {
  		QString str = floorFunction;
  	
  		// this is the header of the Terrain.ter file
  		// it specifies the dimensions in x and z direction and the dimension of the grid (always 1)
  		ausgabeTerrain << floorDimensionX << " " << floorDimensionZ << " " << 1 << endl;
  	
  		const QByteArray funcBytes = str.toUtf8();
		const char* func = funcBytes.constData();

  		FunctionParser fp;
  		if (fp.Parse(func,"xz") != -1) {
  			SIGEL_Tools::SIG_IO::cerr << "Warning: the specified terrain function could not be parsed. It must depend on x and z, and contain only valid expressions -- see the documentation in supportingLibs/fparser." << Qt::endl;
  			return false;
  		}
  	
  		double vals[2];
  	
  		// calculate the y value for each (x,z) position
  		for (int z=0;z<floorDimensionZ;++z)
  		{
  			for (int x=0;x<floorDimensionX;++x)
  			{
  				vals[0] = x;
  				vals[1] = z;
    			ausgabeTerrain << fp.Eval(vals) << " ";
  			}
  			ausgabeTerrain << endl;
  		}
  	} // if(floorFuncSelected)
  	
  	else { // a pictureFile is specified
  		QString input = floorPictureFile;
  		std::ifstream pgm(input.toUtf8().constData());
  		if (!pgm) {
  			SIGEL_Tools::SIG_IO::cerr << "Warning: the specified terrain file does not exist." << Qt::endl;
  			return false;
  		}
  		
  		string s;
  		QString str;
			double max = 0;
			int counter = 0;
			while (!pgm.eof()) {
			    pgm >> s; // read file content till white space
    			
    			if (s.find("#",0)!=string::npos) {
    				pgm.ignore(255, '\n');
    			}
    			else {
    				switch(counter) {
    					case 0: // Magic Key
    						if (s.compare("P2")!=0) {
									SIGEL_Tools::SIG_IO::cerr << "Warning: the specified picture file is not in PGM format." << Qt::endl;
									return false;
    						}
								else ++counter;
								break;
							case 1: // x-DIM
								ausgabeTerrain << s << " ";
								++counter;
								break;
							case 2: // y-DIM
    						ausgabeTerrain << s << " ";
    						ausgabeTerrain << 1 << endl;
								++counter;
								break;
							case 3: // max. grey-value
    						max = QString(s.c_str()).toDouble();
    						++counter;
								break;
							default:
								str = QString(s.c_str());
								double ausgabe = str.toDouble();
								ausgabeTerrain << ausgabe/max*5 << endl;						
    				} // switch-tree
    			} // else, no comment has been read
			} // while-loop

  		pgm.close();
  	} // else, picturefile specified
  	
  	ausgabeTerrain.close();
  	return true;
	};

	void SIG_Environment::setAutosave(int _autosave) {
		autosave = _autosave;
	};

	int SIG_Environment::getAutosave() {
		return autosave;
	};
	
}
