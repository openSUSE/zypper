/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include "zypp/base/NamedValue.h"
#include "zypp/repo/RepoException.h"
#include "RepoType.h"

namespace zypp
{
namespace repo
{
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    static NamedValue<RepoType::Type> & table()
    {
      static NamedValue<RepoType::Type> & _t( *new NamedValue<RepoType::Type> );
      if ( _t.empty() )
      {
	_t( RepoType::RPMMD_e )		| "rpm-md"	| "rpm"|"rpmmd"|"repomd"|"yum"|"up2date";
	_t( RepoType::YAST2_e )		| "yast2"	| "yast"|"susetags";
	_t( RepoType::RPMPLAINDIR_e )	| "plaindir";
	_t( RepoType::NONE_e )		| "NONE"	| "none";
      }
      return _t;
    }
  } // namespace
  ///////////////////////////////////////////////////////////////////

  const RepoType RepoType::RPMMD	( RepoType::RPMMD_e );
  const RepoType RepoType::YAST2	( RepoType::YAST2_e );
  const RepoType RepoType::RPMPLAINDIR	( RepoType::RPMPLAINDIR_e );
  const RepoType RepoType::NONE		( RepoType::NONE_e );

  RepoType::RepoType(const std::string & strval_r)
    : _type(parse(strval_r))
  {}

  RepoType::Type RepoType::parse( const std::string & strval_r )
  {
    RepoType::Type type;
    if ( ! table().getValue( str::toLower( strval_r ), type ) )
    {
      ZYPP_THROW( RepoUnknownTypeException( "Unknown repository type '" + strval_r + "'") );
    }
    return type;
  }

  const std::string & RepoType::asString() const
  {
    return table().getName( _type );
  }


  } // ns repo
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
