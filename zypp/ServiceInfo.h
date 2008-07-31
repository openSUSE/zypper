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

#include <string>

#include "zypp/Url.h"

#include "zypp/repo/RepoInfoBase.h"


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(ServiceInfo);

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

  public:

    /**
     * Sets url for this service
     *
     * \param url url to this service
     */
    void setUrl( const Url& url );

  public:
    /**
     * Writes ServiceInfo to stream in ".service" format
     *
     * \param str stream where serialized version service is written
     */
    virtual std::ostream & dumpAsIniOn( std::ostream & str ) const;

    class Impl;

  private:
      RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ServiceInfo Stream output */
  std::ostream & operator<<( std::ostream & str, const ServiceInfo & obj );

  /** \relates ServiceInfo */
  inline bool operator==( const ServiceInfo & lhs, const ServiceInfo & rhs )
  { return lhs.alias() == rhs.alias(); }

  /** \relates ServiceInfo */
  inline bool operator!=( const ServiceInfo & lhs, const ServiceInfo & rhs )
  { return lhs.alias() != rhs.alias(); }

  /** \relates ServiceInfo */
  inline bool operator<( const ServiceInfo & lhs, const ServiceInfo & rhs )
  { return lhs.alias() < rhs.alias(); }

    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_REPOSITORY_H
