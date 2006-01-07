/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

#ifndef ZYPP_SOLVER_TEMPORARY_SPEC_H
#define ZYPP_SOLVER_TEMPORARY_SPEC_H

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/temporary/Hash.h"
#include "zypp/solver/temporary/SpecPtr.h"
#include "zypp/solver/temporary/XmlNode.h"

#include "zypp/Edition.h"
#include "zypp/ResObject.h"

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
//	CLASS NAME : Spec
/**
 *
 **/
typedef std::string Name;
class Spec : public base::ReferenceCounted, private base::NonCopyable {


  private:
    Resolvable::Kind _kind;
    Name _name;
    Edition _edition;
    Arch _arch;

  public:
    typedef std::list<Spec> SpecList;

    Spec( const Resolvable::Kind & kind,
	  const std::string & name,
	  int epoch = Edition::noepoch,
	  const std::string & version = "",
	  const std::string & release = "",
	  const zypp::Arch & arch = Arch());

    Spec (const Resolvable::Kind & kind, const std::string & name, const Edition & edition, const Arch & arch = Arch());

    Spec (XmlNode_constPtr node);

    virtual ~Spec();

    // ---------------------------------- I/O

    const xmlNodePtr asXmlNode (const char *name) const;

    static std::string toString ( const Spec & spec, bool full = false );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream&, const Spec& );

    std::string asString ( bool full = false ) const;

    // ---------------------------------- accessors

    const std::string & version() const { return _edition.version(); }
    void setVersion (const std::string & version);

    const std::string & release() const { return _edition.release(); }
    void setRelease (const std::string & release);

    const int epoch() const { return _edition.epoch(); }
    void setEpoch (int epoch);
    bool hasEpoch() const { return _edition.epoch() > 0; }

    const Arch & arch() const { return _arch; }
    void setArch (const Arch & arch) { _arch = arch; }
    void setArch (const std::string & arch) { _arch = Arch(arch); }

    const Resolvable::Kind & kind() const { return _kind; }
    void setKind (const Resolvable::Kind & kind) { _kind = kind; }

    const std::string name() const { return _name; }
    void setName (const std::string & name) { _name = Name(name.c_str()); }

    const Edition & edition() const { return _edition; }
    void setEdition (const Edition & edition) { _edition = edition; }

    // calculate hash
    HashValue hash (void) const;

    // match operator
    bool match(Spec_constPtr spec) const;
    bool equals (Spec_constPtr spec) const;

    // find spec in SpecList by name
    const Spec * findByName (const SpecList &speclist, const Name & name) const;
    static int compare (Spec_constPtr spec1, Spec_constPtr spec2);

    // copy

    Spec_constPtr copy (void) const;
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

#endif // ZYPP_SOLVER_TEMPORARY_SPEC_H
