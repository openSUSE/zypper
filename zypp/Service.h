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
        typedef std::vector<std::string> RepoContainer;
        typedef RepoContainer::const_iterator const_iterator;
        typedef RepoContainer::size_type size_type;

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

        void setLocation( const Pathname& location ) const;

        void setUrl( const Url& url );

    public:

        /** Whether \ref Service contains solvables. */
        bool empty() const;

        /** Number of solvables in \ref Service. */
        size_type size() const;

        /** Iterator to the first \ref Solvable. */
        const_iterator begin() const;

        /** Iterator behind the last \ref Solvable. */
        const_iterator end() const;

   public:
        /**
         * Refreshes local information about service.
         * It can addi, remove or modify repositories.
         * it is const due to use Service in set or map and name doesn't change
         */
        void refresh( RepoManager& repomanager) const;

        /**
         * Writes Service to stream in ".service" format
         */
        void dumpServiceOn( std::ostream & str ) const;

        /**
         * set repositories by alias.
         * Used only during local loading, for update repository use refresh
         */
        template<typename InputIterator>
        void setRepos(InputIterator begin, InputIterator end)
        {
          for_(it,begin,end)
            addRepo(*it);
        }

        void addRepo(const std::string& alias);

        class Impl;

    private:
        mutable RW_pointer<Impl> _pimpl;
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
