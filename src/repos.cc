/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <boost/logic/tribool.hpp>
#include <boost/lexical_cast.hpp>
#include <iterator>
#include <list>

#include <zypp/ZYpp.h>
#include <zypp/base/Logger.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/String.h>
#include <zypp/base/Flags.h>

#include <zypp/RepoManager.h>
#include <zypp/repo/RepoException.h>
#include <zypp/parser/ParseException.h>
#include <zypp/media/MediaException.h>
#include <zypp/media/MediaAccess.h>

#include "output/Out.h"
#include "main.h"
#include "getopt.h"
#include "Table.h"
#include "utils/messages.h"
#include "utils/misc.h"
#include "repos.h"

using namespace std;
using namespace boost;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::media;

extern ZYpp::Ptr God;

typedef list<RepoInfoBase_Ptr> ServiceList;

static bool refresh_service(Zypper & zypper, const ServiceInfo & service);

// ----------------------------------------------------------------------------

inline std::string volatileTag()
{
  // translators: used as 'XYZ changed to SOMETHING [volatile]' to tag specific property changes.
  return std::string( " [" + ColorString( ColorContext::MSG_WARNING, _("volatile") ).str() + "]" );
}

inline std::string volatileServiceRepoChange( const RepoInfo & repo_r )
{
  return boost::str(format(
    // translators: 'Volatile' refers to changes we previously tagged as 'XYZ changed to SOMETHING [volatile]'
    _("Repo '%1%' is managed by service '%2%'. Volatile changes are reset by the next service refresh!")
  ) % repo_r.alias() % repo_r.service() );
}


template <typename Target, typename Source>
void safe_lexical_cast (Source s, Target &tr) {
  try {
    tr = boost::lexical_cast<Target> (s);
  }
  catch (boost::bad_lexical_cast &) {
  }
}

unsigned parse_priority(Zypper & zypper)
{
  //! \todo use some preset priorities (high, medium, low, ...)
  unsigned ret = 0U;
  parsed_opts::const_iterator cArg = zypper.cOpts().find("priority");
  if ( cArg == zypper.cOpts().end() )
    return ret;     // 0: no --priority arg

  int prio = -1;
  std::string prio_str = *cArg->second.begin();
  safe_lexical_cast(prio_str, prio); // try to make an int out of the string

  if ( prio < 0 )
  {
    zypper.out().error(boost::str(format(
      _("Invalid priority '%s'. Use a positive integer number. The greater the number, the lower the priority."))
      % prio_str));
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    ZYPP_THROW(ExitRequestException("Invalid priority."));
  }

  ret = ( prio ? unsigned(prio) : RepoInfo::defaultPriority() );
  return ret;
}

// | Enabled | GPG Check |  Colored strings for enabled and GPG Check status
// +---------+-----------+
// | Yes     | (  ) No   |
// | Yes     | (rp) Yes  |
// | No      | ----      |
struct RepoGpgCheckStrings
{
  RepoGpgCheckStrings()
  : _tagColor( ColorContext::DEFAULT )
  {}

  RepoGpgCheckStrings( const ServiceInfo & service_r )
  {
    if ( service_r.enabled() )
    {
       _tagColor = ColorContext::DEFAULT;
      _enabledYN = ColorString( _tagColor, _("Yes") );
      _gpgCheckYN = ColorString( _tagColor, "----" );
    }
    else
    {
      _tagColor = ColorContext::LOWLIGHT;
      _enabledYN = ColorString( _tagColor, _("No") );
      _gpgCheckYN = ColorString( _tagColor, "----" );
    }
  }

  RepoGpgCheckStrings( const RepoInfo & repo_r )
  {
    if ( repo_r.enabled() )
    {
      bool gpgOK = false;
      std::string tagStr( "(  ) " );
      if ( repo_r.validRepoSignature() )	// is TriBool!
      {
	gpgOK = true;
	tagStr[1] = 'r';
      }
      if ( repo_r.pkgGpgCheck() )
      {
	gpgOK = true;
	tagStr[2] = 'p';
      }
      _tagColor = gpgOK ? ColorContext::DEFAULT : ColorContext::NEGATIVE;
      _enabledYN = ColorString( ColorContext::DEFAULT, _("Yes") );
      _gpgCheckYN = ColorString( _tagColor, tagStr+asYesNo(gpgOK) );
    }
    else
    {
      _tagColor = ColorContext::LOWLIGHT;
      _enabledYN = ColorString( _tagColor, _("No") );
      _gpgCheckYN = ColorString( _tagColor, "----" );
    }
  }
  ColorContext _tagColor;	///< color according to enabled and GPG Check status
  ColorString _enabledYN;	///< colored enabled Yes/No
  ColorString _gpgCheckYN;	///< colored GPG Check status if enabled else "----"
};

// ----------------------------------------------------------------------------

static bool refresh_raw_metadata(Zypper & zypper,
                                 const RepoInfo & repo,
                                 bool force_download)
{
  RuntimeData & gData = zypper.runtimeData();
  gData.current_repo = repo;
  bool do_refresh = false;
  string & plabel = zypper.runtimeData().raw_refresh_progress_label;

  // reset the gData.current_repo when going out of scope
  struct Bye { ~Bye() { Zypper::instance()->runtimeData().current_repo = RepoInfo(); } } reset __attribute__ ((__unused__));

  RepoManager & manager = zypper.repoManager();

  try
  {
    if (!force_download)
    {
      // check whether libzypp indicates a refresh is needed, and if so,
      // print a message
      zypper.out().info(boost::str(format( _("Checking whether to refresh metadata for %s")) % repo.asUserString()),
          Out::HIGH);
      if (!repo.baseUrlsEmpty())
      {
	// Suppress (interactive) media::MediaChangeReport if we in have multiple basurls (>1)
	media::ScopedDisableMediaChangeReport guard( repo.baseUrlsSize() > 1 );

        for(RepoInfo::urls_const_iterator it = repo.baseUrlsBegin();
            it != repo.baseUrlsEnd();)
        {
          try
          {
            RepoManager::RefreshCheckStatus stat = manager.
                checkIfToRefreshMetadata(repo, *it,
                  zypper.command() == ZypperCommand::REFRESH ||
                  zypper.command() == ZypperCommand::REFRESH_SERVICES ?
                    RepoManager::RefreshIfNeededIgnoreDelay :
                    RepoManager::RefreshIfNeeded);
            do_refresh = (stat == RepoManager::REFRESH_NEEDED);
            if (!do_refresh &&
                (zypper.command() == ZypperCommand::REFRESH ||
                 zypper.command() == ZypperCommand::REFRESH_SERVICES))
            {
              switch (stat)
              {
              case RepoManager::REPO_UP_TO_DATE:
	      {
		TermLine outstr( TermLine::SF_SPLIT | TermLine::SF_EXPAND );
		outstr.lhs << boost::str(format(_("Repository '%s' is up to date.")) % repo.asUserString());
		//outstr.rhs << repoGpgCheckStatus( repo );
		zypper.out().infoLine( outstr );
	      }
              break;
              case RepoManager::REPO_CHECK_DELAYED:
                zypper.out().info(boost::str(format(
		  _("The up-to-date check of '%s' has been delayed.")) % repo.asUserString()), Out::HIGH);
              break;
              default:
                WAR << "new item in enum, which is not covered" << endl;
              }
            }
            break; // don't check all the urls, just the first successfull.
          }
          catch (const Exception & e)
          {
            ZYPP_CAUGHT(e);
            Url badurl(*it);
            if (++it == repo.baseUrlsEnd())
              ZYPP_RETHROW(e);
            ERR << badurl << " doesn't look good. Trying another url ("
                << *it << ")." << endl;
          }
        }
      }
    }
    else
    {
      zypper.out().info(_("Forcing raw metadata refresh"));
      do_refresh = true;
    }

    if (do_refresh)
    {
      plabel = str::form(
          _("Retrieving repository '%s' metadata"), repo.asUserString().c_str());
      zypper.out().progressStart("raw-refresh", plabel, true);

      manager.refreshMetadata(repo,
        force_download ?
          RepoManager::RefreshForced :
            zypper.command() == ZypperCommand::REFRESH ||
            zypper.command() == ZypperCommand::REFRESH_SERVICES ?
              RepoManager::RefreshIfNeededIgnoreDelay :
              RepoManager::RefreshIfNeeded);

      //plabel += repoGpgCheckStatus( repo );
      zypper.out().progressEnd("raw-refresh", plabel);
      plabel.clear();
    }
  }
  catch (const AbortRequestException & e)
  {
    ZYPP_CAUGHT(e);
    // rethrow ABORT exception, stop executing the command
    ZYPP_RETHROW(e);
  }
  catch (const SkipRequestException & e)
  {
    ZYPP_CAUGHT(e);

    std::string question = boost::str( boost::format(_("Do you want to disable the repository %s permanently?")) % repo.name().c_str() );

    if ( read_bool_answer(PROMPT_YN_MEDIA_CHANGE, question, false) )
    {
      MIL << "Disabling repository " << repo.name().c_str() << " permanently." << endl;

      try
      {
        RepoInfo origRepo( manager.getRepositoryInfo(repo.alias()) );

        origRepo.setEnabled(false);
        manager.modifyRepository(repo.alias(), origRepo);
      }
      catch (const Exception & ex)
      {
        ZYPP_CAUGHT(ex);
        zypper.out().error( ex, boost::str(format(_("Error while disabling repository '%s'."))
                                           % repo.alias()));

        ERR << "Error while disabling the repository." << endl;
      }
    }
    // will disable repo in gData.repos
    return true;
  }
  catch (const MediaException & e)
  {
    ZYPP_CAUGHT(e);
    if (do_refresh)
    {
      zypper.out().progressEnd("raw-refresh", plabel, true);
      plabel.clear();
    }
    zypper.out().error(e,boost::str(format(
      _("Problem retrieving files from '%s'.")) % repo.asUserString()),
      _("Please see the above error message for a hint."));

    return true; // error
  }
  catch (const RepoNoUrlException & e)
  {
    ZYPP_CAUGHT(e);
    if (do_refresh)
    {
      zypper.out().progressEnd("raw-refresh", plabel, true);
      plabel.clear();
    }
    zypper.out().error(boost::str(format(
      _("No URIs defined for '%s'.")) % repo.asUserString()));
    if (!repo.filepath().empty())
      zypper.out().info(boost::str(format(
	// TranslatorExplanation the first %s is a .repo file path
	_("Please add one or more base URI (baseurl=URI) entries to %s for repository '%s'.")) % repo.filepath() % repo.asUserString()));

    return true; // error
  }
  catch (const RepoNoAliasException & e)
  {
    ZYPP_CAUGHT(e);
    if (do_refresh)
    {
      zypper.out().progressEnd("raw-refresh", plabel, true);
      plabel.clear();
    }
    zypper.out().error(_("No alias defined for this repository."));
    report_a_bug(zypper.out());
    return true; // error
  }
  catch (const RepoException & e)
  {
    ZYPP_CAUGHT(e);
    if (do_refresh)
    {
      zypper.out().progressEnd("raw-refresh", plabel, true);
      plabel.clear();
    }
    zypper.out().error(e, boost::str(format(
      _("Repository '%s' is invalid.")) % repo.asUserString()),
      _("Please check if the URIs defined for this repository are pointing to a valid repository."));

    return true; // error
  }
  catch (const Exception &e)
  {
    ZYPP_CAUGHT(e);
    if (do_refresh)
    {
      zypper.out().progressEnd("raw-refresh", plabel, true);
      plabel.clear();
    }
    zypper.out().error(e, boost::str(format(
      _("Error retrieving metadata for '%s':")) % repo.asUserString()));
    // log untranslated message
    ERR << format("Error reading repository '%s':") % repo.asUserString() << endl;

    return true; // error
  }

  return false; // no error
}

