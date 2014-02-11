/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Date.h
 *
*/
#ifndef ZYPP_DATE_H
#define ZYPP_DATE_H

#include <ctime>
#include <iosfwd>
#include <string>

#include "zypp/base/Exception.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Date
  //
  /** Store and operate on date (time_t).
  */
  class Date
  {
    friend std::ostream & operator<<( std::ostream & str, const Date & obj );

  public:

    typedef time_t ValueType;

    static const ValueType second	= 1;
    static const ValueType minute	= 60;
    static const ValueType hour		= 3600;
    static const ValueType day		= 86400;
    static const ValueType month28	= 2419200;
    static const ValueType month29	= 2505600;
    static const ValueType month30	= 2592000;
    static const ValueType month31	= 2678400;
    static const ValueType month	= month30;
    static const ValueType year365	= 31536000;
    static const ValueType year366	= 31622400;
    static const ValueType year		= year365;

    enum TimeBase { TB_LOCALTIME, TB_UTC };

    /** Default ctor: 0 */
    Date()
    : _date( 0 )
    {}
    /** Ctor taking time_t value. */
    Date( ValueType date_r )
    : _date( date_r )
    {}
    /** Ctor taking time_t value as string. */
    Date( const std::string & seconds_r );

    /**
     * Ctor from a \a date_str (in localtime) formatted using \a format.
     *
     * \throws DateFormatException in case \a date_str cannot be
     *         parsed according to \a format.
     */
    Date( const std::string & date_str, const std::string & format );
    /** \overload with explicitly given \ref TimeBase. */
    Date( const std::string & date_str, const std::string & format, TimeBase base_r );

    /** Return the current time. */
    static Date now()
    { return ::time( 0 ); }

  public:
    /** Conversion to time_t. */
    operator ValueType() const
    { return _date; }

    /** \name Arithmetic operations. */
    //@{
    Date operator+( const time_t rhs ) const { return _date + rhs; }
    Date operator-( const time_t rhs ) const { return _date - rhs; }
    Date operator*( const time_t rhs ) const { return _date * rhs; }
    Date operator/( const time_t rhs ) const { return _date / rhs; }

    Date & operator+=( const time_t rhs ) { _date += rhs; return *this; }
    Date & operator-=( const time_t rhs ) { _date -= rhs; return *this; }
    Date & operator*=( const time_t rhs ) { _date *= rhs; return *this; }
    Date & operator/=( const time_t rhs ) { _date /= rhs; return *this; }

    Date & operator++(/*prefix*/) { _date += 1; return *this; }
    Date & operator--(/*prefix*/) { _date -= 1; return *this; }

    Date operator++(int/*postfix*/) { return _date++; }
    Date operator--(int/*postfix*/) { return _date--; }
    //@}

  public:
    /** Return string representation according to format as localtime.
     * \see 'man strftime' (which is used internaly) for valid
     * conversion specifiers in format.
     *
     * \return An empty string on illegal format.
     **/
    std::string form( const std::string & format_r ) const
    { return form( format_r, TB_LOCALTIME ); }
    /** \overload with explicitly given \ref TimeBase. */
    std::string form( const std::string & format_r, TimeBase base_r ) const;

    /** Default string representation of Date.
     * The preferred date and time representation for the current locale.
     **/
    std::string asString() const
    { return form( "%c" ); }

    /** Convert to string representation of calendar time in
     *  numeric form (like "1029255142").
     **/
    std::string asSeconds() const
    { return form( "%s" ); }

  private:
    /** Calendar time.
     * The number of seconds elapsed since 00:00:00 on January 1, 1970,
     * Coordinated Universal Time (UTC).
     **/
    ValueType _date;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Date Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Date & obj )
  { return str << obj.asString(); }

  class DateFormatException : public Exception
  {
  public:
    DateFormatException( const std::string & msg ) : Exception( msg )
    {}
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DATE_H
