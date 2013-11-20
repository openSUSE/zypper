#include <sys/types.h>
#include <signal.h>

#include <iostream>
#include <sstream>

#include "TestSetup.h"
#include "zypp/PluginScript.h"

BOOST_AUTO_TEST_CASE(InitialSettings)
{
  PluginScript::defaultTimeout( 3 );
}

BOOST_AUTO_TEST_CASE(PluginFrameDefaultCtor)
{
  PluginFrame f;
  BOOST_CHECK_EQUAL( f.empty(), true );
  BOOST_CHECK_EQUAL( bool(f), f.empty() );
  BOOST_CHECK_EQUAL( f.command().empty(), true );
  BOOST_CHECK_EQUAL( f.body().empty(), true );
  BOOST_CHECK_EQUAL( f.headerEmpty(), true );
  BOOST_CHECK_EQUAL( (f == f), true );
  BOOST_CHECK_EQUAL( (f != f), false );
}

BOOST_AUTO_TEST_CASE(PluginFrameCtorAssign)
{
  PluginFrame f( "command" );
  BOOST_CHECK_EQUAL( f.empty(), false );
  BOOST_CHECK_EQUAL( bool(f), f.empty() );
  BOOST_CHECK_EQUAL( f.command(), "command" );
  BOOST_CHECK_EQUAL( f.body().empty(), true );
  BOOST_CHECK_EQUAL( f.headerEmpty(), true );
  BOOST_CHECK_EQUAL( (f == f), true );
  BOOST_CHECK_EQUAL( (f != f), false );

  PluginFrame g( "command", "body" );
  BOOST_CHECK_EQUAL( g.empty(), false );
  BOOST_CHECK_EQUAL( bool(g), g.empty() );
  BOOST_CHECK_EQUAL( g.command(), "command" );
  BOOST_CHECK_EQUAL( g.body(), "body");
  BOOST_CHECK_EQUAL( g.headerEmpty(), true );
  BOOST_CHECK_EQUAL( (f == g), false );
  BOOST_CHECK_EQUAL( (f != g), true );

  PluginFrame h( g );
  BOOST_CHECK_EQUAL( (g == h), true );

  h.addHeader( "" ); // empty KV in header is ok, if you like it
  BOOST_CHECK_EQUAL( (g == h), false );

  h.addHeader({
    { "a", "a1" },
    { "a", "a2" },
    { "b", "b1" },
    { "b", "b2" },
    { "c", "c1" },
    { "c", "c1" }
  });
  BOOST_CHECK_EQUAL( h.headerSize(), 7 );

  h.setHeader( "b", "b" ); // replaces existing 'b:" headers
  BOOST_CHECK_EQUAL( h.headerSize(), 6 );

  // now write and reparse from stream:
  std::string data;
  {
    std::ostringstream datas;
    h.writeTo( datas );
    datas.str().swap( data );
  }
  std::istringstream datas( data );
  PluginFrame i( datas );
  BOOST_CHECK_EQUAL( (h == i), true );
}

void doParse( const std::string & str_r )
{
  std::string data( str_r );
  data.push_back( '\0' );
  std::istringstream datas( data );
   PluginFrame i( datas );
}

BOOST_AUTO_TEST_CASE(PluginFrameExceptipn)
{
  BOOST_CHECK_THROW( PluginFrame( "c\nc" ), PluginFrameException );
  PluginFrame f;
  BOOST_CHECK_THROW( f.addHeader( "c\nc" ), PluginFrameException );
  BOOST_CHECK_THROW( f.addHeader( "c:c" ), PluginFrameException );
  BOOST_CHECK_THROW( f.addHeader( "cc", "c\nc" ), PluginFrameException );

  BOOST_CHECK_THROW( doParse( "c" ), PluginFrameException );			// no NL after command
  BOOST_CHECK_THROW( doParse( "c\n" ), PluginFrameException );			// no NL after header
  doParse( "c\n\n" ); 		// valid empy header and body
  BOOST_CHECK_THROW( doParse( "c\nh:v\nbody" ), PluginFrameException );		// no NL after header
  doParse( "c\nh:v\n\nbody" );	// valid
  BOOST_CHECK_THROW( doParse( "c\nhv\n\nbody" ), PluginFrameException );	// no : in header
}

BOOST_AUTO_TEST_CASE(PluginScriptTest)
{
  PluginScript scr;
  BOOST_CHECK_EQUAL( scr.isOpen(), false );
  BOOST_CHECK_EQUAL( scr.getPid(), PluginScript::NotConnected );
  BOOST_CHECK_EQUAL( scr.script(), "" );

  BOOST_CHECK_THROW( scr.open( "bla" ), PluginScriptException );	// script does not exist
  BOOST_CHECK_EQUAL( scr.isOpen(), false );				// stay closed
  BOOST_CHECK_EQUAL( scr.getPid(), PluginScript::NotConnected );
  BOOST_CHECK_EQUAL( scr.script(), "" );

  scr.open( "/bin/cat" );
  BOOST_CHECK_EQUAL( scr.isOpen(), true );
  BOOST_CHECK_EQUAL( (scr.getPid() != PluginScript::NotConnected ), true );
  BOOST_CHECK_EQUAL( scr.script(), "/bin/cat" );			// set after successfull open

  BOOST_CHECK_THROW( scr.open( "/bin/ls" ), PluginScriptException );	// already open
  BOOST_CHECK_EQUAL( scr.isOpen(), true );				// stay with "/bin/cat"
  BOOST_CHECK_EQUAL( (scr.getPid() != PluginScript::NotConnected ), true );
  BOOST_CHECK_EQUAL( scr.script(), "/bin/cat" );

  PluginFrame f;
  scr.send( f );
  PluginFrame r( scr.receive() );
  BOOST_CHECK_EQUAL( f, r );

  f.setCommand( "CMD" );
  f.addHeader( "a","value" );
  f.setBody( "foo" );
  scr.send( f );
  r = scr.receive();
  BOOST_CHECK_EQUAL( f, r );

  scr.close();
  BOOST_CHECK_EQUAL( scr.isOpen(), false );
  BOOST_CHECK_EQUAL( scr.getPid(), PluginScript::NotConnected );
  BOOST_CHECK_EQUAL( scr.script(), "/bin/cat" );			// not reset by close, may be reused by open()

  scr.close();								// no exception on dupl. close.
}

BOOST_AUTO_TEST_CASE(PluginScriptSend)
{
  PluginFrame f( "a" );
  f.setBody( std::string( 1020, '0' ) );

  PluginScript scr( "/bin/cat" );
  BOOST_CHECK_THROW( scr.send( f ), PluginScriptNotConnected );

  scr.open();
  BOOST_CHECK_THROW( do { scr.send( f ); } while ( true ), PluginScriptSendTimeout );

  ::kill( scr.getPid(), SIGKILL);
  BOOST_CHECK_THROW( scr.send( f ), PluginScriptDiedUnexpectedly );
}

BOOST_AUTO_TEST_CASE(PluginScriptReceive)
{
  PluginFrame f( "a" );
  f.setBody( std::string( 1020, '0' ) );

  PluginScript scr( "/bin/cat" );
  scr.open();
  BOOST_CHECK_THROW( scr.receive(), PluginScriptReceiveTimeout );

  ::kill( scr.getPid(), SIGKILL);
  BOOST_CHECK_THROW(  scr.receive(), PluginScriptDiedUnexpectedly );
}
