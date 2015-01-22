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
#include "zypp/base/Flags.h"

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
      friend std::ostream & operator<<( std::ostream & str, const MountPoint & obj );
      std::string dir;			///< Directory name
      std::string fstype;		///< Filesystem type  (provided by \ref detectMountPoints)
      long long block_size;		///< Block size of the filesystem in B (0 if you don't care)
      long long total_size;		///< Total size of the filesystem in KiB (0 if you don't care)
      long long used_size;		///< Used size of the filesystem in KiB (0 if you don't care)
      mutable long long pkg_size;	///< Used size after installation in KiB (computed by \ref DiskUsageCoutner)
      // hint bits:
      bool readonly:1;			///< hint for readonly partitions
      bool growonly:1;			///< hint for growonly partitions (e.g. snapshotting btrfs)


      /** HinFlags for ctor */
      enum Hint
      {
	NoHint		= 0,
	Hint_readonly	= (1<<0),	///< readonly partitions
	Hint_growonly	= (1<<1),	///< growonly partitions (e.g. snapshotting btrfs)
      };
      ZYPP_DECLARE_FLAGS(HintFlags,Hint);

      /** Ctor initialize directory, fstype and sizes */
      MountPoint( const std::string & d = "/",
		  const std::string & f = std::string(),
		  long long bs = 0LL, long long total = 0LL, long long used = 0LL, long long pkg = 0LL,
		  HintFlags hints = NoHint )
	: dir(d), fstype(f)
	, block_size(bs), total_size(total), used_size(used), pkg_size(pkg)
	, readonly(hints.testFlag(Hint_readonly))
	, growonly(hints.testFlag(Hint_growonly))
      {}
       /** \overload <tt>const char *</tt> to allow e.g. initiailzer lists
       * \code
       *   MountPointSet( { "/", "/usr", "/var" } )
       * \endcode
       */
      MountPoint( const char * d,
		  const std::string & f = std::string(),
		  long long bs = 0LL, long long total = 0LL, long long used = 0LL, long long pkg = 0LL,
		  HintFlags hints = NoHint )
	: MountPoint( std::string(d?d:""), f, bs, total, used, pkg, hints )
      {}


      /** Ctor initialize directory and sizes */
      MountPoint( const std::string & d,
		  long long bs, long long total = 0LL, long long used = 0LL, long long pkg = 0LL,
		  HintFlags hints = NoHint )
	: MountPoint( d, std::string(), bs, total, used, pkg, hints )
      {}
      /** \overload <tt>const char *</tt> */
      MountPoint( const char * d,
		  long long bs, long long total = 0LL, long long used = 0LL, long long pkg = 0LL,
		  HintFlags hints = NoHint )
	: MountPoint( std::string(d?d:""), bs, total, used, pkg, hints )
      {}


      /** Ctor just name and hints, all sizes 0 */
      MountPoint( const std::string & d, HintFlags hints )
	: MountPoint( d, std::string(), 0LL, 0LL, 0LL, 0LL, hints )
      {}
      /** \overload <tt>const char *</tt> */
      MountPoint( const char * d, HintFlags hints )
	: MountPoint( std::string(d?d:""), hints )
      {}
      /** \overload to prevent propagation Hint -> long long */
      MountPoint( const std::string & d, Hint hint )
	: MountPoint( d, HintFlags(hint) )
      {}
      /** \overload to prevent propagation Hint -> long long */
      MountPoint( const char * d, Hint hint )
	: MountPoint( std::string(d?d:""), HintFlags(hint) )
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

    /** Get mountpoints of system below \a rootdir
     * If we happen to detect snapshotting btrfs partitions, the MountPoint::growonly
     * hint is set. Disk usage computation will assume that deleted packages will not
     * free any space (kept in a snapshot).
     */
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

  ZYPP_DECLARE_OPERATORS_FOR_FLAGS(DiskUsageCounter::MountPoint::HintFlags);

  /** \relates DiskUsageCounter::MountPoint Stream output */
  std::ostream & operator<<( std::ostream & str, const DiskUsageCounter::MountPoint & obj );

  /** \relates DiskUsageCounter::MountPointSet Stream output */
  std::ostream & operator<<( std::ostream & str, const DiskUsageCounter::MountPointSet & obj );

  /** \relates DiskUsageCounter Stream output */
  inline std::ostream & operator<<( std::ostream & str, const DiskUsageCounter & obj )
  { return str << obj.getMountPoints(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DISKUSAGE_COUNTER_H
