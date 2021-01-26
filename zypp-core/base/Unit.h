/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Unit.h
 *
*/
#ifndef ZYPP_BASE_UNIT_H
#define ZYPP_BASE_UNIT_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Unit
    //
    /** Simple handling of Units.
     *
     * Unit stores factor and symbol, and a precision value for printing.
     * \ref form builds a string from a value according to the format
     * specification.
     * \code
     * static const Unit B( 1, "B", 0 );
     * static const Unit K( 1024, "K", 1 );
     * static const Unit M( 1048576, "M", 1 );
     * static const Unit G( 1073741824, "G", 2 );
     * static const Unit T( 1099511627776, "T", 3 );
     * \endcode
    */
      class Unit
      {
      public:
        typedef long long ValueType;

        /** Default ctor */
        Unit()
        : _factor( 1 )
        , _prec( 0 )
        {}

        /** ctor */
        Unit( ValueType factor_r, std::string symbol_r, unsigned prec_r )
        : _factor( factor_r )
        , _symbol( symbol_r )
        , _prec( prec_r )
        {}

        ValueType factor() const
        { return _factor; }

        const std::string & symbol() const
        { return _symbol; }

        unsigned prec() const
        { return _prec; }

        /** Build string representation of \a val_r. */
        std::string form( ValueType val_r,
                          unsigned field_width_r = 0,
                          unsigned unit_width_r  = 1 ) const
        { return form( val_r, field_width_r, unit_width_r, _prec ); }

        std::string form( ValueType val_r,
                          unsigned field_width_r,
                          unsigned unit_width_r,
                          unsigned prec_r ) const
        { return form( double(val_r)/_factor, _symbol,
                       field_width_r, unit_width_r, prec_r ); }


        static std::string form( double val_r,
                                 const std::string & symbol_r,
                                 unsigned field_width_r,
                                 unsigned unit_width_r,
                                 unsigned prec_r );

      private:
        ValueType   _factor;
        std::string _symbol;
        unsigned    _prec;
      };
    ///////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_UNIT_H
