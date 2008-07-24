/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Service.h
 *
*/
#ifndef ZYPP_SERVICE_H
#define ZYPP_SERVICE_H

#include <string>

#include "zypp/Url.h"
#include "zypp/Pathname.h"


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Service
    //
    /** */
    class Service
    {
    public:
        /** Default ctor creates \ref noService.*/
        Service();

        /**
         *  Service with this name.
         *
         * \param name unique name of service
         */
        Service( const std::string& name );

        /**
         * Service with name and its URL
         *
         * \param name unique name of service
         * \param url url to service
         */
        Service( const std::string& name, const Url& url );

    public:
        /** Represents no \ref Service. */
        static const Service noService;

    public:
        /**
         * Gets unique name
         *
         * \return name of service
         */
        std::string name() const;

        /**
         * Gets url to service
         *
         * \return url to service
         */
        Url url() const;

        /**
         * Returns 'enabled' flag of the services.
         * 
         * Disabled services imply disabled repositories of these services
         * and they won't be refreshed by \ref RepoManager::refreshServices().
         * 
         * Enabled services will be refreshed by \ref RepoManager::refreshServices().
         */
        bool enabled() const;

        /**
         * Gets from which file is this service loaded or saved.
         *
         * \note this is empty for newly created file until it is saved
         * \return path to file storing this service
         */
        Pathname location() const;

    public:
        
        /**
         * Sets file where this service is stored.
         *
         * \warning don't use this function, only parses and serializer can
         *          use it
         * \param location path where service is stored
         */
        void setLocation( const Pathname& location );

        /**
         * Sets url for this service
         *
         * \param url url to this service
         */
        void setUrl( const Url& url );

        /**
         * Sets name for this service
         *
         * \param name new name of this service
         */
        void setName( const std::string& name );

        /**
         * Sets enabled status of the services.
         * \param enabled The desired status.
         */
        void setEnabled( const bool enabled );

   public:
        /**
         * Writes Service to stream in ".service" format
         *
         * \param str stream where serialized version service is written
         */
        void dumpServiceOn( std::ostream & str ) const;

        class Impl;

    private:
        RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Service Stream output */
    std::ostream & operator<<( std::ostream & str, const Service & obj );

    /** \relates Service */
    inline bool operator==( const Service & lhs, const Service & rhs )
    { return lhs.name() == rhs.name(); }

    /** \relates Service */
    inline bool operator!=( const Service & lhs, const Service & rhs )
    { return lhs.name() != rhs.name(); }

    /** \relates Service */
    inline bool operator<( const Service & lhs, const Service & rhs )
    { return lhs.name() < rhs.name(); }

    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_REPOSITORY_H
