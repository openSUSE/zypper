 /*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/CommitLog.cc
 *
*/
#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"

#include "zypp/target/CommitLog.h"
#include "zypp/PathInfo.h"
#include "zypp/Date.h"

using std::endl;

namespace zypp {
  namespace target {

    ///////////////////////////////////////////////////////////////////

    Pathname CommitLog::_fname;
    std::ofstream CommitLog::_log;
    unsigned CommitLog::_refcnt = 0;

    ///////////////////////////////////////////////////////////////////

    void CommitLog::openLog() {
      if ( !_fname.empty() ) {
        _log.clear();
        _log.open( _fname.asString().c_str(), std::ios::out|std::ios::app );
        if( !_log )
          ERR << "Could not open logfile '" << _fname << "'" << endl;
      }
    }
    void CommitLog::closeLog() {
      _log.clear();
      _log.close();
    }
    void CommitLog::refUp() {
      if ( !_refcnt )
        openLog();
      ++_refcnt;
    }
    void CommitLog::refDown() {
      --_refcnt;
      if ( !_refcnt )
        closeLog();
    }

    std::ostream & CommitLog::operator()( bool timestamp ) {
      if ( timestamp ) {
        _log << Date(Date::now()).form( "%Y-%m-%d %H:%M:%S ");
      }
      return _log;
    }

    void CommitLog::setFname( const Pathname & fname_r ) {
      MIL << "installation log file " << fname_r << endl;
      if ( _refcnt )
        closeLog();
      _fname = fname_r;
      if ( ! _fname.empty() )
        filesystem::assert_dir( _fname.dirname() );
      if ( _refcnt )
        openLog();
    }

    const Pathname & CommitLog::fname()
    { return _fname; }

  } // namespace target
} // namespace zypp
