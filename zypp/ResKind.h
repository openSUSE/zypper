/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResKind.h
 *
*/
#ifndef ZYPP_RESKIND_H
#define ZYPP_RESKIND_H

#include <iosfwd>
#include <string>

#include "zypp/base/String.h"
#include "zypp/IdStringType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResKind
  //
  /** Resolvable kinds.
   * A \b lowercased string and used as identification.
   * Comparison against string values is always case
   * insensitive.
   */
  class ResKind : public IdStringType<ResKind>
  {
    public:
      /** \name Some builtin ResKind constants. */
      //@{
      /** Value representing \c nokind (<tt>""</tt>)*/
      static const ResKind nokind;

      static const ResKind atom;
      static const ResKind language;
      static const ResKind message;
      static const ResKind package;
      static const ResKind patch;
      static const ResKind pattern;
      static const ResKind product;
      static const ResKind script;
      static const ResKind selection;
      static const ResKind srcpackage;
      static const ResKind system;
      //@}

    public:
      /** Default ctor: \ref nokind */
      ResKind() {}

      /** Ctor taking kind as string. */
      explicit ResKind( sat::detail::IdType id_r )  : _str( str::toLower(IdString(id_r).c_str()) ) {}
      explicit ResKind( const IdString & idstr_r )  : _str( str::toLower(idstr_r.c_str()) ) {}
      explicit ResKind( const std::string & str_r ) : _str( str::toLower(str_r) ) {}
      explicit ResKind( const char * cstr_r )       : _str( str::toLower(cstr_r) ) {}

    private:
      static int _doCompare( const char * lhs,  const char * rhs )
      {
        if ( lhs == rhs ) return 0;
        if ( lhs && rhs ) return ::strcasecmp( lhs, rhs );
        return( lhs ? 1 : -1 );
      }

    private:
      friend class IdStringType<ResKind>;
      IdString _str;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESKIND_H
