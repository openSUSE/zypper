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
#include <list>

#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

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
   * \see \ref ZYpp::commit
   * \todo document fields.
   */
  class ZYppCommitResult
  {
    public:
      ZYppCommitResult();
      ZYppCommitResult( const Pathname & root_r );

    public:
      /** Remembered root directory of the target.
       *  \Note Pathnames within this class are relative to the
       * targets root directory.
      */
      const Pathname & root() const;

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

      /** Change list of update messages installed during this commit.
       * \Note Pathnames are relative to the targets root directory.
       */
      UpdateNotifications & setUpdateMessages();

    public:
      /** \name Oldstlye interface to be removed asap.
       */
      //@{
      typedef std::list<PoolItem> PoolItemList;
      /**
       * number of committed resolvables
       **/
      int          _result;
      /**
       * list of resolvables with error
       **/
      PoolItemList _errors;
      /**
       * list of resolvables remaining (due to wrong media)
       **/
      PoolItemList _remaining;
      /**
       * list of kind:source resolvables remaining (due to wrong media)
       **/
      PoolItemList _srcremaining;
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
