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
#ifndef TST_FREEWATCH_H
#define TST_FREEWATCH_H

#include <atomic>

namespace SIGEL_Tools
{

  /**
   * Watches one pointer for the life of the object and tells whether a delete of
   * a complete type freed it. One watch at a time, for the whole test program.
   */
  class TST_FreeWatch
  {
  public:
    TST_FreeWatch( const void *pointer );
    ~TST_FreeWatch();
    TST_FreeWatch( const TST_FreeWatch & ) = delete;
    TST_FreeWatch &operator=( const TST_FreeWatch & ) = delete;
    bool freed() const;

    // The pointer whose freeing the operator delete in SIGEL_Tools/TST_FreeWatch.cpp records, or nullptr.
    // Atomic, as any thread's delete reads them.
    static inline std::atomic< const void * > watchedPointer = nullptr;
    static inline std::atomic< bool > watchedPointerFreed = false;
  };

}

#endif // TST_FREEWATCH_H
