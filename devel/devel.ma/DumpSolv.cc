#include "Tools.h"

#include <zypp/ResObjects.h>
#include <zypp/ProgressData.h>
#include <zypp/sat/WhatObsoletes.h>
#include "zypp/pool/GetResolvablesToInsDel.h"

///////////////////////////////////////////////////////////////////
#if 0
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
#endif
///////////////////////////////////////////////////////////////////

static std::string appname( __FILE__ );

#define OUT   USR
#define HEADL SEC << "===> "

inline std::ostream & errmessage( const std::string & msg_r = std::string() )
{
  cerr << "*** ";
  if ( ! msg_r.empty() )
    cerr << msg_r << endl;
  return cerr;
}

int usage( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
  {
    cerr << endl;
    errmessage( msg_r );
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

struct ArgList
{
  typedef std::vector<std::string>::const_iterator const_iterator;

  ArgList()
  {}

  ArgList( const std::string & line_r )
  { str::splitEscaped( line_r, std::back_inserter(_argv) ); }

  const_iterator begin() const { const_iterator ret =_argv.begin(); for ( unsigned i = _carg; i; --i ) ++ret; return ret; }
  const_iterator end()   const { return _argv.end(); }

  void     clear()       { _argv.clear(); _carg = 0; }
  bool     empty() const { return _argv.size() == _carg; }
  unsigned size()  const { return _argv.size() - _carg; }

  std::string &       operator[]( int idx )       { return _argv[_carg+idx]; }
  const std::string & operator[]( int idx ) const { return _argv[_carg+idx]; }

  std::string at( int idx ) const { return _carg+idx < _argv.size() ? _argv[_carg+idx] : std::string(); }

  unsigned carg() const { return _carg; }
  void poparg( int cnt = 1 ) { _carg = arange( _carg + cnt ); }

  public:
    std::vector<std::string> &       get()       { return _argv; }
    const std::vector<std::string> & get() const { return _argv; }
 private:
   unsigned arange( int idx ) const { return idx < 0 ? 0 : std::min( unsigned(idx), _argv.size() ); }
 private:
    DefaultIntegral<unsigned,0> _carg;
    std::vector<std::string> _argv;
};

std::ostream & operator<<( std::ostream & str, const ArgList & obj )
{
  for_( it, 0, obj.get().size() )
  {
    str << ( it == obj.carg() ? " | " : " " ) << obj.get()[it];
  }
  return str;
}

///////////////////////////////////////////////////////////////////
#define DELGATE(N,F) if ( argv.at(0) == #N ) { argv.poparg(); F( argv ); return; }
///////////////////////////////////////////////////////////////////

void listReposCmd( ArgList & argv )
{
  errmessage() << "Not inplemented: " << argv << endl;
}

void listIdent( IdString ident )
{
  HEADL << "list " << ident << endl;

  ui::Selectable::Ptr sel( ui::Selectable::get( ident ) );
  OUT <<  dump(sel) << endl;

  sat::WhatProvides qp( (Capability( ident.id() )) );
  OUT << "Provided by " << qp << endl;


}


void listCmd( ArgList & argv )
{
  DELGATE( repos, listReposCmd );

  for_( it, argv.begin(), argv.end() )
  {
    listIdent( IdString(*it) );
  }
}

///////////////////////////////////////////////////////////////////

bool gocmd( ArgList & argv )
{
  switch ( argv[0][0] )
  {
#define DOCMD(n) if ( argv[0] == #n ) { argv.poparg(); n##Cmd( argv ); return true; }
    case 'e':
      if ( argv[0] == "exit" ) { return false; }
      break;

    case 'l':
      DOCMD( list );
      break;
#undef DOCMD
  }
  // no command fall back to list
  listCmd( argv );
  return true;
}

void goprompt()
{
  std::cin.tie( &std::cout );
  ArgList argv;
  do {
    argv.clear();
    std::cout << "Hallo : ";
    str::splitEscaped( iostr::getline( std::cin ), std::back_inserter(argv.get()) );
  } while ( argv.empty() || gocmd( argv ) );
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

  if ( !argc )
    return usage();

  Pathname mtest( *argv );
  --argc;
  ++argv;

  if ( ! PathInfo( mtest / "solver-test.xml" ).isFile() )
  {
    return usage( "No testcase at " + mtest.asString() );
  }

  ///////////////////////////////////////////////////////////////////
  TestSetup test;
  test.loadTestcaseRepos( mtest ); // <<< repos
#define GOCMD(c) { ArgList argv( #c ); gocmd( argv ); }
  GOCMD( libsndfile );
  GOCMD( libsndfile1 );

  goprompt();

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::TmpLineWriter shutUp;
  return 0;
}
