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
#include "zypp/base/EnumClass.h"

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
    typedef time_t Duration;

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
    explicit Date( const std::string & seconds_r );

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
     * \return An empty string on illegal format, "0" if date is unspecified.
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

  public:
    /** \name Printing in various predefined formats */
    //@{
    /** Date formats for printing (use like 'enum class \ref DateFormat') */
    struct EDateFormatDef { enum Enum {
      none,	///< ""
      calendar,	///< 2014-02-07
      month,	///< 2014-02
      year,	///< 2014
      week,	///< 2014-W06
      weekday,	///< 2014-W06-5 (1 is Monday)
      ordinal,	///< 2014-038
    };};
    typedef base::EnumClass<EDateFormatDef> DateFormat;	///< 'enum class DateFormat'

    /** Time formats for printing (use like 'enum class \ref TimeFormat') */
    struct ETimeFormatDef { enum Enum {
      none,	///< ""
      seconds,	///< 07:06:41
      minutes,	///< 07:06
      hours,	///< 07
    };};
    typedef base::EnumClass<ETimeFormatDef> TimeFormat;	///< 'enum class TimeFormat'

    /** Timezone indicator for printing (use like 'enum class \ref TimeZoneFormat') */
    struct ETimeZoneFormatDef { enum Enum {
      none,	///< ""
      name,	///< UTC, CET, ...
      offset,	///< +00[:00]
    };};
    typedef base::EnumClass<ETimeZoneFormatDef> TimeZoneFormat;	///< 'enum class TimeZoneFormat'

    /** Default format is <tt>'2014-02-07 07:06:41 CET'</tt>
     * The default is \ref DateFormat::calendar, \ref TimeFormat::seconds, \ref TimeZoneFormat::name and
     * \ref TB_LOCALTIME. For other formats you don't have to repeat all the defaults, just pass the
     * values where you differ.
     */
    std::string print( DateFormat dateFormat_r = DateFormat::calendar, TimeFormat timeFormat_r = TimeFormat::seconds, TimeZoneFormat timeZoneFormat_r = TimeZoneFormat::name, TimeBase base_r = TB_LOCALTIME ) const;
    /** \overload */
    std::string print( TimeFormat timeFormat_r, TimeZoneFormat timeZoneFormat_r = TimeZoneFormat::name, TimeBase base_r = TB_LOCALTIME ) const
    { return print( DateFormat::calendar, timeFormat_r, timeZoneFormat_r, base_r ); }
    /** \overload */
    std::string print( DateFormat dateFormat_r, TimeZoneFormat timeZoneFormat_r, TimeBase base_r = TB_LOCALTIME ) const
    { return print( dateFormat_r, TimeFormat::seconds, timeZoneFormat_r, base_r ); }
    /** \overload */
    std::string print( DateFormat dateFormat_r, TimeFormat timeFormat_r, TimeBase base_r ) const
    { return print( dateFormat_r, timeFormat_r, TimeZoneFormat::name, base_r ); }
    /** \overload */
    std::string print( TimeZoneFormat timeZoneFormat_r, TimeBase base_r = TB_LOCALTIME ) const
    { return print( DateFormat::calendar, TimeFormat::seconds, timeZoneFormat_r, base_r ); }
    /** \overload */
    std::string print( TimeFormat timeFormat_r, TimeBase base_r ) const
    { return print( DateFormat::calendar, timeFormat_r, TimeZoneFormat::name, base_r ); }
    /** \overload */
    std::string print( DateFormat dateFormat_r, TimeBase base_r ) const
    { return print( dateFormat_r, TimeFormat::seconds, TimeZoneFormat::name, base_r ); }
    /** \overload */
    std::string print( TimeBase base_r ) const
    { return print( DateFormat::calendar, TimeFormat::seconds, TimeZoneFormat::name, base_r ); }

    /** Convenience for printing the date only [<tt>'2014-02-07'</tt>]
     * The default is \ref DateFormat::calendar and \ref TB_LOCALTIME
     */
    std::string printDate( DateFormat dateFormat_r = DateFormat::calendar, TimeBase base_r = TB_LOCALTIME ) const
    { return print( dateFormat_r, TimeFormat::none, TimeZoneFormat::none, base_r ); }
    /** \overload */
    std::string printDate( TimeBase base_r ) const
    { return printDate( DateFormat::calendar, base_r ); }

    /** Convenience for printing the time only [<tt>'07:06:41 CET'</tt>]
     * The default is \ref DateFormat::calendar and \ref TB_LOCALTIME
     */
    std::string printTime( TimeFormat timeFormat_r = TimeFormat::seconds, TimeZoneFormat timeZoneFormat_r = TimeZoneFormat::name, TimeBase base_r = TB_LOCALTIME ) const
    { return print( DateFormat::none, timeFormat_r, timeZoneFormat_r, base_r ); }
    /** \overload */
    std::string printTime( TimeZoneFormat timeZoneFormat_r , TimeBase base_r = TB_LOCALTIME ) const
    { return printTime( TimeFormat::seconds, timeZoneFormat_r, base_r ); }
    /** \overload */
    std::string printTime( TimeFormat timeFormat_r , TimeBase base_r ) const
    { return printTime( timeFormat_r, TimeZoneFormat::name, base_r ); }
    /** \overload */
    std::string printTime( TimeBase base_r ) const
    { return printTime( TimeFormat::seconds, TimeZoneFormat::name, base_r ); }

    /** Default ISO 8601 format is <tt>'2014-02-07T07:06:41+01'</tt>
     * \note As timezone names are not used in ISO, \ref TimeZoneFormat::name is the same as
     * \ref TimeZoneFormat::offset when printing in \ref TB_LOCALTIME. When printing \ref TB_UTC
     * it uses a \c 'Z' to indicate UTC (Zulu time) rather than printing \c '+00'.
     */
    std::string printISO( DateFormat dateFormat_r = DateFormat::calendar, TimeFormat timeFormat_r = TimeFormat::seconds, TimeZoneFormat timeZoneFormat_r = TimeZoneFormat::name, TimeBase base_r = TB_LOCALTIME ) const;
    /** \overload */
    std::string printISO( TimeFormat timeFormat_r, TimeZoneFormat timeZoneFormat_r = TimeZoneFormat::name, TimeBase base_r = TB_LOCALTIME ) const
    { return printISO( DateFormat::calendar, timeFormat_r, timeZoneFormat_r, base_r ); }
    /** \overload */
    std::string printISO( DateFormat dateFormat_r, TimeZoneFormat timeZoneFormat_r, TimeBase base_r = TB_LOCALTIME ) const
    { return printISO( dateFormat_r, TimeFormat::seconds, timeZoneFormat_r, base_r ); }
    /** \overload */
    std::string printISO( DateFormat dateFormat_r, TimeFormat timeFormat_r, TimeBase base_r ) const
    { return printISO( dateFormat_r, timeFormat_r, TimeZoneFormat::name, base_r ); }
    /** \overload */
    std::string printISO( TimeZoneFormat timeZoneFormat_r, TimeBase base_r = TB_LOCALTIME ) const
    { return printISO( DateFormat::calendar, TimeFormat::seconds, timeZoneFormat_r, base_r ); }
    /** \overload */
    std::string printISO( TimeFormat timeFormat_r, TimeBase base_r ) const
    { return printISO( DateFormat::calendar, timeFormat_r, TimeZoneFormat::name, base_r ); }
    /** \overload */
    std::string printISO( DateFormat dateFormat_r, TimeBase base_r ) const
    { return printISO( dateFormat_r, TimeFormat::seconds, TimeZoneFormat::name, base_r ); }
    /** \overload */
    std::string printISO( TimeBase base_r ) const
    { return printISO( DateFormat::calendar, TimeFormat::seconds, TimeZoneFormat::name, base_r ); }
    //@}

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

  /** \relates Date XML output.
   * Print \c time_t and \c text attribute. Allow alternate node name [date].
   */
  std::ostream & dumpAsXmlOn( std::ostream & str, const Date & obj, const std::string & name_r = "date" );

  ///////////////////////////////////////////////////////////////////
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
