/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Section.h
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

#ifndef _Section_h
#define _Section_h

#include <iosfwd>
#include <string.h>

#include <y2util/Ustring.h>

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
      //	CLASS NAME : Section
      /**
       *
       **/
      class Section {
      
        private:
      
          typedef enum {
      	SECTION_OFFICE = 0,
      	SECTION_IMAGING,
      	SECTION_PIM,
      	SECTION_XAPP,
      	SECTION_GAME,
      	SECTION_MULTIMEDIA,
      	SECTION_INTERNET,
      	SECTION_UTIL,
      	SECTION_SYSTEM,
      	SECTION_DOC,
      	SECTION_LIBRARY,
      	SECTION_DEVEL,
      	SECTION_DEVELUTIL,
      	SECTION_MISC,
      	SECTION_LAST
          } section_t;
      
          section_t _section;
      
        private:
          section_t section () const { return _section; }
      
        public:
      
          Section(const char *section_str);
          virtual ~Section();
      
          // ---------------------------------- I/O
      
          static std::string toString ( const Section & section);
          static std::string toUserString ( const Section & section);
      
          virtual std::ostream & dumpOn( std::ostream & str ) const;
      
          friend std::ostream& operator<<( std::ostream&, const Section & section);
      
          std::string asString ( void ) const;
          std::string asUserString ( void ) const;
      
          // ---------------------------------- accessors
      
          // equality
          bool operator==( const Section & section) const {
      	return _section == section.section();
          }
      
          // inequality
          bool operator!=( const Section & section) const {
      	return !(*this == section);
          }
      
      };
      
      typedef Section * SectionPtr;
      
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // _Section_h
