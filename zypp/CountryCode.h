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

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class CountryCode;
  inline bool operator==( const CountryCode & lhs, const CountryCode & rhs );
  inline bool operator!=( const CountryCode & lhs, const CountryCode & rhs );

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CountryCode
  //
  /** Country codes (iso3166-1-alpha-2).
   *
   * In fact the class will not prevent to use a non iso country code.
   * Just a warning will appear in the log.
  */
  class CountryCode
  {
    friend std::ostream & operator<<( std::ostream & str, const CountryCode & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    CountryCode();

    /** Ctor taking a string. */
    explicit
    CountryCode( const std::string & code_r );

    /** Dtor */
    ~CountryCode();

  public:

    /** \name CountryCode constants. */
    //@{
    /** No or empty code. */
    static const CountryCode noCode;
    //@}

  public:
    /** Return the country code. */
    std::string code() const;

    /** Return the country name; if not available the country code. */
    std::string name() const;

    /** <tt>*this != noCode</tt>. */
    bool hasCode() const
    { return *this != noCode; }

  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates CountryCode Stream output */
  inline std::ostream & operator<<( std::ostream & str, const CountryCode & obj )
  { return str << obj.code(); }

  /** Comparison based on string value. */
  //@{
  /** \relates CountryCode */
  inline bool operator==( const CountryCode & lhs, const CountryCode & rhs ) {
    return( lhs.code() == rhs.code() );
  }
  /** \relates CountryCode */
  inline bool operator==( const std::string & lhs, const CountryCode & rhs ) {
    return( lhs == rhs.code() );
  }
  /** \relates CountryCode */
  inline bool operator==( const CountryCode & lhs, const std::string & rhs ) {
    return( lhs.code() == rhs );
  }

  /** \relates CountryCode */
  inline bool operator!=( const CountryCode & lhs, const CountryCode & rhs ) {
    return( ! operator==( lhs, rhs ) );
  }
  /** \relates CountryCode */
  inline bool operator!=( const std::string & lhs, const CountryCode & rhs ) {
    return( ! operator==( lhs, rhs ) );
  }
  /** \relates CountryCode */
  inline bool operator!=( const CountryCode & lhs, const std::string & rhs ) {
    return( ! operator==( lhs, rhs ) );
  }
  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////
  /** \relates zypp::CountryCode Default order for std::container based on code string value.*/
  template<>
    inline bool less<zypp::CountryCode>::operator()( const zypp::CountryCode & lhs, const zypp::CountryCode & rhs ) const
    { return lhs.code() < rhs.code(); }
  /////////////////////////////////////////////////////////////////
} // namespace std
///////////////////////////////////////////////////////////////////
#endif // ZYPP_COUNTRYCODE_H
