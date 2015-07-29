/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CountryCode.h
 *
*/
#ifndef ZYPP_COUNTRYCODE_H
#define ZYPP_COUNTRYCODE_H

#include <iosfwd>
#include <string>

#include "zypp/IdStringType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class CountryCode
  /// \brief Country codes (iso3166-1-alpha-2).
  ///
  /// In fact the class will not prevent to use a non iso country code.
  /// Just a warning will appear in the log.
  ///////////////////////////////////////////////////////////////////
  class CountryCode : public IdStringType<CountryCode>
  {
  public:
    /** Default Ctor: \ref noCode */
    CountryCode();

    /** Ctor from string. */
    explicit CountryCode( IdString str_r );

    /** Ctor from string. */
    explicit CountryCode( const std::string & str_r );

    /** Ctor from string. */
    explicit CountryCode( const char * str_r );

    /** Dtor */
    ~CountryCode();

  public:

    /** \name CountryCode constants. */
    //@{
    /** Empty code. */
    static const CountryCode noCode;
    //@}

  public:
    /** Return the country code asString. */
    std::string code() const
    { return std::string(_str); }

    /** Return the translated country name; if unknown the country code. */
    std::string name() const;

  private:
    friend class IdStringType<CountryCode>;
    IdString _str;
  };
} // namespace zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////

ZYPP_DEFINE_ID_HASHABLE( ::zypp::CountryCode );

#endif // ZYPP_COUNTRYCODE_H
