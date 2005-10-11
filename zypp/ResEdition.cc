/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ResEdition.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ResEdition.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResEdition::Impl
  //
  /** ResEdition implementation */
  struct ResEdition::Impl
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
  //	CLASS NAME : ResEdition
  //
  ///////////////////////////////////////////////////////////////////

  ResEdition::ResEdition()
  : _pimpl( new Impl )
  {}

  ResEdition::ResEdition( const std::string & version_r,
                          const std::string & release_r,
                          epoch_t epoch_r )
  : _pimpl( new Impl( version_r, release_r, epoch_r ) )
  {}

  ResEdition::~ResEdition()
  {}

  ResEdition::epoch_t ResEdition::epoch() const
  { return _pimpl->_epoch; }

  const std::string & ResEdition::version() const
  { return _pimpl->_version; }

  const std::string & ResEdition::release() const
  { return _pimpl->_release; }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ResEdition & obj )
  {
    return str << obj.version() << '-' << obj.release();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
