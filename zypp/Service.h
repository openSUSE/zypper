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
#include "zypp/RepoManager.h"


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
         */
        Service( const std::string& name );

        /**
         * Creates service with name and it's url.
         * Used for adding new services.
         * \note internal, do not used outside libzypp
         */
        Service( const std::string& name, const Url& url );

    public:
        /** Represents no \ref Service. */
        static const Service noService;

    public:
        std::string name() const;

        Url url() const;

        Pathname location() const;

    public:

        void setLocation( const Pathname& location );

        void setUrl( const Url& url );

        void setName( const std::string& name );

   public:
        /**
         * Writes Service to stream in ".service" format
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
