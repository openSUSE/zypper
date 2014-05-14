/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_SOURCE_CALLBACKS_H
#define ZMART_SOURCE_CALLBACKS_H

#include <sstream>
#include <boost/format.hpp>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/Url.h>

#include "Zypper.h"
#include "utils/prompt.h"
#include "utils/misc.h"

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{
///////////////////////////////////////////////////////////////////

// progress for downloading a resolvable
struct DownloadResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::repo::DownloadResolvableReport>
{
  zypp::Resolvable::constPtr _resolvable_ptr;
  zypp::Url _url;
  zypp::Pathname _delta;
  zypp::ByteCount _delta_size;
  std::string _label_apply_delta;
  zypp::Pathname _patch;
  zypp::ByteCount _patch_size;

  // Dowmload delta rpm:
  // - path below url reported on start()
  // - expected download size (0 if unknown)
  // - download is interruptable
  // - problems are just informal
  virtual void startDeltaDownload( const zypp::Pathname & filename, const zypp::ByteCount & downloadsize )
  {
    _delta = filename;
    _delta_size = downloadsize;
    std::ostringstream s;
    s << _("Retrieving delta") << ": "
        << _delta << ", " << _delta_size;
    Zypper::instance()->out().info(s.str());
  }

  virtual bool progressDeltaDownload( int value )
  {
    // seems this is never called, the progress is reported by the media backend anyway
    INT << "not impelmented" << std::endl;
    // TranslatorExplanation This text is a progress display label e.g. "Retrieving delta [42%]"
    //display_step( "apply-delta", ~("Retrieving delta") /*+ _delta.asString()*/, value );
    return true;
  }

  virtual void problemDeltaDownload( const std::string & description )
  {
    Zypper::instance()->out().error(description);
  }

  // implementation not needed prehaps - the media backend reports the download progress
  /*
  virtual void finishDeltaDownload()
  {
    display_done ("download-resolvable", cout_v);
  }
  */

  // Apply delta rpm:
  // - local path of downloaded delta
  // - aplpy is not interruptable
  // - problems are just informal
  virtual void startDeltaApply( const zypp::Pathname & filename )
  {
    _delta = filename.basename();
    std::ostringstream s;
    // translators: this text is a progress display label e.g. "Applying delta foo [42%]"
    s << _("Applying delta") << ": " << _delta;
    _label_apply_delta = s.str();
    Zypper::instance()->out().progressStart("apply-delta", _label_apply_delta, false);
  }

  virtual void progressDeltaApply( int value )
  {
    Zypper::instance()->out().progress("apply-delta", _label_apply_delta, value);
  }

  virtual void problemDeltaApply( const std::string & description )
  {
    Zypper::instance()->out().progressEnd("apply-delta", _label_apply_delta, true);
    Zypper::instance()->out().error(description);
  }

  virtual void finishDeltaApply()
  {
    Zypper::instance()->out().progressEnd("apply-delta", _label_apply_delta);
  }

  // Dowmload patch rpm:
  // - path below url reported on start()
  // - expected download size (0 if unknown)
  // - download is interruptable
  virtual void startPatchDownload( const zypp::Pathname & filename, const zypp::ByteCount & downloadsize )
  {
    _patch = filename.basename();
    _patch_size = downloadsize;
    std::ostringstream s;
    s << _("Retrieving patch rpm") << ": " << _patch << ", " << _patch_size;
    Zypper::instance()->out().info(s.str());
  }

  virtual bool progressPatchDownload( int value )
  {
    // seems this is never called, the progress is reported by the media backend anyway
    INT << "not impelmented" << std::endl;
    // TranslatorExplanation This text is a progress display label e.g. "Applying patch rpm [42%]"
    //display_step( "apply-delta", _("Applying patch rpm") /* + _patch.asString() */, value );
    return true;
  }

  virtual void problemPatchDownload( const std::string & description )
  {
    Zypper::instance()->out().error(description);
  }

  // implementation not needed prehaps - the media backend reports the download progress
  /*
  virtual void finishPatchDownload()
  {
    display_done ("apply-delta", cout_v);
  }
  */

  void fillsRhs( TermLine & outstr_r, Zypper & zypper_r, zypp::Package::constPtr pkg_r )
  {
    outstr_r.rhs << " (" << ++zypper_r.runtimeData().commit_pkg_current
		 << "/" << zypper_r.runtimeData().commit_pkgs_total << ")";
    if ( pkg_r )
    {
      outstr_r.rhs << ", " << pkg_r->downloadSize().asString( 5 ) << " "
		   // TranslatorExplanation %s is package size like "5.6 M"
		   << boost::format(_("(%s unpacked)")) % pkg_r->installSize().asString( 5 );
    }
  }

  virtual void infoInCache( Resolvable::constPtr res_r, const Pathname & localfile_r )
  {
    Zypper & zypper = *Zypper::instance();

    TermLine outstr( TermLine::SF_SPLIT | TermLine::SF_EXPAND );
    outstr.lhs << boost::format(_("In cache %1%")) % localfile_r.basename();
    fillsRhs( outstr, zypper, zypp::asKind<zypp::Package>(res_r) );
    zypper.out().infoLine( outstr );
  }

  /** this is interesting because we have full resolvable data at hand here
   * The media backend has only the file URI
   * \todo combine this and the media data progress callbacks in a reasonable manner
   */
  virtual void start( zypp::Resolvable::constPtr resolvable_ptr, const zypp::Url & url )
  {
    _resolvable_ptr =  resolvable_ptr;
    _url = url;
    Zypper & zypper = *Zypper::instance();

    TermLine outstr( TermLine::SF_SPLIT | TermLine::SF_EXPAND );
    outstr.lhs << boost::format(_("Retrieving %s %s-%s.%s"))
        % kind_to_string_localized(_resolvable_ptr->kind(), 1)
        % _resolvable_ptr->name()
        % _resolvable_ptr->edition() % _resolvable_ptr->arch();
    fillsRhs( outstr, zypper, zypp::asKind<zypp::Package>(resolvable_ptr) );

    // temporary fix for bnc #545295
    if ( zypper.runtimeData().commit_pkg_current == zypper.runtimeData().commit_pkgs_total )
      zypper.runtimeData().commit_pkg_current = 0;

    zypper.out().infoLine( outstr );
    zypper.runtimeData().action_rpm_download = true;
  }

  // return false if the download should be aborted right now
  virtual bool progress(int value, zypp::Resolvable::constPtr /*resolvable_ptr*/)
  {
    // seems this is never called, the progress is reported by the media backend anyway
    // INT << "not impelmented" << std::endl;
    // TranslatorExplanation This text is a progress display label e.g. "Retrieving [42%]"
//    display_step( "download-resolvable", ~("Retrieving") /* + resolvable_ptr->name() */, value );
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable_ptr, Error /*error*/, const std::string & description )
  {
    Zypper::instance()->out().error(description);
    DBG << "error report" << std::endl;

    Action action = (Action) read_action_ari(PROMPT_ARI_RPM_DOWNLOAD_PROBLEM, ABORT);
    if (action == DownloadResolvableReport::RETRY)
      --Zypper::instance()->runtimeData().commit_pkg_current;
    else
      Zypper::instance()->runtimeData().action_rpm_download = false;
    return action;
  }

  // implementation not needed prehaps - the media backend reports the download progress
  virtual void finish( zypp::Resolvable::constPtr /*resolvable_ptr**/, Error error, const std::string & reason )
  {
    Zypper::instance()->runtimeData().action_rpm_download = false;
/*
    display_done ("download-resolvable", cout_v);
    display_error (error, reason);
*/
  }
};

struct ProgressReportReceiver  : public zypp::callback::ReceiveReport<zypp::ProgressReport>
{
  virtual void start( const zypp::ProgressData &data )
  {
    Zypper::instance()->out().progressStart(
        zypp::str::numstring(data.numericId()),
        data.name(),
        data.reportAlive());
  }

  virtual bool progress( const zypp::ProgressData &data )
  {
    if (data.reportAlive())
      Zypper::instance()->out().progress(
          zypp::str::numstring(data.numericId()),
          data.name());
    else
      Zypper::instance()->out().progress(
          zypp::str::numstring(data.numericId()),
          data.name(), data.val());
    return true;
  }

//   virtual Action problem( zypp::Repository /*repo*/, Error error, const std::string & description )
//   {
//     display_done ();
//     display_error (error, description);
//     return (Action) read_action_ari ();
//   }

  virtual void finish( const zypp::ProgressData &data )
  {
    Zypper::instance()->out().progressEnd(
        zypp::str::numstring(data.numericId()),
        data.name());
  }
};


struct RepoReportReceiver  : public zypp::callback::ReceiveReport<zypp::repo::RepoReport>
{
  virtual void start(const zypp::ProgressData & pd, const zypp::RepoInfo repo)
  {
    _repo = repo;
    Zypper::instance()->out()
      .progressStart("repo", "(" + _repo.name() + ") " + pd.name());
  }

  virtual bool progress(const zypp::ProgressData & pd)
  {
    Zypper::instance()->out()
      .progress("repo", "(" + _repo.name() + ") " + pd.name(), pd.val());
    return true;
  }

  virtual Action problem( zypp::Repository /*repo*/, Error error, const std::string & description )
  {
    Zypper::instance()->out()
      .progressEnd("repo", "(" + _repo.name() + ") ");
    Zypper::instance()->out().error(zcb_error2str(error, description));
    return (Action) read_action_ari (PROMPT_ARI_REPO_PROBLEM, ABORT);
  }

  virtual void finish( zypp::Repository /*repo*/, const std::string & task, Error error, const std::string & reason )
  {
    Zypper::instance()->out()
      .progressEnd("repo", "(" + _repo.name() + ") ");
    if (error != NO_ERROR)
      Zypper::instance()->out().error(zcb_error2str(error, reason));
//    display_step(100);
    // many of these, avoid newline -- probably obsolete??
    //if (task.find("Reading patch") == 0)
      //cout_n << '\r' << flush;
//    else
//      display_done ("repo", cout_n);
  }

  zypp::RepoInfo _repo;
};
    ///////////////////////////////////////////////////////////////////
}; // namespace ZmartRecipients
///////////////////////////////////////////////////////////////////

class SourceCallbacks {

  private:
    ZmartRecipients::RepoReportReceiver _repoReport;
    ZmartRecipients::DownloadResolvableReportReceiver _downloadReport;
    ZmartRecipients::ProgressReportReceiver _progressReport;
  public:
    SourceCallbacks()
    {
      _repoReport.connect();
      _downloadReport.connect();
      _progressReport.connect();
    }

    ~SourceCallbacks()
    {
      _repoReport.disconnect();
      _downloadReport.disconnect();
      _progressReport.disconnect();
    }

};

#endif
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
