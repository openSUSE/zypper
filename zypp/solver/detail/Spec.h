/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Spec.h  resItem specification: name + edition
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

#ifndef _Spec_h
#define _Spec_h

#include <list>
#include <iosfwd>
#include <string.h>

#include <y2util/Ustring.h>

#include <zypp/solver/detail/Hash.h>
#include <zypp/solver/detail/SpecPtr.h>
#include <zypp/solver/detail/XmlNode.h>
#include <zypp/solver/detail/Edition.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//      CLASS NAME : Name
/**
 * A resItem name
 **/

class Name : public Ustring {

  private:

    static UstringHash _nameHash;

  public:

    explicit Name( const std::string & n = "" ) : Ustring( _nameHash, n ) {}
};

///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//
//      CLASS NAME : Kind
/**
 * A spec kind (package, patch, ...)
 **/

class Kind : public Ustring {

  private:

    static UstringHash _kindHash;

    explicit Kind( const std::string & t = "" ) : Ustring( _kindHash, t ) {}

  public:

    static const Kind & Unknown;
    static const Kind & Package;
    static const Kind & Patch;
    static const Kind & Script;
    static const Kind & Message;
    static const Kind & Selection;
    static const Kind & Product;
};

///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Spec
/**
 *
 **/

class Spec : public CountedRep {
    REP_BODY(Spec);

  private:
    Kind _kind;
    Name _name;
    EditionPtr _edition;

  public:
    typedef std::list<Spec> SpecList;

    Spec( const Kind & kind,
	  const std::string & name,
	  int epoch = -1,
	  const std::string & version = "",
	  const std::string & release = "",
	  const Arch * arch = Arch::Unknown);

    Spec (const Kind & kind, const std::string & name, constEditionPtr edition);

    Spec (constXmlNodePtr node);

    virtual ~Spec();

    // ---------------------------------- I/O

    const xmlNodePtr asXmlNode (const char *name) const;

    static std::string toString ( const Spec & spec, bool full = false );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream&, const Spec& );

    std::string asString ( bool full = false ) const;

    // ---------------------------------- accessors

    const std::string & version() const { return _edition->version(); }
    void setVersion (const std::string & version) { _edition->setVersion (version); }

    const std::string & release() const { return _edition->release(); }
    void setRelease (const std::string & release) { _edition->setRelease (release); }

    const int epoch() const { return _edition->epoch(); }
    void setEpoch (int epoch) { _edition->setEpoch (epoch); }
    bool hasEpoch() const { return _edition->hasEpoch(); }

    const Arch * arch() const { return _edition->arch(); }
    void setArch (const Arch * arch) { _edition->setArch (arch); }
    void setArch (const std::string & arch) { _edition->setArch (arch); }

    const Kind & kind() const { return _kind; }
    void setKind (const Kind & kind) { _kind = kind; }

    const std::string name() const { return _name; }
    void setName (const std::string & name) { _name = Name(name.c_str()); }

    constEditionPtr edition() const { return _edition; }
    void setEdition (constEditionPtr edition) { _edition = edition->copy(); }

    // calculate hash
    HashValue hash (void) const;

    // match operator
    bool match(constSpecPtr spec) const;
    bool equals (constSpecPtr spec) const;

    // find spec in SpecList by name
    const Spec * findByName (const SpecList &speclist, const Name & name) const;

    // copy

    constSpecPtr copy (void) const;
};

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _Spec_h
