/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file        zypp/PatchRpm.h
 *
*/
#ifndef ZYPP_PATCHRPM_H
#define ZYPP_PATCHRPM_H

#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/BaseVersion.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class PatchRpm
  {
  public:
    PatchRpm(const Arch & arch,
             const std::string & filename, 
             const ByteCount & downloadsize,
             const CheckSum & checksum,
             const Date & buildtime,
             const std::list<BaseVersion> & base_versions)
    : _arch(arch)
    , _filename(filename)
    , _downloadsize(downloadsize)
    , _checksum(checksum)
    , _buildtime(buildtime)
    , _base_versions(base_versions)
    {}
    Arch arch() { return _arch; }
    std::string filename() { return _filename; }
    ByteCount downloadsize() { return _downloadsize; }
    CheckSum checksum() { return _checksum; }
    Date buildtime() { return _buildtime; }
    std::list<BaseVersion> baseVersions() { return _base_versions; }
  private:
    Arch _arch;
    std::string _filename;
    ByteCount _downloadsize;
    CheckSum _checksum;
    Date _buildtime;
    std::list<BaseVersion> _base_versions;
  };


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATCHRPM_H
