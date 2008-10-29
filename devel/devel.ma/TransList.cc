#include "Tools.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <set>

#include <boost/call_traits.hpp>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>

#include "zypp/base/Exception.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/DefaultIntegral.h"
#include <zypp/base/Function.h>
#include <zypp/base/Iterator.h>
#include <zypp/Pathname.h>
#include <zypp/Edition.h>
#include <zypp/CheckSum.h>
#include <zypp/Date.h>
#include <zypp/Rel.h>
#include <zypp/Bit.h>

#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/xml/ParseDef.h"
#include "zypp/parser/xml/ParseDefConsume.h"

#include "zypp/parser/ProductFileReader.h"

using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

template<class _Cl>
  void ti( const _Cl & c )
  {
    SEC << __PRETTY_FUNCTION__ << endl;
  }

///////////////////////////////////////////////////////////////////

bool nopNode( xml::Reader & reader_r )
{
  return true;
}

///////////////////////////////////////////////////////////////////

bool accNode( xml::Reader & reader_r )
{
  int i;
  xml::XmlString s;
#define X(m) reader_r->m()
      i=X(readState);
      i=X(lineNumber);
      i=X(columnNumber);
      i=X(depth);
      i=X(nodeType);
      s=X(name);
      s=X(prefix);
      s=X(localName);
      i=X(hasAttributes);
      i=X(attributeCount);
      i=X(hasValue);
      s=X(value);
#undef X
      return true;
}

///////////////////////////////////////////////////////////////////

bool dumpNode( xml::Reader & reader_r )
{
  switch ( reader_r->nodeType() )
    {
    case XML_READER_TYPE_ATTRIBUTE:
    case XML_READER_TYPE_TEXT:
    case XML_READER_TYPE_CDATA:
       DBG << *reader_r << endl;
       break;
    case XML_READER_TYPE_ELEMENT:

       MIL << *reader_r << endl;
       break;
    default:
       //WAR << *reader_r << endl;
       break;
    }
  return true;
}

///////////////////////////////////////////////////////////////////

bool dumpEd( xml::Reader & reader_r )
{
  static int num = 5;
  if ( reader_r->nodeType() == XML_READER_TYPE_ELEMENT
       && reader_r->name() == "version" )
    {
      MIL << *reader_r << endl;
      DBG << reader_r->getAttribute( "rel" ) << endl;
      ERR << *reader_r << endl;
      DBG << reader_r->getAttribute( "ver" ) << endl;
      ERR << *reader_r << endl;
      DBG << reader_r->getAttribute( "epoch" ) << endl;
      ERR << *reader_r << endl;
      WAR << Edition( reader_r->getAttribute( "ver" ).asString(),
                      reader_r->getAttribute( "rel" ).asString(),
                      reader_r->getAttribute( "epoch" ).asString() ) << endl;
      --num;
    }
  return num;
}

///////////////////////////////////////////////////////////////////

template<class _OutputIterator>
  struct DumpDeps
  {
    DumpDeps( _OutputIterator result_r )
    : _result( result_r )
    {}

    bool operator()( xml::Reader & reader_r )
    {
      if ( reader_r->nodeType()     == XML_READER_TYPE_ELEMENT
           && reader_r->prefix()    == "rpm"
           && reader_r->localName() == "entry" )
        {
          string n( reader_r->getAttribute( "name" ).asString() );
          Rel op( reader_r->getAttribute( "flags" ).asString() );
          if ( op != Rel::ANY )
            {
              n += " ";
              n += op.asString();
              n += " ";
              n += reader_r->getAttribute( "ver" ).asString();
              n += "-";
              n += reader_r->getAttribute( "rel" ).asString();
            }
          *_result = n;
          ++_result;
        }
      return true;
    }

    _OutputIterator _result;
  };

template<class _OutputIterator>
  DumpDeps<_OutputIterator> dumpDeps( _OutputIterator result_r )
  { return DumpDeps<_OutputIterator>( result_r ); }

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

bool Consumer( const parser::ProductFileData & data_r )
{
  MIL << data_r << endl;
  return true;
}

#include "zypp/base/Functional.h"

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  {
    Measure x( "PARSE" );

    parser::ProductFileData              val;
    std::vector<parser::ProductFileData> result;

    parser::ProductFileReader::scanDir( functor::getFirst( val ),
                                        //functor::getAll( std::back_inserter( result ) ),
                                        sysRoot / "etc/products.d" );

    USR << val << endl;
    USR << result << endl;
  }

#if 0
  bool write = false;
  bool read = true;

  if ( 1 )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;
      getZYpp()->initTarget( sysRoot );
    }
  if ( write )
    {
      ZYpp::LocaleSet lset;
      lset.insert( Locale("gr") );
      getZYpp()->setRequestedLocales( lset );
    }

  ResPool pool( getZYpp()->pool() );
  USR << pool << endl;

  if ( write )
    {
      syscontent::Writer contentW;
      contentW.name( "mycollection" )
              .edition( Edition( "1.0" ) )
              .description( "All the cool stuff..." );
      for_each( pool.begin(), pool.end(),
                bind( &syscontent::Writer::addIf, ref(contentW), _1 ) );

      ofstream s( "mycollection.xml" );
      s << contentW;
    }

  if ( read )
    {
      Measure x( "Parse" );
      std::ifstream input( "mycollection.xml" );
      syscontent::Reader contentR;
      try
        {
          contentR = syscontent::Reader( input );
        }
      catch( const Exception & excpt_r )
        {
          ERR << excpt_r << endl;
        }

      MIL << contentR << endl;

      for_each( contentR.begin(), contentR.end(), Print() );
    }
#endif
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

