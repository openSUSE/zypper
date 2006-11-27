#include <libxml/xmlreader.h>

#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/Function.h>
#include <zypp/base/GzStream.h>
#include <zypp/parser/yum/YUMParser.h>
#include <zypp/Pathname.h>

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;

#include "zypp/parser/yum/YUMParser.h"

///////////////////////////////////////////////////////////////////

template<class _Cl>
  void ti( const _Cl & c )
  {
    SEC << __PRETTY_FUNCTION__ << endl;
  }
///////////////////////////////////////////////////////////////////

template<class _Parser>
  bool consume( const typename _Parser::value_type & node_r )
  {
    //DBG << node_r << endl;
    return true;
  }

template<class _Parser>
  void parseXmlFile( const Pathname & file_r,
                     function<bool(const typename _Parser::value_type &)> consume_r
                     = consume<_Parser> )
  {
    Measure x( "    zparse "+file_r.asString() );
    ifgzstream istr( file_r.asString().c_str() );
    if ( ! istr )
      {
        ZYPP_THROW( Exception( "Bad stream" ) );
      }

    for( _Parser parser( istr, "" ); ! parser.atEnd(); ++parser )
      {
        if ( consume_r && ! consume_r( *parser ) )
          {
            DBG << "abort parseXmlFile " << file_r << endl;
            return;
          }
      }
  }

bool consumeRepomd( const YUMRepomdParser::value_type & node_r )
{
  DBG << node_r << endl;
  return true;
}

void zparse( const Pathname & repodata_r )
{
  Measure x( "ZPARSE" );
  parseXmlFile<YUMRepomdParser>  ( repodata_r / "repomd.xml", consumeRepomd );
  parseXmlFile<YUMPrimaryParser> ( repodata_r / "primary.xml" );
  parseXmlFile<YUMOtherParser>   ( repodata_r / "other.xml" );
  parseXmlFile<YUMFileListParser>( repodata_r / "filelists.xml" );
  //parseXmlFile<YUMPatchesParser> ( repodata_r / "patches.xml" );
}

///////////////////////////////////////////////////////////////////

/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
template<class _ParserValueType>
static void
processNode(xmlTextReaderPtr reader, const _ParserValueType & stp ) {
    const xmlChar *name, *value;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
	name = BAD_CAST "--";

    value = xmlTextReaderConstValue(reader);
    string t;
    if ( value )
      {
        t = (const char *)value;
      }
    return;
    printf("%d %d %s %d %d",
	    xmlTextReaderDepth(reader),
	    xmlTextReaderNodeType(reader),
	    name,
	    xmlTextReaderIsEmptyElement(reader),
	    xmlTextReaderHasValue(reader));
    if (value == NULL)
	printf("\n");
    else {
        if (xmlStrlen(value) > 40)
            printf(" %.40s...\n", value);
        else
	    printf(" %s\n", value);
    }
}


/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse and print information about an XML file.
 */
template<class _Parser>
static void
streamFile(const char *filename) {
    Measure x( string("    lparse ")+filename );
    xmlTextReaderPtr reader;
    int ret;

    typename _Parser::value_type stp;

    reader = xmlReaderForFile(filename, NULL, 0);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            stp = new typename _Parser::value_type::element_type;
            processNode(reader, stp);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
          ZYPP_THROW( Exception( string("Failed to parse ") + filename ) );
        }
    } else {
      ZYPP_THROW( Exception( string("Unable to open ") + filename ) );
    }
}

void lparse( const Pathname & repodata_r )
{
  Measure x( "LPARSE" );
    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    streamFile<YUMRepomdParser>  ( (repodata_r / "repomd.xml").asString().c_str() );
    streamFile<YUMPrimaryParser> ( (repodata_r / "primary.xml").asString().c_str() );
    streamFile<YUMOtherParser>   ( (repodata_r / "other.xml").asString().c_str() );
    streamFile<YUMFileListParser>( (repodata_r / "filelists.xml").asString().c_str() );
    //streamFile<YUMPatchesParser> ( (repodata_r / "patches.xml").asString().c_str() );

    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();

    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Pathname repodata( "/Local/PATCHES/repodata" );
  repodata = "/Local/FACTORY/repodata";
  lparse( repodata );
  zparse( repodata );

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

