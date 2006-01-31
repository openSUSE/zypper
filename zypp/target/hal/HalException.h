/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/hal/HalContext.h
 *
 *  \brief Hardware abstaction layer library wrapper.
 */
#ifndef ZYPP_TARGET_HAL_HALEXCEPTION_H
#define ZYPP_TARGET_HAL_HALEXCEPTION_H

#include <zypp/base/Exception.h>
#include <string>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace target
  { //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    namespace hal
    { ////////////////////////////////////////////////////////////////


      ////////////////////////////////////////////////////////////////
      //
      // CLASS NAME : HalException
      //
      /** Hardware abstaction layer exception.
       * Just inherits Exception to separate hal exceptions.
       */
      class HalException: public zypp::Exception
      {
      public:
        /** Default constructor.
         * Use \ref ZYPP_THROW to throw exceptions.
         */
        HalException()
          : zypp::Exception("Hal Exception")
        {}

        /** Constructor taking hal error message.
         * Use \ref ZYPP_THROW to throw exceptions.
         */
        HalException(const std::string &msg)
          : zypp::Exception(msg)
        {}

        /** Destructor.
         */
        virtual ~HalException() throw() {};
      };


      ////////////////////////////////////////////////////////////////
    } // namespace hal
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
  } // namespace target
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif // ZYPP_TARGET_HAL_HALEXCEPTION_H

/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
