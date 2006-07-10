/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/cache/SourceCacher.h"

#include "zypp/base/GzStream.h"
#include "zypp/parser/yum/YUMParser.h"

#include "zypp/parser/SAXParser.h"
#include "zypp/source/yum/YUMSourceCacher.h"

using namespace std;
using namespace zypp::parser::yum;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
namespace yum
{ /////////////////////////////////////////////////////////////////


class YUMPrimaryReader : public parser::SAXParser
{
  public:
  YUMPrimaryReader( YUMSourceCacher &cacher )
  {
    _cacher.reset(&cacher);
  }
  
  virtual void startElement(const std::string name, const xmlChar **atts)
  {
    if ( name == "package" )
      _package.reset(new zypp::data::Package());
  }

  virtual void characters(const xmlChar *ch, int len)
  {
    _buffer.append( (const char *)ch, len);
  }

  virtual void endElement(const std::string name)
  {
    if ( name == "name" )
      _package->name = popBuffer();
    if ( name == "arch" )
      _package->arch = Arch(popBuffer());

    if ( name == "package" )
    {
      _cacher->packageParsed(*_package);
    }
  }
  
  std::string popBuffer()
  {
    std::string rt = _buffer;
    _buffer.clear();
    return rt;
  }

  private:
  shared_ptr<zypp::data::Package> _package;
  shared_ptr<YUMSourceCacher> _cacher;
  std::string _buffer;
};

YUMSourceCacher::YUMSourceCacher( const Pathname &root_r ) : zypp::cache::SourceCacher(root_r)
{
  
}

YUMSourceCacher::~YUMSourceCacher()
{
}

void YUMSourceCacher::packageParsed( const data::Package &package)
{
  MIL << "caching " << package << std::endl;
}

void YUMSourceCacher::cache( const Url &url, const Pathname &path )
{
  filesystem::TmpDir tmpdir = downloadMetadata(url, path);
  YUMPrimaryReader reader(*this);
  reader.parseFile( tmpdir.path() + "/repodata/primary.xml.gz");

}

filesystem::TmpDir YUMSourceCacher::downloadMetadata(const Url &url, const Pathname &path)
{
  filesystem::TmpDir tmpdir;
  filesystem::TmpDir _cache_dir;

  MediaSetAccess media(url, path);

  int copy_result;
  MIL << "Downloading metadata to " << tmpdir.path() << std::endl;

  Pathname local_dir = tmpdir.path();
  if (0 != assert_dir(local_dir + "/repodata" , 0755))
    ZYPP_THROW(Exception("Cannot create /repodata in download directory"));

  MIL << "Storing data to tmp dir " << local_dir << endl;

  // first read list of all files in the repository
  Pathname remote_repomd;
  try
  {
    remote_repomd = media.provideFile( path + "/repodata/repomd.xml");
  }
  catch(Exception &e)
  {
    ZYPP_THROW(Exception("Can't provide " + path.asString() + "/repodata/repomd.xml from " + url.asString() ));
  }

  // provide optional files
  //Pathname remote_repomd_key;
  //Pathname remote_repomd_signature;
  //try {
  //  remote_repomd_key = tryToProvideFile( _path + "/repodata/repomd.xml.key");
  //}
  //catch( const Exception &e ) {
  //  WAR << "Repository does not contain repomd signing key" << std::endl;
  //}

  //try {
  //  remote_repomd_signature = tryToProvideFile( _path + "/repodata/repomd.xml.asc");
  //}
  //catch( const Exception &e ) {
  //  WAR << "Repository does not contain repomd signature" << std::endl;
  //}

  copy_result = filesystem::copy( remote_repomd, local_dir + "/repodata/repomd.xml");
  if ( copy_result != 0 )
    ZYPP_THROW(Exception("Can't copy " + remote_repomd.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml"));

  //   if (PathInfo(remote_repomd_key).isExist())
  //   {
  //     copy_result = filesystem::copy( remote_repomd_key, local_dir + "/repodata/repomd.xml.key");
  //     if ( copy_result != 0 )
  //       ZYPP_THROW(Exception("Can't copy " + remote_repomd_key.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml.key"));
  //     getZYpp()->keyRing()->importKey(local_dir + "/repodata/repomd.xml.key" , false);
  //   }
  // 
  //   if (PathInfo(remote_repomd_signature).isExist())
  //   {
  //     copy_result = filesystem::copy( remote_repomd_signature, local_dir + "/repodata/repomd.xml.asc");
  //     if ( copy_result != 0 )
  //       ZYPP_THROW(Exception("Can't copy " + remote_repomd_signature.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml.asc"));
  //   }

  DBG << "Reading file " << remote_repomd << endl;
  ifstream repo_st(remote_repomd.asString().c_str());
  YUMRepomdParser repomd(repo_st, "");

  for(; ! repomd.atEnd(); ++repomd)
  {
    if ((*repomd)->type == "other")     // don't parse 'other.xml' (#159316)
      continue;

    if ((*repomd)->type == "filelists")
      continue;

    media.providePossiblyCachedMetadataFile( path + (*repomd)->location, 1, local_dir + (*repomd)->location, _cache_dir + (*repomd)->location, CheckSum((*repomd)->checksumType, (*repomd)->checksum) );

    // if it is a patch, we read the patches individually
    if ((*repomd)->type == "patches")
    {
      // use the local copy now
      Pathname patches_list = local_dir + (*repomd)->location;
      MIL << "Reading patches file " << patches_list << std::endl;
      ifgzstream st ( patches_list.asString().c_str() );
      YUMPatchesParser patch(st, "");
      for (; !patch.atEnd(); ++patch)
      {

        media.providePossiblyCachedMetadataFile( path + (*patch)->location, 1, local_dir + (*patch)->location, _cache_dir + (*patch)->location, CheckSum((*patch)->checksumType, (*patch)->checksum) );
      } // end of single patch parsing
    }// end of patches file parsing
  } // end of copying

  // check signature
  //   MIL << "Checking [" << (local_dir + "/repodata/repomd.xml") << "] signature"  << endl;
  //   if (! getZYpp()->keyRing()->verifyFileSignatureWorkflow(local_dir + "/repodata/repomd.xml", (_path + "/repodata/repomd.xml").asString()+ " (" + url().asString() + ")", local_dir + "/repodata/repomd.xml.asc"))
  //     ZYPP_THROW(Exception(N_("Signed repomd.xml file fails signature check")));

  // ok, now we have a consistent repo in the tmpdir.
  return tmpdir;
}


std::ostream & YUMSourceCacher::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}
}
