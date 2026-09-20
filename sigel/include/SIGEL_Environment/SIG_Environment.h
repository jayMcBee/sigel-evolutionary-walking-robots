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
#ifndef SIGEL_ENVIRONMENT_SIG_ENVIRONMENT_H
#define SIGEL_ENVIRONMENT_SIG_ENVIRONMENT_H

// DL_vector only.
#include <pointvector.h>
#include <qtextstream.h>
#include <qstring.h>
#include <dmEnvironment.hpp>


/** This namespace just holds the environment for now */
namespace SIGEL_Environment
{

/**
 * The SIG_Environment contains the Environment Data
 *
 * Later on, we hope to have some objects here as obstacles.
 * The class can read from File and write to file.
 */
class SIG_Environment {
 public:

  /** Default constructor
   *
   * @post
   *  Sets yPlaneLevel to 0 and gravity to 9.81 downwards.
   */  
  SIG_Environment();

  /** Value constructor
   *
   * @param newGravity
   *  The vector for the gravity, where the length defines the acceleration.
   * @param newYPlaneLevel
   *  The minimal level for object to be (the ground plane).
   * @post
   *  Sets yPlaneLevel to newYPlaneLevel and gravity to newGravity.
   */  
  SIG_Environment(DL_vector newGravity, double newYPlaneLevel, double newVeloDamping, 
		  DL_vector newStartPosition, QString newFloorMaterialName, 
		  double newGroundPlanarSpringConstant, double newGroundNormalSpringConstant,
		  double newGroundPlanarDamperConstant, double newGroundNormalDamperConstant, 
		  double newFrictionCoeff_u_s, double newFrictionCoeff_u_k);

  /** File constructor
   *
   * @param file
   *  the filestream to be read from.
   * @post
   *  Reads yPlaneLevel and gravity from file.
   */  
  SIG_Environment(QTextStream& file);

  /** Writes the Environment to a File
   *
   * @param file
   *  the filestream to write to.
   * @post
   *  yPlaneLevel and gravity are witten to file.
   */  
  void writeToFile(QTextStream& file);

  /** Reads the Environment from a File
   *
   * @param file
   *  the filestream to read from.
   * @post
   *  yPlaneLevel and gravity are read from file.
   */  
  void readFromFile(QTextStream& file);

  /** Sets the gravity
   *
   * @param newGravity
   *  The vector for the gravity, where the length defines the acceleration.
   * @post
   *  Sets gravity to newGravity.
   */  
  void setGravity(DL_vector newGravity);

  /** Reads the gravity
   *
   * @return
   *  The vector for the gravity, where the length defines the acceleration.
   */  
  DL_vector getGravity() const;

  /** Sets the veloDamping
   *
   * @param newVeloDamping
   *  The amount of velo damping must be between 1 (no damping) and 0 (full damping)
   * @post
   *  Sets veloDamping to newVeloDamping.
   */  
  void setVeloDamping(double newVeloDamping);

  /** Reads the veloDamping
   *
   * @return
   *  The amount of velo damping must be between 1 (no damping) and 0 (full damping).
   */  
  double getVeloDamping() const;

  /** Sets the yPlaneLevel
   *
   * @param newYPlaneLevel
   *  The minimal level for object to be (the ground plane).
   * @post
   *  Sets yPlaneLevel to newYPlaneLevel.
   */  
  void setYPlaneLevel(double newYPlaneLevel);

  /** Reads the yPlaneLevel
   *
   * @return
   *  The minimal level for object to be (the ground plane).
   */  
  double getYPlaneLevel() const;

  /** Sets the startPosition
   *
   * @param newStartPosition
   *  The starting Position of the Robot in the simulation.
   * @post
   *  Sets startPosition to newStartPosition.
   */  
  void setStartPosition(DL_vector newStartPosition);

  /** Reads the startPositon
   *
   * @return
   *  The starting Position of the Robot in the simulation.
   */  
  DL_vector getStartPosition() const;

   /** Sets the floorDimensionX
   *
   * @param newFloorDimensionX
   *  the x-dimension of dynaMechs Environment.
   * @post
   *  Sets floorDimensionX to newFloorDimensionX
   */
  void setFloorDimensionX(int newFloorDimensionX);

