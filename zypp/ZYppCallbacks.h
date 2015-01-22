/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ZYppCallbacks.h
 *
*/
#ifndef ZYPP_ZYPPCALLBACKS_H
#define ZYPP_ZYPPCALLBACKS_H

#include "zypp/base/EnumClass.h"
#include "zypp/Callback.h"
#include "zypp/UserData.h"
#include "zypp/Resolvable.h"
#include "zypp/RepoInfo.h"
#include "zypp/Pathname.h"
#include "zypp/Package.h"
#include "zypp/Patch.h"
#include "zypp/Url.h"
#include "zypp/ProgressData.h"
#include "zypp/media/MediaUserAuth.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    class Queue;
    class FileConflicts;
  } // namespace sat
  ///////////////////////////////////////////////////////////////////

  struct ProgressReport : public callback::ReportBase
  {
    virtual void start( const ProgressData &/*task*/ )
    {}

    virtual bool progress( const ProgressData &/*task*/ )
    { return true; }

//     virtual Action problem(
//         Repo /*source*/
//         , Error /*error*/
//         , const std::string &/*description*/ )
//     { return ABORT; }

    virtual void finish( const ProgressData &/*task*/ )
    {}

  };

  struct ProgressReportAdaptor
  {

    ProgressReportAdaptor( const ProgressData::ReceiverFnc &fnc,
                           callback::SendReport<ProgressReport> &report )
      : _fnc(fnc)
      , _report(report)
      , _first(true)
    {
    }

    bool operator()( const ProgressData &progress )
    {
      if ( _first )
      {
        _report->start(progress);
        _first = false;
      }

      _report->progress(progress);
      bool value = true;
      if ( _fnc )
        value = _fnc(progress);


      if ( progress.finalReport() )
      {
        _report->finish(progress);
      }
      return value;
    }

    ProgressData::ReceiverFnc _fnc;
    callback::SendReport<ProgressReport> &_report;
    bool _first;
  };

  ////////////////////////////////////////////////////////////////////////////

  namespace repo
  {
    // progress for downloading a resolvable
    struct DownloadResolvableReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
        IGNORE, // ignore this resolvable but continue
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
        IO,		// IO error
        INVALID		// the downloaded file is invalid
      };

      /** Hint that package is available in the local cache (no download needed).
       * This will be the only trigger for an already cached package.
       */
      virtual void infoInCache( Resolvable::constPtr res_r, const Pathname & localfile_r )
      {}

      virtual void start(
        Resolvable::constPtr /*resolvable_ptr*/
        , const Url &/*url*/
      ) {}


      // Dowmload delta rpm:
      // - path below url reported on start()
      // - expected download size (0 if unknown)
      // - download is interruptable
      // - problems are just informal
      virtual void startDeltaDownload( const Pathname & /*filename*/, const ByteCount & /*downloadsize*/ )
      {}

      virtual bool progressDeltaDownload( int /*value*/ )
      { return true; }

      virtual void problemDeltaDownload( const std::string &/*description*/ )
      {}

      virtual void finishDeltaDownload()
      {}

      // Apply delta rpm:
      // - local path of downloaded delta
      // - aplpy is not interruptable
      // - problems are just informal
      virtual void startDeltaApply( const Pathname & /*filename*/ )
      {}

      virtual void progressDeltaApply( int /*value*/ )
      {}

      virtual void problemDeltaApply( const std::string &/*description*/ )
      {}

      virtual void finishDeltaApply()
      {}

      // return false if the download should be aborted right now
      virtual bool progress(int /*value*/, Resolvable::constPtr /*resolvable_ptr*/)
      { return true; }

      virtual Action problem(
        Resolvable::constPtr /*resolvable_ptr*/
	, Error /*error*/
	, const std::string &/*description*/
      ) { return ABORT; }

      virtual void finish(Resolvable::constPtr /*resolvable_ptr*/
        , Error /*error*/
        , const std::string &/*reason*/
      ) {}
    };

    // progress for probing a source
    struct ProbeRepoReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY	// retry
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
        IO,		// IO error
        INVALID,		// th source is invalid
        UNKNOWN
      };

      virtual void start(const Url &/*url*/) {}
      virtual void failedProbe( const Url &/*url*/, const std::string &/*type*/ ) {}
      virtual void successProbe( const Url &/*url*/, const std::string &/*type*/ ) {}
      virtual void finish(const Url &/*url*/, Error /*error*/, const std::string &/*reason*/ ) {}

      virtual bool progress(const Url &/*url*/, int /*value*/)
      { return true; }

      virtual Action problem( const Url &/*url*/, Error /*error*/, const std::string &/*description*/ ) { return ABORT; }
    };

    struct RepoCreateReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
        IGNORE  // skip refresh, ignore failed refresh
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
        IO,		// IO error
        REJECTED,
        INVALID, // th source is invali
        UNKNOWN
      };

      virtual void start( const zypp::Url &/*url*/ ) {}
      virtual bool progress( int /*value*/ )
      { return true; }

      virtual Action problem(
          const zypp::Url &/*url*/
          , Error /*error*/
          , const std::string &/*description*/ )
      { return ABORT; }

      virtual void finish(
          const zypp::Url &/*url*/
          , Error /*error*/
          , const std::string &/*reason*/ )
      {}
    };

    struct RepoReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
        IGNORE  // skip refresh, ignore failed refresh
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
        IO,		// IO error
        INVALID		// th source is invalid
      };

      virtual void start( const ProgressData &/*task*/, const RepoInfo /*repo*/  ) {}
      virtual bool progress( const ProgressData &/*task*/ )
      { return true; }

      virtual Action problem(
          Repository /*source*/
          , Error /*error*/
          , const std::string &/*description*/ )
      { return ABORT; }

      virtual void finish(
          Repository /*source*/
          , const std::string &/*task*/
          , Error /*error*/
          , const std::string &/*reason*/ )
      {}
    };


    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace media
  {
    // media change request callback
    struct MediaChangeReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
        IGNORE, // ignore this media in future, not available anymore
        IGNORE_ID,	// ignore wrong medium id
        CHANGE_URL,	// change media URL
        EJECT		// eject the medium
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND,  // the medie not found at all
        IO,	// error accessing the media
        INVALID, // media is broken
        WRONG,	// wrong media, need a different one
        IO_SOFT       /**< IO error which can happen on worse connection like timeout exceed */
      };

      /**
       *
       * \param url         in: url for which the media is requested,
       *                    out: url to use instead of the original one
       * \param mediumNr    requested medium number
       * \param label       label of requested medium
       * \param error       type of error from \ref Error enum
       * \param description error message (media not desired or error foo occured)
       * \param devices     list of the available devices (for eject)
       * \param dev_current in: index of the currently used device in the \a devices list
       *                    out: index of the devices to be ejected in the \a devices list
       * \return \ref Action (ABORT by default)
       */
      virtual Action requestMedia(
        Url & /* url (I/O parameter) */
        , unsigned /*mediumNr*/
        , const std::string & /* label */
        , Error /*error*/
        , const std::string & /*description*/
        , const std::vector<std::string> & /* devices */
        , unsigned int & /* dev_current (I/O param) */
      ) { return ABORT; }
    };

    ///////////////////////////////////////////////////////////////////
    /// \class ScopedDisableMediaChangeReport
    /// \brief Temporarily disable MediaChangeReport
    /// Sometimes helpful to suppress interactive messages connected to
    /// MediaChangeReport while fallback URLs are avaialble.
    struct ScopedDisableMediaChangeReport
    {
      /** Disbale MediaChangeReport if \a condition_r is \c true.*/
      ScopedDisableMediaChangeReport( bool condition_r = true );
    private:
      shared_ptr<callback::TempConnect<media::MediaChangeReport> > _guard;
    };

    // progress for downloading a file
    struct DownloadProgressReport : public callback::ReportBase
    {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
          IGNORE	// ignore the failure
        };

        enum Error {
          NO_ERROR,
          NOT_FOUND, 	// the requested Url was not found
          IO,		// IO error
          ACCESS_DENIED, // user authent. failed while accessing restricted file
          ERROR // other error
        };

        virtual void start( const Url &/*file*/, Pathname /*localfile*/ ) {}

        /**
         * Download progress.
         *
         * \param value        Percentage value.
         * \param file         File URI.
         * \param dbps_avg     Average download rate so far. -1 if unknown.
         * \param dbps_current Current download (cca last 1 sec). -1 if unknown.
         */
        virtual bool progress(int /*value*/, const Url &/*file*/,
                              double dbps_avg = -1,
                              double dbps_current = -1)
        { return true; }

        virtual Action problem(
          const Url &/*file*/
  	  , Error /*error*/
  	  , const std::string &/*description*/
        ) { return ABORT; }

        virtual void finish(
          const Url &/*file*/
          , Error /*error*/
	  , const std::string &/*reason*/
        ) {}
    };

    // authentication issues report
    struct AuthenticationReport : public callback::ReportBase
    {
      /**
       * Prompt for authentication data.
       *
       * \param url       URL which required the authentication
       * \param msg       prompt text
       * \param auth_data input/output object for handling authentication
       *        data. As an input parameter auth_data can be prefilled with
       *        username (from previous try) or auth_type (available
       *        authentication methods aquired from server (only CurlAuthData)).
       *        As an output parameter it serves for sending username, pasword
       *        or other special data (derived AuthData objects).
       * \return true if user chooses to continue with authentication,
       *         false otherwise
       */
      virtual bool prompt(const Url & /* url */,
        const std::string & /* msg */,
        AuthData & /* auth_data */)
      {
        return false;
      }
    };

    /////////////////////////////////////////////////////////////////
  } // namespace media
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace target
  {
    /** Request to display the pre commit message of a patch. */
    struct PatchMessageReport : public callback::ReportBase
    {
      /** Display \c patch->message().
       * Return \c true to continue, \c false to abort commit.
      */
      virtual bool show( Patch::constPtr & /*patch*/ )
      { return true; }
    };

    /** Indicate execution of a patch script. This is a sort of
     * \c %post script shipped by a package and to be executed
     * after the package was installed.
    */
    struct PatchScriptReport : public callback::ReportBase
    {
      enum Notify { OUTPUT, PING };
      enum Action {
        ABORT,  // abort commit and return error
        RETRY,	// (re)try to execute this script
        IGNORE	// ignore any failue and continue
      };

      /** Start executing the script provided by package.
      */
      virtual void start( const Package::constPtr & /*package*/,
                          const Pathname & /*script path*/ )
      {}
      /** Progress provides the script output. If the script is quiet,
       * from time to time still-alive pings are sent to the ui. Returning \c FALSE
       * aborts script execution.
      */
      virtual bool progress( Notify /*OUTPUT or PING*/,
                             const std::string & /*output*/ = std::string() )
      { return true; }
      /** Report error. */
      virtual Action problem( const std::string & /*description*/ )
      { return ABORT; }
      /** Report success. */
      virtual void finish()
      {}
    };

    ///////////////////////////////////////////////////////////////////
    /// \class FindFileConflictstReport
    /// \brief Check for package file conflicts in commit (after download)
    ///
    /// File conflict check requires that all packages are downloaded and
    /// now available in the cache (need to access the filelists). Missing
    /// packages are omitted from check and their number is reported in
    /// \a noFilelist_r. This usually happens if download mode 'as-needed'
    /// is used.
    ///////////////////////////////////////////////////////////////////
    struct FindFileConflictstReport : public callback::ReportBase
    {
      /**
       * \param progress_r	Progress counter for packages to check.
       * \return \c true to continue, \c false upon user abort request.
       */
      virtual bool start( const ProgressData & progress_r )
      { return true; }

      /**
       * \param progress_r	Progress counter for packages to check.
       * \param noFilelist_r	Queue of so far skipped solvables (no filelist/not yet downloaded).
       * \return \c true to continue, \c false upon user abort request.
       */
      virtual bool progress( const ProgressData & progress_r, const sat::Queue & noFilelist_r )
      { return true; }

      /**
       * \param progress_r	Progress counter for packages to check.
       * \param noFilelist_r	Queue of skipped solvables (no filelist/not yet downloaded).
       * \param conflicts_r	File conflits queue.
       * \return \c true to continue, \c false upon user abort request.
       */
      virtual bool result( const ProgressData & progress_r, const sat::Queue & noFilelist_r, const sat::FileConflicts & conflicts_r )
      { return true; }
    };


    ///////////////////////////////////////////////////////////////////
    namespace rpm
    {

      // progress for installing a resolvable
      struct InstallResolvableReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
          NOT_FOUND, 	// the requested Url was not found
	  IO,		// IO error
	  INVALID		// th resolvable is invalid
        };

        // the level of RPM pushing
        /** \deprecated We fortunately no longer do 3 attempts. */
        enum RpmLevel {
            RPM,
            RPM_NODEPS,
            RPM_NODEPS_FORCE	//!< only this one used
        };

        virtual void start(
	  Resolvable::constPtr /*resolvable*/
        ) {}

        virtual bool progress(int /*value*/, Resolvable::constPtr /*resolvable*/)
        { return true; }

        virtual Action problem(
          Resolvable::constPtr /*resolvable*/
  	  , Error /*error*/
   	  , const std::string &/*description*/
	  , RpmLevel /*level*/
        ) { return ABORT; }

        virtual void finish(
          Resolvable::constPtr /*resolvable*/
          , Error /*error*/
	  , const std::string &/*reason*/
	  , RpmLevel /*level*/
        ) {}
      };

      // progress for removing a resolvable
      struct RemoveResolvableReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
          NOT_FOUND, 	// the requested Url was not found
	  IO,		// IO error
	  INVALID		// th resolvable is invalid
        };

        virtual void start(
	  Resolvable::constPtr /*resolvable*/
        ) {}

        virtual bool progress(int /*value*/, Resolvable::constPtr /*resolvable*/)
        { return true; }

        virtual Action problem(
          Resolvable::constPtr /*resolvable*/
  	  , Error /*error*/
  	  , const std::string &/*description*/
        ) { return ABORT; }

        virtual void finish(
          Resolvable::constPtr /*resolvable*/
          , Error /*error*/
	  , const std::string &/*reason*/
        ) {}
      };

      // progress for rebuilding the database
      struct RebuildDBReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
	  FAILED		// failed to rebuild
        };

        virtual void start(Pathname /*path*/) {}

        virtual bool progress(int /*value*/, Pathname /*path*/)
        { return true; }

        virtual Action problem(
	  Pathname /*path*/
  	 , Error /*error*/
  	 , const std::string &/*description*/
        ) { return ABORT; }

        virtual void finish(
	  Pathname /*path*/
          , Error /*error*/
	  , const std::string &/*reason*/
        ) {}
      };

      // progress for converting the database
      struct ConvertDBReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
	  FAILED		// conversion failed
        };

        virtual void start(
	  Pathname /*path*/
        ) {}

        virtual bool progress(int /*value*/, Pathname /*path*/)
        { return true; }

        virtual Action problem(
	  Pathname /*path*/
  	  , Error /*error*/
  	 , const std::string &/*description*/
        ) { return ABORT; }

        virtual void finish(
	  Pathname /*path*/
          , Error /*error*/
          , const std::string &/*reason*/
        ) {}
      };

      /////////////////////////////////////////////////////////////////
    } // namespace rpm
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////

  class PoolQuery;

  /** \name Locks */
  //@{
  /**
   * Callback for cleaning locks which doesn't lock anything in pool.
   */

  struct CleanEmptyLocksReport : public callback::ReportBase
  {
    /**
     * action performed by cleaning api to specific lock
     */
    enum Action {
      ABORT,  /**< abort and return error */
      DELETE, /**< delete empty lock    */
      IGNORE  /**< skip empty lock */
    };

    /**
     * result of cleaning
     */
    enum Error {
      NO_ERROR, /**< no problem */
      ABORTED /** cleaning aborted by user */
    };

    /**
     * cleaning is started
     */
    virtual void start(
    ) {}

    /**
     * progress of cleaning specifies in percents
     * \return if continue
     */
    virtual bool progress(int /*value*/)
    { return true; }

    /**
     * When find empty lock ask what to do with it
     * \return action
     */
    virtual Action execute(
        const PoolQuery& /*error*/
     ) { return DELETE; }

      /**
       * cleaning is done
       */
     virtual void finish(
       Error /*error*/
      ) {}

  };

  /**
   * this callback handles merging old locks with newly added or removed
   */
  struct SavingLocksReport : public callback::ReportBase
  {
    /**
     * action for old lock which is in conflict
     * \see ConflictState
     */
    enum Action {
      ABORT,  /**< abort and return error*/
      DELETE, /**< delete conflicted lock    */
      IGNORE  /**< skip conflict lock */
    };

    /**
     * result of merging
     */
    enum Error {
      NO_ERROR, /**< no problem */
      ABORTED  /**< cleaning aborted by user */
    };

    /**
     * type of conflict of old and new lock
     */
    enum ConflictState{
      SAME_RESULTS, /**< locks lock same item in pool but his parameters is different */
      INTERSECT /**< locks lock some file and unlocking lock unlock only part
      * of iti, so removing old lock can unlock more items in pool */
    };

    virtual void start() {}

    /**
     * merging still live
     * \return if continue
     */
    virtual bool progress()
    { return true; }

    /**
     * When user unlock something which is locked by non-identical query
     */
    virtual Action conflict(
	 const PoolQuery&, /**< problematic query*/
       ConflictState
     ) { return DELETE; }

     virtual void finish(
       Error /*error*/
      ) {}
  };

  ///////////////////////////////////////////////////////////////////
  /// \class JobReport
  /// \brief Generic report for sending messages.
  ///////////////////////////////////////////////////////////////////
  struct JobReport : public callback::ReportBase
  {
  public:
    /** message type (use like 'enum class \ref MsgType') */
    struct _MsgTypeDef {
      enum Enum { debug, info, warning, error, important, data };
    };
    typedef base::EnumClass<_MsgTypeDef> MsgType;	///< 'enum class MsgType'

    /** typsafe map of userdata */
    typedef callback::UserData UserData;

  public:
    /** Send a ready to show message text. */
    virtual bool message( MsgType type_r, const std::string & msg_r, const UserData & userData_r ) const
    { return true; }


    /** \name Static sender instance */
    //@{
    /** Singleton sender instance */
    static callback::SendReport<JobReport> & instance();	// impl in ZYppImpl.cc

    /** send debug message text */
    static bool debug( const MessageString & msg_r, const UserData & userData_r = UserData() )
    { return instance()->message( MsgType::debug, msg_r, userData_r ); }

    /** send message text */
    static bool info( const MessageString & msg_r, const UserData & userData_r = UserData() )
    { return instance()->message( MsgType::info, msg_r, userData_r ); }

    /** send warning text */
    static bool warning( const MessageString & msg_r, const UserData & userData_r = UserData() )
    { return instance()->message( MsgType::warning, msg_r, userData_r ); }

    /** send error text */
    static bool error( const MessageString & msg_r, const UserData & userData_r = UserData() )
    { return instance()->message( MsgType::error, msg_r, userData_r ); }

    /** send important message text */
    static bool important( const MessageString & msg_r, const UserData & userData_r = UserData() )
    { return instance()->message( MsgType::important, msg_r, userData_r ); }

    /** send data message */
    static bool data( const MessageString & msg_r, const UserData & userData_r = UserData() )
    { return instance()->message( MsgType::data, msg_r, userData_r ); }
    //@}
  };


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_ZYPPCALLBACKS_H
