/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CpeId.cc
 */
#include <iostream>
//#include "zypp/base/LogTools.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/CpeId.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class CpeId::Impl
  /// \brief CpeId implementation.
  ///////////////////////////////////////////////////////////////////
  class CpeId::Impl : private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const Impl & obj );
  public:
    Impl() {}

    Impl( const std::string & cpe_r )
      : _cpe( cpe_r )
    {}

    Impl( const std::string & cpe_r, NoThrowType )
      : _cpe( cpe_r )
    {}

  public:
    const std::string & asString() const
    { return _cpe; }

    Match match( const Impl & rhs ) const
    {
      return _cpe == rhs._cpe ? Match::equal : Match::undefined;
    }

  private:
    std::string _cpe;
  };

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CpeId
  //
  ///////////////////////////////////////////////////////////////////

  CpeId::CpeId()
    : _pimpl( new Impl )
  {}

  CpeId::CpeId( const std::string & cpe_r )
    : _pimpl( new Impl( cpe_r ) )
  {}

  CpeId::CpeId( const std::string & cpe_r, NoThrowType )
    : _pimpl( new Impl( cpe_r, noThrow ) )
  {}

  CpeId::~CpeId()
  {}

  CpeId::operator bool() const
  { return !_pimpl->asString().empty(); }

  std::string CpeId::asString() const
  { return _pimpl->asString(); }

  CpeId::Match CpeId::match( const CpeId & rhs ) const
  { return _pimpl->match( *rhs._pimpl ); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
