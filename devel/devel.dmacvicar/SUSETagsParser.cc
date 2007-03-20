#include <list>
#include <string>
#include <fstream>
#include "zypp/base/Logger.h"
#include "zypp/Arch.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp/parser/taggedfile/TaggedParser.h"
#include "zypp/parser/taggedfile/TaggedFile.h"
#include "zypp/parser/taggedfile/TagCacheRetrieval.h"
#include "zypp/parser/taggedfile/TagCacheRetrievalPtr.h"

using namespace zypp;
using namespace std;

typedef Exception ParseException;

struct PackageDataProvider;

typedef shared_ptr<PackageDataProvider> PackageDataProviderPtr;

struct PackageDataProvider
{
  TagRetrievalPos _attr_SUMMARY;
  TagRetrievalPos _attr_DESCRIPTION;
  TagRetrievalPos _attr_INSNOTIFY;
  TagRetrievalPos _attr_DELNOTIFY;
  TagRetrievalPos _attr_LICENSETOCONFIRM;

  // retrieval pointer for packages data
  TagCacheRetrievalPtr _package_retrieval;
  // retrieval pointer for packages.<locale> data
  TagCacheRetrievalPtr _locale_retrieval;
  // retrieval pointer for packages.DU data
  TagCacheRetrievalPtr _du_retrieval;
  // fallback provider (Share entry in packages)
  PackageDataProviderPtr _fallback_provider;
};

struct PackagesParser
{
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

  // our parser
  TaggedParser _parser;
  // our set of tags, initialized in constructor
  TaggedFile::TagSet _tagset;
  zypp::Arch _system_arch;

  typedef std::map <std::string, PackageDataProviderPtr> pkgmaptype;
  pkgmaptype _pkgmap;

  zypp::cache::CacheStore *_consumer;

  std::map<std::string, data::RecordId> _idmap;

  PackagesParser( zypp::cache::CacheStore *consumer )
    : _consumer(consumer)
  {
    ZYpp::Ptr z = getZYpp();
    _system_arch = z->architecture();
    
    // initialize tagset
    _tagset.setAllowMultipleSets (true);  // multiple tagsets per file
    _tagset.setAllowUnknownTags (true);   // skip unknown tags

    // Using loop and switch to get a compiler warning, if tags are
    // defined but uninitialized, or vice versa.
    for ( Tags tag = Tags(0); tag < NUM_TAGS; tag = Tags(tag+1) )
    {
      switch ( tag )
      {
  #define DEFTAG(T,ARGS) case T: _tagset.addTag ARGS; break
        // for the 'packages' file
        DEFTAG( PACKAGE,  ( "Pkg", tag, TaggedFile::SINGLE, TaggedFile::START) );
        DEFTAG( REQUIRES, ( "Req", tag, TaggedFile::MULTI) );
        DEFTAG( PREREQUIRES,  ( "Prq", tag, TaggedFile::MULTI) );
        DEFTAG( PROVIDES, ( "Prv", tag, TaggedFile::MULTI) );
        DEFTAG( CONFLICTS,  ( "Con", tag, TaggedFile::MULTI) );
        DEFTAG( OBSOLETES,  ( "Obs", tag, TaggedFile::MULTI) );
        DEFTAG( RECOMMENDS, ( "Rec", tag, TaggedFile::MULTI) );
        DEFTAG( SUGGESTS, ( "Sug", tag, TaggedFile::MULTI) );
      
        DEFTAG( LOCATION, ( "Loc", tag, TaggedFile::SINGLE) );
        DEFTAG( SIZE,   ( "Siz", tag, TaggedFile::SINGLE) );
        DEFTAG( BUILDTIME,  ( "Tim", tag, TaggedFile::SINGLE) );
        DEFTAG( SOURCERPM,  ( "Src", tag, TaggedFile::SINGLEPOS) );
        DEFTAG( GROUP,    ( "Grp", tag, TaggedFile::SINGLE) );
        DEFTAG( LICENSE,  ( "Lic", tag, TaggedFile::SINGLEPOS) );
        DEFTAG( AUTHORS,  ( "Aut", tag, TaggedFile::MULTI) );
        DEFTAG( SHAREWITH,  ( "Shr", tag, TaggedFile::SINGLE) );
        DEFTAG( KEYWORDS, ( "Key", tag, TaggedFile::MULTI) );
        
        // for the 'packages.<locale>' file
        DEFTAG( SUMMARY,  ( "Sum", tag, TaggedFile::SINGLE) );
        DEFTAG( DESCRIPTION,  ( "Des", tag, TaggedFile::MULTI) );
        DEFTAG( INSNOTIFY,  ( "Ins", tag,   TaggedFile::MULTI) );
        DEFTAG( DELNOTIFY,  ( "Del", tag,   TaggedFile::MULTI) );
        DEFTAG( LICENSETOCONFIRM,( "Eul", tag,   TaggedFile::MULTI) );
        // for the 'packages.DU' file
        DEFTAG( DU,   ( "Dir", DU,  TaggedFile::MULTI) );

        // No default: let compiler warn missing enumeration values
        case NUM_TAGS: break;
#undef DEFTAG
      }
    }
  }

