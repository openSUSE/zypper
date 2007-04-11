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

bool dump( xml::Reader & reader_r )
{
  MIL << *reader_r << endl;
}

///////////////////////////////////////////////////////////////////

bool dumpNode( xml::Reader & reader_r )
{
  switch ( reader_r->nodeType() )
  {
    case XML_READER_TYPE_ELEMENT:
      MIL << *reader_r << endl;
      for ( int i = 0; i < reader_r->attributeCount(); ++i )
      {
	MIL << " attr no " << i << " '" << reader_r->getAttributeNo( i ) << "'" << endl;
      }
      break;

    case XML_READER_TYPE_ATTRIBUTE:
      WAR << *reader_r << endl;
      break;

    case XML_READER_TYPE_TEXT:
    case XML_READER_TYPE_CDATA:
      DBG << *reader_r << endl;
      break;

    default:
      //ERR << *reader_r << endl;
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
#define _show(x) DBG << #x << " = " << reader_r->getAttribute( #x ) << endl
      _show( rel );
      _show( ver );
      _show( epoch );
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
      repodata /= "primary.xml";
    }

  if ( 1 )
    {
      Measure m( "Parse" );
      xml::Reader reader( repodata );

      switch ( 3 )
      {
	case 1:
	  reader.foreachNode( dumpNode );
	  break;
	case 2:
	  reader.foreachNodeOrAttribute( dumpNode );
	  break;
	case 3:
	  reader.foreachNode( dumpEd );
	  break;

	default:
	  WAR << "NOP" << endl;
	  break;
      }
    }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

/*
<?xml version="1.0" encoding="UTF-8"?>
<metadata xmlns="http://linux.duke.edu/metadata/common" xmlns:rpm="http://linux.duke.edu/metadata/rpm" packages="23230">
<package type="rpm">
<name>fam-devel</name>
<arch>ppc</arch>
<version epoch="0" ver="2.6.10" rel="141"/>
<checksum type="sha" pkgid="YES">59d6a65cdadd911fe8ceee87740a54305b2ab053</checksum>
<summary>Include Files and Libraries Mandatory for Development</summary>
<description>Fam is a file alteration monitoring service. This means that you can

foreachNode( dumpNode )
=======================
START MEASURE(Parse)
0:ELEMENT <metadata>  [attr 3]
 attr no 0 'http://linux.duke.edu/metadata/common'
 attr no 1 'http://linux.duke.edu/metadata/rpm'
 attr no 2 '23230'
1: ELEMENT <package>  [attr 1]
 attr no 0 'rpm'
2:  ELEMENT <name>  [noattr]
3:   TEXT <#text>  [noattr] {fam-devel}
2:  ELEMENT <arch>  [noattr]
3:   TEXT <#text>  [noattr] {ppc}
2:  ELEMENT <version>  [attr 3|empty]
 attr no 0 '0'
 attr no 1 '2.6.10'
 attr no 2 '141'
2:  ELEMENT <checksum>  [attr 2]
 attr no 0 'sha'
 attr no 1 'YES'
3:   TEXT <#text>  [noattr] {59d6a65cdadd911fe8ceee87740a54305b2ab053}
2:  ELEMENT <summary>  [noattr]
3:   TEXT <#text>  [noattr] {Include Files and Libraries Mandatory for Development}
2:  ELEMENT <description>  [noattr]
3:   TEXT <#text>  [noattr] {Fam is a file alteration monitoring service. This means that you can

foreachNodeOrAttribute( dumpNode )
==================================
START MEASURE(Parse)
0:ELEMENT <metadata>  [attr 3]
 attr no 0 'http://linux.duke.edu/metadata/common'
 attr no 1 'http://linux.duke.edu/metadata/rpm'
 attr no 2 '23230'
1: ATTRIBUTE <xmlns>  [noattr] {http://linux.duke.edu/metadata/common}
1: ATTRIBUTE <xmlns:rpm>  [noattr] {http://linux.duke.edu/metadata/rpm}
1: ATTRIBUTE <packages>  [noattr] {23230}
1: ELEMENT <package>  [attr 1]
 attr no 0 'rpm'
2:  ATTRIBUTE <type>  [noattr] {rpm}
2:  ELEMENT <name>  [noattr]
3:   TEXT <#text>  [noattr] {fam-devel}
2:  ELEMENT <arch>  [noattr]
3:   TEXT <#text>  [noattr] {ppc}
2:  ELEMENT <version>  [attr 3|empty]
 attr no 0 '0'
 attr no 1 '2.6.10'
 attr no 2 '141'
3:   ATTRIBUTE <epoch>  [noattr] {0}
3:   ATTRIBUTE <ver>  [noattr] {2.6.10}
3:   ATTRIBUTE <rel>  [noattr] {141}
2:  ELEMENT <checksum>  [attr 2]
 attr no 0 'sha'
 attr no 1 'YES'
3:   ATTRIBUTE <type>  [noattr] {sha}
3:   ATTRIBUTE <pkgid>  [noattr] {YES}
3:   TEXT <#text>  [noattr] {59d6a65cdadd911fe8ceee87740a54305b2ab053}
3:   ATTRIBUTE <type>  [noattr] {sha}
3:   ATTRIBUTE <pkgid>  [noattr] {YES}
2:  ELEMENT <summary>  [noattr]
3:   TEXT <#text>  [noattr] {Include Files and Libraries Mandatory for Development}
2:  ELEMENT <description>  [noattr]
3:   TEXT <#text>  [noattr] {Fam is a file alteration monitoring service. This means that you can

*/
