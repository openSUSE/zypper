/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PluginExecutor.h
 */
#ifndef ZYPP_PLUGINEXECUTOR_H
#define ZYPP_PLUGINEXECUTOR_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/PluginScript.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class PluginExecutor
  /// \brief Parallel execution of stateful PluginScripts
  ///
  /// Sent PluginFrames are distributed to all open PluginScripts and
  /// need to be receipted by sending back either \c ACK or \c _ENOMETHOD
  /// command.
  ///
  /// All PluginScripts receive an initial \c PLUGINBEGIN frame, containing
  /// a \c userdata header if \ref ZConfig::userData are defined.
  /// \see also zypper '--userdata' option
  ///
  /// A final \c PLUGINEND frame is sent and open scripts are closed, when the
  /// executors last reference goes out of scope. Failing PluginScripts are
  /// closed immediately.
  ///
  /// \see PluginScript
  /// \ingroup g_RAII
  ///////////////////////////////////////////////////////////////////
  class PluginExecutor
  {
    friend std::ostream & operator<<( std::ostream & str, const PluginExecutor & obj );
    friend bool operator==( const PluginExecutor & lhs, const PluginExecutor & rhs );

    public:
      /** Default ctor: Empty plugin list */
      PluginExecutor();

      /** Dtor: Send \c PLUGINEND and close all plugins */
      ~PluginExecutor();

    public:
      /**  Validate object in a boolean context: There are plugins waiting for input */
      explicit operator bool() const
      { return !empty(); }

      /** Whether no plugins are waiting */
      bool empty() const;

      /** Number of open plugins */
      size_t size() const;

    public:
      /** Find and launch plugins sending \c PLUGINBEGIN.
       *
       * If \a path_r is a directory all executable files within are
       * expected to be plugins. Otherwise \a path_r must point to an
       * executable plugin.
       */
      void load( const Pathname & path_r );

      /** Send \ref PluginFrame to all open plugins.
       * Failed plugins are removed from the execution list.
       */
      void send( const PluginFrame & frame_r );

    public:
      class Impl;		///< Implementation class.
    private:
      RW_pointer<Impl> _pimpl;	///< Pointer to implementation.
  };

  /** \relates PluginExecutor Stream output */
  std::ostream & operator<<( std::ostream & str, const PluginExecutor & obj );

  /** \relates PluginExecutor Comparison based on reference. */
  inline bool operator==( const PluginExecutor & lhs, const PluginExecutor & rhs )
  { return ( lhs._pimpl == rhs._pimpl ); }

  /** \relates PluginExecutor Comparison based on reference. */
  inline bool operator!=( const PluginExecutor & lhs, const PluginExecutor & rhs )
  { return( ! operator==( lhs, rhs ) ); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PLUGINEXECUTOR_H
