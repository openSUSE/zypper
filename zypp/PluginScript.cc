/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PluginScript.cc
 *
*/
#include <iostream>
#include <sstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"

#include "zypp/PluginScript.h"
#include "zypp/ExternalProgram.h"
#include "zypp/PathInfo.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PluginScript::Impl
  //
  /** \ref PluginScript implementation. */
  struct PluginScript::Impl
  {
    public:
      Impl( const Pathname & script_r = Pathname(), const Arguments & args_r = Arguments() )
        : _script( script_r )
        , _args( args_r )
      {}

      ~ Impl()
      { try { close(); } catch(...) {} }

    public:
      const Pathname & script() const
      { return _script; }

      const Arguments & args() const
      { return _args; }

      pid_t getPid() const
      { return _cmd ? _cmd->getpid() : NotConnected; }

      bool isOpen() const
      { return _cmd; }

    public:
      void open( const Pathname & script_r = Pathname(), const Arguments & args_r = Arguments() );

      void close();

      void send( const PluginFrame & frame_r ) const;

      PluginFrame receive() const;

    private:
      Pathname _script;
      Arguments _args;
      scoped_ptr<ExternalProgram> _cmd;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PluginScrip::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PluginScript::Impl & obj )
  {
    return dumpRangeLine( str << "PluginScript[" << obj.getPid() << "] " << obj.script(),
			  obj.args().begin(), obj.args().end() );
  }

  ///////////////////////////////////////////////////////////////////

  void PluginScript::Impl::open( const Pathname & script_r, const Arguments & args_r )
  {
    dumpRangeLine( DBG << "Open " << script_r, args_r.begin(), args_r.end() ) << endl;

    if ( _cmd )
      ZYPP_THROW( PluginScriptException( "Already connected", str::Str() << *this ) );

    {
      PathInfo pi( script_r );
      if ( ! ( pi.isFile() && pi.isX() ) )
	ZYPP_THROW( PluginScriptException( "Script is not executable", str::Str() << pi ) );
    }

    // go and launch script
    // TODO: ExternalProgram::maybe use Stderr_To_FileDesc for script loging
    Arguments args;
    args.reserve( args_r.size()+1 );
    //args.push_back( "<"+script_r.asString() );
    args.push_back( script_r.asString() );
    args.insert( args.end(), args_r.begin(), args_r.end() );
    _cmd.reset( new ExternalProgram( args, ExternalProgram::Discard_Stderr ) );

    // store running scripts data
    _script = script_r;
    _args = args_r;

    DBG << *this << endl;
  }

  void PluginScript::Impl::close()
  {
    if ( _cmd )
    {
      DBG << "Close:" << *this << endl;
      _cmd->kill();
      _cmd.reset();
    }
    DBG << *this << endl;
  }

  void PluginScript::Impl::send( const PluginFrame & frame_r ) const
  {
    if ( !_cmd )
      ZYPP_THROW( PluginScriptException( "Not connected", str::Str() << *this ) );

    if ( frame_r.command().empty() )
      WAR << "Send: No command in frame" << frame_r << endl;

    // TODO: dumb writer does not care about error or deadlocks
    std::string data;
    {
      std::ostringstream datas;
      frame_r.writeTo( datas );
      datas.str().swap( data );
    }
    DBG << frame_r << endl;
    DBG << " ->write " << data.size() << endl;

    FILE * outputfile = _cmd->outputFile();
    if ( ::fwrite( data.c_str(), data.size(), 1, outputfile ) != 1 )
      ZYPP_THROW( PluginScriptException( "Send: send error" ) );
    ::fflush( outputfile );
  }

  PluginFrame PluginScript::Impl::receive() const
  {
    if ( !_cmd )
      ZYPP_THROW( PluginScriptException( "Not connected", str::Str() << *this ) );

    // TODO: dumb writer does not care about error or deadlocks
    std::string data( _cmd->receiveUpto( '\0' ) );
    DBG << " <-read " << data.size() << endl;
    std::istringstream datas( data );
    PluginFrame ret( datas );
    DBG << ret << endl;
    return ret;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PluginScript
  //
  ///////////////////////////////////////////////////////////////////

  const pid_t PluginScript::NotConnected( -1 );

  PluginScript::PluginScript()
    : _pimpl( new Impl )
  {}

  PluginScript::PluginScript( const Pathname & script_r )
    : _pimpl( new Impl( script_r ) )
  {}

  PluginScript::PluginScript( const Pathname & script_r, const Arguments & args_r )
    : _pimpl( new Impl( script_r, args_r ) )
  {}

  const Pathname & PluginScript::script() const
  { return _pimpl->script(); }

  const PluginScript::Arguments & PluginScript::args() const
  { return _pimpl->args(); }

  bool PluginScript::isOpen() const
  { return _pimpl->isOpen(); }

  pid_t PluginScript::getPid() const
  { return _pimpl->getPid(); }

  void PluginScript::open()
  { _pimpl->open( _pimpl->script(), _pimpl->args() ); }

  void PluginScript::open( const Pathname & script_r )
  { _pimpl->open( script_r ); }

  void PluginScript::open( const Pathname & script_r, const Arguments & args_r )
  { _pimpl->open( script_r, args_r ); }

  void PluginScript::close()
  { _pimpl->close(); }

  void PluginScript::send( const PluginFrame & frame_r ) const
  { _pimpl->send( frame_r ); }

  PluginFrame PluginScript::receive() const
  { return _pimpl->receive(); }

  ///////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const PluginScript & obj )
  { return str << *obj._pimpl; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
