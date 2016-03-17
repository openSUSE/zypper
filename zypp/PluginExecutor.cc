/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PluginExecutor.cc
 */
#include <iostream>
#include "zypp/base/LogTools.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/ZConfig.h"
#include "zypp/PathInfo.h"
#include "zypp/PluginExecutor.h"

using std::endl;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::plugin"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class PluginExecutor::Impl
  /// \brief PluginExecutor implementation.
  ///////////////////////////////////////////////////////////////////
  class PluginExecutor::Impl : private base::NonCopyable
  {
  public:
    Impl()
    {}

    ~Impl()
    {
      if ( ! empty() )
	send( PluginFrame( "PLUGINEND" ) );
      // ~PluginScript will disconnect all remaining plugins!
    }

    bool empty() const
    { return _scripts.empty(); }

    size_t size() const
    { return _scripts.size(); }

    void load( const Pathname & path_r )
    {
      PathInfo pi( path_r );
      DBG << "+++++++++++++++ load " << pi << endl;
      if ( pi.isDir() )
      {
	std::list<Pathname> entries;
	if ( filesystem::readdir( entries, pi.path(), false ) != 0 )
	{
	  WAR << "Plugin dir is not readable: " << pi << endl;
	  return;
	}
	for_( it, entries.begin(), entries.end() )
	{
	  PathInfo pii( *it );
	  if ( pii.isFile() && pii.userMayRX() )
	    doLoad( pii );
	}
      }
      else if ( pi.isFile() )
      {
	if ( pi.userMayRX() )
	  doLoad( pi );
	else
	  WAR << "Plugin file is not executable: " << pi << endl;
      }
      else
      {
	WAR << "Plugin path is neither dir nor file: " << pi << endl;
      }
      DBG << "--------------- load " << pi << endl;
    }

    void send( const PluginFrame & frame_r )
    {
      DBG << "+++++++++++++++ send " << frame_r << endl;
      for ( auto it = _scripts.begin(); it != _scripts.end(); )
      {
	doSend( *it, frame_r );
	if ( it->isOpen() )
	  ++it;
	else
	  it = _scripts.erase( it );
      }
      DBG << "--------------- send " << frame_r << endl;
    }

    const std::list<PluginScript> scripts() const
    { return _scripts; }

  private:
    /** Launch a plugin sending PLUGINSTART message. */
    void doLoad( const PathInfo & pi_r )
    {
      MIL << "Load plugin: " << pi_r << endl;
      try {
	PluginScript plugin( pi_r.path() );
	plugin.open();

	PluginFrame frame( "PLUGINBEGIN" );
	if ( ZConfig::instance().hasUserData() )
	  frame.setHeader( "userdata", ZConfig::instance().userData() );

	doSend( plugin, frame );	// closes on error
	if ( plugin.isOpen() )
	  _scripts.push_back( plugin );
      }
      catch( const zypp::Exception & e )
      {
	WAR << "Failed to load plugin " << pi_r << endl;
      }
    }

    PluginFrame doSend( PluginScript & script_r, const PluginFrame & frame_r )
    {
      PluginFrame ret;

      try {
	script_r.send( frame_r );
	ret = script_r.receive();
      }
      catch( const zypp::Exception & e )
      {
	ZYPP_CAUGHT(e);
	WAR << e.asUserHistory() << endl;
      }

      // Allow using "/bin/cat" as reflector-script for testing
      if ( ! ( ret.isAckCommand() || ret.isEnomethodCommand() || ( script_r.script() == "/bin/cat" && frame_r.command() != "ERROR" ) ) )
      {
	WAR << "Bad plugin response from " << script_r << ": " << ret << endl;
	WAR << "(Expected " << PluginFrame::ackCommand() << " or " << PluginFrame::enomethodCommand() << ")" << endl;
	script_r.close();
      }

      return ret;
    }
  private:
    std::list<PluginScript> _scripts;
  };

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PluginExecutor
  //
  ///////////////////////////////////////////////////////////////////

  PluginExecutor::PluginExecutor()
    : _pimpl( new Impl() )
  {}

  PluginExecutor::~PluginExecutor()
  {}

  bool PluginExecutor::empty() const
  { return _pimpl->empty(); }

  size_t PluginExecutor::size() const
  { return _pimpl->size(); }

  void PluginExecutor::load( const Pathname & path_r )
  { _pimpl->load( path_r ); }

  void PluginExecutor::send( const PluginFrame & frame_r )
  { _pimpl->send( frame_r ); }

  std::ostream & operator<<( std::ostream & str, const PluginExecutor & obj )
  { return str << obj._pimpl->scripts(); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
