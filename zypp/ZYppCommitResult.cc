/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppCommitResult.cc
 *
*/

#include <iostream>

#include "zypp/ZYppCommitResult.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppCommitResult::Impl
  //
  ///////////////////////////////////////////////////////////////////

  class ZYppCommitResult::Impl
  {
    public:
      Impl()
      {}

    public:
      Pathname			_root;
      UpdateNotifications	_updateMessages;

    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const { return new Impl( *this ); }
  };

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppCommitResult
  //
  ///////////////////////////////////////////////////////////////////

  ZYppCommitResult::ZYppCommitResult()
  : _result(0), _pimpl( new Impl )
  {}

  ZYppCommitResult::ZYppCommitResult( const Pathname & root_r )
  : _result(0), _pimpl( new Impl )
  { _pimpl->_root = root_r; }

  const Pathname & ZYppCommitResult::root() const
  { return _pimpl->_root; }

  const UpdateNotifications & ZYppCommitResult::updateMessages() const
  { return _pimpl->_updateMessages; }

  UpdateNotifications & ZYppCommitResult::setUpdateMessages()
  { return _pimpl->_updateMessages; }

  ///////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const ZYppCommitResult & obj )
  {
    str << "CommitResult " << obj._result
        << " (errors " << obj._errors.size()
        << ", remaining " << obj._remaining.size()
        << ", srcremaining " << obj._srcremaining.size()
        << ", updateMessages " << obj.updateMessages().size()
        << ")";
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
