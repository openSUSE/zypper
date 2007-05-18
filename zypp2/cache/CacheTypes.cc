/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/cache/CacheTypes.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"
#include "zypp/CheckSum.h"
#include "zypp2/cache/CacheTypes.h"

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
            _deptype_cache[id] = name;
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
    
    Dep CacheTypes::deptypeFor( const data::RecordId &id )
    {
      std::map<data::RecordId, string>::const_iterator it;
      if ( (it = _deptype_cache.find(id) ) != _deptype_cache.end() )
        return Dep(it->second);
      else
      {
        ERR << "deptype: " << id << endl;
        ZYPP_THROW(Exception("Inconsistent deptype"));
      }
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
} // namespace zypp2
///////////////////////////////////////////////////////////////////
