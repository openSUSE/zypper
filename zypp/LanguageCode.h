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

#include "zypp/IdStringType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class LanguageCode
  /// \brief Language codes (iso639_2/iso639_1).
  ///
  /// In fact the class will not prevent to use a non iso language code.
  /// Just a warning will appear in the log.
  ///////////////////////////////////////////////////////////////////
  class LanguageCode : public IdStringType<LanguageCode>
  {
  public:
    /** Default Ctor: \ref noCode */
    LanguageCode();

    /** Ctor from string. */
    explicit LanguageCode( IdString str_r );

    /** Ctor from string. */
    explicit LanguageCode( const std::string & str_r );

    /** Ctor from string. */
    explicit LanguageCode( const char * str_r );

     /** Dtor */
    ~LanguageCode();

  public:
    /** \name LanguageCode constants. */
    //@{
    /** Empty code. */
    static const LanguageCode noCode;
    /** Last resort "en". */
    static const LanguageCode enCode;
    //@}

  public:
    /** Return the language code asString. */
    std::string code() const
    { return std::string(_str); }

    /** Return the translated language name; if unknown the language code. */
    std::string name() const;

  private:
    friend class IdStringType<LanguageCode>;
    IdString _str;
  };
} // namespace zypp
///////////////////////////////////////////////////////////////////

ZYPP_DEFINE_ID_HASHABLE( ::zypp::LanguageCode );

#endif // ZYPP_LANGUAGECODE_H
