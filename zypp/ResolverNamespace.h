/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResolverNamespace.h
 */
#ifndef ZYPP_RESOLVERNAMESPACE_H
#define ZYPP_RESOLVERNAMESPACE_H

#include <iosfwd>
#include <cstdint>

#include "zypp/base/Flags.h"
#include "zypp/IdString.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  /** The resolvers dependency namespaces */
  enum class ResolverNamespace : std::uint8_t
  {
    language	= 1<<0,	///< language support
    modalias	= 1<<1,	///< hardware support
    filesystem	= 1<<2,	///< filesystems
  };

  /** \relates ResolverNamespace Flags */
  ZYPP_DECLARE_FLAGS_AND_OPERATORS(ResolverNamespaces,ResolverNamespace);

  /** \relates ResolverNamespace The underlying libsolv ID */
  inline constexpr IdString asIdString( ResolverNamespace obj )
  {
    return IdString( obj == ResolverNamespace::language ? sat::detail::namespaceLanguage
                   : obj == ResolverNamespace::modalias ? sat::detail::namespaceModalias
                   : obj == ResolverNamespace::filesystem ? sat::detail::namespaceFilesystem
                   : sat::detail::noId );
  }

  /** \relates ResolverNamespace String representation */
  inline std::string asString( ResolverNamespace obj )
  { return asIdString( obj ).asString(); }

  /** \relates ResolverNamespace Stream output */
  inline std::ostream & operator<<( std::ostream & str, ResolverNamespace obj )
  { return str << asIdString( obj ); }

  /** \relates ResolverNamespaces Stream output */
  inline std::ostream & operator<<( std::ostream & str, ResolverNamespaces obj )
  {
    return str << stringify( obj, {
      { ResolverNamespace::language,	"language" },
      { ResolverNamespace::modalias,	"modalias" },
      { ResolverNamespace::filesystem,	"filesystem" },
    }, "namespace:", "|", "" );
  }

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVERNAMESPACE_H
