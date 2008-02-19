#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/lexical_cast.hpp>

#include "zypp/ZYpp.h"
#include "zypp/base/Logger.h"
#include "zypp/base/IOStream.h"

#include "zypp/RepoManager.h"
#include "zypp/RepoInfo.h"
#include "zypp/repo/RepoException.h"
#include "zypp/parser/ParseException.h"
#include "zypp/media/MediaException.h"

#include "zypper.h"
#include "output/Out.h"
#include "zypper-main.h"
#include "zypper-getopt.h"
#include "zypper-tabulator.h"
#include "zypper-callbacks.h"
#include "zypper-utils.h"
#include "zypper-repos.h"

using namespace std;
using namespace boost;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::media;
using namespace zypp::parser;

extern ZYpp::Ptr God;
extern RuntimeData gData;

// ----------------------------------------------------------------------------

template <typename Target, typename Source>
void safe_lexical_cast (Source s, Target &tr) {
  try {
    tr = boost::lexical_cast<Target> (s);
  }
  catch (boost::bad_lexical_cast &) {
  }
}

// ----------------------------------------------------------------------------

static bool refresh_raw_metadata(Zypper & zypper,
                                 const RepoInfo & repo,
                                 bool force_download)
{
  gData.current_repo = repo;

  // reset the gData.current_repo when going out of scope
  struct Bye { ~Bye() { gData.current_repo = RepoInfo(); } } reset __attribute__ ((__unused__));

  try
  {
    RepoManager manager(zypper.globalOpts().rm_options);

    if (!force_download)
    {
      // check whether libzypp indicates a refresh is needed, and if so,
      // print a message
      zypper.out().info(boost::str(format(
          _("Checking whether to refresh metadata for %s")) % repo.name()),
          Out::HIGH);
      for(RepoInfo::urls_const_iterator it = repo.baseUrlsBegin();
          it != repo.baseUrlsEnd(); ++it)
      {
        try
        {
          if (manager.checkIfToRefreshMetadata(repo, *it))
          {
            ostringstream s;
            s << format(_("Refreshing '%s'")) % repo.name();
            if (zypper.command() == ZypperCommand::REFRESH &&
                zypper.cOpts().count("force"))
              s << " " << _("(forced)");
            zypper.out().info(s.str(), Out::HIGH);
          }
          else if (zypper.command() == ZypperCommand::REFRESH)
          {
            zypper.out().info(boost::str(
              format(_("Repository '%s' is up to date.")) % repo.name()));
          }
          break; // don't check all the urls, just the first succussfull.
        }
        catch (const Exception & e)
        {
          ZYPP_CAUGHT(e);
          ERR << *it << " doesn't look good. Trying another url." << endl;
        }
      }
    }
    else
      zypper.out().info(_("Forcing raw metadata refresh"));

    manager.refreshMetadata(repo, force_download ?
      RepoManager::RefreshForced : RepoManager::RefreshIfNeeded);
  }
  catch (const MediaException & e)
  {
    zypper.out().error(e,
        boost::str(format(_("Problem downloading files from '%s'.")) % repo.name()),
        _("Please see the above error message for a hint."));

    return true; // error
  }
  catch (const RepoNoUrlException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(boost::str(
      format(_("No URLs defined for '%s'.")) % repo.name()));
    if (!repo.filepath().empty())
      zypper.out().info(boost::str(format(
          // TranslatorExplanation the first %s is a .repo file path
          _("Please add one or more base URL (baseurl=URL) entries to %s for repository '%s'."))
          % repo.filepath() % repo.name()));

    return true; // error
  }
  catch (const RepoNoAliasException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(_("No alias defined for this repository."));
    report_a_bug(zypper.out());
    return true; // error
  }
  catch (const RepoException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
        boost::str(format(_("Repository '%s' is invalid.")) % repo.name()),
        _("Please check if the URLs defined for this repository are pointing to a valid repository."));

    return true; // error
  }
  catch (const Exception &e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
        boost::str(format(_("Error downloading metadata for '%s':")) % repo.name()));
    // log untranslated message
    ERR << format("Error reading repository '%s':") % repo.name() << endl;

    return true; // error
  }

  return false; // no error
}

// ---------------------------------------------------------------------------
/*
bool build_cache_callback(const ProgressData & pd)
{
  static AliveCursor cursor;
  if ( pd.val() == 100 )
    cout << CLEARLN << cursor.done() << " " << pd.name();
  else
    cout << CLEARLN << cursor++ << " " << pd.name();
  cout << " [" << pd.val() << "%] :O)";
  cout << flush;
  return true;
}
*/
static bool build_cache(Zypper & zypper, const RepoInfo &repo, bool force_build)
{
  if (force_build)
    zypper.out().info(_("Forcing building of repository cache"));

  try
  {
    RepoManager manager(zypper.globalOpts().rm_options);
    manager.buildCache(repo, force_build ?
      RepoManager::BuildForced : RepoManager::BuildIfNeeded);
  }
  catch (const parser::ParseException & e)
  {
    ZYPP_CAUGHT(e);

    zypper.out().error(e,
        boost::str(format(_("Error parsing metadata for '%s':")) % repo.name()),
        // TranslatorExplanation Don't translate the URL unless it is translated, too
        _("This may be caused by invalid metadata in the repository,"
          " or by a bug in the metadata parser. In the latter case,"
          " or if in doubt, please, file a bug report by following"
          " instructions at http://en.opensuse.org/Zypper#Troubleshooting"));

    // log untranslated message
    ERR << format("Error parsing metadata for '%s':") % repo.name() << endl;

    return true; // error
  }
  catch (const repo::RepoMetadataException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
        boost::str(format(_("Repository metadata for '%s' not found in local cache.")) % repo.name()));
    // this should not happend and is probably a bug, rethrowing
    ZYPP_RETHROW(e);
  }
  catch (const Exception &e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
        _("Error building the cache database:"));
    // log untranslated message
    ERR << "Error writing to cache db" << endl;

    return true; // error
  }

  return false; // no error
}