  void start( const Pathname &path )
  {
    std::ifstream content_stream((path + "/content").asString().c_str());
    std::string buffer;
    Pathname descr_dir;
  
    // Note this code assumes DESCR comes before as META
    while ( content_stream && !content_stream.eof())
    {
      getline(content_stream, buffer);
      if ( buffer.substr( 0, 5 ) == "DESCR" )
      {
        std::vector<std::string> words;
        if ( str::split( buffer, std::back_inserter(words) ) != 2 )
        {
          // error
          ZYPP_THROW(Exception("bad DESCR line")); 
        }
        descr_dir = words[1];
      }
    }

    list<Locale> locales;
    ZYpp::Ptr z = getZYpp();
    Locale curr_locale( z->getTextLocale() );

    while ( curr_locale != Locale() )
    {
      locales.push_back(curr_locale);
      curr_locale = curr_locale.fallback();
    }
    MIL << "I have " << locales.size() << " locales" << endl;
    
    MIL << "Welcome to new parsing" << endl;
    Pathname packages_path = (path + descr_dir + "/packages");
    
    std::ifstream pkgstream ( packages_path.asString().c_str() );

    if (!pkgstream.is_open())
    {
      ERR << "Cant open " << packages_path << endl;
      ZYPP_THROW( ParseException( "cant open packages file" ) );
    }

    // find initial version tag
    // find any initial tag
    TaggedParser::TagType type = _parser.lookupTag (pkgstream);
    if ((type != TaggedParser::SINGLE) || (_parser.currentTag() != "Ver") || (!_parser.currentLocale().empty()))
    {
      ERR << path << ": Initial '=Ver:' tag missing" << endl;
      ZYPP_THROW( ParseException( "bad packages file" ) );
    }

    string version = _parser.data();
    if (version != "2.0")
    {
      ERR << path << ": Version '" << version << "' != 2.0" << endl;
      ZYPP_THROW( ParseException( "bad packages file" ) );
    }
    
    //---------------------------------------------------------------
    // assign set repeatedly

    // create a single cache for all packages
    TagCacheRetrievalPtr pkgcache (new TagCacheRetrieval (packages_path));
    
    //TagCacheRetrievalPtr ducache (new TagCacheRetrieval (dupath));

    pkgcache->startRetrieval();
    //ducache->startRetrieval();

    //PMError ret = PMError::E_ok;
    for (;;)
    {
      TaggedFile::assignstatus status = _tagset.assignSet (_parser, pkgstream);

      if (status == TaggedFile::REJECTED_EOF)
        break;

      if (status == TaggedFile::ACCEPTED_FULL)
      {
        try
        {
          fromCache (pkgcache);
        }
        catch ( const Exception &e )
        {
          ERR << path << ":" << _parser.lineNumber() << endl;
          ZYPP_RETHROW(e);
        }
      }
      else
      {
        ERR << path << ":" << _parser.lineNumber() << endl;
        ERR << "Status " << (int)status << ", Last tag read: " << _parser.currentTag();
        if (!_parser.currentLocale().empty()) ERR << "." << _parser.currentLocale();
        ERR << endl;
        //ret = InstSrcError::E_data_bad_packages;
        ZYPP_THROW( ParseException( "bad packages file" ) );
        break;
      }
    }

    pkgcache->stopRetrieval();
    
    //ducache->stopRetrieval();

    // now parse locales files
    for ( list<Locale>::const_iterator it = locales.begin(); it != locales.end(); ++it )
    {
      Locale locale = *it;
      Pathname locale_path = (path + descr_dir + "/packages." + locale.code() );

      if ( ! PathInfo(locale_path).isExist() )
      {
        ERR << locale_path << " doesn't exists." << endl;
        continue;
      }
      
      TagCacheRetrievalPtr localecache (new TagCacheRetrieval (locale_path));
      localecache->startRetrieval();
      
      std::ifstream localestream (path.asString().c_str());
  
      if (!localestream.is_open())
      {
        ERR << "Cant open " << path.asString() << endl;
        ZYPP_THROW( ParseException( "can't open translation" + locale_path.asString() ) );
      }
  
      //---------------------------------------------------------------
      // find initial version tag
  
      // find any initial tag
      TaggedParser::TagType type = _parser.lookupTag (localestream);
      if ((type != TaggedParser::SINGLE) || (_parser.currentTag() != "Ver") || (!_parser.currentLocale().empty()))
      {
        ERR << path << ": Initial '=Ver:' tag missing" << endl;
        //return InstSrcError::E_data_bad_packages_lang;
      }
  
      string version = _parser.data();
      if (version != "2.0")
      {
        ERR << path << ": Version '" << version << "' != 2.0" << endl;
        ZYPP_THROW( ParseException( "wrong packages.en version" ) );
      }
  
      //---------------------------------------------------------------
      // assign set repeatedly
      for (;;)
      {
        TaggedFile::assignstatus status = _tagset.assignSet (_parser, localestream);
        if (status == TaggedFile::REJECTED_EOF)
          break;
  
        if (status == TaggedFile::ACCEPTED_FULL)
        {
          try
          {
            fromLocale ();
          }
          catch ( const Exception &e )
          {
            ERR << path << ":" << _parser.lineNumber() << endl;
            ZYPP_RETHROW(e);
          }
        }
        else
        {
          ERR << path << ":" << _parser.lineNumber() << endl;
          ERR << "Status " << (int)status << ", Last tag read: " << _parser.currentTag();
          ERR << endl;
          ZYPP_THROW( ParseException( "bad packages file" ) );
        }
      } //loop
      
      localecache->stopRetrieval();
      
    } // locales iterator
  }

