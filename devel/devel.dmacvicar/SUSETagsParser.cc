#include <list>
#include <string>
#include <fstream>
#include "zypp/base/Logger.h"
#include "zypp/Arch.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
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

  PackagesParser()
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
    MIL << "Welcome to new parsing" << endl;
    Pathname packages_path = (path + "/suse/setup/descr/packages");
    Pathname locale_path = (path + "/suse/setup/descr/packages.en");

    std::ifstream pkgstream ( packages_path.asString().c_str() );
    std::ifstream lclstream ( locale_path.asString().c_str() );

    if (!pkgstream.is_open())
    {
      ERR << "Cant open " << packages_path << endl;
      ZYPP_THROW( ParseException( "cant open packages file" ) );
    }

    if (!lclstream.is_open())
    {
      ERR << "Cant open " << locale_path << endl;
      ZYPP_THROW( ParseException( "cant open packages.en file" ) );
    }
    //---------------------------------------------------------------
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
    TagCacheRetrievalPtr localecache (new TagCacheRetrieval (locale_path));
    //TagCacheRetrievalPtr ducache (new TagCacheRetrieval (dupath));

    pkgcache->startRetrieval();
    localecache->startRetrieval();
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
          fromCache (pkgcache, localecache /*, ducache*/);
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
    localecache->stopRetrieval();
    //ducache->stopRetrieval();

    //return ret;
  }

  void fromCache ( TagCacheRetrievalPtr pkgcache, TagCacheRetrievalPtr localecache )
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
    
    std::list<std::string> description;
    if (pkgcache->retrieveData (GET_TAG(DESCRIPTION)->Pos(), pkglist))
    {
      cout << pkglist.front() << endl;
      //collectDeps( pkglist, _nvrad[Dep::REQUIRES] );
    }
    
    pkglist.clear();
  }

  void fromPathLocale (const Pathname& path)
  {
    std::ifstream localestream (path.asString().c_str());

    if (!localestream.is_open())
    {
      ERR << "Cant open " << path.asString() << endl;
      ZYPP_THROW( ParseException( "can't open translation" + path.asString() ) );
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
    }
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
    /*
    SET_CACHE (SUMMARY);
    SET_CACHE (DESCRIPTION);
    SET_CACHE (INSNOTIFY);
    SET_CACHE (DELNOTIFY);
    SET_CACHE (LICENSETOCONFIRM);
    */

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
      PackagesParser parser;
      parser.start("/tmp/repo-cache");
    }
    catch ( const Exception &e )
    {
      cout << "ups! " << e.msg() << std::endl;
    }
    return 0;
}