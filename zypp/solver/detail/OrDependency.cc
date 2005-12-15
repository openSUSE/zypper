/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* OrDependency.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'or dependency'
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

#include <zypp/solver/detail/OrDependency.h>
#include <zypp/solver/detail/Dependency.h>

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
      
      IMPL_BASE_POINTER(OrDependency);
      
      OrDependency::OrDependencyTable OrDependency::_or_dep_table;
      
      //---------------------------------------------------------------------------
      
      string
      OrDependency::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      OrDependency::toString ( const OrDependency & dependency )
      {
          string res ("<ordependency/>");
      
          return res;
      }
      
      
      ostream &
      OrDependency::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream & os, const OrDependency & dependency)
      {
          return os << dependency.asString();
      }
      
      //---------------------------------------------------------------------------
      
      OrDependency::OrDependency (string & dep, const CDependencyList & split_ors)
          : _or_dep (dep)
          , _split_ors (split_ors)
          , _ref(1)
      {
      }
      
      
      OrDependency::OrDependency (constXmlNodePtr node)
      {
      }
      
      
      OrDependency::~OrDependency()
      {
      }
      
      
      //---------------------------------------------------------------------------
      
      OrDependencyPtr
      OrDependency::fromDependencyList (const CDependencyList & deplist)
      {
          string depstr = dependencyListToString(deplist);
      
          OrDependencyTable::iterator pos = _or_dep_table.find(depstr);
          if (pos != _or_dep_table.end()) {
      	(*pos).second->incRef();
      	return (*pos).second;
          }
      
          OrDependencyPtr or_dep = new OrDependency (depstr, deplist);
          _or_dep_table[depstr] = or_dep;
          return or_dep;
      }
      
      
      OrDependencyPtr
      OrDependency::fromString (const char *dep)
      {
          string depstr = string (dep);
      
          OrDependencyTable::iterator pos = _or_dep_table.find(depstr);
          if (pos != _or_dep_table.end()) {
      	(*pos).second->incRef();
      	return (*pos).second;
          }
      
          CDependencyList deplist = stringToDependencyList (dep);
          OrDependencyPtr or_dep = new OrDependency (depstr, deplist);
          _or_dep_table[depstr] = or_dep;
          return or_dep;
      
      }
      
      
      string
      OrDependency::dependencyListToString (const CDependencyList & deplist)
      {
          string str ("(||");
      
          for (CDependencyList::const_iterator dep = deplist.begin(); dep != deplist.end(); dep++) {
      	if (dep != deplist.begin())
      	    str += "|";
      
      	str += (*dep)->name();
      
      	Relation relation = (*dep)->relation();
      
      	if (relation != Relation::Any) {
      	    str += "&";
      	    str += relation.asString();
      	    str += "&";
      
      	    if ((*dep)->epoch() >= 0) {
      		str += stringutil::form("%d:", (*dep)->epoch());
      	    }
      
      	    str += (*dep)->version();
      
      	    string rel = (*dep)->release();
      	    if (!rel.empty()) {
      	 	str += "-";
      		str += rel;
      	    }
      	}
      
          }
      
          str += ")";
      
          return str;
      }
      
      
      CDependencyList
      OrDependency::stringToDependencyList (const char *s)
      {
          const char *p, *zz;
          CDependencyList out_dep;
          bool have_more = true;
      
          if (strncmp (s, "(||", 3)) {
      	fprintf (stderr, "'%s' is not a 'munged or' string!\n", s);
      	return out_dep;
          }
      
          s += 3;
      
          zz = strchr (s, ')');
      
          if (!zz)
      	return out_dep;
      
          /* s now points to the start of the first thing */
          do {
      	char *z;
      	SpecPtr spec;
      	char *name;
      	Relation relation = Relation::Any;
      	EditionPtr edition = NULL;
      
      	/* grab the name */
      
      	z = strchr (s, '|');
      	p = strchr (s, '&');
      
      	if (!z) {
      	    have_more = false;
      	}
      	else {
      	    /* We don't want to get a p from a later element. */
      	    if (p > z)
      		p = NULL;
      	}
      
      	name = strndup (s, p ? p - s : (z ? z - s : zz - s));
      
      	if (p) {
      	    char *e;
      	    char op[4];
      	    char *vstr;
      
      	    /* We need to parse version things */
      	    p++;
      	    e = strchr (p, '&');
      	    if (!e || e-p > 3) {
      		/* Bad. */
      		fprintf (stderr, "Couldn't parse ver str [%s]\n", p);
      	    }
      
      	    /* text between p and e is an operator */
      	    strncpy (op, p, e - p);
      	    op[e - p] = 0;
      	    relation = Relation::parse (op);
      
      	    e++;
      	    if (z) {
      		p = z;
      	    } else {
      		p = zz;
      	    }
      
      	    /* e .. p is the epoch:version-release */
      	    vstr = strndup (e, p - e);
      
      	    EditionPtr edition = Edition::fromString (vstr);
      
      	    free ((void *)vstr);
      
      	}
      
      	DependencyPtr dep = new Dependency (name, relation, Kind::Package, NULL, edition);
      
      	out_dep.push_back (dep);
      	free ((void *)name);
      
      	s = z + 1;
      
      	if (p == zz)
      	    have_more = false;
          } while (have_more);
      
          return out_dep;
      }
      
      
      void
      OrDependency::addCreatedProvide (constDependencyPtr dep)
      {
          _created_provides.push_back (dep);
      }
      
      
      constDependencyPtr
      OrDependency::find (const char *dep)
      {
          string depstr (dep);
      
          OrDependencyTable::iterator pos = _or_dep_table.find(depstr);
          if (pos != _or_dep_table.end()) {
      	return new Dependency ((*pos).second);
          }
      
          return NULL;
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

