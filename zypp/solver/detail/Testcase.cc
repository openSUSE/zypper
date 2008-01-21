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
#include "zypp/PathInfo.h"
#include "zypp/Product.h"
#include "zypp/Package.h"
#include "zypp/Edition.h"
#include "zypp/target/store/xml_escape_parser.hpp"
#include "zypp/capability/VersionedCap.h"
#include "zypp/base/String.h"
#include "zypp/base/PtrTypes.h"


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
using namespace zypp::capability;
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
    VersionedCap::constPtr vercap = asKind<VersionedCap>(cap);
    if (vercap
	&& vercap->op() != Rel::NONE
	&& vercap->op() != Rel::ANY
	&& !vercap->edition().version().empty() )
    {
	// version capability
	str << "<dep name='" << xml_escape(vercap->index()) << "' op='" << xml_escape(vercap->op().asString()) <<
	    "' version='" << vercap->edition().version() << "'";
	if (!vercap->edition().release().empty())
	    str << " release='" << vercap->edition().release() << "'";
	if (vercap->edition().epoch() != Edition::noepoch)
	    str << " epoch='" << numstring(vercap->edition().epoch()) << "'";
    }
    else
    {
      str << "<dep name='" << xml_escape(cap.asString()) << "'";
    }
    
    str << " kind=\"" << toLower (cap.refers().asString()) << "\""
	<< " />" << endl;
	
    return str.str();    
}

template<> 
std::string helixXML( const Capabilities &caps )
{
    stringstream str;
    Capabilities::iterator it = caps.begin();
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

std::string helixXML( const PoolItem_Ref &item )
{
  const Resolvable::constPtr resolvable = item.resolvable();
  stringstream str;
  if ( isKind<SystemResObject>(resolvable)
       || isKind<Language>(resolvable) ) {
      // system resolvable will be generated by the resolver
      // language dependencies will be written i another part
      return str.str();
  }
  
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
  str << helixXML (resolvable->deps());              

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

bool Testcase::createTestcase(Resolver & resolver)
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
    
    zypp::base::LogControl::instance().logfile( dumpPath +"/y2log" );
    zypp::base::LogControl::TmpExcessive excessive;

    resolver.reset(true); // true = resetting all valid solverresults
    resolver.resolvePool();

    zypp::base::LogControl::instance().logfile( "/var/log/YaST2/y2log" );    

    ResPool pool 	= resolver.pool();
    RepositoryTable		repoTable;
    PoolItemList	items_to_install;
    PoolItemList 	items_to_remove;
    PoolItemList 	items_locked;
    PoolItemList 	items_keep;    
    PoolItemList	language;
    HelixResolvable 	system (dumpPath + "/solver-system.xml");    

    for ( ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it )
    {
	Resolvable::constPtr res = it->resolvable();

	if (isKind<Language>(res)) {
	    if ( it->status().isInstalled()
		 || it->status().isToBeInstalled()) {
		language.push_back (*it);		
	    }
	} else {
	    if ( it->status().isInstalled() ) {
		// system channel
		system.addResolvable (*it);
	    } else {
		// repo channels
		ResObject::constPtr repoItem = it->resolvable();
		Repository repo  = repoItem->repository();
		if (repoTable.find (repo) == repoTable.end()) {
		    repoTable[repo] = new HelixResolvable(dumpPath + "/"
							  + numstring(repo.numericId())
							  + "-package.xml");
		}
		repoTable[repo]->addResolvable (*it);
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
		 && !(it->status().isBySolver())
		 && !isKind<SystemResObject>(res)) {
		items_locked.push_back (*it);
	    }
	    
	}
    }

    // writing control file "*-test.xml"

    HelixControl control (dumpPath + "/solver-test.xml",
			  repoTable,
			  resolver.architecture(),
			  language);

    for (PoolItemList::const_iterator iter = items_to_install.begin(); iter != items_to_install.end(); iter++) {
	control.installResolvable (iter->resolvable());	
    }

    for (PoolItemList::const_iterator iter = items_locked.begin(); iter != items_locked.end(); iter++) {
	control.lockResolvable (iter->resolvable());	
    }
    
    for (PoolItemList::const_iterator iter = items_keep.begin(); iter != items_keep.end(); iter++) {
	control.keepResolvable (iter->resolvable());	
    }

    for (PoolItemList::const_iterator iter = items_to_remove.begin(); iter != items_to_remove.end(); iter++) {
	control.deleteResolvable (iter->resolvable());	
    }

    control.addDependencies (resolver.extraCapability(), resolver.extraConflicts());

    return true;
}

//---------------------------------------------------------------------------

HelixResolvable::HelixResolvable(const std::string & path)
    :dumpFile (path)    
{
    file = new ofstream(path.c_str());
    if (!file) {
	ZYPP_THROW (Exception( "Can't open " + path ) );
    }

    *file << "<channel><subchannel>" << endl;
}

HelixResolvable::~HelixResolvable()
{
    *file << "</subchannel></channel>" << endl;
}
    

void HelixResolvable::addResolvable(const PoolItem_Ref item)
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
	Repository repo = it->first;
	*file << TAB << "<!-- " << endl
	      << TAB << "- alias       : " << repo.info().alias() << endl;
	for ( RepoInfo::urls_const_iterator itUrl = repo.info().baseUrlsBegin();
	      itUrl != repo.info().baseUrlsEnd();
	      ++itUrl )
	{
	    *file << TAB << "- url         : " << *itUrl << endl;
	}	
	*file << TAB << "- path        : " << repo.info().path() << endl;
	*file << TAB << "- type        : " << repo.info().type() << endl;	
	*file << TAB << " -->" << endl;
	
	*file << TAB << "<channel file=\"" << numstring(repo.numericId())
	      << "-package.xml\" name=\"" << numstring(repo.numericId())
	      << "\" />" << endl << endl;
    }
    for (PoolItemList::const_iterator iter = languages.begin(); iter != languages.end(); iter++) {
	*file << TAB << "<locale name=\"" <<  iter->resolvable()->name()
	      << "\" />" << endl;
    }    
    *file << "</setup>" << endl
	  << "<trial>" << endl
	  << "<showpool all=\"yes\"/>" << endl
	  << "<establish/>" << endl
	  << "<showpool all=\"true\" prefix=\">!> ESTABLISHED:\"/>" << endl;
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
}

