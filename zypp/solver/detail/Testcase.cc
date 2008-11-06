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
#include "zypp/parser/xml/XmlEscape.h"
#include "zypp/base/String.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Capabilities.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/solver/detail/SystemCheck.h"


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

inline std::string xml_escape( const std::string &text )
{
  return zypp::xml::escape(text);
}

inline std::string xml_tag_enclose( const std::string &text, const std::string &tag, bool escape = false )
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
    CapDetail detail = cap.detail();
    if (detail.isSimple()) {
	if (detail.isVersioned()) {
	    str << "<dep name='" << xml_escape(detail.name().asString()) << "'"
		<< " op='" << xml_escape(detail.op().asString()) << "'"
		<< " version='" <<  xml_escape(detail.ed().version()) << "'";
	    if (!detail.ed().release().empty())
		str << " release='" << xml_escape(detail.ed().release()) << "'";
	    if (detail.ed().epoch() != Edition::noepoch)
		str << " epoch='" << xml_escape(numstring(detail.ed().epoch())) << "'";
	    str << " />" << endl;
	} else {
	    str << "<dep name='" << xml_escape(cap.asString()) << "' />" << endl;
	}
    } else if (detail.isExpression()) {
	if (detail.capRel() == CapDetail::CAP_AND
	    && detail.lhs().detail().isNamed()
	    && detail.rhs().detail().isNamed()) {
	    // packageand dependency
	    str << "<dep name='packageand("
		<< IdString(detail.lhs().id()) << ","
		<< IdString(detail.rhs().id()) << ")' />" << endl;
	} else {
	    // modalias ?
	    IdString packageName;
	    if (detail.capRel() == CapDetail::CAP_AND) {
		packageName = IdString(detail.lhs().id());
		detail = detail.rhs().detail();
	    }
	    if (detail.capRel() == CapDetail::CAP_NAMESPACE
		&& detail.lhs().id() == NAMESPACE_MODALIAS) {
		str << "<dep name='modalias(";
		if (!packageName.empty())
		    str << packageName << ":";
		str << IdString(detail.rhs().id()) << ")' />" << endl;
	    } else {
		str << "<!--- ignoring '" << xml_escape(cap.asString()) << "' -->" << endl;
	    }
	}
    } else {
	str << "<!--- ignoring '" << xml_escape(cap.asString()) << "' -->" << endl;
    }

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
  str << TAB << xml_tag_enclose (item->buildtime().asSeconds(), "buildtime", true) << endl;
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
	    Repository repo  = it->resolvable()->satSolvable().repository();
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
        zypp::base::LogControl::TmpLineWriter tempRedirect;
	zypp::base::LogControl::instance().logfile( dumpPath +"/y2log" );
	zypp::base::LogControl::TmpExcessive excessive;

	resolver.reset(true); // true = resetting all valid solverresults
	resolver.resolvePool();
    }

    ResPool pool 	= resolver.pool();
    RepositoryTable	repoTable;
    PoolItemList	items_to_install;
    PoolItemList 	items_to_remove;
    PoolItemList 	items_locked;
    PoolItemList 	items_keep;
    HelixResolvable_Ptr	system = NULL;

    if (dumpPool)
	system = new HelixResolvable(dumpPath + "/solver-system.xml.gz");

    for ( ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it )
    {
	Resolvable::constPtr res = it->resolvable();

	if ( system && it->status().isInstalled() ) {
	    // system channel
	    system->addResolvable (*it);
	} else {
	    // repo channels
	    Repository repo  = it->resolvable()->satSolvable().repository();
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
    HelixControl control (dumpPath + "/solver-test.xml",
			  repoTable,
			  ZConfig::instance().systemArchitecture(),
			  pool.getRequestedLocales(),
			  "solver-system.xml.gz",
			  resolver.forceResolve(),
			  resolver.onlyRequires(),
			  resolver.ignorealreadyrecommended() );

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
    control.addDependencies (SystemCheck::instance().requiredSystemCap(),
			     SystemCheck::instance().conflictSystemCap());

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
			   const LocaleSet &languages,
			   const std::string & systemPath,
			   const bool forceResolve,
			   const bool onlyRequires,
			   const bool ignorealreadyrecommended)
    :dumpFile (controlPath)
{
    file = new ofstream(controlPath.c_str());
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
	      << "-package.xml.gz\" name=\"" << repo.alias() << "\""
	      << " priority=\"" << repo.priority() 
	      << "\" />" << endl << endl;
    }
    for (LocaleSet::const_iterator iter = languages.begin(); iter != languages.end(); iter++) {
	*file << TAB << "<locale name=\"" <<  iter->code()
	      << "\" />" << endl;
    }

    if (forceResolve)
	*file << TAB << "<forceResolve/>" << endl;
    if (onlyRequires)
	*file << TAB << "<onlyRequires/>" << endl;

    if (ignorealreadyrecommended)
	*file << TAB << "<ignorealreadyrecommended/>" << endl;
    
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
