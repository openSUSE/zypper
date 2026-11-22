/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "refresh.h"
#include "repos.h"
#include "commands/conditions.h"
#include "commands/services/refresh.h"


#include "utils/messages.h"
#include "utils/flags/flagtypes.h"
#include "Zypper.h"

#include <zypp-core/base/UserRequestException>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <limits.h>
#include <map>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace zypp;

extern ZYpp::Ptr God;

namespace {
  /** Repositories expected to provide updated GPG keys (bsc#1234752).
   *  They must be refreshed before the other repositories.
   *  \li 2: 'update' directory within the repo's path
   *  \li 1: 'update' in alias or name
   */
  unsigned updateRepoRank( const RepoInfo & repo_r )
  {
    static const std::string update { "update" };
    if ( str::containsCI( repo_r.url().getPathName(), update ) )
      return 2;
    if ( str::containsCI( repo_r.alias(), update ) || str::containsCI( repo_r.name(), update ) )
      return 1;
    return 0;
  }

  // bsc#1234752: Try to refresh update repos first (to have updated GPG keys on the fly).
  // GA repos usually ship the old, maybe meanwhile expired, GPG key. If such a key was
  // prolonged, the update repo may contain it and zypp updates the trusted key on the fly
  // when refreshing it. This avoids a 'key has expired' warning being issued when refreshing
  // the GA repos
  inline std::list<RepoInfo> inRefreshOrder( std::list<RepoInfo> && list_r )
  {
    list_r.sort( []( const RepoInfo & lhs, const RepoInfo & rhs ) {
      const unsigned lrank = updateRepoRank( lhs );
      const unsigned rrank = updateRepoRank( rhs );
      if ( lrank != rrank )
        return lrank > rrank;
      return lhs.alias() < rhs.alias();
    } );
    return std::move(list_r);
  }

  /** Path of the running zypper executable. */
  const std::string & selfExecutable()
  {
    static const std::string exe = []() {
      char buf[ PATH_MAX + 1 ] = { 0 };
      ssize_t len = ::readlink( "/proc/self/exe", buf, PATH_MAX );
      return std::string( len > 0 ? buf : "/usr/bin/zypper" );
    }();
    return exe;
  }

  struct RefreshChildPipes
  {
    int outFd = -1;     ///< parent read end of the child's stdout/stderr
    int statusFd = -1;  ///< parent read end of the child's status messages
  };

