/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ByteCount.h
 *
*/
#ifndef ZYPP_BYTECOUNT_H
#define ZYPP_BYTECOUNT_H

#include <iosfwd>

#include <zypp-core/base/Unit.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ByteCount
  //
  /** Store and operate with byte count.
   *
  */
  class ByteCount
  {
    friend std::ostream & operator<<( std::ostream & str, const ByteCount & obj );

  public:

    typedef base::Unit      Unit;
    typedef Unit::ValueType SizeType;

    /** \name Byte unit constants. */
    //@{
    /** 1 Byte */
    static const Unit B;

    /** 1024 Byte */
    static const Unit K;
    static const Unit KiB;
    /** 1024^2 Byte */
    static const Unit M;
    static const Unit MiB;
    /** 1024^3 Byte */
    static const Unit G;
    static const Unit GiB;
    /** 1024^4 Byte */
    static const Unit T;
    static const Unit TiB;

    /** 1000 Byte */
    static const Unit kB;
    /** 1000^2 Byte */
    static const Unit MB;
    /** 1000^3 Byte */
    static const Unit GB;
    /** 1000^4 Byte */
    static const Unit TB;
    //@}

  public:

    /** Default ctor */
    ByteCount()
    : _count( 0 )
    {}
    /** Ctor taking 1 Unit. */
    ByteCount( const Unit & unit_r )
    : _count( unit_r.factor() )
    {}
    /** Ctor taking a count and optinal Unit. */
    ByteCount( const SizeType count_r, const Unit & unit_r = B )
    : _count( count_r * unit_r.factor() )
    {}

 public:

    /** Conversion to SizeType. */
    operator SizeType() const
    { return _count; }

    /** \name Arithmetic operations.
     * \c + \c - \c * \c / are provided via conversion to SizeType.
    */
    //@{
    ByteCount & operator+=( const SizeType rhs ) { _count += rhs; return *this; }
    ByteCount & operator-=( const SizeType rhs ) { _count -= rhs; return *this; }
    ByteCount & operator*=( const SizeType rhs ) { _count *= rhs; return *this; }
    ByteCount & operator/=( const SizeType rhs ) { _count /= rhs; return *this; }

    ByteCount & operator++(/*prefix*/) { _count += 1; return *this; }
    ByteCount & operator--(/*prefix*/) { _count -= 1; return *this; }

    ByteCount operator++(int/*postfix*/) { return _count++; }
    ByteCount operator--(int/*postfix*/) { return _count--; }
    //@}

    /** Adjust count to multiple of \a blocksize_r (default 1K).
     * Zero blocksize_r is treated as 1B.
    */
    ByteCount & fillBlock( ByteCount blocksize_r = K );

    /** Return count adjusted to multiple of \a blocksize_r (default 1K). */
    ByteCount fullBlocks( ByteCount blocksize_r = K ) const
    { return ByteCount(*this).fillBlock( blocksize_r ); }

    /** Return number of blocks of size \a blocksize_r (default 1K). */
    SizeType blocks( ByteCount blocksize_r = K ) const
    { return fullBlocks( blocksize_r ) / blocksize_r; }

  public:

    /** Return the best Unit (B,K,M,G,T) for count. */
    const Unit & bestUnit() const;

    /** Return the best Unit (B,kB,MB,GB,TB) for count. */
    const Unit & bestUnit1000() const;

    /** \name Conversion to string.
     * \li \a field_width_r Width for the number part (incl. precision)
     * \li \a unit_width_r With for the unit symbol (without symbol if zero)
     * \li \a prec_r Precision to use.
     * \see zypp::base::Unit
    */
    //@{
    /** Auto selected Unit and precision. */
    std::string asString( unsigned field_width_r = 0,
                          unsigned unit_width_r  = 1 ) const
    { return asString( bestUnit(), field_width_r, unit_width_r ); }
    /** Auto selected Unit. */
    std::string asString( unsigned field_width_r,
                          unsigned unit_width_r,
                          unsigned prec_r ) const
    { return asString( bestUnit(), field_width_r, unit_width_r, prec_r ); }
    /** Auto selected precision. */
    std::string asString( const Unit & unit_r,
                          unsigned field_width_r = 0,
                          unsigned unit_width_r  = 1 ) const
    { return asString( unit_r, field_width_r, unit_width_r, unit_r.prec() ); }
    /** Nothing auto selected. */
    std::string asString( const Unit & unit_r,
                          unsigned field_width_r,
                          unsigned unit_width_r,
                          unsigned prec_r ) const
    { return unit_r.form( _count, field_width_r, unit_width_r, prec_r ); }
    //@}

  private:
    SizeType _count;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ByteCount Stream output */
  inline std::ostream & operator<<( std::ostream & str, const ByteCount & obj )
  { return str << obj.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BYTECOUNT_H
