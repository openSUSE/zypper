/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/solver/detail/Testcase.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "zypp/solver/detail/Testcase.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/ZConfig.h"
#include "zypp/PathInfo.h"
#include "zypp/Product.h"
#include "zypp/Package.h"
#include "zypp/Edition.h"
#include "zypp/parser/xml_escape_parser.hpp"
#include "zypp/base/String.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Capabilities.h"
#include "zypp/sat/Solvable.h"


/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

#define TAB "\t"
#define TAB2 "\t\t"

using namespace std;
using namespace zypp::str;

IMPL_PTR_TYPE(HelixResolvable);

static std::string xml_escape( const std::string &text )
{
  iobind::parser::xml_escape_parser parser;
  return parser.escape(text);
}

static std::string xml_tag_enclose( const std::string &text, const std::string &tag, bool escape = false )
{
  string result;
  result += "<" + tag + ">";

  if ( escape)
   result += xml_escape(text);
  else
   result += text;

  result += "</" + tag + ">";
  return result;
}


template<class T>
std::string helixXML( const T &obj ); //undefined

template<>
std::string helixXML( const Edition &edition )
{
    stringstream str;
    str << xml_tag_enclose(edition.version(), "version");
    if (!edition.release().empty())
	str << xml_tag_enclose(edition.release(), "release");
    if (edition.epoch() != Edition::noepoch)
	str << xml_tag_enclose(numstring(edition.epoch()), "epoch");
    return str.str();
}

template<>
std::string helixXML( const Arch &arch )
{
    stringstream str;
    str << xml_tag_enclose(arch.asString(), "arch");
    return str.str();
}

template<>
std::string helixXML( const Capability &cap )
{
    stringstream str;
    str << "<dep name='" << xml_escape(cap.asString()) << "'  />" << endl;

    return str.str();
}

template<>
std::string helixXML( const Capabilities &caps )
{
    stringstream str;
    Capabilities::const_iterator it = caps.begin();
    str << endl;
    for ( ; it != caps.end(); ++it)
    {
	str << TAB2 << helixXML((*it));
    }
    str << TAB;
    return str.str();
}

template<>
std::string helixXML( const CapabilitySet &caps )
{
    stringstream str;
    CapabilitySet::const_iterator it = caps.begin();
    str << endl;
    for ( ; it != caps.end(); ++it)
    {
	str << TAB2 << helixXML((*it));
    }
    str << TAB;
    return str.str();
}


template<>
std::string helixXML( const Dependencies &dep )
{
    stringstream str;
    if ( dep[Dep::PROVIDES].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::PROVIDES]), "provides") << endl;
    if ( dep[Dep::CONFLICTS].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::CONFLICTS]), "conflicts") << endl;
    if ( dep[Dep::OBSOLETES].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::OBSOLETES]), "obsoletes") << endl;
    if ( dep[Dep::FRESHENS].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::FRESHENS]), "freshens") << endl;
    if ( dep[Dep::REQUIRES].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::REQUIRES]), "requires") << endl;
    if ( dep[Dep::RECOMMENDS].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::RECOMMENDS]), "recommends") << endl;
    if ( dep[Dep::ENHANCES].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::ENHANCES]), "enhances") << endl;
    if ( dep[Dep::SUPPLEMENTS].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::SUPPLEMENTS]), "supplements") << endl;
    if ( dep[Dep::SUGGESTS].size() > 0 )
	str << TAB << xml_tag_enclose(helixXML(dep[Dep::SUGGESTS]), "suggests") << endl;
    return str.str();
}

inline string helixXML( const Resolvable::constPtr &obj, Dep deptag_r )
{
  stringstream out;
  Capabilities caps( obj->dep(deptag_r) );
  if ( ! caps.empty() )
    out << TAB << xml_tag_enclose(helixXML(caps), deptag_r.asString()) << endl;
  return out.str();
}

