/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/librpmDb.h
 *
*/
#ifndef librpmDb_h
#define librpmDb_h

#include <iosfwd>

#include <zypp/base/ReferenceCounted.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/PathInfo.h>
#include <zypp/Package.h>
#include <zypp/target/rpm/RpmHeader.h>
#include <zypp/target/rpm/RpmException.h>

namespace zypp
{
namespace target
{
namespace rpm
{

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : librpmDb
/**
 * @short Manage access to librpm database.
 **/
class librpmDb : public base::ReferenceCounted, private base::NonCopyable
{
public:
  typedef intrusive_ptr<librpmDb> Ptr;
  typedef intrusive_ptr<const librpmDb> constPtr;
private:
  /**
   * <B>INTENTIONALLY UNDEFINED<\B> because of bug in Ptr classes
   * which allows implicit conversion from librpmDb::Ptr to
   * librpmDb::constPtr. Currently we don't want to provide non const
   * handles, as the database is opened READONLY.
  *
   * \throws RpmException
   *
   **/
  static void dbAccess( librpmDb::Ptr & ptr_r );

public:

  ///////////////////////////////////////////////////////////////////
  //
  //	static interface
  //
  ///////////////////////////////////////////////////////////////////
private:

  /**
   * Current root directory for all operations.
   * (initialy /)
   **/
  static Pathname _defaultRoot;

  /**
   * Current directory (below root) that contains the rpmdb.
   * (initialy /var/lib/rpm)
   **/
  static const Pathname _defaultDbPath;

  /**
   * Current rpmdb handle.
   **/
  static librpmDb::constPtr _defaultDb;

  /**
   * Whether access is blocked (no _defaultDb will be available).
   **/
  static bool _dbBlocked;

  /**
   * For internal use. Pointer returned should immediately be
   * wrapped into librpmDb::Ptr.
   *
   * \throws RpmException
   *
   **/
  static librpmDb * newLibrpmDb();

  /**
   * Access the database at the current default location. If necessary
   * (eg. after @ref dbRelease), the database is opened. This just creates
   * the internal handle. Once the handle is passed to e.g. some
   * @ref db_const_iterator, the database will be closed if the last
   * outstanding reference goes out of scope. If no external reference is
   * created, you'll have to explicitly call @ref dbRelease to close the
   * database.
   *
   * \throws RpmException
   *
   **/
  static void dbAccess();

public:

  /**
   * Initialize lib librpm (read configfiles etc.). It's called
   * on demand but you may call it anytime.
   *
   * @return Whether librpm is initialized.
   **/
  static bool globalInit();

  /**
   * @return librpm macro expansion.
   **/
  static std::string expand( const std::string & macro_r );

  /**
   * @return String '(root_r)sub_r' used in debug output.
   **/
  static std::string stringPath( const Pathname & root_r, const Pathname & sub_r )
  {
    return std::string( "'(" ) + root_r.asString() + ")" + sub_r.asString() + "'";
  }

public:

  /**
   * @return Current root directory for all operations.
   **/
  static const Pathname & defaultRoot()
  {
    return _defaultRoot;
  }

  /**
   * @return Current directory (below root) that contains the rpmdb.
   **/
  static const Pathname & defaultDbPath()
  {
    return _defaultDbPath;
  }

  /**
   * Adjust access to the given database location, making it the new
   * default location on success. No relative Pathnames are allowed.
   *
   * It's not possible to access a database while access is blocked
   * (see @ref blockAccess), but valid Pathnames provided will be stored
   * as new default location.
   *
   * It's not allowed to switch to another location while a database
   * is accessed. Use @ref dbRelease to force releasing the database first.
  *
   * \throws RpmException
   *
   **/
  static void dbAccess( const Pathname & root_r );

  /**
   * Same as &ref dbAccess(), but returns the database handle if
   * avaialble, otherwise NULL. This creates an external reference, thus
   * it should not be used longer than necessary. Be prepared that the
   * handle might become invalid (see @ref dbRelease) later.
   *
   * \throws RpmException
   *
   **/
  static void dbAccess( librpmDb::constPtr & ptr_r );

  /**
   * If there are no outstanding references to the database (e.g. by @ref db_const_iterator),
   * the database is closed. Subsequent calls to @ref dbAccess may however
   * open the database again.
   *
   * If forced, the internal reference is dropped and it will look like
   * the database was closed. But physically the database will be closed
   * after all outstanding references are gone.
   *
   * @return The number of outstandig references to the database, 0 if
   * if database was physically closed.
   **/
  static unsigned dbRelease( bool force_r = false );

  /**
   * Blocks further access to rpmdb. Basically the same as @ref dbRelease( true ),
   * but subsequent calls to @ref dbAccess will fail returning E_RpmDB_access_blocked.
   *
   * @return The number of outstandig references to the database, 0 if
   * if database was physically closed.
   **/
  static unsigned blockAccess();

  /**
   * Allow access to rpmdb e.g. after @ref blockAccess. Subsequent calls to
   * @ref dbAccess will perform.
   *
   * <B>NOTE:</B> Initially we're in blocked mode. So you must call @ref unblockAccess
   * unblockAccess at least once. Othwise nothing will happen.
   *
   * @return The number of outstandig references to the database, 0 if
   * if database was physically closed.
   **/
  static void unblockAccess();

  /**
   * @return Whether database access is blocked.
   **/
  static bool isBlocked()
  {
    return _dbBlocked;
  }

