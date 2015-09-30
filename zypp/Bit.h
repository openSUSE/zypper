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
   * at compiletime! There various like (TInt is an integral type)
   * (begin+size < maxbits) or ( field dependent
   * constants must be within the range defined by size ).
  */
  namespace bit
  { /////////////////////////////////////////////////////////////////

    namespace bit_detail
    {
      /** Generate constants with \a _size trailing '1'-bits */
      template<class TInt, unsigned _size>
        struct Gen1Bits
        {
          static const TInt value = (Gen1Bits<TInt,_size-1>::value << 1)+1;
        };
      /** Specialization for \a _length 0 */
      template<class TInt>
        struct Gen1Bits<TInt, 0>
        {
          static const TInt value = 0;
        };
    }

    /** Number of bits available in \a TInt. */
    template<class TInt>
      struct MaxBits
      {
        typedef TInt IntT;
        static const unsigned value = (sizeof(IntT)*8);
      };

    /** For printing bits. */
    template<class TInt>
      inline std::string asString( TInt val, char zero = '0', char one = '1' )
      {
        std::string s( MaxBits<TInt>::value, zero );
        for( unsigned i = MaxBits<TInt>::value; i; )
          {
            --i;
            if ( val & (TInt)1 )
              s[i] = one;
            val = val >> 1;
          };
        return s;
      }

    /** A bitmaks of \a _size 1-bits starting at bit \a _begin. */
    template<class TInt, unsigned _begin, unsigned _size>
      struct Mask
      {
        typedef TInt IntT;
        static const IntT value    = bit_detail::Gen1Bits<IntT,_size>::value << _begin;
        static const IntT inverted = ~value;
      };

    /** Range of bits starting at bit \a _begin with length \a _size. */
    template<class TInt, unsigned _begin, unsigned _size>
      struct Range
      {
        typedef TInt IntT;
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
    template<class TInt, unsigned _begin>
      struct Range<TInt, _begin, 0>
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
    template<class TRange, typename TRange::IntT _value>
      struct RangeValue
      {
        typedef TRange                RangeT;
        typedef typename TRange::IntT IntT;

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
    template<class TRange, unsigned _pos>
      struct RangeBit
      {
        typedef TRange                RangeT;
        typedef typename TRange::IntT IntT;

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
    template<class TInt>
      class BitField  : public Range<TInt, 0, MaxBits<TInt>::value>
      {
      public:
        /** Default ctor: zero. */
        BitField()
        : _value( (TInt)0 )
        {}
        /** Ctor taking an \a TInt. */
        BitField( const TInt & value_r )
        : _value( value_r )
        {}

      public:
        /** Validate in a boolean context. */
        explicit operator bool() const
        { return _value != (TInt)0; }

      public:
        /** Return the value. */
        template<class TRange>
          TInt value() const
          {
            return _value & TRange::Mask::value;
          }
        TInt value() const
        {
          return _value;
        }

        /** Value as bit string. */
        template<class TRange>
          std::string asString() const
          {
            return bit::asString( _value & TRange::Mask::value, '_' );
          }
        std::string asString() const
        {
          return bit::asString( _value, '_' );
        }

        /** Assign Range in \a rhs to \c this. */
        template<class TRange>
          BitField & assign( TInt rhs )
          {
            _value = (_value & TRange::Mask::inverted)
                   | (rhs & TRange::Mask::value);
            return *this;
          }
        BitField & assign( TInt rhs )
        {
          _value = rhs;
          return *this;
        }

        /** Test for equal value within a Range. */
        template<class TRange>
          bool isEqual( TInt rhs ) const
          {
            return (_value & TRange::Mask::value)
                == (rhs & TRange::Mask::value);
          }
        bool isEqual( TInt rhs ) const
        {
          return _value == rhs;
        }

       public:

         /** Set or unset bits of \a rhs. */
        template<class TRange>
            BitField & set( TInt rhs, bool doset_r )
            { return set( (rhs & TRange::Mask::value), doset_r ); }

        BitField & set( TInt rhs, bool doset_r )
        { return doset_r ? set( rhs ) : unset( rhs ); }

        /** Set bits of \a rhs. */
        template<class TRange>
            BitField & set( TInt rhs )
            { return set( rhs & TRange::Mask::value ); }

        BitField & set( TInt rhs )
        { _value |= rhs; return *this; }

        /** Unset bits of \a rhs. */
        template<class TRange>
            BitField & unset( TInt rhs )
            { return unset( rhs & TRange::Mask::value ); }

        BitField & unset( TInt rhs )
        { _value &= ~rhs; return *this; }

        /** Test whether \b all bits of \a rhs are set. */
        template<class TRange>
            bool test( TInt rhs )
            { return test( rhs & TRange::Mask::value ); }

        bool test( TInt rhs ) const
        { return (_value & rhs) == rhs; }

        /** Test whether \b at \b least \b one bit of \a rhs is set. */
        template<class TRange>
            bool testAnyOf( TInt rhs )
            { return testAnyOf( rhs & TRange::Mask::value ); }

        bool testAnyOf( TInt rhs ) const
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
        TInt _value;
      };
    ///////////////////////////////////////////////////////////////////

    /** \relates BitField Stream output */
    template<class TInt>
      std::ostream & operator<<( std::ostream & str, const BitField<TInt> & obj )
      {
        return str << obj.asString();
      }

    /** \relates BitField */
    template<class TInt>
      inline bool operator==( const BitField<TInt> & lhs, const BitField<TInt> & rhs )
      { return lhs.value() == rhs.value(); }

    /** \relates BitField */
    template<class TInt>
      inline bool operator!=( const BitField<TInt> & lhs, const BitField<TInt> & rhs )
      { return ! (lhs == rhs); }


    /** \relates BitField */
    template<class TInt>
      inline BitField<TInt> operator&( const BitField<TInt> & lhs, const BitField<TInt> & rhs )
      { return BitField<TInt>(lhs) &= rhs; }

    /** \relates BitField */
    template<class TInt>
      inline BitField<TInt> operator|( const BitField<TInt> & lhs, const BitField<TInt> & rhs )
      { return BitField<TInt>(lhs) |= rhs; }

    /** \relates BitField */
    template<class TInt>
      inline BitField<TInt> operator^( const BitField<TInt> & lhs, const BitField<TInt> & rhs )
      { return BitField<TInt>(lhs) ^= rhs; }

    /** \relates BitField */
    template<class TInt>
      inline BitField<TInt> operator<<( const BitField<TInt> & lhs, unsigned num )
      { return BitField<TInt>(lhs) <<= num; }

    /** \relates BitField */
    template<class TInt>
      inline BitField<TInt> operator>>( const BitField<TInt> & lhs, unsigned num )
      { return BitField<TInt>(lhs) >>= num; }

    /////////////////////////////////////////////////////////////////
  } // namespace bit
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BIT_H
