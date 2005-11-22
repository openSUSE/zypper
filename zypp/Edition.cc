/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Edition.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/Edition.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Edition::Impl
  //
  /** Edition implementation */
  struct Edition::Impl
  {
    /** Default ctor*/
    Impl()
    : _epoch( 0 )
    {}

    Impl( const std::string & version_r,
          const std::string & release_r,
          epoch_t epoch_r )
    : _epoch( epoch_r )
    , _version( version_r )
    , _release( release_r )
    {}

    /** Dtor */
    ~Impl()
    {}

    epoch_t      _epoch;
    std::string _version;
    std::string _release;
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Edition
  //
  ///////////////////////////////////////////////////////////////////

  Edition::Edition()
  : _pimpl( new Impl )
  {}

  Edition::Edition( const std::string & version_r,
                    const std::string & release_r,
                    epoch_t epoch_r )
  : _pimpl( new Impl( version_r, release_r, epoch_r ) )
  {}

  Edition::~Edition()
  {}

  /** \todo Beautyfy */
  std::string Edition::asString() const
  { return _pimpl->_version + "-" + _pimpl->_release; }

  Edition::epoch_t Edition::epoch() const
  { return _pimpl->_epoch; }

  const std::string & Edition::version() const
  { return _pimpl->_version; }

  const std::string & Edition::release() const
  { return _pimpl->_release; }

  /** \todo implement */
  bool Edition::compare( RelOp op, const Edition & lhs, const Edition & rhs )
  {
    return false;
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
