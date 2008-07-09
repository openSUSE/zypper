/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Package.cc
 *
*/
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/Package.h"
#include "zypp/sat/LookupAttr.h"
#include "zypp/ZYppFactory.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/target/rpm/RpmHeader.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Package);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Package::Package
  //	METHOD TYPE : Ctor
  //
  Package::Package( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Package::~Package
  //	METHOD TYPE : Dtor
  //
  Package::~Package()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Package interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  Changelog Package::changelog() const
  {
      Target_Ptr target;
      try
      {
          target = getZYpp()->target();
      }
      catch ( const Exception &e )
      {
           ERR << "Target not initialized. Changelog is not available." << std::endl;
           return Changelog();
      }


      if ( repository().isSystemRepo() )
      {
          target::rpm::RpmHeader::constPtr header;
          target->rpmDb().getData(name(), header);
          return header->tag_changelog();
      }
      WAR << "changelog is not available for uninstalled packages" << std::endl;
      return Changelog();
  }

  std::string Package::buildhost() const
  { return lookupStrAttribute( sat::SolvAttr::buildhost ); }

  std::string Package::distribution() const
  { return lookupStrAttribute( sat::SolvAttr::distribution ); }

  std::string Package::license() const
  { return lookupStrAttribute( sat::SolvAttr::license ); }

  std::string Package::packager() const
  { return lookupStrAttribute( sat::SolvAttr::packager ); }

  std::string Package::group() const
  { return lookupStrAttribute( sat::SolvAttr::group ); }

  Package::Keywords Package::keywords() const
  { return Keywords( sat::SolvAttr::keywords, satSolvable() ); }

  std::string Package::url() const
  { return lookupStrAttribute( sat::SolvAttr::url ); }

  ByteCount Package::sourcesize() const
  { return lookupNumAttribute( sat::SolvAttr::sourcesize ); }

  std::list<std::string> Package::authors() const
  {
    std::list<std::string> ret;
    str::split( lookupStrAttribute( sat::SolvAttr::authors ), std::back_inserter(ret), "\n" );
    return ret;
  }

  std::list<std::string> Package::filenames() const
  {
    std::list<std::string> files;
    sat::LookupAttr q( sat::SolvAttr::filelist, *this );
    for_( it, q.begin(), q.end() )
    {
        files.push_back(it.asString());
    }
    return files;
  }

  CheckSum Package::checksum() const
  { return lookupCheckSumAttribute( sat::SolvAttr::checksum ); }

  OnMediaLocation Package::location() const
  { return lookupLocation(); }

  std::string Package::sourcePkgName() const
  {
   // no id means same as package
    sat::detail::IdType id( lookupIdAttribute( sat::SolvAttr::sourcename ) );
    id = lookupIdAttribute(sat::SolvAttr::sourcearch);
    return id ? IdString( id ).asString() : name();
  }

  Edition Package::sourcePkgEdition() const
  {
   // no id means same as package
    sat::detail::IdType id( lookupIdAttribute( sat::SolvAttr::sourceevr ) );
    return id ? Edition( id ) : edition();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
