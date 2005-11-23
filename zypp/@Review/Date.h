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

  File:       Date.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Store and operate on date (time_t).

/-*/
#ifndef _Date_h_
#define _Date_h_

#include <ctime>
#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Date
//
/**
 * @short Store and operate on date (time_t).
 **/
class Date {

  private:

    /**
     * Calendar time. The number of seconds elapsed since
     * 00:00:00 on January 1, 1970, Coordinated Universal Time (UTC).
     **/
    time_t _date;

  public:

    // static constructors and conversion

    /**
     * Return the current time.
     **/
    static time_t now() { return time( 0 ); }

    /**
     * Return string representation of date according to format.
     * See 'man strftime' (which is used internaly) for valid
     * conversion specifiers in format.
     *
     * Retruns an empty string on illegal format.
     **/
    static std::string form( const std::string & format, time_t tval_r );

    /**
     * Convert from string representation of calendar time in
     * numeric form (like "1029255142").
     **/
    static time_t fromSECONDS( const std::string & str_r );

    /**
     * Convert to string representation of calendar time in
     * numeric form (like "1029255142").
     **/
    static std::string toSECONDS( time_t tval_r );

  public:

    /**
     * Constructor
     **/
    Date( time_t date_r = 0 ) : _date( date_r ) {}
    Date( const std::string & seconds_r ) : _date( fromSECONDS (seconds_r) ) {}

    /**
     * Conversion to time_t
     **/
    operator time_t() const { return _date; }

    Date & operator+=( const time_t rhs ) { _date += rhs; return *this; }
    Date & operator-=( const time_t rhs ) { _date -= rhs; return *this; }
    Date & operator*=( const time_t rhs ) { _date *= rhs; return *this; }
    Date & operator/=( const time_t rhs ) { _date /= rhs; return *this; }

    Date & operator++(/*prefix*/) { _date += 1; return *this; }
    Date & operator--(/*prefix*/) { _date -= 1; return *this; }

    Date operator++(int/*postfix*/) { return _date++; }
    Date operator--(int/*postfix*/) { return _date--; }

    /**
     * Member version of 'static form'.
     **/
    std::string form( const std::string & format ) const { return form( format, _date ); }

    /**
     * Default string representation of Date. The preferred
     * date and time representation for the current locale.
     **/
    std::string asString() const;

    /**
     * Write asString.
     **/
    friend std::ostream & operator<<( std::ostream & str, const Date & obj );
};

///////////////////////////////////////////////////////////////////

#endif // _Date_h_
