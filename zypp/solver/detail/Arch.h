/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Arch.h
 *
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

#ifndef _Arch_h
#define _Arch_h

#include <iosfwd>
#include <list>
#include <map>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

class Arch;
typedef std::list<const Arch *> ArchList;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Arch
/**
 *
 **/
class Arch {

  private:

    std::string _arch;

    explicit Arch( const std::string & a = "" );

  public:
    static const Arch *Any;
    static const Arch *Unknown;
    static const Arch *Noarch;
    static const Arch *System;

    static const Arch *create ( const std::string & arch );		// factory
    virtual ~Arch() {};

    // ---------------------------------- I/O

    static const std::string toString ( const Arch & arch );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<< ( std::ostream &, const Arch & arch);

    const std::string asString ( void ) const;

    // ---------------------------------- accessors

    // ---------------------------------- methods

    bool isAny (void) const { return this == Any; }
    bool isUnknown (void) const { return this == Unknown; }
    bool isNoarch (void) const { return this == Noarch; }

    ArchList getCompatList () const;
    int getCompatScore (const ArchList & archlist) const;

};

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // _Arch_h
