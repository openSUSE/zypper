/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Dependency.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'dependency'
 *  contains name-op-spec
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

#include <zypp/solver/detail/Dependency.h>
#include <zypp/solver/detail/OrDependency.h>
#include <zypp/solver/detail/Version.h>
#include <zypp/Arch.h>
#include <zypp/Edition.h>

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
      
      IMPL_DERIVED_POINTER(Dependency,Spec);
      
      
      string
      Dependency::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      Dependency::toString ( const Dependency & dependency )
      {
          string res;
      
          res += dependency.name();
          if (dependency.relation() != Rel::ANY) {
      	res += " ";
      	res += dependency.relation().asString();
      	res += " ";
      
      	res += dependency.edition().asString();
          }
          if (dependency.orDep()) res += " [OR]";
          if (dependency.preDep()) res += " [PRE]";
      
          return res;
      }
      
      
      string
      Dependency::toString (const CDependencyList & dl)
      {
          string res("[");
          for (CDependencyList::const_iterator iter = dl.begin(); iter != dl.end(); iter++) {
      	if (iter != dl.begin()) res += ", ";
      	res += (*iter)->asString();
          }
          return res + "]";
      }
      
      
      
      
      ostream &
      Dependency::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const Dependency& dependency)
      {
          return os << dependency.asString();
      }
      
      //---------------------------------------------------------------------------
      
      Dependency::Dependency (const string & name, const Rel & relation, const Resolvable::Kind & kind,
      	constChannelPtr channel,
      	int epoch, const string & version, const string & release, const zypp::Arch & arch,
      	bool or_dep, bool pre_dep)
          : Spec (kind, name, epoch, version, release, arch)
          , _relation (relation)
          , _channel (channel)
          , _or_dep (or_dep)
          , _pre_dep (pre_dep)
      {
      }
      
      
      Dependency::Dependency (const string & name, const Rel & relation, const Resolvable::Kind & kind,
      	constChannelPtr channel, const Edition & edition, bool or_dep, bool pre_dep)
          : Spec (kind, name, edition)
          , _relation (relation)
          , _channel (channel)
          , _or_dep (or_dep)
          , _pre_dep (pre_dep)
      {
      }
      
      
      Dependency::Dependency (OrDependencyPtr or_dep)
          : Spec (ResTraits<zypp::Package>::kind, or_dep->name())
          , _relation (Rel::ANY)
          , _channel (NULL)
          , _or_dep (false)
          , _pre_dep (true)
      {
          or_dep->addCreatedProvide (this);
      }
      
      
      Dependency::Dependency (constXmlNodePtr node)
          : Spec (ResTraits<zypp::Package>::kind, "")
          , _relation (Rel::ANY)
          , _channel (new Channel(CHANNEL_TYPE_ANY))
          , _or_dep (false)
          , _pre_dep (false)
      {
          const char *tmp;
      
          if (!node->equals("dep")) {
      	fprintf (stderr, "Dependency::Dependency bad node\n");
      	abort();
          }
      
          setName(node->getProp ("name"));
          tmp = node->getProp ("op", NULL);
          if (tmp) {
      	_relation = Rel(tmp);
      	setEpoch (node->getIntValueDefault ("epoch", Edition::noepoch));
      	setVersion (node->getProp ("version"));
      	setRelease (node->getProp ("release"));
          }
      
          tmp = node->getProp ("arch", NULL);
          if (tmp) {
      	setArch ( Arch(node->getProp ("arch")));
          } else {
      	setArch ( Arch());
          }
      #if 0
          tmp = node->getProp ("kind", NULL);
          if (tmp) {
      	setKind (Kind (node->getProp ("kind")));
          }
      #endif
          /* FIXME: should get channel from XML */
          /* FIXME: should get arch from XML */
      }
      
      
      Dependency::~Dependency()
      {
      }
      
      //---------------------------------------------------------------------------
      
      DependencyPtr parseXml (constXmlNodePtr node)
      {
          if (node->equals("dep")) {
              return new Dependency (node);
          } else if (node->equals("or")) {
              CDependencyList or_dep_list;
      
              node = node->children();
      
              while (node) {
                  if (node->isElement()) {
                      or_dep_list.push_back (new Dependency (node));
                  }
      
                  node = node->next();
              }
      
              OrDependencyPtr or_dep = OrDependency::fromDependencyList(or_dep_list);
              return new Dependency (or_dep);
          }
      
          fprintf (stderr, "Unhandled dependency [%s]\n", node->name());
      
          return NULL;
      }
      
      
      bool
      Dependency::verifyRelation (constDependencyPtr prov) const
      {
          int compare_ret = 0;
          if (getenv ("SPEW_DEP")) fprintf (stderr, "Dependency::verifyRelation(dep: %s, prov: %s)", asString().c_str(), prov->asString().c_str());
          /* No dependency can be met by a different token name */
          if (name() != prov->name()) {
              if (getenv ("SPEW_DEP")) fprintf (stderr, "-> wrong name\n");
              return false;
          }
      
          /* No dependency can be met by a different type */
          if (kind() != prov->kind()) {
              if (getenv ("SPEW_DEP")) fprintf (stderr, "-> wrong kind(dep: %s, prov: %s)\n", kind().asString().c_str(), prov->kind().asString().c_str());
              return false;
          }
      
          /* WARNING: RC_RELATION_NONE is NOT handled */
      
          /* No specific version in the req, so return */
          if (_relation == Rel::ANY) {
              if (getenv ("SPEW_DEP")) fprintf (stderr, " (any) -> true\n");
              return true;
          }
      
          /* No specific version in the prov.  In RPM this means it will satisfy
           * any version, but debian it will not satisfy a versioned dep */
          if (prov->relation() == Rel::ANY) {
              if (GVersion.hasProperty (VERSION_PROP_PROVIDE_ANY)) {
                  if (getenv ("SPEW_DEP")) fprintf (stderr, " provides (any) matches GVersion -> true\n");
                  return true;
              }
              else {
                  if (getenv ("SPEW_DEP")) fprintf (stderr, " provides (any) does not match GVersion -> false\n");
                  return false;
              }
          }
      
          if (!channel()->equals (prov->channel()))
          {
              if (getenv ("SPEW_DEP")) fprintf (stderr, " wrong channel -> false\n");
              return false;
          }
      
          SpecPtr newdepspec;
          SpecPtr newprovspec;
      
          if (epoch() >= 0 && prov->epoch() >= 0) {
              /* HACK: This sucks, but I don't know a better way to compare elements one at a time */
              newdepspec = new Spec(kind(), "", epoch());
              newprovspec = new Spec(prov->kind(), "", prov->epoch());
              compare_ret = GVersion.compare (newprovspec, newdepspec);
          } else if (prov->epoch() > 0 ) {
              if (GVersion.hasProperty (VERSION_PROP_IGNORE_ABSENT_EPOCHS)) {
                  compare_ret = 0;
              }
              else {
                  compare_ret = 1;
              }
          } else if (epoch() > 0 ) {
              compare_ret = Edition::noepoch;
          }
          if (getenv ("SPEW_DEP")) fprintf (stderr, "epoch(%d), prov->epoch(%d) -> compare_ret %d\n", epoch(), prov->epoch(), compare_ret);
          if (compare_ret == 0) {
              if (GVersion.hasProperty (VERSION_PROP_ALWAYS_VERIFY_RELEASE)
                  || (!(release().empty() || prov->release().empty()))) {
                  newdepspec = new Spec(kind(), "", Edition::noepoch, version(), release());
                  newprovspec = new Spec(prov->kind(), "", Edition::noepoch, prov->version(), prov->release());
              } else {
                  newdepspec = new Spec(kind(), "", Edition::noepoch, version());
                  newprovspec = new Spec(prov->kind(), "", Edition::noepoch, prov->version());
              }
              compare_ret = GVersion.compare (newprovspec, newdepspec);
          }
          if (getenv ("SPEW_DEP")) fprintf (stderr, " (compare result -> %d)", compare_ret);
      
          if (compare_ret < 0
              && ((prov->relation() == Rel::GT || prov->relation() == Rel::GE )
                  || (_relation == Rel::LT || _relation == Rel::LE )))
          {
              if (getenv ("SPEW_DEP")) fprintf (stderr, " -> true\n");
              return true;
          } else if (compare_ret > 0 
                     && ((prov->relation() == Rel::LT || prov->relation() == Rel::LE )
                         || (_relation == Rel::GT || _relation == Rel::GE )))
          {
              if (getenv ("SPEW_DEP")) fprintf (stderr, " -> true\n");
              return true;
          } else if (compare_ret == 0
                     && (((prov->relation() == Rel::EQ || prov->relation() == Rel::GE || prov->relation() == Rel::LE)
                          && (_relation == Rel::EQ || _relation == Rel::GE || _relation == Rel::LE))
                         || ((prov->relation() == Rel::LT) && (_relation == Rel::LT))
                         || ((prov->relation() == Rel::GT) && (_relation == Rel::GT))))
          {
              if (getenv ("SPEW_DEP")) fprintf (stderr, " -> true\n");
              return true;
          }
          if (getenv ("SPEW_DEP")) fprintf (stderr, " -> false\n");
          return false;
      }
      
      //---------------------------------------------------------------------------
      
      #if 0
      xmlNode *
      rc_resItem_dep_or_slist_to_xml_node (RCResItemDepSList *dep)
      {
          xmlNode *or_node;
          const RCResItemDepSList *dep_iter;
      
          or_node = xmlNewNode (NULL, "or");
      
          dep_iter = dep;
          while (dep_iter) {
      	RCResItemDep *dep_item = (RCResItemDep *)(dep_iter->data);
      	xmlAddChild (or_node, rc_resItem_dep_to_xml_node (dep_item));
      	dep_iter = dep_iter->next;
          }
      
          return or_node;
      } /* rc_resItem_dep_or_slist_to_xml_node */
      
      xmlNode *
      rc_resItem_dep_to_xml_node (RCResItemDep *dep_item)
      {
          RCResItemSpec *spec = (RCResItemSpec *) dep_item;
          xmlNode *dep_node;
      
          if (rc_resItem_dep_is_or (dep_item)) {
      	RCResItemDepSList *dep_or_slist;
      	dep_or_slist = rc_dep_string_to_or_dep_slist
      	    (rc_resItem_spec_get_name (spec));
      	dep_node = rc_resItem_dep_or_slist_to_xml_node (dep_or_slist);
      	rc_resItem_dep_slist_free (dep_or_slist);
      	return dep_node;
          }
      
          dep_node = xmlNewNode (NULL, "dep");
      
          xmlSetProp (dep_node, "name", rc_resItem_spec_get_name (spec));
      
          if (rc_resItem_dep_get_relation (dep_item) != Relation::Any) {
      	xmlSetProp (dep_node, "op",
      		    rc_resItem_relation_to_string (
      			rc_resItem_dep_get_relation (dep_item), false));
      
      	if (rc_resItem_spec_has_epoch (spec)) {
      	    gchar *tmp;
      
      	    tmp = g_strdup_printf ("%d", rc_resItem_spec_get_epoch (spec));
      	    xmlSetProp (dep_node, "epoch", tmp);
      	    g_free (tmp);
      	}
      
      	if (spec->version) {
      	    xmlSetProp (dep_node, "version", spec->version);
      	}
      
      	if (spec->release) {
      	    xmlSetProp (dep_node, "release", spec->release);
      	}
          }
      
          return (dep_node);
      } /* rc_resItem_dep_to_xml_node */
      
      #endif
        
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

