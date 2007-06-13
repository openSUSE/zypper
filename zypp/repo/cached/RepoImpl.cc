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

#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"
#include "zypp/capability/Capabilities.h"
#include "zypp/cache/ResolvableQuery.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/CapFactory.h"

#include "zypp/repo/cached/RepoImpl.h"
#include "zypp/repo/cached/PackageImpl.h"

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

RepoImpl::RepoImpl( const RepoInfo &repoinfo, const Pathname &dbdir, const data::RecordId &repository_id )
  : RepositoryImpl(repoinfo),
    _dbdir(dbdir),
    _type_cache(dbdir),
    _repository_id(repository_id),
    _rquery(dbdir)
{

}

RepoImpl::~RepoImpl()
{

}


void RepoImpl::factoryInit()
{
  MIL << "Plaindir repository initialized." << std::endl;
}

void read_capabilities( sqlite3_connection &con, map<data::RecordId, NVRAD> &nvras );


void RepoImpl::createResolvables()
{
  debug::Measure m("create resolvables");
  CapFactory capfactory;
  try
  { 
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");

    sqlite3_command cmd( con, "select id,name,version,release,epoch,arch,kind from resolvables where repository_id=:repository_id;");
    cmd.bind(":repository_id", _repository_id);
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
    
    read_capabilities( con, nvras);
    
    for ( map<data::RecordId, pair<Resolvable::Kind, NVRAD> >::const_iterator it = nvras.begin(); it != nvras.end(); ++it )
    {
      if ( it->second.first == ResTraits<Package>::kind )
      {
        ResImplTraits<cached::PackageImpl>::Ptr impl = new cached::PackageImpl(it->first, this);
        Package::Ptr package = detail::makeResolvableFromImpl( it->second.second, impl );
        _store.insert (package);
      }
    }
    con.executenonquery("COMMIT;");
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
  try
  { 
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
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
    deltas_cmd.bind(":repository_id", _repository_id);
    sqlite3_reader reader = deltas_cmd.executereader();
    while ( reader.read() )
    {
      zypp::OnMediaLocation on_media;
      on_media.medianr(reader.getint(1));
      on_media.filename(reader.getstring(2));
      
      string checksum_string(reader.getstring(3));
      CheckSum checksum = encoded_string_to_checksum(checksum_string);
      if ( checksum.empty() )
      {
        ERR << "Wrong checksum for delta, skipping..." << endl;
        continue;
      }
      on_media.checksum(checksum);
      on_media.downloadsize(reader.getint(4));
      
      packagedelta::DeltaRpm::BaseVersion baseversion;
      baseversion.edition( Edition(reader.getstring(6), reader.getstring(7), reader.getstring(8) ) );
      
      checksum_string = reader.getstring(9);
      checksum = encoded_string_to_checksum(checksum_string);
      if ( checksum.empty() )
      {
        ERR << "Wrong checksum for delta, skipping..." << endl;
        continue;
      }
      baseversion.checksum(checksum);
      baseversion.buildtime(reader.getint(10));
      baseversion.sequenceinfo(reader.getstring(11));
        
      zypp::packagedelta::DeltaRpm delta;
      delta.location( on_media );
      delta.baseversion( baseversion );
      delta.buildtime(reader.getint(5));
      
      //impl->addDeltaRpm(delta);
    }
    
    // patch rpms
    // bind the master package id to the query
    // bind the master repo id to the query
    sqlite3_command pp_cmd( con, pp_query);
    sqlite3_command pp_bv_cmd( con, pp_bv_query);
    pp_cmd.bind(":repository_id", _repository_id);
    reader = pp_cmd.executereader();

    while ( reader.read() )
    {
      long long patch_package_id = reader.getint64(0);
      
      zypp::OnMediaLocation on_media;
      on_media.medianr( reader.getint(1) );
      on_media.filename( reader.getstring(2) );
      
      string checksum_string(reader.getstring(3));
      CheckSum checksum = encoded_string_to_checksum(checksum_string);
      if ( checksum.empty() )
      {
        ERR << "Wrong checksum for delta, skipping..." << endl;
        continue;
      }
      on_media.checksum(checksum);
      on_media.downloadsize(reader.getint(4));
      
      zypp::packagedelta::PatchRpm patch;
      patch.location( on_media );
      patch.buildtime(reader.getint(5));
      
      pp_bv_cmd.bind( ":patch_package_id", patch_package_id );
      
      sqlite3_reader bv_reader = pp_bv_cmd.executereader();
      while (bv_reader.read())
      {
        packagedelta::PatchRpm::BaseVersion baseversion = packagedelta::PatchRpm::BaseVersion( bv_reader.getstring(0) , bv_reader.getstring(1), bv_reader.getint(2) );
        patch.baseversion(baseversion);
      }
        
      //impl->addPatchRpm(patch);
    }
  }
  catch(exception &ex) {
      cerr << "Exception Occured: " << ex.what() << endl;
  }
}
    
ResolvableQuery RepoImpl::resolvableQuery()
{
  return _rquery;
}

void RepoImpl::read_capabilities( sqlite3_connection &con, map<data::RecordId, pair<Resolvable::Kind, NVRAD> > &nvras )
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
  sqlite3_command select_named_cmd( con, "select v.refers_kind, n.name, v.version, v.release, v.epoch, v.relation, v.dependency_type, v.resolvable_id from named_capabilities v, names n where v.name_id=n.id;");
  sqlite3_command select_file_cmd( con, "select fc.refers_kind, dn.name, fn.name, fc.dependency_type, fc.resolvable_id from file_capabilities fc, files f, dir_names dn, file_names fn where f.id=fc.file_id and f.dir_name_id=dn.id and f.file_name_id=fn.id;");
  
  {
    debug::Measure mnc("read named capabilities");
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

