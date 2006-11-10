/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMSourceImpl.cc
*
*/

#include "zypp/source/yum/YUMSourceImpl.h"
#include "zypp/source/yum/YUMAtomImpl.h"
#include "zypp/source/yum/YUMPackageImpl.h"
#include "zypp/source/yum/YUMScriptImpl.h"
#include "zypp/source/yum/YUMMessageImpl.h"
#include "zypp/source/yum/YUMPatchImpl.h"
#include "zypp/source/yum/YUMProductImpl.h"
#include "zypp/source/yum/YUMGroupImpl.h"
#include "zypp/source/yum/YUMPatternImpl.h"

#include "zypp/NVRA.h"
#include "zypp/PathInfo.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/CapFactory.h"
#include "zypp/Digest.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"
#include "zypp/ZYppFactory.h"
#include "zypp/KeyRing.h"

#include "zypp/parser/yum/YUMParser.h"
#include "zypp/SourceFactory.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/SilentCallbacks.h"

#include "zypp/base/GzStream.h"
#include "zypp/base/Gettext.h"
#include "zypp/PathInfo.h"

#include <fstream>

using namespace std;
using namespace zypp::detail;
using namespace zypp::parser::yum;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
namespace yum
{

bool YUMProber::operator()()
{
  MIL << "Probing for YUM source..." << std::endl;
  bool result = false;
  media::MediaManager mm;
  result = mm.doesFileExist(_media_id, _path + Pathname("/repodata/repomd.xml"));
  if ( result )
  {
    MIL << "YUM source detected..." << std::endl;
    return true;
  }

  MIL << "Not a YUM source..." << std::endl;
  return false;
}

struct YUMSourceEventHandler
{
  YUMSourceEventHandler( callback::SendReport<SourceReport> &report ) : _report(report)
  {}

  void operator()( int p )
  {
    _report->progress(p);
  }

  callback::SendReport<SourceReport> &_report;
};

static long int get_stream_size( const Pathname &p )
{
  ifgzstream input( p.asString().c_str() );

  if ( input.bad() )
    ZYPP_THROW(Exception("Can't read " + p.asString() + " to calculate compressed stream size"));

  // get the size of the stream
  DBG << "Getting size of the stream." << std::endl;
  input.seekg (0, ios::end);
  long int stream_size = input.tellg();
  DBG << "XML stream size: " << stream_size << std::endl;
  return stream_size;
}

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : YUMSourceImpl
//
///////////////////////////////////////////////////////////////////

YUMSourceImpl::YUMSourceImpl()
{}

Date YUMSourceImpl::timestamp() const
{
  return PathInfo(repomdFile()).mtime();
}

bool YUMSourceImpl::cacheExists()
{
  bool exists = PathInfo(repomdFile()).isExist();
  if (exists)
    MIL << "YUM cache found at " << _cache_dir << std::endl;
  else
    MIL << "YUM cache not found" << std::endl;

  return exists;
}

const Pathname YUMSourceImpl::metadataRoot() const
{
  return _cache_dir.empty() ? tmpMetadataDir() : _cache_dir;
}

const Pathname YUMSourceImpl::repomdFile() const
{
  return metadataRoot() + "/repodata/repomd.xml";
}

const Pathname YUMSourceImpl::repomdFileSignature() const
{
  return metadataRoot() + "/repodata/repomd.xml.asc";
}

const Pathname YUMSourceImpl::repomdFileKey() const
{
  return metadataRoot() + "/repodata/repomd.xml.key";
}

const TmpDir YUMSourceImpl::downloadMetadata()
{
  TmpDir tmpdir;
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
    remote_repomd = provideFile(_path + "/repodata/repomd.xml");
  }
  catch (Exception &e)
  {
    ZYPP_CAUGHT(e);
    ZYPP_THROW(SourceIOException("Can't provide " + _path.asString() + "/repodata/repomd.xml from " + url().asString() ));
  }

  // provide optional files
  Pathname remote_repomd_key;
  Pathname remote_repomd_signature;
  try
  {
    remote_repomd_key = tryToProvideFile( _path + "/repodata/repomd.xml.key");
  }
  catch ( const Exception &e )
  {
    WAR << "Repository does not contain repomd signing key" << std::endl;
  }

  try
  {
    remote_repomd_signature = tryToProvideFile( _path + "/repodata/repomd.xml.asc");
  }
  catch ( const Exception &e )
  {
    WAR << "Repository does not contain repomd signature" << std::endl;
  }

