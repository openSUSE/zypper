#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/Measure.h>
#include <zypp/SourceFactory.h>
#include <zypp/Source.h>
#include <zypp/Product.h>
#include <zypp/ResStore.h>
#include <zypp/ResObject.h>
#include <zypp/pool/PoolStats.h>
#include <zypp/KeyRing.h>
#include <zypp/Date.h>
#include <zypp/SourceManager.h>

using namespace std;
using namespace zypp;

static bool verbose = false;
static bool debug_flag   = false;

#define LOG (debug_flag ? USR : cout)

struct KeyRingReceiver : public callback::ReceiveReport<KeyRingReport>
{
  KeyRingReceiver()
  {
    connect();
  }

  virtual bool askUserToAcceptUnsignedFile( const std::string & file )
  {
    LOG << "===[UnsignedFile " << file << "]" << endl;
    return true;
  }
  virtual bool askUserToAcceptUnknownKey( const std::string &file,
                                          const std::string &id )
  {
    LOG << "===[UnknownKey " << id << "]" << endl;
    return true;
  }
  virtual bool askUserToTrustKey( const PublicKey &key)
  {
    LOG << "===[TrustKey" << key << "]" << endl;
    return true;
  }
  virtual bool askUserToImportKey( const PublicKey &key)
  {
    LOG << "===[ImportKey " << key << "]" << endl;
    return true;
  }
  virtual bool askUserToAcceptVerificationFailed( const std::string &file,
                                                  const PublicKey &key )
  {
    LOG << "===[VerificationFailed " << file << " " << key << "]" << endl;
    return true;
  }
};

struct ResStoreStats : public pool::PoolStats
{
  void operator()( const ResObject::constPtr & obj )
  {
    if ( isKind<Product>( obj ) )
      {
        LOG << obj << endl;
      }
    pool::PoolStats::operator()( obj );
  }
};

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "" );
  INT << "===[START]==========================================" << endl;
  --argc;
  ++argv;

  if ( ! argc )
    {
      LOG << "Usage: ScanSource [options] url [[options] url...]" << endl;
      LOG << "  Display summary of Sources found at 'url'. " << endl;
      LOG << "  " << endl;
      LOG << "  " << endl;
      LOG << "  options:" << endl;
      LOG << "  +/-l    enable/disable detailed listing of Source content" << endl;
      LOG << "  +/-d    enable/disable debug output" << endl;
      return 0;
    }

  KeyRingReceiver accept;

  for ( ; argc; --argc, ++argv )
    {
      if ( *argv == string("+l") )
        {
          verbose = true;
          continue;
        }
      if ( *argv == string("-l") )
        {
          verbose = false;
          continue;
        }
      if ( *argv == string("+d") )
        {
          zypp::base::LogControl::instance().logfile( "-" );
          debug_flag = true;
          continue;
        }
      if ( *argv == string("-d") )
        {
          zypp::base::LogControl::instance().logfile( "" );
          debug_flag = false;
          continue;
        }

      LOG << "====================================================" << endl;
      LOG << "===Search Source at Url(" << *argv << ")..." << endl;
      Source_Ref src;
      try
        {
          debug::Measure m( "Create" );
          Url url(*argv);
          try
            {
              src = SourceFactory().createFrom( url, "/", Date::now().asSeconds() );
            }
          catch ( const source::SourceUnknownTypeException & )
            {
              src = SourceFactory().createFrom( "Plaindir", url, "/", Date::now().asSeconds(), "", false, true );
            }
            m.elapsed();
            //LOG << m.asString() << endl;
        }
      catch ( const Exception & except_r )
        {
          LOG << "***Failed: " << except_r << endl;
          continue;
        }
      LOG << "type:           " << src.type() << endl;
      LOG << "numberOfMedia:  " << src.numberOfMedia() << endl;
      LOG << "alias:          " << src.alias() << endl;
      LOG << "vendor:         " << src.vendor() << endl;
      LOG << "unique_id:      " << src.unique_id() << endl;
      LOG << "baseSource:     " << src.baseSource() << endl;
      LOG << "autorefresh:    " << src.autorefresh() << endl;
      LOG << "publicKeys:     " << src.publicKeys() << endl;

      LOG << "===Parse content..." << endl;
      try
      {
        debug::Measure m( "Parse" );
        src.resolvables();
        m.elapsed();
        //LOG << m.asString() << endl;
      }
      catch ( const Exception & except_r )
      {
        LOG << "***Failed: " << except_r << endl;
        continue;
      }
      LOG << for_each( src.resolvables().begin(), src.resolvables().end(),
                       ResStoreStats() ) << endl;
      if ( verbose )
      {
        dumpRange( LOG, src.resolvables().begin(), src.resolvables().end() ) << endl;
      }
#define TestKind Product

      for (ResStore::const_iterator it = src.resolvables().begin(); it != src.resolvables().end(); ++it)
      {
        if ( isKind<TestKind>(*it) )
        {
          zypp::TestKind::constPtr res = asKind<TestKind>( *it );
          cout << res->name() << " | " << res->edition() << std::endl;
          cout << res->distributionName() << " | " << res->distributionEdition() << std::endl;
        }
      }
      
      //SourceManager::sourceManager()->addSource( src );
      //SourceManager::sourceManager()->store( "/", true );
    }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

