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
    str << xml_tag_enclose(edition.version(), "version")
	<< xml_tag_enclose(edition.release(), "release")
	<< xml_tag_enclose(numstring(edition.epoch()), "epoch");    
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
    if (isKind<VersionedCap>(cap)
	&& cap.op() != Rel::NONE
	&& cap.op() != Rel::ANY) {
	// version capability
	str << "<dep name='" << cap.index() << "' op='" << xml_escape(cap.op().asString()) <<
	    "' version='" << cap.edition().version() << "' release='" << cap.edition().release() << "' />" << endl;
    } else {
	// anything else
	str << "<dep name='" << cap.asString() << "' />" << endl;	
    }
    return str.str();    
}

template<> 
std::string helixXML( const CapSet &caps )
{
    stringstream str;
    CapSet::iterator it = caps.begin();
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

std::string helixXML( const Resolvable::constPtr &resolvable )
{
  stringstream str;
  str << "<" << toLower (resolvable->kind().asString()) << ">" << endl;
  str << TAB << xml_tag_enclose (resolvable->name(), "name") << endl;  
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
    }
    zypp::base::LogControl::instance().logfile( dumpPath +"/y2log" );
    zypp::base::LogControl::TmpExcessive excessive; // ZYPP_FULLLOG=1
    
    resolver.resolveDependencies();

    ResPool pool 	= resolver.pool();
    SourceTable		sourceTable;
    PoolItemList	items_to_install;
    PoolItemList 	items_to_remove;    

    HelixResolvable 	system (dumpPath + "/solver-system.xml");    

    for ( ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it )
    {
	Resolvable::constPtr res = it->resolvable();
	
	if ( it->status().isInstalled() ) {
	    // system channel
	    system.addResolvable (res);
	} else {
	    // source channels
	    ResObject::constPtr sourceItem = it->resolvable();
	    Source_Ref source  = sourceItem->source();
	    if (sourceTable.find (source) == sourceTable.end()) {
		sourceTable[source] = new HelixResolvable(dumpPath + "/"
							  + numstring(source.numericId())
							  + "-package.xml");
	    }
	    sourceTable[source]->addResolvable (res);
	}
	
	if ( it->status().isToBeInstalled()
	     && !(it->status().isBySolver())) {
	    items_to_install.push_back (*it);
	}
	if ( it->status().isToBeUninstalled()
	     && !(it->status().isBySolver())) {
	    items_to_remove.push_back (*it);
	}
    }

    // writing control file "*-test.xml"

    HelixControl control (dumpPath + "/solver-test.xml",
			  sourceTable);

    for (PoolItemList::const_iterator iter = items_to_install.begin(); iter != items_to_install.end(); iter++) {
	control.installResolvable (iter->resolvable());	
    }

    for (PoolItemList::const_iterator iter = items_to_remove.begin(); iter != items_to_remove.end(); iter++) {
	control.deleteResolvable (iter->resolvable());	
    }

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
    

void HelixResolvable::addResolvable(const Resolvable::constPtr &resolvable)
{
    *file << helixXML (resolvable);
}

//---------------------------------------------------------------------------

HelixControl::HelixControl(const std::string & controlPath,
			   const SourceTable & sourceTable,
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
	  << "<setup>" << endl
	  << TAB << "<system file=\"" << systemPath << "\"/>" << endl;
    for ( SourceTable::const_iterator it = sourceTable.begin();
	  it != sourceTable.end(); ++it ) {
	Source_Ref source = it->first;
	*file << TAB << "<channel file=\"" << numstring(source.numericId())
	      << "-package.xml\" name=\"" << numstring(source.numericId())
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
    Source_Ref source  = resObject->source();
    *file << "<install channel=\"" << numstring(source.numericId()) << "\" kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << "/>" << endl;
}
    
void HelixControl::deleteResolvable(const ResObject::constPtr &resObject)
{
    Source_Ref source  = resObject->source();    
    *file << "<uninstall channel=\"" << numstring(source.numericId()) << "\" kind=\"" << toLower (resObject->kind().asString()) << "\""
	  << " name=\"" << resObject->name() << "\"" << "/>" << endl;    
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
