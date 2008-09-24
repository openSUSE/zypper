/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/HistoryLog.h
 *
 */
#ifndef ZYPP_TARGET_COMMITLOG_H
#define ZYPP_TARGET_COMMITLOG_H

#include <iosfwd>

#include "zypp/Pathname.h"

namespace zypp
{
  class PoolItem;
  class RepoInfo;
  
  struct HistoryActionID
  {
    static const HistoryActionID NONE;

    static const HistoryActionID INSTALL;
    static const HistoryActionID REMOVE;
    static const HistoryActionID REPO_ADD;
    static const HistoryActionID REPO_REMOVE;
    static const HistoryActionID REPO_CHANGE_ALIAS;
    static const HistoryActionID REPO_CHANGE_URL;

    enum ID
    {
      NONE_e,

      INSTALL_e,
      REMOVE_e,
      REPO_ADD_e,
      REPO_REMOVE_e,
      REPO_CHANGE_ALIAS_e,
      REPO_CHANGE_URL_e
    };

    HistoryActionID() : _id(NONE_e) {}

    HistoryActionID(ID id) : _id(id) {}

    explicit HistoryActionID(const std::string & strval_r);

    ID toEnum() const { return _id; }

    HistoryActionID::ID parse(const std::string & strval_r);

    const std::string & asString(bool pad = false) const;

    ID _id;
  };

  /** \relates HistoryActionID */
  std::ostream & operator << (std::ostream & str, const HistoryActionID & id);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : HistoryLog
    /**
     * Simple wrapper for progress log. Refcnt, filename and corresponding
     * ofstream are static members. Logfile constructor raises, destructor
     * lowers refcounter. On refcounter changing from 0->1, file is opened.
     * Changing from 1->0 the file is closed. Thus Logfile objects should be
     * local to those functions, writing the log, and must not be stored
     * permanently.
     *
     * Usage:
     * <code>
     *  some method ()
     *  {
     *    PoolItem pi;
     *    ...
     *    HistoryLog().install(pi);
     *    ...
     *    HistoryLog().comment(someMessage);
     *  }
     * </code>
     * 
     * \note Take care to set proper target root dir if needed. Either pass
     *       it via the constructor, or set it via setRoot(Pathname) method.
     *       The default location of the file is determined by
     *       \ref ZConfig::historyLogPath() which defaults to
     *       /var/log/zypp/history.
     */
    class HistoryLog
    {
      HistoryLog( const HistoryLog & );
      HistoryLog & operator=( const HistoryLog & );
    private:
      static std::ofstream _log;
      static unsigned _refcnt;
      static Pathname _fname;
      static const char _sep;

      static void openLog();
      static void closeLog();
      static void refUp();
      static void refDown();

    public:
      HistoryLog( const Pathname & rootdir = Pathname() );
      ~HistoryLog()
      { refDown(); }

      static void setRoot( const Pathname & root );
      static const Pathname & fname();

      void comment( const std::string & comment, bool timestamp = false );
      void install( const PoolItem & pi );
      void remove( const PoolItem & pi );

      void addRepository( const RepoInfo & repo );
      void removeRepository( const RepoInfo & repo );
      void modifyRepository( const RepoInfo & oldrepo, const RepoInfo & newrepo );
    };
    ///////////////////////////////////////////////////////////////////

} // namespace zypp

#endif // ZYPP_TARGET_COMMITLOG_H
