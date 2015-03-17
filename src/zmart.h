/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_H
#define ZMART_H

#include <zypp/base/LogControl.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include <zypp/ResStore.h>
#include <zypp/base/String.h>
#include <zypp/Digest.h>
#include <zypp/CapFactory.h>

#define ZYPP_CHECKPATCHES_LOG "/var/log/YaST2/zypper.log"
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypper"
#define RANDOM_TOKEN "sad987432JJDJD948394DDDxxx22"

// ===== exit codes ======

#define ZYPPER_EXIT_OK                     0
// errors
#define ZYPPER_EXIT_ERR_BUG                1 // undetermined error
#define ZYPPER_EXIT_ERR_SYNTAX             2 // syntax error, e.g. zypper instal, zypper in --unknown option
#define ZYPPER_EXIT_ERR_INVALID_ARGS       3 // invalid arguments given, e.g. zypper source-add httttps://asdf.net 
#define ZYPPER_EXIT_ERR_ZYPP               4 // error indicated from within libzypp, e.g. God = zypp::getZYpp() threw an exception
#define ZYPPER_EXIT_ERR_PRIVILEGES         5 // unsufficient privileges for the operation
// info
#define ZYPPER_EXIT_INF_UPDATE_NEEDED      100 // update needed
#define ZYPPER_EXIT_INF_SEC_UPDATE_NEEDED  101 // security update needed
#define ZYPPER_EXIT_INF_REBOOT_NEEDED      102 // reboot needed after install/upgrade 
#define ZYPPER_EXIT_INF_RESTART_NEEDED     103 // restart of package manager itself needed
#define ZYPPER_EXIT_INF_CAP_NOT_FOUND      104 // given capability not found (for install/remove)

struct Settings
{
  Settings()
  : previous_token(RANDOM_TOKEN),
  verbose(0),  
  previous_code(-1),
  disable_system_sources(false),
  disable_system_resolvables(false),
  is_rug_compatible(false),
  non_interactive(false),
  no_gpg_checks(false),
  license_auto_agree(false),
  downloadOnly(false),
  root_dir("/")
  {}

  std::list<zypp::Url> additional_sources;
  std::string previous_token;
  int verbose;
  int previous_code;
  std::string command;
  bool disable_system_sources;
  bool disable_system_resolvables;
  bool is_rug_compatible;
  bool non_interactive;
  bool no_gpg_checks;
  bool license_auto_agree;
  bool downloadOnly;
  std::string root_dir;
};

struct Error
{
  Error( const std::string &desc )
  : description(desc)
  {}
  std::string description;
};

struct RuntimeData
{
  RuntimeData()
  : patches_count(0),
  security_patches_count(0)
  {}
    
  std::list<Error> errors;
  std::list<zypp::Source_Ref> sources;
  int patches_count;
  int security_patches_count;
  std::vector<std::string> packages_to_install; 
  std::vector<std::string> packages_to_uninstall; 
};

extern RuntimeData gData;
extern Settings gSettings;
extern std::ostream no_stream;

#define COND_STREAM(STREAM,LEVEL) ((gSettings.verbose >= LEVEL)? STREAM: no_stream)
#define cerr_v COND_STREAM(cerr,1)
#define cout_v COND_STREAM(cout,1)
#define cerr_vv COND_STREAM(cerr,2)
#define cout_vv COND_STREAM(cout,2)

// undefine _ macro from libzypp
#ifdef _
#undef _
#endif

// define new _ macro
#define _(MSG) ::gettext(MSG)

// Local Variables:
// c-basic-offset: 2
// End:
#endif
