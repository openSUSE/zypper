/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/LanguageCode.h
 *
*/
#ifndef ZYPP_LANGUAGECODE_H
#define ZYPP_LANGUAGECODE_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class LanguageCode;
  inline bool operator==( const LanguageCode & lhs, const LanguageCode & rhs );
  inline bool operator!=( const LanguageCode & lhs, const LanguageCode & rhs );

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : LanguageCode
  //
  /** Language codes (iso639_2/iso639_1).
   *
   * In fact the class will not prevent to use a non iso language code.
   * Just a warning will appear in the log.
  */
  class LanguageCode
  {
    friend std::ostream & operator<<( std::ostream & str, const LanguageCode & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    LanguageCode();

    /** Ctor taking a string. */
    explicit
    LanguageCode( const std::string & code_r );

    /** Dtor */
    ~LanguageCode();

  public:
    /** \name LanguageCode constants. */
    //@{
    /** No or empty code. */
    static const LanguageCode noCode;
    //@}

  public:
    /** Return the language code. */
    std::string code() const;

    /** Return the language name; if not available the language code. */
    std::string name() const;

    /** <tt>*this != noCode</tt>. */
    inline bool hasCode() const
    { return *this != noCode; }

  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates LanguageCode Stream output */
  inline std::ostream & operator<<( std::ostream & str, const LanguageCode & obj )
  { return str << obj.code(); }

  /** Comparison based on string value. */
  //@{
  /** \relates LanguageCode */
  inline bool operator==( const LanguageCode & lhs, const LanguageCode & rhs ) {
    return( lhs.code() == rhs.code() );
  }
  /** \relates LanguageCode */
  inline bool operator==( const std::string & lhs, const LanguageCode & rhs ) {
    return( lhs == rhs.code() );
  }
  /** \relates LanguageCode */
  inline bool operator==( const LanguageCode & lhs, const std::string & rhs ) {
    return( lhs.code() == rhs );
  }

  /** \relates LanguageCode */
  inline bool operator!=( const LanguageCode & lhs, const LanguageCode & rhs ) {
    return( ! operator==( lhs, rhs ) );
  }
  /** \relates LanguageCode */
  inline bool operator!=( const std::string & lhs, const LanguageCode & rhs ) {
    return( ! operator==( lhs, rhs ) );
  }
  /** \relates LanguageCode */
  inline bool operator!=( const LanguageCode & lhs, const std::string & rhs ) {
    return( ! operator==( lhs, rhs ) );
  }
  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////
  /** \relates zypp::LanguageCode Default order for std::container based on code string value.*/
  template<>
    inline bool less<zypp::LanguageCode>::operator()( const zypp::LanguageCode & lhs, const zypp::LanguageCode & rhs ) const
    { return lhs.code() < rhs.code(); }
  /////////////////////////////////////////////////////////////////
} // namespace std
///////////////////////////////////////////////////////////////////
#endif // ZYPP_LANGUAGECODE_H
