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
  : _result(0), _pimpl( new Impl )
  {}

  ZYppCommitResult::ZYppCommitResult( const Pathname & root_r )
  : _result(0), _pimpl( new Impl )
  { _pimpl->_root = root_r; }

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

  ZYppCommitResult::InsDelCnt ZYppCommitResult::totalCount() const
  {
    InsDelCnt ret;
    for_( it, _pimpl->_transaction.actionBegin(), _pimpl->_transaction.actionEnd() )
    {
      ++( it->stepType() == sat::Transaction::TRANSACTION_ERASE ? ret.second : ret.first );
    }

    return ret;
  }

  ZYppCommitResult::InsDelCnt ZYppCommitResult::stepStageCount( sat::Transaction::StepStage stage_r ) const
  {
    InsDelCnt ret;
    for_( it, _pimpl->_transaction.actionBegin(), _pimpl->_transaction.actionEnd() )
    {
      if ( it->stepStage() != stage_r )
	continue;
      ++( it->stepType() == sat::Transaction::TRANSACTION_ERASE ? ret.second : ret.first );
    }
    return ret;
  }

  void  ZYppCommitResult::resultCount( InsDelCnt & total_r, InsDelCnt & done_r, InsDelCnt & error_r, InsDelCnt & skipped_r ) const
  {
    total_r = done_r = error_r = skipped_r = InsDelCnt();
    for_( it, _pimpl->_transaction.actionBegin(), _pimpl->_transaction.actionEnd() )
    {
      ++( it->stepType() == sat::Transaction::TRANSACTION_ERASE ? total_r.second : total_r.first );
      switch ( it->stepStage() )
      {
	case sat::Transaction::STEP_DONE:
	  ++( it->stepType() == sat::Transaction::TRANSACTION_ERASE ? done_r.second : done_r.first );
	  break;
	case sat::Transaction::STEP_ERROR:
	  ++( it->stepType() == sat::Transaction::TRANSACTION_ERASE ? error_r.second : error_r.first );
	  break;
	case sat::Transaction::STEP_TODO:
	  ++( it->stepType() == sat::Transaction::TRANSACTION_ERASE ? skipped_r.second : skipped_r.first );
	  break;
      }
    }
  }

  ///////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const ZYppCommitResult::InsDelCnt & obj )
  { return str << obj.first << '/' << obj.second; }

  std::ostream & operator<<( std::ostream & str, const ZYppCommitResult & obj )
  {
    ZYppCommitResult::InsDelCnt result[4];
    obj.resultCount( result[0], result[1], result[2], result[3] );

    str << "CommitResult "
        << " (ins/del total " << result[0]
        << ", done " << result[1]
        << ", error " << result[2]
        << ", skipped " << result[3]
        << ", updateMessages " << obj.updateMessages().size()
        << ")"
        << std::endl << obj.transaction();
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
