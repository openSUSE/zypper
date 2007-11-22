#ifndef ZYPPERMAIN_H_
#define ZYPPERMAIN_H_

#include <libintl.h>

// ===== exit codes ======

#define ZYPPER_EXIT_OK                     0
// errors
#define ZYPPER_EXIT_ERR_BUG                1 // undetermined error
#define ZYPPER_EXIT_ERR_SYNTAX             2 // syntax error, e.g. zypper instal, zypper in --unknown option
#define ZYPPER_EXIT_ERR_INVALID_ARGS       3 // invalid arguments given, e.g. zypper source-add httttps://asdf.net 
#define ZYPPER_EXIT_ERR_ZYPP               4 // error indicated from within libzypp, e.g. God = zypp::getZYpp() threw an exception
#define ZYPPER_EXIT_ERR_PRIVILEGES         5 // unsufficient privileges for the operation
#define ZYPPER_EXIT_NO_REPOS               6 // no repositories defined

// info
#define ZYPPER_EXIT_INF_UPDATE_NEEDED      100 // update needed
#define ZYPPER_EXIT_INF_SEC_UPDATE_NEEDED  101 // security update needed
#define ZYPPER_EXIT_INF_REBOOT_NEEDED      102 // reboot needed after install/upgrade 
#define ZYPPER_EXIT_INF_RESTART_NEEDED     103 // restart of package manager itself needed


#define VERBOSITY_NORMAL 0
#define VERBOSITY_MEDIUM 1
#define VERBOSITY_HIGH 2

/**
 * Macro to filter output above the current verbosity level.
 *
 * \see Output Macros
 * \see Settings::verbosity
 */
#define COND_STREAM(STREAM,LEVEL) ((gSettings.verbosity >= LEVEL) ? STREAM : no_stream)

/** \name Output Macros
 * Alway use these macros to produce output so that the verbosity options
 * like -v or --quiet are respected. Use standard cout and cerr only in
 * cases where it is desirable to ignore them (e.g. help texts (when -h is
 * used) or brief error messages must always be displayed, even if --quiet
 * has been specified).
 */
//!@{
//! normal output
#define cout_n COND_STREAM(cout, VERBOSITY_NORMAL)
//! verbose output
#define cout_v COND_STREAM(cout, VERBOSITY_MEDIUM)
//! verbose error output
#define cerr_v COND_STREAM(cerr, VERBOSITY_MEDIUM)
//! debug info output
#define cout_vv COND_STREAM(cout, VERBOSITY_HIGH)
//! debug error output (details)
#define cerr_vv COND_STREAM(cerr, VERBOSITY_HIGH)
//!@}

// undefine _ and _PL macros from libzypp
#ifdef _
#undef _
#endif
#ifdef _PL
#undef _PL
#endif

// define new macros
#define _(MSG) ::gettext(MSG)
#define _PL(MSG1,MSG2,N) ::ngettext(MSG1,MSG2,N)

// libzypp logger settings
#define ZYPPER_LOG "/var/log/zypper.log"
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypper"

#endif /*ZYPPERMAIN_H_*/