// ---------------------------------------------------------------------------

static bool build_cache(Zypper & zypper, const RepoInfo & repo, bool force_build)
{
  if (force_build)
    zypper.out().info(_("Forcing building of repository cache"));

  try
  {
    RepoManager & manager = zypper.repoManager();
    manager.buildCache(repo, force_build ?
      RepoManager::BuildForced : RepoManager::BuildIfNeeded);

    // Also load the solv file to check wheter it was created with the right
    // version of satsolver-tools. If there's a version mismatch or some other
    // problem, the solv file will be rebuilt even though the cookie files
    // indicate the solv file is up to date with raw metadata (bnc #456718)
    if (!force_build &&
        // only do this if the refresh commands are running
        // this function is also used when loading repos for other commands
        (zypper.command() == ZypperCommand::REFRESH
         || zypper.command() == ZypperCommand::REFRESH_SERVICES))
    {
      manager.loadFromCache(repo);
    }
  }
  catch (const parser::ParseException & e)
  {
    ZYPP_CAUGHT(e);

    zypper.out().error(e, boost::str(format(
      _("Error parsing metadata for '%s':")) % repo.asUserString()),
      // TranslatorExplanation Don't translate the URL unless it is translated, too
      _("This may be caused by invalid metadata in the repository,"
        " or by a bug in the metadata parser. In the latter case,"
        " or if in doubt, please, file a bug report by following"
        " instructions at http://en.opensuse.org/Zypper/Troubleshooting"));

    // log untranslated message
    ERR << format("Error parsing metadata for '%s':") % repo.alias() << endl;

    return true; // error
  }
  catch (const repo::RepoMetadataException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, boost::str(format(
      _("Repository metadata for '%s' not found in local cache.")) % repo.asUserString()));
    // this should not happend and is probably a bug, rethrowing
    ZYPP_RETHROW(e);
  }
  catch (const Exception &e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Error building the cache:"));
    // log untranslated message
    ERR << "Error writing to cache db" << endl;

    return true; // error
  }

  return false; // no error
}

// ---------------------------------------------------------------------------

bool match_repo(Zypper & zypper, string str, RepoInfo *repo)
{
  RepoManager & manager = zypper.repoManager();

  // Quick check for alias/reponumber/name first.
  // Name can be ambiguous, in which case the first match found will be returned
  {
    unsigned int number = 1; // repo number
    unsigned int tmp    = 0;
    safe_lexical_cast (str, tmp); // try to make an int out of the string
    for (RepoManager::RepoConstIterator known_it = manager.repoBegin();
         known_it != manager.repoEnd(); ++known_it, ++number)
    {
      if (known_it->alias() == str || tmp == number || known_it->name() == str)
      {
        if (repo)
          *repo = *known_it;

        return true;
      }
    }
  }

  // expensive URL analysis only if the above did not find anything.
  // URL can be ambiguous, in which case the first found match will be returned.
  bool found = false;
  try
  {
    Url strurl(str);	// no need to continue if str is no Url.

    for (RepoManager::RepoConstIterator known_it = manager.repoBegin();
	 known_it != manager.repoEnd(); ++known_it)
    {
      try
      {
	// first strip any trailing slash from the path in URLs before comparing
	// (bnc #585082)
	// we can afford this because we expect that the repo urls are directories
	// and it is common practice in servers and operating systems to accept
	// directory paths both with and without trailing slashes.
	Url uurl(strurl);
	uurl.setPathName(Pathname(uurl.getPathName()).asString());

	url::ViewOption urlview =
	url::ViewOption::DEFAULTS + url::ViewOption::WITH_PASSWORD;
	if (zypper.cOpts().count("loose-auth"))
	{
	  urlview = urlview
	  - url::ViewOptions::WITH_PASSWORD
	  - url::ViewOptions::WITH_USERNAME;
	}
	if (zypper.cOpts().count("loose-query"))
	  urlview = urlview - url::ViewOptions::WITH_QUERY_STR;

	// need to do asString(withurlview) comparison here because the user-given
	// string is expected to have no credentials or query
	if (!(urlview.has(url::ViewOptions::WITH_PASSWORD)
	  && urlview.has(url::ViewOptions::WITH_QUERY_STR)))
	{
	  if (!known_it->baseUrlsEmpty())
	  {
	    for_(urlit, known_it->baseUrlsBegin(), known_it->baseUrlsEnd())
	    {
	      Url newrl(*urlit);
	      newrl.setPathName(Pathname(newrl.getPathName()).asString());
	      if (newrl.asString(urlview) == uurl.asString(urlview))
	      {
		found = true;
		break;
	      }
	    }
	  }
	}
	// ordinary == comparison suffices here (quicker)
	else
	{
	  if (!known_it->baseUrlsEmpty())
	  {
	    for_(urlit, known_it->baseUrlsBegin(), known_it->baseUrlsEnd())
	    {
	      Url newrl(*urlit);
	      newrl.setPathName(Pathname(newrl.getPathName()).asString());
	      if (newrl == uurl)
	      {
		found = true;
		break;
	      }
	    }
	  }
	}

	if (found)
	{
	  if (repo)
	    *repo = *known_it;
	  break;
	}
      }
      catch(const url::UrlException &){}

    } // END for all known repos
  }
  catch(const url::UrlException &){}

  return found;
}

// ---------------------------------------------------------------------------

/** \return true if aliases are equal, and all lhs urls can be found in rhs */
static bool repo_cmp_alias_urls(const RepoInfo & lhs, const RepoInfo & rhs)
{
  bool equals = true;

  // alias
  if (lhs.alias() != rhs.alias())
  {
    equals = false;
  }
  else
  {
    // URIs (all of them must be in the other RepoInfo)
    if (!lhs.baseUrlsEmpty())
    {
      bool urlfound = false;
      for (RepoInfo::urls_const_iterator urlit = lhs.baseUrlsBegin();
          urlit != lhs.baseUrlsEnd(); ++urlit)
      {
        if (find(rhs.baseUrlsBegin(), rhs.baseUrlsEnd(), Url(*urlit))
            != rhs.baseUrlsEnd())
          urlfound = true;
        if (!urlfound)
        {
          equals = false;
          break;
        }
      }
    }
    else if (!rhs.baseUrlsEmpty())
      equals = false;
  }

  return equals;
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
  for (T it = begin; it != end; ++it)
  {
    RepoInfo repo;

    if (!match_repo(zypper, *it, &repo))
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
      if (repo_cmp_alias_urls(repo, *repo_it))
      {
        duplicate = true;
        break;
      }
    } // END for all found so far

    if (!duplicate)
      repos.push_back(repo);
  }
}

// Explicit instantiations required for other translation units:
template void get_repos(Zypper &, const list<string>::const_iterator &, const list<string>::const_iterator &, list<RepoInfo> &, list<string> & );

// ---------------------------------------------------------------------------

/**
 * Say "Repository %s not found" for all strings in \a not_found list.
 */
void report_unknown_repos(Out & out, list<string> not_found)
{
  for_(it, not_found.begin(), not_found.end())
    out.error(boost::str(format(
      _("Repository '%s' not found by its alias, number, or URI.")) % *it));

  if (!not_found.empty())
    out.info(str::form(
      _("Use '%s' to get the list of defined repositories."),
      "zypper repos"));
}

// ----------------------------------------------------------------------------

unsigned repo_specs_to_aliases(Zypper & zypper,
    const list<string> & rspecs, list<string> & aliases, bool enabled_only)
{
  list<string> not_found;
  list<RepoInfo> repos;
  get_repos(zypper, rspecs.begin(), rspecs.end(), repos, not_found);
  if (!not_found.empty())
  {
    report_unknown_repos(zypper.out(), not_found);
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    ZYPP_THROW(ExitRequestException("Unknown repo specified."));
  }
  for_(it, repos.begin(), repos.end())
  {
    if (!enabled_only || it->enabled())
      aliases.push_back(it->alias());
    else
      zypper.out().warning(str::form(_("Ignoring disabled repository '%s'"),
        it->asUserString().c_str()));
  }
  return aliases.size();
}

// ---------------------------------------------------------------------------

/**
 * Fill gData.repositories with active repos (enabled or specified) and refresh
 * if autorefresh is on.
 */

