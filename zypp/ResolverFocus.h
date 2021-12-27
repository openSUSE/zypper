/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResolverFocus.h
 */
#ifndef ZYPP_RESOLVERFOCUS_H
#define ZYPP_RESOLVERFOCUS_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  /** The resolver's general attitude. */
  enum class ResolverFocus
  {
    Default = 0	///< Request the standard behavior (as defined in zypp.conf or 'Job')
    ,Job	///< Focus on installing the best version of the requested packages
    ,Installed	///< Focus on applying as little changes to the installed packages as needed
    ,Update	///< Focus on updating requested packages and their dependencies as much as possible
  };

  /** \relates ResolverFocus Conversion to string (enumerator name) */
  std::string asString( const ResolverFocus & val_r );

  /** \relates ResolverFocus Conversion from string (enumerator name, case insensitive, empty string is Default)
   * \returns \c false if \a val_r is not recognized
   */
  bool fromString( const std::string & val_r, ResolverFocus & ret_r );

  /** \relates ResolverFocus Conversion from string (convenience)
   * \returns \ref ResolverFocus::Default if \a val_r is not recognized
   */
  inline ResolverFocus resolverFocusFromString( const std::string & val_r )
  { ResolverFocus ret_r { ResolverFocus::Default }; fromString( val_r, ret_r ); return ret_r; }

  /** \relates ResolverFocus Stream output */
  inline std::ostream & operator<<( std::ostream & str, const ResolverFocus & obj )
  { return str << asString( obj ); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVERFOCUS_H