  copy_result = filesystem::copy( remote_repomd, local_dir + "/repodata/repomd.xml");
  if ( copy_result != 0 )
    ZYPP_THROW(SourceIOException("Can't copy " + remote_repomd.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml"));

  if (PathInfo(remote_repomd_key).isExist())
  {
    copy_result = filesystem::copy( remote_repomd_key, local_dir + "/repodata/repomd.xml.key");
    if ( copy_result != 0 )
      ZYPP_THROW(SourceIOException("Can't copy " + remote_repomd_key.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml.key"));

    getZYpp()->keyRing()->importKey(local_dir + "/repodata/repomd.xml.key" , false);
  }

  if (PathInfo(remote_repomd_signature).isExist())
  {
    copy_result = filesystem::copy( remote_repomd_signature, local_dir + "/repodata/repomd.xml.asc");
    if ( copy_result != 0 )
      ZYPP_THROW(SourceIOException("Can't copy " + remote_repomd_signature.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml.asc"));
  }

  DBG << "Reading file " << remote_repomd << endl;
  ifstream repo_st(remote_repomd.asString().c_str());

  callback::SendReport<SourceReport> report;
  report->start( selfSourceRef(), _("Reading index files") );
  YUMRepomdParser repomd(repo_st, "");


  for (; ! repomd.atEnd(); ++repomd)
  {
    if ((*repomd)->type == "other")     // don't parse 'other.xml' (#159316)
      continue;

    getPossiblyCachedMetadataFile( _path + (*repomd)->location, local_dir + (*repomd)->location, _cache_dir + (*repomd)->location, CheckSum((*repomd)->checksumType, (*repomd)->checksum) );

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
        getPossiblyCachedMetadataFile( _path + (*patch)->location, local_dir + (*patch)->location, _cache_dir + (*patch)->location, CheckSum((*patch)->checksumType, (*patch)->checksum) );
      } // end of single patch parsing
    }// end of patches file parsing
  } // end of copying
  report->finish( selfSourceRef(), _("Reading index files"), source::SourceReport::NO_ERROR, "" );


  // check signature
  MIL << "Checking [" << (local_dir + "/repodata/repomd.xml") << "] signature"  << endl;
  if (! getZYpp()->keyRing()->verifyFileSignatureWorkflow(local_dir + "/repodata/repomd.xml", (_path + "/repodata/repomd.xml").asString()+ " (" + url().asString() + ")", local_dir + "/repodata/repomd.xml.asc"))
    ZYPP_THROW(SourceMetadataException(N_("Signed repomd.xml file fails signature check")));

  // ok, now we have a consistent repo in the tmpdir.
  return tmpdir;
}

void YUMSourceImpl::factoryInit()
{
  resetMediaVerifier();

  bool cache = cacheExists();
  if ( cache )
  {
    DBG << "Cached metadata found in [" << _cache_dir << "]." << endl;
    if ( autorefresh() )
      storeMetadata(_cache_dir);
    else
      readRepomd();
  }
  else
  {
    if ( _cache_dir.empty() || !PathInfo(_cache_dir).isExist() )
    {
      DBG << "Cache dir not set. Downloading to temp dir: " << tmpMetadataDir() << std::endl;
      // as we have no local dir set we use a tmp one, but we use a member variable because
      // it cant go out of scope while the source exists.
      saveMetadataTo(tmpMetadataDir());
    }
    else
    {
      DBG << "Cached metadata not found in [" << _cache_dir << "]. Will download." << std::endl;
      saveMetadataTo(_cache_dir);
    }
    readRepomd();
  }

  MIL << "YUM source initialized." << std::endl;
  MIL << "   Url      : " << url() << std::endl;
  MIL << "   Alias    : " << alias() << std::endl;
  MIL << "   Path     : " << path() << std::endl;
  MIL << "   Metadata : " << metadataRoot() << (_cache_dir.empty() ? " [TMP]" : " [CACHE]") << std::endl;
}

bool YUMSourceImpl::downloadNeeded(const Pathname & localdir)
{
  // we can only assume repomd intact means the source changed if the source is signed.
  if ( cacheExists() && PathInfo( repomdFileSignature() ).isExist() )
  {
    Pathname remote_repomd;
    try
    {
      remote_repomd = provideFile(_path + "/repodata/repomd.xml");
    }
    catch (Exception &e)
    {
      ZYPP_THROW(Exception("Can't provide " + _path.asString() + "/repodata/repomd.xml from " + url().asString() ));
    }

    CheckSum old_repomd_checksum( "SHA1", filesystem::sha1sum(localdir + "/repodata/repomd.xml"));
    CheckSum new_repomd_checksum( "SHA1", filesystem::sha1sum(remote_repomd));
    if ( (new_repomd_checksum == old_repomd_checksum) && (!new_repomd_checksum.empty()) && (! old_repomd_checksum.empty()))
    {
      return false;
    }
  }
  return true;
}

void YUMSourceImpl::storeMetadata(const Pathname & cache_dir_r)
{
  if ( !_cache_dir.empty() )
  {
    saveMetadataTo(cache_dir_r);
  }
  else
  {
    // no previous cache, use the data read temporarely
    copyLocalMetadata(tmpMetadataDir(), cache_dir_r);
  }

  MIL << "Metadata saved in " << cache_dir_r << ". Setting as cache." << std::endl;
  _cache_dir = cache_dir_r;
 
  readRepomd();
}

void YUMSourceImpl::saveMetadataTo(const Pathname & dir_r)
{
  TmpDir download_tmp_dir;

  bool need_to_refresh = true;
  try
  {
    need_to_refresh = downloadNeeded(dir_r);
  }
  catch (Exception &e)
  {
    ZYPP_THROW(Exception("Can't check if source has changed or not. Aborting refresh."));
  }

  if ( need_to_refresh )
  {
    MIL << "YUM source '" << alias() << "' has changed since last download. Re-reading metadata into " << dir_r << endl;
  }
  else
  {
    MIL << "YUM source '" << alias() << "' has not changed. Refresh completed. SHA1 of repomd.xml file is  the same." << std::endl;
    return;
  }

  try
  {
    download_tmp_dir = downloadMetadata();
  }
  catch (Exception &e)
  {
    ZYPP_THROW(Exception("Downloading metadata failed (is YUM source?) or user did not accept remote source. Aborting refresh."));
  }

  copyLocalMetadata(download_tmp_dir, dir_r);

  // download_tmp_dir go out of scope now but it is ok as we already copied the content.
}


void YUMSourceImpl::readRepomd()
{
  _repo_primary.clear();
  _repo_files.clear();
  _repo_group.clear();
  _repo_pattern.clear();
  _repo_product.clear();
  _repo_patches.clear();
  
  parser::ParserProgress::Ptr progress;
  callback::SendReport<SourceReport> report;
  YUMSourceEventHandler npp(report);
  progress.reset( new parser::ParserProgress( npp ) );

  report->start( selfSourceRef(), "Parsing index file" );
  try
  {
    DBG << "Reading ifgz file " << repomdFile() << endl;
    ifgzstream repo_st(repomdFile().asString().c_str());
    YUMRepomdParser repomd(repo_st, "", progress);
    for (; ! repomd.atEnd(); ++repomd)
    {
      // note that we skip adding other.xml to the list of files to provide
      if ((*repomd)->type == "primary")
        _repo_primary.push_back(*repomd);
      else if ((*repomd)->type == "filelists")
        _repo_files.push_back(*repomd);
      else if ((*repomd)->type == "group")
        _repo_group.push_back(*repomd);
      else if ((*repomd)->type == "pattern")
        _repo_pattern.push_back(*repomd);
      else if ((*repomd)->type == "product")
        _repo_product.push_back(*repomd);
      else if ((*repomd)->type == "patches")
        _repo_patches.push_back(*repomd);
      else if ((*repomd)->type != "other")        // type "other" is ok, anything else not
        ERR << "Unknown type of repo file: " << (*repomd)->type << endl;
    }
    report->finish( selfSourceRef(), "Parsing index file", source::SourceReport::NO_ERROR, "" );
  }
  catch ( const Exception &  excpt_r )
  {
    ZYPP_CAUGHT( excpt_r );       // log the caught exception
    report->finish( selfSourceRef(), "Parsing index file", source::SourceReport::INVALID, "" );
    ZYPP_THROW( SourceMetadataException("Error parsing repomd.xml file") );

  }
}

std::set<zypp::Resolvable::Kind>
YUMSourceImpl::resolvableKinds() const
{
  std::set<zypp::Resolvable::Kind> kinds;
  
  if (_repo_product.size() > 0 )
    kinds.insert( ResTraits<zypp::Product>::kind ); 
  
  if (_repo_pattern.size() > 0 )
    kinds.insert( ResTraits<zypp::Pattern>::kind );
  
  if (_repo_group.size() > 0 )
    kinds.insert( ResTraits<zypp::Selection>::kind );
  
  if (_repo_primary.size() > 0 )
  kinds.insert( ResTraits<zypp::Package>::kind );
  
  if (_repo_patches.size() > 0 )
    kinds.insert( ResTraits<zypp::Patch>::kind );
  
  return kinds;
}

void YUMSourceImpl::provideProducts(Source_Ref source_r, ResStore& store)
{
  Pathname filename;
  callback::SendReport<SourceReport> report;

  try
  {
    for (std::list<YUMRepomdData_Ptr>::const_iterator it = _repo_product.begin();
         it != _repo_product.end();
         it++)
    {
      filename = metadataRoot() + (*it)->location;
      ifgzstream st ( filename.asString().c_str() );

      parser::ParserProgress::Ptr progress;
      callback::SendReport<SourceReport> report;
      YUMSourceEventHandler npp(report);
      progress.reset( new parser::ParserProgress( npp ) );

       // TranslatorExplanation %s = product file
      report->start( selfSourceRef(), str::form(_("Reading product from %s"), filename.asString().c_str()) );
      
      YUMProductParser product(st, "", progress);
      for (; !product.atEnd(); ++product)
      {
        Product::Ptr p = createProduct( source_r, **product );
        store.insert (p);
      }

      if (product.errorStatus())
      {
        // TranslatorExplanation %s = product file
        report->finish( selfSourceRef(), str::form(_("Reading product from %s"), filename.asString().c_str()), source::SourceReport::INVALID, product.errorStatus()->msg() );
        ZYPP_THROW(SourceMetadataException( "Error reading product from " + filename.asString()+ " : " + product.errorStatus()->msg()));
      }
      else
      {
        // TranslatorExplanation %s = product file
        report->finish( selfSourceRef(), str::form(_("Reading product from %s"), filename.asString().c_str()), source::SourceReport::NO_ERROR, "" );
      }
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    // TranslatorExplanation %s = product file
    report->finish( selfSourceRef(), str::form(_("Reading product from %s"), filename.asString().c_str()), source::SourceReport::INVALID, e.msg() );
    ZYPP_THROW(SourceMetadataException(e.msg()));
  }

}

void YUMSourceImpl::providePackages(Source_Ref source_r, ResStore& store)
{
  Pathname filename;
  callback::SendReport<SourceReport> report;

  // now put other and filelist data to structures for easier find
  map<NVRA, YUMFileListData_Ptr> files_data;
  map<NVRA, YUMOtherData_Ptr> other_data;

  try
  {
    for (std::list<YUMRepomdData_Ptr>::const_iterator it
         = _repo_files.begin();
         it != _repo_files.end();
         it++)
    {
      Pathname filename = metadataRoot() + (*it)->location;
      DBG << "Reading ifgz file " << filename << endl;
      ifgzstream st( filename.asString().c_str() );

      parser::ParserProgress::Ptr progress;
      YUMSourceEventHandler npp(report);
      progress.reset( new parser::ParserProgress( npp, get_stream_size(filename) ) );
      // TranslatorExplanation %s = package file list
      report->start( selfSourceRef(), str::form(_("Reading filelist from %s"), filename.asString().c_str()) );

      YUMFileListParser filelist ( st, "", progress );
      for (; ! filelist.atEnd(); ++filelist)
      {
        if (*filelist == NULL) continue;  // skip incompatible archs
        NVRA nvra( (*filelist)->name,
                   Edition( (*filelist)->ver, (*filelist)->rel, str::strtonum<int>( (*filelist)->epoch ) ),
                   Arch ( (*filelist)->arch ) );
        files_data[nvra] = *filelist;
      }

      if (filelist.errorStatus())
      {
        // TranslatorExplanation %s = package file list
        report->finish( selfSourceRef(), str::form(_("Reading filelist from %s"), filename.asString().c_str()), source::SourceReport::INVALID, filelist.errorStatus()->msg() );
        ZYPP_THROW(SourceMetadataException( "Error reading filelists from " + filename.asString()+ " : " + filelist.errorStatus()->msg()));
      }
      else
      {
        // TranslatorExplanation %s = package file list
        report->finish( selfSourceRef(), str::form(_("Reading filelist from %s"), filename.asString().c_str()), source::SourceReport::NO_ERROR, "" );
      }
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    // TranslatorExplanation %s = package file list
    report->finish( selfSourceRef(),str::form(_("Reading filelist from %s"), filename.asString().c_str()), source::SourceReport::INVALID, e.msg() );
    ZYPP_THROW(SourceMetadataException(e.msg()));
  }

  try
  {
    // now read primary data, merge them with filelist and changelog
    for (std::list<YUMRepomdData_Ptr>::const_iterator it = _repo_primary.begin(); it != _repo_primary.end(); it++)
    {
      filename = metadataRoot() + (*it)->location;
      DBG << "Reading file " << filename << endl;

      parser::ParserProgress::Ptr progress;
      YUMSourceEventHandler npp(report);
      progress.reset( new parser::ParserProgress( npp ) );
      // TranslatorExplanation %s = packages file
      report->start( selfSourceRef(), str::form(_("Reading packages from %s"), filename.asString().c_str()) );

      ifgzstream st ( filename.asString().c_str() );

      YUMPrimaryParser prim(st, "", progress);
      //YUMPrimaryParser prim(filename.asString(), "", progress);
      for (; !prim.atEnd(); ++prim)
      {
        if (*prim == NULL) continue;      // incompatible arch detected during parsing

        Arch arch;
        if (!(*prim)->arch.empty())
          arch = Arch((*prim)->arch);

        NVRA nvra( (*prim)->name,
                   Edition( (*prim)->ver, (*prim)->rel, str::strtonum<int>( (*prim)->epoch ) ),
                   arch );
        map<NVRA, YUMOtherData_Ptr>::iterator found_other = other_data.find( nvra );
        map<NVRA, YUMFileListData_Ptr>::iterator found_files = files_data.find( nvra );

        YUMFileListData filelist_empty;
        YUMOtherData other_empty;
        ResImplTraits<YUMPackageImpl>::Ptr impl;
        Package::Ptr p = createPackage( source_r, **prim, found_files != files_data.end()
                                        ? *(found_files->second)
                                        : filelist_empty,
                                        found_other != other_data.end()
                                        ? *(found_other->second)
                                        : other_empty,
                                        impl
                                      );
        ImplAndPackage iap = { impl, p };
        _package_impl[nvra] = iap;
//                MIL << "inserting package "<< p->name() << std::endl;
        store.insert (p);
      }

      if (prim.errorStatus())
      {
        // TranslatorExplanation %s = packages file
        report->finish( selfSourceRef(), str::form(_("Reading packages from %s"), filename.asString().c_str()), source::SourceReport::INVALID, prim.errorStatus()->msg() );
        ZYPP_THROW(SourceMetadataException( "Error packages from " + filename.asString()+ " : " + prim.errorStatus()->msg()));
      }
      else
      {
        // TranslatorExplanation %s = packages file
        report->finish( selfSourceRef(), str::form(_("Reading packages from %s"), filename.asString().c_str()), source::SourceReport::NO_ERROR, "" );
      }
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    // TranslatorExplanation %s = packages file
    report->finish( selfSourceRef(), str::form(_("Reading packages from %s"), filename.asString().c_str()), source::SourceReport::INVALID, e.msg() );
    ZYPP_THROW(SourceMetadataException(e.msg()));
  }

}

void YUMSourceImpl::provideSelections(Source_Ref source_r, ResStore& store)
{
  callback::SendReport<SourceReport> report;
  Pathname filename;
  try
  {
    for (std::list<YUMRepomdData_Ptr>::const_iterator it = _repo_group.begin();
         it != _repo_group.end();
         it++)
    {
      Pathname filename = metadataRoot() + (*it)->location;
      DBG << "Reading file " << filename << endl;
      ifgzstream st ( filename.asString().c_str() );

      parser::ParserProgress::Ptr progress;
      YUMSourceEventHandler npp(report);
      progress.reset( new parser::ParserProgress( npp ) );
      // TranslatorExplanation %s = selection metadata file
      report->start( selfSourceRef(), str::form(_("Reading selection from %s"), filename.asString().c_str()) );

      YUMGroupParser group(st, "", progress);
      for (; !group.atEnd(); ++group)
      {
        Selection::Ptr p = createGroup( source_r, **group );
        store.insert (p);
      }

      if (group.errorStatus())
      {
        // TranslatorExplanation %s = selection metadata file
        report->finish( selfSourceRef(), str::form(_("Reading selection from %s"), filename.asString().c_str()), source::SourceReport::INVALID, group.errorStatus()->msg() );
        ZYPP_THROW(SourceMetadataException( "Error Parsing selection " + filename.asString()+ " : " + group.errorStatus()->msg()));
      }
      else
      {
        // TranslatorExplanation %s = selection metadata file
        report->finish( selfSourceRef(), str::form(_("Reading selection from %s"), filename.asString().c_str()), source::SourceReport::NO_ERROR, "" );
      }
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    // TranslatorExplanation %s = selection metadata file
    report->finish( selfSourceRef(), str::form(_("Reading selection from %s"), filename.asString().c_str()), source::SourceReport::INVALID, e.msg() );
    ZYPP_THROW(SourceMetadataException(e.msg()));
  }

}

void YUMSourceImpl::providePatterns(Source_Ref source_r, ResStore& store)
{
  callback::SendReport<SourceReport> report;
  Pathname filename;
  try
  {
    for (std::list<YUMRepomdData_Ptr>::const_iterator it = _repo_pattern.begin();
         it != _repo_pattern.end(); it++)
    {
      Pathname filename = metadataRoot() + (*it)->location;

      DBG << "Reading file " << filename << endl;
      ifgzstream st ( filename.asString().c_str() );

      parser::ParserProgress::Ptr progress;
      YUMSourceEventHandler npp(report);
      progress.reset( new parser::ParserProgress( npp )  );
      // TranslatorExplanation %s = pattern metadata file
      report->start( selfSourceRef(), str::form(_("Reading pattern from %s"), filename.asString().c_str()) );

      YUMPatternParser pattern(st, "", progress);
      for (; !pattern.atEnd(); ++pattern)
      {
        Pattern::Ptr p = createPattern( source_r, **pattern );
        store.insert (p);
      }

      if (pattern.errorStatus())
      {
        // TranslatorExplanation %s = pattern metadata file
        report->finish( selfSourceRef(), str::form(_("Reading pattern from %s"), filename.asString().c_str()), source::SourceReport::INVALID, pattern.errorStatus()->msg() );
        ZYPP_THROW(SourceMetadataException( "Error parsing pattern" + filename.asString()+ " : " + pattern.errorStatus()->msg()));
      }
      else
      {
        // TranslatorExplanation %s = pattern metadata file
        report->finish( selfSourceRef(), str::form(_("Reading pattern from %s"), filename.asString().c_str()), source::SourceReport::NO_ERROR, "" );
      }
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    // TranslatorExplanation %s = pattern metadata file
    report->finish( selfSourceRef(), str::form(_("Reading pattern from %s"), filename.asString().c_str()), source::SourceReport::INVALID, e.msg() );
    ZYPP_THROW(SourceMetadataException(e.msg()));
  }

}

void YUMSourceImpl::providePatches(Source_Ref source_r, ResStore& store)
{
  std::list<std::string> patch_files;
  callback::SendReport<SourceReport> report;
  Pathname filename;

  try
  {
    for (std::list<YUMRepomdData_Ptr>::const_iterator it = _repo_patches.begin();
         it != _repo_patches.end();
         it++)
    {
      filename = metadataRoot() + (*it)->location;

      DBG << "Reading file " << filename << endl;
      ifgzstream st ( filename.asString().c_str() );

      parser::ParserProgress::Ptr progress;
      YUMSourceEventHandler npp(report);
      progress.reset( new parser::ParserProgress( npp ) );
      // TranslatorExplanation %s = patches index file
      report->start( selfSourceRef(), str::form(_("Reading patches index %s"), filename.asString().c_str()) );
      YUMPatchesParser patch(st, "", progress);

      for (; !patch.atEnd(); ++patch)
      {
        string filename = (*patch)->location;
        patch_files.push_back(filename);
      }

      if (patch.errorStatus())
      {
        // TranslatorExplanation %s = patches index file
        report->finish( selfSourceRef(), str::form(_("Reading patches index %s"), filename.asString().c_str()), source::SourceReport::INVALID, patch.errorStatus()->msg() );
        ZYPP_THROW(SourceMetadataException( "Error Parsing patch " + filename.asString()+ " : " + patch.errorStatus()->msg()));
      }
      else
      {
        // TranslatorExplanation %s = patches index file
        report->finish( selfSourceRef(), str::form(_("Reading patches index %s"), filename.asString().c_str()), source::SourceReport::NO_ERROR, "" );
      }
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    // TranslatorExplanation %s = patches index file
    report->finish( selfSourceRef(), str::form(_("Reading patches index %s"), filename.asString().c_str()), source::SourceReport::INVALID, e.msg() );
    ZYPP_THROW(SourceMetadataException(e.msg()));
  }

  try
  {
    //---------------------------------
    // now the individual patch files
    for (std::list<std::string>::const_iterator it = patch_files.begin();
         it != patch_files.end();
         it++)
    {
      filename = metadataRoot() + *it;
      DBG << "Reading file " << filename << endl;

      //FIXME error handling
      ifgzstream st ( filename.asString().c_str() );

      parser::ParserProgress::Ptr progress;
      YUMSourceEventHandler npp(report);
      progress.reset( new parser::ParserProgress( npp ) );

      // TranslatorExplanation %s = patch metadata file
      report->start( selfSourceRef(), str::form(_("Reading patch %s"), filename.asString().c_str()) );

      YUMPatchParser ptch(st, "", progress);
      for (; !ptch.atEnd(); ++ptch)
      {
        Patch::Ptr p = createPatch( source_r, **ptch );
        store.insert (p);
        Patch::AtomList atoms = p->atoms();
        for (Patch::AtomList::iterator at = atoms.begin(); at != atoms.end(); at++)
        {
          _store.insert (*at);
        }
      }

      if (ptch.errorStatus())
      {
        // TranslatorExplanation %s = patch metadata file
        report->finish( selfSourceRef(), str::form(_("Reading patch %s"), filename.asString().c_str()), source::SourceReport::INVALID, ptch.errorStatus()->msg() );
        ZYPP_THROW(SourceMetadataException( "Error Parsing patch " + filename.asString()+ " : " + ptch.errorStatus()->msg()));
      }
      else
      {
        // TranslatorExplanation %s = patch metadata file
        report->finish( selfSourceRef(), str::form(_("Reading patch %s"), filename.asString().c_str()), source::SourceReport::NO_ERROR, "" );
      }
    }
  }
  catch ( const Exception &e )
  {
    ERR << "Cannot read patch metadata" << endl;
    ZYPP_CAUGHT(e);
    // TranslatorExplanation %s = patch metadata file
    report->finish( selfSourceRef(), str::form(_("Reading patch %s"), filename.asString().c_str()), source::SourceReport::INVALID, e.msg() );
    ZYPP_THROW(SourceMetadataException(e.msg()));
  }
}

ResStore YUMSourceImpl::provideResolvablesByKind(Source_Ref source_r, zypp::Resolvable::Kind kind)
{
  ResStore store;

  //readRepomd();

  if ( kind == ResTraits<Product>::kind )
    provideProducts ( selfSourceRef(), store );
  else if ( kind == ResTraits<Package>::kind )
    providePackages (selfSourceRef(), store );
  else if ( kind == ResTraits<Selection>::kind )
    provideSelections ( selfSourceRef(), store );
  else if ( kind == ResTraits<Pattern>::kind )
    providePatterns ( selfSourceRef(), store );
  else if ( kind == ResTraits<Pattern>::kind )
    providePatches ( selfSourceRef(), store );

  return store;
}

void YUMSourceImpl::createResolvables(Source_Ref source_r)
{

  //readRepomd();
  provideProducts(selfSourceRef(), _store);
  providePackages(selfSourceRef(), _store);
  provideSelections(selfSourceRef(), _store);
  providePatterns(selfSourceRef(), _store);
  providePatches(selfSourceRef(), _store);

}


Package::Ptr YUMSourceImpl::createPackage(
  Source_Ref source_r,
  const zypp::parser::yum::YUMPrimaryData & parsed,
  const zypp::parser::yum::YUMFileListData & filelist,
  const zypp::parser::yum::YUMOtherData & other,
  ResImplTraits<YUMPackageImpl>::Ptr & impl
)
{
  try
  {
    impl = new YUMPackageImpl( source_r, parsed, filelist, other );

    Dependencies deps( createDependencies( parsed, ResTraits<Package>::kind ) );

    CapFactory f;

    for (std::list<FileData>::const_iterator it = filelist.files.begin();
         it != filelist.files.end();
         it++)
    {
      deps[Dep::PROVIDES].insert( f.parse( ResTraits<Package>::kind, it->name ) );
    }

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name,
                       Edition( parsed.ver, parsed.rel, parsed.epoch ),
                       arch,
                       deps
                     );
    Package::Ptr package = detail::makeResolvableFromImpl(
                             dataCollect, impl
                           );
    return package;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create package object"));
  }
  return 0L;
}

Atom::Ptr YUMSourceImpl::augmentPackage(
  Source_Ref source_r,
  const zypp::parser::yum::YUMPatchPackage & parsed
)
{
  try
  {
    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch( parsed.arch );

    Edition edition( parsed.ver, parsed.rel, parsed.epoch );
    NVRA nvra( parsed.name,
               edition,
               arch );

    DBG << "augmentPackage(" << nvra << ")" << endl;

    // create Atom
    CapFactory f;
    Dependencies deps = createDependencies( parsed, ResTraits<Package>::kind );
//        deps[Dep::REQUIRES].insert( f.parse( ResTraits<Package>::kind, parsed.name, Rel::EQ, edition ) );
    NVRAD atomdata( nvra, deps );
    ResImplTraits<YUMAtomImpl>::Ptr atomimpl = new YUMAtomImpl( source_r );
    Atom::Ptr atom = detail::makeResolvableFromImpl( atomdata, atomimpl );

    //source_r
    PackageImplMapT::const_iterator it = _package_impl.find( nvra );
    if (it == _package_impl.end())
    {
      WAR << "Patch augments non-existant package " << nvra << endl;
    }
    else
    {
      ResImplTraits<YUMPackageImpl>::Ptr impl = it->second.impl;

      if (!parsed.location.empty())
      {
        impl->_location = parsed.location;
        impl->_mediaNumber = str::strtonum<unsigned>( parsed.media );
        impl->_checksum = CheckSum(parsed.checksumType, parsed.checksum);
      }
      impl->_install_only = parsed.installOnly;

      //DBG << "Inserting patch RPMs" << endl;
      impl->_patch_rpms = std::list<YUMPackageImpl::PatchRpm>();
      for ( std::list<YUMPatchRpm>::const_iterator it = parsed.patchRpms.begin();
            it != parsed.patchRpms.end(); ++it )
      {
        YUMPackageImpl::PatchRpm patch_rpm;

        patch_rpm.location( source::OnMediaLocation()
                            .medianr( str::strtonum<unsigned>( it->media ) )
                            .filename( it->location )
                            .checksum( CheckSum( it->checksumType, it->checksum ) )
                            .downloadsize( str::strtonum<ByteCount::SizeType>( it->downloadsize ) ) );

        for ( std::list<YUMPatchBaseVersion>::const_iterator bvit = it->baseVersions.begin();
              bvit != it->baseVersions.end(); ++bvit )
        {
          patch_rpm.baseversion( Edition( bvit->edition.ver,
                                          bvit->edition.rel,
                                          bvit->edition.epoch ) );
        }

        patch_rpm.buildtime( str::strtonum<Date::ValueType>( it->buildtime ) );

        impl->_patch_rpms.push_back( patch_rpm );
      }

      //DBG << "Inserting delta RPMs" << endl;
      impl->_delta_rpms = std::list<YUMPackageImpl::DeltaRpm>();
      for ( std::list<YUMDeltaRpm>::const_iterator it = parsed.deltaRpms.begin();
            it != parsed.deltaRpms.end(); ++it )
      {
        YUMPackageImpl::DeltaRpm delta_rpm;

        delta_rpm.location( source::OnMediaLocation()
                            .medianr( str::strtonum<unsigned>( it->media ) )
                            .filename( it->location )
                            .checksum( CheckSum( it->checksumType, it->checksum ) )
                            .downloadsize( str::strtonum<ByteCount::SizeType>( it->downloadsize ) ) );

        const YUMDeltaBaseVersion & ybv( it->baseVersion );
        delta_rpm.baseversion( YUMPackageImpl::DeltaRpm::BaseVersion()
                               .edition( Edition( ybv.edition.ver,
                                                  ybv.edition.rel,
                                                  ybv.edition.epoch ) )
                               .buildtime( str::strtonum<Date::ValueType>( ybv.buildtime ) )
                               .checksum( CheckSum::md5( ybv.md5sum ) )
                               .sequenceinfo( ybv.sequence_info )
                             );

        delta_rpm.buildtime( str::strtonum<Date::ValueType>( it->buildtime ) );

        impl->_delta_rpms.push_back( delta_rpm );
      }
    }
    return atom;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create augmented package object"));
  }
  return 0L;
}

Selection::Ptr YUMSourceImpl::createGroup(
  Source_Ref source_r,
  const zypp::parser::yum::YUMGroupData & parsed
)
{
  try
  {
    ResImplTraits<YUMGroupImpl>::Ptr impl(new YUMGroupImpl(source_r, parsed));
    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.groupId,
                       Edition::noedition,			// group has just a name,
                       Arch_noarch,				//   pattern has edition & arch
                       createGroupDependencies(parsed));
    Selection::Ptr group = detail::makeResolvableFromImpl(
                             dataCollect, impl
                           );
    return group;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create package group object"));
  }
  return 0L;
}

Pattern::Ptr YUMSourceImpl::createPattern(
  Source_Ref source_r,
  const zypp::parser::yum::YUMPatternData & parsed
)
{
  try
  {
    ResImplTraits<YUMPatternImpl>::Ptr impl(new YUMPatternImpl(source_r, parsed));
    // Collect basic Resolvable data
    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    NVRAD dataCollect( parsed.name,
                       Edition( parsed.ver, parsed.rel, parsed.epoch ),
                       arch,
                       createDependencies(parsed, ResTraits<Pattern>::kind));
    Pattern::Ptr pattern = detail::makeResolvableFromImpl(
                             dataCollect, impl
                           );
    return pattern;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create installation pattern object"));
  }
  return 0L;
}

Message::Ptr YUMSourceImpl::createMessage(
  Source_Ref source_r,
  const zypp::parser::yum::YUMPatchMessage & parsed,
  Patch::constPtr patch
)
{
  try
  {
    ResImplTraits<YUMMessageImpl>::Ptr impl(new YUMMessageImpl(source_r, parsed, patch));
    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);
    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name,
                       Edition( parsed.ver, parsed.rel, parsed.epoch ),
                       arch,
                       createDependencies(parsed,
                                          ResTraits<Message>::kind)
                     );
    Message::Ptr message = detail::makeResolvableFromImpl(
                             dataCollect, impl
                           );
    return message;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create message object"));
  }
  return 0L;
}

Script::Ptr YUMSourceImpl::createScript(
  Source_Ref source_r,
  const zypp::parser::yum::YUMPatchScript & parsed
)
{
  try
  {
    ResImplTraits<YUMScriptImpl>::Ptr impl(new YUMScriptImpl(source_r, parsed));
    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);
    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name,
                       Edition( parsed.ver, parsed.rel, parsed.epoch ),
                       arch,
                       createDependencies(parsed,
                                          ResTraits<Script>::kind)
                     );
    Script::Ptr script = detail::makeResolvableFromImpl(
                           dataCollect, impl
                         );
    return script;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create script object"));
  }
  return 0L;
}

Product::Ptr YUMSourceImpl::createProduct(
  Source_Ref source_r,
  const zypp::parser::yum::YUMProductData & parsed
)
{
  try
  {
    ResImplTraits<YUMProductImpl>::Ptr impl(new YUMProductImpl(source_r, parsed));

    // Collect basic Resolvable data
    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);
    NVRAD dataCollect( parsed.name,
                       Edition( parsed.ver, parsed.rel, parsed.epoch ),
                       arch,
                       createDependencies(parsed,
                                          ResTraits<Product>::kind)
                     );
    Product::Ptr product = detail::makeResolvableFromImpl(
                             dataCollect, impl
                           );
    return product;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create product object"));
  }
  return 0L;
}

Patch::Ptr YUMSourceImpl::createPatch(
  Source_Ref source_r,
  const zypp::parser::yum::YUMPatchData & parsed
)
{
  try
  {
    ResImplTraits<YUMPatchImpl>::Ptr impl(new YUMPatchImpl(source_r, parsed, *this));

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name,
                       Edition( parsed.ver, parsed.rel, parsed.epoch ),
                       arch,
                       createDependencies( parsed,
                                           ResTraits<Patch>::kind)
                     );
    Patch::Ptr patch = detail::makeResolvableFromImpl(
                         dataCollect, impl
                       );
    // now process the atoms
    CapFactory _f;
    Capability cap( _f.parse(
                      Patch::TraitsType::kind,
                      parsed.name,
                      Rel::EQ,
                      Edition(parsed.ver, parsed.rel, parsed.epoch)
                    ));

    // maps name to parser data in order to find 'best' architectureC
    typedef std::map<std::string, shared_ptr<YUMPatchPackage> > PkgAtomsMap;
    PkgAtomsMap pkg_atoms;

    for (std::list<shared_ptr<YUMPatchAtom> >::const_iterator it
         = parsed.atoms.begin();
         it != parsed.atoms.end();
         it++)
    {
      switch ((*it)->atomType())
      {
        // for packages, try to find best architecture for name-version-release first (#168840)
        // we can't use the name alone as there might be different editions for the same name
        // with different architecture.
        // So we only choose the best architecture if name-version-edition matches (#170098)

      case YUMPatchAtom::Package:
        {
          shared_ptr<YUMPatchPackage> package_data
          = dynamic_pointer_cast<YUMPatchPackage>(*it);
          string atomkey( package_data->name + "-" + package_data->epoch + ":" + package_data->ver + "-" + package_data->rel );

          // check if atomkey is already known
          PkgAtomsMap::iterator pa_pos = pkg_atoms.find( atomkey );
          if (pa_pos != pkg_atoms.end())
          {
            try
            {
              Arch oldarch, newarch;
              if (!(pa_pos->second->arch.empty())) oldarch = Arch( pa_pos->second->arch );
              if (!(package_data->arch.empty())) newarch = Arch( package_data->arch );
              if (newarch.compatibleWith( getZYpp()->architecture() ) )
              {			// new one is compatible (if not, we don't care)

                if (!oldarch.compatibleWith( getZYpp()->architecture() )			// old one is not compatible
                    || oldarch.compare( newarch ) < 0)					//  or compatible but worse
                {
                  pa_pos->second = package_data;				// new one is it !
                }
              }
            }
            catch ( const Exception & excpt_r )
            {
              ZYPP_CAUGHT( excpt_r );
              ERR << "Package " << package_data->name << " in patch's atomlist has bad architecture '" << package_data->arch << "'" << endl;
            }
          }
          else
          {
            pkg_atoms[atomkey] = package_data;					// first occurence of this atomkey
          }
          break;
        }
      case YUMPatchAtom::Message:
        {
          shared_ptr<YUMPatchMessage> message_data
          = dynamic_pointer_cast<YUMPatchMessage>(*it);
          Message::Ptr message = createMessage(source_r, *message_data, patch);
          impl->_atoms.push_back(message);
          break;
        }
      case YUMPatchAtom::Script:
        {
          shared_ptr<YUMPatchScript> script_data
          = dynamic_pointer_cast<YUMPatchScript>(*it);
          Script::Ptr script = createScript(source_r, *script_data);
          impl->_atoms.push_back(script);
          break;
        }
      default:
        ERR << "Unknown type of atom" << endl;
      }
#if 0					// atoms require their patch, why ?
      for (Patch::AtomList::iterator it = impl->_atoms.begin();
           it != impl->_atoms.end();
           it++)
      {
        (*it)->injectRequires(cap);
      }
#endif
    }

    for (PkgAtomsMap::const_iterator pa_pos = pkg_atoms.begin(); pa_pos != pkg_atoms.end(); ++pa_pos)
    {
      Atom::Ptr atom = augmentPackage( source_r, *(pa_pos->second) );
      impl->_atoms.push_back(atom);
    }

    return patch;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create patch object"));
  }
  return 0L;
}

Dependencies YUMSourceImpl::createDependencies(
  const zypp::parser::yum::YUMObjectData & parsed,
  const Resolvable::Kind my_kind
)
{
  Dependencies _deps;
  for (std::list<YUMDependency>::const_iterator it = parsed.provides.begin();
       it != parsed.provides.end();
       it++)
  {
    _deps[Dep::PROVIDES].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.conflicts.begin();
       it != parsed.conflicts.end();
       it++)
  {
    _deps[Dep::CONFLICTS].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.obsoletes.begin();
       it != parsed.obsoletes.end();
       it++)
  {
    _deps[Dep::OBSOLETES].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.freshens.begin();
       it != parsed.freshens.end();
       it++)
  {
    _deps[Dep::FRESHENS].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.recommends.begin();
       it != parsed.recommends.end();
       it++)
  {
    _deps[Dep::RECOMMENDS].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.suggests.begin();
       it != parsed.suggests.end();
       it++)
  {
    _deps[Dep::SUGGESTS].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.supplements.begin();
       it != parsed.supplements.end();
       it++)
  {
    _deps[Dep::SUPPLEMENTS].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.enhances.begin();
       it != parsed.enhances.end();
       it++)
  {
    _deps[Dep::ENHANCES].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.prerequires.begin();
       it != parsed.prerequires.end();
       it++)
  {
    _deps[Dep::PREREQUIRES].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.requires.begin();
       it != parsed.requires.end();
       it++)
  {
    if (it->pre == "1")
      _deps[Dep::PREREQUIRES].insert(createCapability(*it, my_kind));
    else
      _deps[Dep::REQUIRES].insert(createCapability(*it, my_kind));
  }

  return _deps;
}

Dependencies YUMSourceImpl::createGroupDependencies(
  const zypp::parser::yum::YUMGroupData & parsed
)
{
  Dependencies _deps;

  for (std::list<PackageReq>::const_iterator it = parsed.packageList.begin();
       it != parsed.packageList.end();
       it++)
  {
    Dep _dep_kind = Dep::REQUIRES;
    if (it->type == "mandatory" || it->type == "")
    {
      _dep_kind = Dep::REQUIRES;
    }
    else if (it->type == "default")
    {
      _dep_kind = Dep::RECOMMENDS;
    }
    else if (it->type == "optional")
    {
      _dep_kind = Dep::SUGGESTS;
    }
    _deps[_dep_kind].insert(createCapability(YUMDependency(
                              "",
                              it->name,
                              "EQ",
                              it->epoch,
                              it->ver,
                              it->rel,
                              ""
                            ),
                            ResTraits<Package>::kind));
  }
  for (std::list<MetaPkg>::const_iterator it = parsed.grouplist.begin();
       it != parsed.grouplist.end();
       it++)
  {
    Dep _dep_kind = Dep::REQUIRES;
    if (it->type == "mandatory" || it->type == "")
    {
      _dep_kind = Dep::REQUIRES;
    }
    else if (it->type == "default")
    {
      _dep_kind = Dep::RECOMMENDS;
    }
    else if (it->type == "optional")
    {
      _dep_kind = Dep::SUGGESTS;
    }
    _deps[_dep_kind].insert(createCapability(YUMDependency(
                              "",
                              it->name,
                              "",
                              "",
                              "",
                              "",
                              ""
                            ),
                            ResTraits<Selection>::kind));
  }
  return _deps;
}

Capability YUMSourceImpl::createCapability(const YUMDependency & dep,
    const Resolvable::Kind & my_kind)
{
  CapFactory _f;
  Resolvable::Kind _kind = dep.kind == "" ? my_kind : Resolvable::Kind(dep.kind);
  Capability cap;
  if ( ! dep.isEncoded() )
  {
    cap = _f.parse(
            _kind,
            dep.name,
            Rel(dep.flags),
            Edition(dep.ver, dep.rel, dep.epoch)
          );
  }
  else
  {
    cap = _f.parse( _kind, dep.encoded );
  }
  return cap;
}

} // namespace yum
/////////////////////////////////////////////////////////////////
} // namespace source
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
