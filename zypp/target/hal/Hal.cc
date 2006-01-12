/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/hal/Hal.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/target/hal/Hal.h"

using std::endl;

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
      //	CLASS NAME : Hal::Impl
      //
      /** Hal implementation. */
      struct Hal::Impl
      {
        /** Ctor. */
        Impl()
        {}

        /** \todo Implement it.
         * And maybee std::ostream & operator<< Hal::Impl below too.
         * return libhal vetsion ore something like that.
        */
        std::string query( const std::string & val_r ) const
        { return std::string(); }

      public:
        /** Offer default Impl. */
        static shared_ptr<Impl> nullimpl()
        {
          static shared_ptr<Impl> _nullimpl( new Impl );
          return _nullimpl;
        }
      };
      ///////////////////////////////////////////////////////////////////

      /** \relates Hal::Impl Stream output */
      inline std::ostream & operator<<( std::ostream & str, const Hal::Impl & obj )
      {
        return str << "Hal::Impl";
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Hal
      //
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Hal::Hal
      //	METHOD TYPE : Ctor
      //
      Hal::Hal()
      : _pimpl( Impl::nullimpl() )
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Hal::~Hal
      //	METHOD TYPE : Dtor
      //
      Hal::~Hal()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Hal::instance
      //	METHOD TYPE : Hal &
      //
      Hal & Hal::instance()
      {
        static Hal _singleton;
        return _singleton;
      }

      ///////////////////////////////////////////////////////////////////
      // Foreward to implenemtation
      ///////////////////////////////////////////////////////////////////

      std::string Hal::query( const std::string & val_r ) const
      { return _pimpl->query( val_r ); }

      /******************************************************************
      **
      **	FUNCTION NAME : operator<<
      **	FUNCTION TYPE : std::ostream &
      */
      std::ostream & operator<<( std::ostream & str, const Hal & obj )
      {
        return str << *obj._pimpl;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace hal
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
