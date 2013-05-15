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
#include "zypp/base/LogTools.h"

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
      sat::Transaction          _transaction;
      TransactionStepList       _transactionStepList;
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
  : _pimpl( new Impl )
  {}

  ZYppCommitResult::ZYppCommitResult( const ZYppCommitResult & lhs_r )
  : _pimpl( lhs_r._pimpl )
  {}

  ZYppCommitResult::ZYppCommitResult( const Pathname & root_r )
  : _pimpl( new Impl )
  { _pimpl->_root = root_r; }

  ZYppCommitResult::~ZYppCommitResult()
  {}

  const Pathname & ZYppCommitResult::root() const
  { return _pimpl->_root; }

  const sat::Transaction & ZYppCommitResult::transaction() const
  { return _pimpl->_transaction; }

  sat::Transaction & ZYppCommitResult::rTransaction()
  { return _pimpl->_transaction; }

  const ZYppCommitResult::TransactionStepList & ZYppCommitResult::transactionStepList() const
  { return _pimpl->_transactionStepList; }

  ZYppCommitResult::TransactionStepList & ZYppCommitResult::rTransactionStepList()
  { return _pimpl->_transactionStepList; }

  const UpdateNotifications & ZYppCommitResult::updateMessages() const
  { return _pimpl->_updateMessages; }

  UpdateNotifications & ZYppCommitResult::rUpdateMessages()
  { return _pimpl->_updateMessages; }

  ///////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const ZYppCommitResult & obj )
  {
    DefaultIntegral<unsigned,0> result[4];
    for_( it, obj.transaction().actionBegin(), obj.transaction().actionEnd() )
    {
      ++result[0];
      switch ( it->stepStage() )
      {
	case sat::Transaction::STEP_DONE :	++result[1]; break;
	case sat::Transaction::STEP_ERROR :	++result[2]; break;
	case sat::Transaction::STEP_TODO :	++result[3]; break;
      }
    }
    str << "CommitResult "
        << " (total " << result[0]
        << ", done " << result[1]
        << ", error " << result[2]
        << ", skipped " << result[3]
        << ", updateMessages " << obj.updateMessages().size()
        << ")";
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
