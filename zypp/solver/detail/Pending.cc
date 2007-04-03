/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Pending.cc
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'edition'
 *  contains epoch-version-release-arch
 *  and comparision functions
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <iostream>

#include "zypp/solver/detail/Pending.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      using namespace std;

      IMPL_PTR_TYPE(Pending);

      //---------------------------------------------------------------------------

      string
      Pending::asString ( void ) const
      {
          return toString (*this);
      }


      string
      Pending::toString ( const Pending & pending )
      {
          return "<pending/>";
      }


      ostream &
      Pending::dumpOn (ostream & str) const
      {
          str << asString();
          return str;
      }


      ostream &
      operator<< (ostream & os, const Pending & pending)
      {
          return os << pending.asString();
      }

      //---------------------------------------------------------------------------

      Pending::Pending (const char *description)
      {
      }


      Pending::~Pending()
      {
      }

      //---------------------------------------------------------------------------

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

