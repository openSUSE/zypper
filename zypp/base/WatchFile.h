/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/WatchFile.h
 *
*/
#ifndef ZYPP_BASE_WATCHFILE_H
#define ZYPP_BASE_WATCHFILE_H

#include <iosfwd>

#include "zypp/PathInfo.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : WatchFile
  //
  /** Remember a files attributes to detect content changes.
   *
   * Repeatedly call \ref hasChanged to check whether the content has
   * changed since the last call. Creation or deletion of the file will
   * be reported as change as well.
   *
   * Per default the ctor stats the file, so \ref hasChanged will detect
   * changes done after \ref WatchFile was created.
   *
   * You may omit the initial stat by passing \c NO_INIT as second argument
   * to the ctor. \ref WatchFile  will behave as if the file did not exist
   * at the time \ref WatchFile was created.
   *
   * \code
   * static WatchFile sysconfigFile( "/etc/sysconfig/SuSEfirewall2",
   *                                 WatchFile::NO_INIT );
   * if ( sysconfigFile.hasChanged() )
   * {
   *   // reload the file...
   * }
   * \endcode
  */
  class WatchFile
  {
    public:
      enum Initial { NO_INIT, INIT };

    public:
      /** */
      WatchFile( const Pathname & path_r = Pathname(),
		 Initial mode            = INIT )
      : _path( path_r )
      {
	PathInfo pi( mode == INIT ? path_r : Pathname() );
	_size  = pi.size();
	_mtime = pi.mtime();
      }

      const Pathname & path() const
      { return _path; }

      bool hasChanged()
      {
	PathInfo pi( _path );
	if ( _size != pi.size() || _mtime != pi.mtime() )
	{
	  _size = pi.size();
	  _mtime = pi.mtime();
	  return true;
	}
	return false;
      }

    private:
      Pathname _path;
      off_t  _size;
      time_t _mtime;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_WATCHFILE_H
