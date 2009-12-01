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

  static const ZypperCommand HELP;
  static const ZypperCommand SHELL;
  static const ZypperCommand SHELL_QUIT;
  static const ZypperCommand MOO;

  /** Special void command value meaning <b>not set</b> */
  static const ZypperCommand NONE;

  /** name rug commands */
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
    ADD_SERVICE_e                    = 1,
    REMOVE_SERVICE_e                 = 2,
    MODIFY_SERVICE_e                 = 3,
    LIST_SERVICES_e                  = 4,
    REFRESH_SERVICES_e               = 5,

    ADD_REPO_e                       = 10,
    REMOVE_REPO_e                    = 11,
    RENAME_REPO_e                    = 12,
    MODIFY_REPO_e                    = 13,
    LIST_REPOS_e                     = 14,
    REFRESH_e                        = 15,
    CLEAN_e                          = 16,

    INSTALL_e                        = 20,
    REMOVE_e                         = 21,
    SRC_INSTALL_e                    = 22,
    VERIFY_e                         = 23,
    INSTALL_NEW_RECOMMENDS_e         = 24,

    UPDATE_e                         = 30,
    LIST_UPDATES_e                   = 31,
    PATCH_e                          = 32,
    LIST_PATCHES_e                   = 33,
    PATCH_CHECK_e                    = 34,
    DIST_UPGRADE_e                   = 35,

    SEARCH_e                         = 40,
    INFO_e                           = 41,
    PACKAGES_e                       = 42,
    PATCHES_e                        = 43,
    PATTERNS_e                       = 44,
    PRODUCTS_e                       = 45,

    WHAT_PROVIDES_e                  = 50,
    //WHAT_REQUIRES_e,
    //WHAT_CONFLICTS_e,

    ADD_LOCK_e                       = 60,
    REMOVE_LOCK_e                    = 61,
    LIST_LOCKS_e                     = 62,
    CLEAN_LOCKS_e                    = 63,

    TARGET_OS_e                      = 80,
    VERSION_CMP_e                    = 81,
    LICENSES_e                       = 82,
    PS_e                             = 83,

    HELP_e                           = 90,
    SHELL_e                          = 91,
    SHELL_QUIT_e                     = 92,
    MOO_e                            = 93,

    NONE_e                           = 0,

    RUG_PATCH_INFO_e                 = 101,
    RUG_PATTERN_INFO_e               = 102,
    RUG_PRODUCT_INFO_e               = 103,
    RUG_SERVICE_TYPES_e              = 104,
    RUG_LIST_RESOLVABLES_e           = 105,
    RUG_MOUNT_e                      = 106,
    RUG_INFO_PROVIDES_e              = 107,
    RUG_INFO_CONFLICTS_e             = 108,
    RUG_INFO_OBSOLETES_e             = 109,
    RUG_INFO_REQUIREMENTS_e          = 110,
    RUG_PATCH_SEARCH_e               = 111,
    RUG_PING_e                       = 112
  };

  ZypperCommand(Command command) : _command(command) {}

  explicit ZypperCommand(const std::string & strval_r);

  Command toEnum() const { return _command; }

  ZypperCommand::Command parse(const std::string & strval_r);

  const std::string & asString() const;


  Command _command;
};
/*
inline std::ostream & operator<<( std::ostream & str, const ZypperCommand & obj )
{ return str << obj.asString(); }
*/
inline bool operator==(const ZypperCommand & obj1, const ZypperCommand & obj2)
{ return obj1._command == obj2._command; }

inline bool operator!=(const ZypperCommand & obj1, const ZypperCommand & obj2)
{ return obj1._command != obj2._command; }

// for use in std::set
inline bool operator<(const ZypperCommand & obj1, const ZypperCommand & obj2)
{ return obj1._command < obj2._command; }

#endif /*ZYPPERCOMMAND_H_*/
