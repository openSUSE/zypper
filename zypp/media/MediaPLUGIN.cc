/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaPLUGIN.cc
 *
*/
#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"

#include "zypp/ExternalProgram.h"

#include "zypp/media/MediaPLUGIN.h"
#include "zypp/media/MediaManager.h"

using std::endl;

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace media
  { //////////////////////////////////////////////////////////////////

    MediaPLUGIN::MediaPLUGIN( const Url & url_r, const Pathname & attach_point_hint_r )
      : MediaHandler( url_r, attach_point_hint_r, /*path below attachpoint*/"/", /*does_download*/false )
    {
      MIL << "MediaPLUGIN::MediaPLUGIN(" << url_r << ", " << attach_point_hint_r << ")" << endl;
    }
    void MediaPLUGIN::attachTo( bool next_r )
    {}

    void MediaPLUGIN::releaseFrom( const std::string & ejectDev_r )
    {}

    void MediaPLUGIN::getFile( const Pathname & filename_r ) const
    {}

    void MediaPLUGIN::getDir( const Pathname & dirname_r, bool recurse_r ) const
    {}

    void MediaPLUGIN::getDirInfo( std::list<std::string> & retlist_r, const Pathname & dirname_r, bool dots_r ) const
    {}

    void MediaPLUGIN::getDirInfo( filesystem::DirContent & retlist_r, const Pathname & dirname_r, bool dots_r ) const
    {}

    bool MediaPLUGIN::getDoesFileExist( const Pathname & filename_r ) const
    { return false; }

    //////////////////////////////////////////////////////////////////
  } // namespace media
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
