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
#include "zypp/base/SerialNumber.h"

#include "zypp/capability/FilesystemCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    namespace
    {
      const Pathname & sysconfigStoragePath()
      {
        static Pathname _p( "/etc/sysconfig/storage" );
        return _p;
      }
    }

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
      static SerialNumberWatcher sysconfigStorage;
      static std::set<std::string> fs;

      if ( sysconfigStorage.remember( sysconfigStorageSerial() ) )
      {
	std::set<std::string> newfs;
	str::split( base::sysconfig::read( sysconfigStoragePath() )["USED_FS_LIST"],
		    std::inserter( newfs, newfs.end() ) );
	fs.swap( newfs );
      }

      return( fs.find( _name ) != fs.end() );
    }

    const SerialNumber & FilesystemCap::sysconfigStorageSerial()
    {
      static WatchFile    _sysconfigFile( sysconfigStoragePath(), WatchFile::NO_INIT );
      static SerialNumber _serial;

      if ( _sysconfigFile.hasChanged() )
      {
        _serial.setDirty();
      }
      return _serial;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
