#include <zypp-proto/commit.pb.h>
#include <zypp-core/zyppng/core/ByteArray>
#include <zypp-core/zyppng/rpc/rpc.h>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/AutoDispose.h>
#include <zypp-core/Pathname.h>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/base/String.h>
#include <zypp-core/base/StringV.h>

// we do not link against libzypp, but these are pure header only files, if that changes
// a copy should be created directly in the zypp-rpm project
#include <zypp/target/rpm/librpm.h>
#include <zypp/target/rpm/RpmFlags.h>

#include <zypp-core/zyppng/rpc/zerocopystreams.h>

extern "C"
{
#include <rpm/rpmcli.h>
#include <rpm/rpmlog.h>
}

#include "BinHeader.h"
#include "errorcodes.h"

#include <cstdio>
#include <iostream>
#include <signal.h>
#include <unistd.h>


// this is the order we expect the FDs we need to communicate to be set up
// by the parent. This is not pretty but it works and is less effort than
// setting up a Unix Domain Socket and sending FDs over that.
// Usually relying on conventions is not exactly a good idea but in this case we make an exception ;)
enum class ExpectedFds : int {
  MessageFd = STDERR_FILENO+1,
  ScriptFd  = STDERR_FILENO+2
};

using zypp::target::rpm::RpmInstFlag;
using zypp::target::rpm::RpmInstFlags;
using namespace zypprpm;

void *rpmLibCallback( const void *h, const rpmCallbackType what, const rpm_loff_t amount, const rpm_loff_t total, fnpyKey key, rpmCallbackData data );
int rpmLogCallback ( rpmlogRec rec, rpmlogCallbackData data );

template <typename Stream>
void rpmpsPrintToStream ( Stream &str, rpmps ps )
{
  if ( !ps )
    return;

  rpmProblem p;
  zypp::AutoDispose<rpmpsi> psi ( ::rpmpsInitIterator(ps), ::rpmpsFreeIterator );
  while ((p = rpmpsiNext(psi))) {
    zypp::AutoFREE<char> msg( rpmProblemString(p) );
    str << "\t" << msg << std::endl;
  }
}

bool recvBytes ( int fd, char *buf, size_t n ) {
  size_t read = 0;
  while ( read != n ) {
    const auto r = zyppng::eintrSafeCall( ::read, fd, buf+read, n - read );
    if ( r <= 0 )
      return false;

    read += r;
  }
  return true;
}

bool sendBytes ( int fd, const void *buf, size_t n ) {
  const auto written =  zyppng::eintrSafeCall( ::write, fd, buf, n );
  return written == n;
}

template <typename Message>
bool pushMessage ( const Message &msg ) {

  zypp::proto::Envelope env;
  env.set_messagetypename( msg.GetTypeName() );
  msg.SerializeToString( env.mutable_value() );

  const auto &str = env.SerializeAsString();
  zyppng::rpc::HeaderSizeType msgSize = str.length();
  if ( !sendBytes( static_cast<int>( ExpectedFds::MessageFd ), reinterpret_cast< void* >( &msgSize), sizeof (zyppng::rpc::HeaderSizeType) ) )
    return false;
  return sendBytes( static_cast<int>( ExpectedFds::MessageFd ), str.c_str(), str.length() );
}

bool pushTransactionErrorMessage ( rpmps ps )
{
  if ( !ps )
    return false;

  zypp::proto::target::TransactionError err;

  rpmProblem p;
  zypp::AutoDispose<rpmpsi> psi ( ::rpmpsInitIterator(ps), ::rpmpsFreeIterator );
  while ((p = rpmpsiNext(psi))) {
    zypp::AutoFREE<char> msg( rpmProblemString(p) );

    zypp::proto::target::TransactionProblemDesc desc;
    desc.set_message( zypp::str::asString( msg.value() ) );

    *err.mutable_problems()->Add() = std::move(desc);
  }

  return pushMessage( err );
}


