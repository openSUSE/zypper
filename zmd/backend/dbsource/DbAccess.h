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

#include "zypp/ResStatus.h"
#include "zypp/ResStore.h"
#include "zypp/Resolvable.h"
#include "zypp/Package.h"
#include "zypp/Patch.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"
#include "zypp/Product.h"
#include "zypp/Pathname.h"
#include "zypp/ResPool.h"
#include "zypp/CapSet.h"
#include "zypp/Dep.h"
#include "zypp/Rel.h"
#include "zypp/Arch.h"

DEFINE_PTR_TYPE(DbAccess);

typedef std::list<zypp::ResObject::constPtr> ResObjectList;
typedef std::map<sqlite_int64, zypp::ResObject::constPtr> IdMap;
//-----------------------------------------------------------------------------
// relations

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

//-----------------------------------------------------------------------------
// architectures

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

//-----------------------------------------------------------------------------
// dependencies

typedef enum {
	RC_DEP_TYPE_REQUIRE = 0,
	RC_DEP_TYPE_PROVIDE,			// 1
	RC_DEP_TYPE_CONFLICT,			// 2
	RC_DEP_TYPE_OBSOLETE,			// 3
	RC_DEP_TYPE_PREREQUIRE,			// 4
	RC_DEP_TYPE_FRESHEN,			// 5
	RC_DEP_TYPE_RECOMMEND,			// 6
	RC_DEP_TYPE_SUGGEST,			// 7
	RC_DEP_TYPE_SUPPLEMENT,			// 8
	RC_DEP_TYPE_ENHANCE			// 9
} RCDependencyType;

//-----------------------------------------------------------------------------
// kinds (dependencies.dep_target

typedef enum {
	RC_DEP_TARGET_PACKAGE = 0,
	RC_DEP_TARGET_SCRIPT,			// 1
	RC_DEP_TARGET_MESSAGE,			// 2
	RC_DEP_TARGET_PATCH,			// 3
	RC_DEP_TARGET_SELECTION,		// 4
	RC_DEP_TARGET_PATTERN,			// 5
	RC_DEP_TARGET_PRODUCT,			// 6
	RC_DEP_TARGET_LANGUAGE,			// 7
	RC_DEP_TARGET_ATOM,			// 8
	RC_DEP_TARGET_SRC,			// 9
	RC_DEP_TARGET_UNKNOWN=42		// 42
} RCDependencyTarget;

//-----------------------------------------------------------------------------
// status (mostly for patches)
//   to be evaluated together with resolvables.installed

typedef enum {
	RC_RES_STATUS_UNDETERMINED = 0,
	RC_RES_STATUS_UNNEEDED,			// not needed
	RC_RES_STATUS_SATISFIED,		// needed, dependencies complete
	RC_RES_STATUS_BROKEN			// needed, dependencies incomplete
} RCResolvableStatus;

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
    sqlite3_stmt *_insert_patch_handle;
    sqlite3_stmt *_insert_selection_handle;
    sqlite3_stmt *_insert_pattern_handle;
    sqlite3_stmt *_insert_product_handle;
    sqlite3_stmt *_insert_dep_handle;

    void commit();

    sqlite_int64 writeResObject( zypp::ResObject::constPtr obj, zypp::ResStatus status, const char *catalog = NULL );

    sqlite_int64 writePackage( sqlite_int64 id, zypp::Package::constPtr package, zypp::ResStatus status );
    sqlite_int64 writePatch( sqlite_int64 id, zypp::Patch::constPtr patch, zypp::ResStatus status );
    sqlite_int64 writeSelection( sqlite_int64 id, zypp::Selection::constPtr selection, zypp::ResStatus status );
    sqlite_int64 writePattern( sqlite_int64 id, zypp::Pattern::constPtr pattern, zypp::ResStatus status );
    sqlite_int64 writeProduct( sqlite_int64 id, zypp::Product::constPtr product, zypp::ResStatus status );

    void writeDependencies( sqlite_int64 id, zypp::Resolvable::constPtr res);
    void writeDependency( sqlite_int64 pkg_id, RCDependencyType type, const zypp::CapSet & capabilities);

    bool prepareWrite( void );

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
    bool openDb( bool for_writing );
    void closeDb( void );

    /** check if catalog exists */
    bool haveCatalog( const std::string & catalog );
    /** insert catalog */
    bool insertCatalog( const std::string & catalog, const std::string & name, const std::string & alias, const std::string & description );
    /** remove catalog, remove all resolvables of this catalog */
    bool removeCatalog( const std::string & catalog );

    /** write resolvables from store to db */
    void writeStore( const zypp::ResStore & resolvables, zypp::ResStatus status, const char *catalog = NULL );
    /** write resolvables from pool to db */
    void writePool( const zypp::ResPool & pool, const char *catalog = NULL );

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
