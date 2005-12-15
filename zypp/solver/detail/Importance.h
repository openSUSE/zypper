/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Importance.h
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

#ifndef _Importance_h
#define _Importance_h

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
      //	CLASS NAME : Importance
      /**
       *
       **/
      class Importance {
      
        private:
      
          typedef enum {
      	IMPORTANCE_INVALID = -1,
      
      	IMPORTANCE_NECESSARY,
      	IMPORTANCE_URGENT,
      	IMPORTANCE_SUGGESTED,
      	IMPORTANCE_FEATURE,
      	IMPORTANCE_MINOR,
      	IMPORTANCE_UNDEFINED,
      
      	// Not a real importance
      	IMPORTANCE_LAST
          } importance_t;
      
          importance_t _importance;
      
        private:
          importance_t importance () const { return _importance; }
      
        public:
      
          Importance(const char *importance_str);
          virtual ~Importance();
      
          static const Importance & Undefined;
      
          // ---------------------------------- I/O
      
          static std::string toString ( const Importance & importance);
      
          virtual std::ostream & dumpOn( std::ostream & str ) const;
      
          friend std::ostream& operator<<( std::ostream&, const Importance & importance);
      
          std::string asString ( void ) const;
      
          // ---------------------------------- accessors
      
          // equality
          bool operator==( const Importance & importance) const {
      	return _importance == importance.importance();
          }
      
          // inequality
          bool operator!=( const Importance & importance) const {
      	return !(*this == importance);
          }
      
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

#endif // _Importance_h
