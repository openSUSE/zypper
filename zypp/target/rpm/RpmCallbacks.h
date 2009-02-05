/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmCallbacks.h
 *
*/

#ifndef ZYPP_TARGET_RPM_RPMCALLBACKS_H
#define ZYPP_TARGET_RPM_RPMCALLBACKS_H

#include <iosfwd>

#include "zypp/Url.h"
#include "zypp/Callback.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

namespace zypp
{
namespace target
{
namespace rpm
{

///////////////////////////////////////////////////////////////////
// Reporting progress of package removing
///////////////////////////////////////////////////////////////////
struct RpmRemoveReport : public callback::ReportBase
{

  enum Action {
    ABORT,  // abort and return error
    RETRY,   // retry
    IGNORE   // ignore
  };

  /** Start the operation */
  virtual void start( const std::string & name )
  {}
  /**
   * Inform about progress
   * Return true on abort
   */
  virtual bool progress( unsigned percent )
  { return false; }

  virtual Action problem( Exception & excpt_r )
  { return ABORT; }

  /** Additional rpm output to be reported in \ref finish in case of success. */
  virtual void finishInfo( const std::string & info_r )
  {}

  /** Finish operation in case of success */
  virtual void finish()
  {}
  /** Finish operation in case of fail, report fail exception */
  virtual void finish( Exception & excpt_r )
  {}
};

///////////////////////////////////////////////////////////////////
// Reporting progress of package installation
///////////////////////////////////////////////////////////////////
struct RpmInstallReport : public callback::ReportBase
{

  enum Action {
    ABORT,  // abort and return error
    RETRY,   // retry
    IGNORE   // ignore
  };

  /** Start the operation */
  virtual void start( const Pathname & name )
  {}
  /**
   * Inform about progress
   * Return false on abort
   */
  virtual bool progress( unsigned percent )
  { return true; }

  /** Additional rpm output to be reported in \ref finish in case of success. */
  virtual void finishInfo( const std::string & info_r )
  {}

  /** Finish operation in case of success */
  virtual void finish()
  {}

  virtual Action problem( Exception & excpt_r )
  { return ABORT; }

  /** Finish operation in case of fail, report fail exception */
  virtual void finish( Exception & excpt_r )
  {}
};

} // namespace rpm
} // namespace target
} // namespace zypp

#endif // ZYPP_TARGET_RPM_RPMCALLBACKS_H