std::string helixXML( const PoolItem &item )
{
  const Resolvable::constPtr resolvable = item.resolvable();
  stringstream str;
  str << "<" << toLower (resolvable->kind().asString()) << ">" << endl;
  str << TAB << xml_tag_enclose (resolvable->name(), "name", true) << endl;
  str << TAB << xml_tag_enclose (item->vendor(), "vendor", true) << endl;
  if ( isKind<Package>(resolvable) ) {
      str << TAB << "<history>" << endl << TAB << "<update>" << endl;
      str << TAB2 << helixXML (resolvable->arch()) << endl;
      str << TAB2 << helixXML (resolvable->edition()) << endl;
      str << TAB << "</update>" << endl << TAB << "</history>" << endl;
  } else {
      str << TAB << helixXML (resolvable->arch()) << endl;
      str << TAB << helixXML (resolvable->edition()) << endl;
  }
  str << helixXML( resolvable, Dep::PROVIDES);
  str << helixXML( resolvable, Dep::PREREQUIRES);
  str << helixXML( resolvable, Dep::CONFLICTS);
  str << helixXML( resolvable, Dep::OBSOLETES);
  str << helixXML( resolvable, Dep::FRESHENS);
  str << helixXML( resolvable, Dep::REQUIRES);
  str << helixXML( resolvable, Dep::RECOMMENDS);
  str << helixXML( resolvable, Dep::ENHANCES);
  str << helixXML( resolvable, Dep::SUPPLEMENTS);
  str << helixXML( resolvable, Dep::SUGGESTS);

  str << "</" << toLower (resolvable->kind().asString()) << ">" << endl;
  return str.str();
}

//---------------------------------------------------------------------------

Testcase::Testcase()
    :dumpPath("/var/log/YaST2/solverTestcase")
{
}

Testcase::Testcase(const std::string & path)
    :dumpPath(path)
{
}


Testcase::~Testcase()
{
}


bool Testcase::createTestcasePool(const ResPool &pool)
{
    PathInfo path (dumpPath);

    if ( !path.isExist() ) {
	if (zypp::filesystem::mkdir (dumpPath)!=0) {
	    ERR << "Cannot create directory " << dumpPath << endl;
	    return false;
	}
    } else {
	if (!path.isDir()) {
	    ERR << dumpPath << " is not a directory." << endl;
	    return false;
	}
	// remove old stuff
	zypp::filesystem::clean_dir (dumpPath);
    }
    
    RepositoryTable		repoTable;
    HelixResolvable 	system (dumpPath + "/solver-system.xml.gz");    

    for ( ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it )
    {
	if ( it->status().isInstalled() ) {
	    // system channel
	    system.addResolvable (*it);
	} else {
	    // repo channels
	    sat::Repo repo  = it->resolvable()->satSolvable().repo();
	    if (repoTable.find (repo) == repoTable.end()) {
		repoTable[repo] = new HelixResolvable(dumpPath + "/"
						      + str::numstring((long)repo.id())
						      + "-package.xml.gz");
	    }
	    repoTable[repo]->addResolvable (*it);
	}
    }	
    return true;
}

