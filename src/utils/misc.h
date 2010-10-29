/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_H
#define ZYPPER_UTILS_H

#include <string>
#include <set>
#include <list>

#include "zypp/Url.h"
#include "zypp/Pathname.h"

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

bool is_changeable_media(const zypp::Url & url);

/** Converts user-supplied kind to zypp::ResKind object.
 * Returns an empty one if not recognized. */
zypp::ResKind string_to_kind(const std::string & skind);

ResKindSet kindset_from(const std::list<std::string> & kindstrings);

std::string kind_to_string_localized(
    const zypp::ResKind & kind, unsigned long count);

std::string string_patch_status(const zypp::PoolItem & pi);

/**
 * Creates a Url out of \a urls_s. If the url_s looks looks_like_url()
 * Url(url_s) is returned. Otherwise if \a url_s represends a valid path to
 * a file or directory, a dir:// Url is returned. Otherwise an empty Url is
 * returned.
 */
zypp::Url make_url (const std::string & url_s);

/**
 * Creates Url out of obs://project/platform URI with given base URL and default
 * platform (used in case the platform is not specified in the URI).
 */
zypp::Url make_obs_url(
    const std::string & obsuri,
    const zypp::Url & base_url,
    const std::string & default_platform = "openSUSE_Factory");

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


/**
 * Check whether one of --download-* or --download options was given and return
 * the specified mode.
 */
zypp::DownloadMode get_download_option(Zypper & zypper, bool quiet = false);

/** Check whether packagekit is running using a DBus call */
bool packagekit_running();

/** Send suggestion to quit to PackageKit via DBus */
void packagekit_suggest_quit();

#endif /*ZYPPER_UTILS_H*/