// ---------------------------------------------------------------------------

/**
 * Try to find RepoInfo counterparts among known repositories by alias, number,
 * or URI, based on the list of strings given as the iterator range \a being and
 * \a end. Matching RepoInfos will be added to \a repos and those with no match
 * will be added to \a not_found.
 */
template<typename T>
void get_repos(Zypper & zypper,
               const T & begin, const T & end,
               list<RepoInfo> & repos, list<string> & not_found)
{
  RepoManager manager(zypper.globalOpts().rm_options);
  list<RepoInfo> known = manager.knownRepositories();

  for (T it = begin; it != end; ++it)
  {
    RepoInfo repo;

    // can i find it by alias, #, or the url, in that order?
    unsigned int number = 1; // repo number
    for (list<RepoInfo>::const_iterator known_it = known.begin();
        known_it != known.end(); ++known_it, number++)
    {
      unsigned int tmp = 0;
      safe_lexical_cast (*it, tmp); // try to make an int out of the string

      try
      {
        if (known_it->alias() == *it ||
            tmp == number ||
            find(known_it->baseUrlsBegin(),known_it->baseUrlsEnd(),Url(*it))
              != known_it->baseUrlsEnd())
        {
          repo = *known_it;
          break;
        }
      }
      catch(const url::UrlException &){}
    } // END for all known repos

    // no match found
    if (repo.alias().empty())
    {
      not_found.push_back(*it);
      continue;
    }

    // repo found
    // is it a duplicate? compare by alias and URIs
    //! \todo operator== in RepoInfo? 
    bool duplicate = false;
    for (list<RepoInfo>::const_iterator repo_it = repos.begin();
        repo_it != repos.end(); ++repo_it)
    {
      bool equals = true;
      
      // alias
      if (repo_it->alias() != repo.alias())
        equals = false;

      if (equals)
      {
        // URIs (all of them)
        for (RepoInfo::urls_const_iterator urlit = repo_it->baseUrlsBegin();
            urlit != repo_it->baseUrlsEnd(); ++urlit)
          try {
            if (find(repo.baseUrlsBegin(),repo.baseUrlsEnd(),Url(*it))
                != repo.baseUrlsEnd())
              equals = false;
          }
          catch(const url::UrlException &){}
      }

      if (equals)
        duplicate = true;
    } // END for all found so far

    if (!duplicate)
      repos.push_back(repo);
  }
}

// ---------------------------------------------------------------------------

/**
 * Say "Repository %s not found" for all strings in \a not_found list.
 */
static void report_unknown_repos(Out & out, list<string> not_found)
{
  for (list<string>::iterator it = not_found.begin();
      it != not_found.end(); ++it)
    out.error(boost::str(format(
      _("Repository '%s' not found by its alias, number, or URI.")) % *it));

  if (!not_found.empty())
    out.info(_("Use 'zypper repos' to get the list of defined repositories."));
}

// ---------------------------------------------------------------------------

/**
 * Fill gData.repositories with active repos (enabled or specified) and refresh
 * if autorefresh is on.
 */
