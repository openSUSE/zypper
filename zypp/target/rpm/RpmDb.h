 /*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmDb.h
 *
*/

// -*- C++ -*-

#ifndef ZYPP_TARGET_RPM_RPMDB_H
#define ZYPP_TARGET_RPM_RPMDB_H

#include <iosfwd>
#include <list>
#include <vector>
#include <string>

#include "zypp/Pathname.h"
#include "zypp/ExternalProgram.h"

#include "zypp/Package.h"
#include "zypp/Source.h"
#include "zypp/KeyRing.h"

#include "zypp/target/rpm/RpmHeader.h"
#include "zypp/target/rpm/RpmCallbacks.h"
#include "zypp/ZYppCallbacks.h"

namespace zypp {
  namespace target {
    namespace rpm {


      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : RpmDb
      /**
       * @short Interface to the rpm program
       **/
      class RpmDb : public base::ReferenceCounted, private base::NonCopyable
      {
	public:
      
	  /**
	   * Default error class
	   **/
	  typedef class InstTargetError Error;
      
	  ///////////////////////////////////////////////////////////////////
	  //
	  // INITALISATION
	  //
	  ///////////////////////////////////////////////////////////////////
	private:
      
	  enum DbStateInfoBits {
	    DbSI_NO_INIT	= 0x0000,
	    DbSI_HAVE_V4	= 0x0001,
	    DbSI_MADE_V4	= 0x0002,
	    DbSI_MODIFIED_V4	= 0x0004,
	    DbSI_HAVE_V3	= 0x0008,
	    DbSI_HAVE_V3TOV4	= 0x0010,
	    DbSI_MADE_V3TOV4	= 0x0020
	  };
      
	  friend std::ostream & operator<<( std::ostream & str, const DbStateInfoBits & obj );
      
	  void dbsi_set( DbStateInfoBits & val_r, const unsigned & bits_r ) const {
	    val_r = (DbStateInfoBits)(val_r | bits_r);
	  }
	  void dbsi_clr( DbStateInfoBits & val_r, const unsigned & bits_r ) const {
	    val_r = (DbStateInfoBits)(val_r & ~bits_r);
	  }
	  bool dbsi_has( const DbStateInfoBits & val_r, const unsigned & bits_r ) const {
	    return( (val_r & bits_r) == bits_r );
	  }
      
	  /**
	   * Internal state info
	   **/
	  DbStateInfoBits _dbStateInfo;
      
	  /**
	   * Root directory for all operations.
	   **/
	  Pathname _root;
      
	  /**
	   * Directory that contains the rpmdb.
	   **/
	  Pathname _dbPath;
      
	  /**
	   * Internal helper for @ref initDatabase.
	   *
	   * \throws RpmException
	   *
	   **/
	  void internal_initDatabase( const Pathname & root_r, const Pathname & dbPath_r,
      				   DbStateInfoBits & info_r );
      
	  /**
	   * Remove the rpm4 database in dbdir_r and optionally any backup created
	   * on conversion.
	   **/
	  static void removeV4( const Pathname & dbdir_r, bool v3backup_r );
      
	  /**
	   * Remove the rpm3 database in dbdir_r. Create a backup copy named
	   * packages.rpm3 if it does not already exist.
	   **/
	  static void removeV3( const Pathname & dbdir_r, bool v3backup_r );
      
	  /**
	   * Called before the database is modified by installPackage/removePackage.
	   * Invalidates Packages list and moves away any old database.
	   **/
	  void modifyDatabase();
      
	public:
      
	  /**
	   * Constructor. There's no rpmdb access until @ref initDatabase
	   * was called.
	   **/
	  RpmDb();
      
	  /**
	   * Destructor.
	   **/
	  ~RpmDb();
      
	  /**
	   * @return Root directory for all operations (empty if not initialized).
	   **/
	  const Pathname & root() const { return _root; }
      
	  /**
	   * @return Directory that contains the rpmdb (empty if not initialized).
	   **/
	  const Pathname & dbPath() const { return _dbPath; }
      
	  /**
	   * @return Whether we are initialized.
	   **/
	  bool initialized() const { return( ! _root.empty() ); }
      
