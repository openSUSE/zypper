/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/HistoryLog.cc
 *
 */
#include <iostream>
#include <fstream>
#include <unistd.h>

#include "zypp/ZConfig.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

#include "zypp/PathInfo.h"
#include "zypp/Date.h"

#include "zypp/PoolItem.h"
#include "zypp/Package.h"
#include "zypp/RepoInfo.h"

#include "zypp/HistoryLog.h"

using std::endl;
using std::string;

namespace
{
  inline string timestamp()
  { return zypp::Date::now().form( "%Y-%m-%d %H:%M:%S" ); }

  inline string userAtHostname()
  {
    static char buf[256];
    string result;
    char * tmp = ::cuserid(buf); 
    if (tmp)
    {
      result = string(tmp);
      if (!::gethostname(buf, 255))
        result += "@" + string(buf);
    }
    return result;
  }
}

namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : HistoryActionID
  //
  ///////////////////////////////////////////////////////////////////

  static std::map<std::string,HistoryActionID::ID> _table;

  const HistoryActionID HistoryActionID::NONE(HistoryActionID::NONE_e);
  const HistoryActionID HistoryActionID::INSTALL(HistoryActionID::INSTALL_e);
  const HistoryActionID HistoryActionID::REMOVE(HistoryActionID::REMOVE_e);
  const HistoryActionID HistoryActionID::REPO_ADD(HistoryActionID::REPO_ADD_e);
  const HistoryActionID HistoryActionID::REPO_REMOVE(HistoryActionID::REPO_REMOVE_e);
  const HistoryActionID HistoryActionID::REPO_CHANGE_ALIAS(HistoryActionID::REPO_CHANGE_ALIAS_e);
  const HistoryActionID HistoryActionID::REPO_CHANGE_URL(HistoryActionID::REPO_CHANGE_URL_e);

  HistoryActionID::HistoryActionID(const std::string & strval_r)
    : _id(parse(strval_r))
  {}

  HistoryActionID::ID HistoryActionID::parse(const std::string & strval_r)
  {
    if (_table.empty())
    {
      // initialize it
      _table["install"] = INSTALL_e;
      _table["remove"]  = REMOVE_e;
      _table["radd"]    = REPO_ADD_e;
      _table["rremove"] = REPO_REMOVE_e;
      _table["ralias"]  = REPO_CHANGE_ALIAS_e;
      _table["rurl"]    = REPO_CHANGE_URL_e;
      _table["NONE"] = _table["none"] = HistoryActionID::NONE_e;
    }

    std::map<std::string,HistoryActionID::ID>::const_iterator it =
      _table.find(strval_r);

    if (it == _table.end())
      WAR << "Unknown history action ID '" + strval_r + "'";

    return it->second;
  }


  const std::string & HistoryActionID::asString(bool pad) const
  {
    static std::map<ID, std::string> _table;
    if ( _table.empty() )
    {
      // initialize it
      _table[INSTALL_e]           = "install";
      _table[REMOVE_e]            = "remove";
      _table[REPO_ADD_e]          = "radd";
      _table[REPO_REMOVE_e]       = "rremove";
      _table[REPO_CHANGE_ALIAS_e] = "ralias";
      _table[REPO_CHANGE_URL_e]   = "rurl";
      _table[NONE_e]              = "NONE";
    }
    // add spaces so that the size of the returned string is always 7 (for now)
    if (pad)
      return _table[_id].append(7 - _table[_id].size(), ' ');
    return _table[_id];
  }

  std::ostream & operator << (std::ostream & str, const HistoryActionID & id)
  { return str << id.asString(); }

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : HistoryLog
  //
  ///////////////////////////////////////////////////////////////////

    Pathname HistoryLog::_fname(ZConfig::instance().historyLogFile());
    std::ofstream HistoryLog::_log;
    unsigned HistoryLog::_refcnt = 0;
    const char HistoryLog::_sep = '|';

    ///////////////////////////////////////////////////////////////////

    HistoryLog::HistoryLog( const Pathname & rootdir )
    {
      refUp();
      if (!rootdir.empty() && rootdir.absolute())
        _fname = rootdir / ZConfig::instance().historyLogFile();
    }

    void HistoryLog::openLog()
    {
      if ( !_fname.empty() )
      {
        _log.clear();
        _log.open( _fname.asString().c_str(), std::ios::out|std::ios::app );
        if( !_log )
          ERR << "Could not open logfile '" << _fname << "'" << endl;
      }
    }

    void HistoryLog::closeLog()
    {
      _log.clear();
      _log.close();
    }

    void HistoryLog::refUp()
    {
      if ( !_refcnt )
        openLog();
      ++_refcnt;
    }

    void HistoryLog::refDown()
    {
      --_refcnt;
      if ( !_refcnt )
        closeLog();
    }


    void HistoryLog::setRoot( const Pathname & rootdir )
    {
      if (rootdir.empty() || !rootdir.absolute())
        return;

      if ( _refcnt )
        closeLog();

      _fname = rootdir / "/var/log/zypp/history";
      filesystem::assert_dir( _fname.dirname() );
      MIL << "installation log file " << _fname << endl;

      if ( _refcnt )
        openLog();
    }


    const Pathname & HistoryLog::fname()
    { return _fname; }

    /////////////////////////////////////////////////////////////////////////

    void HistoryLog::comment( const string & comment, bool timestamp )
    {
      if (comment.empty())
        return;

      _log << "# ";
      if ( timestamp )
        _log << ::timestamp() << " ";

      const char * s = comment.c_str();
      const char * c = s;
      unsigned size = comment.size();

      // ignore the last newline
      if (comment[size-1] == '\n')
        --size;

      for ( unsigned i = 0; i < size; ++i, ++c )
        if ( *c == '\n' )
        {
          _log << string( s, c + 1 - s ) << "# ";
          s = c + 1;
        }

      if ( s < c )
        _log << std::string( s, c-s );

      _log << endl;
    }

    /////////////////////////////////////////////////////////////////////////
    
    void HistoryLog::install( const PoolItem & pi )
    {
      const Package::constPtr p = asKind<Package>(pi.resolvable());
      if (!p)
        return;

      _log
        << timestamp()                                   // 1 timestamp
        << _sep << HistoryActionID::INSTALL.asString(true) // 2 action
        << _sep << p->name()                             // 3 name
        << _sep << p->edition()                          // 4 evr
        << _sep << p->arch();                            // 5 arch

      if (pi.status().isByUser())
        _log << _sep << userAtHostname();                // 6 reqested by
      //else if (pi.status().isByApplHigh() || pi.status().isByApplLow())
      //  _log << _sep << "appl";
      else
        _log << _sep;

      _log
        << _sep << p->repoInfo().alias()                 // 7 repo alias
        << _sep << p->checksum().checksum();             // 8 checksum

      _log << endl; 

      //_log << pi << endl;
    }


    void HistoryLog::remove( const PoolItem & pi )
    {
      const Package::constPtr p = asKind<Package>(pi.resolvable());
      if (!p)
        return;

      _log
        << timestamp()                                   // 1 timestamp
        << _sep << HistoryActionID::REMOVE.asString(true) // 2 action
        << _sep << p->name()                             // 3 name
        << _sep << p->edition()                          // 4 evr
        << _sep << p->arch();                            // 5 arch

      if (pi.status().isByUser())
        _log << _sep << userAtHostname();                // 6 reqested by
      //else if (pi.status().isByApplHigh() || pi.status().isByApplLow())
      //  _log << _sep << "appl";
      else
        _log << _sep;

      // we don't have checksum in rpm db
      //  << _sep << p->checksum().checksum();           // x checksum

      _log << endl; 

      //_log << pi << endl;
    }

    /////////////////////////////////////////////////////////////////////////

    void HistoryLog::addRepository(const RepoInfo & repo)
    {
      _log
        << timestamp()                                   // 1 timestamp
        << _sep << HistoryActionID::REPO_ADD.asString(true) // 2 action 
        << _sep << repo.alias()                          // 3 alias
        // what about the rest of the URLs??
        << _sep << *repo.baseUrlsBegin()                 // 4 primary URL
        << endl;
    }


    void HistoryLog::removeRepository(const RepoInfo & repo)
    {
      _log
        << timestamp()                                   // 1 timestamp
        << _sep << HistoryActionID::REPO_REMOVE.asString(true) // 2 action 
        << _sep << repo.alias()                          // 3 alias
        << endl;
    }


    void HistoryLog::modifyRepository(
        const RepoInfo & oldrepo, const RepoInfo & newrepo)
    {
      if (oldrepo.alias() != newrepo.alias())
      {
        _log
          << timestamp()                                    // 1 timestamp
          << _sep << HistoryActionID::REPO_CHANGE_ALIAS.asString(true) // 2 action
          << _sep << oldrepo.alias()                        // 3 old alias
          << _sep << newrepo.alias();                       // 4 new alias
      }
      
      if (*oldrepo.baseUrlsBegin() != *newrepo.baseUrlsBegin())
      {
        _log
          << timestamp()                                    // 1 timestamp
          << _sep << HistoryActionID::REPO_CHANGE_URL.asString(true) // 2 action
          << _sep << oldrepo.alias()                        // 3 old url
          << _sep << newrepo.alias();                       // 4 new url
      }
    }

    ///////////////////////////////////////////////////////////////////

} // namespace zypp
