/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_H
#define ZYPPER_UTILS_H

#include <ostream>
#include <string>
#include <set>

#include "zypp/Url.h"
#include "zypp/Pathname.h"

#include "zypp/Capability.h"
#include "zypp/ResKind.h"
#include "zypp/RepoInfo.h"
#include "zypp/ZYppCommitPolicy.h"

class Zypper;

namespace zypp
{
  class PoolItem;
  class Resolvable;
  class Product;
  class Pattern;
}


typedef std::set<zypp::ResKind> ResKindSet;

std::string readline_getline();

/**
 * Reads COLUMNS environment variable or gets the screen width from readline,
 * in that order. Falls back to 80 if all that fails.
 * \NOTE In case stdout is not connected to a terminal max. unsigned
 * is returned. This should prevent clipping when output is redirected.
 */
unsigned get_screen_width();

bool is_changeable_media(const zypp::Url & url);

std::string kind_to_string_localized(
    const zypp::ResKind & kind, unsigned long count);

std::string string_patch_status(const zypp::PoolItem & pi);

bool equalNVRA(const zypp::Resolvable & lhs, const zypp::Resolvable & rhs);

/**
 * Creates a Url out of \a urls_s. If the url_s looks looks_like_url()
 * Url(url_s) is returned. Otherwise if \a url_s represends a valid path to
 * a file or directory, a dir:// Url is returned. Otherwise an empty Url is
 * returned.
 */
zypp::Url make_url (const std::string & url_s);

/**
 * Returns <code>true</code> if the string \a s contains a substring starting
 * at the beginning and ending before the first colon matches any of registered
 * schemes (Url::isRegisteredScheme()).
 */
bool looks_like_url (const std::string& s);

/**
 * Returns <code>true</code> if \a s ends with ".rpm" or starts with "/", "./",
 * or "../".
 */
bool looks_like_rpm_file(const std::string & s);

/**
 * Download the RPM file specified by \a rpm_uri_str and copy it into
 * \a cache_dir.
 *
 * \return The local Pathname of the file in the cache on success, empty
 *      Pathname if a problem occurs.
 */
zypp::Pathname cache_rpm(const std::string & rpm_uri_str,
                         const std::string & cache_dir);

std::string xml_encode(const std::string & text);

std::string & indent(std::string & text, int columns);

zypp::Capability safe_parse_cap (Zypper & zypper,
                                 const std::string & capstr,
                                 const zypp::ResKind & kind = zypp::ResKind::nokind,
                                 const std::string & arch = "");


// comparator for RepoInfo set
class RepoInfoAliasComparator
{
  public: bool operator()(const zypp::RepoInfo & a, const zypp::RepoInfo & b)
  { return a.alias() < b.alias(); }
};


// comparator for Service set
class ServiceAliasComparator
{
  public: bool operator()(const zypp::repo::RepoInfoBase_Ptr & a,
                          const zypp::repo::RepoInfoBase_Ptr & b)
  { return a->alias() < b->alias(); }
};


/**
 * checks name for .repo string
 */
inline bool isRepoFile(const std::string& name)
{
  return name.find(".repo") != name.npos;
}

std::string asXML(const zypp::Product & p, bool is_installed);

std::string asXML(const zypp::Pattern & p, bool is_installed);

/**
 * fate #300763
 * Used by 'zypper ps' to show running processes that use
 * libraries or other files that have been removed since their execution.
 * This is particularly useful after 'zypper remove' or 'zypper update'.
 */
void list_processes_using_deleted_files(Zypper & zypper);

zypp::DownloadMode get_download_option(Zypper & zypper);

#endif /*ZYPPER_UTILS_H*/
