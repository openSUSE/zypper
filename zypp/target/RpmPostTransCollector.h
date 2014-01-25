/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/RpmPostTransCollector.h
 */
#ifndef ZYPP_TARGET_RPMPOSTTRANSCOLLECTOR_H
#define ZYPP_TARGET_RPMPOSTTRANSCOLLECTOR_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/ManagedFile.h"
#include "zypp/Pathname.h"

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp:posttrans"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace target
  {
    ///////////////////////////////////////////////////////////////////
    /// \class RpmPostTransCollector
    /// \brief Extract and remember %posttrans scripts for later execution
    /// \todo Maybe embedd this into the TransactionSteps.
    ///////////////////////////////////////////////////////////////////
    class RpmPostTransCollector
    {
      friend std::ostream & operator<<( std::ostream & str, const RpmPostTransCollector & obj );
      friend std::ostream & dumpOn( std::ostream & str, const RpmPostTransCollector & obj );

      public:
        /** Default ctor */
        RpmPostTransCollector( const Pathname & root_r );

        /** Dtor */
        ~RpmPostTransCollector();

      public:
	/** Extract and remember a packages %posttrans script for later execution.
	 * \return whether a script was collected.
	 */
	bool collectScriptFromPackage( ManagedFile rpmPackage_r );

	/** Execute te remembered scripts. */
	void executeScripts();

	/** Discard all remembered scrips. */
	void discardScripts();

      public:
        class Impl;              ///< Implementation class.
      private:
        RW_pointer<Impl> _pimpl; ///< Pointer to implementation.
    };

    /** \relates RpmPostTransCollector Stream output */
    std::ostream & operator<<( std::ostream & str, const RpmPostTransCollector & obj );

    /** \relates RpmPostTransCollector Verbose stream output */
    std::ostream & dumOn( std::ostream & str, const RpmPostTransCollector & obj );

  } // namespace target
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_RPMPOSTTRANSCOLLECTOR_H
