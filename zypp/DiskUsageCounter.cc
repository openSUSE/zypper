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
#include "zypp/DiskUsageCounter.h"

#include <zypp/Package.h>
#include <zypp/base/String.h>

#include <fstream>

#include <sys/statvfs.h>
       

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DiskUsageCounter::MountPointSet DiskUsageCounter::disk_usage(const ResPool &pool)
  {
    MountPointSet result = mps;

    if (mps.empty())
    {
      // partitioning is not set
      return result;	
    }
    
    // set used size after commit to the current used size
    for (MountPointSet::iterator mpit = result.begin();
      mpit != result.end();
      mpit++)
    {
      mpit->pkg_size = mpit->used_size;
    }

    // iterate through all packages
    for (ResPool::byKind_iterator it = pool.byKindBegin(ResTraits<Package>::kind);
      it != pool.byKindEnd(ResTraits<Package>::kind);
      ++it)
    {
      bool inst = it->status().isToBeInstalled();
      bool rem = it->status().isToBeUninstalled();

      // if the package is not selected for installation or removing
      // it can't affect disk usage
      if (inst || rem)
      {
	Package::constPtr pkg = boost::dynamic_pointer_cast<const Package>( it->resolvable() );
	DiskUsage du = pkg->diskusage();

	// iterate trough all mount points, add usage to each directory
	// directory tree must be processed from leaves to the root directory
	// so iterate in reverse order so e.g. /usr is used before /
	for (MountPointSet::reverse_iterator mpit = result.rbegin();
	  mpit != result.rend();
	  mpit++)
	{
	  // get usage for the mount point
	  DiskUsage::Entry entry = du.extract(mpit->dir);

	  // add or subtract it to the current value
	  if (inst)
	  {
	      mpit->pkg_size += entry._size;
	  }
	  else // the package will be uninstalled
	  {
	      mpit->pkg_size -= entry._size;
	  }
	}
      }
    }

    return result;    
  }


  DiskUsageCounter::MountPointSet DiskUsageCounter::detectMountPoints(const std::string &rootdir)
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

	    //
	    // Filter some common unwanted mountpoints
	    //
	    char * mpunwanted[] = {
	      "/mnt", "/media", "/mounts", "/floppy", "/cdrom",
	      "/suse", "/var/tmp", "/var/adm/mount", "/var/adm/YaST",
	      /*last*/0/*entry*/
	    };

	    char ** nomp = mpunwanted;
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
	      ret.insert( DiskUsageCounter::MountPoint( mp, sb.f_bsize, 
		((long long)sb.f_blocks)*sb.f_bsize/1024,
		((long long)(sb.f_blocks - sb.f_bfree))*sb.f_bsize/1024, 0LL, ro ) );
	    }
	  }
	}
    }

    return ret;
  }

} // namespace zypp
///////////////////////////////////////////////////////////////////