  /** Reads the floorDimensionX
   *
   * @return
   *  The x-dimension of the dynaMech Environment.
   */
  int getFloorDimensionX() const;

   /** Sets the floorDimensionZ
   *
   * @param newFloorDimensionZ
   *  the z-dimension of dynaMechs Environment.
   * @post
   *  Sets floorDimensionZ to newFloorDimensionZ
   */
  void setFloorDimensionZ(int newFloorDimensionZ);

  /** Reads the floorDimensionZ
   *
   * @return
   *  The z-dimension of the dynaMech Environment.
   */
  int getFloorDimensionZ() const;

  /** Sets the floorFunction
   *
   * @param newFloorFunction
   *  The function of the floor for the dynaMechs Environment.
   * @post
   *  Sets floorFunction to newFloorFunction.
   */
  void setFloorFunction(QString newFloorFunction);

  /** Reads the floorFunction
   *
   * @return
   *  The function of the floor for the dynaMechs Environment.
   */
  QString getFloorFunction() const;

  /** set the boolean variable floorFuncSelected.
  	*	it is used to determine how the terrain.ter should be produced
  	*/
	void setFloorFuncSelected(bool _floorFuncSelected);
	
	/**	reads the floorFuncSelected variable.
		*
		*	@return
		*		the boolean variable floorFuncSelected.
		*/
  bool getFloorFuncSelected();

  /**	Sets the FloorPictureFile.
  	*
  	*	@param newFloorPictureFile
  	*		The path of the file.
  	*/
  void setFloorPictureFile(QString newFloorPictureFile);

  /**	Reads the TextureFile.
  	*
  	*	@return
  	*		The path of the file.
  	*/
 	QString getTextureFile() const;

	/**	Sets the TextureFile.
 		*
 		*	@param _FloorPictureFile
 		*		The path of the file.
 	*/
  void setTextureFile(QString _textureFile);

  /**	Reads the alpha value for the texture.
  	*
  	*	@return
  	*		The alpha value for the texture.
  	*/
 	int getTexAlpha() const;

	/**	Sets the alpha value for the texture.
 		*
 		*	@param _texAlpha
 		*		The alpha value for the texture.
 	*/
  void setTexAlpha(int _texAlpha);


 	bool getWithTexture() const;

  void setWithTexture(bool _withTexture);

  /**	Reads the FloorPictureFile.
  	*
  	*	@return
  	*		The path of the file.
  	*/
 	QString getFloorPictureFile() const;
 	
  /** Sets the floorMaterialName
   *
   * @param newFloorMaterialName
   *  The name of the Material of the Floor.
   * @post
   *  Sets floorMaterialName to newFloorMaterialName.
   */
  void setFloorMaterialName(QString newFloorMaterialName);

  /** Reads the floorMaterialName
   *
   * @return
   *  The name of the Material of the Floor.
   */  
  QString getFloorMaterialName() const;

  /** Sets the ground planar spring constant
   *
   * @param newGroundPlanarSpringConstant
   *  The ground planar spring constant.
   * @post
   *  Sets groundPlanarSpringConstant to newGroundPlanarSpringConstant
   */
  void setGroundPlanarSpringConstant(double newGroundPlanarSpringConstant);

  /** Reads the ground planar spring constant
   *
   * @return
   *  The value of the ground planar spring constant
   */
  double getGroundPlanarSpringConstant() const;

  /** Sets the ground normal spring constant
   *
   * @param newGroundNormalSpringConstant
   *  The ground normal spring constant
   * @post
   *  Sets groundNormalSpringConstant to newGroundNormalSpringConstant
   */
  void setGroundNormalSpringConstant(double newGroundNormalSpringConstant);

  /** Reads the ground normal spring constant
   *
   * @return
   *  The value of the ground normal spring constant
   */
  double getGroundNormalSpringConstant() const;

  /** Sets the the ground planar damper constant
   *
   * @param newGroundPlanarDamperConstant
   *  The ground planar damper constant
   * @post
   *  Sets groundPlanarDamperConstant to newGroundPlanarDamperConstant
   */
  void setGroundPlanarDamperConstant(double newGroundPlanarDamperConstant);

