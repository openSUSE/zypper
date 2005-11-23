/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:       FSize.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Store and operate on (file/package/partition) sizes (long long).

/-*/
#ifndef _FSize_h_
#define _FSize_h_

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : FSize
//
/**
 * @short Store and operate on (file/package/partition) sizes (long long).
 **/
class FSize {

  public:

    /**
     * The Units
     **/
    enum Unit { B = 0, K, M, G, T };

  private:

    /**
     * The size in Byte
     **/
    long long _size;

  public:

    static const long long KB = 1024;
    static const long long MB = 1024 * KB;
    static const long long GB = 1024 * MB;
    static const long long TB = 1024 * GB;

    /**
     * Return ammount of Byte in Unit.
     **/
    static long long factor( const Unit unit_r ) {
      switch ( unit_r ) {
      case T: return TB;
      case G: return GB;
      case M: return MB;
      case K: return KB;
      case B: break;
      }
      return 1;
    }

    /**
     * String representation of Unit.
     **/
    static const char * unit( const Unit unit_r ) {
      switch ( unit_r ) {
      case T: return "TB";
      case G: return "GB";
      case M: return "MB";
      case K: return "kB";
      case B: break;
      }
      return "B";
    }

  public:

    /**
     * Construct from size in Byte.
     **/
    FSize( const long long size_r = 0 )
      : _size( size_r )
    {}

    /**
     * Construct from size in certain unit.
     * E.g. <code>FSize( 1, FSize::K )<code> makes 1024 Byte.
     **/
    FSize( const long long size_r, const Unit unit_r )
      : _size( size_r * factor( unit_r ) )
    {}

    /**
      Construct from string containing a number in given unit.
    */
    FSize( const std::string &sizeStr, const Unit unit_r = B );

    /**
     * Conversion to long long
     **/
    operator long long() const { return _size; }

    FSize & operator+=( const long long rhs ) { _size += rhs; return *this; }
    FSize & operator-=( const long long rhs ) { _size -= rhs; return *this; }
    FSize & operator*=( const long long rhs ) { _size *= rhs; return *this; }
    FSize & operator/=( const long long rhs ) { _size /= rhs; return *this; }

    FSize & operator++(/*prefix*/) { _size += 1; return *this; }
    FSize & operator--(/*prefix*/) { _size -= 1; return *this; }

    FSize operator++(int/*postfix*/) { return _size++; }
    FSize operator--(int/*postfix*/) { return _size--; }

    /**
     * Adjust size to multiple of <code>blocksize_r</code>
     **/
    FSize & fillBlock( FSize blocksize_r = KB );

    /**
     * Return size adjusted to multiple of <code>blocksize_r</code>
     **/
    FSize fullBlock( FSize blocksize_r = KB ) const { FSize ret( _size ); return ret.fillBlock(  blocksize_r );	}

    /**
     * Return size in Unit ( not rounded )
     **/
    long long operator()( const Unit unit_r ) const { return _size / factor( unit_r ); }

    /**
     * Return the best unit for string representation.
     **/
    Unit bestUnit() const;

    /**
     * Used as precision argument to form(), the 'best' precision according to
     * Unist is chosen.
     **/
    static const unsigned bestPrec = (unsigned)-1;

    /**
     * Return string representation in given Unit. Parameter <code>fw</code> and
     * <code>prec</code> denote field width and precision as in a "%*.*f" printf
     * format string. Avalue of <code>bestPrec</code> automatically picks an
     * appropriate precision depending on the unit.
     * If <code>showunit</code> ist true, the string representaion
     * of Unit is <em>appended<em> separated by a single blank.
     *
     * If Unit is <b>B</b>yte, precision is set to zero.
     **/
    std::string form( const Unit unit_r, unsigned fw = 0, unsigned prec = bestPrec, const bool showunit = true ) const;

    /**
     * Return string representation in bestUnit.
     **/
    std::string form( unsigned fw = 0, unsigned prec = bestPrec, const bool showunit = true ) const {
      return form( bestUnit(), fw, prec, showunit );
    }

    /**
     * Default string representation (precision 1 and unit appended).
     **/
    std::string asString() const;

    /**
     * Write asString.
     **/
    friend std::ostream & operator<<( std::ostream & str, const FSize & obj );
};

///////////////////////////////////////////////////////////////////

#endif // _FSize_h_
