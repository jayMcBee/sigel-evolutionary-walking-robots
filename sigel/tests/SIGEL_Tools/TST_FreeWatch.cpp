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
#include "TST_FreeWatch.h"

#include <cstddef>
#include <new>

SIGEL_Tools::TST_FreeWatch::TST_FreeWatch( const void *pointer )
{
  watchedPointer = pointer;
  watchedPointerFreed = false;
}

SIGEL_Tools::TST_FreeWatch::~TST_FreeWatch()
{
  watchedPointer = nullptr;
}

bool SIGEL_Tools::TST_FreeWatch::freed() const
{
  return watchedPointerFreed;
}

// Replaces, for the whole test program, the sized form that a delete of a
// complete type such as SIG_Geometry calls since C++14. It only records the
// watched pointer and leaves the freeing to the default unsized form, which
// therefore stays as it is, though GCC warns that it should be replaced too.
// In build-asan this turns off AddressSanitizer's new-delete-type-mismatch
// check for the whole test program, as every sized delete passes through here.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsized-deallocation"
void operator delete( void *pointer, std::size_t ) noexcept
{
  if ( pointer && pointer == SIGEL_Tools::TST_FreeWatch::watchedPointer )
  {
    SIGEL_Tools::TST_FreeWatch::watchedPointerFreed = true;
  }
  ::operator delete( pointer );
}
#pragma GCC diagnostic pop
