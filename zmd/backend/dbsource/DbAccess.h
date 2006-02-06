/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/DbAccess.h
 *
*/
#ifndef ZMD_BACKEND_DBSOURCE_DBACCESS_H
#define ZMD_BACKEND_DBSOURCE_DBACCESS_H

#include <iosfwd>
#include <string>

#include <sqlite3.h>

#include "zypp/base/PtrTypes.h"

#include "zypp/ResStore.h"
#include "zypp/Resolvable.h"
#include "zypp/Package.h"
#include "zypp/Pathname.h"
#include "zypp/ResPool.h"
#include "zypp/CapSet.h"
#include "zypp/Dep.h"

DEFINE_PTR_TYPE(DbAccess);

typedef std::list<zypp::ResObject::constPtr> ResObjectList;


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : DbAccess
//
/**
*/

class DbAccess : public zypp::base::ReferenceCounted, public zypp::base::NonCopyable
{
  private:
    typedef enum {
	RC_DEP_TYPE_REQUIRE = 0,
	RC_DEP_TYPE_PROVIDE,
	RC_DEP_TYPE_CONFLICT,
	RC_DEP_TYPE_OBSOLETE,
	RC_DEP_TYPE_PREREQUIRE,
	RC_DEP_TYPE_FRESHEN,
	RC_DEP_TYPE_RECOMMEND,
	RC_DEP_TYPE_SUGGEST,
	RC_DEP_TYPE_ENHANCE
    } RCDependencyType;

  private:
    std::string _dbfile;

    sqlite3 *_db;
    sqlite3_stmt *_insert_res_handle;
    sqlite3_stmt *_insert_pkg_handle;
    sqlite3_stmt *_insert_dep_handle;

    bool openDb(bool for_writing);
    void closeDb(void);

    bool prepareWrite(void);
    bool prepareRead(void);

    void commit();

    sqlite_int64 writeResObject( zypp::ResObject::constPtr obj, bool is_installed);
    sqlite_int64 writePackage( sqlite_int64 id, zypp::Package::constPtr pkg);
    void writeDependencies( sqlite_int64 id, zypp::Resolvable::constPtr res);
    void writeDependency( sqlite_int64 pkg_id, RCDependencyType type, const zypp::CapSet & capabilities);

public:
    /** Ctor */
    DbAccess( const std::string & dbfile_r );
    /** Dtor */
    ~DbAccess( );

    void writeStore( const zypp::ResStore & resolvables, bool is_installed );
    void writeResObjects( const ResObjectList & resolvables, bool is_installed );

private:
    friend std::ostream & operator<<( std::ostream & str, const DbAccess & obj );
    /** Stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;

};
///////////////////////////////////////////////////////////////////

/** \relates DbAccess Stream output. */
inline std::ostream & operator<<( std::ostream & str, const DbAccess & obj )
{ return obj.dumpOn( str ); }


#endif // ZMD_BACKEND_DBSOURCE_DBACCESS_H