  /** Fork a child process refreshing a single repository.
   *  \param capture_r redirect the child's output and progress into pipes
   *         owned by the parent.
   *  \return child pid, -1 on error. */
  pid_t startRefreshChild( const Zypper & zypper, const RepoInfo & repo_r, RefreshRepoCmd::RefreshFlags flags_r,
                           bool capture_r, RefreshChildPipes & pipes_r )
  {
    std::vector<std::string> args { selfExecutable() };
    for ( const std::string & globalOpt : zypper.globalOptions() )
      args.push_back( globalOpt );
    args.push_back( "--internal-refresh-child" );
    args.push_back( "refresh" );
    args.push_back( "--jobs" );
    args.push_back( "1" );
    if ( flags_r.testFlag( RefreshRepoCmd::Force ) )
      args.push_back( "--force" );
    if ( flags_r.testFlag( RefreshRepoCmd::ForceBuild ) )
      args.push_back( "--force-build" );
    if ( flags_r.testFlag( RefreshRepoCmd::ForceDownload ) )
      args.push_back( "--force-download" );
    if ( flags_r.testFlag( RefreshRepoCmd::DownloadOnly ) )
      args.push_back( "--download-only" );
    if ( flags_r.testFlag( RefreshRepoCmd::AllArchs ) )
      args.push_back( "--include-all-archs" );
    args.push_back( repo_r.alias() );

    std::vector<char *> argv;
    argv.reserve( args.size() + 1 );
    for ( std::string & arg : args )
      argv.push_back( const_cast<char *>( arg.c_str() ) );
    argv.push_back( nullptr );

    int outPipe[2] = { -1, -1 };
    int statusPipe[2] = { -1, -1 };
    std::string statusFdEnv;

    if ( capture_r )
    {
      if ( ::pipe( outPipe ) != 0 )
        return -1;
      if ( ::pipe( statusPipe ) != 0 )
      {
        ::close( outPipe[0] );
        ::close( outPipe[1] );
        return -1;
      }
      // The status fd must survive the dup2 of stdout/stderr
      if ( statusPipe[1] <= STDERR_FILENO )
      {
        int fd = ::fcntl( statusPipe[1], F_DUPFD, STDERR_FILENO + 1 );
        if ( fd >= 0 )
        {
          ::close( statusPipe[1] );
          statusPipe[1] = fd;
        }
      }
      statusFdEnv = "ZYPPER_REFRESH_STATUS_FD=" + std::to_string( statusPipe[1] );
    }

    // The parallel refresh parent holds the ZYpp lock. Children work on
    // distinct repositories and must not try to acquire it.
    std::vector<std::string> envStore;
    bool haveReadonlyHack = false;
    for ( char ** env = environ; *env; ++env )
    {
      std::string entry( *env );
      if ( entry.rfind( "ZYPP_READONLY_HACK=", 0 ) == 0 )
      {
        entry = "ZYPP_READONLY_HACK=1";
        haveReadonlyHack = true;
      }
      else if ( entry.rfind( "ZYPPER_REFRESH_STATUS_FD=", 0 ) == 0 )
        continue;
      envStore.push_back( std::move( entry ) );
    }
    if ( !haveReadonlyHack )
      envStore.emplace_back( "ZYPP_READONLY_HACK=1" );
    if ( !statusFdEnv.empty() )
      envStore.push_back( statusFdEnv );

    std::vector<char *> envp;
    envp.reserve( envStore.size() + 1 );
    for ( std::string & entry : envStore )
      envp.push_back( const_cast<char *>( entry.c_str() ) );
    envp.push_back( nullptr );

    const std::string & exe = selfExecutable();	// initialized before fork()
    const pid_t pid = ::fork();
    if ( pid == 0 )
    {
      if ( capture_r )
      {
        ::dup2( outPipe[1], STDOUT_FILENO );
        ::dup2( outPipe[1], STDERR_FILENO );
        ::close( outPipe[0] );
        if ( outPipe[1] != STDOUT_FILENO && outPipe[1] != STDERR_FILENO )
          ::close( outPipe[1] );
        ::close( statusPipe[0] );
      }
      ::execve( exe.c_str(), argv.data(), envp.data() );
      ::_exit( 127 );
    }

    if ( capture_r )
    {
      ::close( outPipe[1] );
      ::close( statusPipe[1] );
      if ( pid > 0 )
      {
        pipes_r.outFd = outPipe[0];
        pipes_r.statusFd = statusPipe[0];
        // Do not leak the parent's read ends into subsequent children
        ::fcntl( pipes_r.outFd, F_SETFD, FD_CLOEXEC );
        ::fcntl( pipes_r.statusFd, F_SETFD, FD_CLOEXEC );
        ::fcntl( pipes_r.outFd, F_SETFL, O_NONBLOCK );
        ::fcntl( pipes_r.statusFd, F_SETFL, O_NONBLOCK );
      }
      else
      {
        ::close( outPipe[0] );
        ::close( statusPipe[0] );
      }
    }
    return pid;
  }

  /** Draws one status line per repository and keeps it out of the way of
   *  the regular messages emitted by the child processes. */
  class RefreshDisplay
  {
  public:
    explicit RefreshDisplay( Zypper & zypper_r )
      : _zypper( zypper_r )
    {
      const char * term = ::getenv( "TERM" );
      _enabled = ::isatty( STDOUT_FILENO )
                 && term && std::string( term ) != "dumb"
                 && _zypper.out().type() == Out::TYPE_NORMAL
                 && _zypper.out().verbosity() > Out::QUIET;
      const unsigned width = _zypper.out().termwidth();
      _width = width > 1 ? width : 80;
    }

    ~RefreshDisplay()
    { eraseBlock(); }

    bool enabled() const
    { return _enabled; }

    void addLine( pid_t pid_r, const std::string & alias_r )
    {
      if ( !_enabled )
        return;
      _order.push_back( pid_r );
      _aliases[pid_r] = alias_r;
      _percent[pid_r] = -1;
      if ( _active )
        drawBlock();
    }

    void update( pid_t pid_r, int percent_r, const std::string & label_r )
    {
      if ( !_enabled || !_aliases.count( pid_r ) )
        return;
      _percent[pid_r] = percent_r;
      _labels[pid_r] = label_r;
      if ( _active )
        drawBlock();
    }

    void removeLine( pid_t pid_r )
    {
      if ( !_enabled )
        return;
      if ( _active )
        eraseBlock();
      _order.erase( std::remove( _order.begin(), _order.end(), pid_r ), _order.end() );
      _aliases.erase( pid_r );
      _percent.erase( pid_r );
      _labels.erase( pid_r );
      if ( _active )
        drawBlock();
    }

    /** Print a message above the status block. */
    void message( const std::string & text_r, bool addNewline_r = true )
    {
      if ( !_enabled )
      {
        std::cout << text_r;
        std::cout.flush();
        return;
      }
      eraseBlock();
      std::cout << text_r;
      if ( addNewline_r && !text_r.empty() && text_r.back() != '\n' )
        std::cout << '\n';
      std::cout.flush();
      if ( !text_r.empty() )
        _atLineStart = ( text_r.back() == '\n' );
      if ( _active )
        drawBlock();
    }

