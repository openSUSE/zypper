/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/hal/Hal.h
 *
*/
#ifndef ZYPP_TARGET_HAL_HAL_H
#define ZYPP_TARGET_HAL_HAL_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/Rel.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace hal
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Hal
      //
      /** Hardware abstaction layer.
       * Singleton.
      */
      class Hal
      {
        friend std::ostream & operator<<( std::ostream & str, const Hal & obj );

      public:
        /** Implementation  */
        class Impl;

      public:
        /** Singleton access. */
        static Hal & instance();

        /** Dtor */
        ~Hal();

      public:

        /** */
        bool query( const std::string & cap_r ) const;

        /** */
        bool query( const std::string & cap_r,
                    Rel op_r,
                    const std::string & val_r ) const;

      private:
        /** Singleton ctor. */
        Hal();

        /** Pointer to implementation */
        RW_pointer<Impl> _pimpl;
      };
      ///////////////////////////////////////////////////////////////////

      /** \relates Hal Stream output */
      std::ostream & operator<<( std::ostream & str, const Hal & obj );

      /////////////////////////////////////////////////////////////////
    } // namespace hal
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_HAL_HAL_H
