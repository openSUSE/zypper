/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include <map>

#include "zypp/base/Gettext.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"
#include "zypp/repo/cached/RepoImpl.h"
#include "zypp/cache/ResolvableQuery.h"
#include "zypp/cache/CacheAttributes.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"

using namespace zypp::cache;
using namespace std;
using namespace sqlite3x;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////
namespace cached
{ /////////////////////////////////////////////////////////////////

RepoImpl::RepoImpl( const RepoOptions &opts )
  : RepositoryImpl(opts.repoinfo)
  , _type_cache(opts.dbdir)
  , _rquery(opts.dbdir)
  , _options(opts)
{
}

RepoImpl::~RepoImpl()
{
  MIL << "Destroying repo '" << info().alias() << "'" << endl;
}

void RepoImpl::createResolvables()
{
#warning IMPLEMENT REPOIMPL::CREATERESOLVABLES
#if 0
  ProgressData ticks;
  ticks.sendTo(_options.readingResolvablesProgress);
  ticks.name(str::form(_( "Reading '%s' repository cache"), info().alias().c_str()));
  CombinedProgressData subprogrcv(ticks);

  debug::Measure m("create resolvables");
  CapFactory capfactory;
  try
  {
    sqlite3_connection con((_options.dbdir + "zypp.db").asString().c_str());
    con.setprogresshandler(subprogrcv, 100);

    con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");

//     We could use this to calculate total resolvables
//     sqlite3_command total_cmd("select count(id) from resolvables where repository_id=:repository_id;");
//     total_cmd.bind(":repository_id", _repository_id);
//     int total = total_cmd.executeint();

    sqlite3_command cmd( con, "select id,name,version,release,epoch,arch,kind from resolvables where repository_id=:repository_id;");
    cmd.bind(":repository_id", _options.repository_id);
    map<data::RecordId, pair<Resolvable::Kind, NVRAD> > nvras;

    sqlite3_reader reader = cmd.executereader();

    while(reader.read())
    {
      long long id = reader.getint64(0);
      Dependencies deps;
      Resolvable::Kind kind = _type_cache.kindFor(reader.getint(6));
      // Collect basic Resolvable data
      nvras[id] = make_pair( kind, NVRAD( reader.getstring(1),
                                          Edition( reader.getstring(2), reader.getstring(3), reader.getint(4) ),
                                          _type_cache.archFor(reader.getint(5)),
                                           deps ) );
    }

    MIL << "Done reading resolvables nvra" << endl;

    ticks.tick();

    read_capabilities( con, _options.repository_id, nvras, ticks );

    ticks.tick();

    for ( map<data::RecordId, pair<Resolvable::Kind, NVRAD> >::const_iterator it = nvras.begin(); it != nvras.end(); ++it )
    {
      if ( it->second.first == ResTraits<Package>::kind )
      {
        ResImplTraits<cached::PackageImpl>::Ptr impl = new cached::PackageImpl(it->first, this);
        Package::Ptr package = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert( package );
      }
      else if ( it->second.first == ResTraits<SrcPackage>::kind )
      {
        ResImplTraits<cached::SrcPackageImpl>::Ptr impl = new cached::SrcPackageImpl(it->first, this);
        SrcPackage::Ptr srcpackage = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert( srcpackage );
      }
      else if ( it->second.first == ResTraits<Product>::kind )
      {
        ResImplTraits<cached::ProductImpl>::Ptr impl = new cached::ProductImpl(it->first, this);
        Product::Ptr product = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert( product );
      }
      else if ( it->second.first == ResTraits<Pattern>::kind )
      {
        ResImplTraits<cached::PatternImpl>::Ptr impl = new cached::PatternImpl(it->first, this);
        Pattern::Ptr pattern = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert( pattern );
      }
      else if ( it->second.first == ResTraits<Patch>::kind )
      {
        ResImplTraits<cached::PatchImpl>::Ptr impl = new cached::PatchImpl(it->first, this);
        Patch::Ptr patch = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert( patch );
      }
      else if ( it->second.first == ResTraits<Message>::kind )
      {
        ResImplTraits<cached::MessageImpl>::Ptr impl = new cached::MessageImpl(it->first, this);
        Message::Ptr message = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert( message );
      }
      else if ( it->second.first == ResTraits<Script>::kind )
      {
        ResImplTraits<cached::ScriptImpl>::Ptr impl = new cached::ScriptImpl(it->first, this);
        Script::Ptr script = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert( script );
      }
      else if ( it->second.first == ResTraits<Atom>::kind )
      {
        ResImplTraits<cached::AtomImpl>::Ptr impl = new cached::AtomImpl(it->first, this);
        Atom::Ptr atom = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert( atom );
      }
    }
    con.executenonquery("COMMIT;");
    con.resetprogresshandler();
  }
  catch(exception &ex) {
      cerr << "Exception Occured: " << ex.what() << endl;
   }
  //extract_packages_from_directory( _store, thePath, selfRepositoryRef(), true );
#endif
}

void RepoImpl::createPatchAndDeltas()
{
  ProgressData ticks;
  ticks.sendTo(_options.readingPatchDeltasProgress );
  ticks.name(str::form(_( "Reading patch and delta rpms from '%s' repository cache"), info().alias().c_str()));
  CombinedProgressData subprogrcv(ticks);
  try
  {
    sqlite3_connection con((_options.dbdir + "zypp.db").asString().c_str());
    con.setprogresshandler(subprogrcv);
    con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");

    // Refer to the enum when retrieving the data from reader.

    enum pp_query_val {    pp_id, pp_name, pp_version, pp_release, pp_epoch, pp_arch, pp_media_nr, pp_location, pp_checksum, pp_checksum_type, pp_download_size, pp_build_time };
    string pp_query = "SELECT id,    name,    version,    release,    epoch,    arch,    media_nr,    location,    checksum,    checksum_type,    download_size,    build_time "

                      "FROM patch_packages WHERE repository_id=:repository_id;";

    enum pp_bv_query_val { pp_bv_version, pp_bv_release, pp_bv_epoch };
    string pp_bv_query = "SELECT version,       release,       epoch "

                         "FROM patch_packages_baseversions WHERE patch_package_id = :patch_package_id";

    enum dp_query_val {    dp_id, dp_name, dp_version, dp_release, dp_epoch, dp_arch, dp_media_nr, dp_location, dp_checksum, dp_checksum_type, dp_download_size, dp_build_time,  dp_baseversion_version, dp_baseversion_release, dp_baseversion_epoch, dp_baseversion_checksum, dp_baseversion_checksum_type, dp_baseversion_build_time, dp_baseversion_sequence_info };
    string dp_query = "SELECT id,    name,    version,    release,    epoch,    arch,    media_nr,    location,    checksum,    checksum_type,    download_size,    build_time,     baseversion_version,    baseversion_release,    baseversion_epoch,    baseversion_checksum,    baseversion_checksum_type,    baseversion_build_time,    baseversion_sequence_info "

                      "FROM delta_packages WHERE repository_id=:repository_id;";

    {
      // bind the master repo id to the query
      sqlite3_command deltas_cmd( con, dp_query);
      deltas_cmd.bind(":repository_id", _options.repository_id);
      sqlite3_reader reader = deltas_cmd.executereader();
      while ( reader.read() )
      {
        zypp::OnMediaLocation on_media( reader.getstring(dp_location), reader.getint(dp_media_nr) );

        CheckSum checksum(reader.getstring(dp_checksum_type), reader.getstring(dp_checksum));
        if ( checksum.empty() )
        {
          ERR << "Wrong checksum for delta, skipping..." << endl;
          continue;
        }
        on_media.setChecksum(checksum);
        on_media.setDownloadSize(reader.getint(dp_download_size));

        packagedelta::DeltaRpm::BaseVersion baseversion;
        baseversion.setEdition( Edition(reader.getstring(dp_baseversion_version), reader.getstring(dp_baseversion_release), reader.getstring(dp_baseversion_epoch) ) );

        checksum = CheckSum(reader.getstring(dp_baseversion_checksum_type), reader.getstring(dp_baseversion_checksum));
        if ( checksum.empty() )
        {
          ERR << "Wrong checksum for delta, skipping..." << endl;
          continue;
        }
        baseversion.setChecksum(checksum);
        baseversion.setBuildtime(reader.getint(dp_baseversion_build_time));
        baseversion.setSequenceinfo(reader.getstring(dp_baseversion_sequence_info));

        zypp::packagedelta::DeltaRpm delta;
        delta.setName   ( reader.getstring(dp_name) );
        delta.setEdition( Edition( reader.getstring(dp_version), reader.getstring(dp_release), reader.getint(dp_epoch) ) );
        delta.setArch   ( _type_cache.archFor( reader.getint(dp_arch) ) );
        delta.setLocation( on_media );
        delta.setBaseversion( baseversion );
        delta.setBuildtime(reader.getint(dp_build_time));

        _deltaRpms.push_back(delta);
      }
      reader.close();
    }
    {
      // patch rpms
      // bind the master package id to the query
      // bind the master repo id to the query
      sqlite3_command pp_cmd( con, pp_query);
      sqlite3_command pp_bv_cmd( con, pp_bv_query);
      pp_cmd.bind(":repository_id", _options.repository_id);
      sqlite3_reader reader = pp_cmd.executereader();

      while ( reader.read() )
      {
        //MIL << "Addining patch rpm " << endl;
        long long patch_package_id = reader.getint64(pp_id);

        zypp::OnMediaLocation on_media( reader.getstring(pp_location), reader.getint(pp_media_nr) );

        CheckSum checksum(reader.getstring(pp_checksum_type), reader.getstring(pp_checksum));
        if ( checksum.empty() )
        {
          ERR << "Wrong checksum for delta, skipping..." << endl;
          continue;
        }
        on_media.setChecksum(checksum);
        on_media.setDownloadSize(reader.getint(pp_download_size));

        zypp::packagedelta::PatchRpm patch;
        patch.setName   ( reader.getstring(pp_name) );
        patch.setEdition( Edition( reader.getstring(pp_version), reader.getstring(pp_release), reader.getint(pp_epoch) ) );
        patch.setArch   ( _type_cache.archFor( reader.getint(pp_arch) ) );
        patch.setLocation( on_media );
        patch.setBuildtime(reader.getint(pp_build_time));

        pp_bv_cmd.bind( ":patch_package_id", patch_package_id );

        sqlite3_reader bv_reader = pp_bv_cmd.executereader();
        while (bv_reader.read())
        {
          //MIL << "  * Adding baseversion " << endl;
          packagedelta::PatchRpm::BaseVersion baseversion = packagedelta::PatchRpm::BaseVersion( bv_reader.getstring(pp_bv_version) ,
                                                                                                 bv_reader.getstring(pp_bv_release),
                                                                                                 bv_reader.getint(pp_bv_epoch) );
          patch.addBaseversion(baseversion);
        }

        bv_reader.close();

        _patchRpms.push_back(patch);
      }
      reader.close();
      MIL << _patchRpms.size() << " patch rpms read." << endl;
    }
    //con.resetprogresshandler();
    con.close();
  }
  catch(exception &ex) {
      cerr << "Exception Occured: " << ex.what() << endl;
  }
}

ResolvableQuery RepoImpl::resolvableQuery()
{
  return _rquery;
}


/////////////////////////////////////////////////////////////////
} // namespace cached
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace repo
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

