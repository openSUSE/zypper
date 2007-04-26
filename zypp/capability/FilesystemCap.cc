/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/FilesystemCap.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/WatchFile.h"
#include "zypp/base/Sysconfig.h"

#include "zypp/capability/FilesystemCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(FilesystemCap)
    
    /** Ctor */
    FilesystemCap::FilesystemCap( const Resolvable::Kind & refers_r,
				  const std::string & name_r )
    : CapabilityImpl( refers_r )
    , _name( name_r )
    {}

    const CapabilityImpl::Kind & FilesystemCap::kind() const
    { return CapTraits<Self>::kind; }

    CapMatch FilesystemCap::matches( const CapabilityImpl::constPtr & rhs ) const
    {
      if ( sameKindAndRefers( rhs ) )
        {
          intrusive_ptr<const Self> filesystemrhs( asKind<Self>(rhs) );
          if ( isEvalCmd() == filesystemrhs->isEvalCmd() )
            return CapMatch::irrelevant;

          return( isEvalCmd() ? filesystemrhs->evaluate() : evaluate() );
        }
      return false;
    }

    std::string FilesystemCap::encode() const
    {
      std::string ret( "filesystem(" );
      ret += _name;
      ret += ")";
      return ret;
    }

    std::string FilesystemCap::index() const
    {
      return "filesystem()";
    }

    bool FilesystemCap::isEvalCmd() const
    { return _name.empty(); }

    bool FilesystemCap::evaluate() const
    {
      static WatchFile sysconfigFile( "/etc/sysconfig/storage", WatchFile::NO_INIT );
      static std::set<std::string> fs;

      if ( sysconfigFile.hasChanged() )
      {
	std::set<std::string> newfs;
	str::split( base::sysconfig::read( sysconfigFile.path() )["USED_FS_LIST"],
		    std::inserter( newfs, newfs.end() ) );
	fs.swap( newfs );
      }

      return( fs.find( _name ) != fs.end() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
