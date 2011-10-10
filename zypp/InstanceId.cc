/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/InstanceId.cc
 *
*/
//#include <iostream>
//#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"

#include "zypp/InstanceId.h"
#include "zypp/ResPool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  std::string InstanceId::getIdFor( sat::Solvable slv_r ) const
  {
    if ( ! slv_r )
      return std::string();

    std::string ret( _namespace );
    if ( ! ret.empty() )
      ret += ':';

    if ( slv_r.isKind<SrcPackage>() ) // libsolv uses no namespace in SrcPackage ident!
    {
      ret += ResKind::srcpackage.c_str();
      ret += ':';
    }

    ret += str::form( "%s-%s-%s.%s@%s",
                      slv_r.ident().c_str(),
                      slv_r.edition().version().c_str(),
                      slv_r.edition().release().c_str(),
                      slv_r.arch().c_str(),
                      slv_r.repository().alias().c_str() );
    return ret;
  }

  PoolItem InstanceId::findPoolItem( const std::string str_r ) const
  {
    // [namespace:]<name>-<version>-<release>.<arch>@<repoalias>
    std::string::size_type namespaceOff( _namespace.size() );

    if ( namespaceOff )
    {
      if ( ! str::hasPrefix( str_r, _namespace ) || str_r[namespaceOff] != ':' )
        return PoolItem();
      ++namespaceOff; // for the ':'
    }

    // check repo
    std::string::size_type rdelim( str_r.find( "@" ) );
    if ( rdelim == std::string::npos )
      return PoolItem();

    Repository repo( sat::Pool::instance().reposFind( str_r.substr( rdelim+1) ) );
    if ( ! repo )
      return PoolItem();

    // check n-v-r.a from behind
    std::string::size_type delim = str_r.rfind( ".", rdelim );
    if ( delim == std::string::npos )
      return PoolItem();

    Arch arch( str_r.substr( delim+1, rdelim-delim-1 ) );

    // v-r starts at one but last '-'
    rdelim = delim;
    delim = str_r.rfind( "-", rdelim );
    if ( delim == std::string::npos )
      return PoolItem();

    if ( delim == rdelim-1 ) // supress an empty release
      rdelim = delim;

    delim = str_r.rfind( "-", delim-1 );
    if ( delim == std::string::npos )
      return PoolItem();

    Edition ed( str_r.substr( delim+1, rdelim-delim-1 ) );

    // eveythig before is name (except the leading "<namespace>:")
    std::string identstring( str_r.substr( namespaceOff, delim-namespaceOff ) );

    // now lookup in pool..
    sat::Solvable::SplitIdent ident( (IdString(identstring)) );
    ResPool pool( ResPool::instance() );
    for_( it, pool.byIdentBegin( ident.kind(), ident.name() ), pool.byIdentEnd( ident.kind(), ident.name() ) )
    {
      sat::Solvable solv( (*it).satSolvable() );
      if ( solv.repository() == repo && solv.arch() == arch && solv.edition() == ed )
      {
        return *it;
      }
    }
    return PoolItem();
  }

  bool InstanceId::isSystemId( const std::string str_r ) const
  { return str::hasSuffix( str_r, Repository::systemRepoAlias() ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