template <class Container>
void do_init_repos(Zypper & zypper, const Container & container)
{
  // load gpg keys & get target info
  // the target must be known before refreshing services so that repo manager
  // can ignore repos targetted for other systems
  init_target(zypper);

  if (geteuid() == 0 && !zypper.globalOpts().no_refresh)
  {
    MIL << "Refreshing autorefresh services." << endl;

    const list<ServiceInfo> & services = zypper.repoManager().knownServices();
    bool called_refresh = false;
    for_(s, services.begin(), services.end())
    {
      if (s->enabled() && s->autorefresh())
      {
        refresh_service(zypper, *s);
        called_refresh = true;
      }
    }
    // reinitialize the repo manager to re-read the list of repos
    if (called_refresh)
      zypper.initRepoManager();
  }

  MIL << "Going to initialize repositories." << endl;
  RepoManager & manager = zypper.repoManager();
  RuntimeData & gData = zypper.runtimeData();

  // get repositories specified with --repo or --catalog or in the container

  list<string> not_found;
  parsed_opts::const_iterator it;
  // --repo
  if ((it = copts.find("repo")) != copts.end())
    get_repos(zypper, it->second.begin(), it->second.end(), gData.repos, not_found);
  // --catalog - rug compatibility
  if ((it = copts.find("catalog")) != copts.end())
    get_repos(zypper, it->second.begin(), it->second.end(), gData.repos, not_found);
  // container
  if (!container.empty())
    get_repos(zypper, container.begin(), container.end(), gData.repos, not_found);
  if (!not_found.empty())
  {
    report_unknown_repos(zypper.out(), not_found);
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    return;
  }

  // if no repository was specified on the command line, use all known repos
  if (gData.repos.empty())
    gData.repos.insert(gData.repos.end(), manager.repoBegin(), manager.repoEnd());

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

  bool no_cd = zypper.globalOpts().no_cd;
  bool no_remote = zypper.globalOpts().no_remote;
  for (list<RepoInfo>::iterator it = gData.repos.begin(); it !=  gData.repos.end();)
  {
    if (no_cd && (it->url().getScheme() == "cd"
                  || it->url().getScheme() == "dvd"))
    {
      zypper.out().info(str::form(
          _("Ignoring repository '%s' because of '%s' option."),
          it->asUserString().c_str(), "no-cd"));
      gData.repos.erase(it++);
    }
    if (no_remote && it->url().schemeIsDownloading())
    {
      zypper.out().info(str::form(
          _("Ignoring repository '%s' because of '%s' option."),
          it->asUserString().c_str(), "no-remote"));
      gData.repos.erase(it++);
    }
    else
      ++it;
  }

  for (std::list<RepoInfo>::iterator it = gData.repos.begin();
       it !=  gData.repos.end(); ++it)
  {
    RepoInfo repo(*it);
    MIL << "checking if to refresh " << repo.alias() << endl;

    // disabled repos may get temp. enabled to check for --plus-content
    bool contentcheck = false;
    if ( ! ( gData.additional_content_repos.empty()
          || repo.url().schemeIsVolatile()
	  || repo.enabled() ) )
    {
      // Preliminarily enable if last content matches or no content info available.
      // Final check is done after refresh.
      if ( repo.hasContentAny( gData.additional_content_repos ) || ! repo.hasContent() )
      {
	contentcheck = true;
	repo.setEnabled( true );
	zypper.out().info( boost::format(_("Scanning content of disabled repository '%s'."))
			  % repo.asUserString(),
			  " [--plus-content]" );
	MIL << "[--plus-content] check " << repo.alias() << endl;
      }
    }

    bool do_refresh =
      repo.enabled() &&
      repo.autorefresh() &&
      !zypper.globalOpts().no_refresh;

    if (do_refresh)
    {
      MIL << "calling refresh for " << repo.alias() << endl;

      // handle root user differently
      if (geteuid() == 0 && !zypper.globalOpts().changedRoot)
      {
        if (refresh_raw_metadata(zypper, repo, false)
            || build_cache(zypper, repo, false))
        {
          zypper.out().info(boost::str(format(
              _("Skipping repository '%s' because of the above error."))
              % repo.asUserString()), Out::QUIET);
          WAR << format("Skipping repository '%s' because of the above error.")
              % repo.alias() << endl;

          it->setEnabled(false);
	  contentcheck = false;
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
          zypper.out().info(boost::str(format(_(
              "Repository '%s' is out-of-date. You can run 'zypper refresh'"
              " as root to update it."))
              % repo.asUserString()));

          MIL << "We're running as non-root, skipping refresh of "
              << repo.alias() << endl;
        }
      }
    }
    // even if refresh is not required, try to build the sqlite cache
    // for the case of non-existing cache
    else if (repo.enabled())
    {
      // handle root user differently
      if (geteuid() == 0 && !zypper.globalOpts().changedRoot)
      {
        if (build_cache(zypper, repo, false))
        {
          zypper.out().warning(boost::str(format(
              _("Skipping repository '%s' because of the above error."))
              % repo.asUserString()), Out::QUIET);
          WAR << format("Skipping repository '%s' because of the above error.")
              % repo.alias() << endl;

          it->setEnabled(false);
	  contentcheck = false;
        }
      }
      // non-root user
      else
      {
        // if error is returned, it means zypp attempted to build the metadata
        // cache for the repo and failed because writing is not allowed for
        // non-root. Thus, just display refresh hint for non-root user.
        if (build_cache(zypper, repo, false))
        {
          zypper.out().warning(boost::str(format(_(
              "The metadata cache needs to be built for the '%s' repository."
              " You can run 'zypper refresh' as root to do this."))
              % repo.asUserString()), Out::QUIET);

          MIL <<  "We're running as non-root, skipping building of "
            << repo.alias() + "cache" << endl;

          zypper.out().info(boost::str(format(_("Disabling repository '%s'."))
              % repo.asUserString()));
          WAR << "Disabling repository '" << repo.alias() << "'" << endl;
          it->setEnabled(false);
	  contentcheck = false;
        }
      }
    }

    if ( contentcheck )
    {
      if ( repo.hasContentAny( gData.additional_content_repos ) )
      {
	zypper.out().info( boost::format(_("Temporarily enabling repository '%s'."))
			   % repo.asUserString(),
			   " [--plus-content]" );
	it->setEnabled(true);
	MIL << "[--plus-content] check says use " << repo.alias() << endl;
      }
      else
      {
	zypper.out().info( boost::format(_("Repository '%s' stays disabled."))
			   % repo.asUserString(),
			   " [--plus-content]" );
	MIL << "[--plus-content] check says disable " << repo.alias() << endl;
      }
    }
  }
}

// Explicit instantiation required for versions used outside repos.o
template void init_repos( Zypper &, const std::vector<std::string> & );

// ----------------------------------------------------------------------------

void init_repos(Zypper & zypper)
{ init_repos(zypper, std::vector<std::string>()); }


template <typename Container>
void init_repos(Zypper & zypper, const Container & container)
{
  static bool done = false;
  //! \todo this has to be done so that it works in zypper shell
  if (done)
    return;

  if ( !zypper.globalOpts().disable_system_sources )
    do_init_repos(zypper, container);

  done = true;
}

// ----------------------------------------------------------------------------

void init_target (Zypper & zypper)
{
  static bool done = false;
  if (!done)
  {
    zypper.out().info(_("Initializing Target"), Out::HIGH);
    MIL << "Initializing target" << endl;

    try
    {
      God->initializeTarget(zypper.globalOpts().root_dir);
    }
    catch (const Exception & e)
    {
      zypper.out().error(e,
        _("Target initialization failed:"),
        geteuid() != 0 ?
          _("Running 'zypper refresh' as root might resolve the problem."):""
      );

      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      ZYPP_THROW( ExitRequestException("Target initialization failed: " + e.msg()) );
    }

    done = true;
  }
}

// ----------------------------------------------------------------------------

static void print_repo_list(Zypper & zypper,
                            const std::list<zypp::RepoInfo> &repos )
{
  Table tbl;
  bool all = zypper.cOpts().count("details");
  string list_cols = zypper.config().repo_list_columns;
  bool showalias = zypper.cOpts().count("alias")
      || zypper.cOpts().count("sort-by-alias")
      || list_cols.find_first_of("aA") != string::npos;
  bool showname = zypper.cOpts().count("name")
      || zypper.cOpts().count("sort-by-name")
      || list_cols.find_first_of("nN") != string::npos;
  bool showrefresh = zypper.cOpts().count("refresh")
      || list_cols.find_first_of("rR") != string::npos;
  bool showuri = zypper.cOpts().count("uri") || zypper.cOpts().count("url")
      || zypper.cOpts().count("sort-by-uri")
      || list_cols.find_first_of("uU") != string::npos;
  bool showprio = zypper.cOpts().count("priority")
      || zypper.cOpts().count("sort-by-priority")
      || list_cols.find_first_of("pP") != string::npos;
  bool showservice = zypper.cOpts().count("service");
  bool sort_override = zypper.cOpts().count("sort-by-uri")
      || zypper.cOpts().count("sort-by-priority")
      || zypper.cOpts().count("sort-by-alias")
      || zypper.cOpts().count("sort-by-name");
  bool show_enabled_only = zypper.cOpts().count("show-enabled-only");


  // header
  TableHeader th;
  // keep count of columns so that we know which one to sort
  // TODO might be worth to improve Table to allow named columns so this can be avoided
  unsigned index = 0;
  // number of the column to sort by
  unsigned sort_index = 0;

  // repo number
  th << "#";

  // alias
  if (all || showalias)
  {
    th << _("Alias");
    ++index;
    // if (zypper.cOpts().count("sort-by-alias")
    //    || (list_cols.find("A") != string::npos && !sort_override))
    // sort by alias by default
    sort_index = index;
  }

  // name
  if (all || showname)
  {
     th << _("Name");
     ++index;
     if (zypper.cOpts().count("sort-by-name")
         || (list_cols.find("N") != string::npos && !sort_override))
       sort_index = index;
  }

  // 'enabled' flag
  th << _("Enabled");
  ++index;

  // GPG Check
  th << _("GPG Check");
  ++index;

  // 'autorefresh' flag
  if (all || showrefresh)
  {
    // translators: 'zypper repos' column - whether autorefresh is enabled
    // for the repository
    th << _("Refresh");
    ++index;
    if (list_cols.find("R") != string::npos && !sort_override)
      sort_index = index;
  }

  // priority
  if (all || showprio)
  {
    // translators: repository priority (in zypper repos -p or -d)
    th << _("Priority");
    ++index;
    if (zypper.cOpts().count("sort-by-priority")
        || (list_cols.find("P") != string::npos && !sort_override))
      sort_index = index;
  }

  // type
  if (all)
  {
    th << _("Type");
    ++index;
  }

  // URI
  if (all || showuri)
  {
    th << _("URI");
    ++index;
    if (zypper.cOpts().count("sort-by-uri")
        || (list_cols.find("U") != string::npos && !sort_override))
      sort_index = index;
  }

  // service alias
  if (all || showservice)
  {
    th << _("Service");
    ++index;
  }

  tbl << th;

  // table data
  int i = 0;
  unsigned nindent = repos.size() > 9 ? repos.size() > 99 ? 3 : 2 : 1;
  for_( it, repos.begin(), repos.end() )
  {
    ++i; // continuous numbering including skipped ones
    RepoInfo repo = *it;

    if ( show_enabled_only && !repo.enabled() )
      continue;

    TableRow tr(index);
    RepoGpgCheckStrings repoGpgCheck( repo );	// color strings for tag/enabled/gpgcheck

    // number
    tr << ColorString( repoGpgCheck._tagColor, str::numstring(i, nindent) ).str();
    // alias
    if (all || showalias) tr << repo.alias();
    // name
    if (all || showname) tr << repo.name();
    // enabled?
    tr << repoGpgCheck._enabledYN.str();
    // GPG Check
    tr << repoGpgCheck._gpgCheckYN.str();
    // autorefresh?
    if (all || showrefresh) tr << asYesNo(repo.autorefresh());
    // priority
    if (all || showprio)
      // output flush right; looks nicer and sorts correctly
      tr << str::numstring (repo.priority(), 4);
    // type
    if (all)
      tr << repo.type().asString();
    // url
    /**
     * \todo properly handle multiple baseurls - show "(multiple)"
     */
    if (all || showuri)
      tr << (repo.baseUrlSet() ? repo.url().asString() : (repo.mirrorListUrl().asString().empty() ? "n/a" : repo.mirrorListUrl().asString() ));

    if (all || showservice)
      tr << repo.service();

    tbl << tr;
  }

  if (tbl.empty()) {
    zypper.out().warning(_("No repositories defined."));
    zypper.out().info(_("Use the 'zypper addrepo' command to add one or more repositories."));
  }
  else
  {
    // sort
    tbl.sort(sort_index);
    // print
    cout << tbl;
  }
}

// ----------------------------------------------------------------------------

static void print_repo_details(Zypper & zypper, list<RepoInfo> & repos)
{
  bool another = false;
  for_(it, repos.begin(), repos.end())
  {
    if (another)
      cout << endl;

    RepoInfo repo = *it;
    RepoGpgCheckStrings repoGpgCheck( repo );

    Table t;
    t.lineStyle(::Colon);
    t.allowAbbrev(1);

    t << (  TableRow() << _("Alias")		<< repo.alias() )
      << (  TableRow() << _("Name")		<< repo.name() )
      << (  TableRow() << _("URI")		<< (repo.baseUrlSet()
						    ? repo.url().asString()
						    : (repo.mirrorListUrl().asString().empty()
						       ? "n/a"
						       : repo.mirrorListUrl().asString())) )
      << (  TableRow() << _("Enabled")		<< repoGpgCheck._enabledYN.str() )
      << (  TableRow() << _("GPG Check")	<< repoGpgCheck._gpgCheckYN.str() )
      << (  TableRow() << _("Priority")		<< str::form("%d", repo.priority()) )
      << (  TableRow() << _("Auto-refresh")	<< (repo.autorefresh() ? _("On") : _("Off")) )
      << (  TableRow() << _("Keep Packages")	<< (repo.keepPackages() ? _("On") : _("Off")) )
      << (  TableRow() << _("Type")		<< repo.type().asString() )
      << (  TableRow() << _("GPG Key URI")	<< repo.gpgKeyUrl() )
      << (  TableRow() << _("Path Prefix")	<< repo.path() )
      << (  TableRow() << _("Parent Service")	<< repo.service() )
      << (  TableRow() << _("Repo Info Path")	<< repo.filepath() )
      << (  TableRow() << _("MD Cache Path")	<< repo.metadataPath() )
      ;

    cout << t;
    another = true;
  }
}

// ----------------------------------------------------------------------------

