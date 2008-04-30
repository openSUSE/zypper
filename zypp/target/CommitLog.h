 /*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/CommitLog.h
 *
*/
#ifndef ZYPP_TARGET_COMMITLOG_H
#define ZYPP_TARGET_COMMITLOG_H

#include <iosfwd>

#include "zypp/Pathname.h"

namespace zypp {
  namespace target {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CommitLog
    /**
     * Simple wrapper for progress log. Refcnt, filename and corresponding
     * ofstream are static members. Logfile constructor raises, destructor
     * lowers refcounter. On refcounter changing from 0->1, file is opened.
     * Changing from 1->0 the file is closed. Thus Logfile objects should be
     * local to those functions, writing the log, and must not be stored
     * permanently;
     *
     * Usage:
     *  some methothd ()
     *  {
     *    CommitLog progresslog;
     *    ...
     *    progresslog() << "some message" << endl;
     *    ...
     *  }
     **/
    class CommitLog {
      CommitLog( const CommitLog & );
      CommitLog & operator=( const CommitLog & );
    private:
      static std::ofstream _log;
      static unsigned _refcnt;
      static Pathname _fname;

      static void openLog();
      static void closeLog();
      static void refUp();
      static void refDown();
    public:
      CommitLog() { refUp(); }
      ~CommitLog() { refDown(); }
      std::ostream & operator()( bool timestamp = false );
      static void setFname( const Pathname & fname_r );
      static const Pathname & fname();
    };
    ///////////////////////////////////////////////////////////////////

  } // namespace target
} // namespace zypp


#endif // ZYPP_TARGET_COMMITLOG_H
