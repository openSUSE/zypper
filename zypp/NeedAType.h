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
#include "zypp/Locale.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  /** \defgroup NEEDATYPE Wishlist of types

   Whenever you find something that might be worth being a
   type, and not just an \c unsigned or \c string. Keep it here.

   Start with a reasonable \c typedef or \c class draft, so you
   can start using the type within the implementation. If you're
   in need for interface methods, add them to the draft. Think about
   the interface and expected behaviour, but implement \b here just
   as much as you actually need.

   Goal is to provide a real class for it, or to find out that a
   typedef is sufficient, or the type is not needed anyway.

   If you already implemented something which would fit into this
   group, but don't want to move it here, put doxygens
   \code \ingroup NEEDATYPE \endcode tag in it's comment. So it will
   at least appear in the doc.

   \note Don't put stuff here, that's (almost) ready to use. This is
   not meant to be a convenience include. Goal is to have this file empty.
  */
  //@{


  /** Single line of (human readable) text.
  probably sufficient as typedef. we may use it to classify the
  various strings and string lists within resolvable and other classes.
  More a hint to the UI describing the purpose of the string. */
  typedef std::string Label;

  /** Single line of (human readable) text. See Label. A description would
   Text, while a a packages file list would be list<string> */
  typedef std::string Text;

  /** Offer a License text and methods to remember confirmation. */
  typedef std::string License;

  /** An rpm package group value. Also provide access to a
   * (singleton) tree like group hierarchy which contains
   * all existing groups. No more need to fiddle with YStringTreeItem
   * classes and forgetting to add parsed groups there for use in the UI.
   * PackageGroup can be selforganizing.
  */
  typedef std::string PackageGroup;

  /** Candidate for string unification? */
  typedef std::list<std::string> PackageKeywords;

  /** Vendor. Worth a typedef. Maybe a class unifying the strings. */
  typedef std::string Vendor;

  /** Id used inside ZMD */
  typedef long ZmdId;

  /** Handle data depending on a locale. Translated strings, maybe
   other too. */
  template<class _Val>
    struct MultiLocale : public std::map<Locale,_Val>
    {};

  //@}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_NEEDATYPE_H
