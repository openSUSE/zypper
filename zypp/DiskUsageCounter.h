/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/DiskUsageCounter.h
 *
*/
#ifndef ZYPP_DISKUSAGE_COUNTER_H
#define ZYPP_DISKUSAGE_COUNTER_H

#include <set>
#include <string>
#include <iosfwd>

#include "zypp/ResPool.h"
#include "zypp/Bitmap.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class DiskUsageCounter
  /// \brief Compute disk space occupied by packages across partitions/directories
  ///////////////////////////////////////////////////////////////////
  class DiskUsageCounter
  {

  public:
    ///////////////////////////////////////////////////////////////////
    /// \class MountPoint
    /// \brief Mount point description
    /// If \ref block_size is set \ref DiskUsageCoutner will assume
    /// half a block_size is wasted per file, in case a package
    /// provides detailed isk usage information.
    ///////////////////////////////////////////////////////////////////
    struct MountPoint
    {
      std::string dir;			///< Directory name
      long long block_size;		///< Block size of the filesystem in B (0 if you don't care)
      long long total_size;		///< Total size of the filesystem in KiB (0 if you don't care)
      long long used_size;		///< Used size of the filesystem in KiB (0 if you don't care)
      mutable long long pkg_size;	///< Used size after installation in KiB (computed by \ref DiskUsageCoutner)
      bool readonly;			///< hint for readonly partitions

      /** Ctor initialize directory and sizes */
      MountPoint( const std::string & d = "/", long long bs = 0LL, long long total = 0LL, long long used = 0LL, long long pkg = 0LL, bool ro=false)
	: dir(d), block_size(bs), total_size(total), used_size(used), pkg_size(pkg), readonly(ro)
      {}
      /** \overload to allow e.g. initiailzer lists
       * \code
       *   MountPointSet( { "/", "/usr", "/var" } )
       * \endcode
       */
      MountPoint( const char * d, long long bs = 0LL, long long total = 0LL, long long used = 0LL, long long pkg = 0LL, bool ro=false)
	: dir(d?d:""), block_size(bs), total_size(total), used_size(used), pkg_size(pkg), readonly(ro)
      {}

      /** Sort by directory name */
      bool operator<( const MountPoint & rhs ) const
      { return dir < rhs.dir; }

      /** Block size of the filesystem as \ref ByteCount for convenience. */
      ByteCount blockSize() const
      { return ByteCount( block_size, ByteCount::B ); }

      /** Total size of the filesystem as \ref ByteCount for convenience. */
      ByteCount totalSize() const
      { return ByteCount( total_size, ByteCount::K ); }

      /** Used size of the filesystem as \ref ByteCount for convenience. */
      ByteCount usedSize() const
      { return ByteCount( used_size, ByteCount::K ); }

      /** Free size of the filesystem as \ref ByteCount for convenience. */
      ByteCount freeSize() const
      { return ByteCount( total_size-used_size, ByteCount::K ); }

      /** Used size after installation as \ref ByteCount for convenience. */
      ByteCount usedAfterCommit() const
      { return ByteCount( pkg_size, ByteCount::K ); }

      /** Free size after installation as \ref ByteCount for convenience. */
      ByteCount freeAfterCommit() const
      { return ByteCount( total_size-pkg_size, ByteCount::K ); }

      /** Size change due to installation as \ref ByteCount for convenience. */
      ByteCount commitDiff() const
      { return ByteCount( pkg_size-used_size, ByteCount::K ); }
    };
    ///////////////////////////////////////////////////////////////////

    typedef std::set<MountPoint> MountPointSet;

    DiskUsageCounter()
    {}

    /** Ctor taking the MountPointSet to compute */
    DiskUsageCounter( const MountPointSet & mps_r )
    : _mps( mps_r )
    {}

    /** Set a MountPointSet to compute */
    void setMountPoints( const MountPointSet & mps_r )
    { _mps = mps_r; }

    /** Get the current MountPointSet */
    const MountPointSet & getMountPoints() const
    { return _mps; }


    /** Get mountpoints of system below \a rootdir */
    static MountPointSet detectMountPoints( const std::string & rootdir = "/" );

    /** Only one entry for "/" to collect total sizes */
    static MountPointSet justRootPartition();


    /** Compute disk usage if the current transaction woud be commited. */
    MountPointSet disk_usage( const ResPool & pool ) const;

    /** Compute disk usage of a single Solvable */
    MountPointSet disk_usage( sat::Solvable solv_r ) const;
    /** \overload for PoolItem */
    MountPointSet disk_usage( const PoolItem & pi_r  ) const
    { return disk_usage( sat::asSolvable()( pi_r ) ); }
    /** \overload for ResObject */
    MountPointSet disk_usage( const ResObject::constPtr & obj_r ) const
    { return disk_usage( sat::asSolvable()( obj_r ) ); }

    /** Compute disk usage of a collection defined by a solvable bitmap. */
    MountPointSet disk_usage( const Bitmap & bitmap_r ) const;

    /** Compute disk usage of a collection (convertible by \ref asSolvable). */
    template<class Iterator>
    MountPointSet disk_usage( Iterator begin_r, Iterator end_r ) const
    {
      Bitmap bitmap( Bitmap::poolSize );
      for_( it, begin_r, end_r )
	bitmap.set( sat::asSolvable()( *it ).id() );
      return disk_usage( bitmap );
    }

  private:
    MountPointSet _mps;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates DiskUsageCounter::MountPoint Stream output */
  std::ostream & operator<<( std::ostream & str, const DiskUsageCounter::MountPoint & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DISKUSAGE_COUNTER_H
