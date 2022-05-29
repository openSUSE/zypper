/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/PluginRepoverification.h
 */
#ifndef ZYPP_REPO_PLUGINREPOVERIFICATION_H
#define ZYPP_REPO_PLUGINREPOVERIFICATION_H

#include <iosfwd>

#include <zypp/Globals.h>
#include <zypp/RepoInfo.h>
#include <zypp/FileChecker.h>
#include <zypp/base/PtrTypes.h>

///////////////////////////////////////////////////////////////////
namespace zypp_private
{
  using namespace zypp;
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {
    ///////////////////////////////////////////////////////////////////
    /// \class PluginRepoverificationCheckException
    /// \brief Exceptiontype thrown if a plugins verification fails.
    ///////////////////////////////////////////////////////////////////
    class PluginRepoverificationCheckException : public FileCheckException
    {
    public:
      PluginRepoverificationCheckException( const std::string &msg )
      : FileCheckException(msg)
      {}
    };

    ///////////////////////////////////////////////////////////////////
    /// \class PluginRepoverification
    /// \brief Repository metadata verification beyond GPG.
    ///
    /// Implements the repoverification plugin setup and workflow. Also
    /// serves as factory for a file \ref FileChecker that can be passed
    /// to e.g. \ref repo::Downloader when fetching the repos master index file.
    ///
    /// If a root dir is defined, plugin scripts will be  executed chrooted.
    ///
    /// \see \ref plugin-repoverification for more details.
    ///////////////////////////////////////////////////////////////////
    class PluginRepoverification
    {
      friend std::ostream & operator<<( std::ostream & str, const PluginRepoverification & obj );
      friend std::ostream & dumpOn( std::ostream & str, const PluginRepoverification & obj );
      friend bool operator==( const PluginRepoverification & lhs, const PluginRepoverification & rhs );

      using ExceptionType = PluginRepoverificationCheckException;

    public:
      /** Default ctor, do nothing */
      PluginRepoverification();

      /** Ctor monitoring a \a plugindir_r and optional chroot for plugin execution */
      PluginRepoverification( Pathname plugindir_r, Pathname chroot_r = Pathname() );

      /** Dtor */
      ~PluginRepoverification();

    public:
      /** Whether the last \ref checkIfNeeded found plugins to execute at all. */
      bool isNeeded() const;

      /** Checks whether there are plugins to execute at all. */
      bool checkIfNeeded();

    public:
      ///////////////////////////////////////////////////////////////////
      /// \class Checker
      /// \brief \ref FileChecker checking all repoverification plugins.
      ///////////////////////////////////////////////////////////////////
      class Checker
      {
      public:
        ~Checker();

        /** Check the downloaded master index file.
         * \throws PluginRepoverificationCheckException If a plugins verification fails.
         */
        void operator()( const Pathname & file_r ) const;

      public:
        class Impl;                 ///< Implementation class.
      private:
        friend class PluginRepoverification;  ///< Factory for \ref Checker
        Checker( Impl* pimpl );
      private:
        RW_pointer<Impl> _pimpl;    ///< Pointer to implementation (ref).
      };
      ///////////////////////////////////////////////////////////////////

      /** \ref FileChecker factory remembering the location of the master index files GPG signature and key. */
      Checker getChecker( const Pathname & sigpathLocal_r, const Pathname & keypathLocal_r, const RepoInfo & repo_r ) const;

    public:
      class Impl;                 ///< Implementation class.
    private:
      RW_pointer<Impl> _pimpl;    ///< Pointer to implementation (ref).
    };

    /** \relates PluginRepoverification Stream output */
    std::ostream & operator<<( std::ostream & str, const PluginRepoverification & obj );

    /** \relates PluginRepoverification Verbose stream output */
    std::ostream & dumOn( std::ostream & str, const PluginRepoverification & obj );

    /** \relates PluginRepoverification */
    bool operator==( const PluginRepoverification & lhs, const PluginRepoverification & rhs );

    /** \relates PluginRepoverification */
    inline bool operator!=( const PluginRepoverification & lhs, const PluginRepoverification & rhs )
    { return !( lhs == rhs ); }

  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REPO_PLUGINREPOVERIFICATION_H
