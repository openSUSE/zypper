/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMPackageImpl.cc
 *
*/

#include "zypp/source/yum/YUMPackageImpl.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    {
      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMPackageImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
       * \bug CANT BE CONSTUCTED THAT WAY ANYMORE
      */
      YUMPackageImpl::YUMPackageImpl(
	const zypp::parser::yum::YUMPatchPackage & parsed
      )
#warning MA cannot be constructed that way
#if 0
      : PackageImpl(
	parsed.name,
	Edition(parsed.ver, parsed.rel, strtol(parsed.epoch.c_str(), NULL, 10)),
	Arch(parsed.arch)
      )
#endif
      {
/*
    std::string type;
    std::string checksumType;
    std::string checksumPkgid;
    std::string checksum;
    std::string summary;
    std::string description;
    std::string packager;
    std::string url;
    std::string timeFile;
    std::string timeBuild;
    std::string sizePackage;
    std::string sizeInstalled;
    std::string sizeArchive;
    std::string location;
    std::string license;
    std::string vendor;
    std::string group;
    std::string buildhost;
    std::string sourcerpm;
    std::string headerStart;
    std::string headerEnd;
    std::list<YUMDependency> provides;
    std::list<YUMDependency> conflicts;
    std::list<YUMDependency> obsoletes;
    std::list<YUMDependency> requires;
    std::list<FileData> files;
    // SuSE specific data
    std::list<std::string> authors;
    std::list<std::string> keywords;
    std::string  media;
    std::list<YUMDirSize> dirSizes;
    std::list<YUMDependency> freshen;
    bool installOnly;
    // Change Log
    std::list<ChangelogEntry> changelog;
    // Package Files
    struct {
      std::string arch;
      std::string filename;
      std::string downloadsize;
      std::string md5sum;
      std::string buildtime;
    } plainRpm;
    struct {
      std::string arch;
      std::string filename;
      std::string downloadsize;
      std::string md5sum;
      std::string buildtime;
      std::list<YUMBaseVersion> baseVersions;
    } patchRpm;
    struct {
      std::string arch;
      std::string filename;
      std::string downloadsize;
      std::string md5sum;
      std::string buildtime;
      YUMBaseVersion baseVersion;
    } deltaRpm;






*/
      }
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
