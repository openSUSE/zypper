/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#ifndef ResolvableData_h
#define ResolvableData_h

#include <iosfwd>
#include <list>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/capability/CapabilityImpl.h"
#include "zypp/Pathname.h"
#include "zypp/NVR.h"
#include "zypp/Edition.h"
#include "zypp/ByteCount.h"
#include "zypp/Arch.h"
#include "zypp/CheckSum.h"
#include "zypp/Url.h"
#include "zypp/Date.h"
#include "zypp/TranslatedText.h"

namespace zypp
{
namespace data
{

  typedef std::set<capability::CapabilityImpl::Ptr> DependencyList;
  typedef std::map<zypp::Dep, DependencyList> Dependencies;

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Resolvable);

  class Resolvable : public base::ReferenceCounted, private base::NonCopyable
  {
    public:
      Resolvable()
      {};

      std::string name;
      Edition edition;
      Arch arch;

      Dependencies deps;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(ResObject);

  class ResObject : public Resolvable
  {
    public:
      ResObject()
        : source_media_nr(1), install_only(false)
      {}

      TranslatedText summary;
      TranslatedText description;

      TranslatedText insnotify;
      TranslatedText delnotify;

      TranslatedText license_to_confirm;
      std::string vendor;

      /** Installed size. \see zypp::ResObject::size() */
      ByteCount size;
      /** RPM package size. \see zypp::ResObject::archive_size() */
      ByteCount archive_size;

      std::string source;

      int source_media_nr;

      bool install_only;

      Date build_time;
      Date install_time;

    protected:
      /** Overload to realize std::ostream & operator\<\<. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(AtomBase);

  class AtomBase : public ResObject
  {
    public:
      enum AtomType { TypePackage, TypeScript, TypeMessage };
      virtual AtomType atomType() = 0;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Script);

  class Script : public AtomBase
  {
    public:
      Script() {};
      virtual AtomType atomType() { return TypeScript; };
      std::string do_script;
      std::string undo_script;
      std::string do_location;
      std::string undo_location;
      std::string do_media;
      std::string undo_media;
      std::string do_checksum_type;
      std::string do_checksum;
      std::string undo_checksum_type;
      std::string undo_checksum;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Message);

  class Message : public AtomBase
  {
    public:
      Message() {};
      virtual AtomType atomType() { return TypeMessage; };
      TranslatedText text;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Selection);

  class Selection : public ResObject
  {
    public:

      Selection() {};
      std::string groupId;
      TranslatedText name;
      std::string default_;
      std::string user_visible;
      TranslatedText description;
      //std::list<MetaPkg> grouplist;
      //std::list<PackageReq> packageList;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Patch);

  class Patch : public ResObject
  {
    public:
      Patch() {};

    /** Patch ID */
    std::string id;
    /** Patch time stamp */
    Date timestamp;
    /** Patch category (recommended, security,...) */
    std::string category;
    /** Does the system need to reboot to finish the update process? */
    bool reboot_needed;
    /** Does the patch affect the package manager itself? */
    bool affects_pkg_manager;
    /** The list of all atoms building the patch */
    //AtomList atoms;
    /** Is the patch installation interactive? (does it need user input?) */
    bool interactive;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Pattern);

  /*
   * Data Object for Pattern
   * resolvable
   */
  class Pattern : public ResObject
  {
    public:

      Pattern()
        : is_default(false), user_visible(true)
      {};

      bool is_default;
      bool user_visible;
      TranslatedText category;
      std::string icon;
      std::string order;
      std::string script;

      DependencyList includes;
      DependencyList extends;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Product);

  /*
   * Data Object for Product
   * resolvable
   */
  class Product : public ResObject
  {
    public:
      Product() {};

      std::string type;
      std::string vendor;
      std::string name;
      std::string distribution_name;
      Edition distribution_edition;
      TranslatedText short_name;
        // those are suse specific tags
      std::string releasenotesurl;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Packagebase);

  /**
   * Common Data Object for Package and Sourcepackage.
   *
   * We treat them as differend kind of Resolvable, but they have
   * almost identical data.
   */
  class Packagebase : public ResObject
  {
    public:
      enum PackageType { BIN, SRC };
      virtual PackageType packageType() const = 0;
    public:
      std::string type;
      CheckSum checksum;
      // changlelog?
      std::string buildhost;
      std::string distribution;
      std::string license;
      std::string packager;
      std::string group;
      /**
       * Upstream home page URL.
       * \see zypp::Package::url();
       */
      std::string url;
      std::string os;

      std::string prein;
      std::string postin;
      std::string preun;
      std::string postun;

      ByteCount source_size;

      std::list<std::string> authors;
      std::list<std::string> keywords;

      Pathname location;
  };

  DEFINE_PTR_TYPE(Package);
  /**
   * Data Object for Package resolvable
   */
  struct Package : public Packagebase
  {
    virtual PackageType packageType() const { return BIN; }

    /** NVR of the corresponding SrcPackage. */
    shared_ptr<NVR> srcPackageIdent;
  };

  DEFINE_PTR_TYPE(SrcPackage);
  /**
   * Data Object for SrcPackage resolvable
   */
  struct SrcPackage : public Packagebase
  {
    virtual PackageType packageType() const { return SRC; }
  };
  ///////////////////////////////////////////////////////////////////

 template<class _Res> class SpecificData;

 template<> class SpecificData<Package>
 {
   public:
    std::string type;
    CheckSum checksum;
      // changlelog?
    std::string buildhost;
    std::string distribution;
    std::string license;
    std::string packager;
    std::string group;
    std::string url;
    std::string os;

    std::string prein;
    std::string postin;
    std::string preun;
    std::string postun;

    ByteCount source_size;

    std::list<std::string> authors;
    std::list<std::string> keywords;

    Pathname location;
 };

} // namespace data
} // namespace zypp


#endif

