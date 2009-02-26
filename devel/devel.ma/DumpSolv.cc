#include "Tools.h"
#include <zypp/ResObjects.h>
#include <zypp/ProgressData.h>
#include <zypp/sat/WhatObsoletes.h>
#include "zypp/pool/GetResolvablesToInsDel.h"

///////////////////////////////////////////////////////////////////
#include "zypp/parser/xml/ParseDef.h"
#include "zypp/parser/xml/ParseDefConsume.h"
#include "zypp/parser/xml/Reader.h"
namespace zypp
{
  namespace xml
  {
    namespace parsedefassign
    {
      template <class _Type> struct Assigner;

      /** Common interface to all Assigner. */
      template <>
          struct Assigner<void>
      {
        virtual ~Assigner()
        {}
        virtual void assign( const char * text_r )
        {}
      };

      /** Assigner assigns text to types constructible from \c char*. */
      template <class _Type>
          struct Assigner : public Assigner<void>
      {
        Assigner(_Type & value_r )
          : _value( &value_r )
        {}

        virtual void assign( const char * text_r )
        { *_value = _Type( text_r ); }

        private:
          _Type * _value;
      };

      /** \name Assigner specialisation for numeric and boolean values.
       *  \relates Assigner
       */
      //@{
      template <>
          inline void Assigner<short>::assign( const char * text_r ) { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<int>::assign( const char * text_r ) { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<long>::assign( const char * text_r ) { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<long long>::assign( const char * text_r ) { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<unsigned short>::assign( const char * text_r ) { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<unsigned>::assign( const char * text_r ) { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<unsigned long>::assign( const char * text_r )  { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<unsigned long long>::assign( const char * text_r ) { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<bool>::assign( const char * text_r ) { str::strToBoolNodefault( text_r, *_value ); }
      //@}
    }

    struct ParseDefAssignConsumer : public ParseDefConsume
    {
      virtual void start( const Node & node_r )
      {}

      virtual void text( const Node & node_r )
      {}
    };

    struct ParseDefAssign
    {
      template <class _Type>
          ParseDefAssign( _Type & value_r )
      {}

      operator const shared_ptr<ParseDefAssignConsumer> &() const
      { return _ptr; }

      private:
        shared_ptr<ParseDefAssignConsumer> _ptr;
    };

  }

  /** Parse \c input_r and store data in \c data_r.
   *
   * \c _Data must be defaultconstructible and assignable.
   *
   * \c _Data::RootNode must be a \ref xml::ParseDef constructible
   * from \c _Data&.
   *
   * \throws ParseDefException on parse errors.
   *
   * To parse a xml file like this:
   * \code
   * <test><setup>value</setup></test>
   * \endcode
   *
   * You need something like this:
   * \code
   *  struct XmlData
   *  {
   *    std::string value;

   *   public:
   *      // Convenience parsing to *this.
   *      void parse( const Pathname & path_r )
   *      { rnParse( path_r, *this ); }

   *    public:
   *      // Parser description
   *      struct RootNode : public xml::ParseDef, public xml::ParseDefConsume
   *      {
   *        RootNode( XmlData & data )
   *          : ParseDef( "test", MANDTAORY )
   *        {
   *          (*this)
   *              ("setup", MANDTAORY, xml::parseDefAssignText( data.value ) )
   *              ;
   *        }
   *      };
   *  };
   *
   *  XmlData xmlData;
   *  xmlData.parse( "/tmp/mytest.xml" );
   * \endcode
   */
  template<class _Data>
  inline void rnParse( const InputStream & input_r, _Data & data_r )
  {
    typedef typename _Data::RootNode RootNode;
    MIL << "+++ " << input_r << endl;
    _Data pdata;
    try
    {
      xml::Reader reader( input_r );
      RootNode rootNode( pdata );
      rootNode.take( reader );
    }
    catch ( const Exception & err )
    {
      // parse error
      ERR << err << endl;
      ERR << "--- " << input_r << endl;
      ZYPP_RETHROW( err );
    }
    MIL << "--- " << input_r << endl;
    data_r = pdata;
  }

  struct SolverTestXml
  {
    public:
      struct Chanel
      {
        Chanel() :  priority( RepoInfo::defaultPriority() ) {}
        std::string file;
        std::string alias;
        unsigned    priority;
      };

      Arch              systemArch;
      Locale            systemLocale;
      std::string       systemFile;
      std::list<Chanel> chanelList;

    public:
      /** Convenience parsing to \c this, appending \c "solver-test.xml" to directories. */
      void parse( const Pathname & path_r )
      { rnParse( (PathInfo(path_r).isDir() ? path_r/"solver-test.xml" : path_r ), *this ); }

    public:
      struct RootNode : public xml::ParseDef, public xml::ParseDefConsume
      {
        RootNode( SolverTestXml & data )
          : ParseDef( "test", MANDTAORY, xml::parseDefAssignText( data.systemFile ) )
        {
          (*this)
            ("setup",        MANDTAORY, xml::parseDefAssign( data.systemArch ) )
            ;
        }
      };
  };
}
///////////////////////////////////////////////////////////////////

static std::string appname( __FILE__ );

void message( const std::string & msg_r )
{
  cerr << "*** " << msg_r << endl;
}

int usage( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
  {
    cerr << endl;
    message( msg_r );
    cerr << endl;
  }
  cerr << "Usage: " << appname << " TESTCASE" << endl;
  cerr << "  Load and process testcase." << endl;
  return exit_r;
}

///////////////////////////////////////////////////////////////////

bool progressReceiver( const ProgressData & v )
{
  DBG << "...->" << v << endl;
  return true;
}

///////////////////////////////////////////////////////////////////

bool upgrade()
{
  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    UpgradeStatistics u;
    rres = getZYpp()->resolver()->doUpgrade( u );
  }
  if ( ! rres )
  {
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  return true;
}

bool solve()
{
  static unsigned run = 0;
  USR << "Solve " << run++ << endl;
  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  appname = Pathname::basename( argv[0] );
  --argc;
  ++argv;

  ///////////////////////////////////////////////////////////////////

  Pathname mtest( "/suse/ma/BUGS/472099/zypper.solverTestCase" );
  Arch     march( Arch_i686 );

  while ( argc )
  {
    --argc;
    ++argv;
  }

  if ( mtest.empty() )
  {
    return usage( "Missing Testcase", 102 );
  }

  ///////////////////////////////////////////////////////////////////

  //xml::ParseDef::_debug = true;
  SolverTestXml solvertest;
  solvertest.parse( mtest );
  USR << solvertest.systemFile << endl;
  USR << solvertest.systemArch << endl;

  INT << "===[END]============================================" << endl << endl;
  return 0;

  TestSetup test( march );
  ResPool   pool( test.pool() );
  sat::Pool satpool( test.satpool() );

  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    test.loadTarget();
    test.loadTestcaseRepos( mtest ); // <<< repos
  }
  test.poolProxy().saveState();

  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    upgrade();
  }
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::TmpLineWriter shutUp;
  return 0;
}
