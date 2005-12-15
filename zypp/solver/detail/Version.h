/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Version.h
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

#ifndef _Version_h
#define _Version_h

#include <iosfwd>
#include <string.h>

#include <y2util/Ustring.h>
#include <zypp/solver/detail/Spec.h>

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
      //	CLASS NAME : Version
      /*
       * Version properties: These exist to signal various parts of the
       * world, packman, and dependency code of certain (mis)features of a
       * packaging system.
       *
       * PROVIDE_ANY - An unversioned provide matches all versions.  An
       * unversioned provide translates into an RC_RELATION_ANY relation, which
       * will meet any requirement for any version. (RPM)
       *
       * IGNORE_ABSENT_EPOCHS - If an epoch isn't specified in a requirement,
       * it's ignored when verifying the provide.  For example, if package "foo"
       * requires "bar >= 2.0" then both "bar 21" and "bar 1:2.0" meet the
       * requirement. (RPM)
       *
       * ALWAYS_VERIFY_RELEASE - When verifying relations, the release field is
       * usually only compared when both the requirement specifies it.  For
       * example, a requirement of "foo > 2.0" would not be met by a package
       * providing "foo 2.0-10", because the release field ("10") would be
       * ignored and "2.0" is not greater than "2.0".  When this property is
       * set, however, the release field will always be compared, and the
       * requirement in the previous example would be met, because "2.0-10" is
       * greater than "2.0". (Debian)
       *
       */
      
      #define VERSION_PROP_NONE                  (0)
      #define VERSION_PROP_PROVIDE_ANY           (1 << 0)
      #define VERSION_PROP_IGNORE_ABSENT_EPOCHS  (1 << 2)
      #define VERSION_PROP_ALWAYS_VERIFY_RELEASE (1 << 4)
      
      class Version {
      
        private:
      
          unsigned int _properties;
      
          EditionPtr (*_parse)(const char *input);
      
          // compare uses SpecPtr and takes name into account
          int (*_compare)(constSpecPtr a, constSpecPtr b);
      
        public:
      
          Version ();
          virtual ~Version();
      
          // ---------------------------------- I/O
      
          static std::string toString ( const Version & section);
      
          virtual std::ostream & dumpOn( std::ostream & str ) const;
      
          friend std::ostream& operator<<( std::ostream&, const Version & section);
      
          std::string asString ( void ) const;
      
          // ---------------------------------- accessors
      
          // ---------------------------------- methods
      
          // compare uses SpecPtr and takes name into account
          int compare (constSpecPtr a, constSpecPtr b) const { return (*_compare) (a, b); }
      
          EditionPtr parse (const char *input) const { return (*_parse)(input); }
      
          bool hasProperty (unsigned int property) const { return (_properties & property) != 0; }
      
      };
      
      extern Version GVersion;
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // _Version_h
