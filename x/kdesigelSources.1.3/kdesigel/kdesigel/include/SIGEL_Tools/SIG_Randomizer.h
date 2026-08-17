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
#ifndef SIGEL_TOOLS_SIG_RANDOMIZER_H
#define SIGEL_TOOLS_SIG_RANDOMIZER_H

namespace SIGEL_Tools
{

/**
* This randomizer creates random numbers, which are used for the 
* genetic programming.
*/

class SIG_Randomizer
{
 /**
  * This randomizer creates random numbers, which are used for the 
  * genetic programming.
  */


public:   

   /**
    * This randomizer creates random numbers, which are used for the 
    * genetic programming.
    */

   SIG_Randomizer();

     /**
     * This constructor initializes the generator with seed.
     */


   SIG_Randomizer(int seed);



   ~SIG_Randomizer();

    /**
     * This operation returns a randomly generated integer number between 0 and maximum.
     * @param maximum
     * maximum defines the maximum range of the generated numbers.
     */

   int getRandomInt(int maximum);

    /**
     * This operation returns a randomly generated long integer number between 0 and maximum.
     * @param maximum
     * maximum defines the maximum range of the generated numbers.
     */

   long getRandomLong(long maximum); 

   /**
    * Sets a new seed. The randomizer will be reinitialized."
    * @post
    * The randomizer is reinitialized due to the new seed."
    */

   void setNewSeed(int seed);

 private:
   unsigned long int next;

};

}
#endif //  SIGEL_TOOLS_SIG_RANDOMIZER_H











