
#ifndef ZYPP_SOURCE_SUSETAGSPARSER_H
#define ZYPP_SOURCE_SUSETAGSPARSER_H
// #include <list>
// #include <string>
// #include <fstream>
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"
// #include "zypp/Arch.h"
// #include "zypp/ZYppFactory.h"
// #include "zypp/ZYpp.h"
// #include "zypp/capability/CapabilityImpl.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/parser/taggedfile/TaggedParser.h"
#include "zypp/parser/taggedfile/TaggedFile.h"
#include "zypp/parser/taggedfile/TagCacheRetrieval.h"
#include "zypp/parser/taggedfile/TagCacheRetrievalPtr.h"

namespace zypp
{
  struct PackageDataProvider;
  typedef zypp::shared_ptr<PackageDataProvider> PackageDataProviderPtr;
  struct PackageDataProvider;
  
  struct PackagesParser
  {
    typedef function<bool( int )> Progress;
    
    // tag ids for the TaggedParser
    enum Tags {
      PACKAGE,  // name version release arch
      REQUIRES, // list of requires tags
      PREREQUIRES,// list of pre-requires tags
      PROVIDES, // list of provides tags
      CONFLICTS,  // list of conflicts tags
      OBSOLETES,  // list of obsoletes tags
      RECOMMENDS, // list of recommends tags
      SUGGESTS, // list of suggests tags
      LOCATION, // file location
      SIZE, // packed and unpacked size
      BUILDTIME,  // buildtime
      SOURCERPM,  // source package
      GROUP,  // rpm group
      LICENSE,  // license
      AUTHORS,  // list of authors
      SHAREWITH,  // package to share data with
      KEYWORDS, // list of keywords
  
        // packages.<locale>
      SUMMARY,  // short summary (label)
      DESCRIPTION,// long description
      INSNOTIFY,  // install notification
      DELNOTIFY,  // delete notification
      LICENSETOCONFIRM, // license to confirm upon install
      // packages.DU
      DU,   // disk usage data
      NUM_TAGS
    };
  
    PackagesParser( const data::RecordId &catalog_id, zypp::cache::CacheStore &consumer );
    void start( const zypp::Pathname &path, Progress progress_fnc );
    void fromCache ( TagCacheRetrieval_Ptr pkgcache);
    void collectDeps( zypp::Dep deptype, const std::list<std::string> &depstrlist, data::Dependencies &deps );
    void fromPathLocale (const zypp::Pathname& path);
    void fromLocale ();
    
  private:
    // our parser
    TaggedParser _parser;
    // our set of tags, initialized in constructor
    TaggedFile::TagSet _tagset;
    zypp::Arch _system_arch;
    typedef std::map <std::string, PackageDataProviderPtr> pkgmaptype;
    pkgmaptype _pkgmap;
    zypp::cache::CacheStore &_consumer;
    std::map<std::string, data::RecordId> _idmap;
    data::RecordId _catalog_id;
  };
}

#endif


