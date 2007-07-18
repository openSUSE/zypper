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
#include "zypp/capability/Capabilities.h"
#include "zypp/cache/ResolvableQuery.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/CapFactory.h"

#include "zypp/Package.h"
#include "zypp/SrcPackage.h"
#include "zypp/Product.h"
#include "zypp/Pattern.h"
#include "zypp/Patch.h"
#include "zypp/Message.h"
#include "zypp/Script.h"
#include "zypp/Atom.h"

#include "zypp/repo/cached/RepoImpl.h"
#include "zypp/repo/cached/PackageImpl.h"
#include "zypp/repo/cached/SrcPackageImpl.h"
#include "zypp/repo/cached/ProductImpl.h"
#include "zypp/repo/cached/PatternImpl.h"
#include "zypp/repo/cached/PatchImpl.h"
#include "zypp/repo/cached/MessageImpl.h"
#include "zypp/repo/cached/ScriptImpl.h"
#include "zypp/repo/cached/AtomImpl.h"
#include "zypp/cache/CacheAttributes.h"

using namespace zypp::detail;
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

static int global_progress_handler(void* ptr)
{
  MIL << "BOOOOOOOOOH" << std::endl;
  //RepoImpl *r = dynamic_cast<RepoImpl *>(ptr);
  RepoImpl *r = (RepoImpl *)(ptr);
  if ( r )
    return r->progress_handler(ptr);

  return 1;
}

void read_capabilities( sqlite3_connection &con,
                        map<data::RecordId, NVRAD> &nvras,
                        ProgressData &progress );


int RepoImpl::progress_handler(void* ptr)
{
  if ( _ticks.tick() )
    return 1;
  return 1;
}

void RepoImpl::createResolvables()
{
  _ticks = ProgressData();
  _ticks.sendTo(_options.readingResolvablesProgress);
  _ticks.name(str::form(_( "Reading '%s' repository cache"), info().alias().c_str()));

  debug::Measure m("create resolvables");
  CapFactory capfactory;
  try
  {
    sqlite3_connection con((_options.dbdir + "zypp.db").asString().c_str());
    //con.setprogresshandler(100, global_progress_handler, (void*)this);

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

    _ticks.tick();

    read_capabilities( con, _options.repository_id, nvras );

    _ticks.tick();

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
    con.setprogresshandler(00, NULL, NULL);
  }
  catch(exception &ex) {
      cerr << "Exception Occured: " << ex.what() << endl;
   }
  //extract_packages_from_directory( _store, thePath, selfRepositoryRef(), true );
}

static CheckSum encoded_string_to_checksum( const std::string &encoded )
{
  vector<string> words;
  if ( str::split( encoded, std::back_inserter(words), ":" ) != 2 )
  {
    return CheckSum();
  }
  else
  {
    return CheckSum( words[0], words[1] );
  }
}

