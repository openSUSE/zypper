/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/RepoException.h
 *
*/
#ifndef ZYPP_REPO_REPOEXCEPTION_H
#define ZYPP_REPO_REPOEXCEPTION_H

#include <iosfwd>
#include <string>

#include "zypp/base/Exception.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/RepoInfo.h"
#include "zypp/ServiceInfo.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    /** \name Repository related exceptions.
    */
    //@{

    /**
     * \short Exception for repository handling.
     */
    class RepoException : public Exception
    {
      public:
        RepoException();
        RepoException( const std::string & msg_r );
        RepoException( const RepoInfo & info );
        RepoException( const RepoInfo & info, const std::string & msg_r );
        virtual ~RepoException() throw();

        RepoInfo info()
        { return _info; }

        std::string alias()
        { return info().alias(); }

      protected:
        virtual std::ostream & dumpOn( std::ostream & str ) const;

      private:
        RepoInfo _info;
    };
    ///////////////////////////////////////////////////////////////////

    /**
     * The repository cache is not built yet
     * so you can't create the repostories from
     * the cache.
     */
    class RepoNotCachedException : public RepoException
    {
      public:
        RepoNotCachedException();
        RepoNotCachedException( const std::string & msg_r );
        RepoNotCachedException( const RepoInfo & info );
        RepoNotCachedException( const RepoInfo & info, const std::string & msg_r );
    };

    /**
     * thrown when it was impossible to
     * determine one url for this repo.
     */
    class RepoNoUrlException : public RepoException
    {
      public:
        RepoNoUrlException();
        RepoNoUrlException( const std::string & msg_r );
        RepoNoUrlException( const RepoInfo & info );
        RepoNoUrlException( const RepoInfo & info, const std::string & msg_r );
    };

    /**
     * thrown when it was impossible to
     * determine an alias for this repo.
     */
    class RepoNoAliasException : public RepoException
    {
      public:
        RepoNoAliasException();
        RepoNoAliasException( const std::string & msg_r );
        RepoNoAliasException( const RepoInfo & info );
        RepoNoAliasException( const RepoInfo & info, const std::string & msg_r );
    };

    /**
     * Thrown when the repo alias is found to be invalid.
     */
    class RepoInvalidAliasException : public RepoException
    {
    public:
      RepoInvalidAliasException();
      RepoInvalidAliasException( const std::string & msg_r );
      RepoInvalidAliasException( const RepoInfo & info );
      RepoInvalidAliasException( const RepoInfo & info, const std::string & msg_r );
    };

    /**
     * thrown when it was impossible to
     * match a repository
     */
    class RepoNotFoundException : public RepoException
    {
      public:
        RepoNotFoundException();
        RepoNotFoundException( const std::string & msg_r );
        RepoNotFoundException( const RepoInfo & info );
        RepoNotFoundException( const RepoInfo & info, const std::string & msg_r );
    };

    /**
     * Repository already exists and some unique
     * attribute can't be duplicated.
     */
    class RepoAlreadyExistsException : public RepoException
    {
      public:
        RepoAlreadyExistsException();
        RepoAlreadyExistsException( const std::string & msg_r );
        RepoAlreadyExistsException( const RepoInfo & info );
        RepoAlreadyExistsException( const RepoInfo & info, const std::string & msg_r );
    };

    /**
     * thrown when it was impossible to
     * determine this repo type.
     */
    class RepoUnknownTypeException : public RepoException
    {
      public:
        RepoUnknownTypeException();
        RepoUnknownTypeException( const std::string & msg_r );
        RepoUnknownTypeException( const RepoInfo & info );
        RepoUnknownTypeException( const RepoInfo & info, const std::string & msg_r );
    };

    /**
     * thrown when it was impossible to
     * use the raw metadata for this repo.
     */
    class RepoMetadataException : public RepoException
    {
      public:
        RepoMetadataException();
        RepoMetadataException( const std::string & msg_r );
        RepoMetadataException( const RepoInfo & info );
        RepoMetadataException( const RepoInfo & info, const std::string & msg_r );
    };

    //@}
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////

    /** \name Service related exceptions.
    */
    //@{

    /** Base Exception for service handling.
     */
    class ServiceException : public Exception
    {
      public:
        ServiceException();
        ServiceException( const std::string & msg_r );
        ServiceException( const ServiceInfo & service_r );
        ServiceException( const ServiceInfo & service_r, const std::string & msg_r );
        virtual ~ServiceException() throw();

        ServiceInfo service()
        { return _service; }

        std::string alias()
        { return service().alias(); }

     protected:
        virtual std::ostream & dumpOn( std::ostream & str ) const;

      private:
        ServiceInfo _service;
    };
    ///////////////////////////////////////////////////////////////////

    /** Service without alias was used in an operation.
     */
    class ServiceNoAliasException : public ServiceException
    {
      public:
        ServiceNoAliasException();
        ServiceNoAliasException( const std::string & msg_r );
        ServiceNoAliasException( const ServiceInfo & service_r );
        ServiceNoAliasException( const ServiceInfo & service_r, const std::string & msg_r );
    };

    /**
     * Thrown when the repo alias is found to be invalid.
     */
    class ServiceInvalidAliasException : public ServiceException
    {
    public:
      ServiceInvalidAliasException();
      ServiceInvalidAliasException( const std::string & msg_r );
      ServiceInvalidAliasException( const ServiceInfo & info );
      ServiceInvalidAliasException( const ServiceInfo & info, const std::string & msg_r );
    };

    /** Service already exists and some unique attribute can't be duplicated.
     */
    class ServiceAlreadyExistsException : public ServiceException
    {
      public:
        ServiceAlreadyExistsException();
        ServiceAlreadyExistsException( const std::string & msg_r );
        ServiceAlreadyExistsException( const ServiceInfo & service_r );
        ServiceAlreadyExistsException( const ServiceInfo & service_r, const std::string & msg_r );
    };

    /** Service has no or invalid url defined.
     */
    class ServiceNoUrlException : public ServiceException
    {
      public:
        ServiceNoUrlException();
        ServiceNoUrlException( const std::string & msg_r );
        ServiceNoUrlException( const ServiceInfo & service_r );
        ServiceNoUrlException( const ServiceInfo & service_r, const std::string & msg_r );
    };
    //@}


    /** \name PLUGIN Service related exceptions.
    */
    //@{

    /** PLUGIN Service related exceptions
     */
    class ServicePluginException : public ServiceException
    {
      public:
        ServicePluginException();
        ServicePluginException( const std::string & msg_r );
        ServicePluginException( const ServiceInfo & service_r );
        ServicePluginException( const ServiceInfo & service_r, const std::string & msg_r );
    };

    /** Service plugin has trouble providing the metadata but this should not be treated as error.
     */
    class ServicePluginInformalException : public ServicePluginException
    {
      public:
        ServicePluginInformalException();
        ServicePluginInformalException( const std::string & msg_r );
        ServicePluginInformalException( const ServiceInfo & service_r );
        ServicePluginInformalException( const ServiceInfo & service_r, const std::string & msg_r );
    };

    /** Service plugin is immutable.
     */
    class ServicePluginImmutableException : public ServicePluginException
    {
      public:
        ServicePluginImmutableException();
        ServicePluginImmutableException( const std::string & msg_r );
        ServicePluginImmutableException( const ServiceInfo & service_r );
        ServicePluginImmutableException( const ServiceInfo & service_r, const std::string & msg_r );
    };
    //@}

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_TAGFILE_PARSEEXCEPTION_H
