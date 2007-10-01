/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/cache/CacheTypes.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"
#include "zypp/CheckSum.h"
#include "zypp/cache/CacheTypes.h"

using namespace std;
using namespace sqlite3x;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CacheTypes
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CacheTypes::CacheTypes
    //	METHOD TYPE : Ctor
    //
    CacheTypes::CacheTypes( const Pathname &dbdir )
      : _dbdir(dbdir)
    {
      refreshCache();
    }

    void CacheTypes::refreshCache()
    {
      try
      {
        sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
        con.executenonquery("PRAGMA cache_size=8000;");
        con.executenonquery("BEGIN;");
      
        // get all types
        sqlite3_command select_types_cmd( con, "select id,class,name from types;");
        sqlite3_reader reader = select_types_cmd.executereader();
      
        while(reader.read())
        {
          data::RecordId id = reader.getint64(0);
          string klass = reader.getstring(1);
          string name = reader.getstring(2);
          if ( klass == "arch" )
            _arch_cache[id] = Arch(name);
          if ( klass == "rel" )
            _rel_cache[id] = Rel(name);
          if ( klass == "kind" )
            _kind_cache[id] = Resolvable::Kind(name);
          if ( klass == "deptype" )
	    /* As Dep has no default ctor we can't use the [] accessor.  */
	    _deptype_cache.insert (make_pair (id, Dep(name)));
        }
        
        MIL << "archs: " << _arch_cache.size() << endl;
        MIL << "rel  : " << _rel_cache.size() << endl;
        MIL << "kind : " << _kind_cache.size() << endl;
        MIL << "deptype : " << _deptype_cache.size() << endl;
      }
      catch ( std::exception &e )
      {
        ZYPP_THROW(Exception("Error reading types"));
      }
    }
    
    Rel CacheTypes::relationFor( const data::RecordId &id )
    {
      Rel rel;
      std::map<data::RecordId, Rel>::const_iterator it;
      if ( (it = _rel_cache.find(id) ) != _rel_cache.end() )
        rel = it->second;
      else
        ZYPP_THROW(Exception("Inconsistent Rel"));
      
      return rel;
    
    }

    data::RecordId CacheTypes::idForRelation( const Rel &rel )
    {
      std::map<data::RecordId, Rel>::const_iterator it;
      for ( it = _rel_cache.begin(); it != _rel_cache.end(); ++it )
      {
        if ( rel == it->second )
	  return it->first;
      }
      ZYPP_THROW(Exception("Inconsistent Rel"));   
    }
    
    Resolvable::Kind CacheTypes::kindFor( const data::RecordId &id )
    {
      Resolvable::Kind kind;
      std::map<data::RecordId, Resolvable::Kind>::const_iterator it;
      if ( (it = _kind_cache.find(id) ) != _kind_cache.end() )
        kind = it->second;
      else
        ZYPP_THROW(Exception("Inconsistent Kind"));
      
      return kind;
    }

    data::RecordId CacheTypes::idForKind( const Resolvable::Kind & kind )
    {
      std::map<data::RecordId, Resolvable::Kind>::const_iterator it;
      for ( it = _kind_cache.begin(); it != _kind_cache.end(); ++it )
      {
        if ( kind == it->second )
	  return it->first;
      }
      ZYPP_THROW(Exception("Inconsistent Kind"));   
    }
    
    Dep CacheTypes::deptypeFor( const data::RecordId &id )
    {
      std::map<data::RecordId, Dep>::const_iterator it;
      if ( (it = _deptype_cache.find(id) ) != _deptype_cache.end() )
        return it->second;
      else
      {
        ERR << "deptype: " << id << endl;
        ZYPP_THROW(Exception("Inconsistent deptype"));
      }
    }

    data::RecordId CacheTypes::idForDeptype( const Dep & dep )
    {
      std::map<data::RecordId, Dep>::const_iterator it;
      for ( it = _deptype_cache.begin(); it != _deptype_cache.end(); ++it )
      {
        if ( dep == it->second )
	  return it->first;
      }
      ZYPP_THROW(Exception("Inconsistent deptype"));   
    }
    
    Arch CacheTypes::archFor( const data::RecordId &id )
    {
      
      Arch arch;
      std::map<data::RecordId, Arch>::const_iterator it;
      if ( (it = _arch_cache.find(id) ) != _arch_cache.end() )
        arch = it->second;
      else
        ZYPP_THROW(Exception("Inconsistent Arch"));
      
      return arch;
    }

    data::RecordId CacheTypes::idForArch( const Arch & arch )
    {
      std::map<data::RecordId, Arch>::const_iterator it;
      for ( it = _arch_cache.begin(); it != _arch_cache.end(); ++it )
      {
        if ( arch == it->second )
	  return it->first;
      }
      ZYPP_THROW(Exception("Inconsistent Arch"));   
    }



    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CacheTypes::~CacheTypes
    //	METHOD TYPE : Dtor
    //
    CacheTypes::~CacheTypes()
    {}
    
    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const CacheTypes & obj )
    {
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
