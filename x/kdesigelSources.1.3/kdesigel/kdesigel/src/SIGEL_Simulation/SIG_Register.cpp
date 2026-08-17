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
#ifdef _WINDOWS
#pragma warning( disable : 4290 )
#endif

#include "SIGEL_Simulation/SIG_Register.h"
#include <cmath> // needed for computing the power of a number

SIGEL_Simulation::SIG_Register::SIG_Register(int size)
  throw(SIGEL_Simulation::SIG_RegisterWrongSizeException)
  : value(0)
{
  if ( (size>0) && (size<100) )
    this->size = size;
  else 
    throw SIG_RegisterWrongSizeException(__FILE__,__LINE__,"Size of Register must be between 1 and 99");
};

void SIGEL_Simulation::SIG_Register::makeValid()
{
#ifdef _WINDOWS
  int max= static_cast<int>(::pow(2,size-1));
#else
  int max= static_cast<int>(std::pow(2,size-1));
#endif

  if ( max - 1 < value )
    {
      value = -max + ( ( value + max ) % (2*max) );
    };

  if ( value < -max )
    value = max - 1 + ( ( value - ( max - 1) ) % (2*max) );
};

void SIGEL_Simulation::SIG_Register::copyReg(SIG_Register const& otherRegister)
{ 
  value=otherRegister.getValue();
  // makeValid(); superflous
};

void SIGEL_Simulation::SIG_Register::addReg(SIG_Register const& otherRegister)
{
  value=value+otherRegister.getValue();
  makeValid();
};

void SIGEL_Simulation::SIG_Register::subReg(SIG_Register const& otherRegister)
{ 
  value=value-otherRegister.getValue();
  makeValid();
};

void SIGEL_Simulation::SIG_Register::loadValue(int newValue)
{
  value=newValue;
  makeValid();
};

void SIGEL_Simulation::SIG_Register::mulReg(SIG_Register const& otherRegister)
{
  value=value*otherRegister.getValue();
  makeValid();
};

void SIGEL_Simulation::SIG_Register::divReg(SIG_Register const& otherRegister)
{
  if( otherRegister.getValue() == 0 )
    value = 0;
  else
    value=value/otherRegister.getValue();
  makeValid();
};

void SIGEL_Simulation::SIG_Register::minReg(SIG_Register const& otherRegister)
{
  if (otherRegister.getValue()<value)
    value=otherRegister.getValue();
  // makeValid(); superflous
};

void SIGEL_Simulation::SIG_Register::maxReg(SIG_Register const& otherRegister)
{
  if (otherRegister.getValue()>value)
    value=otherRegister.getValue();
  // makeValid(); superflous
};

void SIGEL_Simulation::SIG_Register::modReg(SIG_Register const& otherRegister)
{
  if( otherRegister.getValue() == 0 )
    value = 0;
  else
  {  
    int modnumber = otherRegister.getValue();
    if ( modnumber < 0 )
      {
        modnumber = -modnumber;
      }
    if ( value < 0 )
      value = modnumber + ( value % modnumber );
    else
      value = value % modnumber;
  };
  makeValid();
};

int SIGEL_Simulation::SIG_Register::getValue() const
{
  return value;
};

int SIGEL_Simulation::SIG_Register::getSize() const
{
  return size;
};

int SIGEL_Simulation::SIG_Register::getMaxValue() const
{
#ifdef _WINDOWS
  return static_cast<int>( ::pow(2,size-1) - 1);
#else
  return static_cast<int>( std::pow(2,size-1) - 1);
#endif
};

int SIGEL_Simulation::SIG_Register::getMinValue() const
{
#ifdef _WINDOWS
  return static_cast<int>( - ::pow(2,size-1));
#else
  return static_cast<int>( - std::pow(2,size-1));
#endif
};
