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
#include <zypp/Edition.h>
#include <zypp/ResObject.h>

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
//
      //	CLASS NAME : Spec
      /**
       *
       **/
      
      class Spec : public CountedRep {
          REP_BODY(Spec);
      
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
      
          Spec (constXmlNodePtr node);
      
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
          bool match(constSpecPtr spec) const;
          bool equals (constSpecPtr spec) const;
      
          // find spec in SpecList by name
          const Spec * findByName (const SpecList &speclist, const Name & name) const;
          static int compare (constSpecPtr spec1, constSpecPtr spec2);
      
          // copy
      
          constSpecPtr copy (void) const;
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

#endif // _Spec_h