  /**
   * Dump debug info.
   **/
  static std::ostream & dumpState( std::ostream & str );

public:

  /**
   * Subclass to retrieve database content.
   **/
  class db_const_iterator;

private:
  ///////////////////////////////////////////////////////////////////
  //
  //	internal database handle interface (nonstatic)
  //
  ///////////////////////////////////////////////////////////////////

  /**
   * Hides librpm specific data
   **/
  class D;
  D & _d;

protected:

  /**
   * Private constructor! librpmDb objects are to be created via
   * static interface only.
   **/
  librpmDb( const Pathname & root_r, const Pathname & dbPath_r, bool readonly_r );

  /**
   * Trigger from @ref Rep, after refCount was decreased.
   **/
  virtual void unref_to( unsigned refCount_r ) const;

public:

  /**
   * Destructor. Closes rpmdb.
   **/
  virtual ~librpmDb();

  /**
   * @return This handles root directory for all operations.
   **/
  const Pathname & root() const;

  /**
   * @return This handles directory that contains the rpmdb.
   **/
  const Pathname & dbPath() const;

  /**
   * Return any database error. Usg. if the database was
   * blocked by calling @ref dbRelease(true) or @ref blockAccess.
   **/
  shared_ptr<RpmException> error() const;

  /**
   * @return Whether handle is valid.
   **/
  bool valid() const
  {
    return( ! error() );
  }

  /**
   * @return True if handle is valid and database is empty.
   **/
  bool empty() const;

  /**
   * @return Number of entries in the database (0 if not valid).
   **/
  unsigned size() const;

public:

  /**
   * Dont call it ;) It's for development and testing only.
   **/
  void * dont_call_it() const;

  /**
   * Dump debug info.
   **/
  virtual std::ostream & dumpOn( std::ostream & str ) const;
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : librpmDb::db_const_iterator
/**
 * @short Subclass to retrieve database content.
 *
 *
 **/
class librpmDb::db_const_iterator
{
  db_const_iterator & operator=( const db_const_iterator & ); // NO ASSIGNMENT!
  db_const_iterator ( const db_const_iterator & );            // NO COPY!
  friend std::ostream & operator<<( std::ostream & str, const db_const_iterator & obj );
  friend class librpmDb;

private:

  /**
   * Hides librpm specific data
   **/
  class D;
  D & _d;

public:

  /**
   * Constructor. Iterator is initialized to @ref findAll.
   * The default form accesses librpmDb's default database.
   * Explicitly providing a database handle should not be
   * neccesary, except for testing.
   **/
  db_const_iterator( librpmDb::constPtr dbptr_r = 0 );

  /**
   * Destructor.
   **/
  ~db_const_iterator();

  /**
   * Return any database error.
   *
   * <B>NOTE:</B> If the database gets blocked (see @ref dbRelease)
   * dbError will immediately report this, but an already running
   * iteration will proceed to its end. Then the database is dropped.
   **/
  shared_ptr<RpmException> dbError() const;

  /**
   * Advance to next RpmHeader::constPtr.
   **/
  void operator++();

  /**
   * Returns the current headers index in database,
   * 0 if no header.
   **/
  unsigned dbHdrNum() const;

  /**
   * Returns the current RpmHeader::constPtr or
   * NULL, if no more entries available.
   **/
  const RpmHeader::constPtr & operator*() const;

  /**
   * Forwards to the current RpmHeader::constPtr.
   **/
  const RpmHeader::constPtr & operator->() const
  {
    return operator*();
  }

public:

  /**
   * Reset to iterate all packages. Returns true if iterator
   * contains at least one entry.
   *
   * <B>NOTE:</B> No entry (false) migt be returned due to a
   * meanwhile blocked database (see @ref dbRelease). Use
   * @ref dbError to check this.
   **/
  bool findAll();

  /**
   * Reset to iterate all packages that own a certain file.
   **/
  bool findByFile( const std::string & file_r );

  /**
   * Reset to iterate all packages that provide a certain tag.
   **/
  bool findByProvides( const std::string & tag_r );

  /**
   * Reset to iterate all packages that require a certain tag.
   **/
  bool findByRequiredBy( const std::string & tag_r );

  /**
   * Reset to iterate all packages that conflict with a certain tag.
   **/
  bool findByConflicts( const std::string & tag_r );

  /**
   * Reset to iterate all packages with a certain name.
   *
   * <B>NOTE:</B> Multiple entries for one package installed
   * in different versions are possible but not desired. Usually
   * you'll want to use @ref findPackage instead.
   *
   * findByName is needed to retrieve pseudo packages like
   * 'gpg-pubkey', which in fact exist in multiple instances.
   **/
  bool findByName( const std::string & name_r );

public:

  /**
   * Find package by name.
   *
   * Multiple entries for one package installed in different versions
   * are possible but not desired. If so, the last package installed
   * is returned.
   **/
  bool findPackage( const std::string & name_r );

  /**
   * Find package by name and edition.
   * Commonly used by PMRpmPackageDataProvider.
   **/
  bool findPackage( const std::string & name_r, const Edition & ed_r );

  /**
   * Abbr. for <code>findPackage( which_r->name(), which_r->edition() );</code>
   **/
  bool findPackage( const Package::constPtr & which_r );
};

///////////////////////////////////////////////////////////////////
} //namespace rpm
} //namespace target
} // namespace zypp

#endif // librpmDb_h