static void do_init_repos(Zypper & zypper)
{
  MIL << "Going to initialize repositories." << endl;

  // load gpg keys
  init_target(zypper);
  RepoManager manager(zypper.globalOpts().rm_options);

  // get repositories specified with --repo or --catalog
  list<string> not_found;
  parsed_opts::const_iterator it;
  if ((it = copts.find("repo")) != copts.end())
    get_repos(zypper, it->second.begin(), it->second.end(), gData.repos, not_found);
  // rug compatibility
  if ((it = copts.find("catalog")) != copts.end())
    get_repos(zypper, it->second.begin(), it->second.end(), gData.repos, not_found);
  if (!not_found.empty())
  {
    report_unknown_repos(zypper.out(), not_found);
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    return;
  }

  // if no repository was specified on the command line, use all known repos
  if (gData.repos.empty())
    gData.repos = manager.knownRepositories();

  // additional repositories (--plus-repo)
  if (!gData.additional_repos.empty())
  {
    for (list<RepoInfo>::iterator it = gData.additional_repos.begin();
        it != gData.additional_repos.end(); ++it)
    {
      add_repo(zypper, *it);
      gData.repos.push_back(*it);
    }
  }

  for (std::list<RepoInfo>::iterator it = gData.repos.begin();
       it !=  gData.repos.end(); ++it)
  {
    RepoInfo repo(*it);
    MIL << "checking if to refresh " << repo.name() << endl;

    bool do_refresh =
      repo.enabled() &&
      repo.autorefresh() &&
      !zypper.globalOpts().no_refresh;

    if (do_refresh)
    {
      MIL << "calling refresh for " << repo.name() << endl;

      // handle root user differently
      if (geteuid() == 0)
      {
        if (refresh_raw_metadata(zypper, repo, false)
            || build_cache(zypper, repo, false))
        {
          cerr << format(_("Disabling repository '%s' because of the above error."))
              % repo.name() << endl;
          ERR << format("Disabling repository '%s' because of the above error.")
              % repo.name() << endl;

          it->setEnabled(false);
        }
      }
      // non-root user
      else
      {
        try { manager.refreshMetadata(repo, RepoManager::RefreshIfNeeded); }
        // any exception thrown means zypp attempted to refresh the repo
        // i.e. it is out-of-date. Thus, just display refresh hint for non-root
        // user
        catch (const Exception & ex)
        {
          cout << format(_(
              "Repository '%s' is out-of-date. You can run 'zypper refresh'"
              " as root to update it.")) % repo.name()
            << endl;

          string nonroot =
            "We're running as non-root, skipping refresh of " + repo.name();
          MIL << nonroot << endl;
          cout_vv << nonroot << endl;
        }
      }
    }
    // even if refresh is not required, try to build the sqlite cache
    // for the case of non-existing cache
    else if (repo.enabled())
    {
      // handle root user differently
      if (geteuid() == 0)
      {
        if (build_cache(zypper, repo, false))
        {
          cerr << format(_("Disabling repository '%s' because of the above error."))
              % repo.name() << endl;
          ERR << format("Disabling repository '%s' because of the above error.")
              % repo.name() << endl;

          it->setEnabled(false);
        }
      }
      // non-root user
      else
      {
        // if error is returned, it means zypp attempted to build the sqlite
        // cache for the repo and failed because writing is not allowed for
        // non-root Thus, just display refresh hint for non-root user.
        if (build_cache(zypper, repo, false))
        {
          cout << format(_(
              "The cache database needs to be built for the '%s' repository."
              " You can run 'zypper refresh' as root to do this.")) % repo.name()
            << endl;

          string nonroot =
            "We're running as non-root, skipping building of " + repo.name() + "cache";
          MIL << nonroot << endl;
          cout_vv << nonroot << endl;

          cerr << format(_("Disabling repository '%s'."))
              % repo.name() << endl << endl;
          ERR << "Disabling repository '" << repo.name() << "'" << endl;
          it->setEnabled(false);
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------

void init_repos(Zypper & zypper)
{
  static bool done = false;
  //! \todo this has to be done so that it works in zypper shell
  if (done)
    return;

  if ( !zypper.globalOpts().disable_system_sources )
    do_init_repos(zypper);

  done = true;
}

// ----------------------------------------------------------------------------

void init_target (Zypper & zypper) {
  static bool done = false;
  //! \todo do this so that it works in zypper shell
  if (!done) {
    cout_v << _("Initializing Target") << endl;

    try
    {
      God->initializeTarget(zypper.globalOpts().root_dir);
    }
    catch (const Exception & e)
    {
      report_problem(e,
        _("Target initialization failed:"),
        geteuid() != 0 ?
          _("Running 'zypper refresh' as root might resolve the problem."):""
      );

      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      throw ExitRequestException(
        "Target initialization failed: " + e.msg());
    }

    done = true;
  }
}

// ----------------------------------------------------------------------------

static void print_rug_sources_list(const std::list<zypp::RepoInfo> &repos)
{
  Table tbl;

  // header
  TableHeader th;
  th << "#" << _("Status") << _("Type") << _("Name") << "URI";
  tbl << th;

  int i = 1;

  for (std::list<RepoInfo>::const_iterator it = repos.begin();
       it !=  repos.end(); ++it)
  {
    RepoInfo repo = *it;
    TableRow tr(5);

    // number
    tr << str::numstring(i);

    // status
    // rug's status (active, pending => active, disabled <= enabled, disabled)
    // this is probably the closest possible compatibility arrangement
    tr << (repo.enabled() ? _("Active") : _("Disabled"));

    // type
    tr << repo.type().asString();
    // name
    tr << repo.name();
    // url
    tr << (*repo.baseUrlsBegin()).asString();

    tbl << tr;
    i++;
  }

  cout << tbl;
}

// ----------------------------------------------------------------------------

static void print_repo_list(const Zypper & zypper,
                            const std::list<zypp::RepoInfo> &repos )
{
  Table tbl;

  // header
  TableHeader th;
  th << "#" << _("Enabled") << _("Refresh") << _("Type") << _("Alias") << _("Name");
  if (zypper.globalOpts().verbosity > VERBOSITY_NORMAL)
    th << "URI";
  tbl << th;

  int i = 1;

  for (std::list<RepoInfo>::const_iterator it = repos.begin();
       it !=  repos.end(); ++it)
  {
    RepoInfo repo = *it;
    TableRow tr (zypper.globalOpts().verbosity > VERBOSITY_NORMAL ? 6 : 7);

    // number
    tr << str::numstring (i);
    // enabled?
    tr << (repo.enabled() ? _("Yes") : _("No"));
    // autorefresh?
    tr << (repo.autorefresh() ? _("Yes") : _("No"));
    // type
    tr << repo.type().asString();
    // alias
    tr << repo.alias();
    // name
    tr << repo.name();
    // url
    if (zypper.globalOpts().verbosity > VERBOSITY_NORMAL)
      tr << (*repo.baseUrlsBegin()).asString(); //! \todo properly handle multiple baseurls

    tbl << tr;
    i++;
  }

  if (tbl.empty())
    cout_n << _("No repositories defined."
        " Use the 'zypper addrepo' command to add one or more repositories.")
         << endl;
  else
    cout << tbl;
}

// ----------------------------------------------------------------------------

void print_repos_to(const std::list<zypp::RepoInfo> &repos, ostream & out)
{
  for (std::list<RepoInfo>::const_iterator it = repos.begin();
       it !=  repos.end(); ++it)
  {
    it->dumpRepoOn(out);
    out << endl;
  }
}

// ----------------------------------------------------------------------------

void list_repos(Zypper & zypper)
{
  RepoManager manager(zypper.globalOpts().rm_options);
  list<RepoInfo> repos;

  try
  {
    repos = manager.knownRepositories();
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    cerr << _("Error reading repositories:") << endl
         << e.asUserString() << endl;
    exit(ZYPPER_EXIT_ERR_ZYPP);
  }

  // add the additional repos specified with the --plus-repo to the list
  if (!gData.additional_repos.empty())
    repos.insert(repos.end(),
        gData.additional_repos.begin(),
        gData.additional_repos.end());

  // export to file or stdout in repo file format
  if (copts.count("export"))
  {
    string filename_str = copts["export"].front();
    if (filename_str == "-")
    {
      print_repos_to(repos, cout);
    }
    else
    {
      if (filename_str.rfind(".repo") == string::npos)
        filename_str += ".repo";

      Pathname file(filename_str);
      std::ofstream stream(file.c_str());
      if (!stream)
      {
        cerr << format(_("Can't open %s for writing. Maybe you don't have write permissions?"))
            % file.asString() << endl;
        exit(ZYPPER_EXIT_ERR_INVALID_ARGS);
      }
      else
      {
        print_repos_to(repos, stream);
        cout << format(
            _("Repositories have been successfully exported to %s."))
            % (file.absolute() ? file.asString() : file.asString().substr(2))
          << endl;
      }
    }
  }
  // print repo list the rug's way
  else if (zypper.globalOpts().is_rug_compatible)
    print_rug_sources_list(repos);
  // print repo list as table
  else
    print_repo_list(zypper, repos);
}

// ----------------------------------------------------------------------------

void refresh_repos(Zypper & zypper)
{
  // need gpg keys when downloading (#304672)
  init_target(zypper);
  RepoManager manager(zypper.globalOpts().rm_options);

  list<RepoInfo> repos;
  try
  {
    repos = manager.knownRepositories();
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    report_problem(e,
        _("Error reading repositories:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }

  // get the list of repos specified on the command line ...
  list<RepoInfo> specified;
  list<string> not_found;
  // ...as command arguments
  get_repos(zypper, zypper.arguments().begin(), zypper.arguments().end(),
      specified, not_found);
  // ...as --repo options
  parsed_opts::const_iterator tmp1;
  if ((tmp1 = copts.find("repo")) != copts.end())
    get_repos(zypper, tmp1->second.begin(), tmp1->second.end(), specified, not_found);
  report_unknown_repos(zypper.out(), not_found);

  ostringstream s;
  s << _("Specified repositories: ");
  for (list<RepoInfo>::const_iterator it = specified.begin();
      it != specified.end(); ++it)
    s << it->alias() << " ";
  zypper.out().info(s.str(), Out::HIGH);

  unsigned error_count = 0;
  unsigned enabled_repo_count = repos.size();

  if (!specified.empty() || not_found.empty())
  {
    for (std::list<RepoInfo>::iterator it = repos.begin();
         it !=  repos.end(); ++it)
    {
      RepoInfo repo(*it);
  
      if (!specified.empty())
      {
        bool found = false;
        for (list<RepoInfo>::const_iterator it = specified.begin();
            it != specified.end(); ++it)
          if (it->alias() == repo.alias())
          {
            found = true;
            break;
          }
  
        if (!found)
        {
          DBG << repo.alias() << "(#" << ") not specified,"
              << " skipping." << endl;
          enabled_repo_count--;
          continue;
        }
      }
  
      // skip disabled repos
      if (!repo.enabled())
      {
        string msg = boost::str(
          format(_("Skipping disabled repository '%s'")) % repo.name());
  
        if (specified.empty())
          zypper.out().info(msg, Out::HIGH);
        else
          zypper.out().error(msg);
  
        enabled_repo_count--;
        continue;
      }
  
      // do the refresh
  
      // raw metadata refresh
      bool error = false;
      if (!copts.count("build-only"))
      {
        bool force_download =
          copts.count("force") || copts.count("force-download");
  
        // without this a cd is required to be present in the drive on each refresh
        // (or more 'refresh needed' check)
        bool is_cd = is_changeable_media(*repo.baseUrlsBegin());
        if (!force_download && is_cd)
        {
          MIL << "Skipping refresh of a changeable read-only media." << endl;
          continue;
        }
  
        MIL << "calling refreshMetadata" << (force_download ? ", forced" : "")
            << endl;
  
        error = refresh_raw_metadata(zypper, repo, force_download);
      }
  
      // db rebuild
      if (!(error || copts.count("download-only")))
      {
        bool force_build =
          copts.count("force") || copts.count("force-build");
  
        MIL << "calling buildCache" << (force_build ? ", forced" : "") << endl;
  
        error = build_cache(zypper, repo, force_build);
      }
  
      if (error)
      {
        zypper.out().error(boost::str(format(
          _("Skipping repository '%s' because of the above error."))
            % repo.name()));
        ERR << format("Skipping repository '%s' because of the above error.")
            % repo.name() << endl;
        error_count++;
      }
    }
  }
  else
    enabled_repo_count = 0;

  // print the result message
  if (enabled_repo_count == 0)
  {
    string hint =
      _("Use 'zypper addrepo' or 'zypper modifyrepo' commands to add or enable repositories.");
    if (!specified.empty() || !not_found.empty())
      zypper.out().error(_("Specified repositories are not enabled or defined."), hint);
    else
      zypper.out().error(_("There are no enabled repositories defined."), hint);
  }
  else if (error_count == enabled_repo_count)
  {
    zypper.out().error(_("Could not refresh the repositories because of errors."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  else if (error_count)
  {
    zypper.out().error(_("Some of the repositories have not been refreshed because of an error."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  else if (!specified.empty())
    zypper.out().info(_("Specified repositories have been refreshed."));
  else
    zypper.out().info(_("All repositories have been refreshed."));
}

// ----------------------------------------------------------------------------

void clean_repos(Zypper & zypper)
{
  RepoManager manager(zypper.globalOpts().rm_options);

  list<RepoInfo> repos;
  try
  {
    repos = manager.knownRepositories();
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    report_problem(e,
        _("Error reading repositories:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }

  // get the list of repos specified on the command line ...
  list<RepoInfo> specified;
  list<string> not_found;
  // ...as command arguments
  get_repos(zypper, zypper.arguments().begin(), zypper.arguments().end(),
      specified, not_found);
  // ...as --repo options
  parsed_opts::const_iterator tmp1;
  if ((tmp1 = copts.find("repo")) != copts.end())
    get_repos(zypper, tmp1->second.begin(), tmp1->second.end(), specified, not_found);
  report_unknown_repos(zypper.out(), not_found);

  cout_v << _("Specified repositories: ");
  for (list<RepoInfo>::const_iterator it = specified.begin();
      it != specified.end(); ++it)
    cout_v << it->alias() << " ";
  cout_v << endl;

  // should we clean packages or metadata ?
  bool clean_metadata = (copts.find("metadata") != copts.end());
  bool clean_packages = (copts.find("metadata") == copts.end());

  if( copts.find("all") != copts.end() )
  {
    clean_metadata = true;
    clean_packages = true;
  }
  
  cout_vv << "Metadata will be cleaned: " << clean_metadata << endl;
  cout_vv << "Packages will be cleaned: " << clean_packages << endl;

  unsigned error_count = 0;
  unsigned enabled_repo_count = repos.size();

  if (!specified.empty() || not_found.empty())
  {
    for (std::list<RepoInfo>::iterator it = repos.begin();
         it !=  repos.end(); ++it)
    {
      RepoInfo repo(*it);
  
      if (!specified.empty())
      {
        bool found = false;
        for (list<RepoInfo>::const_iterator it = specified.begin();
            it != specified.end(); ++it)
          if (it->alias() == repo.alias())
          {
            found = true;
            break;
          }
  
        if (!found)
        {
          DBG << repo.alias() << "(#" << ") not specified,"
              << " skipping." << endl;
          enabled_repo_count--;
          continue;
        }
      }
  
      bool error = false;
      try {
        if( clean_metadata )
	{
	    cout_v << "Cleaning metadata for '" << repo.alias () << "'" << endl;
	    manager.cleanMetadata(repo);
	}
        if( clean_packages )
	{
	    cout_v << "Cleaning packages for '" << repo.alias () << "'" << endl;
    	    manager.cleanPackages(repo);
	}
      } catch(...) {
	error = true;
      }

      if (error)
      {
        cerr << format(_("Skipping repository '%s' because of the above error."))
            % repo.name() << endl;
        ERR << format("Skipping repository '%s' because of the above error.")
            % repo.name() << endl;
        error_count++;
      }
    }
  }
  else
    enabled_repo_count = 0;

  if (error_count == enabled_repo_count)
  {
    cerr << _("Could not clean the repositories because of errors.") << endl;
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  else if (error_count)
  {
    cerr << _("Some of the repositories have not been cleaned up because of an error.") << endl;
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  else if (!specified.empty())
    cout << _("Specified repositories have been cleaned up.") << endl;
  else
    cout << _("All repositories have been cleaned up.") << endl;
}

// ----------------------------------------------------------------------------

static
std::string timestamp ()
{
  time_t t = time(NULL);
  struct tm * tmp = localtime(&t);

  if (tmp == NULL) {
    return "";
  }

  char outstr[50];
  if (strftime(outstr, sizeof(outstr), "%Y%m%d-%H%M%S", tmp) == 0) {
    return "";
  }
  return outstr;
}

// ----------------------------------------------------------------------------

void add_repo(Zypper & zypper, RepoInfo & repo)
{
  RepoManager manager(zypper.globalOpts().rm_options);

  bool is_cd = true;
  for(RepoInfo::urls_const_iterator it = repo.baseUrlsBegin();
      it != repo.baseUrlsEnd(); ++it)
  {
    is_cd = is_changeable_media(*it);
    if (!is_cd)
      break;
  }
  if (is_cd)
  {
    cout_v << _("This is a changeable read-only media (CD/DVD), disabling autorefresh.") << endl;
    repo.setAutorefresh(false);
  }


  MIL << "Going to add repository: " << repo << endl;

  try
  {
    gData.current_repo = repo;

    // reset the gData.current_repo when going out of scope
    struct Bye { ~Bye() { gData.current_repo = RepoInfo(); } } reset __attribute__ ((__unused__));

    manager.addRepository(repo);
  }
  catch (const RepoAlreadyExistsException & e)
  {
    ZYPP_CAUGHT(e);
    cerr << format(_("Repository named '%s' already exists. Please use another alias."))
        % repo.alias() << endl;
    ERR << "Repository named '" << repo.alias() << "' already exists." << endl;
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const RepoUnknownTypeException & e)
  {
    ZYPP_CAUGHT(e);
    cerr << _("Can't find a valid repository at given location:") << endl;
    cerr << _("Could not determine the type of the repository."
        " Please check if the defined URLs (see below) point to a valid repository:");
    for(RepoInfo::urls_const_iterator uit = repo.baseUrlsBegin();
        uit != repo.baseUrlsEnd(); ++uit)
      cerr << (*uit) << endl;
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const RepoException & e)
  {
    ZYPP_CAUGHT(e);
    report_problem(e,
        _("Problem transferring repository data from specified URL:"),
        is_cd ? "" : _("Please check whether the specified URL is accessible."));
    ERR << "Problem transferring repository data from specified URL" << endl;
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const Exception & e)
  {
    ZYPP_CAUGHT(e);
    report_problem(e, _("Unknown problem when adding repository:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_BUG);
    return;
  }

  cout << format(_("Repository '%s' successfully added")) % repo.name();
  cout_n << ":";
  cout << endl;

  if (zypper.globalOpts().is_rug_compatible)
  {
    cout_n << ( repo.enabled() ? "[x]" : "[ ]" );
    cout_n << ( repo.autorefresh() ? "* " : "  " );
    cout_n << repo.name() << " (" << *repo.baseUrlsBegin() << ")" << endl;
  }
  else
  {
    // TranslatorExplanation used as e.g. "Enabled: Yes"
    cout_n << _("Enabled") << ": " << (repo.enabled() ? _("Yes") : _("No")) << endl;
    // TranslatorExplanation used as e.g. "Autorefresh: Yes"
    cout_n << _("Autorefresh") << ": " << (repo.autorefresh() ? _("Yes") : _("No")) << endl;

    cout_n << "URL:";
    for (RepoInfo::urls_const_iterator uit = repo.baseUrlsBegin();
        uit != repo.baseUrlsEnd(); uit++)
      cout_n << " " << *uit;
    cout_n << endl;
  }

  MIL << "Repository successfully added: " << repo << endl;

  if(is_cd)
  {
    cout_n << format(_("Reading data from '%s' media")) % repo.name() << endl;
    bool error = refresh_raw_metadata(zypper, repo, false);
    if (!error)
      error = build_cache(zypper, repo, false);
    if (error)
    {
      cerr << format(_("Problem reading data from '%s' media")) % repo.name() << endl;
      cerr << _("Please check if your installation media is valid and readable.") << endl;
      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      return;
    }
  }
}

// ----------------------------------------------------------------------------

void add_repo_by_url( Zypper & zypper,
                     const zypp::Url & url, const string & alias,
                     const string & type,
                     tribool enabled, tribool autorefresh)
{
  MIL << "going to add repository by url (alias=" << alias << ", url=" << url
      << ")" << endl;

  RepoManager manager(zypper.globalOpts().rm_options);
  RepoInfo repo;

  if ( ! type.empty() )
    repo.setType(RepoType(type));

  repo.setAlias(alias.empty() ? timestamp() : alias);
  repo.addBaseUrl(url);

  if ( !indeterminate(enabled) )
    repo.setEnabled((enabled == true));
  if ( !indeterminate(autorefresh) )
    repo.setAutorefresh((autorefresh == true));

  add_repo(zypper, repo);
}

// ----------------------------------------------------------------------------

//! \todo handle zypp exceptions
void add_repo_from_file( Zypper & zypper,
                         const std::string & repo_file_url,
                         tribool enabled, tribool autorefresh)
{
  //! \todo handle local .repo files, validate the URL
  Url url = make_url(repo_file_url);
  if (!url.isValid())
  {
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    return;
  }

  RepoManager manager(zypper.globalOpts().rm_options);
  list<RepoInfo> repos;

  // read the repo file
  try { repos = readRepoFile(url); }
  catch (const media::MediaException & e)
  {
    ZYPP_CAUGHT(e);
    report_problem(e,
      _("Problem accessing the file at the specified URL") + string(":"),
      _("Please check if the URL is valid and accessible."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const parser::ParseException & e)
  {
    ZYPP_CAUGHT(e);
    report_problem(e,
      _("Problem parsing the file at the specified URL") + string(":"),
      // TranslatorExplanation don't translate the URL if the URL itself is not translated.
      // Also don't translate the '.repo' string.
      _("Is it a .repo file? See http://en.opensuse.org/Standards/RepoInfo for details."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const Exception & e)
  {
    ZYPP_CAUGHT(e);
    report_problem(e,
      _("Problem encountered while trying to read the file at the specified URL") + string(":"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }

  // add repos
  for (list<RepoInfo>::const_iterator it = repos.begin();
       it !=  repos.end(); ++it)
  {
    RepoInfo repo = *it;

    if(repo.alias().empty())
    {
      cerr << _("A repository with no alias defined found in the file, skipping.") << endl;
      continue;
    }

    if(repo.baseUrlsEmpty())
    {
      cerr << format(_("Repository '%s' has no URL defined, skipping.")) % repo.name() << endl;
      continue;
    }

    MIL << "enabled: " << enabled << " autorefresh: " << autorefresh << endl;
    if ( !indeterminate(enabled) )
      repo.setEnabled((enabled == true));
    if ( !indeterminate(autorefresh) )
      repo.setAutorefresh((autorefresh == true));
    MIL << "enabled: " << repo.enabled() << " autorefresh: " << repo.autorefresh() << endl;

    add_repo(zypper, repo);
  }

  return;
}

// ----------------------------------------------------------------------------

template<typename T>
ostream& operator << (ostream& s, const vector<T>& v) {
  std::copy (v.begin(), v.end(), ostream_iterator<T> (s, ", "));
  return s;
}

// ----------------------------------------------------------------------------

bool remove_repo(Zypper & zypper, const RepoInfo & repoinfo)
{
  RepoManager manager(zypper.globalOpts().rm_options);
  bool found = true;
  try
  {
    manager.removeRepository(repoinfo);
    cout << format(_("Repository '%s' has been removed.")) % repoinfo.name() << endl;
    MIL << format("Repository '%s' has been removed.") % repoinfo.name() << endl;
  }
  catch (const repo::RepoNotFoundException & ex)
  {
    found = false;
  }

  return found;
}


// ----------------------------------------------------------------------------

bool remove_repo(Zypper & zypper, const std::string &alias )
{
  list<RepoInfo> repos =
    RepoManager(zypper.globalOpts().rm_options).knownRepositories();
  for (list<RepoInfo>::const_iterator it = repos.begin();
      it != repos.end(); ++it)
    if (it->alias() == alias)
      return remove_repo(zypper, *it);

  return false;
}

bool remove_repo(Zypper & zypper, const Url & url, const url::ViewOption & urlview)
{
  RepoManager manager(zypper.globalOpts().rm_options);
  bool found = true;
  try
  {
    RepoInfo info = manager.getRepositoryInfo(url, urlview);
    found = remove_repo(zypper, info);
  }
  catch (const repo::RepoNotFoundException & ex)
  {
    found = false;
  }

  return found;
}

// ----------------------------------------------------------------------------

void rename_repo(Zypper & zypper,
                 const std::string & alias, const std::string & newalias)
{
  RepoManager manager(zypper.globalOpts().rm_options);

  try
  {
    RepoInfo repo(manager.getRepositoryInfo(alias));
    repo.setAlias(newalias);
    manager.modifyRepository(alias, repo);

    cout << format(_("Repository '%s' renamed to '%s'")) % alias % repo.alias() << endl;
    MIL << format("Repository '%s' renamed to '%s'") % alias % repo.alias() << endl;
  }
  catch (const RepoNotFoundException & ex)
  {
    cerr << format(_("Repository '%s' not found.")) % alias << endl;
    ERR << "Repo " << alias << " not found" << endl;
  }
  catch (const RepoAlreadyExistsException & ex)
  {
    cerr << format(_(
        "Repository named '%s' already exists. Please use another alias."))
      % newalias << endl;
  }
  catch (const Exception & ex)
  {
    cerr << _("Error while modifying the repository:") << endl;
    cerr << ex.asUserString() << endl;
    cerr << format(_("Leaving repository '%s' unchanged.")) % alias << endl;

    ERR << "Error while modifying the repository:" << ex.asUserString() << endl;
  }
}

// ----------------------------------------------------------------------------

void modify_repo(Zypper & zypper, const string & alias)
{
  // tell whether currenlty processed options are contradicting each other
  // bool contradiction = false;
  // TranslatorExplanation speaking of two mutually contradicting command line options
  string msg_contradition =
    _("%s used together with %s, which contradict each other."
      " This property will be left unchanged.");

  // enable/disable repo
  tribool enable = indeterminate;
  if (copts.count("enable"))
    enable = true;
  if (copts.count("disable"))
  {
    if (enable)
    {
      cerr << format(msg_contradition) % "--enable" % "--disable" << endl;

      enable = indeterminate;
    }
    else
      enable = false;
  }
  DBG << "enable = " << enable << endl;

  // autorefresh
  tribool autoref = indeterminate;
  if (copts.count("refresh") || copts.count("enable-autorefresh"))
    autoref = true;
  if (copts.count("no-refresh") || copts.count("disable-autorefresh"))
  {
    if (autoref)
    {
      cerr << format(msg_contradition)
        % "--refresh" % "--no-refresh" << endl;

      autoref = indeterminate;
    }
    else
      autoref = false;
  }
  DBG << "autoref = " << autoref << endl;

  try
  {
    RepoManager manager(zypper.globalOpts().rm_options);
    RepoInfo repo(manager.getRepositoryInfo(alias));

    if (!indeterminate(enable))
      repo.setEnabled(enable);

    if (!indeterminate(autoref))
      repo.setAutorefresh(autoref);

    manager.modifyRepository(alias, repo);

    cout << format(_("Repository %s has been sucessfully modified.")) % alias << endl;
    MIL << format("Repository %s modified:") % alias << repo << endl;
  }
  catch (const RepoNotFoundException & ex)
  {
    cerr << format(_("Repository %s not found.")) % alias << endl;
    ERR << "Repo " << alias << " not found" << endl;
  }
  catch (const Exception & ex)
  {
    cerr << _("Error while modifying the repository:") << endl;
    cerr << ex.asUserString();
    cerr << format(_("Leaving repository %s unchanged.")) % alias << endl;

    ERR << "Error while modifying the repository:" << ex.asUserString() << endl;
  }
}

// ---------------------------------------------------------------------------

void cond_load_resolvables(Zypper & zypper)
{
  static bool done = false;
  // don't call this fuction more than once for a single ZYpp instance
  // (e.g. in shell)
  if (done)
    return;

  MIL << "Going to load resolvables" << endl;

  load_repo_resolvables(zypper);
  if (!zypper.globalOpts().disable_system_resolvables)
    load_target_resolvables(zypper);

  done = true;
  MIL << "Done loading resolvables" << endl;
}

// ---------------------------------------------------------------------------

void load_repo_resolvables(Zypper & zypper)
{
  RepoManager manager(zypper.globalOpts().rm_options);

  for (std::list<RepoInfo>::iterator it = gData.repos.begin();
       it !=  gData.repos.end(); ++it)
  {
    RepoInfo repo(*it);
    MIL << "Loading " << repo.alias() << " resolvables." << endl;

    if (! it->enabled())
      continue;     // #217297

    try
    {
      bool error = false;
      // if there is no metadata locally
      if ( manager.metadataStatus(repo).empty() )
      {
        cout_v << format(_("Retrieving repository '%s' data..."))
                         % repo.name() << endl;
        error = refresh_raw_metadata(zypper, repo, false);
      }

      if (!error && !manager.isCached(repo))
      {
        cout_v << format(_("Repository '%s' not cached. Caching..."))
                         % repo.name() << endl;
        error = build_cache(zypper, repo, false);
      }

      if (error)
      {
        cerr << format(_("Problem loading data from '%s'")) % repo.name() << endl;
        cerr << format(_("Resolvables from '%s' not loaded because of error."))
            % repo.name() << endl;
        continue;
      }

      manager.loadFromCache(repo);
      //cout_v << " " << format(_("(%d resolvables found)")) % store.size() << endl;
    }
    catch (const Exception & e)
    {
      ZYPP_CAUGHT(e);
      report_problem(e,
          boost::str(format(_("Problem loading data from '%s'")) % repo.name()),
          _("Try 'zypper refresh', or even remove /var/cache/zypp/zypp.db before doing so."));
      cerr << format(_("Resolvables from '%s' not loaded because of error."))
                      % repo.name() << endl;
    }
  }
}

// ---------------------------------------------------------------------------

void load_target_resolvables(Zypper & zypper)
{
  if (!zypper.globalOpts().machine_readable)
    cout_n << _("Reading installed packages...");
  MIL << "Going to read RPM database" << endl;

  God->target()->load();

  if (!zypper.globalOpts().machine_readable)
  {
    //cout_v << "   " <<  format(_("(%s resolvables)")) % tgt_resolvables.size();
    cout_n << endl;
  }
  //DBG << tgt_resolvables.size() << " resolvables read";

  //if (to_pool)
  //  God->addResolvables(tgt_resolvables, true /*installed*/);
  //else
  //  gData.target_resolvables = tgt_resolvables;
}

// ---------------------------------------------------------------------------

/*
//! rename a source, identified in any way: alias, url, id
void rename_source( const std::string& anystring, const std::string& newalias )
{
  cerr_vv << "Constructing SourceManager" << endl;
  SourceManager_Ptr manager = SourceManager::sourceManager();
  cerr_vv << "Restoring SourceManager" << endl;
  manager->restore (gSettings.root_dir, true use_cache*//*);

  Source_Ref src;

  SourceManager::SourceId sid = 0;
  safe_lexical_cast (anystring, sid);
  if (sid > 0) {
    try {
      src = manager->findSource (sid);
    }
    catch (const Exception & ex) {
      ZYPP_CAUGHT (ex);
      // boost::format: %s is fine regardless of the actual type :-)
      cerr << format (_("Source %s not found.")) % sid << endl;
    }
  }
  else {
    bool is_url = false;
    if (looks_like_url (anystring)) {
	is_url = true;
	cerr_vv << "Looks like a URL" << endl;

	Url url;
	try {
	  url = Url (anystring);
	}
	catch ( const Exception & excpt_r ) {
	  ZYPP_CAUGHT( excpt_r );
	  cerr << _("URL is invalid: ") << excpt_r.asUserString() << endl;
	}
	if (url.isValid ()) {
	  try {
	    src = manager->findSourceByUrl (url);
	  }
	  catch (const Exception & ex) {
	    ZYPP_CAUGHT (ex);
	    cerr << format (_("Source %s not found.")) % url.asString() << endl;
	  }
	}
    }

    if (!is_url) {
      try {
	src = manager->findSource (anystring);
      }
      catch (const Exception & ex) {
	ZYPP_CAUGHT (ex);
	cerr << format (_("Source %s not found.")) % anystring << endl;
      }
    }
  }

  if (src) {
    // getting Source_Ref is useless if we only can use an id
    manager->renameSource (src.numericId (), newalias);
  }

  cerr_vv << "Storing source data" << endl;
  manager->store( gSettings.root_dir, true metadata_cache*//* );
}
*/
// ----------------------------------------------------------------------------

// #217028
void warn_if_zmd()
{
  if (system ("pgrep -lx zmd") == 0)
  { // list name, exact match
    cout_n << _("ZENworks Management Daemon is running.\n"
              "WARNING: this command will not synchronize changes.\n"
              "Use rug or yast2 for that.\n");
    USR << ("ZMD is running. Tell the user this will get"
        " ZMD and libzypp out of sync.") << endl;
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
