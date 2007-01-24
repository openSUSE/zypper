#include "Tools.h"

extern "C"
{
#include <libxml/xmlreader.h>
#include <libxml/xmlerror.h>
}
#include <iostream>
#include <fstream>
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

#include "zypp/parser/xml/Reader.h"

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

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  --argc;
  ++argv;

  Pathname repodata;
  if ( argc )
    {
      repodata = *argv;
    }
  else
    {
      repodata = "/Local/FACTORY/repodata";
      //repodata = "/Local/PATCHES/repodata";
      repodata /= "primary.xml";
    }

  if ( 1 )
    {
      Measure m( "Parse" );
      xml::Reader reader( repodata );
      //reader.foreachNodeOrAttribute( accNode );
      reader.foreachNodeOrAttribute( accNode );
    }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