	  /**
	   * Prepare access to the rpm database. Optional arguments may denote the
	   * root directory for all operations and the directory (below root) that
	   * contains the rpmdb (usg. you won't need to set this).
	   *
	   * On empty Pathnames the default is used:
	   * <PRE>
	   *     root:   /
	   *     dbPath: /var/lib/rpm
	   * </PRE>
	   *
	   * Calling initDatabase a second time with different arguments will return
	   * an error but leave the database in it's original state.
	   *
	   * Converting an old batabase is done if necessary. On update: The converted
	   * database will be removed by @ref closeDatabase, if it was not modified
	   * (no packages were installed or deleted). Otherwise the new database
	   * is kept, and the old one is removed.
	   *
	   * \throws RpmException
	   *
	   **/
	  void initDatabase( Pathname root_r = Pathname(),
      			  Pathname dbPath_r = Pathname() );
      
	  /**
	   * Block further access to the rpm database and go back to uninitialized
	   * state. On update: Decides what to do with any converted database
	   * (see @ref initDatabase).
	   *
	   * \throws RpmException
	   *
	   **/
	  void closeDatabase();
      
	  /**
	   * Rebuild the rpm database (rpm --rebuilddb).
	   *
	   * \throws RpmException
	   *
	   **/
	  void rebuildDatabase();
      
	  /**
	   * Import ascii armored public key in file pubkey_r.
	   *
	   * \throws RpmException
	   *
	   **/
	  void importPubkey( const Pathname & pubkey_r );
      
	  /**
	   * Return the long ids of all installed public keys.
	   **/
          std::list<PublicKey> pubkeys() const;
          
          /**
           * Return the edition of all installed public keys.
           **/
          std::set<Edition> pubkeyEditions() const;
      
	  ///////////////////////////////////////////////////////////////////
	  //
	  // Cached RPM database retrieval via librpm.
	  //
	  ///////////////////////////////////////////////////////////////////
	private:
      
	  class Packages;
      
	  Packages & _packages;
      
	  std::set<std::string> _filerequires;
      
	public:
      
	  /**
	   * @return Whether the list of installed packages is valid, or
	   * you'd better reread it. (<B>NOTE:</B> returns valid, if not
	   * initialized).
	   **/
	  bool packagesValid() const;
      
	  /**
	   * If necessary build, and return the list of all installed packages.
	   **/
	  const std::list<Package::Ptr> & getPackages();
      
     #warning uncomment
#if 0 
	  /**
	   * Hack to lookup required and conflicting file relations.
	   **/
	  void traceFileRel( const PkgRelation & rel_r );
#endif
      
	  ///////////////////////////////////////////////////////////////////
	  //
	  // Direct RPM database retrieval via librpm.
	  //
	  ///////////////////////////////////////////////////////////////////
	public:

	  /**
	   * return complete file list for installed package name_r (in FileInfo.filename)
	   * if edition_r != Edition::noedition, check for exact edition
	   * if full==true, fill all attributes of FileInfo
	   **/
	  std::list<FileInfo> fileList( const std::string & name_r, const Edition & edition_r ) const;

	  /**
	   * Return true if at least one package owns a certain file (name_r empty)
	   * Return true if package name_r owns file file_r (name_r nonempty).
	   **/
	  bool hasFile( const std::string & file_r, const std::string & name_r = "" ) const;
      
	  /**
	   * Return name of package owning file
	   * or empty string if no installed package owns file
	   **/
	  std::string whoOwnsFile( const std::string & file_r ) const;
      
	  /**
	   * Return true if at least one package provides a certain tag.
	   **/
	  bool hasProvides( const std::string & tag_r ) const;
      
	  /**
	   * Return true if at least one package requires a certain tag.
	   **/
	  bool hasRequiredBy( const std::string & tag_r ) const;
      
	  /**
	   * Return true if at least one package conflicts with a certain tag.
	   **/
	  bool hasConflicts( const std::string & tag_r ) const;
      
	  /**
	   * Return true if package is installed.
	   **/
	  bool hasPackage( const std::string & name_r ) const;
      
	  /**
	   * Get an installed packages data from rpmdb. Package is
	   * identified by name. Data returned via result are NULL,
	   * if packge is not installed (PMError is not set), or RPM database
	   * could not be read (PMError is set).
	   *
	   * \throws RpmException
	   *
	   * FIXME this and following comment
	   *
	   **/
	  void getData( const std::string & name_r,
      		     RpmHeader::constPtr & result_r ) const;
      
