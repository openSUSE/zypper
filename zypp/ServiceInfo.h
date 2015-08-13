/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ServiceInfo.h
 *
 */
#ifndef ZYPP_SERVICE_H
#define ZYPP_SERVICE_H

#include <set>
#include <string>

#include "zypp/Url.h"

#include "zypp/base/Iterable.h"
#include "zypp/repo/ServiceType.h"
#include "zypp/RepoInfo.h"
#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class ServiceInfo
  /// \brief Service data
  ///
  /// \note Name and Url are subject to repo variable replacement
  /// (\see \ref RepoVariablesStringReplacer).
  ///
  class ServiceInfo : public repo::RepoInfoBase
  {
  public:
    /** Default ctor creates \ref noService.*/
    ServiceInfo();

    /**
     *  Creates ServiceInfo with specified alias.
     *
     * \param alias unique short name of service
     */
    ServiceInfo( const std::string & alias );

    /**
     * ServiceInfo with alias and its URL
     *
     * \param alias unique shortname of service
     * \param url url to service
     */
    ServiceInfo( const std::string & alias, const Url& url );

    virtual ~ServiceInfo();

  public:
    /** Represents an empty service. */
    static const ServiceInfo noService;

  public:

    /** The service url */
    Url url() const;

    /** The service raw url (no variables replaced) */
    Url rawUrl() const;

    /** Set the service url (raw value) */
    void setUrl( const Url& url );


    /** Service type */
    repo::ServiceType type() const;

    /** Set service type */
    void setType( const repo::ServiceType & type );

    /** Lazy init service type */
    void setProbedType( const repo::ServiceType & t ) const;

    /** \name Housekeeping data
     * You don't want to use the setters unless you are a \ref RepoManager.
     */
    //@{
    /** Sugested TTL between two metadata auto-refreshs.
     * The value (in seconds) may be provided in repoindex.xml:xpath:/repoindex@ttl.
     * Default is \a 0 - perform each auto-refresh request.
     */
    Date::Duration ttl() const;

    /** Set sugested TTL. */
    void setTtl( Date::Duration ttl_r );

    /** Lazy init sugested TTL. */
    void setProbedTtl( Date::Duration ttl_r ) const;

    /** Date of last refresh (if known). */
    Date lrf() const;

    /** Set date of last refresh. */
    void setLrf( Date lrf_r );
    //@}
    //
    /** \name Set of repos (repository aliases) to enable on next refresh.
     *
     * Per default new repositories are created in disabled state. But repositories
     * mentioned here will be created in enabled state on the next refresh.
     * Afterwards they get removed from the list.
     */
    //@{
    /** Container of repos. */
    typedef std::set<std::string> ReposToEnable;
    bool                          reposToEnableEmpty() const;
    ReposToEnable::size_type      reposToEnableSize() const;
    ReposToEnable::const_iterator reposToEnableBegin() const;
    ReposToEnable::const_iterator reposToEnableEnd() const;
    Iterable<ReposToEnable::const_iterator> reposToEnable() const
    { return makeIterable( reposToEnableBegin(), reposToEnableEnd() ); }

    /** Whether \c alias_r is mentioned in ReposToEnable. */
    bool repoToEnableFind( const std::string & alias_r ) const;

    /** Add \c alias_r to the set of ReposToEnable. */
    void addRepoToEnable( const std::string & alias_r );
    /** Remove \c alias_r from the set of ReposToEnable. */
    void delRepoToEnable( const std::string & alias_r );
    /** Clear the set of ReposToEnable. */
    void clearReposToEnable();
    //@}

    /** \name Set of repos (repository aliases) to disable on next refresh.
     *
     * Repositories mentioned here will be disabled on the next refresh, in case they
     * still exist. Afterwards they get removed from the list.
     */
    //@{
    /** Container of repos. */
    typedef std::set<std::string>  ReposToDisable;
    bool                           reposToDisableEmpty() const;
    ReposToDisable::size_type      reposToDisableSize() const;
    ReposToDisable::const_iterator reposToDisableBegin() const;
    ReposToDisable::const_iterator reposToDisableEnd() const;
    Iterable<ReposToDisable::const_iterator> reposToDisable() const
    { return makeIterable( reposToDisableBegin(), reposToDisableEnd() ); }

    /** Whether \c alias_r is mentioned in ReposToDisable. */
    bool repoToDisableFind( const std::string & alias_r ) const;

    /** Add \c alias_r to the set of ReposToDisable. */
    void addRepoToDisable( const std::string & alias_r );
    /** Remove \c alias_r from the set of ReposToDisable. */
    void delRepoToDisable( const std::string & alias_r );
    /** Clear the set of ReposToDisable. */
    void clearReposToDisable();
    //@}

    /** \name The original repo state as defined by the repoindex.xml upon last refresh.
     *
     * This state is remembered to detect any user modifications applied to the repos.
     * It may not be available for all repos or in plugin services. In this case all
     * changes requested by a service refresh are applied unconditionally.
     */
    //@{
    struct RepoState
    {
      bool	enabled;
      bool	autorefresh;
      unsigned	priority;

      RepoState()
	: enabled( false ), autorefresh( true ), priority( RepoInfo::defaultPriority() )
      {}
      RepoState( const RepoInfo & repo_r )
	: enabled( repo_r.enabled() ), autorefresh( repo_r.autorefresh() ), priority( repo_r.priority() )
      {}
      bool operator==( const RepoState & rhs ) const
      { return( enabled==rhs.enabled && autorefresh==rhs.autorefresh && priority==rhs.priority ); }
      bool operator!=( const RepoState & rhs ) const
      { return ! operator==( rhs ); }
      friend std::ostream & operator<<( std::ostream & str, const RepoState & obj );
    };
    typedef std::map<std::string,RepoState> RepoStates;

    /** Access the remembered repository states. */
    const RepoStates & repoStates() const;

    /** Remember a new set of repository states. */
    void setRepoStates( RepoStates newStates_r );
    //@}

  public:
    /**
     * Writes ServiceInfo to stream in ".service" format
     *
     * \param str stream where serialized version service is written
     */
    virtual std::ostream & dumpAsIniOn( std::ostream & str ) const;

    /**
     * Write an XML representation of this ServiceInfo object.
     *
     * \param str
     * \param content if not empty, produces <service ...>content</service>
     *                otherwise <service .../>
     */
    virtual std::ostream & dumpAsXmlOn( std::ostream & str, const std::string & content = "" ) const;

    class Impl;

  private:
      RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ServiceInfo */
  typedef shared_ptr<ServiceInfo> ServiceInfo_Ptr;
  /** \relates ServiceInfo */
  typedef shared_ptr<const ServiceInfo> ServiceInfo_constPtr;
  /** \relates ServiceInfo */
  typedef std::list<ServiceInfo> ServiceInfoList;

  /** \relates ServiceInfo Stream output */
  std::ostream & operator<<( std::ostream & str, const ServiceInfo & obj );


    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_REPOSITORY_H