  void fromCache ( TagCacheRetrievalPtr pkgcache)
  {
    //---------------------------------------------------------------
    // PACKAGE
    string single ((_tagset.getTagByIndex (PACKAGE))->Data());
    if (single.empty ())
    {
      ERR << "No '=Pkg' value found" << endl;
      ZYPP_THROW( ParseException( "No '=Pkg' value found" ) );
      //return InstSrcError::E_data_bad_packages;
    }
    
    /*
    pkgmaptype::iterator pkgpos = _pkgmap.find (single);
    if (pkgpos != _pkgmap.end())
    {
      ERR << "Duplicate '=Pkg' value '" << single << "'" << endl;
      return InstSrcError::E_data_bad_packages;
    }
    */
    
    std::vector<std::string> splitted;
    str::split (single, std::back_inserter(splitted) );
    if (  splitted.size() != 4 )
    {
      ERR << "Invalid '=Pkg: N V R A' value '" << single << "'" << endl;
      //return InstSrcError::E_data_bad_packages;
      ZYPP_THROW( ParseException( "Invalid '=Pkg: N V R A' value" ) );
    }
    
    // reset
    // this means this is either the first package, or we just finished parsing a package and a new one is starting
    // collect previous pending package if needed
    //collectPkg();
    cout << splitted[0] << " " << splitted[1] << " " << splitted[2] << " " << splitted[3] << endl;
    std::string arch = splitted[3];
    
    // warning: according to autobuild, this is the wrong check
    //  a 'src/norsrc' packages is determined by _not_ having a "=Src:" tag in packages
    // However, the 'arch' string we're checking here must be remembered since
    // it determines the directory below the <DATADIR> where the real package
    // can be found.
    if (arch == "src" || arch == "nosrc")
    {
      arch = "noarch";
      //_srcPkgImpl = SrcPkgImplPtr( new source::susetags::SuseTagsSrcPackageImpl( _source ) );
    }
    //else
    //  _srcPkgImpl = NULL;

    //_pkgImpl = PkgImplPtr( new source::susetags::SuseTagsPackageImpl( _source ) );
    //_nvrad = NVRAD( splitted[0], Edition( splitted[1], splitted[2] ), Arch( arch ) );
     //_pkgImpl->_nvra = NVRA( splitted[0], Edition( splitted[1], splitted[2] ), Arch( arch ) );

     //_isPendingPkg = true;

     // DEPENDENCIES
    #define GET_TAG(tagname) \
    _tagset.getTagByIndex (tagname)
     
    std::list<std::string> pkglist;
    if (pkgcache->retrieveData (GET_TAG(REQUIRES)->Pos(), pkglist))
    {
      cout << pkglist.front() << endl;
      //collectDeps( pkglist, _nvrad[Dep::REQUIRES] );
    }
    
    pkglist.clear();
  }

