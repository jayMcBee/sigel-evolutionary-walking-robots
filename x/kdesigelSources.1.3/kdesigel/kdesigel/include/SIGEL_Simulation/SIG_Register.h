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
#ifndef SIGEL_SIMULATION_SIG_REGISTER_H
#define SIGEL_SIMULATION_SIG_REGISTER_H

#ifdef _WINDOWS
#pragma warning( disable : 4290 )
#endif

#include "SIGEL_Simulation/SIG_RegisterWrongSizeException.h"

namespace SIGEL_Simulation
{

/**
 * This class represents one register.
 *
 * The interpreter holds an QVector of Registers. At the moment every
 * register is an integer value that holds the condition
 * @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
 */
class SIG_Register {
 public:

  /**
   * Constructs a register with size bits.
   *
   * @exception SIG_RegisterWrongSizeException
   *            The supplid size parameter was @f$\le 0@f$.
   * @exception SIG_RegisterWrongSizeException
   *            The supplid size parameter was @f$\ge 100@f$.
   *
   * @post size is set to parameter size, if @f$size>0@f$.
   * @post value is set to 0.
   *
   * @param size The desired size of this SIG_Register in Bits.
   *             Has to be @f$>0@f$ and @f$<100@f$.
   */
  SIG_Register(int size)
    throw(SIG_RegisterWrongSizeException);

  /**
   * Copies the value of the supplied SIG_Register
   * into this SIG_Register.
   *
   * @param otherRegister The SIG_Register object whose
   *        value is to be copied.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  void copyReg(SIG_Register const& otherRegister);

  /**
   * Adds the value of the supplied SIG_Register
   * to this SIG_Register's value.
   *
   * @param otherRegister The SIG_Register object whose value
   *                      is to be added.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  void addReg(SIG_Register const& otherRegister);

  /**
   * Substracts the value of the supplied SIG_Register
   * from this SIG_Register's value.
   *
   * @param otherRegister The SIG_Register object whose value
   *                      is to be substracted.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  void subReg(SIG_Register const& otherRegister);

  /**
   * Sets this SIG_Register's value to newValue.
   *
   * @param newValue The integer-value to which this SIG_Register's
   *                 value is to be set.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  void loadValue(int newValue);

  /**
   * Multiplies this SIG_Register's value with the supplied
   * ones.
   *
   * @param otherRegister The SIG_Register to multiply with.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  void mulReg(SIG_Register const& otherRegister);

  /**
   * Divides this SIG_Register's value by the supplied ones.
   * 
   * The result is rounded toward zero.
   *
   * @param otherRegister The SIG_Register to divide by.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  void divReg(SIG_Register const& otherRegister);

  /**
   * Sets this SIG_Register's value to the minimum of
   * value and otherRegister.value.
   *
   * @param otherRegister The SIG_Register to compare with.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  void minReg(SIG_Register const& otherRegister);

  /**
   * Sets this SIG_Register's value to the maximum of
   * value and otherRegister.value.
   *
   * @param otherRegister The SIG_Register to compare with.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   *
   */
  void maxReg(SIG_Register const& otherRegister);

  /**
   * Modulo-divides this SIG_Register's value by the supplied ones.
   *
   * @param otherRegister The SIG_Register to modulo-divide by.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   *  
   */
  void modReg(SIG_Register const& otherRegister);

  /**
   * @return This SIG_Register's integer-value.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  int getValue() const;

  /**
   * Returns the size of the register.
   * @return The size of the register.
   */
  int getSize() const;

  /**
   * Returns the maximal value the register is able to hold.
   * @return The maximal value the register is able to hold.
   */
  int getMaxValue() const;

  /**
   * Returns the minimal value the register is able to hold.
   * @return The minimal value the register is able to hold.
   */
  int getMinValue() const;

 private:

  /**
   * Produces the condition that
   * @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   *
   * In the case @f$value < -2^{size-1}@f$ value is set to
   * @f$-2^{size-1}@f$.
   * In the case @f$2^{size-1} - 1 < value@f$ value is set to
   * @f$2^{size-1} - 1@f$.
   * This method is called by load, add, sub, mul, div and mod.
   *
   * @post @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  void makeValid();

  /**
   * The value of the Register.
   * 
   * It is always @f$-2^{size-1} \le value \le 2^{size-1} - 1@f$.
   */
  int value;

  /**
   * The size of this SIG_Register in bits.
   */
  int size;

};

}

#endif // SIGEL_SIMULATION_SIG_REGISTER_H
