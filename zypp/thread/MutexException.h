/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/thread/MutexException.h
 *
*/
#ifndef   ZYPP_THREAD_MUTEXEXCEPTION_H
#define   ZYPP_THREAD_MUTEXEXCEPTION_H

#include "zypp/base/Exception.h"


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace thread
  { //////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : MutexException
    //
    /** Exception type thrown on mutex errors.
     */
    class MutexException: public zypp::Exception
    {
    public:
      MutexException()
        : zypp::Exception( ::zypp::Exception::strErrno(errno))
      {}

      MutexException(const std::string &msg)
        : zypp::Exception( msg)
      {}

      virtual ~MutexException() throw()
      {}
    };


    //////////////////////////////////////////////////////////////////
  } // namespace thread
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif // ZYPP_THREAD_MUTEXEXCEPTION_H
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
