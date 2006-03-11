/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/modalias/Modalias.h
 *
*/
#ifndef ZYPP_TARGET_MODALIAS_MODALIAS_H
#define ZYPP_TARGET_MODALIAS_MODALIAS_H

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
    namespace modalias
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Modalias
      //
      /** Hardware abstaction layer.
       * Singleton.
      */
      class Modalias
      {
        friend std::ostream & operator<<( std::ostream & str, const Modalias & obj );

      public:
        /** Implementation  */
        class Impl;

      public:
        /** Singleton access. */
        static Modalias & instance();

        /** Dtor */
        ~Modalias();

      public:

        /** */
        bool query( const std::string & cap_r ) const;

        /** */
        bool query( const std::string & cap_r,
                    Rel op_r,
                    const std::string & val_r ) const;

      private:
        /** Singleton ctor. */
        Modalias();

        /** Pointer to implementation */
        RW_pointer<Impl> _pimpl;
      };
      ///////////////////////////////////////////////////////////////////

      /** \relates Modalias Stream output */
      std::ostream & operator<<( std::ostream & str, const Modalias & obj );

      /////////////////////////////////////////////////////////////////
    } // namespace modalias
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_MODALIAS_MODALIAS_H
