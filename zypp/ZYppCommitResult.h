/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppCommitResult.h
 *
*/
#ifndef ZYPP_ZYPPCOMMITRESULT_H
#define ZYPP_ZYPPCOMMITRESULT_H

#include <iosfwd>
#include <vector>
#include <list>

#include "zypp/PoolItem.h"
#include "zypp/sat/Transaction.h"
#include "zypp/base/DefaultIntegral.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace sat
  {
    class Transaction;
  }

  /** Pair of \ref sat::Solvable and \ref Pathname. */
  class UpdateNotificationFile
  {
    public:
      UpdateNotificationFile( sat::Solvable solvable_r, const Pathname & file_r )
      : _solvable( solvable_r ), _file( file_r )
      {}
    public:
      sat::Solvable solvable() const { return _solvable; }
      const Pathname & file() const { return _file; }
    private:
      sat::Solvable _solvable;
      Pathname      _file;
  };

  typedef std::list<UpdateNotificationFile> UpdateNotifications;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppCommitResult
  //
  /** Result returned from ZYpp::commit.
   *
   * \note Transaction data are provided and maintained during commit.
   * Though the interface does not inhibit manipulation of transaction
   * data outside commit (those methods could have been made \c private:),
   * this is not recommended as you may easily mess up things.
   *
   * \see \ref ZYpp::commit
   */
  class ZYppCommitResult
  {
    public:
      typedef std::vector<sat::Transaction::Step> TransactionStepList;

    public:
      ZYppCommitResult();
      ZYppCommitResult( const Pathname & root_r );

    public:
      /** Remembered root directory of the target.
       *  \Note Pathnames within this class are relative to the
       * targets root directory.
      */
      const Pathname & root() const;

      /** The full transaction list.
       * The complete list including transaction steps that do not require
       * any action (like obsoletes or non-package actions). Depending on
       * \ref ZYppCommitPolicy::restrictToMedia only a subset of this
       * transaction might have been executed.
       * \see \ref transactionStepList.
       */
      const sat::Transaction & transaction() const;

      /** Manipulate \ref transaction */
      sat::Transaction & rTransaction();

      /** List of \ref sat::Transaction::Step to be executed by commit.
       * The list of transaction step commit actually tried to execute.
       */
      const TransactionStepList & transactionStepList() const;

      /** Manipulate \ref transactionStepList. */
      TransactionStepList & rTransactionStepList();

      /** List of update messages installed during this commit.
       * \Note Pathnames are relative to the targets root directory.
       * \code
       *   ZYppCommitResult result;
       *   ...
       *   if ( ! result.updateMessages().empty() )
       *   {
       *     MIL << "Received " << result.updateMessages().size() << " update notification(s):" << endl;
       *     for_( it, result.updateMessages().begin(), result.updateMessages().end() )
       *     {
       *       MIL << "- From " << it->solvable().asString() << " in file " << Pathname::showRootIf( result.root(), it->file() ) << ":" << endl;
       *       {
       *         // store message files content in a string:
       *         InputStream istr( Pathname::assertprefix( result.root(), it->file() ) );
       *         std::ostringstream strstr;
       *         iostr::copy( istr, strstr );
       *         std::string message( strstr.str() ); // contains the message
       *       }
       *       {
       *         // or write out the message file indented:
       *         InputStream istr( Pathname::assertprefix( result.root(), it->file() ) );
       *         iostr::copyIndent( istr, MIL, "> " ) << endl;
       *       }
       *     }
       *   }
       * \endcode
       */
      const UpdateNotifications & updateMessages() const;

      /** Manipulate \ref updateMessages
       * \Note Pathnames are relative to the targets root directory.
       */
      UpdateNotifications & rUpdateMessages();

    public:

      /** \name Some statistics based on \ref transaction
       */
      //@{
	/** Basically std::pair<unsigned,unsigned>, but default ctor sets <0,0>. */
	typedef std::pair<DefaultIntegral<unsigned,0>, DefaultIntegral<unsigned,0> > InsDelCnt;

	/** Total number of install/delete actions to be performed. */
	InsDelCnt totalCount() const;

	/** Number of install/delete actions with result \a stage_r. */
	InsDelCnt stepStageCount( sat::Transaction::StepStage stage_r ) const;

	/** Number of install/delete actions done. */
	InsDelCnt doneCount() const
	{ return stepStageCount( sat::Transaction::STEP_DONE ); }

	/** Number of install/delete actions that failed with error. */
	InsDelCnt errorCount() const
	{ return stepStageCount( sat::Transaction::STEP_ERROR ); }

	/** Number of install/delete actions that were skipped (e.g. due to unwanted media). */
	InsDelCnt skippedCount() const
	{ return stepStageCount( sat::Transaction::STEP_TODO ); }

	/** All in one go. */
	void resultCount( InsDelCnt & total_r, InsDelCnt & done_r, InsDelCnt & error_r, InsDelCnt & skipped_r ) const;
      //@}

    public:
      /** \name Oldstlye interface to be removed asap.
       * \deprecated PoolItem is not suitable for reporting errors about
       * packages to be deteled, as reloading the rpm database after commit
       * invalidates them.
       */
      //@{
      typedef std::list<PoolItem> PoolItemList;
      /**
       * number of committed resolvables
       **/
      int          _result ZYPP_DEPRECATED;
      /**
       * list of resolvables with error
       **/
      PoolItemList _errors ZYPP_DEPRECATED;
      /**
       * list of resolvables remaining (due to wrong media)
       **/
      PoolItemList _remaining ZYPP_DEPRECATED;
      /**
       * list of kind:source resolvables remaining (due to wrong media)
       **/
      PoolItemList _srcremaining ZYPP_DEPRECATED;
      //@}

    public:
      /** Implementation  */
      class Impl;
    private:
      /** Pointer to data. */
      RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ZYppCommitResult Stream output. */
  std::ostream & operator<<( std::ostream & str, const ZYppCommitResult & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPPCOMMITRESULT_H
