/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SolvAttr.h
 *
*/
#ifndef ZYPP_SolvAttr_H
#define ZYPP_SolvAttr_H

#include <iosfwd>
#include <string>

#include "zypp/base/String.h"
#include "zypp/IdStringType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace sat
{ /////////////////////////////////////////////////////////////////

  
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SolvAttr
  //
  /** Resolvable kinds.
   * A \b lowercased string and used as identification.
   * Comparison against string values is always case
   * insensitive.
   */
  class SolvAttr : public IdStringType<SolvAttr>
  {
    public:
      /** \name Some builtin SolvAttr constants. */
      //@{
      /** Value representing \c nokind (<tt>""</tt>)*/
      static const SolvAttr noAttr;
      static const SolvAttr summary;
      static const SolvAttr description;
      static const SolvAttr insnotify;
      static const SolvAttr delnotify;
      static const SolvAttr vendor;
      static const SolvAttr license;
      static const SolvAttr size;
      static const SolvAttr downloadsize;
      //@}

    public:
      /** Default ctor: \ref nokind */
      SolvAttr() {}

      /** Ctor taking kind as string. */
      explicit SolvAttr( sat::detail::IdType id_r )  : _str( str::toLower(IdString(id_r).c_str()) ) {}
      explicit SolvAttr( const IdString & idstr_r )  : _str( str::toLower(idstr_r.c_str()) ) {}
      explicit SolvAttr( const std::string & str_r ) : _str( str::toLower(str_r) ) {}
      explicit SolvAttr( const char * cstr_r )       : _str( str::toLower(cstr_r) ) {}

    public:
      private:
      static int _doCompare( const char * lhs,  const char * rhs )
      {
        if ( lhs == rhs ) return 0;
        if ( lhs && rhs ) return ::strcasecmp( lhs, rhs );
        return( lhs ? 1 : -1 );
      }

    private:
      friend class IdStringType<SolvAttr>;
      IdString _str;
  };

  /////////////////////////////////////////////////////////////////
} // namespace sat
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SolvAttr_H