void HelixControl::installResolvable(const ResObject::constPtr &resObject)
{
    Repository repo  = resObject->repository();
    *file << "<install channel=\"" << numstring(repo.numericId()) << "\" kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << " arch=\"" << resObject->arch().asString() << "\""
	  << " version=\"" << resObject->edition().version() << "\"" << " release=\"" << resObject->edition().release() << "\"" 
	  << "/>" << endl;
}

void HelixControl::lockResolvable(const ResObject::constPtr &resObject)
{
    Repository repo  = resObject->repository();
    *file << "<lock channel=\"" << numstring(repo.numericId()) << "\" kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << " arch=\"" << resObject->arch().asString() << "\""
	  << " version=\"" << resObject->edition().version() << "\"" << " release=\"" << resObject->edition().release() << "\"" 
	  << "/>" << endl;
}

void HelixControl::keepResolvable(const ResObject::constPtr &resObject)
{
    Repository repo  = resObject->repository();
    *file << "<keep channel=\"" << numstring(repo.numericId()) << "\" kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << " arch=\"" << resObject->arch().asString() << "\""
	  << " version=\"" << resObject->edition().version() << "\"" << " release=\"" << resObject->edition().release() << "\"" 
	  << "/>" << endl;
}
    
void HelixControl::deleteResolvable(const ResObject::constPtr &resObject)
{
    Repository repo  = resObject->repository();    
    *file << "<uninstall " << " kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << "/>" << endl;    
}

void HelixControl::addDependencies (const Capabilities & capRequire, const Capabilities & capConflict)
{
    for (Capabilities::const_iterator iter = capRequire.begin(); iter != capRequire.end(); iter++) {
	*file << "<addRequire " << " kind=\"" << toLower (iter->kind().asString()) << "\""
	  << " name=\"" << iter->asString() << "\"" << "/>" << endl;    
    }
    for (Capabilities::const_iterator iter = capConflict.begin(); iter != capConflict.end(); iter++) {
	*file << "<addConflict " << " kind=\"" << toLower (iter->kind().asString()) << "\""
	  << " name=\"" << iter->asString() << "\"" << "/>" << endl;    
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
