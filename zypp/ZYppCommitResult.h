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

    public:
      /** List of update messages installed during this commit.
       * \Note Pathnames are relative to the targets root directory.
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
