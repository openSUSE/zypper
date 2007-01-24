#include "Tools.h"
#include <libxml/xmlreader.h>
#include <libxml/xmlerror.h>
#include <boost/call_traits.hpp>

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <set>

#include <zypp/base/Hash.h>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>

#include "zypp/parser/xml/Reader.h"

using namespace std;
using namespace zypp;

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
#include <zypp/CapFactory.h>

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

void fillSet( const std::list<std::string> & list_r )
{
  Measure m( "Set " );
  std::set<std::string> s( list_r.begin(), list_r.end() );
  MIL << s.size() << endl;
  m.stop();
}

void fillHashSet( const std::list<std::string> & list_r )
{
  Measure m( "HashSet " );
  hash_set<std::string> s( list_r.begin(), list_r.end() );
  MIL << s.size() << endl;
  m.stop();
}

void fillMap( const std::list<std::string> & list_r )
{
  Measure m( "Map " );
  std::map<std::string,std::string> s;
  for ( std::list<std::string>::const_iterator it = list_r.begin(); it != list_r.end(); ++it )
    {
      s[*it] = *it;
    }
  MIL << s.size() << endl;
  m.stop();
}

void fillHashMap( const std::list<std::string> & list_r )
{
  Measure m( "HashMap " );
  hash_map<std::string,std::string> s;
  for ( std::list<std::string>::const_iterator it = list_r.begin(); it != list_r.end(); ++it )
    {
      s[*it] = *it;
    }
  MIL << s.size() << endl;
  m.stop();
}

void lfillStd( const std::list<std::string> & list_r )
{
  Measure x( "lfillStd " );
  std::set<std::string> s;
  std::map<std::string,std::string> m;

  for ( std::list<std::string>::const_iterator it = list_r.begin(); it != list_r.end(); ++it )
    {
      if ( m[*it].empty() )
        {
          m[*it] = *it;
          s.insert( *it );
        }
    }
  MIL << m.size() << " " << s.size() << endl;
  x.stop();
}

void parseCaps( const std::list<std::string> & list_r )
{
  Measure x( "parseCaps" );
  for ( std::list<std::string>::const_iterator it = list_r.begin(); it != list_r.end(); ++it )
    {
      CapFactory().parse( ResTraits<Package>::kind, *it );
    }
  x.stop();
}

void parseCapsAndInsert( const std::list<std::string> & list_r )
{
  Measure x( "parseCapsAndInsert" );
  CapSet s;
  for ( std::list<std::string>::const_iterator it = list_r.begin(); it != list_r.end(); ++it )
    {
      s.insert(  CapFactory().parse( ResTraits<Package>::kind, *it ) );
    }
  x.stop();
}


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
      repodata /= "primary.xml";
    }

  std::list<std::string> depstrings;

  Measure m( "Parse" );
  xml::Reader reader( repodata );
  reader.foreachNodeOrAttribute( dumpDeps( std::back_inserter(depstrings) ) );
  m.stop();

  MIL << "raw depstrings " << depstrings.size() << endl;
  parseCaps( depstrings );
  parseCapsAndInsert( depstrings );

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

