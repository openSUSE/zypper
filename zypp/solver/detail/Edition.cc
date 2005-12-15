/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Edition.cc
 *
 * Definition of 'edition'
 *  contains epoch-version-release-arch
 *  and comparision functions
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
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

#include <y2util/stringutil.h>

#include <zypp/solver/detail/Edition.h>
#include <zypp/solver/detail/Version.h>

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
      
      IMPL_BASE_POINTER(Edition);
      
      //---------------------------------------------------------------------------
      
      string
      Edition::asString ( bool full ) const
      {
          return toString (*this, full);
      }
      
      
      string
      Edition::toString ( const Edition & edition, bool full )
      {
          string res ("");
      
          if (edition._epoch >= 0) {
      	res += stringutil::numstring (edition._epoch);
      	res += ":";
          }
      
          res += edition._version;
          if (!edition._release.empty()) {
      	res += "-";
      	res += edition._release;
          }
      
          if (full) {
      	res += ".";
      	res += edition._arch->asString();
          }
      
          return res;
      }
      
      
      ostream &
      Edition::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const Edition& edition)
      {
          return os << edition.asString();
      }
      
      //---------------------------------------------------------------------------
      
      Edition::Edition( int epoch, const string & version, const string & release, const Arch * arch)
          : _epoch (epoch)
          , _version (version)
          , _release (release)
          , _arch (arch)
      {
      }
      
      
      Edition::~Edition()
      {
      }
      
      
      EditionPtr
      Edition::copy (void) const
      {
          return new Edition (_epoch, _version, _release, _arch);
      }
      
      
      EditionPtr
      Edition::fromString (const char *s)
      {
          return GVersion.parse (s);
      }
      
      
      bool
      Edition::match( constEditionPtr edition ) const {
      //fprintf (stderr, "<%s> match <%s>\n", asString().c_str(), edition->asString().c_str());
          if (_epoch != edition->_epoch) return false;
      //fprintf (stderr, "epoch ok\n");
          if (_version != edition->_version) return false;
      //fprintf (stderr, "version ok\n");
          if (_release != edition->_release) return false;
      //fprintf (stderr, "release ok\n");
          if (_arch != edition->_arch) return false;
      //fprintf (stderr, "arch ok\n");
          return true;
      }
      
      
      bool
      Edition::equals( constEditionPtr edition ) const {
      //fprintf (stderr, "<%s> equals <%s>\n", asString().c_str(), edition->asString().c_str());
          return match (edition);
      }
      
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

