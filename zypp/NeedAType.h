/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/NeedAType.h
 *
*/
#ifndef ZYPP_NEEDATYPE_H
#define ZYPP_NEEDATYPE_H

#include <iosfwd>
#include <map>
#include <list>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  /** \defgroup NEEDATYPE Wishlist of types

   Whenever you find something that might be worth being a
   type, and not just an \c unsigned or \c string. Keep it here.

   Start with a resonable \c typedef or \c class draft, so you
   can start using the type within the implementation. If you're
   in need for interface methods, add them to the draft. Think about
   the interface and expected behaviour, but implement \b here just
   as much as you actually need.

   Goal is to provide a real class for it, or to find out that a
   typedef is sufficient, ot the type is not needed anyway.

   If you already implemented something which would fit into this
   group, but don't want to move it here, put doxygens
   \code \ingroup NEEDATYPE \endcode tag in it's comment. So it will
   at least appear in the doc.

   \note Don't put stuff here, that's (almost) ready to use. This is
   not meant to be a convenience include. Goal is to have this file empty.
  */
  //@{


  /** Class representing an URL.
   something like liby2utils Url class. But we
   need clearly documented rules for parsing, and the way the broken down
   URL parts are stored withtin the class. This addresses especially
   the escaping and deescaping of special characters. It must be obvious to
   the user of the class, wheter an interface funtion deals with
   escaped, deescaped or hidden (in case of password) representation of the
   value.

   These rule may depend on the kind of URL protocol. The same for the
   recognized set of URL options we support. User/Passwd can be given
   in the URL or as option.

   There could be a common URL interface class which forwards all requests
   to an implementation class which may be specialized dependent on the URL
   protocol.

   There should be no obvious difference between known and unknown protocols.
   It should be possible to register an implementation class factory (or
   prototype) for a certain protocol at runtime.
  */
  typedef std::string Url;

  /** Convenient handling of pathnmes.  */
  typedef std::string Pathname;

  /** Single line of (human readable) text.
  probabely sufficient as typedef. we may use it to classify the
  various strings and string lists within resolvable and other classes.
  More a hint to the UI describing the purpose of the string. */
  typedef std::string Label;

  /** Single line of (human readable) text. See Label. A description would
   Text, while a a packages file list would be list<string> */
  typedef std::list<std::string> Text;

  /** Offer a License text and methods to remember confirmation. */
  typedef std::list<std::string> License;

  /** An rpm package group value. Also provide acces to a
   * (singleton) tree like group hierarchy which contains
   * all existing groups. No more need to fiddle with YStringTreeItem
   * classes and forgetting to add parsed groups there for use in the UI.
   * PackageGroup can be selforganizing.
  */
  typedef std::string PackageGroup;

  /** Candidate for string unification? */
  typedef std::list<std::string> PackageKeywords;

  /** Class representing a Date (time_t). Basically store a time_t and offer
  conversions into formated strings. */
  typedef time_t Date;

  /** Class representing a F(ile)Size, or ammount of byte. arithmetic
   operations needed e.g. for disk usage calculation. conversions into
   formated strings. Interface supports specifying FSize in (ammount,unit)
   KB MB GB....
  */
  typedef long long FSize;

  /** Vendor. Worth a typedef. Maybe a class unifying the strings. */
  typedef std::string Vendor;

  /** Handle Strings like "en_US". Offerrs their CountryCode and
   LanguageCode parts. */
  typedef std::string Locale;

  /** CountryCode of locale (from libutil incl. translations) */
  typedef std::string CountryCode;

  /** LanguageCode of locale (from libutil incl. translations) */
  typedef std::string LanguageCode;

  /** Handle data depending on a locale. Translated strings, maybe
   other too. */
  template<class _Val>
    struct MultiLocale : public std::map<Locale,_Val>
    {};

  /** stream reading compessed or uncompress files */
  typedef std::ifstream ifgzstream;

  /** stream writing compessed or uncompress files */
  typedef std::ofstream ofgzstream;

  //@}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_NEEDATYPE_H