	  /**
	   * Get an installed packages data from rpmdb. Package is
	   * identified by name and edition. Data returned via result are NULL,
	   * if packge is not installed (PMError is not set), or RPM database
	   * could not be read (PMError is set).
	   *
	   * \throws RpmException
	   *
	   **/
	  void getData( const std::string & name_r, const Edition & ed_r,
      		     RpmHeader::constPtr & result_r ) const;
      

	  /**
	   * Create a package from RpmHeader
	   * return NULL on error
	   **/

	  static Package::Ptr makePackageFromHeader( const RpmHeader::constPtr header, std::set<std::string> * filerequires, const Pathname & location, Source_Ref source );

	  ///////////////////////////////////////////////////////////////////
	  //
	  ///////////////////////////////////////////////////////////////////
	private:
          /**
           * iterates through zypp keyring and import all non existant keys
           * into rpm keyring
           */
          void importZyppKeyRingTrustedKeys();
          /**
           * insert all rpm trusted keys into zypp trusted keyring
           */
          void exportTrustedKeysInZyppKeyRing();
          
	  /**
	   * The connection to the rpm process.
	  */
	  ExternalProgram *process;
      
	  typedef std::vector<const char*> RpmArgVec;
      
	  /**
	   * Run rpm with the specified arguments and handle stderr.
	   * @param n_opts The number of arguments
	   * @param options Array of the arguments, @ref n_opts elements
	   * @param stderr_disp How to handle stderr, merged with stdout by default
	   *
	   * \throws RpmException
	   *
	   **/
	  void run_rpm( const RpmArgVec& options,
      		  ExternalProgram::Stderr_Disposition stderr_disp =
      		  ExternalProgram::Stderr_To_Stdout);
      
      
	  /**
	   * Read a line from the general rpm query
	  */
	  bool systemReadLine(std::string &line);
      
	  /**
	   * Return the exit status of the general rpm process,
	   * closing the connection if not already done.
	  */
	  int systemStatus();
      
	  /**
	   * Forcably kill the system process
	  */
	  void systemKill();
      
	  /**
	   * The exit code of the rpm process, or -1 if not yet known.
	  */
	  int exit_code;
      
	  /** /var/adm/backup */
	  Pathname _backuppath;
      
	  /** create package backups? */
	  bool _packagebackups;
      
	  /** whether <_root>/<WARNINGMAILPATH> was already created */
	  bool _warndirexists;
      
	  /**
	   * handle rpm messages like "/etc/testrc saved as /etc/testrc.rpmorig"
	   *
	   * @param line rpm output starting with warning:
	   * @param name name of package, appears in subject line
	   * @param typemsg " saved as " or " created as "
	   * @param difffailmsg what to put into mail if diff failed, must contain two %s for the two files
	   * @param diffgenmsg what to put into mail if diff succeeded, must contain two %s for the two files
	   * */
	  void processConfigFiles(const std::string& line,
      			     const std::string& name,
      			     const char* typemsg,
      			     const char* difffailmsg,
      			     const char* diffgenmsg);
      
      
	public:
      
	  typedef std::set<std::string> FileList;
      
	  /**
	   * Bits representing rpm installation options, useable as or
	   * combination
	   *
	   * @see installPackage(), removePackage()
	   * */
	  enum RpmInstFlag
	    {
	      RPMINST_NONE       = 0x0000,
	      RPMINST_NODOCS     = 0x0001,
	      RPMINST_NOSCRIPTS  = 0x0002,
	      RPMINST_FORCE      = 0x0004,
	      RPMINST_NODEPS     = 0x0008,
	      RPMINST_IGNORESIZE = 0x0010,
	      RPMINST_JUSTDB     = 0x0020,
	      RPMINST_NODIGEST   = 0x0040,
	      RPMINST_NOSIGNATURE= 0x0080,
	      RPMINST_NOUPGRADE  = 0x0100,
	      RPMINST_TEST	 = 0x0200
	    };
      
