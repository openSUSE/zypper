/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_DataPACKAGEIMPL_H
#define ZYPP_REPO_DataPACKAGEIMPL_H

#include "zypp/data/ResolvableData.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Source.h"
#include "zypp/DiskUsage.h"
#include "zypp/CheckSum.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repository
  { /////////////////////////////////////////////////////////////////
    namespace data
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : DataPackageImpl
      //
      struct DataPackageImpl : public zypp::detail::PackageImplIf
      {
        DataPackageImpl( zypp::data::Package_Ptr );
        virtual ~DataPackageImpl();

        /** \name ResObject attributes. */
        //@{
        virtual TranslatedText summary() const;
        virtual TranslatedText description() const;
        virtual TranslatedText insnotify() const;
        virtual TranslatedText delnotify() const;
        virtual TranslatedText licenseToConfirm() const;
        virtual Source_Ref source() const;
        virtual unsigned sourceMediaNr() const;
        //@}

        virtual CheckSum checksum() const;
        virtual Date buildtime() const;
        virtual std::string buildhost() const;
        virtual Date installtime() const;
        virtual std::string distribution() const;
        virtual Vendor vendor() const;
        virtual Label license() const;
        virtual std::string packager() const;
        virtual PackageGroup group() const;
       virtual Keywords keywords() const;
	virtual Changelog changelog() const;
        virtual Pathname location() const;
        virtual std::string url() const;
        virtual std::string os() const;
        virtual Text prein() const;
        virtual Text postin() const;
        virtual Text preun() const;
        virtual Text postun() const;
        virtual ByteCount size() const;
        virtual ByteCount sourcesize() const;
        virtual ByteCount archivesize() const;
        virtual DiskUsage diskusage() const;
        virtual std::list<std::string> authors() const;
        virtual std::list<std::string> filenames() const;
        virtual std::list<DeltaRpm> deltaRpms() const;
        virtual std::list<PatchRpm> patchRpms() const;
        virtual bool installOnly() const;
      private:
        zypp::data::Package_Ptr _data;
      };
      ///////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////
    } // namespace Data
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PACKAGEIMPL_H