using RpmHeader = std::shared_ptr<std::remove_pointer_t<Header>>;
std::pair<RpmHeader, int> readPackage( rpmts ts_r, const zypp::filesystem::Pathname &path_r )
{
  zypp::PathInfo file( path_r );
  if ( ! file.isFile() ) {
    std::cerr << "Not a file: " << path_r << std::endl;
    return std::make_pair( RpmHeader(), -1 );
  }

  FD_t fd = ::Fopen( path_r.c_str(), "r.ufdio" );
  if ( fd == 0  || ::Ferror(fd) )
  {
    std::cerr << "Can't open file for reading: " << path_r << " (" << ::Fstrerror(fd) << ")" << std::endl;
    if ( fd )
      ::Fclose( fd );
    return std::make_pair( RpmHeader(), -1 );
  }

  Header nh = 0;
  int res = ::rpmReadPackageFile( ts_r, fd, path_r.asString().c_str(), &nh );
  ::Fclose( fd );

  if ( ! nh )
  {
    std::cerr << "Error reading header from " << path_r << " error(" << res << ")" << std::endl;
    return std::make_pair( RpmHeader(), res );
  }

  RpmHeader h( nh, ::headerFree );
  return std::make_pair( h, res );
}


struct TransactionData {
  zypp::proto::target::Commit &commitData;

  // dbinstance of removals to transaction step index
  std::unordered_map<int, int> removePckIndex = {};

  // the fd used by rpm to dump script output
  zypp::AutoDispose<FD_t> rpmFd = {};
};

