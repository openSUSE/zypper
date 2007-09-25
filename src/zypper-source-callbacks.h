/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_SOURCE_CALLBACKS_H
#define ZMART_SOURCE_CALLBACKS_H

#include <stdlib.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>
#include <zypp/Url.h>

#include "zypper.h"
#include "zypper-callbacks.h"
#include "zypper-utils.h"

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{
///////////////////////////////////////////////////////////////////    
/*    // progress for probing a source
    struct ProbeSourceReceive : public zypp::callback::ReceiveReport<zypp::source::ProbeRepoReport>
    {
      virtual void start(const zypp::Url &url)
      {
        cout << "Determining " << url << " source type..." << endl;
      }
      
      virtual void failedProbe( const zypp::Url & url*//*, const std::string & type )
      {
        cout << ".. not " << type << endl;
      }
      
      virtual void successProbe( const zypp::Url &url, const std::string & type )
      {
        cout << url << " is type " << type << endl;
      }
      
      virtual void finish(const zypp::Url & url*//*, Error error, const std::string & reason )
      {
        if ( error == INVALID )
        {
          cout << reason << endl;
          exit(-1);
        }
      }

      virtual bool progress(const zypp::Url & url*//*, int value*//*)
      { return true; }

      virtual Action problem( const zypp::Url & url*//*, Error error, const std::string & description )
      {
	display_done ();
	display_error (error, description);
        exit(-1);
        return ABORT;
      }
    };
    */
// progress for downloading a resolvable
struct DownloadResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::repo::DownloadResolvableReport>
{
  zypp::Resolvable::constPtr _resolvable_ptr;
  zypp::Url _url;
  zypp::Pathname _delta;
  zypp::ByteCount _delta_size;
  zypp::Pathname _patch;
  zypp::ByteCount _patch_size;
  
  void display_step( const std::string &id, const std::string &what, int value )
  {
    display_progress ("download-resolvable", cout_v, what, value);
  }
  
  // Dowmload delta rpm:
  // - path below url reported on start()
  // - expected download size (0 if unknown)
  // - download is interruptable
  // - problems are just informal
  virtual void startDeltaDownload( const zypp::Pathname & filename, const zypp::ByteCount & downloadsize )
  {
      _delta = filename;
      _delta_size = downloadsize;
      cout_n << _("Downloading delta") << ": "
          << _delta << ", " << _delta_size << std::endl;
  }

  virtual bool progressDeltaDownload( int value )
  {
    // TranslatorExplanation This text is a progress display label e.g. "Downloading delta [42%]"
    display_step( "apply-delta", _("Downloading delta") /*+ _delta.asString()*/, value );
    return true;
  }

  virtual void problemDeltaDownload( const std::string & description )
  {
    std::cerr << description << std::endl;
  }
  
  virtual void finishDeltaDownload()
  {
    display_done ("download-resolvable", cout_v);
  }

  // Apply delta rpm:
  // - local path of downloaded delta
  // - aplpy is not interruptable
  // - problems are just informal
  virtual void startDeltaApply( const zypp::Pathname & filename )
  {
    _delta = filename;
    cout_n << _("Applying delta") << ": " << _delta << std::endl;
  }

  virtual void progressDeltaApply( int value )
  {
    // TranslatorExplanation This text is a progress display label e.g. "Applying delta [42%]"
    display_step( "apply-delta", _("Applying delta") /* + _delta.asString()*/, value );
  }

  virtual void problemDeltaApply( const std::string & description )
  {
    std::cerr << description << std::endl;
  }

  virtual void finishDeltaApply()
  {
    display_done ("apply-delta", cout_v);
  }

  // Dowmload patch rpm:
  // - path below url reported on start()
  // - expected download size (0 if unknown)
  // - download is interruptable
  virtual void startPatchDownload( const zypp::Pathname & filename, const zypp::ByteCount & downloadsize )
  {
    _patch = filename;
    _patch_size = downloadsize;
    cout_n << _("Downloading patch rpm") << ": "
	      << _patch << ", " << _patch_size << std::endl;
  }

  virtual bool progressPatchDownload( int value )
  {
    // TranslatorExplanation This text is a progress display label e.g. "Applying patch rpm [42%]"
    display_step( "apply-delta", _("Applying patch rpm") /* + _patch.asString() */, value );
    return true;
  }
  
  virtual void problemPatchDownload( const std::string & description )
  {
    std::cerr << description << std::endl;
  }
  
  virtual void finishPatchDownload()
  {
    display_done ("apply-delta", cout_v);
  }
  
  
  virtual void start( zypp::Resolvable::constPtr resolvable_ptr, const zypp::Url & url )
  {
    _resolvable_ptr =  resolvable_ptr;
    _url = url;

    cout_n << boost::format(_("Downloading %s %s-%s.%s"))
        % kind_to_string_localized(_resolvable_ptr->kind(), 1)
        % _resolvable_ptr->name()
        % _resolvable_ptr->edition() % _resolvable_ptr->arch();

// grr, bad class??
//    zypp::ResObject::constPtr ro =
//      dynamic_pointer_cast<const zypp::ResObject::constPtr> (resolvable_ptr);
    zypp::Package::constPtr ro = zypp::asKind<zypp::Package> (resolvable_ptr);
    if (ro) {
      cout_n << ", " << ro->downloadSize () << " "
          // TranslatorExplanation %s is package size like "5.6 M"
          << boost::format(_("(%s unpacked)")) % ro->size();
    }
    cout_n << std::endl;
  }

  // return false if the download should be aborted right now
  virtual bool progress(int value, zypp::Resolvable::constPtr /*resolvable_ptr*/)
  {
    // TranslatorExplanation This text is a progress display label e.g. "Downloading [42%]"
    display_step( "download-resolvable", _("Downloading") /* + resolvable_ptr->name() */, value );
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable_ptr, Error /*error*/, const std::string & description )
  {
    std::cerr << description << std::endl;
    return (Action) read_action_ari(ABORT);
  }

  virtual void finish( zypp::Resolvable::constPtr /*resolvable_ptr*/, Error error, const std::string & reason )
  {
    display_done ("download-resolvable", cout_v);
    display_error (error, reason);
  }
};

struct ProgressReportReceiver  : public zypp::callback::ReceiveReport<zypp::ProgressReport>
{
  void tick( const zypp::ProgressData &data )
  {
    if ( data.reportAlive() )
      display_tick (zypp::str::numstring(data.numericId()), cout, data.name() );
    else
      display_progress ( zypp::str::numstring(data.numericId()), cout, data.name() , data.val() );
  }
  
  virtual void start( const zypp::ProgressData &data )
  {
    tick(data);
  }
  
  virtual bool progress( const zypp::ProgressData &data )
  {
    tick(data);
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
    display_done(zypp::str::numstring(data.numericId()), cout, data.name() );
  }
};


struct RepoReportReceiver  : public zypp::callback::ReceiveReport<zypp::repo::RepoReport>
{     
  virtual void start( zypp::Repository repo, const std::string & task )
  {
    _task = task;
    _repo = repo;
    
    display_step(0);
  }

  void display_step( int value )
  {
    display_progress ( "repo", cout, "(" + _repo.info().name() + ") " + _task , value);
  }

  virtual bool progress( int value )
  {
    display_step(value);
    return true;
  }
  
  virtual Action problem( zypp::Repository /*repo*/, Error error, const std::string & description )
  {
    display_done ("repo", cout);
    display_error (error, description);
    return (Action) read_action_ari (ABORT);
  }

  virtual void finish( zypp::Repository /*repo*/, const std::string & task, Error error, const std::string & reason )
  {
    display_step(100);
    // many of these, avoid newline
    if (boost::algorithm::starts_with (task, "Reading patch"))
      cout << '\r' << flush;
    else
      display_done ("repo", cout);
    display_error (error, reason);
  }

  std::string _task;
  zypp::Repository _repo;
};
    ///////////////////////////////////////////////////////////////////
}; // namespace ZmartRecipients
///////////////////////////////////////////////////////////////////

class SourceCallbacks {

  private:
//    ZmartRecipients::ProbeSourceReceive _sourceProbeReport;
    ZmartRecipients::RepoReportReceiver _repoReport;
    ZmartRecipients::DownloadResolvableReportReceiver _downloadReport;
    ZmartRecipients::ProgressReportReceiver _progressReport;
  public:
    SourceCallbacks()
    {
//      _sourceProbeReport.connect();
      _repoReport.connect();
      _downloadReport.connect();
      _progressReport.connect();
    }

    ~SourceCallbacks()
    {
//      _sourceProbeReport.disconnect();
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
