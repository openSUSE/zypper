/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/repo/RepoInfoBase.h
 *
 */
#ifndef REPOINFOBASE_H_
#define REPOINFOBASE_H_

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    
    
    ///////////////////////////////////////////////////////////////////
    //
    //    CLASS NAME : RepoInfoBase
    //
    /**
     * \short Base class implementing common features of \ref RepoInfo and
     *        \ref ServiceInfo.
     */
    class RepoInfoBase
    {
      friend std::ostream & operator<<( std::ostream & str, const RepoInfoBase & obj );

    public:
      RepoInfoBase();
      ~RepoInfoBase();

      /**
       * unique identifier for this source. If not specified
       * It should be generated from the base url.
       *
       * Normally, in a .repo file the section name is used
       * ( [somerepo] )
       */
      std::string alias() const;

      /**
       * Same as alias(), just escaped in a way to be a valid file name.
       */
      std::string escaped_alias() const;

      /**
       * \short Repository short label
       *
       * Short label or description of the repository, to be used on
       * the user interface.
       * ie: "SUSE Linux 10.2 updates"
       */
      std::string name() const;

      /**
       * If enabled is false, then this repository must be ignored as if does
       * not exists, except when checking for duplicate alias.
       */
      bool enabled() const;

      /**
       * If true, the repostory must be refreshed before creating resolvables
       * from it
       */
      bool autorefresh() const;

      /**
       * \short File where this repo was read from
       *
       * \note could be an empty pathname for repo
       * infos created in memory.
       */
       Pathname filepath() const;


    public:

      /**
       * set the repository alias \see alias
       * \param alias
       */
      RepoInfoBase & setAlias( const std::string &alias );

      /**
       * set the repository name \see name
       * \param name
       */
      RepoInfoBase & setName( const std::string &name );

      /**
       * enable or disable the repository \see enabled
       * \param enabled
       */
      RepoInfoBase & setEnabled( bool enabled );

      /**
       * enable or disable autorefresh \see autorefresh
       * \param enabled
       */
      RepoInfoBase & setAutorefresh( bool autorefresh );

      /**
       * \short set the path to the .repo file
       *
       * The path to the .repo file where this repository
       * was defined, or empty if nowhere.
       *
       * \param path File path
       */
      RepoInfoBase & setFilepath( const Pathname &filename );

      /**
       * Write a human-readable representation of this RepoInfoBase object
       * into the \a str stream. Useful for logging.
       */
      virtual std::ostream & dumpOn( std::ostream & str ) const;

      /**
       * Write this RepoInfoBase object into \a str in
       * a <tr>.repo</tt> (ini) file format.
       */
      virtual std::ostream & dumpAsIniOn( std::ostream & str ) const;

      class Impl;
    private:
      /** Pointer to implementation */
      RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates RepoInfoBase Stream output */
    std::ostream & operator<<( std::ostream & str, const RepoInfoBase & obj );

    inline bool operator<( const RepoInfoBase & lhs, const RepoInfoBase & rhs )
    { return lhs.alias() < rhs.alias(); }


    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif /*REPOINFOBASE_H_*/