void RepoImpl::createPatchAndDeltas()
{
  _ticks = ProgressData();
  _ticks.sendTo(_options.readingPatchDeltasProgress );
  _ticks.name(str::form(_( "Reading patch and delta rpms from '%s' repository cache"), info().alias().c_str()));

  try
  {
    sqlite3_connection con((_options.dbdir + "zypp.db").asString().c_str());
    //con.setprogresshandler(500, global_progress_handler, (void*)this);
    con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");

    string pp_query =
   //     0       1          2         3        4            5
    "SELECT id, media_nr, location, checksum, download_size, build_time "
    "FROM patch_packages WHERE repository_id=:repository_id;";

    string pp_bv_query =
    "SELECT version, release, epoch "
    "FROM patch_packages_baseversions WHERE patch_package_id = :patch_package_id";

    string delta_query =
    //       0     1      2          3        4         5              6                      7                      8                 9                     10
    "SELECT id, media_nr, location, checksum, download_size, build_time,  baseversion_version, baseversion_release, baseversion_epoch, baseversion_checksum, baseversion_build_time "
    //    11
    ", baseversion_sequence_info "
    "FROM delta_packages WHERE repository_id=:repository_id;";

    // bind the master repo id to the query
    sqlite3_command deltas_cmd( con, delta_query);
    deltas_cmd.bind(":repository_id", _options.repository_id);
    sqlite3_reader reader = deltas_cmd.executereader();
    while ( reader.read() )
    {
      zypp::OnMediaLocation on_media( reader.getstring(2), reader.getint(1) );

      string checksum_string(reader.getstring(3));
      CheckSum checksum = encoded_string_to_checksum(checksum_string);
      if ( checksum.empty() )
      {
        ERR << "Wrong checksum for delta, skipping..." << endl;
        continue;
      }
      on_media.setChecksum(checksum);
      on_media.setDownloadSize(reader.getint(4));

      packagedelta::DeltaRpm::BaseVersion baseversion;
      baseversion.setEdition( Edition(reader.getstring(6), reader.getstring(7), reader.getstring(8) ) );

      checksum_string = reader.getstring(9);
      checksum = encoded_string_to_checksum(checksum_string);
      if ( checksum.empty() )
      {
        ERR << "Wrong checksum for delta, skipping..." << endl;
        continue;
      }
      baseversion.setChecksum(checksum);
      baseversion.setBuildtime(reader.getint(10));
      baseversion.setSequenceinfo(reader.getstring(11));

      zypp::packagedelta::DeltaRpm delta;
      delta.setLocation( on_media );
      delta.setBaseversion( baseversion );
      delta.setBuildtime(reader.getint(5));

      _deltaRpms.push_back(delta);
    }

    // patch rpms
    // bind the master package id to the query
    // bind the master repo id to the query
    sqlite3_command pp_cmd( con, pp_query);
    sqlite3_command pp_bv_cmd( con, pp_bv_query);
    pp_cmd.bind(":repository_id", _options.repository_id);
    reader = pp_cmd.executereader();

    while ( reader.read() )
    {
      long long patch_package_id = reader.getint64(0);

      zypp::OnMediaLocation on_media( reader.getstring(2), reader.getint(1) );

      string checksum_string(reader.getstring(3));
      CheckSum checksum = encoded_string_to_checksum(checksum_string);
      if ( checksum.empty() )
      {
        ERR << "Wrong checksum for delta, skipping..." << endl;
        continue;
      }
      on_media.setChecksum(checksum);
      on_media.setDownloadSize(reader.getint(4));

      zypp::packagedelta::PatchRpm patch;
      patch.setLocation( on_media );
      patch.setBuildtime(reader.getint(5));

      pp_bv_cmd.bind( ":patch_package_id", patch_package_id );

      sqlite3_reader bv_reader = pp_bv_cmd.executereader();
      while (bv_reader.read())
      {
        packagedelta::PatchRpm::BaseVersion baseversion = packagedelta::PatchRpm::BaseVersion( bv_reader.getstring(0) , bv_reader.getstring(1), bv_reader.getint(2) );
        patch.addBaseversion(baseversion);
      }

      _patchRpms.push_back(patch);
    }
    con.setprogresshandler(0, NULL, NULL);
  }
  catch(exception &ex) {
      cerr << "Exception Occured: " << ex.what() << endl;
  }
}

ResolvableQuery RepoImpl::resolvableQuery()
{
  return _rquery;
}

