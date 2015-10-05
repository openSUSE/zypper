#include <iostream>
#include <list>
#include <map>
#include <set>

#include <zypp/base/LogTools.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/Exception.h>

#include <zypp/parser/xml/Reader.h>
//#include <zypp/parser/xml/ParseDef.h>
//#include <zypp/parser/xml/ParseDefConsume.h>

using namespace std;
using namespace zypp;

#include <zypp/base/Measure.h>
using zypp::debug::Measure;

///////////////////////////////////////////////////////////////////

/** Helper to detect an objects type. */
template<class TCl> void ti( const TCl & c )
{
  SEC << __PRETTY_FUNCTION__ << endl;
}

bool noop( xml::Reader & reader_r )
{
  return true;
}
struct Noop
{
  bool operator()( xml::Reader & reader_r ) const
  { return true; }
};

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

bool consume( xml::Reader & reader_r )
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

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  --argc, ++argv;
  INT << "===[START]==========================================" << endl;

  bool verbose( true );
  Pathname input( "test.xml" );
  xml::Reader::ProcessNode consumer( consume );

  if ( argc && !strcmp( *argv, "-q" ) )
  {
    --argc, ++argv;
    verbose = false;
  }

  {
    Measure m( "Parse all" );
    for ( ; argc; --argc, ++argv )
    {
      input = *argv;

      try {
	Measure m( input.basename() );
// 	zypp::base::LogControl::TmpLineWriter shutUp;
	xml::Reader reader( input );
	if ( verbose )
	  reader.foreachNodeOrAttribute( consumer );
	else
	  reader.foreachNode( consumer );
      }
      catch ( const Exception & exp )
      {
	INT << exp << endl << exp.historyAsString();
      }
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
