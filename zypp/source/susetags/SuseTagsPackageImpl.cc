/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImpl.cc
 *
*/
#include "zypp/source/susetags/SuseTagsPackageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////
  
      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackageImpl::PackageImpl
      //	METHOD TYPE : Ctor
      //
      SuseTagsPackageImpl::SuseTagsPackageImpl(Source_Ref source_r) :
        _source( source_r )
      {}
  
      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackageImpl::~PackageImpl
      //	METHOD TYPE : Dtor
      //
      SuseTagsPackageImpl::~SuseTagsPackageImpl()
      {}


      TranslatedText SuseTagsPackageImpl::summary() const
      { return _summary; }

      TranslatedText SuseTagsPackageImpl::description() const
      { return _description; }

      Date SuseTagsPackageImpl::buildtime() const
      { return _buildtime; }

      std::string SuseTagsPackageImpl::buildhost() const
      { return std::string(); }

      Date SuseTagsPackageImpl::installtime() const
      { return Date(); }				// it was never installed

      std::string SuseTagsPackageImpl::distribution() const
      { return std::string(); }

      Vendor SuseTagsPackageImpl::vendor() const
      { return Vendor(); }

      Label SuseTagsPackageImpl::license() const
      { return _license; }

      std::string SuseTagsPackageImpl::packager() const
      { return std::string(); }

      PackageGroup SuseTagsPackageImpl::group() const
      { return _group; }

      Changelog SuseTagsPackageImpl::changelog() const
      { return Changelog(); }

      Pathname SuseTagsPackageImpl::location() const
      { return _location; }

      std::string SuseTagsPackageImpl::url() const
      { return std::string(); }

      std::string SuseTagsPackageImpl::os() const
      { return std::string(); }

      Text SuseTagsPackageImpl::prein() const
      { return Text(); }

      Text SuseTagsPackageImpl::postin() const
      { return Text(); }

      Text SuseTagsPackageImpl::preun() const
      { return Text(); }

      Text SuseTagsPackageImpl::postun() const
      { return Text(); }

      ByteCount SuseTagsPackageImpl::size() const
      { return _size; }

      ByteCount SuseTagsPackageImpl::sourcesize() const
	// FIXME
      { return 0; }

      ByteCount SuseTagsPackageImpl::archivesize() const
      { return _archivesize; }

      DiskUsage SuseTagsPackageImpl::diskusage() const
      { return _diskusage; }

      std::list<std::string> SuseTagsPackageImpl::authors() const
      {
        return _authors;
       }

      std::list<std::string> SuseTagsPackageImpl::filenames() const
      { return std::list<std::string>(); }

      License SuseTagsPackageImpl::licenseToConfirm() const
      { return License(); }

      std::list<DeltaRpm> SuseTagsPackageImpl::deltaRpms() const
      { return std::list<DeltaRpm>(); }

      std::list<PatchRpm> SuseTagsPackageImpl::patchRpms() const
      { return std::list<PatchRpm>(); }

      bool SuseTagsPackageImpl::installOnly() const
      { return false; }

      Source_Ref SuseTagsPackageImpl::source() const
      { return _source; }

      unsigned SuseTagsPackageImpl::mediaId() const
      { return _media_number; }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
