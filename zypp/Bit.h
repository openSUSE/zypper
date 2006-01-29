/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Bit.h
 *
*/
#ifndef ZYPP_BIT_H
#define ZYPP_BIT_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
  /**
   * \todo Use boost::mpl library to assert constraints
   * at compiletime! There various like (_IntT is an integral type)
   * (begin+size < maxbits) or ( field dependent
   * constants must be within the range defined by size ).
  */
  namespace bit
  { /////////////////////////////////////////////////////////////////

    namespace bit_detail
    {
      /** Generate constants with \a _size trailing '1'-bits */
      template<class _IntT, unsigned _size>
        struct Gen1Bits
        {
          static const _IntT value = (Gen1Bits<_IntT,_size-1>::value << 1)+1;
        };
      /** Specialization for \a _length 0 */
      template<class _IntT>
        struct Gen1Bits<_IntT, 0>
        {
          static const _IntT value = 0;
        };
    }

    /** Number of bits available in \a _IntT. */
    template<class _IntT>
      struct MaxBits
      {
        static const unsigned value = (sizeof(_IntT)*8);
      };

    /** For printing bits. */
    template<class _IntT>
      inline std::string asString( _IntT val, char zero = '0', char one = '1' )
      {
        std::string s( MaxBits<_IntT>::value, zero );
        for( unsigned i = MaxBits<_IntT>::value; i; )
          {
            --i;
            if ( val & (_IntT)1 )
              s[i] = one;
            val = val >> 1;
          };
        return s;
      }

    /** A bitmaks of \a _size 1-bits starting at bit \a _begin. */
    template<class _IntT, unsigned _begin, unsigned _size>
      struct Mask
      {
        static const _IntT value    = bit_detail::Gen1Bits<_IntT,_size>::value << _begin;
        static const _IntT inverted = ~value;
      };

    /** Range of bits starting at bit \_begin with length \a _size. */
    template<class _IntT, unsigned _begin, unsigned _size>
      struct Range
      {
        typedef MaxBits<_IntT>           MaxBits;
        typedef Mask<_IntT,_begin,_size> Mask;

        static const unsigned begin  = _begin;
        static const unsigned size   = _size;
        static const unsigned end    = _begin + _size;

        static const unsigned minval = 1 << _begin;
      };
    /** Range specialisation for (illegal) zero \a _size.
     * Fore error at compiletime. Currently because types
     * and values are undefined
    */
    template<class _IntT, unsigned _begin>
      struct Range<_IntT, _begin, 0>
      {};

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : BitField
    //
    /** An integral type used as BitField.
     *
     * Most methods exist as templated and nontemplated
     * version. The nontemplated operates on the complete
     * BitField, while the tamplated ones are restricted
     * to the given Range.
     * \code
     * BitField<char> bf;                // 00000000
     * typedef Range<char,2,3> SubField; // bits 2,3,4 in a char field
     *
     * bf<SubField>.assign( -1 );        // assign SubField in -1
     *                                   // to SubField in bf.
     *                                   // 00011100
     * bf.assign( -1 );                  // assign -1 to bf
     *                                   // 11111111
     * bf<SubField>.assign( 0 );         // 11100011
     * \endcode
    */
    template<class _IntT>
      class BitField  : public Range<_IntT, 0, MaxBits<_IntT>::value>
      {
      public:
        /** Default ctor: zero. */
        BitField()
        : _value( (_IntT)0 )
        {}
        /** Ctor taking an \a _IntT. */
        BitField( const _IntT & value_r )
        : _value( value_r )
        {}

        /** Return the value. */
        template<class _Range>
          _IntT value() const
          {
            return _value & _Range::Mask::value;
          }
        _IntT value() const
        {
          return _value;
        }

        /** Value as bit string. */
        template<class _Range>
          std::string asString() const
          {
            return bit::asString( _value & _Range::Mask::value, '_' );
          }
        std::string asString() const
        {
          return bit::asString( _value, '_' );
        }

        /** Assign Range in \a rhs to \c this. */
        template<class _Range>
          BitField & assign( _IntT rhs )
          {
            _value = (_value & _Range::Mask::inverted)
                   | (rhs & _Range::Mask::value);
            return *this;
          }
        BitField & assign( _IntT rhs )
        {
          _value = rhs;
          return *this;
        }

        /** Test for equal value within a Range. */
        template<class _Range>
          bool isEqual( _IntT rhs ) const
          {
            return (_value & _Range::Mask::value)
                == (rhs & _Range::Mask::value);
          }
        bool isEqual( _IntT rhs ) const
        {
          return _value == rhs;
        }

      private:
        _IntT _value;
      };
    ///////////////////////////////////////////////////////////////////

    /** \relates BitField Stream output */
    template<class _IntT>
      std::ostream & operator<<( std::ostream & str, const BitField<_IntT> & obj )
      {
        return str << obj.asString();
      }

    /** \relates BitField */
    template<class _IntT>
      inline bool operator==( const BitField<_IntT> & lhs, const BitField<_IntT> & rhs )
      { return lhs.value() == rhs.value(); }

    /** \relates BitField */
    template<class _IntT>
      inline bool operator!=( const BitField<_IntT> & lhs, const BitField<_IntT> & rhs )
      { return ! (lhs == rhs); }

    /////////////////////////////////////////////////////////////////
  } // namespace bit
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BIT_H
