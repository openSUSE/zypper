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

#include "zypp/repo/ServiceType.h"
#include "zypp/repo/RepoInfoBase.h"


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ServiceInfo
  //
  /** */
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

    /**
     * Gets url to service
     *
     * \return url to service
     */
    Url url() const;

    /**
     * Sets url for this service
     *
     * \param url url to this service
     */
    void setUrl( const Url& url );

    /**
     * 
     */
    repo::ServiceType type() const;
    
    /**
     * Set service type.
     * 
     * \param type the new type
     */
    void setType( const repo::ServiceType & type );

    void setProbedType( const repo::ServiceType & t ) const;


    /** \name Set of catalogs (repository aliases) to enable on next refresh.
     *
     * Per default new repositories are created in disabled state. But repositories
     * mentioned here will be created in enabled state on the next refresh.
     * Afterwards they get removed from the list.
     */
    //@{
    /** Container of catalogs. */
    typedef std::set<std::string>    CatalogsToEnable;
    bool                             catalogsToEnableEmpty() const;
    CatalogsToEnable::size_type      catalogsToEnableSize() const;
    CatalogsToEnable::const_iterator catalogsToEnableBegin() const;
    CatalogsToEnable::const_iterator catalogsToEnableEnd() const;

    /** Wheter \c alias_r is mentioned in CatalogsToEnable. */
    bool catalogToEnableFind( const std::string & alias_r ) const;

    /** Add \c alias_r to the set of CatalogsToEnable. */
    void addCatalogToEnable( const std::string & alias_r );
    /** Remove \c alias_r to the set of CatalogsToEnable. */
    void delCatalogToEnable( const std::string & alias_r );
    //@}

    /** \name Set of catalogs (repository aliases) to disable on next refresh.
     *
     * Repositories mentioned here will be disabled on the next refresh, in case they
     * still exist. Afterwards they get removed from the list.
     */
    //@{
    /** Container of catalogs. */
    typedef std::set<std::string>     CatalogsToDisable;
    bool                              catalogsToDisableEmpty() const;
    CatalogsToDisable::size_type      catalogsToDisableSize() const;
    CatalogsToDisable::const_iterator catalogsToDisableBegin() const;
    CatalogsToDisable::const_iterator catalogsToDisableEnd() const;

    /** Wheter \c alias_r is mentioned in CatalogsToDisable. */
    bool catalogToDisableFind( const std::string & alias_r ) const;

    /** Add \c alias_r to the set of CatalogsToDisable. */
    void addCatalogToDisable( const std::string & alias_r );
    /** Remove \c alias_r to the set of CatalogsToDisable. */
    void delCatalogToDisable( const std::string & alias_r );
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
     */
    virtual std::ostream & dumpAsXMLOn(std::ostream & str) const;

    /**
     * Write an XML representation of this ServiceInfo object.
     *
     * \param str
     * \param content if not empty, produces <service ...>content</service>
     *                otherwise <service .../>
     */
    virtual std::ostream & dumpAsXMLOn(
        std::ostream & str, const std::string & content) const;

    class Impl;

  private:
      RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  typedef shared_ptr<ServiceInfo> ServiceInfo_Ptr;

  /** \relates ServiceInfo Stream output */
  std::ostream & operator<<( std::ostream & str, const ServiceInfo & obj );


    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_REPOSITORY_H
