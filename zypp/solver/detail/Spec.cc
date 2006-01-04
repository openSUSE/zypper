/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Spec.cc
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

#include "zypp/solver/detail/Spec.h"

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
      
      IMPL_PTR_TYPE(Spec);
      
      //---------------------------------------------------------------------------
      
      UstringHash Name::_nameHash;
      
      //---------------------------------------------------------------------------
      
      string
      Spec::asString ( bool full ) const
      {
          return toString (*this, full);
      }
      
      
      string
      Spec::toString ( const Spec & spec, bool full )
      {
          string res;
      
          if (full
      	|| (spec.kind() != "Package"
      	    && spec.kind() != "ResObject")) {
      	res += spec.kind().asString();
      	res += ":";
          }
      
          res += spec.name();
      
          string ed = spec.edition().asString();
          if (!ed.empty() &&
              ed != "EDITION-UNSPEC") {
      	res += "-";
      	res += ed;
          }

          if (spec.arch() != Arch_noarch) {
              res += ".";
              res += spec.arch().asString();
          }
          

          return res;
      }
      
      
      ostream &
      Spec::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const Spec& spec)
      {
          return os << spec.asString();
      }
      
      //---------------------------------------------------------------------------
      
      Spec::Spec (const Resolvable::Kind & kind, const string & name, const Edition & edition, const Arch & arch)
          : _kind (kind)
          , _name (Name (name))
          , _edition (edition)
          , _arch (arch)
      {
      }
      
      
      Spec::Spec ( const Resolvable::Kind & kind, const string & name, int epoch, const string & version, const string & release, const Arch & arch)
          : _kind (kind)
          , _name (Name (name))
          , _edition (Edition (version, release, epoch))
          , _arch(arch)
      {
      }
      
      
      Spec::Spec (XmlNode_constPtr node)
          : _kind (ResTraits<zypp::Package>::kind) // Fixme: should be type like unknown
      {
          fprintf (stderr, "Spec::Spec (XmlNode_constPtr node)\nNot implemented\n");
          abort();
      }
      
      
      Spec::~Spec()
      {
      }
      
      
      // needed during xml parsing (-> XmlParser)
      
      Spec_constPtr
      Spec::copy (void) const
      {
          return new Spec (_kind, _name, _edition);
      }
      
      
      #if 0
      xmlNode *
      rc_resItem_spec_to_xml_node (RCResItemSpec *spec)
      {
          xmlNode *spec_node;
          char buffer[128];
      
          spec_node = xmlNewNode (NULL, "spec");
      
          xmlNewTextChild (spec_node, NULL, "name",
      		     g_quark_to_string (spec->nameq));
      
          if (spec->has_epoch) {
      	g_snprintf (buffer, 128, "%d", spec->epoch);
      	xmlNewTextChild (spec_node, NULL, "epoch", buffer);
          }
      
          xmlNewTextChild (spec_node, NULL, "version", spec->version);
      
          if (spec->release)
      	xmlNewTextChild (spec_node, NULL, "release", spec->release);
      
          xmlNewTextChild (spec_node, NULL, "arch",
      		     rc_arch_to_string (spec->arch));
      
          return spec_node;
      }
      
      #endif
      
      //---------------------------------------------------------------------------
      
      
      HashValue
      Spec::hash (void) const
      {
          HashValue ret = _edition.epoch() + 1;
          const char *spec_strs[3], *p;
          int i;
      
          spec_strs[0] = _name.asString().c_str();
          spec_strs[1] = _edition.version().c_str();
          spec_strs[2] = _edition.release().c_str();
      
          for (i = 0; i < 3; ++i) {
      	p = spec_strs[i];
      	if (p) {
      	    for (p += 1; *p != '\0'; ++p) {
      		ret = (ret << 5) - ret + *p;
      	    }
      	} else {
      	    ret = ret * 17;
      	}
          }
      
          return ret;
      }
      
      
      const Spec *
      Spec::findByName (const SpecList &speclist, const Name & name) const
      {
          const Spec *spec = NULL;
          for (SpecList::const_iterator iter = speclist.begin(); iter != speclist.end(); iter++) {
      	if ((*iter).name() == name) {
      	    spec = &(*iter);
      	    break;
      	}
          }
          return spec;
      }
      
      
      bool
      Spec::match(Spec_constPtr spec) const {
          return ((_kind == spec->kind())
        	&& (_name == spec->name())
      	&& Edition::compare( _edition, spec->edition()) == 0);
      }
      
      
      bool
      Spec::equals(Spec_constPtr spec) const {
      //fprintf (stderr, "<%s> equals <%s>\n", asString(true).c_str(), spec->asString(true).c_str());
          return ((_kind == spec->kind())
        	&& (_name == spec->name())
      	&& Edition::compare( _edition, spec->edition()) == 0);
      }

      void Spec::setVersion (const std::string & version) {
          Edition newEdition(version,
                             _edition.release(),
                             _edition.epoch());
          _edition = newEdition;
      }

      void Spec::setRelease (const std::string & release) {
          Edition newEdition(_edition.version(),
                             release,
                             _edition.epoch());
          _edition = newEdition;
      }

      void Spec::setEpoch (int epoch) {
          Edition newEdition(_edition.version(),
                             _edition.release(),
                             epoch);
          _edition = newEdition;
      }
        

      int  Spec::compare (Spec_constPtr spec1, Spec_constPtr spec2) {
          int rc = 0;

          const string name1 = spec1->name();
          const string name2 = spec2->name();
          if (! (name1.empty() && name2.empty()))
          {
              rc = name1.compare (name2);
          }
          if (rc) return rc;

          rc = Edition::compare (spec1->edition(),
                                 spec2->edition());

          return rc;
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

