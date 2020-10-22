/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaPlugin.cc
 *
*/
#include <iostream>

#include <zypp/base/LogTools.h>
#include <zypp/base/String.h>

#include <zypp/ExternalProgram.h>

#include <zypp/media/MediaPlugin.h>
#include <zypp/media/MediaManager.h>

using std::endl;

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace media
  { //////////////////////////////////////////////////////////////////

    MediaPlugin::MediaPlugin( const Url & url_r, const Pathname & attach_point_hint_r )
      : MediaHandler( url_r, attach_point_hint_r, /*path below attachpoint*/"/", /*does_download*/false )
    {
      MIL << "MediaPlugin::MediaPlugin(" << url_r << ", " << attach_point_hint_r << ")" << endl;
    }
    void MediaPlugin::attachTo( bool next_r )
    {}

    void MediaPlugin::releaseFrom( const std::string & ejectDev_r )
    {}

    void MediaPlugin::getFile( const OnMediaLocation &file , const ByteCount & expectedFileSize_r ) const
    {}

    void MediaPlugin::getDir( const Pathname & dirname_r, bool recurse_r ) const
    {}

    void MediaPlugin::getDirInfo( std::list<std::string> & retlist_r, const Pathname & dirname_r, bool dots_r ) const
    {}

    void MediaPlugin::getDirInfo( filesystem::DirContent & retlist_r, const Pathname & dirname_r, bool dots_r ) const
    {}

    bool MediaPlugin::getDoesFileExist( const Pathname & filename_r ) const
    { return false; }

    //////////////////////////////////////////////////////////////////
  } // namespace media
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