  /** Reads the the ground planar damper constant
   *
   * @return
   *  The value of the ground planar damper constant
   */
  double getGroundPlanarDamperConstant() const;

  /** Sets the ground normal damper constant
   *
   * @param newGroundNormalDamperConstant
   *  The ground normal damper constant
   * @post
   *  Sets groundNormalDamperConstant to newGroundNormalDamperConstant
   */
  void setGroundNormalDamperConstant(double newGroundNormalDamperConstant);

  /** Reads the ground normal damper constant
   *
   * @return
   *  The value of the ground normal damper constant
   */
  double getGroundNormalDamperConstant() const;

  /** Sets the static friction coefficient
   *
   * @param newFrictionCoeff_u_s
   *  The static friction coefficient
   * @post
   *  Sets frictionCoeff_u_s to newFrictionCoeff_u_s
   */
  void setFrictionCoeff_u_s(double newFrictionCoeff_u_s);

  /** Reads the static friction coefficient
   *
   * @return
   *  The value of the static friction coefficient
   */
  double getFrictionCoeff_u_s() const;

  /** Sets the kinetic friction coefficient
   *
   * @param newFrictionCoeff_u_k
   *  The kinetic friction coefficient
   * @post
   *  Sets frictionCoeff_u_k to newFrictionCoeff_u_k
   */
  void setFrictionCoeff_u_k(double newFrictionCoeff_u_k);

  /** Reads the kinetic friction coefficient
   *
   * @return
   *  The value of the kinetic friction coefficient
   */
  double getFrictionCoeff_u_k() const;

  /** Loads the DynaMechsEnvironment; this means that it loads the Terrain.ter file
	 *
	 *  @return
	 *		The loaded DynamechsEnvironment
  */
  void loadDynaMechsEnvironment();

  /**
  	*  This function returns a reference to the dynaMechsEnvironment.
  	*/
  dmEnvironment *getDMEnvironment() const;

 	/**
 		*	This function generates a terrain for the DynaMechsEnvironment with
 		*	the specified function in the environment-tab and writes the parsed
 		*	output to the file "Terrain.ter" in SIGEL_ROOT.
 		*
 		*	@return
 		*		
 		*/
  bool generateTerrain();

  /** This is the set method for the variable autosave. */
  void setAutosave(int _autosave);
	
	/** This is the get method for the variable autosave. */
	int getAutosave();

 private:
  /** the storage for the zPlaneLevel */
  double yPlaneLevel;
  /** the storage for the gravity vector */
  DL_vector gravity;
  /** the storage for the velo-damping */
  double veloDamping;
  /** the storage for the start position */
  DL_vector startPosition;
	/** the storage of the x-dimension of the floor */
	int floorDimensionX;
	/** the storage of the z-dimension of the floor */
	int floorDimensionZ;
	/** the storage of the floor function */
	QString floorFunction;
	/** the storage of filename for a floor picture */
	QString floorPictureFile;
	/** determines whether a floorFunction or a floorPictureFile is specified */
	bool floorFuncSelected;
  /** the storage for the material name */
  QString floorMaterialName;
  /** the storage for the texture pixmap */
  QString textureFile;
  /** holds true if the user wants to see the texture else the variable holds false */
  bool withTexture;
  /** the storage of the alpha value for the texture */
	int texAlpha;
	
  /** the ground planar spring constant */
  double groundPlanarSpringConstant;
  /** the ground normal spring constant */
  double groundNormalSpringConstant;
  /** the ground planar damper constant */
  double groundPlanarDamperConstant;
  /** the ground normal damper constant */
  double groundNormalDamperConstant;
  /** the static friction coefficient */
  double frictionCoeff_u_s;
  /** the kinetic friction coefficient */
  double frictionCoeff_u_k;

  /** the DynaMechsEnvironment */
  dmEnvironment dynaMechsEnvironment;

 	/**
 		*	This variable specifies the intervall (in generations) in which the experiment
 		*	is automatically saved to disk.
 		*	For now the range is from 0 to 100 generations, where 0 disables autosave.
 		*/
 	int autosave;
};

}

#endif // SIGEL_ENVIRONMENT_SIG_ENVIRONMENT_H


