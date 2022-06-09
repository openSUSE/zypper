/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/PluginRepoverification.cc
 */
#include <iostream>
#include <sstream>

#include "PluginRepoverification.h"

#include <zypp/Globals.h>
#include <zypp/PathInfo.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/ExternalProgram.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/WatchFile.h>
using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp_private
{
  using namespace zypp;
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {

    struct Monitor
    {
      /** Report a line of output (without trailing NL) otherwise a life ping on timeout. */
      using Callback = std::function<bool(std::optional<std::string>)>;

      Monitor( io::timeout_type timeout_r = io::no_timeout )
      : _timeout { timeout_r }
      {}

      int operator()( ExternalProgram & prog_r, Callback cb_r = Callback() )
      {
        std::string line;
        bool goOn = true;
        prog_r.setBlocking( false );
        FILE * inputfile = prog_r.inputFile();
        do {
          const auto &readResult = io::receiveUpto( inputfile, '\n', _timeout );
          line += readResult.second; // we always may have received a partial line
          goOn = true;
          switch ( readResult.first ) {

            case io::ReceiveUpToResult::Success:
              goOn = reportLine( line, cb_r );
              line.clear(); // in case the CB did not move it out
              break;

            case io::ReceiveUpToResult::Timeout:
              goOn = reportTimeout( cb_r );
              break;

            case io::ReceiveUpToResult::Error:
            case io::ReceiveUpToResult::EndOfFile:
              reportFinalLineUnlessEmpty( line, cb_r );
              line.clear(); // in case the CB did not move it out
              goOn = false;
              break;
          }
        } while ( goOn );

        if ( prog_r.running() ) {
          WAR << "ABORT by callback: pid " << prog_r.getpid() << endl;
          prog_r.kill();
        }
        return prog_r.close();
      }

    private:
      bool reportLine( std::string & line_r, Callback & cb_r )
      {
        if ( cb_r ) {
          if ( not line_r.empty() && line_r.back() == '\n' )
            line_r.pop_back();
          return cb_r( std::move(line_r) );
        }
        return true;
      }
      bool reportTimeout( Callback & cb_r )
      {
        return cb_r ? cb_r( std::nullopt ) : true;
      }
      bool reportFinalLineUnlessEmpty( std::string & line_r, Callback & cb_r )
      {
        if ( cb_r && not line_r.empty() ) // implies an incomplete line (no NL)
          cb_r( std::move(line_r) );
        return false;
      }
    private:
      io::timeout_type _timeout = io::no_timeout;
    };

    ///////////////////////////////////////////////////////////////////
    /// \class PluginRepoverification::Checker::Impl
    /// \brief PluginRepoverification::Checker data storage.
    ///////////////////////////////////////////////////////////////////
    class PluginRepoverification::Checker::Impl
    {
    public:
      Impl( RW_pointer<PluginRepoverification::Impl> parent_r,
            Pathname sigpathLocal_r, Pathname keypathLocal_r, const RepoInfo & repo_r )
      : _parent { parent_r }
      , _sigpathLocal { std::move(sigpathLocal_r) }
      , _keypathLocal { std::move(keypathLocal_r) }
      , _repoinfo { repo_r }
      {}

      RW_pointer<PluginRepoverification::Impl> _parent;
      Pathname _sigpathLocal;
      Pathname _keypathLocal;
      RepoInfo _repoinfo;
    };

    ///////////////////////////////////////////////////////////////////
    /// \class PluginRepoverification::Impl
    /// \brief PluginRepoverification implementation.
    ///////////////////////////////////////////////////////////////////
    class PluginRepoverification::Impl
    {
      friend std::ostream & operator<<( std::ostream & str, const Impl & obj );
      friend std::ostream & dumpOn( std::ostream & str, const Impl & obj );

    public:
      Impl()
      {}

      Impl( Pathname plugindir_r, Pathname chroot_r )
      : _watchPlugindir { std::move(plugindir_r), WatchFile::NO_INIT }
      , _chroot { std::move(chroot_r) }
      {}

      ~Impl()
      {}

      bool isNeeded() const
      { return _isNeeded; }

      bool checkIfNeeded()
      {
        if ( _watchPlugindir.hasChanged() ) {
          _isNeeded = false;
          // check for at least one executable plugin inside..
          filesystem::dirForEach( plugindir(),
                                  [this]( const Pathname & dir_r, const char *const name_r ) -> bool {
                                    PathInfo pi ( dir_r/name_r );
                                    if ( pi.isFile() && pi.userMayRX() ) {
                                      this->_isNeeded = true;
                                      return false;
                                    }
                                    return true;
                                  } );
        }
        return _isNeeded;
      }

      void verifyWorkflow( const Pathname & file_r, RW_pointer<PluginRepoverification::Checker::Impl> datap_r ) const
      {
        // Execute the plugins. They will throw if something is wrong...
        filesystem::dirForEach( plugindir(),
                                [&,this]( const Pathname & dir_r, const char *const name_r ) -> bool {
                                  PathInfo pi ( dir_r/name_r );
                                  if ( pi.isFile() && pi.userMayRX() )
                                    this->pluginVerify( name_r, file_r, *datap_r );
                                  return true;
                                } );
      }

    private:
      void pluginVerify( std::string plugin_r, const Pathname & file_r, const PluginRepoverification::Checker::Impl & data_r ) const
      {
        Pathname pluginPath { plugindir()/plugin_r };
        if ( not _chroot.emptyOrRoot() ) {
          pluginPath = Pathname::stripprefix( _chroot, pluginPath );
          // we need to make sure the files are available inside the chroot
          INT << "chroot PluginRepoverification does not yet work." << endl;
          return;
        }

        ExternalProgram::Arguments args;
        args.push_back( pluginPath.asString() );
        /// NOTE: Update plugin-repoverification page if new args are supplied
        args.push_back( "--file" );
        args.push_back( file_r.asString() );
        args.push_back( "--fsig" );
        args.push_back( data_r._sigpathLocal.asString() );
        args.push_back( "--fkey" );
        args.push_back( data_r._keypathLocal.asString() );
        args.push_back( "--ralias" );
        args.push_back( data_r._repoinfo.alias() );
        ExternalProgram cmd { args, ExternalProgram::Stderr_To_Stdout, false, -1, false, _chroot };

        // draft: maybe integrate jobReport into Monitor
        Monitor monitor( 800 );
        UserDataJobReport jobReport { "cmdout", "monitor" };
        jobReport.set( "CmdId",   unsigned(cmd.getpid()) );
        jobReport.set( "CmdTag",  str::numstring( cmd.getpid() ) );
        jobReport.set( "CmdName", "Repoverification plugin "+plugin_r );
        jobReport.set( "RepoInfo", data_r._repoinfo );

        std::optional<std::ostringstream> buffer; // Send output in exception is no one is listening
        jobReport.debug( "?" ); // someone listening?
        if ( not jobReport.haskey( "!" ) ) // no
          buffer = std::ostringstream();

        int ret = monitor( cmd, [&jobReport,&buffer,&cmd]( std::optional<std::string> line_r )->bool {
          if ( line_r ) {
            DBG << "["<<cmd.getpid()<<"> " << *line_r << endl;
            if ( buffer ) (*buffer) << *line_r << endl;
            return jobReport.data( *line_r );
          }
          else {
            return jobReport.debug( "ping" );
          }
          return true;
        } );

        if ( ret ) {
          const std::string & msg { str::Format( "Metadata rejected by '%1%' plugin (returned %2%)" ) % plugin_r % ret };

          ExceptionType excp { msg };
          if ( buffer ) excp.addHistory( buffer->str() );
          excp.addHistory( str::Format( "%1%%2% returned %3%" ) % (_chroot.emptyOrRoot()?"":"("+_chroot.asString()+")") % pluginPath % ret );

          ZYPP_THROW( std::move(excp) );
        }
      }

      const Pathname & plugindir() const
      { return _watchPlugindir.path(); }

    private:
      WatchFile _watchPlugindir;
      Pathname  _chroot;
      bool _isNeeded = false;
    };

    /** \relates PluginRepoverification::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const PluginRepoverification::Impl & obj )
    { return str << "PluginRepoverification::Impl"; }

    /** \relates PluginRepoverification::Impl Verbose stream output */
    inline std::ostream & dumpOn( std::ostream & str, const PluginRepoverification::Impl & obj )
    { return str << obj; }


    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PluginRepoverification
    //
    ///////////////////////////////////////////////////////////////////

    PluginRepoverification::PluginRepoverification()
    : _pimpl( new Impl )
    {}

    PluginRepoverification::PluginRepoverification( Pathname plugindir_r, Pathname chroot_r )
    : _pimpl( new Impl( std::move(plugindir_r), std::move(chroot_r) ) )
    {}

    PluginRepoverification::~PluginRepoverification()
    {}


    bool PluginRepoverification::isNeeded() const
    { return _pimpl->isNeeded(); }

    bool PluginRepoverification::checkIfNeeded()
    { return _pimpl->checkIfNeeded(); }

    PluginRepoverification::Checker PluginRepoverification::getChecker( const Pathname & sigpathLocal_r, const Pathname & keypathLocal_r,
                                                                        const RepoInfo & repo_r ) const
    { return Checker( new Checker::Impl( _pimpl, sigpathLocal_r, keypathLocal_r, repo_r ) ); }


    std::ostream & operator<<( std::ostream & str, const PluginRepoverification & obj )
    { return str << *obj._pimpl; }

    std::ostream & dumpOn( std::ostream & str, const PluginRepoverification & obj )
    { return dumpOn( str, *obj._pimpl ); }

    bool operator==( const PluginRepoverification & lhs, const PluginRepoverification & rhs )
    { return lhs._pimpl == rhs._pimpl; }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PluginRepoverification::Checker
    //
    ///////////////////////////////////////////////////////////////////
    PluginRepoverification::Checker::Checker( Impl* pimpl_r )
    : _pimpl { pimpl_r }
    {}

    PluginRepoverification::Checker::~Checker()
    {}

    void PluginRepoverification::Checker::operator()( const Pathname & file_r ) const
    { _pimpl->_parent->verifyWorkflow( file_r, _pimpl ); }


  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