    /** Stop drawing the status block while the child waits for user input. */
    void beginPrompt()
    {
      if ( !_enabled )
        return;
      eraseBlock();
      _active = false;
      _drawn.clear();
    }

    /** Resume the status block after the user answered. The answer echoed by
     *  the terminal already moved the cursor to the next line. */
    void endPrompt()
    {
      if ( !_enabled )
        return;
      _atLineStart = true;
      _active = true;
      drawBlock();
    }

    void tick()
    {
      if ( !_enabled || !_active )
        return;
      ++_spin;
      drawBlock();
    }

  private:
    void eraseBlock()
    {
      if ( _lines == 0 )
        return;
      for ( size_t i = 0; i < _lines; ++i )
      {
        std::cout << "\r\033[2K";
        if ( i + 1 < _lines )
          std::cout << "\033[1A";
      }
      std::cout.flush();
      _lines = 0;
      _drawn.clear();
      _atLineStart = true;
    }

    std::string lineFor( pid_t pid_r ) const
    {
      const std::string & alias = _aliases.at( pid_r );
      std::string label;
      auto labelIt = _labels.find( pid_r );
      if ( labelIt != _labels.end() )
        label = labelIt->second;
      const int percent = _percent.at( pid_r );

      std::string line;
      if ( percent >= 0 )
      {
        static const unsigned barWidth = 12;
        const unsigned filled = static_cast<unsigned>( percent ) * barWidth / 100;
        line += "[";
        line += std::string( filled, '#' );
        line += std::string( barWidth - filled, '-' );
        line += "] ";
        std::string num = std::to_string( percent );
        if ( num.size() < 3 )
          line += std::string( 3 - num.size(), ' ' );
        line += num + "% ";
      }
      else
      {
        static const char spin[] = "|/-\\";
        line += "  ";
        line += spin[_spin % 4];
        line += "     ";
      }
      line += alias;
      if ( !label.empty() )
        line += ": " + label;

      if ( line.size() >= _width )
      {
        line.resize( _width - 1 );
        // do not cut a UTF-8 sequence in half
        while ( !line.empty() && ( static_cast<unsigned char>( line.back() ) & 0xC0 ) == 0x80 )
          line.pop_back();
      }
      return line;
    }

    void drawBlock()
    {
      if ( !_enabled || !_active )
        return;
      std::vector<std::string> lines;
      lines.reserve( _order.size() );
      for ( pid_t pid : _order )
        lines.push_back( lineFor( pid ) );
      if ( lines == _drawn )
        return;
      eraseBlock();
      for ( size_t i = 0; i < lines.size(); ++i )
      {
        std::cout << "\033[2K\r" << lines[i];
        if ( i + 1 < lines.size() )
          std::cout << '\n';
      }
      std::cout.flush();
      _lines = lines.size();
      _atLineStart = ( _lines == 0 );
      _drawn = std::move( lines );
    }

    Zypper & _zypper;
    bool _enabled = false;
    bool _active = true;
    bool _atLineStart = true;
    unsigned _width = 80;
    unsigned _spin = 0;
    size_t _lines = 0;
    std::vector<std::string> _drawn;
    std::vector<pid_t> _order;
    std::map<pid_t, std::string> _aliases;
    std::map<pid_t, int> _percent;
    std::map<pid_t, std::string> _labels;
  };

  /** Sequential waitpid based loop used when the terminal can not show a
   *  progress block. */
  unsigned runRefreshChildrenPlain( Zypper & zypper, const std::vector<RepoInfo> & repos_r,
                                    RefreshRepoCmd::RefreshFlags flags_r, unsigned jobs_r )
  {
    unsigned error_count = 0;
    std::vector<unsigned char> failed( repos_r.size(), 0 );
    std::map<pid_t, size_t> running;
    size_t next = 0;
    bool aborted = false;

    auto reportFailure = [&]( size_t idx ) {
      if ( failed[idx] )
        return;
      failed[idx] = 1;
      zypper.out().error( str::Format(_("Skipping repository '%s' because of the above error.")) % repos_r[idx].asUserString() );
      ERR << "Skipping repository '" << repos_r[idx].alias() << "' because of the above error." << endl;
      error_count++;
    };

    while ( next < repos_r.size() || !running.empty() )
    {
      if ( zypper.exitRequested() )
        aborted = true;

      while ( !aborted && running.size() < jobs_r && next < repos_r.size() )
      {
        RefreshChildPipes pipes;
        const pid_t pid = startRefreshChild( zypper, repos_r[next], flags_r, false, pipes );
        if ( pid > 0 )
        {
          running[pid] = next++;
        }
        else
        {
          zypper.out().error( str::Format(_("Could not start a refresh process for repository '%s'.")) % repos_r[next].asUserString() );
          reportFailure( next );
          ++next;
        }
      }

      if ( running.empty() )
        break;

      int status = 0;
      const pid_t pid = ::waitpid( -1, &status, 0 );
      if ( pid < 0 )
      {
        if ( errno == EINTR )
          continue;
        ERR << "waitpid failed: " << ::strerror( errno ) << endl;
        break;
      }

      auto it = running.find( pid );
      if ( it == running.end() )
        continue;
      const size_t idx = it->second;
      running.erase( it );

      if ( WIFEXITED( status ) )
      {
        if ( WEXITSTATUS( status ) != 0 )
          reportFailure( idx );
      }
      else
      {
        zypper.out().error( str::Format(_("Refresh process for repository '%s' was terminated unexpectedly.")) % repos_r[idx].asUserString() );
        reportFailure( idx );
      }
    }

    if ( aborted )
    {
      zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
      throw AbortRequestException();
    }
    return error_count;
  }