/** Repo list as xml */
static void print_xml_repo_list(Zypper & zypper, list<RepoInfo> repos)
{
  cout << "<repo-list>" << endl;
  for (std::list<RepoInfo>::const_iterator it = repos.begin();
       it !=  repos.end(); ++it)
    it->dumpAsXmlOn(cout);
  cout << "</repo-list>" << endl;
}

// ----------------------------------------------------------------------------

void print_repos_to(const std::list<zypp::RepoInfo> &repos, ostream & out)
{
  for (std::list<RepoInfo>::const_iterator it = repos.begin();
       it !=  repos.end(); ++it)
  {
    it->dumpAsIniOn(out);
    out << endl;
  }
}

// ----------------------------------------------------------------------------

void list_repos(Zypper & zypper)
{
  RepoManager & manager = zypper.repoManager();
  RuntimeData & gData = zypper.runtimeData();
  list<RepoInfo> repos;
  list<string> not_found;

  try
  {
    if (zypper.arguments().empty())
      repos.insert(repos.end(), manager.repoBegin(), manager.repoEnd());
    else
    {
      get_repos(zypper, zypper.arguments().begin(), zypper.arguments().end(), repos, not_found);
      report_unknown_repos(zypper.out(), not_found);
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Error reading repositories:"));
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
        zypper.out().error(boost::str(format(
            _("Can't open %s for writing."))
            % file.asString()),
          _("Maybe you do not have write permissions?"));
        exit(ZYPPER_EXIT_ERR_INVALID_ARGS);
      }
      else
      {
        print_repos_to(repos, stream);
        zypper.out().info(boost::str(format(
            _("Repositories have been successfully exported to %s."))
            % (file.absolute() ? file.asString() : file.asString().substr(2))),
          Out::QUIET);
      }
    }
  }
  // print repo list as xml
  else if (zypper.out().type() == Out::TYPE_XML)
    print_xml_repo_list(zypper, repos);
  else if (!zypper.arguments().empty())
    print_repo_details(zypper, repos);
  // print repo list as table
  else
    print_repo_list(zypper, repos);

  if ( repos.empty() )
    zypper.setExitCode(ZYPPER_EXIT_NO_REPOS);
}

// ----------------------------------------------------------------------------

