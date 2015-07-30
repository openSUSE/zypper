/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Locale.h
 *
*/
#ifndef ZYPP_LOCALE_H
#define ZYPP_LOCALE_H

#include <iosfwd>
#include <string>

#include "zypp/base/Hash.h"

#include "zypp/IdStringType.h"
#include "zypp/LanguageCode.h"
#include "zypp/CountryCode.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  class Locale;
  typedef std::unordered_set<Locale> LocaleSet;

  ///////////////////////////////////////////////////////////////////
  /// \class Locale
  /// \brief 'Language[_Country]' codes.
  ///
  /// In fact the class will not prevent to use a non iso code.
  /// Just a warning will appear in the log. Construction from string
  /// consider everything up to the first \c '.' or \c '@'.
  /// \code
  ///   Locale l( "de_DE.UTF-8" );
  ///
  ///   l.code()     == "de_DE";
  ///   l.language() == "de";
  ///   l.country()  == "DE";
  ///
  ///   l.fallback()                       == "de";
  ///   l.fallback().fallback()            == Locale::enCode == "en";
  ///   l.fallback().fallback().fallback() == Locale::noCode == "";
  /// \endcode
  ///////////////////////////////////////////////////////////////////
  class Locale : public IdStringType<Locale>
  {
  public:
    /** Default Ctor: \ref noCode */
    Locale();

    /** Ctor from string. */
    explicit Locale( IdString str_r );

    /** Ctor from string. */
    explicit Locale( const std::string & str_r );

    /** Ctor from string. */
    explicit Locale( const char * str_r );

    /** Ctor taking LanguageCode and optional CountryCode. */
    Locale( LanguageCode language_r, CountryCode country_r = CountryCode() );

    /** Dtor */
    ~Locale();

  public:
    /** \name Locale constants. */
    //@{
    /** Empty code. */
    static const Locale noCode;

    /** Last resort "en". */
    static const Locale enCode;
    //@}

  public:
    /** The language part. */
    LanguageCode language() const;

    /** The county part.*/
    CountryCode country() const;

    /** Return the locale code asString. */
    std::string code() const
    { return std::string(_str); }

    /** Return the translated locale name. */
    std::string name() const;

  public:
    /** Return the fallback locale for this locale, if no fallback exists the empty Locale::noCode.
     * The usual fallback sequence is "language_COUNTRY" -> "language" -> Locale::enCode ("en")
     * ->Locale::noCode (""). Some exceptions like "pt_BR"->"en"->"" do exist.
     */
    Locale fallback() const;

    /** Return the best match for \ref Locale \a requested_r within the available \a avLocales_r.
     *
     * If \a requested_r is not specified \ref ZConfig::textLocale is assumed.
     *
     * If neither \c requested_r nor any of it's \ref fallback locales are available
     * in \a avLocales_r, \ref Locale::noCode is returned.
     */
    static Locale bestMatch( const LocaleSet & avLocales_r, Locale requested_r = Locale() );

  private:
    friend class IdStringType<Locale>;
    IdString _str;
  };
} // namespace zypp
///////////////////////////////////////////////////////////////////

ZYPP_DEFINE_ID_HASHABLE( ::zypp::Locale );

#endif // ZYPP_LOCALE_H