  /** Refresh \a repos_r each in its own child process, at most \a jobs_r at a time.
   *  \return number of failed repositories */
  unsigned runRefreshChildren( Zypper & zypper, const std::vector<RepoInfo> & repos_r,
                               RefreshRepoCmd::RefreshFlags flags_r, unsigned jobs_r )
  {
    if ( repos_r.empty() )
      return 0;

    RefreshDisplay display( zypper );
    if ( !display.enabled() )
      return runRefreshChildrenPlain( zypper, repos_r, flags_r, jobs_r );

    struct ChildState
    {
      size_t idx = 0;
      int outFd = -1;
      int statusFd = -1;
      std::string outBuf;
      std::string statusBuf;
    };

    unsigned error_count = 0;
    std::vector<unsigned char> failed( repos_r.size(), 0 );
    std::map<pid_t, ChildState> children;
    std::string deferredOutput;
    size_t next = 0;
    bool aborted = false;
    pid_t promptingPid = -1;

    // Print above the progress block, or delay if a child is currently prompting
    auto showMessage = [&]( const std::string & msg_r ) {
      if ( promptingPid != -1 )
        deferredOutput += msg_r + '\n';
      else
        display.message( msg_r );
    };

    auto reportFailure = [&]( size_t idx ) {
      if ( failed[idx] )
        return;
      failed[idx] = 1;
      showMessage( str::Format(_("Skipping repository '%s' because of the above error.")) % repos_r[idx].asUserString() );
      ERR << "Skipping repository '" << repos_r[idx].alias() << "' because of the above error." << endl;
      error_count++;
    };

    // Read all available data from a non-blocking fd, close it on EOF
    auto drainFd = []( int & fd_r, std::string & buf_r ) {
      if ( fd_r < 0 )
        return;
      char tmp[4096];
      while ( true )
      {
        const ssize_t len = ::read( fd_r, tmp, sizeof(tmp) );
        if ( len > 0 )
        {
          buf_r.append( tmp, len );
          continue;
        }
        if ( len == 0 )
        {
          ::close( fd_r );
          fd_r = -1;
        }
        break;
      }
    };

    auto deferOutput = [&]( ChildState & child_r ) {
      if ( child_r.outBuf.empty() )
        return;
      deferredOutput += child_r.outBuf;
      if ( child_r.outBuf.back() != '\n' )
        deferredOutput += '\n';
      child_r.outBuf.clear();
    };

    auto flushCompleteLines = [&]( ChildState & child_r ) {
      size_t pos = std::string::npos;
      while ( ( pos = child_r.outBuf.find( '\n' ) ) != std::string::npos )
      {
        std::string text = child_r.outBuf.substr( 0, pos + 1 );
        child_r.outBuf.erase( 0, pos + 1 );
        display.message( text );
      }
    };

    auto handleStatus = [&]( pid_t pid_r, ChildState & child_r, const std::string & line_r ) {
      if ( line_r.rfind( "PROGRESS ", 0 ) == 0 )
      {
        int percent = -1;
        std::string label;
        size_t sep = line_r.find( ' ', 9 );
        if ( sep != std::string::npos )
        {
          percent = std::atoi( line_r.c_str() + 9 );
          label = line_r.substr( sep + 1 );
        }
        display.update( pid_r, percent, label );
      }
      else if ( line_r == "PROMPT_START" && promptingPid == -1 )
      {
        promptingPid = pid_r;
        // The prompt text was written before PROMPT_START was sent
        drainFd( child_r.outFd, child_r.outBuf );
        display.beginPrompt();
        if ( !child_r.outBuf.empty() )
        {
          display.message( child_r.outBuf, false );
          child_r.outBuf.clear();
        }
      }
      else if ( line_r == "PROMPT_END" && promptingPid == pid_r )
      {
        promptingPid = -1;
        if ( !deferredOutput.empty() )
        {
          display.message( deferredOutput );
          deferredOutput.clear();
        }
        display.endPrompt();
      }
    };

    while ( next < repos_r.size() || !children.empty() )
    {
      if ( zypper.exitRequested() )
        aborted = true;

      while ( !aborted && children.size() < jobs_r && next < repos_r.size() )
      {
        RefreshChildPipes pipes;
        const pid_t pid = startRefreshChild( zypper, repos_r[next], flags_r, true, pipes );
        if ( pid > 0 )
        {
          ChildState child;
          child.idx = next;
          child.outFd = pipes.outFd;
          child.statusFd = pipes.statusFd;
          children.emplace( pid, std::move( child ) );
          display.addLine( pid, repos_r[next].alias() );
          ++next;
        }
        else
        {
          showMessage( str::Format(_("Could not start a refresh process for repository '%s'.")) % repos_r[next].asUserString() );
          reportFailure( next );
          ++next;
        }
      }

      if ( children.empty() )
        break;

      std::vector<pollfd> pfds;
      pfds.reserve( children.size() * 2 );
      for ( const auto & entry : children )
      {
        if ( entry.second.outFd >= 0 )
          pfds.push_back( { entry.second.outFd, POLLIN, 0 } );
        if ( entry.second.statusFd >= 0 )
          pfds.push_back( { entry.second.statusFd, POLLIN, 0 } );
      }

      const int pollRes = ::poll( pfds.data(), pfds.size(), 100 );
      if ( pollRes < 0 && errno != EINTR )
      {
        ERR << "poll failed: " << ::strerror( errno ) << endl;
        break;
      }
      if ( pollRes == 0 )
        display.tick();

      for ( auto & entry : children )
      {
        ChildState & child = entry.second;
        drainFd( child.outFd, child.outBuf );
        drainFd( child.statusFd, child.statusBuf );

        size_t pos = std::string::npos;
        while ( ( pos = child.statusBuf.find( '\n' ) ) != std::string::npos )
        {
          std::string line = child.statusBuf.substr( 0, pos );
          child.statusBuf.erase( 0, pos + 1 );
          handleStatus( entry.first, child, line );
        }

        if ( promptingPid == -1 )
          flushCompleteLines( child );
        else if ( promptingPid == entry.first )
        {
          // Prompt text and help messages may not end with a newline
          if ( !child.outBuf.empty() )
          {
            display.message( child.outBuf, false );
            child.outBuf.clear();
          }
        }
        else
          deferOutput( child );		// print after the prompt
      }

      int status = 0;
      pid_t pid = 0;
      while ( ( pid = ::waitpid( -1, &status, WNOHANG ) ) > 0 )
      {
        auto it = children.find( pid );
        if ( it == children.end() )
          continue;
        ChildState & child = it->second;

        drainFd( child.outFd, child.outBuf );
        drainFd( child.statusFd, child.statusBuf );
        if ( child.outFd >= 0 )
          ::close( child.outFd );
        if ( child.statusFd >= 0 )
          ::close( child.statusFd );

        if ( promptingPid == pid )
        {
          promptingPid = -1;
          deferOutput( child );
          if ( !deferredOutput.empty() )
          {
            display.message( deferredOutput );
            deferredOutput.clear();
          }
          display.endPrompt();
        }
        else if ( promptingPid != -1 )
        {
          deferOutput( child );
        }
        else
        {
          flushCompleteLines( child );
          if ( !child.outBuf.empty() )
          {
            display.message( child.outBuf );
            child.outBuf.clear();
          }
        }

        const size_t idx = child.idx;
        display.removeLine( pid );
        if ( WIFEXITED( status ) )
        {
          if ( WEXITSTATUS( status ) != 0 )
            reportFailure( idx );
        }
        else
        {
          showMessage( str::Format(_("Refresh process for repository '%s' was terminated unexpectedly.")) % repos_r[idx].asUserString() );
          reportFailure( idx );
        }
        children.erase( it );
      }
    }

    // In case we bailed out early (poll/waitpid error), do not leave children behind
    for ( auto & entry : children )
    {
      if ( entry.second.outFd >= 0 )
        ::close( entry.second.outFd );
      if ( entry.second.statusFd >= 0 )
        ::close( entry.second.statusFd );
    }
    children.clear();
    while ( ::waitpid( -1, nullptr, 0 ) > 0 )
    {}

    if ( aborted )
    {
      zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
      throw AbortRequestException();
    }
    return error_count;
  }

