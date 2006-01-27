/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/DeltaRpm.h
 *
*/
#ifndef ZYPP_DELTARPM_H
#define ZYPP_DELTARPM_H

#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/BaseVersion.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class DeltaRpm
  {
  public:
    DeltaRpm(const Arch & arch,
             const Pathname & filename, 
    	 const ByteCount & downloadsize,
    	 const CheckSum & checksum,
    	 const Date & buildtime,
    	 const BaseVersion & base_version)
    : _arch(arch)
    , _filename(filename)
    , _downloadsize(downloadsize)
    , _checksum(checksum)
    , _buildtime(buildtime)
    , _base_version(base_version)
    {}
    Arch arch() const { return _arch; }
    Pathname filename() const { return _filename; }
    ByteCount downloadsize() const { return _downloadsize; }
    CheckSum checksum() const { return _checksum; }
    Date buildtime() const { return _buildtime; }
    BaseVersion baseVersion() const { return _base_version; }
  private:
    Arch _arch;
    Pathname _filename;
    ByteCount _downloadsize;
    CheckSum _checksum;
    Date _buildtime;
    BaseVersion _base_version;
  };

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DELTARPM_H
