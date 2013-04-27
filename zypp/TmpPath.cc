/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/TmpPath.cc
 *
*/

#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <iostream>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Logger.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

using namespace std;

namespace zypp {
  namespace filesystem {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TmpPath::Impl
    /**
     * Clean or delete a directory on destruction.
     **/
    class TmpPath::Impl : public base::ReferenceCounted, private base::NonCopyable
    {
      public:

        enum Flags
          {
            NoOp         = 0,
            Autodelete   = 1L << 0,
            KeepTopdir   = 1L << 1,
            //
            CtorDefault  = Autodelete
          };

      public:

        Impl( const Pathname & path_r, Flags flags_r = CtorDefault )
        : _path( path_r ), _flags( flags_r )
        {}

        ~Impl()
        {
          if ( ! (_flags & Autodelete) || _path.empty() )
            return;

          PathInfo p( _path, PathInfo::LSTAT );
          if ( ! p.isExist() )
            return;

          int res = 0;
          if ( p.isDir() )
            {
              if ( _flags & KeepTopdir )
                res = clean_dir( _path );
              else
                res = recursive_rmdir( _path );
            }
          else
            res = unlink( _path );

          if ( res )
            INT << "TmpPath cleanup error (" << res << ") " << p << endl;
          else
            DBG << "TmpPath cleaned up " << p << endl;
        }

        const Pathname &
        path() const
        { return _path; }

        bool autoCleanup() const
        { return( _flags & Autodelete ); }

        void autoCleanup( bool yesno_r )
	{ _flags = yesno_r ? CtorDefault : NoOp; }

      private:
        Pathname _path;
        Flags    _flags;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TmpPath
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpPath::TmpPath
    //	METHOD TYPE : Constructor
    //
    TmpPath::TmpPath()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpPath::TmpPath
    //	METHOD TYPE : Constructor
    //
    TmpPath::TmpPath( const Pathname & tmpPath_r )
    :_impl( tmpPath_r.empty() ? nullptr : new Impl( tmpPath_r ) )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpPath::~TmpPath
    //	METHOD TYPE : Destructor
    //
    TmpPath::~TmpPath()
    {
      // virtual not inlined dtor.
    }

    ///////////////////////////////////////////////////////////////////
    //
    //      METHOD NAME : TmpPath::operator const void *
    //      METHOD TYPE :
    //
    TmpPath::operator const void * () const
    {
      return _impl.get();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpPath::path
    //	METHOD TYPE : Pathname
    //
    Pathname
    TmpPath::path() const
    {
      return _impl.get() ? _impl->path() : Pathname();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpPath::defaultLocation
    //	METHOD TYPE : const Pathname &
    //
    const Pathname &
    TmpPath::defaultLocation()
    {
      static Pathname p( getenv("ZYPPTMPDIR") ? getenv("ZYPPTMPDIR") : "/var/tmp" );
      return p;
    }

    bool TmpPath::autoCleanup() const
    { return _impl.get() ? _impl->autoCleanup() : false; }

    void TmpPath::autoCleanup( bool yesno_r )
    { if ( _impl.get() ) _impl->autoCleanup( yesno_r ); }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TmpFile
    //
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpFile::TmpFile
    //	METHOD TYPE : Constructor
    //
    TmpFile::TmpFile( const Pathname & inParentDir_r,
                      const std::string & prefix_r )
    {
      // parent dir must exist
      if ( filesystem::assert_dir( inParentDir_r ) != 0 )
      {
        ERR << "Parent directory '" << inParentDir_r << "' can't be created." << endl;
        return;
      }

      // create the temp file
      Pathname tmpPath = (inParentDir_r + prefix_r).extend( "XXXXXX");
      char * buf = ::strdup( tmpPath.asString().c_str() );
      if ( ! buf )
        {
          ERR << "Out of memory" << endl;
          return;
        }

      int tmpFd = ::mkostemp( buf, O_CLOEXEC );
      if ( tmpFd != -1 )
        {
          // success; create _impl
          ::close( tmpFd );
          _impl = RW_pointer<Impl>( new Impl( buf ) );
        }
      else
        ERR << "Cant create '" << buf << "' " << ::strerror( errno ) << endl;

      ::free( buf );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpFile::makeSibling
    //	METHOD TYPE : TmpFile
    //
    TmpFile TmpFile::makeSibling( const Pathname & sibling_r )
    {
      TmpFile ret( sibling_r.dirname(), sibling_r.basename() );
      // clone mode if sibling_r exists
      PathInfo p( sibling_r );
      if ( p.isFile() )
      {
        ::chmod( ret.path().c_str(), p.st_mode() );
      }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpFile::defaultPrefix
    //	METHOD TYPE : const std::string &
    //
    const std::string &
    TmpFile::defaultPrefix()
    {
      static string p( "TmpFile." );
      return p;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TmpDir
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpDir::TmpDir
    //	METHOD TYPE : Constructor
    //
    TmpDir::TmpDir( const Pathname & inParentDir_r,
                    const std::string & prefix_r )
    {
      // parent dir must exist
      if ( filesystem::assert_dir( inParentDir_r ) != 0  )
      {
        ERR << "Parent directory '" << inParentDir_r << "' can't be created." << endl;
        return;
      }

      // create the temp dir
      Pathname tmpPath = (inParentDir_r + prefix_r).extend( "XXXXXX");
      char * buf = ::strdup( tmpPath.asString().c_str() );
      if ( ! buf )
        {
          ERR << "Out of memory" << endl;
          return;
        }

      char * tmp = ::mkdtemp( buf );
      if ( tmp )
        // success; create _impl
        _impl = RW_pointer<Impl>( new Impl( tmp ) );
      else
        ERR << "Cant create '" << tmpPath << "' " << ::strerror( errno ) << endl;

      ::free( buf );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpDir::makeSibling
    //	METHOD TYPE : TmpDir
    //
    TmpDir TmpDir::makeSibling( const Pathname & sibling_r )
    {
      TmpDir ret( sibling_r.dirname(), sibling_r.basename() );
      // clone mode if sibling_r exists
      PathInfo p( sibling_r );
      if ( p.isDir() )
      {
        ::chmod( ret.path().c_str(), p.st_mode() );
      }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TmpDir::defaultPrefix
    //	METHOD TYPE : const std::string &
    //
    const std::string &
    TmpDir::defaultPrefix()
    {
      static string p( "TmpDir." );
      return p;
    }

  } // namespace filesystem
} // namespace zypp