  void fromPathLocale (const Pathname& path)
  {
    
  }

  void fromLocale ()
  {
    //---------------------------------------------------------------
    // PACKAGE.<locale>

    string single ((_tagset.getTagByIndex (PACKAGE))->Data());
    if (single.empty ())
    {
      ERR << "No '=Pkg' value found" << endl;
      ZYPP_THROW( ParseException( "bad packages file" ) );
    }

    //---------------------------------------------------------------
    // find corresponding package
    pkgmaptype::iterator pkgpos = _pkgmap.find (single);
    if (pkgpos == _pkgmap.end())
    {
      return;
    }
    PackageDataProviderPtr dataprovider = pkgpos->second;
    TaggedFile::Tag *tagptr;    // for SET_MULTI()
#define GET_TAG(tagname) \
    _tagset.getTagByIndex (tagname)
#define SET_CACHE(tagname) \
    do { tagptr = GET_TAG (tagname); dataprovider->_attr_##tagname = tagptr->Pos(); } while (0)
    
    SET_CACHE (SUMMARY);
    SET_CACHE (DESCRIPTION);
    SET_CACHE (INSNOTIFY);
    SET_CACHE (DELNOTIFY);
    SET_CACHE (LICENSETOCONFIRM);
    

    //std::list<std::string> description;
    //if (localecache->retrieveData (GET_TAG(DESCRIPTION)->Pos(), description))
    //{
    //  cout << description.front() << endl;
      //collectDeps( pkglist, _nvrad[Dep::REQUIRES] );
    //}
    
    /*
    if ( ! dataprovider->_attr_LICENSETOCONFIRM.empty() )
    {
      MIL << "Package " << pkgpos->first << " has a license to confirm" << endl;
      PMPackagePtr package = pkgpos->second.first;
      package -> markLicenseUnconfirmed ();

    }
    */
#undef GET_TAG
#undef SET_CACHE
  }

};

int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
      Pathname dbfile = Pathname(getenv("PWD")) + "data.db";
      zypp::cache::CacheStore store(getenv("PWD"));

      PackagesParser parser(&store);
      parser.start(argv[1]);
    }
    catch ( const Exception &e )
    {
      cout << "ups! " << e.msg() << std::endl;
    }
    return 0;
}