int main( int, char ** )
{

  // check if our env is set up correctly
  if ( ::isatty(STDIN_FILENO) || ::isatty(STDOUT_FILENO) || ::isatty(STDERR_FILENO) ) {
    std::cerr << "Running zypp-rpm directly from the console is not supported. This is just a internal helper tool for libzypp." << std::endl;
    return OtherError;
  }

  // make sure the expected FDs are around too
  struct stat sb;
  if ( fstat( static_cast<int>(ExpectedFds::MessageFd), &sb) == -1 ) {
    std::cerr << "Expected message fd is not valid, aborting" << std::endl;
    return OtherError;
  }
  if ( (sb.st_mode & S_IFMT) != S_IFIFO ){
    std::cerr << "Expected message fd is not a pipe, aborting" << std::endl;
    return OtherError;
  }

  if ( fstat( static_cast<int>(ExpectedFds::ScriptFd), &sb) == -1 ) {
    std::cerr << "Expected script fd is not valid, aborting" << std::endl;
    return OtherError;
  }
  if ( (sb.st_mode & S_IFMT) != S_IFIFO ){
    std::cerr << "Expected script fd is not a pipe, aborting" << std::endl;
    return OtherError;
  }

  // lets ignore those, SIGPIPE is better handled via the EPIPE error, and we do not want anyone
  // to CTRL+C us
  zyppng::blockSignalsForCurrentThread( { SIGPIPE, SIGINT } );

  // lets read our todo from stdin
  zyppng::rpc::HeaderSizeType msgSize = 0;
  if ( !recvBytes( STDIN_FILENO, reinterpret_cast<char *>(&msgSize), sizeof(zyppng::rpc::HeaderSizeType) ) ) {
    std::cerr << "Wrong Header size, aborting" << std::endl;
    return WrongHeaderSize;
  }

  // since all we can receive on stdin is the commit message, there is no need to read a envelope first
  // we read it directly from the FD
  zypp::proto::target::Commit msg;
  {
    zyppng::FileInputStream in( STDIN_FILENO );
    if ( !msg.ParseFromBoundedZeroCopyStream( &in, msgSize ) ) {
      std::cerr << "Wrong commit message format, aborting" << std::endl;
      return WrongMessageFormat;
    }

  }

  // we have all data ready now lets start installing
  // first we initialize the rpmdb
  int rc = ::rpmReadConfigFiles( NULL, NULL );
  if ( rc ) {
    std::cerr << "rpmReadConfigFiles returned " << rc << std::endl;
    return RpmInitFailed;
  }

  ::addMacro( NULL, "_dbpath", NULL, msg.dbpath().c_str(), RMIL_CMDLINE );

  auto ts = zypp::AutoDispose<rpmts>( ::rpmtsCreate(), ::rpmtsFree );;
  ::rpmtsSetRootDir( ts, msg.root().c_str() );

  int tsFlags           = RPMTRANS_FLAG_NONE;
  int tsVerifyFlags     = RPMVSF_DEFAULT;

  if ( msg.flags() & RpmInstFlag::RPMINST_NODIGEST)
    tsVerifyFlags |= _RPMVSF_NODIGESTS;
  if ( msg.flags()  & RpmInstFlag::RPMINST_NOSIGNATURE)
    tsVerifyFlags |= _RPMVSF_NOSIGNATURES;
  if ( msg.flags()  & RpmInstFlag::RPMINST_EXCLUDEDOCS)
    tsFlags |= RPMTRANS_FLAG_NODOCS;
  if ( msg.flags()  & RpmInstFlag::RPMINST_NOSCRIPTS)
    tsFlags |= RPMTRANS_FLAG_NOSCRIPTS;
  if ( msg.flags()  & RpmInstFlag::RPMINST_JUSTDB)
    tsFlags |= RPMTRANS_FLAG_JUSTDB;
  if ( msg.flags()  & RpmInstFlag::RPMINST_TEST)
    tsFlags |= RPMTRANS_FLAG_TEST;
  if ( msg.flags()  & RpmInstFlag::RPMINST_NOPOSTTRANS)
    tsFlags |= RPMTRANS_FLAG_NOPOSTTRANS;
  if ( msg.flags() & RpmInstFlag::RPMINST_NOSCRIPTS )
    tsFlags |= RPMTRANS_FLAG_NOSCRIPTS;

  // setup transaction settings
  ::rpmtsSetFlags( ts, tsFlags );

  // set the verify flags so readPackage does the right thing
  ::rpmtsSetVSFlags( ts, tsVerifyFlags );

#ifdef HAVE_RPMTSSETVFYLEVEL
  {
    int vfylevel = ::rpmtsVfyLevel(ts);
    if ( msg.flags() & RpmInstFlag::RPMINST_NODIGEST)
      vfylevel &= ~( RPMSIG_DIGEST_TYPE );
    if ( msg.flags()  & RpmInstFlag::RPMINST_NOSIGNATURE)
      vfylevel &= ~( RPMSIG_SIGNATURE_TYPE);
    ::rpmtsSetVfyLevel(ts, vfylevel);
  }
#endif

  // open database for reading
  if ( rpmtsGetRdb(ts) == NULL ) {
    int res = ::rpmtsOpenDB( ts, O_RDWR );
    if ( res ) {
      std::cerr << "rpmdbOpen error(" << res << "): " << std::endl;
      return FailedToOpenDb;
    }
  }

  // the transaction data we will get in the callback
  TransactionData data { msg };


  for ( int i = 0; i < msg.steps_size(); i++ ) {
    const auto &step = msg.steps(i);

    if ( step.has_install() ) {

      const auto &file = step.install().pathname();
      auto rpmHeader = readPackage( ts, step.install().pathname() );
      switch(rpmHeader.second) {
        case RPMRC_OK:
          break;
        case RPMRC_NOTTRUSTED:
          std::cerr << zypp::str::Format( "Failed to verify key for %s" ) % file << std::endl;
          return FailedToReadPackage;
        case RPMRC_NOKEY:
          std::cerr << zypp::str::Format( "Public key unavailable for %s" ) % file << std::endl;
          return FailedToReadPackage;
        case RPMRC_NOTFOUND:
          std::cerr << zypp::str::Format( "Signature not found for %s" ) % file << std::endl;
          return FailedToReadPackage;
        case RPMRC_FAIL:
          std::cerr << zypp::str::Format( "Signature does not verify for %s" ) % file << std::endl;
          return FailedToReadPackage;
        default:
          std::cerr << zypp::str::Format( "Failed to open(generic error): %1%" ) % file << std::endl;
          return FailedToReadPackage;
      }

      if ( !rpmHeader.first ) {
        std::cerr << zypp::str::Format( "Failed to read rpm header from: %1%" )% file << std::endl;
        return FailedToReadPackage;
      }

      const auto res = ::rpmtsAddInstallElement( ts, rpmHeader.first.get(), &step, !step.install().multiversion(), nullptr  );
      if ( res ) {
        std::cerr << zypp::str::Format( "Failed to add %1% to the transaction." )% file << std::endl;
        return FailedToAddStepToTransaction;
      }

    } else if ( step.has_remove() ) {

      const auto &remove = step.remove();

      const std::string &name = remove.name()
                                + "-" + remove.version()
                                + "-" + remove.release()
                                + "." + remove.arch();

      bool found = false;
      zypp::AutoDispose<rpmdbMatchIterator> it( ::rpmtsInitIterator( ts, rpmTag(RPMTAG_NAME), remove.name().c_str(), 0 ), ::rpmdbFreeIterator );
      while ( ::Header h = ::rpmdbNextIterator( it ) ) {
        BinHeader hdr(h);
        if ( hdr.string_val( RPMTAG_VERSION ) == remove.version()
             &&  hdr.string_val( RPMTAG_RELEASE ) == remove.release()
             &&  hdr.string_val( RPMTAG_ARCH ) == remove.arch() ) {
          found = true;

          const auto res = ::rpmtsAddEraseElement( ts, hdr.get(), 0  );
          if ( res ) {
            std::cerr << zypp::str::Format( "Failed to add removal of %1% to the transaction." ) % name << std::endl;
            return FailedToAddStepToTransaction;
          }

          data.removePckIndex.insert( std::make_pair( headerGetInstance( h ), i ) );
          break;
        }
      }

      if ( !found ) {
        std::cerr << "Unable to remove " << name << " it was not found!" << std::endl;
      }

    } else {
      std::cerr << "Ignoring step that is neither a remove, nor a install." << std::endl;
    }

  }

  // set the callback function for progress reporting and things
  ::rpmtsSetNotifyCallback( ts, rpmLibCallback, &data );

  // make sure we get da log
  ::rpmlogSetMask( RPMLOG_UPTO( RPMLOG_PRI(RPMLOG_INFO) ) );
  ::rpmlogSetCallback( rpmLogCallback, nullptr );

  // redirect the script output to a fd ( log level MUST be at least INFO )
  data.rpmFd = zypp::AutoDispose<FD_t> (
    ::fdDup( static_cast<int>( ExpectedFds::ScriptFd ) ),
    Fclose
  );

  if ( data.rpmFd.value() ) {
    std::cerr << "Assigning script FD" << std::endl;
    ::rpmtsSetScriptFd( ts,  data.rpmFd );
  } else {
    std::cerr << "Failed to assign script FD" << std::endl;
  }


#if 0
  // unset the verify flags, we already checked those when reading the package header and libzypp made sure
  // all signatures are fine as well
  ::rpmtsSetVSFlags( ts, rpmtsVSFlags(ts) | _RPMVSF_NODIGESTS | _RPMVSF_NOSIGNATURES );

#ifdef HAVE_RPMTSSETVFYLEVEL
  {
    int vfylevel = ::rpmtsVfyLevel(ts);
    vfylevel &= ~( RPMSIG_DIGEST_TYPE | RPMSIG_SIGNATURE_TYPE);
    ::rpmtsSetVfyLevel(ts, vfylevel);
  }
#endif

#endif

  // handle --nodeps
  if ( !( msg.flags() & RpmInstFlag::RPMINST_NODEPS) ) {
    if ( ::rpmtsCheck(ts) ) {
      zypp::AutoDispose<rpmps> ps( ::rpmtsProblems(ts), ::rpmpsFree );
      pushTransactionErrorMessage( ps );

      std::ostringstream sstr;
      sstr << "rpm output:" << "Failed dependencies:" << std::endl;
      rpmpsPrintToStream( sstr, ps );

      const auto &rpmMsg = sstr.str();

      // TranslatorExplanation the colon is followed by an error message
      std::cerr << std::string("RPM failed: ") + rpmMsg << std::endl;
      return RpmFinishedWithTransactionError;
    }
  }

  int tsProbFilterFlags = RPMPROB_FILTER_NONE;


  if ( msg.ignorearch() )
    tsProbFilterFlags |= RPMPROB_FILTER_IGNOREARCH;

  if ( msg.flags() & RpmInstFlag::RPMINST_ALLOWDOWNGRADE )
    tsProbFilterFlags |= ( RPMPROB_FILTER_OLDPACKAGE ); // --oldpackage

  if ( msg.flags() & RpmInstFlag::RPMINST_REPLACEFILES )
    tsProbFilterFlags |= ( RPMPROB_FILTER_REPLACENEWFILES
                           | RPMPROB_FILTER_REPLACEOLDFILES ); // --replacefiles

  if ( msg.flags() & RpmInstFlag::RPMINST_FORCE )
    tsProbFilterFlags |= ( RPMPROB_FILTER_REPLACEPKG
                           | RPMPROB_FILTER_REPLACENEWFILES
                           | RPMPROB_FILTER_REPLACEOLDFILES
                           | RPMPROB_FILTER_OLDPACKAGE ); // --force

  if ( msg.flags() & RpmInstFlag::RPMINST_IGNORESIZE )
    tsProbFilterFlags |= RPMPROB_FILTER_DISKSPACE | RPMPROB_FILTER_DISKNODES;

  const auto orderRes = rpmtsOrder( ts );
  if ( orderRes ) {
    std::cerr << zypp::str::Format( "Failed with error %1% while ordering transaction." )% orderRes << std::endl;
    return RpmOrderFailed;
  }

  // clean up memory that is only used for dependency checks and ordering
  rpmtsClean(ts);

  // transaction steps are set up lets execute it
  // the way how libRPM works is that it will try to install all packages even if some of them fail
  // we need to go over the rpm problem set to mark those steps that have failed, we get no other hint on wether
  // it worked or not
  const auto transRes = ::rpmtsRun( ts, nullptr, tsProbFilterFlags );
  //data.finishCurrentStep( );

  if ( transRes != 0 ) {

    auto err = RpmFinishedWithError;

    std::string errMsg;
    if ( transRes > 0 ) {
      //@NOTE dnf checks if the problem set is empty and if it is seems to treat the transaction as successful, can this really happen?
      zypp::AutoDispose<rpmps> ps( ::rpmtsProblems(ts), ::rpmpsFree );

      pushTransactionErrorMessage( ps );

      std::ostringstream sstr;
      sstr << "rpm output:" << std::endl;
      rpmpsPrintToStream( sstr, ps );
      errMsg = sstr.str();
      err = RpmFinishedWithTransactionError;
    } else {
      errMsg = "Running the transaction failed.";
    }

    //HistoryLog().comment( str::form("Transaction failed"), true /*timestamp*/ );
    std::ostringstream sstr;
    sstr << "rpm output:" << std::endl << errMsg << std::endl;
    //HistoryLog().comment(sstr.str());

    std::cerr << "RPM transaction failed: " + errMsg << std::endl;
    return err;
  }

  std::cerr << "Success !!!!" << std::endl;

  return NoError;
}

