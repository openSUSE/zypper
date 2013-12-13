/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/DiskUsageCounter.cc
 *
 */
extern "C"
{
#include <sys/statvfs.h>
}

#include <iostream>
#include <fstream>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/DtorReset.h"
#include "zypp/base/String.h"

#include "zypp/DiskUsageCounter.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/detail/PoolImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    DiskUsageCounter::MountPointSet calcDiskUsage( DiskUsageCounter::MountPointSet result, const Bitmap & installedmap_r )
    {
      if ( result.empty() )
      {
        // partitioning is not set
        return result;
      }

      sat::Pool satpool( sat::Pool::instance() );

      // init libsolv result vector with mountpoints
      static const ::DUChanges _initdu = { 0, 0, 0 };
      std::vector< ::DUChanges> duchanges( result.size(), _initdu );
      {
        unsigned idx = 0;
        for_( it, result.begin(), result.end() )
        {
          duchanges[idx].path = it->dir.c_str();
          ++idx;
        }
      }

      // now calc...
      ::pool_calc_duchanges( satpool.get(),
                             const_cast<Bitmap &>(installedmap_r),
                             &duchanges[0],
                             duchanges.size() );

      // and process the result
      {
        unsigned idx = 0;
        for_( it, result.begin(), result.end() )
        {
          static const ByteCount blockAdjust( 2, ByteCount::K ); // (files * blocksize) / (2 * 1K)
          it->pkg_size = it->used_size          // current usage
                       + duchanges[idx].kbytes  // package data size
                       + ( duchanges[idx].files * it->block_size / blockAdjust ); // half block per file
          ++idx;
        }
      }

      return result;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  DiskUsageCounter::MountPointSet DiskUsageCounter::disk_usage( const ResPool & pool_r ) const
  {
    Bitmap bitmap( Bitmap::poolSize );

    // build installedmap (installed != transact)
    // stays installed or gets installed
    for_( it, pool_r.begin(), pool_r.end() )
    {
      if ( it->status().isInstalled() != it->status().transacts() )
      {
        bitmap.set( sat::asSolvable()(*it).id() );
      }
    }
    return calcDiskUsage( _mps, bitmap );
  }

  DiskUsageCounter::MountPointSet DiskUsageCounter::disk_usage( sat::Solvable solv_r ) const
  {
    Bitmap bitmap( Bitmap::poolSize );
    bitmap.set( solv_r.id() );

    // temp. unset @system Repo
    DtorReset tmp( sat::Pool::instance().get()->installed );
    sat::Pool::instance().get()->installed = nullptr;

    return calcDiskUsage( _mps, bitmap );
  }

  DiskUsageCounter::MountPointSet DiskUsageCounter::disk_usage( const Bitmap & bitmap_r ) const
  {
    // temp. unset @system Repo
    DtorReset tmp( sat::Pool::instance().get()->installed );
    sat::Pool::instance().get()->installed = nullptr;

    return calcDiskUsage( _mps, bitmap_r );
  }

  DiskUsageCounter::MountPointSet DiskUsageCounter::detectMountPoints( const std::string & rootdir )
  {
    DiskUsageCounter::MountPointSet ret;

      std::ifstream procmounts( "/proc/mounts" );

      if ( !procmounts ) {
	WAR << "Unable to read /proc/mounts" << std::endl;
      } else {

	std::string prfx;
	if ( rootdir != "/" )
	  prfx = rootdir; // rootdir not /

	while ( procmounts ) {
	  std::string l = str::getline( procmounts );
	  if ( !(procmounts.fail() || procmounts.bad()) ) {
	    // data to consume

	    // rootfs 	/ 		rootfs 		rw 0 0
	    // /dev/root 	/ 		reiserfs	rw 0 0
	    // proc 	/proc 		proc		rw 0 0
	    // devpts 	/dev/pts 	devpts		rw 0 0
	    // /dev/hda5 	/boot 		ext2		rw 0 0
	    // shmfs 	/dev/shm 	shm		rw 0 0
	    // usbdevfs 	/proc/bus/usb 	usbdevfs	rw 0 0

	    std::vector<std::string> words;
	    str::split( l, std::back_inserter(words) );

	    if ( words.size() < 3 ) {
	      WAR << "Suspicious entry in /proc/mounts: " << l << std::endl;
	      continue;
	    }

	    //
	    // Filter devices without '/' (proc,shmfs,..)
	    //
	    if ( words[0].find( '/' ) == std::string::npos ) {
	      DBG << "Discard mount point : " << l << std::endl;
	      continue;
	    }

	    // remove /proc entry
	    if (words[0] == "/proc")
	    {
	      DBG << "Discard /proc filesystem: " << l << std::endl;
	      continue;
	    }

	    //
	    // Filter mountpoints not at or below _rootdir
	    //
	    std::string mp = words[1];
	    if ( prfx.size() ) {
	      if ( mp.compare( 0, prfx.size(), prfx ) != 0 ) {
		// mountpoint not below rootdir
		DBG << "Unwanted mount point : " << l << std::endl;
		continue;
	      }
	      // strip prfx
	      mp.erase( 0, prfx.size() );
	      if ( mp.empty() ) {
		mp = "/";
	      } else if ( mp[0] != '/' ) {
		// mountpoint not below rootdir
		DBG << "Unwanted mount point : " << l << std::endl;
		continue;
	      }
	    }

	    //
	    // Filter cdrom
	    //
	    if ( words[2] == "iso9660" ) {
	      DBG << "Discard cdrom : " << l << std::endl;
	      continue;
	    }

	    if ( words[2] == "vfat" || words[2] == "fat" || words[2] == "ntfs" || words[2] == "ntfs-3g")
	    {
	      MIL << words[1] << " contains ignored fs (" << words[2] << ')' << std::endl;
	      continue;
	    }

	    //
	    // Filter some common unwanted mountpoints
	    //
	    const char * mpunwanted[] = {
	      "/mnt", "/media", "/mounts", "/floppy", "/cdrom",
	      "/suse", "/var/tmp", "/var/adm/mount", "/var/adm/YaST",
	      /*last*/0/*entry*/
	    };

	    const char ** nomp = mpunwanted;
	    for ( ; *nomp; ++nomp ) {
	      std::string pre( *nomp );
	      if ( mp.compare( 0, pre.size(), pre ) == 0 // mp has prefix pre
		   && ( mp.size() == pre.size() || mp[pre.size()] == '/' ) ) {
		break;
	      }
	    }
	    if ( *nomp ) {
	      DBG << "Filter mount point : " << l << std::endl;
	      continue;
	    }

	    //
	    // Check whether mounted readonly
	    //
	    bool ro = false;
	    std::vector<std::string> flags;
	    str::split( words[3], std::back_inserter(flags), "," );

	    for ( unsigned i = 0; i < flags.size(); ++i ) {
	      if ( flags[i] == "ro" ) {
		ro = true;
		break;
	      }
	    }
            if ( ro ) {
	      DBG << "Filter ro mount point : " << l << std::endl;
	      continue;
	    }

	    //
	    // statvfs (full path!) and get the data
	    //
	    struct statvfs sb;
	    if ( statvfs( words[1].c_str(), &sb ) != 0 ) {
	      WAR << "Unable to statvfs(" << words[1] << "); errno " << errno << std::endl;
	      ret.insert( DiskUsageCounter::MountPoint( mp ) );
	    }
	    else
	    {
	      //
	      // Filter zero sized devices (bnc#769819)
	      //
	      if ( sb.f_blocks == 0 || sb.f_bsize == 0 )
	      {
		DBG << "Filter zero-sized mount point : " << l << std::endl;
		continue;
	      }
	      ret.insert( DiskUsageCounter::MountPoint( mp, sb.f_bsize,
		((long long)sb.f_blocks)*sb.f_bsize/1024,
		((long long)(sb.f_blocks - sb.f_bfree))*sb.f_bsize/1024, 0LL, ro ) );
	    }
	  }
	}
    }

    return ret;
  }

  DiskUsageCounter::MountPointSet DiskUsageCounter::justRootPartition()
  {
    DiskUsageCounter::MountPointSet ret;
    ret.insert( DiskUsageCounter::MountPoint() );
    return ret;
  }

  std::ostream & operator<<( std::ostream & str, const DiskUsageCounter::MountPoint & obj )
  {
     str << "dir:[" << obj.dir << "] [ bs: " << obj.blockSize()
        << " ts: " << obj.totalSize()
        << " us: " << obj.usedSize()
        << " (+-: " << obj.commitDiff()
        << ")]";
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
