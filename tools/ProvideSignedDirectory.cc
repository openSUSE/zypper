#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/ExternalProgram.h>

#include <zypp/MediaSetAccess.h>
#include <zypp/KeyRing.h>
#include <zypp/Fetcher.h>
#include <zypp/TmpPath.h>


using std::endl;

int main ( int argc, const char * argv[] )
try {
  --argc;
  ++argv;
  //zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ///////////////////////////////////////////////////////////////////

  zypp::Url		oRemoteUrl( argv[0] ); //"http://download.opensuse.org/distribution/openSUSE-current/repo/oss" );
  std::string 		oRemoteDir( argv[1] ); //"/suse/setup/slide" );
  const bool		oRecursive( true );
  zypp::Pathname	oLocalDir( "" );

  zypp::scoped_ptr<zypp::filesystem::TmpDir> tmpdir;
  if ( oLocalDir.empty() )
  {
    tmpdir.reset( new zypp::filesystem::TmpDir );
    oLocalDir = tmpdir->path();
  }

  zypp::Fetcher fetcher;
  fetcher.setOptions( zypp::Fetcher::AutoAddIndexes );
  fetcher.enqueueDir( zypp::OnMediaLocation( oRemoteDir ), oRecursive );

  zypp::KeyRing::setDefaultAccept( zypp::KeyRing::TRUST_KEY_TEMPORARILY );
  zypp::MediaSetAccess media( oRemoteUrl, "/" );
  fetcher.start( oLocalDir, media );

  zypp::ExternalProgram( "find "+(oLocalDir/oRemoteDir).asString()+" -ls" ) >> std::cout;

  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
catch ( const zypp::Exception & exp )
{
  INT << exp << endl << exp.historyAsString();
}
catch (...)
{}
