/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Edition.h
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

#ifndef _Edition_h
#define _Edition_h

#include <iosfwd>
#include <string>

#include <y2util/Ustring.h>

#include <zypp/solver/detail/EditionPtr.h>
#include <zypp/solver/detail/XmlNodePtr.h>
#include <zypp/solver/detail/Arch.h>

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Edition
      /**
       *
       **/
      class Edition : public CountedRep {
          REP_BODY(Edition);
      
        private:
          int _epoch;
          std::string _version;
          std::string _release;
          const Arch *_arch;
      
        public:
      
          //
          // -1 resp. NULL values are treated as 'any'
          //
      
          Edition( int epoch = -1, const std::string & version = "", const std::string & release = "", const Arch * arch = Arch::Unknown);
          virtual ~Edition();
      
          // ---------------------------------- I/O
      
          const XmlNodePtr asXmlNode (void) const;
      
          static std::string toString ( const Edition & edition, bool full = false );
      
          virtual std::ostream & dumpOn( std::ostream & str ) const;
      
          friend std::ostream& operator<<( std::ostream&, const Edition& );
      
          std::string asString ( bool full = false ) const;
      
          // ---------------------------------- accessors
      
          void setVersion (const std::string & version) { _version = version; }
          void setRelease (const std::string & release) { _release = release; }
          void setEpoch (int epoch) { _epoch = epoch; }
          void setArch (const std::string & arch) { _arch = Arch::create(arch); }
          void setArch (const Arch * arch) { _arch = arch; }
      
          const std::string & version() const { return _version; }
          const std::string & release() const { return _release; }
          const int epoch() const { return _epoch; }
          bool hasEpoch() const { return _epoch >= 0; }
          const Arch * arch() const { return _arch; }
      
          bool match( constEditionPtr edition ) const;
          bool equals( constEditionPtr edition ) const;
      
          // ---------------------------------- accessors
      
          EditionPtr copy (void) const;
          static EditionPtr fromString (const char *s);
      
      };
      
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // _Edition_h