  /** Number of parallel repository refreshes to use by default. */
  unsigned defaultJobs()
  {
    const long nprocs = ::sysconf( _SC_NPROCESSORS_ONLN );
    return nprocs > 0 ? static_cast<unsigned>( nprocs ) : 1u;
  }

} // namespace

RefreshRepoCmd::RefreshRepoCmd(std::vector<std::string> &&commandAliases_r )
  : ZypperBaseCommand (
      std::move( commandAliases_r ),
      _("refresh (ref) [ALIAS|#|URI] ..."),
      // translators: command summary: refresh, ref
      _("Refresh all repositories."),
      // translators: command description
      std::string(_("Refresh repositories specified by their alias, number or URI. If none are specified, all enabled repositories will be refreshed.")) + "\n\n" +
      _("HINT: 'zypper -vv ref' will show the mirrors used to download the metadata.")
      ,
      ResetRepoManager
  )
{

}

std::vector<BaseCommandConditionPtr> RefreshRepoCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup RefreshRepoCmd::cmdOptions() const
{
  auto that = const_cast<RefreshRepoCmd *>(this);
  return {{
      {"force",'f', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, Force ),
            // translators: -f, --force
            _("Force a complete refresh.")
      },
      {"force-build", 'b', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, ForceBuild ),
            // translators: -b, --force-build
            _("Force rebuild of the database.")
      },
      {"force-download", 'd', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, ForceDownload ),
            // translators: -d, --force-download
            _("Force download of raw metadata.")
      },
      {"build-only", 'B', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, BuildOnly ),
            // translators: -B, --build-only
            _("Only build the database, don't download metadata.")
      },
      {"download-only", 'D', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, DownloadOnly ),
            // translators: -D, --download-only
            _("Only download raw metadata, don't build the database.")
      },
      {"include-all-archs", 0, ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, AllArchs ),
            // translators: --include-all-archs
            _("Multi-Arch repos: Download raw metadata for all offered architectures even if the repo supports filtering.")
      },
      {"repo", 'r', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable,
            ZyppFlags::StringVectorType( &that->_repos, ARG_REPOSITORY),
            // translators: -r, --repo <ALIAS|#|URI>
            _("Refresh only specified repositories.")
      }
      ,
      {"services", 's', ZyppFlags::NoArgument,
            ZyppFlags::BoolType( &that->_services, ZyppFlags::StoreTrue, _services ),
            // translators: -s, --services
            _("Refresh also services before refreshing repos.")
      },
      {"jobs", 'j', ZyppFlags::RequiredArgument,
            ZyppFlags::IntType( &that->_jobs ),
            // translators: -j, --jobs <N>
            _("Number of repositories to refresh in parallel. The default is the number of CPUs, limited by the number of repositories. Use '--jobs 1' to refresh sequentially.")
      },
  }};
}