void RepoImpl::read_capabilities( sqlite3_connection &con,
                                  data::RecordId repo_id,
                                  map<data::RecordId, pair<Resolvable::Kind, NVRAD> > &nvras )
{
  CapFactory capfactory;


  // precompile statements



//   map<data::RecordId, capability::CapabilityImpl::Ptr> named_caps;
//   sqlite3_command select_named_cmd( con, "select v.id, c.refers_kind, n.name, v.version, v.release, v.epoch, v.relation named_capabilities v, capabilities c, names n where v.name_id=n.id and c.id=ncc.capability_id and ncc.named_capability_id=v.id;");
//   {
//     debug::Measure mnc("read named capabilities");
//     sqlite3_reader reader = select_named_cmd.executereader();
//     while  ( reader.read() )
//     {
//
//     }
//   }
  sqlite3_command select_named_cmd( con, "select v.refers_kind, n.name, v.version, v.release, v.epoch, v.relation, v.dependency_type, v.resolvable_id from named_capabilities v, names n, resolvables res where v.name_id=n.id and v.resolvable_id=res.id and res.repository_id=:repo_id;");

  sqlite3_command select_file_cmd( con, "select fc.refers_kind, dn.name, fn.name, fc.dependency_type, fc.resolvable_id from file_capabilities fc, files f, dir_names dn, file_names fn, resolvables res where f.id=fc.file_id and f.dir_name_id=dn.id and f.file_name_id=fn.id and fc.resolvable_id=res.id and res.repository_id=:repo_id;");

  sqlite3_command select_hal_cmd( con, "select hc.refers_kind, hc.name, hc.value, hc.relation, hc.dependency_type, hc.resolvable_id from hal_capabilities hc, resolvables res where hc.resolvable_id=res.id and res.repository_id=:repo_id;");

  sqlite3_command select_modalias_cmd( con, "select mc.refers_kind, mc.name, mc.value, mc.relation, mc.dependency_type, mc.resolvable_id from modalias_capabilities mc, resolvables res where mc.resolvable_id=res.id and res.repository_id=:repo_id;");

  sqlite3_command select_other_cmd( con, "select oc.refers_kind, oc.value, oc.dependency_type, oc.resolvable_id from other_capabilities oc, resolvables res where oc.resolvable_id=res.id and res.repository_id=:repo_id;");


  {
    debug::Measure mnc("read named capabilities");
    select_named_cmd.bind(":repo_id", repo_id);
    sqlite3_reader reader = select_named_cmd.executereader();
    while  ( reader.read() )
    {

      Resolvable::Kind refer = _type_cache.kindFor(reader.getint(0));
      Rel rel = _type_cache.relationFor(reader.getint(5));

      data::RecordId rid = reader.getint64(7);

      if ( rel == zypp::Rel::NONE )
      {
        capability::NamedCap *ncap = new capability::NamedCap( refer, reader.getstring(1) );
        zypp::Dep deptype = _type_cache.deptypeFor(reader.getint(6));
        nvras[rid].second[deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(ncap) ) );
      }
      else
      {
        capability::VersionedCap *vcap = new capability::VersionedCap( refer, reader.getstring(1), /* rel */ rel, Edition( reader.getstring(2), reader.getstring(3), reader.getint(4) ) );
        zypp::Dep deptype = _type_cache.deptypeFor(reader.getint(6));
        nvras[rid].second[deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(vcap) ) );
      }
    }
  }

  {
    debug::Measure mnf("read file capabilities");
    select_file_cmd.bind(":repo_id", repo_id);
    sqlite3_reader reader = select_file_cmd.executereader();
    while  ( reader.read() )
    {
      Resolvable::Kind refer = _type_cache.kindFor(reader.getint(0));
      capability::FileCap *fcap = new capability::FileCap( refer, reader.getstring(1) + "/" + reader.getstring(2) );
      zypp::Dep deptype = _type_cache.deptypeFor(reader.getint(3));
      data::RecordId rid = reader.getint64(4);
      nvras[rid].second[deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(fcap) ) );
    }
  }

  {
    debug::Measure mnf("read hal capabilities");
    select_hal_cmd.bind(":repo_id", repo_id);
    sqlite3_reader reader = select_hal_cmd.executereader();
    while  ( reader.read() )
    {
      //select hc.refers_kind, hc.name, hc.value, hc.relation, hc.dependency_type, hc.resolvable_id from hal_capabilities hc

      Resolvable::Kind refer = _type_cache.kindFor(reader.getint(0));

      Rel rel = _type_cache.relationFor(reader.getint(3));
      capability::HalCap *hcap = new capability::HalCap( refer, reader.getstring(1), rel, reader.getstring(2) );
      zypp::Dep deptype = _type_cache.deptypeFor(reader.getint(4));
      data::RecordId rid = reader.getint64(5);
      nvras[rid].second[deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(hcap) ) );
    }
  }

  {
    debug::Measure mnf("read modalias capabilities");
    select_modalias_cmd.bind(":repo_id", repo_id);
    sqlite3_reader reader = select_modalias_cmd.executereader();
    while  ( reader.read() )
    {

      //select mc.refers_kind, mc.name, mc.value, mc.relation, mc.dependency_type, mc.resolvable_id from modalias_capabilities mc;
      Resolvable::Kind refer = _type_cache.kindFor(reader.getint(0));

      Rel rel = _type_cache.relationFor(reader.getint(3));
      capability::ModaliasCap *mcap = new capability::ModaliasCap( refer, reader.getstring(1), rel, reader.getstring(2) );
      zypp::Dep deptype = _type_cache.deptypeFor(reader.getint(4));
      data::RecordId rid = reader.getint64(5);
      nvras[rid].second[deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(mcap) ) );
    }
  }

  {
    debug::Measure mnf("read other capabilities");
    select_other_cmd.bind(":repo_id", repo_id);
    sqlite3_reader reader = select_other_cmd.executereader();
    while  ( reader.read() )
    {
      //select oc.refers_kind, oc.value, oc.dependency_type, oc.resolvable_id from other_capabilities oc;

      Resolvable::Kind refer = _type_cache.kindFor(reader.getint(0));
      capability::CapabilityImpl::Ptr cap = capability::parse( refer, reader.getstring(1));

      if ( !cap )
      {
        ERR << "Invalid capability " <<  reader.getstring(1) << endl;
      }

      zypp::Dep deptype = _type_cache.deptypeFor(reader.getint(2));
      data::RecordId rid = reader.getint64(3);
      nvras[rid].second[deptype].insert( capfactory.fromImpl(cap) );
    }
  }

  MIL << nvras.size() << " capabilities" << endl;
}


/////////////////////////////////////////////////////////////////
} // namespace plaindir
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace repository
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

