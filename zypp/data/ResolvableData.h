/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/data/ResolvableData.h
 *
*/
#ifndef ZYPP_DATA_RESOLVABLEDATA_H
#define ZYPP_DATA_RESOLVABLEDATA_H

#include <iosfwd>
#include <list>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/data/RecordId.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/Pathname.h"
#include "zypp/NVR.h"
#include "zypp/Edition.h"
#include "zypp/ByteCount.h"
#include "zypp/Arch.h"
#include "zypp/CheckSum.h"
#include "zypp/Changelog.h"
#include "zypp/Url.h"
#include "zypp/Date.h"
#include "zypp/TranslatedText.h"

namespace zypp
{
namespace data
{
  /** ::data dependencies are CapabilityImpl. */
  typedef std::set<capability::CapabilityImpl::Ptr> DependencyList;
  typedef std::map<zypp::Dep, DependencyList>       Dependencies;

  typedef DefaultIntegral<unsigned,0u>              MediaNr;

  /** Data to retrieve a file from some media. */
  struct Location
  {
    Location()
      : fileSize( -1 ), gzSize( -1 )
    {}

    /** Media number (0==no media access required). */
    MediaNr     mediaNr;
    /** Path on the media. */
    Pathname    filePath;
    /** The uncompressed files size. */
    ByteCount   fileSize;
    /** The uncompressed files checksum. */
    CheckSum    fileChecksum;
    /** The compressed (gz) files size. */
    ByteCount   gzSize;
    /** The compressed (gz) files checksum. */
    CheckSum    gzChecksum;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Resolvable);

  /** Mandatory resolvable data. */
  class Resolvable : public base::ReferenceCounted, private base::NonCopyable
  {
    public:
      Resolvable()
      {};

      /** Name */
      std::string name;
      /** Edition */
      Edition edition;
      /** Architecture */
      Arch arch;
      /** Dependencies */
      Dependencies deps;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(ResObject);

  /** Common resolvable data. */
  class ResObject : public Resolvable
  {
    public:
      ResObject()
      {}

      /** Share some data with another resolvable.*/
      RecordId shareDataWith;

      // Common attributes:
      /** Vendor */
      std::string vendor;
      /** Installed size (UI hint). */
      ByteCount installedSize;
      /** Bildtime. */
      Date buildTime;

      // Flags:
      /** 'rpm -i' mode. */
      DefaultIntegral<bool,false> installOnly;

      // Translated texts:
      /** One line summary. */
      TranslatedText summary;
      /** Multiline description. */
      TranslatedText description;
      /** License to confirm. */
      TranslatedText licenseToConfirm;
      /** UI notification text if selected to install. */
      TranslatedText insnotify;
      /** UI notification text if selected to delete. */
      TranslatedText delnotify;

      // Repository related:
      /** Repository providing this resolvable. */
      RecordId  repository;

    protected:
      /** Overload to realize std::ostream & operator\<\<. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Atom);

  /* Data Object for Atom resolvable. */
  class Atom : public ResObject
  {
    public:
      Atom()
      {};
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Script);

  /* Data Object for Script resolvable. */
  class Script : public ResObject
  {
    public:
      Script()
      {};

      /** Inlined doScript. */
      std::string doScript;
      /** Location of doScript on the repositories media. */
      Location doScriptLocation;

      /** Inlined undoScript. */
      std::string undoScript;
      /** Location of undoScript on the repositories media. */
      Location undoScriptLocation;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Message);

  /* Data Object for Message resolvable. */
  class Message : public ResObject
  {
    public:
      Message()
      {};

      /** Inlined Text. */
      TranslatedText text;
      /** Location od textfile on the repositories media. */
      //Location  repositoryLoaction;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Patch);

  /* Data Object for Patch resolvable. */
  class Patch : public ResObject
  {
    public:
      Patch()
      {};

      /** Patch ID */
      std::string id;
      /** Patch time stamp */
      Date timestamp;
      /** Patch category (recommended, security,...) */
      std::string category;
  
      // Flags:
      /** Does the system need to reboot to finish the update process? */
      DefaultIntegral<bool,false> rebootNeeded;
      /** Does the patch affect the package manager itself? */
      DefaultIntegral<bool,false> affectsPkgManager;

      /**
       * The set of all atoms building the patch. These can be either
       * \ref Atom, \ref Message or \ref Script.
       */
      std::set<ResObject_Ptr> atoms;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Pattern);

  /* Data Object for Pattern resolvable. */
  class Pattern : public ResObject
  {
    public:
      Pattern()
      {}

      // Flags
      /** */
      DefaultIntegral<bool,false> isDefault;
      /** Visible or hidden at the UI. */
      DefaultIntegral<bool,false> userVisible;

      /** Category */
      TranslatedText category;

      /** Icon path. */
      std::string icon;
      /** UI order string */
      std::string order;
      /** ? */
      std::string script;

      /** Included patterns. */
      DependencyList includes;
      /** Extended patterns. */
      DependencyList extends;
  };

  ///////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Product);

  /* Data Object for Product resolvable. */
  class Product : public ResObject
  {
    public:
      Product()
      {};

      /** Abbreviation like \c SLES10 */
      TranslatedText shortName;
      /** More verbose Name like <tt>Suse Linux Enterprise Server 10</tt>*/
      TranslatedText longName;

      /** The product flags.
       * \todo What is it?
      */
      std::list<std::string> flags;

      /** Releasenotes url. */
      Url releasenotesUrl;
      /** Update repositories for the product. */
      std::list<Url> updateUrls;
      /** Additional software for the product.  */
      std::list<Url> extraUrls;
      /** Optional software for the product. */
      std::list<Url> optionalUrls;

      /** Vendor specific distribution id. */
      std::string distributionName;
      /** Vendor specific distribution version. */
      Edition distributionEdition;
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
      /** Location on the repositories media. */
      Location repositoryLocation;

      /** Rpm group.*/
      std::string group;
      /** PackageDb keywors (tags). */
      std::list<std::string> keywords;

      /** Changelog. */
      Changelog changelog;
      /** Author list. */
      std::list<std::string> authors;


      /** Buildhost. */
      std::string buildhost;
      /** Distribution. */
      std::string distribution;
      /** Licensetype. Not the text you have to confirm. */
      std::string license;
      /** Packager. */
      std::string packager;
      /** Upstream home page URL.*/
      std::string url;

      /** operating system **/
      std::string operatingSystem;
      
      /** Pre install script. */
      std::string prein;
      /** Post install script. */
      std::string postin;
      /** Pre uninstall script. */
      std::string preun;
      /** Post uninstall script. */
      std::string postun;
  };

  DEFINE_PTR_TYPE(Package);
  /**
   * Data Object for Package resolvable
   */
  struct Package : public Packagebase
  {
    virtual PackageType packageType() const { return BIN; }

    /** NVR of the corresponding SrcPackage. */
    NVR srcPackageIdent;
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

} // namespace data
} // namespace zypp
#endif // ZYPP_DATA_RESOLVABLEDATA_H