void RefreshRepoCmd::doReset()
{
  _flags = Default;
  _repos.clear();
  _services = false;
  _jobs = 0;
}

int RefreshRepoCmd::execute( Zypper &zypper , const std::vector<std::string> &positionalArgs_r )
{
  if ( zypper.config().no_refresh )
    zypper.out().warning( str::Format(_("The '%s' global option has no effect here.")) % "--no-refresh" );

  if ( _jobs < 0 )
  {
    zypper.out().error( str::Format(_("Invalid number of jobs: '%s'.")) % str::numstring(_jobs) );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  // Child processes spawned for parallel refresh must not refresh services
  // again (the parent has already done this) and must not spawn further children.
  const bool refreshChild = zypper.config().internal_refresh_child;

  bool force = _flags.testFlag(Force);

  if ( _services )
  {
    if ( !positionalArgs_r.empty() )
    {
      zypper.out().error(str::form(_("Arguments are not allowed if '%s' is used."), "--services"));
      return ( ZYPPER_EXIT_ERR_PRIVILEGES );
    }

    RefreshServicesCmd refServiceCmd( {} );
    refServiceCmd.setForce( force );
    refServiceCmd.setRestoreStatus( false );
    refServiceCmd.setWithRepos( false );

    // needed to be able to retrieve target distribution
    int code = defaultSystemSetup ( zypper, InitTarget );
    if ( code != ZYPPER_EXIT_OK )
      return code;

    zypper.configNoConst().rm_options.servicesTargetDistro = God->target()->targetDistribution();

    code = refServiceCmd.refreshServices( zypper );
    if ( code != ZYPPER_EXIT_OK )
      return code;
  }
  else if ( !refreshChild )
  {
    RepoManager::RefreshServiceOptions opts;
    if ( force )
      opts |= RepoManager::RefreshService_forceRefresh;

    checkIfToAutoRefreshServices( zypper, opts );
  }

  MIL << "going to refresh repositories" << endl;
  // need gpg keys when downloading (#304672)
  int code = defaultSystemSetup ( zypper, InitTarget );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  std::vector<std::string> specifiedRepos = _repos;
  for ( const std::string &repoFromCLI : positionalArgs_r )
    specifiedRepos.push_back(repoFromCLI);

  const unsigned jobs = refreshChild ? 1u : static_cast<unsigned>( _jobs );
  return refreshRepositories ( zypper, _flags, specifiedRepos, jobs );
}

bool RefreshRepoCmd::refreshRepository(Zypper &zypper, const RepoInfo &repo, RefreshFlags flags_r)
{
  MIL << "going to refresh repo '" << repo.alias() << "'" << endl;

  // https://code.opensuse.org/leap/features/issue/193
  // https://github.com/openSUSE/zypper/issues/598
  // --include-all-archs is a stub for SUMA(MLM) in case #193 is implemented
  // and honoured by libzypp. Actually we'd prefer using the single-arch
  // repos rather than filtering, but we will see how Leap evolves.
#warning RefreshFlagsBits::AllArchs is a NO-OP - propagate it to libzypp

  // raw metadata refresh
  bool error = false;
  if ( !flags_r.testFlag(BuildOnly) )
  {
    bool force_download = flags_r.testFlag(Force) || flags_r.testFlag(ForceDownload);
    MIL << "calling refreshMetadata" << (force_download ? ", forced" : "") << endl;
    error = refresh_raw_metadata( zypper, repo, force_download, zypper.config().internal_refresh_child );
  }

  // db rebuild
  if ( !( error || flags_r.testFlag(DownloadOnly) ) )
  {
    bool force_build = flags_r.testFlag(Force) || flags_r.testFlag(ForceBuild);;
    MIL << "calling buildCache" << (force_build ? ", forced" : "") << endl;
    error = build_cache( zypper, repo, force_build );
  }

  return error;
}

unsigned RefreshRepoCmd::refreshRepositories( Zypper & zypper, const std::vector<RepoInfo> & repos_r, RefreshFlags flags_r, unsigned jobs_r )
{
  if ( repos_r.empty() )
    return 0;

  unsigned jobs = jobs_r ? jobs_r : defaultJobs();
  jobs = std::min( jobs, static_cast<unsigned>( repos_r.size() ) );

  // Refreshing in parallel child processes is only safe if they do not
  // have to share the parent's state or the output format.
  const bool useChildren = jobs > 1
    && !flags_r.testFlag(BuildOnly)
    && !zypper.runningShell()
    && !zypper.config().machine_readable
    && zypper.runtimeData().plusContentRepos.empty()
    && zypper.config().plusRepoFromCLI.empty();

  if ( !useChildren )
  {
    unsigned error_count = 0;
    for ( const RepoInfo & repo : repos_r )
    {
      if ( refreshRepository( zypper, repo, flags_r ) )
      {
        // A child process reports failures to its parent, see below.
        if ( !zypper.config().internal_refresh_child )
        {
          zypper.out().error( str::Format(_("Skipping repository '%s' because of the above error.")) % repo.asUserString() );
          ERR << "Skipping repository '" << repo.alias() << "' because of the above error." << endl;
        }
        error_count++;
      }
    }
    return error_count;
  }

  // bsc#1234752: refresh repositories providing updated GPG keys first.
  // They are handled sequentially before the others are started in parallel.
  std::vector<RepoInfo> updateRepos;
  std::vector<RepoInfo> otherRepos;
  for ( const RepoInfo & repo : repos_r )
  {
    if ( updateRepoRank( repo ) )
      updateRepos.push_back( repo );
    else
      otherRepos.push_back( repo );
  }

  // bsc#1234752: complete these before starting the parallel refresh of the others
  unsigned error_count = 0;
  for ( const RepoInfo & repo : updateRepos )
  {
    if ( refreshRepository( zypper, repo, flags_r ) )
    {
      zypper.out().error( str::Format(_("Skipping repository '%s' because of the above error.")) % repo.asUserString() );
      ERR << "Skipping repository '" << repo.alias() << "' because of the above error." << endl;
      error_count++;
    }
  }

  error_count += runRefreshChildren( zypper, otherRepos, flags_r, jobs );
  return error_count;
}

int RefreshRepoCmd::refreshRepositories( Zypper &zypper, RefreshFlags flags_r, const std::vector<std::string> repos_r, unsigned jobs_r )
{
  RepoManager & manager( zypper.repoManager() );
  // bsc#1234752: Try to refresh update repos first (to have updated GPG keys on the fly)
  const std::list<RepoInfo> & repos { inRefreshOrder( manager.knownRepositories() ) };

  // get the list of repos specified on the command line ...
  std::list<RepoInfo> specified;
  std::list<std::string> not_found;
  get_repos( zypper, repos_r.begin(),repos_r.end(), specified, not_found );
  report_unknown_repos( zypper.out(), not_found );

  // --plus-content: It either specifies a known repo (by #, alias or URL)
  // or we need to also refresh all disabled repos to get their content
  // keywords.
  std::set<RepoInfo> plusContent;
  bool doContentCheck = false;
  for ( const std::string & spec : zypper.runtimeData().plusContentRepos )
  {
    RepoInfo r;
    if ( match_repo( zypper, spec, &r ) )
      plusContent.insert( r );	// specific repo: add to plusContent
    else if ( ! doContentCheck )
      doContentCheck = true;	// keyword: need to scan all disabled repos
  }

  std::ostringstream s;
  s << _("Specified repositories: ");
  for_( it, specified.begin(), specified.end() )
    s << it->alias() << " ";
  zypper.out().info( s.str(), Out::HIGH );

  unsigned error_count = 0;
  unsigned enabled_repo_count = repos.size();
  std::vector<RepoInfo> refreshList;

  if ( !specified.empty() || not_found.empty() )
  {
    for_( rit, repos.begin(), repos.end() )
    {
      const RepoInfo & repo( *rit );

      if ( repo.enabled() )
      {
        // enabled: Refreshed unless restricted by CLI args or mentioned in
        // --plus-content as specific repo.
        if ( !specified.empty() && std::find( specified.begin(), specified.end(), repo ) == specified.end() )
        {
          if ( plusContent.count( repo ) )
          {
            MIL << "[--plus-content] check " << repo.alias() << endl;
            zypper.out().info( str::Format(_("Refreshing repository '%s'.")) % repo.asUserString(),
                               " [--plus-content]" );
          }
          else
          {
            DBG << repo.alias() << "(#" << ") not specified," << " skipping." << endl;
            enabled_repo_count--;
            continue;
          }
        }
      }
      else
      {
        // disabled: No refresh unless mentioned in --plus-content (specific or content check).
        // CLI args reffering to disabled repos are reported as error.
        if ( doContentCheck || plusContent.count( repo ) )
        {
          MIL << "[--plus-content] check " << repo.alias() << endl;
          zypper.out().info( str::Format(_("Scanning content of disabled repository '%s'.")) % repo.asUserString(),
                             " [--plus-content]" );
        }
        else
        {
          if ( !specified.empty() && std::find( specified.begin(), specified.end(), repo ) == specified.end() )
          {
            DBG << repo.alias() << "(#" << ") not specified," << " skipping." << endl;
          }
          else
          {
            std::string msg( str::Format(_("Skipping disabled repository '%s'")) % repo.asUserString() );

            if ( specified.empty() )
              zypper.out().info( msg, Out::HIGH );
            else
              zypper.out().error( msg );
          }
          enabled_repo_count--;
          continue;
        }
      }

      refreshList.push_back( repo );
    }
  }
  else
    enabled_repo_count = 0;

  error_count = refreshRepositories( zypper, refreshList, flags_r, jobs_r );

  // A refresh child reports problems itself. The overall result is
  // reported by the parent process.
  if ( zypper.config().internal_refresh_child )
  {
    if ( !not_found.empty() )
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
    if ( enabled_repo_count == 0 )
      return ZYPPER_EXIT_NO_REPOS;
    if ( error_count )
      return ZYPPER_EXIT_ERR_ZYPP;
    return ZYPPER_EXIT_OK;
  }

  // print the result message
  if ( !not_found.empty() )
  {
      zypper.out().error(_("Some of the repositories have not been refreshed because they were not known.") );
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }
  else if ( enabled_repo_count == 0 )
  {
    int code = ZYPPER_EXIT_OK;
    if ( !specified.empty() )
      zypper.out().warning(_("Specified repositories are not enabled or defined.") );
    else {
      zypper.out().warning(_("There are no enabled repositories defined.") );
      code = ( ZYPPER_EXIT_NO_REPOS );
    }

    zypper.out().info( str::form(_("Use '%s' or '%s' commands to add or enable repositories."),
                                 "zypper addrepo", "zypper modifyrepo" ) );
    return code;
  }
  else if ( error_count == enabled_repo_count )
  {
    zypper.out().error(_("Could not refresh the repositories because of errors.") );
    return ( ZYPPER_EXIT_ERR_ZYPP );
  }
  else if ( error_count )
  {
    zypper.out().error(_("Some of the repositories have not been refreshed because of an error.") );
    return ( ZYPPER_EXIT_ERR_ZYPP );
  }
  else if ( !specified.empty() )
    zypper.out().info(_("Specified repositories have been refreshed.") );
  else
    zypper.out().info(_("All repositories have been refreshed.") );

  return ZYPPER_EXIT_OK;
}
