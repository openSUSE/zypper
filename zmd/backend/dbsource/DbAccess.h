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
#include "zypp/Rel.h"
#include "zypp/Arch.h"

DEFINE_PTR_TYPE(DbAccess);

typedef std::list<zypp::ResObject::constPtr> ResObjectList;

#define RELATION_ANY 0
#define RELATION_EQUAL (1 << 0)
#define RELATION_LESS (1 << 1)
#define RELATION_GREATER (1 << 2)
#define RELATION_NONE (1 << 3)

typedef enum {
	RC_RELATION_INVALID            = -1,
	RC_RELATION_ANY                = RELATION_ANY,
	RC_RELATION_EQUAL              = RELATION_EQUAL,
	RC_RELATION_LESS               = RELATION_LESS,
	RC_RELATION_LESS_EQUAL         = RELATION_LESS | RELATION_EQUAL,
	RC_RELATION_GREATER            = RELATION_GREATER,
	RC_RELATION_GREATER_EQUAL      = RELATION_GREATER | RELATION_EQUAL,
	RC_RELATION_NOT_EQUAL          = RELATION_LESS | RELATION_GREATER,
	RC_RELATION_NONE               = RELATION_NONE,
} RCResolvableRelation;


typedef enum {
    RC_ARCH_UNKNOWN = -1,
    RC_ARCH_NOARCH = 0,
    RC_ARCH_I386,
    RC_ARCH_I486,
    RC_ARCH_I586,
    RC_ARCH_I686,
    RC_ARCH_X86_64,
    RC_ARCH_IA32E,
    RC_ARCH_ATHLON,
    RC_ARCH_PPC,
    RC_ARCH_PPC64,
    RC_ARCH_S390,
    RC_ARCH_S390X,
    RC_ARCH_IA64,
    RC_ARCH_SPARC,
    RC_ARCH_SPARC64,
} RCArch;

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

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : DbAccess
//
/**
*/

class DbAccess : public zypp::base::ReferenceCounted, public zypp::base::NonCopyable
{
  private:
    std::string _dbfile;

    sqlite3 *_db;
    sqlite3_stmt *_insert_res_handle;
    sqlite3_stmt *_insert_pkg_handle;
    sqlite3_stmt *_insert_dep_handle;

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

    static RCResolvableRelation Rel2Rc (zypp::Rel op);
    static zypp::Rel Rc2Rel (RCResolvableRelation rel);
    static RCArch Arch2Rc (const zypp::Arch & arch);
    static zypp::Arch Rc2Arch (RCArch rc);

    sqlite3 *db() const { return _db; }
    bool openDb(bool for_writing);
    void closeDb(void);

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