std::string_view tagToScriptTypeName ( int tag )
{
  using namespace std::literals;
  switch (tag) {
    case RPMTAG_PREIN:
      return "prein"sv;
    case RPMTAG_PREUN:
      return "preun"sv;
    case RPMTAG_TRIGGERPREIN:
      return "triggerprein"sv;
    case RPMTAG_POSTIN:
      return "postin"sv;
    case RPMTAG_POSTUN:
      return "postun"sv;
    case RPMTAG_PRETRANS:
      return "pretrans"sv;
    case RPMTAG_POSTTRANS:
      return "posttrans"sv;
    case RPMTAG_TRIGGERUN:
      return "triggerun"sv;
    case RPMTAG_TRIGGERIN:
      return "triggerin"sv;
    case RPMTAG_TRIGGERPOSTUN:
      return "triggerpostun"sv;
    case RPMTAG_VERIFYSCRIPT:
      return "verifyscript"sv;
    default:
      return ""sv;
  }
}

void *rpmLibCallback( const void *h, const rpmCallbackType what, const rpm_loff_t amount, const rpm_loff_t total, fnpyKey key, rpmCallbackData data )
{
  void * rc = NULL;
  TransactionData* that = reinterpret_cast<TransactionData *>( data );
  if ( !that )
    return rc;

  static FD_t fd = NULL;
  const BinHeader header( (Header)h );

  auto iStep = key ? reinterpret_cast< const zypp::proto::target::TransactionStep * >( key ) : nullptr;
  if ( !iStep && h ) {
    auto key = headerGetInstance( header.get() );
    if ( key > 0 ) {
      auto i = that->removePckIndex.find(key);
      if ( i != that->removePckIndex.end() )
        iStep = &that->commitData.steps( i->second );
    }
  }

  const auto &sendEndOfScriptTag = [&](){
    std::cerr << "Send end of script" << std::endl;
    ::sendBytes( static_cast<int>( ExpectedFds::ScriptFd ), endOfScriptTag.data(), endOfScriptTag.size() );
  };

  switch (what) {
    case RPMCALLBACK_INST_OPEN_FILE: {

      if ( !iStep || !iStep->has_install() || iStep->install().pathname().empty() )
        return NULL;
      if ( fd != NULL )
        std::cerr << "ERR opening a file before closing the old one?  Really ? " << std::endl;
      fd = Fopen( iStep->install().pathname().data(), "r.ufdio" );
      if (fd == NULL || Ferror(fd)) {
        std::cerr << "Error when opening file " << iStep->install().pathname().data() << std::endl;
        if (fd != NULL) {
          Fclose(fd);
          fd = NULL;
        }
      } else
        fd = fdLink(fd);
      return (void *)fd;
      break;
    }

    case RPMCALLBACK_INST_CLOSE_FILE:
      fd = fdFree(fd);
      if (fd != NULL) {
        Fclose(fd);
        fd = NULL;
      }
      break;

    case RPMCALLBACK_INST_START: {

      if ( !iStep || !iStep->has_install() )
        return rc;

      zypp::proto::target::PackageBegin step;
      step.set_stepid( iStep->stepid() );
      pushMessage( step );

      break;
    }

    case RPMCALLBACK_UNINST_START: {

      if ( !iStep ) {

        if ( header.empty() ) {
          std::cerr << "No header and no transaction step for a uninstall start, not sending anything" << std::endl;
          return rc;
        }

        // this is a package cleanup send the report accordingly
        zypp::proto::target::CleanupBegin step;
        step.set_nvra( header.nvra() );
        pushMessage( step );

      } else {

        if ( !iStep->has_remove() ) {
          std::cerr << "Could not find package in removables " << header << " in transaction elements" << std::endl;
          return rc;
        }

        zypp::proto::target::PackageBegin step;
        step.set_stepid( iStep->stepid() );
        pushMessage( step );
      }
      break;
    }

    case RPMCALLBACK_INST_STOP: {

      if ( !iStep ) {
        std::cerr << "Could not find package " << header << " in transaction elements for " << what << std::endl;
        return rc;
      }

      sendEndOfScriptTag();

      zypp::proto::target::PackageFinished step;
      step.set_stepid( iStep->stepid() );
      pushMessage( step );

      break;
    }

    case RPMCALLBACK_UNINST_STOP: {

      if ( !iStep ) {
        if ( header.empty() ) {
          std::cerr << "No header and no transaction step for a uninstall stop, not sending anything" << std::endl;
          return rc;
        }

        sendEndOfScriptTag();

        // this is a package cleanup send the report accordingly
        zypp::proto::target::CleanupFinished step;
        step.set_nvra( header.nvra() );
        pushMessage( step );
      } else {

        sendEndOfScriptTag();

        zypp::proto::target::PackageFinished step;
        step.set_stepid( iStep->stepid() );
        pushMessage( step );

      }

      break;
    }
    case RPMCALLBACK_UNPACK_ERROR: {

      if ( !iStep ) {
        std::cerr << "Could not find package " << header << " in transaction elements for " << what << std::endl;
        return rc;
      }

      zypp::proto::target::PackageError step;
      step.set_stepid( iStep->stepid() );
      pushMessage( step );
      break;
    }

    case RPMCALLBACK_INST_PROGRESS: {

      if ( !iStep  )
        return rc;

      const auto progress = (double) (total
                                        ? ((((float) amount) / total) * 100)
                                        : 100.0);

      zypp::proto::target::PackageProgress step;
      step.set_stepid( iStep->stepid() );
      step.set_amount( progress );
      pushMessage( step );

      break;
    }

    case RPMCALLBACK_UNINST_PROGRESS: {

      const auto progress = (double) (total
                                        ? ((((float) amount) / total) * 100)
                                        : 100.0);
      if ( !iStep  ) {
        if ( header.empty() ) {
          std::cerr << "No header and no transaction step for a uninstall progress, not sending anything" << std::endl;
          return rc;
        }

        // this is a package cleanup send the report accordingly
        zypp::proto::target::CleanupProgress step;
        step.set_nvra( header.nvra() );
        step.set_amount( progress );
        pushMessage( step );

      } else {
        zypp::proto::target::PackageProgress step;
        step.set_stepid( iStep->stepid() );
        step.set_amount( progress );
        pushMessage( step );
      }
      break;
    }

#ifdef HAVE_RPM_VERIFY_TRANSACTION_STEP
    case RPMCALLBACK_VERIFY_START:
#endif
    case RPMCALLBACK_TRANS_START: {
      zypp::proto::target::TransBegin step;
      const char *n = "Preparing";
#ifdef HAVE_RPM_VERIFY_TRANSACTION_STEP
      if ( what == RPMCALLBACK_VERIFY_START )
        n = "Verifying";
#endif
      step.set_name( n );
      pushMessage( step );
      break;
    }

#ifdef HAVE_RPM_VERIFY_TRANSACTION_STEP
    case RPMCALLBACK_VERIFY_STOP:
#endif
    case RPMCALLBACK_TRANS_STOP: {
      sendEndOfScriptTag();
      zypp::proto::target::TransFinished step;
      pushMessage( step );
      break;
    }

#ifdef HAVE_RPM_VERIFY_TRANSACTION_STEP
    case RPMCALLBACK_VERIFY_PROGRESS:
#endif
    case RPMCALLBACK_TRANS_PROGRESS: {
      const auto percentage = (double) (total
                                      ? ((((float) amount) / total) * 100)
                                      : 100.0);
      zypp::proto::target::TransProgress prog;
      prog.set_amount( percentage );
      pushMessage( prog );
      break;
    }
    case RPMCALLBACK_CPIO_ERROR:
      std::cerr << "CPIO Error when installing package" << std::endl;
      break;
    case RPMCALLBACK_SCRIPT_START: {
      zypp::proto::target::ScriptBegin script;
      if ( iStep )
        script.set_stepid( iStep->stepid() );
      else
        script.set_stepid( -1 );
      script.set_scripttype( std::string( tagToScriptTypeName( amount ) ) );

      if ( header.get() ) {
        script.set_scriptpackage( header.nvra() );
      }

      pushMessage( script );
      break;
    }

    case RPMCALLBACK_SCRIPT_STOP: {
      sendEndOfScriptTag();
      zypp::proto::target::ScriptFinished script;
      if ( iStep )
        script.set_stepid( iStep->stepid() );
      else
        script.set_stepid( -1 );
      pushMessage( script );
      break;
    }
    case RPMCALLBACK_SCRIPT_ERROR: {
      zypp::proto::target::ScriptError script;
      if ( iStep )
        script.set_stepid( iStep->stepid() );
      else
        script.set_stepid( -1 );

      // for RPMCALLBACK_SCRIPT_ERROR 'total' is abused by librpm to distinguish between warning and "real" errors
      script.set_fatal( total != RPMRC_OK );
      pushMessage( script );
      break;
    }
    case RPMCALLBACK_UNKNOWN:
    default:
      break;
  }

  return rc;
}

int rpmLogCallback ( rpmlogRec rec, rpmlogCallbackData )
{
  int logRc = 0;

  zypp::proto::target::RpmLog log;
  log.set_level( rpmlogRecPriority(rec)  );
  log.set_line( zypp::str::asString( ::rpmlogRecMessage(rec) ) );
  std::cerr << "Pushing log message: " << log.line() << std::endl;
  pushMessage( log );

  return logRc;
}
