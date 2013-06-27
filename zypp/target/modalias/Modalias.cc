/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/modalias/Modalias.cc
 *
*/
extern "C"
{
#include <fnmatch.h>
}

#include <iostream>
#include <fstream>
#include <vector>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "MODALIAS"

#include "zypp/base/LogTools.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/InputStream.h"
#include "zypp/AutoDispose.h"
#include "zypp/PathInfo.h"

#include "zypp/target/modalias/Modalias.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace target
  {
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      /** Filter subtrees known to contain no modalias files */
      inline bool isBlackListed( const Pathname & dir_r, const char * file_r )
      {
#define PATH_IS( D, F ) ( ::strcmp( file_r, F ) == 0 && ::strcmp( dir_r.c_str(), D ) == 0 )
	switch ( file_r[0] )
	{
	  case 'm':
	    return PATH_IS( "/sys/devices/system", "memory" );	// bnc#824110: huge tree for systems with large RAM
	    break;
	}
	return false;
#undef PATH_IS
      }

      /** Recursively scan for modalias files and scan them to \a arg. */
      void foreach_file_recursive( const Pathname & dir_r, Modalias::ModaliasList & arg )
      {
	AutoDispose<DIR *> dir( ::opendir( dir_r.c_str() ), ::closedir );
	if ( ! dir )
	  return;

	struct dirent * dirent = NULL;
	while ( (dirent = ::readdir(dir)) != NULL )
	{
	  if ( dirent->d_name[0] == '.' )
	    continue;

	  if ( isBlackListed( dir_r, dirent->d_name ) )
	    continue;

	  PathInfo pi( dir_r / dirent->d_name, PathInfo::LSTAT );

	  if ( pi.isDir() )
	  {
	    foreach_file_recursive( pi.path(), arg );
	  }
	  else if ( pi.isFile() && ::strcmp( dirent->d_name, "modalias" ) == 0 )
	  {
	    // read modalias line from file
	    std::ifstream str( pi.path().c_str() );
	    std::string line( iostr::getline( str ) );
	    if ( ! line.empty() )
	      arg.push_back( line );
	  }
	}
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Modalias::Impl
    //
    /** Modalias implementation. */
    struct Modalias::Impl
    {
      /** Ctor. */
      Impl()
      {
	const char * dir = getenv("ZYPP_MODALIAS_SYSFS");
	if ( dir )
	{
	  PathInfo pi( dir );
	  if (  pi.isFile() )
	  {
	    // Debug/testcases:
	    //   find /sys/ -type f -name modalias -print0 | xargs -0 cat >/tmp/modaliases
	    //   ZYPP_MODALIAS_SYSFS=/tmp/modaliases
	    DBG << "Using $ZYPP_MODALIAS_SYSFS modalias file: " << dir << endl;
	    iostr::forEachLine( InputStream( pi.path() ),
	                        [&]( int num_r, std::string line_r )->bool
	                        {
		                  this->_modaliases.push_back( line_r );
			          return true;
				} );
	    return;
	  }
	  DBG << "Using $ZYPP_MODALIAS_SYSFS: " << dir << endl;
	}
	else
	{
	  dir = "/sys";
	  DBG << "Using /sys directory." << endl;
	}

	foreach_file_recursive( dir, _modaliases );
      }

      /** Dtor. */
      ~Impl()
      {}

      /*
       * Check if a device on the system matches a modalias PATTERN.
       *
       * Returns NULL if no matching device is found, and the modalias
       * of the first matching device otherwise. (More than one device
       * may match a given pattern.)
       *
       * On a system that has the following device,
       *
       *   pci:v00008086d0000265Asv00008086sd00004556bc0Csc03i00
       *
       * modalias_matches("pci:v00008086d0000265Asv*sd*bc*sc*i*") will
       * return a non-NULL value.
       */
      bool query( const char * cap_r ) const
      {
	if ( cap_r && *cap_r )
	{
	  for_( it, _modaliases.begin(), _modaliases.end() )
	  {
	    if ( fnmatch( cap_r, (*it).c_str(), 0 ) == 0 )
	      return true;
	  }
	}
	return false;
      }

    public:
      ModaliasList _modaliases;

    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
	static shared_ptr<Impl> _nullimpl( new Impl );
	return _nullimpl;
      }

    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Modalias::Impl Stream output
     * And maybe std::ostream & operator<< Modalias::Impl below too.
     * return libhal version or something like that.
     */
    inline std::ostream & operator<<( std::ostream & str, const Modalias::Impl & obj )
    {
      return dumpRange( str << "Modaliases: (" << obj._modaliases.size() << ") ", obj._modaliases.begin(), obj._modaliases.end() );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Modalias
    //
    ///////////////////////////////////////////////////////////////////

    Modalias::Modalias()
    : _pimpl( Impl::nullimpl() )
    {}

    Modalias::~Modalias()
    {}

    Modalias & Modalias::instance()
    {
      static Modalias _singleton;
      return _singleton;
    }

    bool Modalias::query( const char * cap_r ) const
    { return _pimpl->query( cap_r ); }

    const Modalias::ModaliasList & Modalias::modaliasList() const
    { return _pimpl->_modaliases; }

    void Modalias::modaliasList( ModaliasList newlist_r )
    { _pimpl->_modaliases.swap( newlist_r ); }

    std::ostream & operator<<( std::ostream & str, const Modalias & obj )
    { return str << *obj._pimpl; }

  } // namespace target
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

