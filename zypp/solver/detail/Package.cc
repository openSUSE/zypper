/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Package.cc
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

#include <zypp/solver/detail/utils.h>
#include <zypp/solver/detail/Package.h>
#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>
#include <zypp/solver/detail/PackageUpdate.h>
#include <zypp/solver/detail/World.h>
#include <zypp/solver/detail/XmlNode.h>
#include <zypp/base/Logger.h>
#include <zypp/CapFactory.h>
#include <zypp/CapSet.h>


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
      
      IMPL_DERIVED_POINTER(Package,ResItem);
      
      struct DepTable {
          CapSet requires;
          CapSet provides;
          CapSet conflicts;
          CapSet obsoletes;
          CapSet children;
          CapSet suggests;
          CapSet recommends;
      };
      
      //---------------------------------------------------------------------------

      static Capability 
      parseXmlDep (constXmlNodePtr node) {
          const char *tmp;
          string epoch,version,release,name = "";
          Arch arch = Arch_noarch;
          Rel relation;
          CapFactory  factory;                          
      
          if (!node->equals("dep")) {
              fprintf (stderr, "parseXmlDep bad node\n");
              abort();
          }
      
          name = node->getProp ("name");
          tmp = node->getProp ("op", NULL);
          if (tmp) {
              relation = Rel(tmp);
              epoch = node->getIntValueDefault ("epoch", Edition::noepoch);
              version = node->getProp ("version");
              release = node->getProp ("release");
          }
      
          tmp = node->getProp ("arch", NULL);
          if (tmp) {
              arch = Arch(node->getProp ("arch"));
          } else {
              arch =  Arch();
          }
          
          return  factory.parse ( ResTraits<zypp::Package>::kind,
                                  name,
                                  relation,
                                  Edition (version, release, epoch));
      }
        
      static void
      extract_dep_info (constXmlNodePtr iter, struct DepTable & dep_table)
      {
          if (iter->equals("requires")) {
      	constXmlNodePtr iter2;
      	
      	iter2 = iter->children();
      
      	while (iter2) {
      	    if (!iter2->isElement()) {
      		iter2 = iter2->next();
      		continue;
      	    }
      
      	    dep_table.requires.insert(parseXmlDep (iter2));
      	    iter2 = iter2->next();
      	}
      
          } else if (iter->equals("recommends")) {
      	constXmlNodePtr iter2;
      
      	iter2 = iter->children();
      
      	while (iter2) {
      	    if (!iter2->isElement()) {
      		iter2 = iter2->next();
      		continue;
      	    }
      
      	    dep_table.recommends.insert (parseXmlDep (iter2));
      	    iter2 = iter2->next();
      	}
      
          } else if (iter->equals("suggests")) {
      	constXmlNodePtr iter2;
      
      	iter2 = iter->children();
      
      	while (iter2) {
      	    if (!iter2->isElement()) {
      		iter2 = iter2->next();
      		continue;
      	    }
      
      	    dep_table.suggests.insert (parseXmlDep (iter2));
      	    iter2 = iter2->next();
      	}
      
          } else if (iter->equals("conflicts")) {
      	XmlNodePtr iter2;
      	bool all_are_obs = false, this_is_obs = false;
      	const char *obs;
      
      	iter2 = iter->children();
      
      	obs = iter->getProp ("obsoletes", NULL);
      	if (obs) {
      	    all_are_obs = true;
      	    free ((void *)obs);
      	}
      
      	while (iter2) {
      
      	    if (!iter2->isElement()) {
      		iter2 = iter2->next();
      		continue;
      	    }
      
      	    Capability dep = parseXmlDep(iter2);
      
      	    if (! all_are_obs) {
      		this_is_obs = false;
      		obs = iter2->getProp ("obsoletes", NULL);
      		if (obs) {
      		    this_is_obs = true;
      		    free ((void *)obs);
      		}
      	    }
      		
      	    if (all_are_obs || this_is_obs) {
      		dep_table.obsoletes.insert (dep);
      	    } else {
      		dep_table.conflicts.insert (dep);
      	    }
      		
      	    iter2 = iter2->next();
      	}
      
          } else if (iter->equals("obsoletes")) {
      	constXmlNodePtr iter2;
      
      	iter2 = iter->children();
      
      	while (iter2) {
      	    if (!iter2->isElement()) {
      		iter2 = iter2->next();
      		continue;
      	    }
      
      	    dep_table.obsoletes.insert (parseXmlDep (iter2));
      	    iter2 = iter2->next();
      	}
      
          } else if (iter->equals("provides")) {
      	constXmlNodePtr iter2;
      
      	iter2 = iter->children();
      
      	while (iter2) {
      	    if (!iter2->isElement()) {
      		iter2 = iter2->next();
      		continue;
      	    }
      
      	    dep_table.provides.insert (parseXmlDep (iter2));
      	    iter2 = iter2->next();
      	}
      
          } else if (iter->equals("children")) {
      	constXmlNodePtr iter2;
      
      	iter2 = iter->children();
      
      	while (iter2) {
      	    if (!iter2->isElement()) {
      		iter2 = iter2->next();
      		continue;
      	    }
      
      	    dep_table.children.insert (parseXmlDep (iter2));
      	    iter2 = iter2->next();
      	}
          }
      }
      
      //---------------------------------------------------------------------------
      
      
      string
      Package::asString ( bool full ) const
      {
          return toString (*this, full);
      }
      
      
      string
      Package::toString ( const PackageUpdateList & l, bool full )
      {
          string ret ("[");
          for (PackageUpdateList::const_iterator i = l.begin(); i != l.end(); i++) {
      	if (i != l.begin()) ret += ", ";
      	ret += (*i)->asString(full);
          }
          return ret + "]";
      }
      
      string
      Package::toString ( const Package & package, bool full )
      {
          string ret;
          ret += ResItem::toString(package, full);
          if (full) {
      //	if (package._section != NULL) ret += (string ("<section '") + package._section->asString() + "'/>");
      //	if (!package._pretty_name.empty()) ret += (string ("<pretty_name '") + package._pretty_name + "'/>");
      //	if (!package._summary.empty()) ret += (string ("<summary '") + package._summary + "'/>");
      //	if (!package._description.empty()) ret += (string ("<description '") + package._description + "'/>");
      	ret += (string ("<history '") + toString(package._history) + "'/>");
          }
          return ret;
      }
      
      
      ostream &
      Package::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const Package& package)
      {
          return os << package.asString();
      }
      
      //---------------------------------------------------------------------------
        
      Package::Package (constChannelPtr channel)
          : ResItem (ResTraits<zypp::Package>::kind, "")
          , _section (NULL)
          , _pretty_name ("")
          , _summary ("")
          , _description ("")
          , _package_filename ("")
          , _signature_filename ("")
          , _install_only (false)
          , _package_set (false)
          , _id ("")
      {
          setChannel (channel);
      }


      Package::Package (constChannelPtr channel,
                        const string & name,
                        const Edition & edition,
                        const Arch arch)

          : ResItem (ResTraits<zypp::Package>::kind, "")
          , _section (NULL)
          , _pretty_name ("")
          , _summary ("")
          , _description ("")
          , _package_filename ("")
          , _signature_filename ("")
          , _install_only (false)
          , _package_set (false)
          , _id ("")
      {

          setChannel (channel);

          shared_ptr<zypp::detail::PackageImpl> pkgImpl;
          zypp::Package::Ptr pkg( zypp::detail::makeResolvableAndImpl( name, edition, arch,
                                                                       pkgImpl ) );
          _resObject = pkg;
      }
                  
      
      Package::Package (constXmlNodePtr node, constChannelPtr channel)
          : ResItem (ResTraits<zypp::Package>::kind, "")
          , _section (NULL)
          , _pretty_name ("")
          , _summary ("")
          , _description ("")
          , _package_filename ("")
          , _signature_filename ("")
          , _install_only (false)
          , _package_set (false)
          , _id ("")
      {
          string name = "";
          int epoch = Edition::noepoch;
          string version = "";
          string release = "";
          Arch arch = Arch_noarch;
          
          if (!node->equals("package")) {
              fprintf (stderr, "Package::Package() not a package node\n");
              exit (1);
          }
      
          struct DepTable dep_table;
      
          setChannel (channel);
      
          constXmlNodePtr iter = node->children();
      
          while (iter) {
              bool extracted_deps = false;
      
              if (iter->equals("name")) {	  		name = iter->getContent();
              } else if (iter->equals("epoch")) {		epoch = atoi (iter->getContent());
              } else if (iter->equals("version")) {		version = iter->getContent();
              } else if (iter->equals("release")) {		release = iter->getContent();
              } else if (iter->equals("summary")) {		_summary = strdup (iter->getContent());
              } else if (iter->equals("description")) {	_description = strdup (iter->getContent());
              } else if (iter->equals("section")) {		_section = new Section (iter->getContent());
              } else if (iter->equals("arch")) {		arch = Arch(iter->getContent());
              } else if (iter->equals("filesize")) {	
                  const char *tmp = iter->getContent();
                  setFileSize (tmp && *tmp ? atoi (tmp) : 0);
                  free ((void *)tmp);
              } else if (iter->equals("installedsize")) {
                  const char *tmp = iter->getContent();
                  setInstalledSize (tmp && *tmp ? atoi (tmp) : 0);
                  free ((void *)tmp);
              } else if (iter->equals("install_only")) {	_install_only = true;
              } else if (iter->equals("package_set")) {	_package_set = true;
              } else if (iter->equals("history")) {
                  constXmlNodePtr iter2;
      
                  iter2 = iter->children();
      
                  while (iter2) {
                      if (!iter2->isElement()) {
                          iter2 = iter2->next();
                          continue;
                      }
      
                      PackageUpdatePtr update = new PackageUpdate (iter2, this);
                      addUpdate (update);
      
                      iter2 = iter2->next();
                  }
              } else if (iter->equals("deps")) {
                  constXmlNodePtr iter2;
      
                  for (iter2 = iter->children(); iter2; iter2 = iter2->next()) {
                      if (!iter2->isElement())
                          continue;
      
                      extract_dep_info (iter2, dep_table);
                  }
      
                  extracted_deps = true;
              }
              else {
                  if (!extracted_deps)
                      extract_dep_info (iter, dep_table);
                  else {
                      /* FIXME: Bitch to the user here? */
                  }
              }
      
              iter = iter->next();
          }
      
          if (!dep_table.children.empty()) {
              // children are used in package sets
              // treat them as normal requires
              //
#warning Children are handled as requires
              CapSet::const_iterator iter;
              for (iter = dep_table.children.begin(); iter != dep_table.children.end(); iter++)
              {
                  dep_table.requires.insert (*iter);
              }
          }
      
      
          // check if we're already listed in the provides
          // if not, provide ourself
          CapFactory  factory;                                    
          Capability selfdep = factory.parse ( ResTraits<zypp::Package>::kind,
                                             name,
                                             Rel::EQ,
					       Edition( version, release, zypp::str::numstring(epoch)));

          
          CapSet::const_iterator piter;
          for (piter = dep_table.provides.begin(); piter != dep_table.provides.end(); piter++) {
              if ((*piter) == selfdep)
              {
                  break;
              }
          }
          if (piter == dep_table.provides.end()) {			// no self provide found, construct one
              _DBG("RC_SPEW") << "Adding self-provide [" << selfdep.asString() << "]" << endl;
              dep_table.provides.insert (selfdep);
          }
      
          Dependencies deps;
          deps.setRequires          (dep_table.requires);
          deps.setProvides          (dep_table.provides);
          deps.setConflicts         (dep_table.conflicts);
          deps.setObsoletes         (dep_table.obsoletes);
          deps.setSuggests          (dep_table.suggests);
          deps.setRecommends        (dep_table.recommends);
          setDependencies (deps);
          
          if (!_history.empty()) {
      
              /* If possible, we grab the version info from the most
                 recent update. */
      
              PackageUpdatePtr update = _history.front();
      
              epoch = update->package()->epoch();
              version = update->package()->version();
              release = update->package()->release();
      
          }
#if 0 //Is this really needed ?          
          else {
      	
              /* Otherwise, try to find where the package provides itself,
                 and use that version info. */
      	
              if (!provides().empty())
                  for (CapSet::const_iterator iter = provides().begin(); iter != provides().end(); iter++) {	    
                      if ((*iter)->relation() == Rel::EQ &&
                          ((*iter)->name() == name))
                      {
                          epoch = (*iter)->epoch();
                          version = (*iter)->version();
                          release = (*iter)->release();
                          break;
                      }
                  }
          }
#endif
          
          Edition     _edition( version, release, zypp::str::numstring(epoch) );
          shared_ptr<zypp::detail::PackageImpl> pkgImpl;
          zypp::Package::Ptr pkg( zypp::detail::makeResolvableAndImpl( name, _edition, arch,
                                                                       pkgImpl ) );
          _resObject = pkg;
      }
      
      Package::~Package()
      {
      }
      
      //---------------------------------------------------------------------------
      
      
      void
      Package::addUpdate (PackageUpdatePtr update)
      {
          if (update == NULL) return;
      
          assert (update->package() == NULL || update->package() == this);
      
          update->setPackage(this);
      
          if (_history.empty()) {
      	_history.push_back (update);
          } else {
      #warning addUpdate incomplete
      #if 1
      	for (PackageUpdateList::iterator iter = _history.begin(); iter != _history.end(); iter++) {
      	    int result = Spec::compare ((SpecPtr)update, (SpecPtr)(*iter));
      
      	    if (result > 0 || (result == 0 && update->parent() != NULL)) {
      		_history.insert (iter, update);					// = g_slist_insert_before (package->history, l, update);
      		break;
      	    } else if (iter == _history.end() ||					// FIXME list.last() ?
      		       (result == 0 && update->parent() == NULL)) {
      		_history.insert (++iter, update);					// = g_slist_insert_before (package->history, l->next, update);
      		break;
      	    }
      	}
      #endif
          }
      }
      
      
      PackageUpdatePtr
      Package::getLatestUpdate (void) const
      {
          WorldPtr world;
      
          if (_history.empty()) {
      	return NULL;
          }
      
          PackageUpdatePtr latest = _history.back();
          /* if the absolute latest is not a patch, just return that */
          if (latest->parent() == NULL) {
      	return latest;
          }
      
          world = World::globalWorld();
      
          for (PackageUpdateList::const_iterator l = _history.begin(); l != _history.end(); l++) {
      	PackageUpdatePtr update = *l;
      	constResItemPtr installed;
      	
      	if (!update->equals (latest)) {
      	    return NULL;
      	}
      
      	/* found a non-patch package equal to the latest, so use that */
      	if (update->parent() == NULL) {
      	    return update;
      	}
      
      	/* see if the required parent for this patch is installed */
        
      	installed = world->findInstalledResItem (update->parent());
      
      	if (installed != NULL &&
      	    installed->equals(update->parent()))
      	    return update;
          }
      
          /* no suitable update found */
          return NULL;
      }
      
      
      #if 0
      xmlNode *
      rc_package_to_xml_node (RCPackage *package)
      {
          xmlNode *package_node;
          xmlNode *tmp_node;
          xmlNode *deps_node;
          RCResItem *r;
          RCResItemSpec *spec;
          RCPackageUpdateSList *history_iter;
          int i;
          char buffer[128];
          char *tmp_str;
      
          r = RC_RESOLVABLE (package);
          spec = rc_resItem_get_spec (r);
      
          package_node = xmlNewNode (NULL, "package");
      
          xmlNewTextChild (package_node, NULL, "name", rc_resItem_get_name (r));
      
          if (spec->has_epoch) {
      	g_snprintf (buffer, 128, "%d", spec->epoch);
      	xmlNewTextChild (package_node, NULL, "epoch", buffer);
          }
      
          xmlNewTextChild (package_node, NULL, "version", spec->version);
      
          if (spec->release) {
      	xmlNewTextChild (package_node, NULL, "release", spec->release);
          }
      
          tmp_str = sanitize_string (package->summary);
          xmlNewTextChild (package_node, NULL, "summary", tmp_str);
          g_free (tmp_str);
      
          tmp_str = sanitize_string (package->description);
          xmlNewTextChild (package_node, NULL, "description", tmp_str);
          g_free (tmp_str);
      
          xmlNewTextChild (package_node, NULL, "arch",
      		     rc_arch_to_string (spec->arch));
      
          xmlNewTextChild (package_node, NULL, "section",
      		     rc_package_section_to_string (package->section));
      
          g_snprintf (buffer, 128, "%u", rc_resItem_get_file_size (r));
          xmlNewTextChild (package_node, NULL, "filesize", buffer);
      
          g_snprintf (buffer, 128, "%u", rc_resItem_get_installed_size (r));
          xmlNewTextChild (package_node, NULL, "installedsize", buffer);
      
          if (package->install_only) {
      	xmlNewTextChild (package_node, NULL, "install_only", "1");
          }
      
          if (package->package_set) {
      	xmlNewTextChild (package_node, NULL, "package_set", "1");
          }
      
          if (package->history) {
      	tmp_node = xmlNewChild (package_node, NULL, "history", NULL);
      	for (history_iter = package->history; history_iter;
      	     history_iter = history_iter->next)
      	{
      	    RCPackageUpdate *update = (RCPackageUpdate *)(history_iter->data);
      	    xmlAddChild (tmp_node, rc_package_update_to_xml_node (update));
      	}
          }
      
          deps_node = xmlNewChild (package_node, NULL, "deps", NULL);
      
          if (r->requires_a) {
      	tmp_node = xmlNewChild (deps_node, NULL, "requires", NULL);
      	for (i = 0; i < r->requires_a->len; i++) {
      	    RCResItemDep *dep = r->requires_a->data[i];
      
      	    xmlAddChild (tmp_node, rc_resItem_dep_to_xml_node (dep));
      	}
          }
      
          if (r->recommends_a) {
      	tmp_node = xmlNewChild (deps_node, NULL, "recommends", NULL);
      	for (i = 0; i < r->recommends_a->len; i++) {
      	    RCResItemDep *dep = r->recommends_a->data[i];
      
      	    xmlAddChild (tmp_node, rc_resItem_dep_to_xml_node (dep));
      	}
          }
      
          if (r->suggests_a) {
      	tmp_node = xmlNewChild (deps_node, NULL, "suggests", NULL);
      	for (i = 0; i < r->suggests_a->len; i++) {
      	    RCResItemDep *dep = r->suggests_a->data[i];
      
      	    xmlAddChild (tmp_node, rc_resItem_dep_to_xml_node (dep));
      	}
          }
      
          if (r->conflicts_a) {
      	tmp_node = xmlNewChild (deps_node, NULL, "conflicts", NULL);
      	for (i = 0; i < r->conflicts_a->len; i++) {
      	    RCResItemDep *dep = r->conflicts_a->data[i];
      
      	    xmlAddChild (tmp_node, rc_resItem_dep_to_xml_node (dep));
      	}
          }
      
          if (r->obsoletes_a) {
      	tmp_node = xmlNewChild (deps_node, NULL, "obsoletes", NULL);
      	for (i = 0; i < r->obsoletes_a->len; i++) {
      	    RCResItemDep *dep = r->obsoletes_a->data[i];
      
      	    xmlAddChild (tmp_node, rc_resItem_dep_to_xml_node (dep));
      	}
          }
      
          if (r->provides_a) {
      	tmp_node = xmlNewChild (deps_node, NULL, "provides", NULL);
      	for (i = 0; i < r->provides_a->len; i++) {
      	    RCResItemDep *dep = r->provides_a->data[i];
      
      	    xmlAddChild (tmp_node, rc_resItem_dep_to_xml_node (dep));
      	}
          }
      
          return (package_node);
      } /* rc_package_to_xml_node */
      
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