bool Testcase::createTestcase(Resolver & resolver, bool dumpPool, bool runSolver)
{
    PathInfo path (dumpPath);

    if ( !path.isExist() ) {
	if (zypp::filesystem::mkdir (dumpPath)!=0) {
	    ERR << "Cannot create directory " << dumpPath << endl;
	    return false;
	}
    } else {
	if (!path.isDir()) {
	    ERR << dumpPath << " is not a directory." << endl;
	    return false;
	}
	// remove old stuff if pool will be dump
	if (dumpPool)
	    zypp::filesystem::clean_dir (dumpPath);
    }

    if (runSolver) {
	zypp::base::LogControl::instance().logfile( dumpPath +"/y2log" );
	zypp::base::LogControl::TmpExcessive excessive;

	resolver.reset(true); // true = resetting all valid solverresults
	resolver.resolvePool();

	zypp::base::LogControl::instance().logfile( "/var/log/YaST2/y2log" );
    }

    ResPool pool 	= resolver.pool();
    RepositoryTable	repoTable;
    PoolItemList	items_to_install;
    PoolItemList 	items_to_remove;
    PoolItemList 	items_locked;
    PoolItemList 	items_keep;
    PoolItemList	language;
    HelixResolvable_Ptr	system = NULL;

    if (dumpPool)
	system = new HelixResolvable(dumpPath + "/solver-system.xml.gz");

    for ( ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it )
    {
	Resolvable::constPtr res = it->resolvable();

#warning NO MORE LANGUAGE RESOLVABLE
        // - use pools list of requested locales and pass it as 'LocaleList language'
        // - restore the list via Pool::setRequestedLocales.
#if 0
	if (isKind<Language>(res)) {
	    if ( it->status().isInstalled()
		 || it->status().isToBeInstalled()) {
		language.push_back (*it);
	    }
	} else {
#endif
	    if ( system && it->status().isInstalled() ) {
		// system channel
		system->addResolvable (*it);
	    } else {
		// repo channels
		sat::Repo repo  = it->resolvable()->satSolvable().repo();
		if (dumpPool) {
		    if (repoTable.find (repo) == repoTable.end()) {
			repoTable[repo] = new HelixResolvable(dumpPath + "/"
							      + str::numstring((long)repo.id())
							      + "-package.xml.gz");
		    }
		    repoTable[repo]->addResolvable (*it);
		}
	    }

	    if ( it->status().isToBeInstalled()
		 && !(it->status().isBySolver())) {
		items_to_install.push_back (*it);
	    }
	    if ( it->status().isKept()
		 && !(it->status().isBySolver())) {
		items_keep.push_back (*it);
	    }
	    if ( it->status().isToBeUninstalled()
		 && !(it->status().isBySolver())) {
		items_to_remove.push_back (*it);
	    }
	    if ( it->status().isLocked()
		 && !(it->status().isBySolver())) {
		items_locked.push_back (*it);
	    }
    }

    // writing control file "*-test.xml"

    HelixControl control (dumpPath + "/solver-test.xml.gz",
			  repoTable,
			  ZConfig::instance().systemArchitecture(),
			  language);

    for (PoolItemList::const_iterator iter = items_to_install.begin(); iter != items_to_install.end(); iter++) {
	control.installResolvable (iter->resolvable(), iter->status());
    }

    for (PoolItemList::const_iterator iter = items_locked.begin(); iter != items_locked.end(); iter++) {
	control.lockResolvable (iter->resolvable(), iter->status());
    }

    for (PoolItemList::const_iterator iter = items_keep.begin(); iter != items_keep.end(); iter++) {
	control.keepResolvable (iter->resolvable(), iter->status());
    }

    for (PoolItemList::const_iterator iter = items_to_remove.begin(); iter != items_to_remove.end(); iter++) {
	control.deleteResolvable (iter->resolvable(), iter->status());
    }

    control.addDependencies (resolver.extraRequires(), resolver.extraConflicts());

    return true;
}

//---------------------------------------------------------------------------

HelixResolvable::HelixResolvable(const std::string & path)
    :dumpFile (path)
{
    file = new ofgzstream(path.c_str());
    if (!file) {
	ZYPP_THROW (Exception( "Can't open " + path ) );
    }

    *file << "<channel><subchannel>" << endl;
}

HelixResolvable::~HelixResolvable()
{
    *file << "</subchannel></channel>" << endl;
    delete(file);
}


void HelixResolvable::addResolvable(const PoolItem item)
{
    *file << helixXML (item);
}

//---------------------------------------------------------------------------

HelixControl::HelixControl(const std::string & controlPath,
			   const RepositoryTable & repoTable,
			   const Arch & systemArchitecture,
			   const PoolItemList &languages,
			   const std::string & systemPath)
    :dumpFile (controlPath)
{
    file = new ofgzstream(controlPath.c_str());
    if (!file) {
	ZYPP_THROW (Exception( "Can't open " + controlPath ) );
    }

    *file << "<?xml version=\"1.0\"?>" << endl
	  << "<!-- testcase generated by YaST -->" << endl
	  << "<test>" << endl
	  << "<setup arch=\"" << systemArchitecture << "\">" << endl
	  << TAB << "<system file=\"" << systemPath << "\"/>" << endl << endl;
    for ( RepositoryTable::const_iterator it = repoTable.begin();
	  it != repoTable.end(); ++it ) {
	RepoInfo repo = it->first.info();
	*file << TAB << "<!-- " << endl
	      << TAB << "- alias       : " << repo.alias() << endl;
	for ( RepoInfo::urls_const_iterator itUrl = repo.baseUrlsBegin();
	      itUrl != repo.baseUrlsEnd();
	      ++itUrl )
	{
	    *file << TAB << "- url         : " << *itUrl << endl;
	}
	*file << TAB << "- path        : " << repo.path() << endl;
	*file << TAB << "- type        : " << repo.type() << endl;
	*file << TAB << " -->" << endl;

	*file << TAB << "<channel file=\"" << str::numstring((long)it->first.id())
	      << "-package.xml\" name=\"" << repo.alias()
	      << "\" />" << endl << endl;
    }
    for (PoolItemList::const_iterator iter = languages.begin(); iter != languages.end(); iter++) {
	*file << TAB << "<locale name=\"" <<  iter->resolvable()->name()
	      << "\" />" << endl;
    }
    *file << "</setup>" << endl
	  << "<trial>" << endl
	  << "<showpool all=\"yes\"/>" << endl;
}

HelixControl::HelixControl()
    :dumpFile ("/var/log/YaST2/solverTestcase/solver-test.xml")
{
    HelixControl (dumpFile);
}

HelixControl::~HelixControl()
{
    *file << "</trial>" << endl
	  << "</test>" << endl;
    delete(file);    
}

void HelixControl::installResolvable(const ResObject::constPtr &resObject,
				     const ResStatus &status)
{
    *file << "<install channel=\"" << resObject->repoInfo().alias() << "\" kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << " arch=\"" << resObject->arch().asString() << "\""
	  << " version=\"" << resObject->edition().version() << "\"" << " release=\"" << resObject->edition().release() << "\""
	  << " status=\"" << status << "\""
	  << "/>" << endl;
}

void HelixControl::lockResolvable(const ResObject::constPtr &resObject,
				  const ResStatus &status)
{
    *file << "<lock channel=\"" << resObject->repoInfo().alias() << "\" kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << " arch=\"" << resObject->arch().asString() << "\""
	  << " version=\"" << resObject->edition().version() << "\"" << " release=\"" << resObject->edition().release() << "\""
	  << " status=\"" << status << "\""
	  << "/>" << endl;
}

void HelixControl::keepResolvable(const ResObject::constPtr &resObject,
				  const ResStatus &status)
{
    *file << "<keep channel=\"" << resObject->repoInfo().alias() << "\" kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << " arch=\"" << resObject->arch().asString() << "\""
	  << " version=\"" << resObject->edition().version() << "\"" << " release=\"" << resObject->edition().release() << "\""
	  << " status=\"" << status << "\""
	  << "/>" << endl;
}

void HelixControl::deleteResolvable(const ResObject::constPtr &resObject,
				    const ResStatus &status)
{
    *file << "<uninstall " << " kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\""
	  << " status=\"" << status << "\""
	  << "/>" << endl;
}

void HelixControl::addDependencies (const CapabilitySet & capRequire, const CapabilitySet & capConflict)
{
    for (CapabilitySet::const_iterator iter = capRequire.begin(); iter != capRequire.end(); iter++) {
	*file << "<addRequire " <<  " name=\"" << iter->asString() << "\"" << "/>" << endl;
    }
    for (CapabilitySet::const_iterator iter = capConflict.begin(); iter != capConflict.end(); iter++) {
	*file << "<addConflict " << " name=\"" << iter->asString() << "\"" << "/>" << endl;
    }
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
