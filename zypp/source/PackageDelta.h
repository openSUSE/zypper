/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/PackageDelta.h
 *
*/
#ifndef ZYPP_SOURCE_PACKAGEDELTA_H
#define ZYPP_SOURCE_PACKAGEDELTA_H

#include <iosfwd>
#include <list>

#include "zypp/Date.h"
#include "zypp/ByteCount.h"
#include "zypp/Arch.h"
#include "zypp/Edition.h"
#include "zypp/CheckSum.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace packagedelta
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////

    class BaseVersion
    {
    public:
      BaseVersion( const Edition & edition,
                   const CheckSum & checksum,
                   const Date & buildtime )
      : _edition( edition )
      , _checksum( checksum )
      , _buildtime( buildtime )
      {}

      Edition edition() const
      { return _edition; }

      CheckSum checksum() const
      { return _checksum; }

      Date buildtime() const
      { return _buildtime; }

    private:
      Edition  _edition;
      CheckSum _checksum;
      Date     _buildtime;
    };

    /** \relates BaseVersion Stream output. */
    std::ostream & operator<<( std::ostream & str, const BaseVersion & obj );

    /** \relates BaseVersion */
    inline bool operator==( const BaseVersion & lhs, const BaseVersion & rhs )
    { return lhs.edition() == rhs.edition(); }

    /** \relates BaseVersion */
    inline bool operator!=( const BaseVersion & lhs, const BaseVersion & rhs )
    { return ! (lhs == rhs); }

    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////



      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////

    class PatchRpm
    {
    public:
      PatchRpm( const Arch & arch,
                const Pathname & filename,
                const ByteCount & downloadsize,
                const CheckSum & checksum,
                const Date & buildtime,
                const std::list<BaseVersion> & base_versions,
                const unsigned media_nr )
      : _arch( arch)
      , _filename( filename )
      , _downloadsize( downloadsize )
      , _checksum( checksum )
      , _buildtime( buildtime )
      , _base_versions( base_versions )
      , _media_nr( media_nr )
      {}

      Arch arch() const
      { return _arch; }

      Pathname filename() const
      { return _filename; }

      ByteCount downloadsize() const
      { return _downloadsize; }

      CheckSum checksum() const
      { return _checksum; }

      Date buildtime() const
      { return _buildtime; }

      const std::list<BaseVersion> & baseVersions() const
      { return _base_versions; }

      unsigned mediaNr() const
      { return _media_nr; }

    private:
      Arch                   _arch;
      Pathname               _filename;
      ByteCount              _downloadsize;
      CheckSum               _checksum;
      Date                   _buildtime;
      std::list<BaseVersion> _base_versions;
      unsigned               _media_nr;
    };

    /** \relates PatchRpm Stream output. */
    std::ostream & operator<<( std::ostream & str, const PatchRpm & obj );

    ///////////////////////////////////////////////////////////////////


    class DeltaRpm
    {
    public:
      DeltaRpm( const Arch & arch,
                const Pathname & filename,
                const ByteCount & downloadsize,
                const CheckSum & checksum,
                const Date & buildtime,
                const BaseVersion & base_version,
                const unsigned media_nr )
      : _arch( arch)
      , _filename( filename )
      , _downloadsize( downloadsize )
      , _checksum( checksum )
      , _buildtime( buildtime )
      , _base_version( base_version )
      , _media_nr( media_nr )
      {}

      Arch arch() const
      { return _arch; }

      Pathname filename() const
      { return _filename; }

      ByteCount downloadsize() const
      { return _downloadsize; }

      CheckSum checksum() const
      { return _checksum; }

      Date buildtime() const
      { return _buildtime; }

      BaseVersion baseVersion() const
      { return _base_version; }

      unsigned mediaNr() const
      { return _media_nr; }

    private:
      Arch        _arch;
      Pathname    _filename;
      ByteCount   _downloadsize;
      CheckSum    _checksum;
      Date        _buildtime;
      BaseVersion _base_version;
      unsigned    _media_nr;
    };

    /** \relates DeltaRpm Stream output. */
    std::ostream & operator<<( std::ostream & str, const DeltaRpm & obj );

    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace packagedelta
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PACKAGEDELTA_H