	  /**
	   * Bits of possible package corruptions
	   * @see checkPackage
	   * @see checkPackageResult2string
	   * */
	  enum checkPackageResult
	    {
	    CHK_OK                = 0x00,
	    CHK_INCORRECT_VERSION = 0x01, // package does not contain expected version
	    CHK_INCORRECT_FILEMD5 = 0x02, // md5sum of file is wrong (outside)
	    CHK_GPGSIG_MISSING    = 0x04, // package is not signeed
	    CHK_MD5SUM_MISSING    = 0x08, // package is not signeed
	    CHK_INCORRECT_GPGSIG  = 0x10, // signature incorrect
	    CHK_INCORRECT_PKGMD5  = 0x20, // md5sum incorrect (inside)
	    CHK_OTHER_FAILURE     = 0x40  // rpm failed for some reason
	    };
      
      
	  /**
	   * Check rpm with rpm --checksig
	   *
	   * @param filename which file to check
	   * @param version check if package really contains this version, leave emtpy to skip check
	   * @param md5 md5sum for whole file, leave empty to skip check (not yet implemented)
	   *
	   * @return checkPackageResult
	  */
	  unsigned checkPackage (const Pathname& filename, std::string version = "", std::string md5 = "" );
      
	  /** install rpm package
	   *
	   * @param filename file to install
	   * @param flags which rpm options to use
	   *
	   * @return success
	   *
	   * \throws RpmException
	   *
	   * */
	  void installPackage (const Pathname& filename, unsigned flags = 0 );
      
	  /** remove rpm package
	   *
	   * @param name_r Name of the rpm package to remove.
	   * @param iflags which rpm options to use
	   *
	   * @return success
	   *
	   * \throws RpmException
	   *
	   * */
	  void removePackage(const std::string & name_r, unsigned flags = 0);
	  void removePackage(Package::constPtr package, unsigned flags = 0);
      
	  /**
	   * get backup dir for rpm config files
	   *
	   * */
	  Pathname getBackupPath (void) { return _backuppath; }
      
	  /**
	   * create tar.gz of all changed files in a Package
	   *
	   * @param packageName name of the Package to backup
	   *
	   * @see setBackupPath
	   * */
	  bool backupPackage(const std::string& packageName);
      
	  /**
	   * queries file for name and then calls above backupPackage
	   * function. For convenience.
	   *
	   * @param filename rpm file that is about to be installed
	   * */
	  bool backupPackage(const Pathname& filename);
      
	  /**
	   * set path where package backups are stored
	   *
	   * @see backupPackage
	   * */
	  void setBackupPath(const Pathname& path);
      
	  /**
	   * whether to create package backups during install or
	   * removal
	   *
	   * @param yes true or false
	   * */
	  void createPackageBackups(bool yes) { _packagebackups = yes; }
      
	  /**
	   * determine which files of an installed package have been
	   * modified.
	   *
	   * @param fileList (output) where to store modified files
	   * @param packageName name of package to query
	   *
	   * @return false if package couln't be queried for some
	   * reason
	   * */
	  bool queryChangedFiles(FileList & fileList, const std::string& packageName);
      
	public: // static members
      
	  /** create error description of bits set according to
	   * checkPackageResult
	   * */
	  static std::string checkPackageResult2string(unsigned code);
      
	public:
      
	  /**
	   * Dump debug info.
	   **/
	  virtual std::ostream & dumpOn( std::ostream & str ) const;
      
	  ///////////////////////////////////////////////////////////////////
	  //
	  // Installation log
	  //
	///////////////////////////////////////////////////////////////////
	private:
      
	  /**
	   * Progress of installation may be logged to file
	   **/
	  class Logfile;
      
	public:
      
	  /**
	   * Set logfile for progress log. Empty filename to disable logging.
	   **/
	  static bool setInstallationLogfile( const Pathname & filename );

	protected:
	  void doRemovePackage( const std::string & name_r, unsigned flags, callback::SendReport<RpmRemoveReport> & report );
	  void doInstallPackage( const Pathname & filename, unsigned flags, callback::SendReport<RpmInstallReport> & report );
	  const std::list<Package::Ptr> & doGetPackages(callback::SendReport<ScanDBReport> & report);
	  void doRebuildDatabase(callback::SendReport<RebuildDBReport> & report);
	  
      
      };

    } // namespace rpm
  } // namespace target
} // namespace zypp

#endif // ZYPP_TARGET_RPM_RPMDB_H
