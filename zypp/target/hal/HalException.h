/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/hal/HalException.h
 *
 *  \brief Hardware abstaction layer library wrapper.
 */
#ifndef ZYPP_TARGET_HAL_HALEXCEPTION_H
#define ZYPP_TARGET_HAL_HALEXCEPTION_H

#include "zypp/base/Exception.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"


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
          : zypp::Exception(_("Hal Exception"))
        {}

        /** Constructor taking complete hal error message.
         * This constructor is used to generate custom error
         * messages, in case, that no DBUS error is avaliable.
         * Use \ref ZYPP_THROW to throw exceptions.
         */
        HalException(const std::string &msg_r)
          : zypp::Exception(_("Hal Exception"))
          , e_name()
          , e_msg(msg_r)
        {}

        /** Constructor taking HAL (DBUS) error message components.
         * Use \ref ZYPP_THROW to throw exceptions.
         */
        HalException(const std::string &err_name, const std::string &err_msg)
          : zypp::Exception(_("Hal Exception"))
          , e_name(err_name)
          , e_msg(err_msg)
        {}

        /** Destructor.
         */
        virtual ~HalException() throw() {};

        /**
         * \return The HAL (DBUS) error name component.
         */
        const std::string & errorName() const
        {
          return e_name;
        }

        /**
         * \return The HAL (DBUS) error message component.
         */
        const std::string & errorMessage() const
        {
          return e_msg;
        }

      protected:
        virtual std::ostream & dumpOn( std::ostream & str ) const;

      private:
        std::string e_name;
        std::string e_msg;
      };

      /** Exception thrown if we were built without hal support (-DNO_HAL).
       */
      struct NoHalException: public Exception
      { NoHalException(); };

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
