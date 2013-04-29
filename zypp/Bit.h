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
        typedef _IntT IntT;
        static const unsigned value = (sizeof(IntT)*8);
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
        typedef _IntT IntT;
        static const IntT value    = bit_detail::Gen1Bits<IntT,_size>::value << _begin;
        static const IntT inverted = ~value;
      };

    /** Range of bits starting at bit \a _begin with length \a _size. */
    template<class _IntT, unsigned _begin, unsigned _size>
      struct Range
      {
        typedef _IntT IntT;
        typedef zypp::bit::MaxBits<IntT>           MaxBits;
        typedef zypp::bit::Mask<IntT,_begin,_size> Mask;

        static const unsigned begin  = _begin;
        static const unsigned size   = _size;
        static const unsigned end    = _begin + _size;
      };
    /** Range specialisation for (illegal) zero \a _size.
     * Force error at compiletime. Currently because types
     * and values are undefined
    */
    template<class _IntT, unsigned _begin>
      struct Range<_IntT, _begin, 0>
      {};

    /** A value with in a Range.
     * \code
     * typedef Range<char,2,3> SubField; // bits 2,3,4 in a char field
     * SubField::Mask::value;            // 00011100
     * RangeValue<SubField,0>::value;    // 00000000
     * RangeValue<SubField,1>::value;    // 00000100
     * RangeValue<SubField,2>::value;    // 00001000
     * RangeValue<SubField,3>::value;    // 00001100
     * \endcode
    */
    template<class _Range, typename _Range::IntT _value>
      struct RangeValue
      {
        typedef _Range                RangeT;
        typedef typename _Range::IntT IntT;

        static const IntT value = _value << RangeT::begin;
      };

    /** A single 1-bit within a Range.
     * \code
     * typedef Range<char,2,3> SubField; // bits 2,3,4 in a char field
     * SubField::Mask::value;            // 00011100
     * RangeBit<SubField,0>::value;      // 00000100
     * RangeBit<SubField,1>::value;      // 00001000
     * RangeBit<SubField,2>::value;      // 00010000
     * \endcode
    */
    template<class _Range, unsigned _pos>
      struct RangeBit
      {
        typedef _Range                RangeT;
        typedef typename _Range::IntT IntT;

        static const IntT value = IntT(1) << (RangeT::begin + _pos);
      };

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

      public:
        /** Validate in a boolean context. */
        explicit operator bool() const
        { return _value != (_IntT)0; }

      public:
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

       public:

         /** Set or unset bits of \a rhs. */
        template<class _Range>
            BitField & set( _IntT rhs, bool doset_r )
            { return set( (rhs & _Range::Mask::value), doset_r ); }

        BitField & set( _IntT rhs, bool doset_r )
        { return doset_r ? set( rhs ) : unset( rhs ); }

        /** Set bits of \a rhs. */
        template<class _Range>
            BitField & set( _IntT rhs )
            { return set( rhs & _Range::Mask::value ); }

        BitField & set( _IntT rhs )
        { _value |= rhs; return *this; }

        /** Unset bits of \a rhs. */
        template<class _Range>
            BitField & unset( _IntT rhs )
            { return unset( rhs & _Range::Mask::value ); }

        BitField & unset( _IntT rhs )
        { _value &= ~rhs; return *this; }

        /** Test whether \b all bits of \a rhs are set. */
        template<class _Range>
            bool test( _IntT rhs )
            { return test( rhs & _Range::Mask::value ); }

        bool test( _IntT rhs ) const
        { return (_value & rhs) == rhs; }

        /** Test whether \b at \b least \b one bit of \a rhs is set. */
        template<class _Range>
            bool testAnyOf( _IntT rhs )
            { return testAnyOf( rhs & _Range::Mask::value ); }

        bool testAnyOf( _IntT rhs ) const
        { return (_value & rhs); }

      public:

        BitField & operator=( const BitField & rhs )
        { _value = rhs._value; return *this; }

        BitField & operator&=( const BitField & rhs )
        { _value &= rhs._value; return *this; }

        BitField & operator|=( const BitField & rhs )
        { _value |= rhs._value; return *this; }

        BitField & operator^=( const BitField & rhs )
        { _value ^= rhs._value; return *this; }

        BitField & operator<<=( unsigned num )
        { _value <<= num; return *this; }

        BitField & operator>>=( unsigned num )
        { _value >>= num; return *this; }

        BitField operator~() const
        { return ~_value; }

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


    /** \relates BitField */
    template<class _IntT>
      inline BitField<_IntT> operator&( const BitField<_IntT> & lhs, const BitField<_IntT> & rhs )
      { return BitField<_IntT>(lhs) &= rhs; }

    /** \relates BitField */
    template<class _IntT>
      inline BitField<_IntT> operator|( const BitField<_IntT> & lhs, const BitField<_IntT> & rhs )
      { return BitField<_IntT>(lhs) |= rhs; }

    /** \relates BitField */
    template<class _IntT>
      inline BitField<_IntT> operator^( const BitField<_IntT> & lhs, const BitField<_IntT> & rhs )
      { return BitField<_IntT>(lhs) ^= rhs; }

    /** \relates BitField */
    template<class _IntT>
      inline BitField<_IntT> operator<<( const BitField<_IntT> & lhs, unsigned num )
      { return BitField<_IntT>(lhs) <<= num; }

    /** \relates BitField */
    template<class _IntT>
      inline BitField<_IntT> operator>>( const BitField<_IntT> & lhs, unsigned num )
      { return BitField<_IntT>(lhs) >>= num; }

    /////////////////////////////////////////////////////////////////
  } // namespace bit
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BIT_H
