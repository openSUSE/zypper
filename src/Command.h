/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPERCOMMAND_H_
#define ZYPPERCOMMAND_H_

//#include<iosfwd>
#include<string>

/**
 * Enumeration of <b>zypper</b> commands with mapping of command aliases.
 * The mapping includes <b>rug</b> equivalents as well.
 */
struct ZypperCommand
{
  /** Special void command value meaning <b>not set</b> */
  static const ZypperCommand NONE;

  static const ZypperCommand ADD_SERVICE;
  static const ZypperCommand REMOVE_SERVICE;
  static const ZypperCommand MODIFY_SERVICE;
  static const ZypperCommand LIST_SERVICES;
  static const ZypperCommand REFRESH_SERVICES;

  static const ZypperCommand ADD_REPO;
  static const ZypperCommand REMOVE_REPO;
  static const ZypperCommand RENAME_REPO;
  static const ZypperCommand MODIFY_REPO;
  static const ZypperCommand LIST_REPOS;
  static const ZypperCommand REFRESH;
  static const ZypperCommand CLEAN;

  static const ZypperCommand INSTALL;
  static const ZypperCommand REMOVE;
  static const ZypperCommand SRC_INSTALL;
  static const ZypperCommand VERIFY;
  static const ZypperCommand INSTALL_NEW_RECOMMENDS;

  static const ZypperCommand UPDATE;
  static const ZypperCommand LIST_UPDATES;
  static const ZypperCommand PATCH;
  static const ZypperCommand LIST_PATCHES;
  static const ZypperCommand PATCH_CHECK;
  static const ZypperCommand DIST_UPGRADE;

  static const ZypperCommand SEARCH;
  static const ZypperCommand INFO;
  static const ZypperCommand PACKAGES;
  static const ZypperCommand PATCHES;
  static const ZypperCommand PATTERNS;
  static const ZypperCommand PRODUCTS;

  static const ZypperCommand WHAT_PROVIDES;
  //static const ZypperCommand WHAT_REQUIRES;
  //static const ZypperCommand WHAT_CONFLICTS;

  static const ZypperCommand ADD_LOCK;
  static const ZypperCommand REMOVE_LOCK;
  static const ZypperCommand LIST_LOCKS;
  static const ZypperCommand CLEAN_LOCKS;

  // utils/others
  static const ZypperCommand TARGET_OS;
  static const ZypperCommand VERSION_CMP;
  static const ZypperCommand LICENSES;
  static const ZypperCommand PS;
  static const ZypperCommand DOWNLOAD;
  static const ZypperCommand SOURCE_DOWNLOAD;

  static const ZypperCommand HELP;
  static const ZypperCommand SHELL;
  static const ZypperCommand SHELL_QUIT;
  static const ZypperCommand MOO;

  //!@{
  static const ZypperCommand RUG_PATCH_INFO;
  static const ZypperCommand RUG_PATTERN_INFO;
  static const ZypperCommand RUG_PRODUCT_INFO;
  static const ZypperCommand RUG_SERVICE_TYPES;
  static const ZypperCommand RUG_LIST_RESOLVABLES;
  static const ZypperCommand RUG_MOUNT;
  //static const ZypperCommand RUG_INFO_PROVIDES;
  //static const ZypperCommand RUG_INFO_CONFLICTS;
  //static const ZypperCommand RUG_INFO_OBSOLETES;
  //static const ZypperCommand RUG_INFO_REQUIREMENTS;
  static const ZypperCommand RUG_PATCH_SEARCH;
  static const ZypperCommand RUG_PING;
  //!@}

  enum Command
  {
    NONE_e,

    ADD_SERVICE_e,
    REMOVE_SERVICE_e,
    MODIFY_SERVICE_e,
    LIST_SERVICES_e,
    REFRESH_SERVICES_e,

    ADD_REPO_e,
    REMOVE_REPO_e,
    RENAME_REPO_e,
    MODIFY_REPO_e,
    LIST_REPOS_e,
    REFRESH_e,
    CLEAN_e,

    INSTALL_e,
    REMOVE_e,
    SRC_INSTALL_e,
    VERIFY_e,
    INSTALL_NEW_RECOMMENDS_e,

    UPDATE_e,
    LIST_UPDATES_e,
    PATCH_e,
    LIST_PATCHES_e,
    PATCH_CHECK_e,
    DIST_UPGRADE_e,

    SEARCH_e,
    INFO_e,
    PACKAGES_e,
    PATCHES_e,
    PATTERNS_e,
    PRODUCTS_e,

    WHAT_PROVIDES_e,
    //WHAT_REQUIRES_e,
    //WHAT_CONFLICTS_e,

    ADD_LOCK_e,
    REMOVE_LOCK_e,
    LIST_LOCKS_e,
    CLEAN_LOCKS_e,

    TARGET_OS_e,
    VERSION_CMP_e,
    LICENSES_e,
    PS_e,
    DOWNLOAD_e,
    SOURCE_DOWNLOAD_e,

    HELP_e,
    SHELL_e,
    SHELL_QUIT_e,
    MOO_e,

    RUG_PATCH_INFO_e,
    RUG_PATTERN_INFO_e,
    RUG_PRODUCT_INFO_e,
    RUG_SERVICE_TYPES_e,
    RUG_LIST_RESOLVABLES_e,
    RUG_MOUNT_e,
    //RUG_INFO_PROVIDES_e,
    //RUG_INFO_CONFLICTS_e,
    //RUG_INFO_OBSOLETES_e,
    //RUG_INFO_REQUIREMENTS_e,
    RUG_PATCH_SEARCH_e,
    RUG_PING_e
  };

  ZypperCommand(Command command) : _command(command) {}

  explicit ZypperCommand(const std::string & strval_r);

  Command toEnum() const { return _command; }

  ZypperCommand::Command parse(const std::string & strval_r) const;

  const std::string & asString() const;

  Command _command;
};

inline std::ostream & operator<<( std::ostream & str, const ZypperCommand & obj )
{ return str << obj.asString(); }

inline bool operator==(const ZypperCommand & obj1, const ZypperCommand & obj2)
{ return obj1._command == obj2._command; }

inline bool operator!=(const ZypperCommand & obj1, const ZypperCommand & obj2)
{ return obj1._command != obj2._command; }

// for use in std::set
inline bool operator<(const ZypperCommand & obj1, const ZypperCommand & obj2)
{ return obj1._command < obj2._command; }

#endif /*ZYPPERCOMMAND_H_*/