void refresh_repos(Zypper & zypper)
{
  MIL << "going to refresh repositories" << endl;
  // need gpg keys when downloading (#304672)
  init_target(zypper);
  RepoManager & manager = zypper.repoManager();

  list<RepoInfo> repos;
  try
  {
    repos.insert(repos.end(), manager.repoBegin(), manager.repoEnd());
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
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
  for_(it, specified.begin(), specified.end())
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
          format(_("Skipping disabled repository '%s'")) % repo.asUserString());

        if (specified.empty())
          zypper.out().info(msg, Out::HIGH);
        else
          zypper.out().error(msg);

        enabled_repo_count--;
        continue;
      }

      // do the refresh
      if (refresh_repo(zypper, repo))
      {
        zypper.out().error(boost::str(format(
          _("Skipping repository '%s' because of the above error.")) % repo.asUserString()));
        ERR << format("Skipping repository '%s' because of the above error.")
            % repo.alias() << endl;
        error_count++;
      }
    }
  }
  else
    enabled_repo_count = 0;

  // print the result message
  if (enabled_repo_count == 0)
  {
    if (!specified.empty() || !not_found.empty())
      zypper.out().warning(_("Specified repositories are not enabled or defined."));
    else
      zypper.out().warning(_("There are no enabled repositories defined."));
    zypper.out().info(str::form(_("Use '%s' or '%s' commands to add or enable repositories."),
				  "zypper addrepo", "zypper modifyrepo"));
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

/** \return false on success, true on error */
bool refresh_repo(Zypper & zypper, const zypp::RepoInfo & repo)
{
  MIL << "going to refresh repo '" << repo.alias() << "'" << endl;

  // raw metadata refresh
  bool error = false;
  if (!zypper.cOpts().count("build-only"))
  {
    bool force_download =
      zypper.cOpts().count("force") || zypper.cOpts().count("force-download");

    MIL << "calling refreshMetadata" << (force_download ? ", forced" : "")
        << endl;

    error = refresh_raw_metadata(zypper, repo, force_download);
  }

  // db rebuild
  if (!(error || zypper.cOpts().count("download-only")))
  {
    bool force_build =
      zypper.cOpts().count("force") || zypper.cOpts().count("force-build");

    MIL << "calling buildCache" << (force_build ? ", forced" : "") << endl;

    error = build_cache(zypper, repo, force_build);
  }

  return error;
}

// ----------------------------------------------------------------------------

void clean_repos(Zypper & zypper)
{
  RepoManager & manager = zypper.repoManager();

  list<RepoInfo> repos;
  try
  {
    repos.insert(repos.end(), manager.repoBegin(), manager.repoEnd());
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
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

  // should we clean packages or metadata ?
  bool clean_metadata = (copts.find("metadata") != copts.end());
  bool clean_raw_metadata = (copts.find("raw-metadata") != copts.end());
  bool clean_packages = !(clean_metadata || clean_raw_metadata);

  if( copts.find("all") != copts.end() )
  {
    clean_metadata = true;
    clean_raw_metadata = true;
    clean_packages = true;
  }

  DBG << "Metadata will be cleaned: " << clean_metadata << endl;
  DBG << "Raw metadata will be cleaned: " << clean_raw_metadata << endl;
  DBG << "Packages will be cleaned: " << clean_packages << endl;

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

      try
      {
        if( clean_metadata )
	{
	    zypper.out().info(boost::str(format(
	        _("Cleaning metadata cache for '%s'.")) % repo.asUserString()),
	        Out::HIGH);
	    manager.cleanCache(repo);
	}
        if( clean_raw_metadata )
        {
            std::string scheme( repo.url().getScheme() );
            if ( ! ( scheme == "cd" || scheme == "dvd" ) )
            {
                zypper.out().info(boost::str(format(
                    _("Cleaning raw metadata cache for '%s'.")) % repo.asUserString()),
                    Out::HIGH);
                manager.cleanMetadata(repo);
            }
            else
            {
                zypper.out().info(boost::str(format(
                    _("Keeping raw metadata cache for %s '%s'.")) % scheme % repo.asUserString()),
                    Out::HIGH);
            }
        }
        if( clean_packages )
	{
          zypper.out().info(boost::str(format(
              // translators: meaning the cached rpm files
              _("Cleaning packages for '%s'.")) % repo.asUserString()),
              Out::HIGH);
    	  manager.cleanPackages(repo);
	}
      }
      catch(...)
      {
        zypper.out().error(boost::str(format(
            _("Cannot clean repository '%s' because of an error.")) % repo.asUserString()));
        ERR << format("Cannot clean repository '%s' because of an error.")
            % repo.alias() << endl;
        error_count++;
      }
    }
  }
  else
    enabled_repo_count = 0;

  // clean the target system cache
  if( clean_metadata )
  {
    zypper.out().info(_("Cleaning installed packages cache."), Out::HIGH);
    try
    {
      init_target(zypper);
      God->target()->cleanCache();
    }
    catch (...)
    {
      zypper.out().error(_("Cannot clean installed packages cache because of an error."));
      ERR << "Couldn't clean @System cache" << endl;
      error_count++;
    }
  }

  if (zypper.arguments().empty() && copts.find("all") != copts.end())
  {
    // clean up garbage
    // this could also be done with a special option or on each 'clean'
    // regardless of the options used ...
    manager.cleanCacheDirGarbage();

    // clean zypper's cache
    // this could also be done with a special option
    filesystem::recursive_rmdir(
        Pathname(zypper.globalOpts().root_dir) / ZYPPER_RPM_CACHE_DIR);
  }

  if (error_count >= enabled_repo_count)
  {
    zypper.out().error(_("Could not clean the repositories because of errors."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  else if (error_count)
  {
    zypper.out().error(
      _("Some of the repositories have not been cleaned up because of an error."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  else if (!specified.empty())
    zypper.out().info(_("Specified repositories have been cleaned up."));
  else
    zypper.out().info(_("All repositories have been cleaned up."));
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
  RepoManager & manager = zypper.repoManager();
  RuntimeData & gData = zypper.runtimeData();

  bool is_cd = true;
  if (!repo.baseUrlsEmpty())
  {
    for(RepoInfo::urls_const_iterator it = repo.baseUrlsBegin();
        it != repo.baseUrlsEnd(); ++it)
    {
      is_cd = is_changeable_media(*it);
      if (!is_cd)
        break;
    }
  }
  if (is_cd)
  {
    zypper.out().info(
      _("This is a changeable read-only media (CD/DVD), disabling autorefresh."),
      Out::QUIET);
    repo.setAutorefresh(false);
  }


  MIL << "Going to add repository: " << repo << endl;

  try
  {
    gData.current_repo = repo;

    // reset the gData.current_repo when going out of scope
    struct Bye { ~Bye() { Zypper::instance()->runtimeData().current_repo = RepoInfo(); } } reset __attribute__ ((__unused__));

    manager.addRepository(repo);
    repo = manager.getRepo(repo);
  }
  catch (const RepoInvalidAliasException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
        boost::str(format(_("Invalid repository alias: '%s'")) % repo.alias()));
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    return;
  }
  catch (const RepoAlreadyExistsException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(boost::str(format(
        _("Repository named '%s' already exists. Please use another alias."))
        % repo.alias()));
    ERR << "Repository named '" << repo.alias() << "' already exists." << endl;
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const RepoUnknownTypeException & e)
  {
    ZYPP_CAUGHT(e);

    ostringstream s;
    s << _("Could not determine the type of the repository."
        " Please check if the defined URIs (see below) point to a valid repository:");
    if (!repo.baseUrlsEmpty())
    {
      for(RepoInfo::urls_const_iterator uit = repo.baseUrlsBegin();
          uit != repo.baseUrlsEnd(); ++uit)
        s << (*uit) << endl;
    }

    zypper.out().error(e,
      _("Can't find a valid repository at given location:"), s.str());

    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const RepoException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
        _("Problem transferring repository data from specified URI:"),
        is_cd ? "" : _("Please check whether the specified URI is accessible."));
    ERR << "Problem transferring repository data from specified URI" << endl;
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Unknown problem when adding repository:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_BUG);
    return;
  }


  if ( !repo.gpgCheck() )
  {
    zypper.out().warning( boost::formatNAC(
      // translators: BOOST STYLE POSITIONAL DIRECTIVES ( %N% )
      // translators: %1% - a repository name
      _("GPG checking is disabled in configuration of repository '%1%'. Integrity and origin of packages cannot be verified."))
      % repo.asUserString()
    );
  }

  ostringstream s;
  s << format(_("Repository '%s' successfully added")) % repo.asUserString();
  s << endl;

  {
    PropertyTable p;
    // translators: property name; short; used like "Name: value"
    p.add( _("Enabled"),	repo.enabled() );
    // translators: property name; short; used like "Name: value"
    p.add( _("Autorefresh"),	repo.autorefresh() );
    // translators: property name; short; used like "Name: value"
    p.add( _("GPG Check"), 	repo.gpgCheck() ).paint( ColorContext::MSG_WARNING, repo.gpgCheck() == false );
    // translators: property name; short; used like "Name: value"
    p.add( _("Priority"),	repo.priority() );
    // translators: property name; short; used like "Name: value"
    p.add( _("URI"),		repo.baseUrlsBegin(), repo.baseUrlsEnd() );
    s << p;

    if ( repo.priority() == RepoInfo::defaultPriority() )
    {
      s << endl << "  " <<
        _("(use \"zypper addrepo --priority <integer> ...\" to ensure repositories are utilized as desired,"
          " lower numbers for repositories that replace default packages and higher for missing packages)");
    }
  }
  zypper.out().info(s.str());



  MIL << "Repository successfully added: " << repo << endl;

  if ( is_cd )
  {
    if ( ! copts.count("no-check") )
    {
      zypper.out().info(boost::str(
	format(_("Reading data from '%s' media")) % repo.asUserString()));
	bool error = refresh_raw_metadata(zypper, repo, false);
	if (!error)
	  error = build_cache(zypper, repo, false);
	if (error)
	{
	  zypper.out().error(boost::str(
	    format(_("Problem reading data from '%s' media")) % repo.asUserString()),
		   _("Please check if your installation media is valid and readable."));
		   zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
		   return;
	}
    }
    else
    {
      zypper.out().info( boost::format(_("Reading data from '%s' media is delayed until next refresh.")) % repo.asUserString(),
			 " [--no-check]" );
    }
  }
}

// ----------------------------------------------------------------------------

void add_repo_by_url( Zypper & zypper,
                     const zypp::Url & url, const string & alias,
                     const string & type,
                     TriBool enabled, TriBool autorefresh, TriBool keepPackages, TriBool gpgCheck)
{
  MIL << "going to add repository by url (alias=" << alias << ", url=" << url
      << ")" << endl;

  unsigned prio = parse_priority(zypper);

  RepoInfo repo;

  if ( ! type.empty() )
    repo.setType(RepoType(type));

  repo.setAlias(alias.empty() ? timestamp() : alias);
  parsed_opts::const_iterator it = zypper.cOpts().find("name");
  if (it != zypper.cOpts().end())
    repo.setName(it->second.front());
  repo.addBaseUrl(url);

  if (prio >= 1)
    repo.setPriority(prio);

  // enable the repo by default
  if ( indeterminate(enabled) )
    enabled = true;
  repo.setEnabled((enabled == true));

  // disable autorefresh by default
  if ( indeterminate(autorefresh) )
    autorefresh = false;
  repo.setAutorefresh((autorefresh == true));

  if ( !indeterminate(keepPackages) )
    repo.setKeepPackages(keepPackages);

  if ( !indeterminate(gpgCheck) )
    repo.setGpgCheck(gpgCheck);

  add_repo(zypper, repo);
}

// ----------------------------------------------------------------------------

void add_repo_from_file( Zypper & zypper,
                         const std::string & repo_file_url, TriBool enabled,
                         TriBool autorefresh, TriBool keepPackages, TriBool gpgCheck)
{
  Url url = make_url(repo_file_url);
  if (!url.isValid())
  {
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    return;
  }

  unsigned prio = parse_priority(zypper);

  list<RepoInfo> repos;

  // read the repo file
  try { repos = readRepoFile(url); }
  catch (const media::MediaException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
      _("Problem accessing the file at the specified URI") + string(":"),
      _("Please check if the URI is valid and accessible."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const parser::ParseException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
      _("Problem parsing the file at the specified URI") + string(":"),
      // TranslatorExplanation don't translate the URI if the URI itself is not translated.
      // Also don't translate the '.repo' string.
      _("Is it a .repo file? See http://en.opensuse.org/Standards/RepoInfo for details."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
      _("Problem encountered while trying to read the file at the specified URI") + string(":"));
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
      zypper.out().warning(
        _("Repository with no alias defined found in the file, skipping."));
      continue;
    }

    if(repo.baseUrlsEmpty())
    {
      zypper.out().warning(boost::str(format(
        _("Repository '%s' has no URI defined, skipping.")) % repo.asUserString()));
      continue;
    }

    MIL << "requested: enabled: " << enabled << " autorefresh: " << autorefresh << endl;
    // enable by default
    if ( indeterminate(enabled) )
      enabled = true;
    repo.setEnabled((enabled == true));
    // disable autorefresh by default
    if ( indeterminate(autorefresh) )
      autorefresh = false;
    repo.setAutorefresh((autorefresh == true));

    if ( !indeterminate(keepPackages) )
      repo.setKeepPackages(keepPackages);

    if ( !indeterminate(gpgCheck) )
      repo.setGpgCheck(gpgCheck);

    if (prio >= 1)
      repo.setPriority(prio);

    MIL << "to-be-added: enabled: " << repo.enabled() << " autorefresh: " << repo.autorefresh() << endl;

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

void remove_repo(Zypper & zypper, const RepoInfo & repoinfo)
{
  bool isServiceRepo = !repoinfo.service().empty();

  RepoManager & manager = zypper.repoManager();
  manager.removeRepository(repoinfo);

  std::string msg(boost::str(format(_("Repository '%s' has been removed.")) % repoinfo.asUserString()));
  if ( isServiceRepo )
    msg += volatileTag();	// '[volatile]'

  zypper.out().info( msg );
  MIL << format("Repository '%s' has been removed.") % repoinfo.alias() << endl;

  if ( isServiceRepo )
    zypper.out().warning( volatileServiceRepoChange( repoinfo ) );
}


// ----------------------------------------------------------------------------

void rename_repo(Zypper & zypper,
                 const std::string & alias, const std::string & newalias)
{
  RepoManager & manager = zypper.repoManager();

  try
  {
    RepoInfo repo(manager.getRepositoryInfo(alias));

    if (!repo.service().empty())
    {
      zypper.out().error(str::form(
          _("Cannot change alias of '%s' repository. The repository"
            " belongs to service '%s' which is responsible for setting its alias."),
          alias.c_str(), repo.service().c_str()));
      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      return;
    }

    repo.setAlias(newalias);
    manager.modifyRepository(alias, repo);

    zypper.out().info(boost::str(format(
      _("Repository '%s' renamed to '%s'.")) % alias % repo.alias()));
    MIL << format("Repository '%s' renamed to '%s'") % alias % repo.alias() << endl;
  }
  catch (const RepoAlreadyExistsException & ex)
  {
    zypper.out().error(boost::str(format(
       _("Repository named '%s' already exists. Please use another alias."))
      % newalias));
  }
  catch (const Exception & ex)
  {
    zypper.out().error(ex,
      _("Error while modifying the repository:"),
      boost::str(format(_("Leaving repository '%s' unchanged.")) % alias));

    ERR << "Error while modifying the repository:" << ex.asUserString() << endl;
  }
}

// ----------------------------------------------------------------------------

void modify_repos_by_option( Zypper & zypper )
{
  RepoManager & manager = zypper.repoManager();
  list<RepoInfo> repos;
  repos.insert(repos.end(), manager.repoBegin(), manager.repoEnd());
  set<string> toModify;

  if ( copts.count("all") )
  {
    for_(it, repos.begin(),repos.end())
    {
      string alias = it->alias();
      modify_repo( zypper, alias );
    }
    return; //no more repository is possible
  }

  if ( copts.count("local") )
  {
    for_(it, repos.begin(),repos.end())
    {
      if (!it->baseUrlsEmpty())
      {
        if ( ! it->url().schemeIsDownloading() )
        {
          string alias = it->alias();
          toModify.insert( alias );
        }
      }
    }
  }

  if ( copts.count("remote") )
  {
    for_(it, repos.begin(),repos.end())
    {
      if (!it->baseUrlsEmpty())
      {
        if ( it->url().schemeIsDownloading() )
        {
          string alias = it->alias();
          toModify.insert( alias );
        }
      }
    }
  }

  if ( copts.count("medium-type") )
  {
    list<string> pars = copts["medium-type"];
    set<string> scheme(pars.begin(),pars.end());

    for_(it, repos.begin(),repos.end())
    {
      if (!it->baseUrlsEmpty())
      {
        if ( scheme.find(it->url().getScheme())!= scheme.end() )
        {
          string alias = it->alias();
          toModify.insert( alias );
        }
      }
    }
  }

  for_(it, toModify.begin(), toModify.end())
  {
    modify_repo( zypper, *it );
  }

}

// ----------------------------------------------------------------------------

void modify_repo(Zypper & zypper, const string & alias)
{
  // enable/disable repo
  tribool enable = get_boolean_option(zypper, "enable", "disable");
  DBG << "enable = " << enable << endl;

  // autorefresh
  tribool autoref = get_boolean_option(zypper, "refresh", "no-refresh");
  DBG << "autoref = " << autoref << endl;

  tribool keepPackages = get_boolean_option(
      zypper, "keep-packages", "no-keep-packages");
  DBG << "keepPackages = " << keepPackages << endl;

  tribool gpgCheck = get_boolean_option(
      zypper, "gpgcheck", "no-gpgcheck");
  DBG << "gpgCheck = " << gpgCheck << endl;

  unsigned prio = parse_priority(zypper);

  try
  {
    RepoManager & manager = zypper.repoManager();
    RepoInfo repo(manager.getRepositoryInfo(alias));
    bool chnaged_enabled = false;
    bool changed_autoref = false;
    bool changed_prio = false;
    bool changed_keeppackages = false;
    bool changed_gpgcheck = false;

    if (!indeterminate(enable))
    {
      if (enable != repo.enabled())
        chnaged_enabled = true;
      repo.setEnabled(enable);
    }

    if (!indeterminate(autoref))
    {
      if (autoref != repo.autorefresh())
        changed_autoref = true;
      repo.setAutorefresh(autoref);
    }

    if (!indeterminate(keepPackages))
    {
      if (keepPackages != repo.keepPackages())
        changed_keeppackages = true;
      repo.setKeepPackages(keepPackages);
    }

    if (!indeterminate(gpgCheck))
    {
      if (gpgCheck != repo.gpgCheck())
        changed_gpgcheck = true;
      repo.setGpgCheck(gpgCheck);
    }

    if (prio >= 1)
    {
      if (prio == repo.priority())
        zypper.out().info(boost::str(format(
          _("Repository '%s' priority has been left unchanged (%d)"))
          % alias % prio));
      else
      {
        repo.setPriority(prio);
        changed_prio = true;
      }
    }

    string name;
    parsed_opts::const_iterator tmp1;
    if ((tmp1 = zypper.cOpts().find("name")) != zypper.cOpts().end())
    {
      name = *tmp1->second.begin();
      if (!name.empty())
        repo.setName(name);
    }

    if (chnaged_enabled || changed_autoref || changed_prio
        || changed_keeppackages || changed_gpgcheck || !name.empty())
    {
      std::string volatileNote;	// service repos changes may be volatile
      std::string volatileNoteIfPlugin;	// plugin service repos changes may be volatile
      if (  ! repo.service().empty() )
      {
	volatileNote = volatileTag();	// '[volatile]'
	ServiceInfo si( manager.getService( repo.service() ) );
	if ( si.type() == ServiceType::PLUGIN )
	  volatileNoteIfPlugin = volatileNote;
      }
      bool didVolatileChanges = false;

      manager.modifyRepository(alias, repo);

      if (chnaged_enabled)
      {
	if ( !volatileNoteIfPlugin.empty() ) didVolatileChanges = true;
	// the by now only persistent change for (non plugin) service repos.
        if (repo.enabled())
          zypper.out().info(boost::str(format(
            _("Repository '%s' has been successfully enabled.")) % alias)+volatileNoteIfPlugin);
        else
          zypper.out().info(boost::str(format(
            _("Repository '%s' has been successfully disabled.")) % alias)+volatileNoteIfPlugin);
      }

      if (changed_autoref)
      {
	if ( !volatileNote.empty() ) didVolatileChanges = true;
        if (repo.autorefresh())
          zypper.out().info(boost::str(format(
            _("Autorefresh has been enabled for repository '%s'.")) % alias)+volatileNote);
        else
          zypper.out().info(boost::str(format(
            _("Autorefresh has been disabled for repository '%s'.")) % alias)+volatileNote);
      }

      if (changed_keeppackages)
      {
	if ( !volatileNote.empty() ) didVolatileChanges = true;
        if (repo.keepPackages())
          zypper.out().info(boost::str(format(
            _("RPM files caching has been enabled for repository '%s'.")) % alias)+volatileNote);
        else
          zypper.out().info(boost::str(format(
            _("RPM files caching has been disabled for repository '%s'.")) % alias)+volatileNote);
      }

      if (changed_gpgcheck)
      {
	if ( !volatileNote.empty() ) didVolatileChanges = true;
        if (repo.gpgCheck())
          zypper.out().info(boost::str(format(
            _("GPG check has been enabled for repository '%s'.")) % alias)+volatileNote);
        else
          zypper.out().info(boost::str(format(
            _("GPG check has been disabled for repository '%s'.")) % alias)+volatileNote);
      }

      if (changed_prio)
      {
	if ( !volatileNote.empty() ) didVolatileChanges = true;
        zypper.out().info(boost::str(format(
          _("Repository '%s' priority has been set to %d.")) % alias % prio)+volatileNote);
      }

      if (!name.empty())
      {
	if ( !volatileNote.empty() ) didVolatileChanges = true;
        zypper.out().info(boost::str(format(
          _("Name of repository '%s' has been set to '%s'.")) % alias % name)+volatileNote);
      }

      if ( didVolatileChanges )
      {
	zypper.out().warning( volatileServiceRepoChange( repo ) );
      }
    }
    else
    {
      zypper.out().info(boost::str(format(
        _("Nothing to change for repository '%s'.")) % alias));
      MIL << format("Nothing to modify in '%s':") % alias << repo << endl;
    }
  }
  catch (const Exception & ex)
  {
    zypper.out().error(ex,
      _("Error while modifying the repository:"),
      boost::str(format(_("Leaving repository %s unchanged.")) % alias));

    ERR << "Error while modifying the repository:" << ex.asUserString() << endl;
  }
}

// ---------------------------------------------------------------------------
// Service Handling
// ---------------------------------------------------------------------------

static ServiceList get_all_services(Zypper & zypper)
{
  RepoManager & manager = zypper.repoManager();
  ServiceList services;

  try
  {
    // RIS type services
    for_(it, manager.serviceBegin(), manager.serviceEnd())
    {
      ServiceInfo_Ptr s;
      s.reset(new ServiceInfo(*it));
      services.insert(services.end(), s);
    }

    // non-services repos
    for_(it, manager.repoBegin(), manager.repoEnd())
    {
      if (!it->service().empty())
        continue;
      RepoInfo_Ptr r;
      r.reset(new RepoInfo(*it));
      services.insert(services.end(), r);
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Error reading services:"));
    exit(ZYPPER_EXIT_ERR_ZYPP);
  }

  return services;
}

bool match_service(Zypper & zypper, string str, RepoInfoBase_Ptr & service_ptr)
{
  ServiceList known = get_all_services(zypper);
  bool found = false;

  unsigned int number = 1; // service number
  for (ServiceList::const_iterator known_it = known.begin();
      known_it != known.end(); ++known_it, ++number)
  {
    unsigned int tmp = 0;
    safe_lexical_cast (str, tmp); // try to make an int out of the string

    try
    {
      // match by alias or number
      found = (*known_it)->alias() == str || tmp == number;

      // match by URL
      if (!found)
      {
        url::ViewOption urlview = url::ViewOption::DEFAULTS + url::ViewOption::WITH_PASSWORD;
        if (zypper.cOpts().count("loose-auth"))
        {
          urlview = urlview
            - url::ViewOptions::WITH_PASSWORD
            - url::ViewOptions::WITH_USERNAME;
        }
        if (zypper.cOpts().count("loose-query"))
          urlview = urlview - url::ViewOptions::WITH_QUERY_STR;

        ServiceInfo_Ptr s_ptr = dynamic_pointer_cast<ServiceInfo>(*known_it);

        if (!(urlview.has(url::ViewOptions::WITH_PASSWORD)
            && urlview.has(url::ViewOptions::WITH_QUERY_STR)))
        {
          if (s_ptr)
            found =
              Url(str).asString(urlview) == s_ptr->url().asString(urlview);
          else
          {
            RepoInfo_Ptr r_ptr = dynamic_pointer_cast<RepoInfo>(*known_it);
            if (!r_ptr->baseUrlsEmpty())
            {
              for_(urlit, r_ptr->baseUrlsBegin(), r_ptr->baseUrlsEnd())
                if (urlit->asString(urlview) == Url(str).asString(urlview))
                {
                  found = true;
                  break;
                }
            }
          }
        }
        else
        {
          if (s_ptr)
            found = Url(str) == s_ptr->url();
          else
          {
            RepoInfo_Ptr r_ptr = dynamic_pointer_cast<RepoInfo>(*known_it);
            if (!r_ptr->baseUrlsEmpty())
            {
              found =
                find(r_ptr->baseUrlsBegin(), r_ptr->baseUrlsEnd(), Url(str))
                != r_ptr->baseUrlsEnd();
            }
          }
        }
      }
      if (found)
      {
        service_ptr = *known_it;
        break;
      }
    }
    catch(const url::UrlException &){}

  } // END for all known services

  return found;
}

/**
 * Say "Service %s not found" for all strings in \a not_found list.
 */
static void report_unknown_services(Out & out, list<string> not_found)
{
  for_(it, not_found.begin(), not_found.end())
    out.error(boost::str(format(
      _("Service '%s' not found by its alias, number, or URI.")) % *it));

  if (!not_found.empty())
    out.info(str::form(
        _("Use '%s' to get the list of defined services."), "zypper repos"));
}

/**
 * Try to find ServiceInfo or RepoInfo counterparts among known services by alias, number,
 * or URI, based on the list of strings given as the iterator range \a begin and
 * \a end. Matching objects will be added to \a services (as RepoInfoBase_Ptr) and those
 * with no match will be added to \a not_found.
 */
template<typename T>
void get_services( Zypper & zypper,
                   const T & begin, const T & end,
                   ServiceList & services, list<string> & not_found)
{
  for (T it = begin; it != end; ++it)
  {
    RepoInfoBase_Ptr service;

    if (!match_service(zypper, *it, service))
    {
      not_found.push_back(*it);
      continue;
    }

    // service found
    // is it a duplicate? compare by alias and URIs
    //! \todo operator== in RepoInfo?
    bool duplicate = false;
    for (ServiceList::const_iterator serv_it = services.begin();
        serv_it != services.end(); ++serv_it)
    {
      ServiceInfo_Ptr s_ptr = dynamic_pointer_cast<ServiceInfo>(*serv_it);
      ServiceInfo_Ptr current_service_ptr = dynamic_pointer_cast<ServiceInfo>(service);

      // one is a service, the other is a repo
      if (s_ptr && !current_service_ptr)
        continue;

      // service
      if (s_ptr)
      {
        if (s_ptr->alias() == current_service_ptr->alias() &&
            s_ptr->url() == current_service_ptr->url())
        {
          duplicate = true;
          break;
        }
      }
      // repo
      else if (repo_cmp_alias_urls(
          *dynamic_pointer_cast<RepoInfo>(service),
          *dynamic_pointer_cast<RepoInfo>(*serv_it)))
      {
        duplicate = true;
        break;
      }
    } // END for all found so far

    if (!duplicate)
      services.push_back(service);
  }
}

// ---------------------------------------------------------------------------

struct RepoCollector
{
  bool collect( const RepoInfo &repo )
  {
    repos.push_back(repo);
    return true;
  }

  RepoInfoList repos;
};

// ---------------------------------------------------------------------------

enum ServiceListFlagsBits
{
  SF_SHOW_ALL        = 7,
  SF_SHOW_URI        = 1,
  SF_SHOW_PRIO       = 1 << 1,
  SF_SHOW_WITH_REPOS = 1 << 2,
  SF_SERVICE_REPO    = 1 << 15
};

ZYPP_DECLARE_FLAGS(ServiceListFlags,ServiceListFlagsBits);
ZYPP_DECLARE_OPERATORS_FOR_FLAGS(ServiceListFlags);

static void service_list_tr(
    Zypper & zypper,
    Table & tbl,
    const RepoInfoBase_Ptr & srv,
    unsigned int reponumber,
    const ServiceListFlags & flags)
{
  ServiceInfo_Ptr service = dynamic_pointer_cast<ServiceInfo>(srv);
  RepoInfo_Ptr repo;
  if ( ! service )
    repo = dynamic_pointer_cast<RepoInfo>(srv);
  RepoGpgCheckStrings repoGpgCheck( service ? RepoGpgCheckStrings(*service) : RepoGpgCheckStrings(*repo) );

  TableRow tr(8);

  // number
  if (flags & SF_SERVICE_REPO)
    if ( repo && ! repo->enabled() )
      tr << ColorString( repoGpgCheck._tagColor, "-" ).str();
    else
      tr << "";
  else
    tr << ColorString( repoGpgCheck._tagColor, str::numstring(reponumber) ).str();

  // alias
  tr << srv->alias();
  // name
  tr << srv->name();
  // enabled?
  tr << repoGpgCheck._enabledYN.str();
  // GPG Check
  tr << repoGpgCheck._gpgCheckYN.str();
  // autorefresh?
  tr << (srv->autorefresh() ? _("Yes") : _("No"));

  // priority
  if (flags & SF_SHOW_PRIO)
  {
    if (service)
      tr << "";
    else
      tr << str::numstring (repo->priority(), 4); // output flush right; looks nicer and sorts correctly
  }

  // type
  if (service)
    tr << service->type().asString();
  else
    tr << repo->type().asString();

  // url
  if (flags & SF_SHOW_URI)
  {
    if (service)
      tr << service->url().asString();
    else
      tr << repo->url().asString();
  }

  tbl << tr;
}

// ---------------------------------------------------------------------------

static void print_service_list(Zypper & zypper,
                               const list<RepoInfoBase_Ptr> & services)
{
  Table tbl;

  // flags

  ServiceListFlags flags(0);
  if (zypper.cOpts().count("details"))
    flags |= SF_SHOW_ALL;
  else
  {
    if (zypper.cOpts().count("uri")
        || zypper.cOpts().count("url")
        || zypper.cOpts().count("sort-by-uri"))
      flags |= SF_SHOW_URI;
    if (zypper.cOpts().count("priority")
        || zypper.cOpts().count("sort-by-priority"))
      flags |= SF_SHOW_PRIO;
  }

  bool with_repos = zypper.cOpts().count("with-repos");
  //! \todo string type = zypper.cOpts().count("type");

  // header
  TableHeader th;

  // fixed 'zypper services' columns
  th << "#"
     << _("Alias")
     << _("Name")
     << _("Enabled")
     << _("GPG Check")
     // translators: 'zypper repos' column - whether autorefresh is enabled for the repository
     << _("Refresh");
  // optional columns
  if (flags & SF_SHOW_PRIO)
    // translators: repository priority (in zypper repos -p or -d)
    th << _("Priority");
  th << _("Type");
  if (flags & SF_SHOW_URI)
    th << _("URI");
  tbl << th;

  int i = 0;

  bool show_enabled_only = zypper.cOpts().count("show-enabled-only");

  for_( it, services.begin(), services.end() )
  {
    ++i; // continuous numbering including skipped ones

    bool servicePrinted = false;
    // Unconditionally print the service before the 1st repo is
    // printed. Undesired, but possible, that a disabled service
    // owns (manually) enabled repos.
    if (with_repos && dynamic_pointer_cast<ServiceInfo>(*it))
    {
      RepoCollector collector;
      RepoManager & rm = zypper.repoManager();

      rm.getRepositoriesInService((*it)->alias(),
          make_function_output_iterator(
              bind(&RepoCollector::collect, &collector, _1)));

      for_(repoit, collector.repos.begin(), collector.repos.end())
      {
        RepoInfoBase_Ptr ptr(new RepoInfo(*repoit));

	if ( show_enabled_only && !repoit->enabled() )
	  continue;

	if ( !servicePrinted )
	{
	  service_list_tr(zypper, tbl, *it, i, flags);
	  servicePrinted = true;
	}
	// SF_SERVICE_REPO: we print repos of the current service
        service_list_tr(zypper, tbl, ptr, i, flags|SF_SERVICE_REPO);
      }
    }
    if ( servicePrinted )
      continue;

    // Here: No repo enforced printing the service, so do so if
    // necessary.
    if ( show_enabled_only && !(*it)->enabled() )
      continue;

    service_list_tr(zypper, tbl, *it, i, flags);
  }

  if (tbl.empty())
    zypper.out().info(str::form(_(
        "No services defined. Use the '%s' command to add one or more services."),
        "zypper addservice"));
  else
  {
    // sort
    if (zypper.cOpts().count("sort-by-uri"))
    {
      if ((flags & SF_SHOW_ALL) == SF_SHOW_ALL)
        tbl.sort(7);
      else if (flags & SF_SHOW_PRIO)
        tbl.sort(7);
      else
        tbl.sort(6);
    }
    else if (zypper.cOpts().count("sort-by-alias"))
      tbl.sort(1);
    else if (zypper.cOpts().count("sort-by-name"))
      tbl.sort(2);
    else if (zypper.cOpts().count("sort-by-priority"))
      tbl.sort(5);

    // print
    cout << tbl;
  }
}

// ----------------------------------------------------------------------------

static void print_xml_service_list(Zypper & zypper,
                                   const list<RepoInfoBase_Ptr> & services)
{
  //string type =

  cout << "<service-list>" << endl;


  ServiceInfo_Ptr s_ptr;
  for (list<RepoInfoBase_Ptr>::const_iterator it = services.begin();
       it != services.end(); ++it)
  {
    s_ptr = dynamic_pointer_cast<ServiceInfo>(*it);
    // print also service's repos
    if (s_ptr)
    {
      RepoCollector collector;
      RepoManager & rm = zypper.repoManager();
      rm.getRepositoriesInService((*it)->alias(),
          make_function_output_iterator(
              bind(&RepoCollector::collect, &collector, _1)));
      ostringstream sout;
      for_(repoit, collector.repos.begin(), collector.repos.end())
        repoit->dumpAsXmlOn(sout);
      (*it)->dumpAsXmlOn(cout, sout.str());
      continue;
    }

    (*it)->dumpAsXmlOn(cout);
  }

  cout << "</service-list>" << endl;
}

// ---------------------------------------------------------------------------

void list_services(Zypper & zypper)
{
  ServiceList services = get_all_services(zypper);

  // export to file or stdout in repo file format
  if (copts.count("export"))
  {
    string filename_str = copts["export"].front();
    if (filename_str == "-")
    {
      //print_repos_to(repos, cout);
    }
    else
    {
      if (filename_str.rfind(".repo") == string::npos)
        filename_str += ".repo";

      Pathname file(filename_str);
      std::ofstream stream(file.c_str());
      if (!stream)
      {
        zypper.out().error(boost::str(format(
            _("Can't open %s for writing."))
            % file.asString()),
          _("Maybe you do not have write permissions?"));
        exit(ZYPPER_EXIT_ERR_INVALID_ARGS);
      }
      else
      {
        //print_repos_to(repos, stream);
        zypper.out().info(boost::str(format(
            _("Repositories have been successfully exported to %s."))
            % (file.absolute() ? file.asString() : file.asString().substr(2))),
          Out::QUIET);
      }
    }
  }
  // print repo list as xml
  else if (zypper.out().type() == Out::TYPE_XML)
    print_xml_service_list(zypper, services);
  else
    print_service_list(zypper, services);
}

// ---------------------------------------------------------------------------

void add_service(Zypper & zypper, const ServiceInfo & service)
{
  RepoManager manager(zypper.globalOpts().rm_options);

  try
  {
    manager.addService(service);
  }
  catch (const RepoAlreadyExistsException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(boost::str(format(
        _("Service aliased '%s' already exists. Please use another alias."))
        % service.alias()));
    ERR << "Service aliased '" << service.alias() << "' already exists." << endl;
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  catch (const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(boost::str(format(
        _("Error occured while adding service '%s'.")) % service.alias()));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }

  zypper.out().info(boost::str(
    format(_("Service '%s' has been successfully added.")) % service.asUserString()));
  MIL << format("Service '%s' has been added.") % service.alias() << endl;
}

// ---------------------------------------------------------------------------

void add_service_by_url( Zypper & zypper,
                         const zypp::Url & url, const string & alias,
                         const string & type, tribool enabled)
{
  MIL << "going to add service by url (alias=" << alias << ", url=" << url
      << ")" << endl;

  ServiceInfo service;

  //! \todo addrepo if type is specified and is not 'ris'.
  if ( ! type.empty() )
    service.setType(ServiceType(type));

  service.setAlias(alias.empty() ? timestamp() : alias);
  parsed_opts::const_iterator it = zypper.cOpts().find("name");
  if (it != zypper.cOpts().end())
    service.setName(it->second.front());
  service.setUrl(url);

  if ( !indeterminate(enabled) )
    service.setEnabled((enabled == true));

  add_service(zypper, service);
}


// ---------------------------------------------------------------------------

void remove_service(Zypper & zypper, const ServiceInfo & service)
{
  RepoManager & manager = zypper.repoManager();

  zypper.out().info(boost::str(
    format(_("Removing service '%s':")) % service.asUserString()));
  manager.removeService(service);
  zypper.out().info(boost::str(
    format(_("Service '%s' has been removed.")) % service.asUserString()));
  MIL << format("Service '%s' has been removed.") % service.alias() << endl;
}

// ---------------------------------------------------------------------------

static bool refresh_service(Zypper & zypper, const ServiceInfo & service)
{
  MIL << "going to refresh service '" << service.alias() << "'" << endl;
  init_target(zypper);	// need targetDistribution for service refresh
  RepoManager & manager = zypper.repoManager();

  bool error = true;
  try
  {
    zypper.out().info( str::form(_("Refreshing service '%s'."), service.asUserString().c_str() ) );

    RepoManager::RefreshServiceOptions opts;
    if ( zypper.cOpts().count("restore-status") )
      opts |= RepoManager::RefreshService_restoreStatus;
    if ( zypper.cOpts().count("force")
      && ( zypper.command() == ZypperCommand::REFRESH || zypper.command() == ZypperCommand::REFRESH_SERVICES ) )
      opts |= RepoManager::RefreshService_forceRefresh;

    manager.refreshService( service, opts );
    error = false;
  }
  catch ( const repo::ServicePluginInformalException & e )
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, str::form(
        _("Problem retrieving the repository index file for service '%s':"), service.asUserString().c_str()));
    zypper.out().warning( str::form(
        _("Skipping service '%s' because of the above error."), service.asUserString().c_str()));
    // this is just an informal note. The service will be used as is (usually empty)
    error = false;
  }
  catch (const MediaException & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, str::form(
        _("Problem retrieving the repository index file for service '%s':"), service.asUserString().c_str()),
        _("Check if the URI is valid and accessible."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }

  return error;
}

// ---------------------------------------------------------------------------

void refresh_services(Zypper & zypper)
{
  MIL << "going to refresh services" << endl;

  ServiceList services = get_all_services(zypper);

  // get the list of repos specified on the command line ...
  ServiceList specified;
  list<string> not_found;
  // ...as command arguments
  get_services(zypper, zypper.arguments().begin(), zypper.arguments().end(),
      specified, not_found);
  report_unknown_services(zypper.out(), not_found);

  unsigned error_count = 0;
  unsigned enabled_service_count = services.size();

  if (!specified.empty() || not_found.empty())
  {
    unsigned number = 0;
    for_(sit, services.begin(), services.end())
    {
      ++number;
      RepoInfoBase_Ptr service_ptr(*sit);

      // skip services not specified on the command line
      if (!specified.empty())
      {
        bool found = false;
        for_(it, specified.begin(), specified.end())
          if ((*it)->alias() == service_ptr->alias())
          {
            found = true;
            break;
          }

        if (!found)
        {
          DBG << service_ptr->alias() << "(#" << number << ") not specified,"
              << " skipping." << endl;
          --enabled_service_count;
          continue;
        }
      }

      // skip disabled services
      if (!service_ptr->enabled())
      {
        string msg = boost::str(
          format(_("Skipping disabled service '%s'")) % service_ptr->asUserString());
        DBG << "skipping disabled service '" << service_ptr->alias() << "'" << endl;

        if (specified.empty())
          zypper.out().info(msg, Out::HIGH);
        else
          zypper.out().error(msg);

        --enabled_service_count;
        continue;
      }

      // do the refresh
      bool error = false;
      ServiceInfo_Ptr s = dynamic_pointer_cast<ServiceInfo>(service_ptr);
      if (s)
      {
        error = refresh_service(zypper, *s);

        // refresh also service's repos
        if (zypper.cOpts().count("with-repos"))
        {
          RepoCollector collector;
          RepoManager & rm = zypper.repoManager();
          rm.getRepositoriesInService(s->alias(),
              make_function_output_iterator(
                  bind(&RepoCollector::collect, &collector, _1)));
          for_(repoit, collector.repos.begin(), collector.repos.end())
            refresh_repo(zypper, *repoit);
        }
      }
      else
      {
        if (!zypper.cOpts().count("with-repos"))
        {
          DBG << str::form(
              "Skipping non-index service '%s' because '%s' is used.",
              service_ptr->asUserString().c_str(), "--no-repos");
          continue;
        }
        error = refresh_repo(zypper, *dynamic_pointer_cast<RepoInfo>(service_ptr));
      }

      if (error)
      {
        zypper.out().error(boost::str(format(
          _("Skipping service '%s' because of the above error.")) % service_ptr->asUserString().c_str()));
        ERR << format("Skipping service '%s' because of the above error.")
            % service_ptr->alias() << endl;
        ++error_count;
      }
    }
  }
  else
    enabled_service_count = 0;

  // print the result message
  if (enabled_service_count == 0)
  {
    string hint = str::form(
        _("Use '%s' or '%s' commands to add or enable services."),
        "zypper addservice", "zypper modifyservice");
    if (!specified.empty() || !not_found.empty())
      zypper.out().error(_("Specified services are not enabled or defined."), hint);
    else
      zypper.out().error(_("There are no enabled services defined."), hint);
  }
  else if (error_count == enabled_service_count)
  {
    zypper.out().error(_("Could not refresh the services because of errors."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  else if (error_count)
  {
    zypper.out().error(_("Some of the services have not been refreshed because of an error."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return;
  }
  else if (!specified.empty())
    zypper.out().info(_("Specified services have been refreshed."));
  else
    zypper.out().info(_("All services have been refreshed."));

  MIL << "DONE";
}

void checkIfToRefreshPluginServices( Zypper & zypper )
{
  // check root user
  if ( geteuid() != 0 )
    return;

  RepoManager & repoManager = zypper.repoManager();
  for ( const auto & service : repoManager.knownServices() )
  {
    if ( service.type() != ServiceType::PLUGIN )
      continue;
    if ( ! service.enabled() )
      continue;
    if ( ! service.autorefresh() )
      continue;

    bool error = refresh_service( zypper, service );
    if (error)
    {
      static const char * msg = N_("Skipping service '%s' because of the above error.");
      zypper.out().error(boost::str(format(_(msg)) % service.asUserString().c_str()));
      ERR << format(msg) % service.alias() << endl;
    }
  }
}


// ---------------------------------------------------------------------------

void modify_service(Zypper & zypper, const string & alias)
{
  // enable/disable repo
  tribool enable = get_boolean_option(zypper,"enable", "disable");
  DBG << "enable = " << enable << endl;

  // autorefresh
  tribool autoref = get_boolean_option(zypper,"refresh", "no-refresh");
  DBG << "autoref = " << autoref << endl;

  try
  {
    RepoManager & manager = zypper.repoManager();
    ServiceInfo srv(manager.getService(alias));

    bool chnaged_enabled = false;
    bool changed_autoref = false;

    if (!indeterminate(enable))
    {
      if (enable != srv.enabled())
        chnaged_enabled = true;
      srv.setEnabled(enable);
    }

    if (!indeterminate(autoref))
    {
      if (autoref != srv.autorefresh())
        changed_autoref = true;
      srv.setAutorefresh(autoref);
    }

    string name;
    parsed_opts::const_iterator tmp1;
    if ((tmp1 = zypper.cOpts().find("name")) != zypper.cOpts().end())
    {
      name = *tmp1->second.begin();
      srv.setName(name);
    }

    set<string> artoenable;
    set<string> artodisable;
    set<string> rrtoenable;
    set<string> rrtodisable;

    // RIS repos to enable
    if (zypper.cOpts().count("cl-to-enable"))
    {
      rrtoenable.insert(srv.reposToEnableBegin(), srv.reposToEnableEnd());
      srv.clearReposToEnable();
    }
    else
    {
      if ((tmp1 = zypper.cOpts().find("ar-to-enable")) != zypper.cOpts().end())
        for_(rit, tmp1->second.begin(), tmp1->second.end())
        {
          if (!srv.repoToEnableFind(*rit))
          {
            srv.addRepoToEnable(*rit);
            artoenable.insert(*rit);
          }
        }
      if ((tmp1 = zypper.cOpts().find("rr-to-enable")) != zypper.cOpts().end())
        for_(rit, tmp1->second.begin(), tmp1->second.end())
        {
          if (srv.repoToEnableFind(*rit))
          {
            srv.delRepoToEnable(*rit);
            rrtoenable.insert(*rit);
          }
        }
    }

    // RIS repos to disable
    if (zypper.cOpts().count("cl-to-disable"))
    {
      rrtodisable.insert(srv.reposToDisableBegin(), srv.reposToDisableEnd());
      srv.clearReposToDisable();
    }
    else
    {
      if ((tmp1 = zypper.cOpts().find("ar-to-disable")) != zypper.cOpts().end())
        for_(rit, tmp1->second.begin(), tmp1->second.end())
        {
          if (!srv.repoToDisableFind(*rit))
          {
            srv.addRepoToDisable(*rit);
            artodisable.insert(*rit);
          }
        }
      if ((tmp1 = zypper.cOpts().find("rr-to-disable")) != zypper.cOpts().end())
        for_(rit, tmp1->second.begin(), tmp1->second.end())
        {
          if (srv.repoToDisableFind(*rit))
          {
            srv.delRepoToDisable(*rit);
            rrtodisable.insert(*rit);
          }
        }
    }

    if (chnaged_enabled
        || changed_autoref
        || !name.empty()
        || !artoenable.empty()
        || !artodisable.empty()
        || !rrtoenable.empty()
        || !rrtodisable.empty())
    {
      manager.modifyService(alias, srv);

      if (chnaged_enabled)
      {
        if (srv.enabled())
          zypper.out().info(boost::str(format(
            _("Service '%s' has been successfully enabled.")) % alias));
        else
          zypper.out().info(boost::str(format(
            _("Service '%s' has been successfully disabled.")) % alias));
      }

      if (changed_autoref)
      {
        if (srv.autorefresh())
          zypper.out().info(boost::str(format(
            _("Autorefresh has been enabled for service '%s'.")) % alias));
        else
          zypper.out().info(boost::str(format(
            _("Autorefresh has been disabled for service '%s'.")) % alias));
      }

      if (!name.empty())
      {
        zypper.out().info(boost::str(format(
          _("Name of service '%s' has been set to '%s'.")) % alias % name));
      }

      if (!artoenable.empty())
      {
        zypper.out().info(boost::str(format(
            PL_("Repository '%s' has been added to enabled repositories of service '%s'",
                "Repositories '%s' have been added to enabled repositories of service '%s'",
                artoenable.size()))
            % str::join(artoenable.begin(), artoenable.end(), ", ") % alias));
      }
      if (!artodisable.empty())
      {
        zypper.out().info(boost::str(format(
            PL_("Repository '%s' has been added to disabled repositories of service '%s'",
                "Repositories '%s' have been added to disabled repositories of service '%s'",
                artodisable.size()))
            % str::join(artodisable.begin(), artodisable.end(), ", ") % alias));
      }
      if (!rrtoenable.empty())
      {
        zypper.out().info(boost::str(format(
            PL_("Repository '%s' has been removed from enabled repositories of service '%s'",
                "Repositories '%s' have been removed from enabled repositories of service '%s'",
                rrtoenable.size()))
            % str::join(rrtoenable.begin(), rrtoenable.end(), ", ") % alias));
      }
      if (!rrtodisable.empty())
      {
        zypper.out().info(boost::str(format(
            PL_("Repository '%s' has been removed from disabled repositories of service '%s'",
                "Repositories '%s' have been removed from disabled repositories of service '%s'",
                rrtodisable.size()))
            % str::join(rrtodisable.begin(), rrtodisable.end(), ", ") % alias));
      }
    }
    else
    {
      zypper.out().info(boost::str(format(
        _("Nothing to change for service '%s'.")) % alias));
      MIL << format("Nothing to modify in '%s':") % alias << srv << endl;
    }
  }
  catch (const Exception & ex)
  {
    zypper.out().error(ex,
      _("Error while modifying the service:"),
      boost::str(format(_("Leaving service %s unchanged.")) % alias));

    ERR << "Error while modifying the service:" << ex.asUserString() << endl;
  }
}

// ---------------------------------------------------------------------------

void modify_services_by_option( Zypper & zypper )
{
  ServiceList known = get_all_services(zypper);
  set<string> repos_to_modify;
  set<string> services_to_modify;

  ServiceInfo_Ptr sptr;
  RepoInfo_Ptr    rptr;

  if ( copts.count("all") )
  {
    for_(it, known.begin(), known.end())
    {
      if (sptr = dynamic_pointer_cast<ServiceInfo>(*it))
        modify_service( zypper, sptr->alias() );
      else
        modify_repo( zypper, (*it)->alias() );
    }
    return;
  }

  bool local = copts.count("local");
  bool remote = copts.count("remote");
  list<string> pars = copts["medium-type"];
  set<string> schemes(pars.begin(), pars.end());

  for_(it, known.begin(), known.end())
  {
    Url url;
    if (sptr = dynamic_pointer_cast<ServiceInfo>(*it))
      url = sptr->url();
    else
    {
      rptr = dynamic_pointer_cast<RepoInfo>(*it);
      if (!rptr->baseUrlsEmpty())
        url = rptr->url();
    }

    if (url.isValid())
    {
      bool modify = false;
      if (local  && ! url.schemeIsDownloading() )
        modify = true;

      if (!modify && remote && url.schemeIsDownloading() )
        modify = true;

      if (!modify && schemes.find(url.getScheme()) != schemes.end())
        modify = true;

      if (modify)
      {
        string alias = (*it)->alias();
        if (sptr)
          services_to_modify.insert( alias );
        else
          repos_to_modify.insert( alias );
      }
    }
    else
      WAR << "got invalid url: " << url.asString() << endl;
  }

  for_(it, services_to_modify.begin(), services_to_modify.end())
    modify_service( zypper, *it );

  for_(it, repos_to_modify.begin(), repos_to_modify.end())
    modify_repo( zypper, *it );
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

void load_resolvables(Zypper & zypper)
{
  static bool done = false;
  // don't call this function more than once for a single ZYpp instance
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
  RepoManager & manager = zypper.repoManager();
  RuntimeData & gData = zypper.runtimeData();

  zypper.out().info(_("Loading repository data..."));

  for (std::list<RepoInfo>::iterator it = gData.repos.begin();
       it !=  gData.repos.end(); ++it)
  {
    RepoInfo repo(*it);

    if (it->enabled())
      MIL << "Loading " << repo.alias() << " resolvables." << endl;
    else
    {
      DBG << "Skipping disabled repo '" << repo.alias() << "'" << endl;
      continue;     // #217297
    }

    try
    {
      bool error = false;
      // if there is no metadata locally
      if ( manager.metadataStatus(repo).empty() )
      {
        zypper.out().info(boost::str(
          format(_("Retrieving repository '%s' data...")) % repo.name()));
        error = refresh_raw_metadata(zypper, repo, false);
      }

      if (!error && !manager.isCached(repo))
      {
        zypper.out().info(boost::str(format(
	  _("Repository '%s' not cached. Caching...")) % repo.name()));
        error = build_cache(zypper, repo, false);
      }

      if (error)
      {
        zypper.out().error(boost::str(format(
	  _("Problem loading data from '%s'")) % repo.asUserString()));

        if (geteuid() != 0 && !zypper.globalOpts().changedRoot && manager.isCached(repo))
        {
          zypper.out().warning(boost::str(format(
            _("Repository '%s' could not be refreshed. Using old cache.")) % repo.asUserString()));
        }
        else
        {
          zypper.out().error(boost::str(format(
	    _("Resolvables from '%s' not loaded because of error.")) % repo.asUserString()));
          continue;
        }
      }

      manager.loadFromCache(repo);

      // check that the metadata is not outdated
      // feature #301904
      // ma@: Using God->pool() here would always rebuild the pools index tables,
      // because loading a new repo invalidates them. Rebuilding the whatprovides
      // index is sometimes slow, so we avoid this overhead by directly accessing
      // the sat::Pool.
      Repository robj = zypp::sat::Pool::instance().reposFind(repo.alias());
      if ( robj != Repository::noRepository &&
           robj.maybeOutdated() )
      {

       zypper.out().warning(boost::str(format(
	 _("Repository '%s' appears to be outdated. Consider using a different mirror or server.")) % repo.asUserString()), Out::QUIET);
       WAR << format("Repository '%s' seems to be outdated") % repo.alias() << endl;

      }
    }
    catch (const Exception & e)
    {
      ZYPP_CAUGHT(e);
      zypper.out().error(e, boost::str(format(
	_("Problem loading data from '%s'")) % repo.asUserString()),
	// translators: the first %s is 'zypper refresh' and the second 'zypper clean -m'
	boost::str(format(_("Try '%s', or even '%s' before doing so.")) % "zypper refresh" % "zypper clean -m")
      );
      zypper.out().info(boost::str(format(
	_("Resolvables from '%s' not loaded because of error.")) % repo.asUserString()));
    }
  }
}

// ---------------------------------------------------------------------------

void load_target_resolvables(Zypper & zypper)
{
  zypper.out().info(_("Reading installed packages..."));
  MIL << "Going to read RPM database" << endl;

  try
  {
    God->target()->load();
  }
  catch ( const Exception & e )
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e,
        _("Problem occurred while reading the installed packages:"),
        _("Please see the above error message for a hint."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }
}

// ---------------------------------------------------------------------------
// Local Variables:
// c-basic-offset: 2
// End:
