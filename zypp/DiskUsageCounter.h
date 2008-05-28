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

#include <zypp/ResPool.h>

#include <set>
#include <string>
#include <iosfwd>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class DiskUsageCounter
  {

  public:

    /**
    * @short Mount point description
    **/
    struct MountPoint
    {
      /**
       * Directory name
       **/
      std::string dir;

      /**
       * Block size of the mount point
       **/
      long long block_size;

      /**
       * Total size in K (1024)
       **/
      long long total_size;

      /**
       * Used size in  (1024)
       **/
      long long used_size;

      /**
       * Used size after commiting the pool (in kB)
       **/
      mutable long long pkg_size;

      bool readonly;

      /**
       * Ctor - initialize directory and package size
       **/
      MountPoint(std::string d = "/", long long bs = 0LL, long long total = 0LL, long long used = 0LL, long long pkg = 0LL, bool ro=false) :
        dir(d), block_size(bs), total_size(total), used_size(used), pkg_size(pkg), readonly(ro) {}

      // sort by directory name
      bool operator<( const MountPoint & rhs ) const
      {
        return dir < rhs.dir;
      }
    };

    typedef std::set<MountPoint> MountPointSet;

    DiskUsageCounter() {}

    bool setMountPoints(const MountPointSet &m)
    {
	mps = m;
	return true;
    }

    MountPointSet getMountPoints() const
    {
	return mps;
    }

    static MountPointSet detectMountPoints(const std::string &rootdir = "/");

    /**
     * Compute disk usage of the pool
     **/
    MountPointSet disk_usage(const ResPool &pool);

  private:

    MountPointSet mps;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates DiskUsageCounter::MountPoint Stream output */
  std::ostream & operator<<( std::ostream & str, const DiskUsageCounter::MountPoint & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DISKUSAGE_COUNTER_H
