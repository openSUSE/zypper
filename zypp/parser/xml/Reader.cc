/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/Reader.cc
 *
*/
extern "C"
{
#include <libxml/xmlreader.h>
#include <libxml/xmlerror.h>
}

#include <iostream>

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Exception.h"

#include "zypp/parser/xml/Reader.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      int ioread( void * context_r, char * buffer_r, int bufferLen_r )
      {
        if ( context_r && buffer_r )
          {
            return reinterpret_cast<InputStream *>(context_r)
                   ->stream().read( buffer_r, bufferLen_r ).gcount();
          }
        INT << "XML parser error: null pointer check failed " << context_r << ' ' << (void *)buffer_r << endl;
        return -1;
      }

      int ioclose( void * /*context_r*/ )
      { return 0; }

      void structuredErrorFunc( void * userData, xmlErrorPtr error )
      {
        if ( error )
          {
#define X(m) SEC << " " << #m << "\t" << error->m << endl
#define XS(m) SEC << " " << #m << "\t" << (error->m?error->m:"NA") << endl
            X(domain);
            X(code);
            XS(message);
            X(level);
            XS(file);
            X(line);
            XS(str1);
            XS(str2);
            XS(str3);
            X(int1);
            X(int2);
            X(ctxt);
            X(node);
#undef X
#undef XS
          }
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Reader::Reader
    //	METHOD TYPE : Constructor
    //
    Reader::Reader( const InputStream & stream_r,
                    const Validate & validate_r )
    : _stream( stream_r )
    , _reader( xmlReaderForIO( ioread, ioclose, &_stream,
                               stream_r.path().asString().c_str(), "utf-8", XML_PARSE_PEDANTIC ) )
    , _node( _reader )
    {
      MIL << "Start Parsing " << _stream << endl;
      if ( ! _reader || ! stream_r.stream().good() )
        ZYPP_THROW( Exception( "Bad input stream" ) );
      // set error handler
      xmlTextReaderSetStructuredErrorHandler( _reader, structuredErrorFunc, NULL );
      // TODO: set validation

      // advance to 1st node
      nextNode();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Reader::~Reader
    //	METHOD TYPE : Destructor
    //
    Reader::~Reader()
    {
      if ( _reader )
        {
          xmlFreeTextReader( _reader );
        }
      MIL << "Done Parsing " << _stream << endl;
    }

    XmlString Reader::nodeText()
    {
      if ( ! _node.isEmptyElement() )
      {
        if ( nextNode() )
        {
          if ( _node.nodeType() == XML_READER_TYPE_TEXT )
          {
            return _node.value();
          }
        }
      }
      return XmlString();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Reader::nextNode
    //	METHOD TYPE : bool
    //
    bool Reader::nextNode()
    {
      int ret = xmlTextReaderRead( _reader );
      if ( ret == 1 )
        {
          return true;
        }
      xmlTextReaderClose( _reader );
      if ( ret != 0 )
        {
          ZYPP_THROW( Exception( "Parse error" ) );
        }
      return false;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Reader::nextNodeAttribute
    //	METHOD TYPE : bool
    //
    bool Reader::nextNodeAttribute()
    {
      int ret = xmlTextReaderMoveToNextAttribute( _reader );
      if ( ret == 1 )
        {
          return true;
        }
      if ( ret != 0 )
        {
          ZYPP_THROW( Exception( "Parse error" ) );
        }
      return false;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Reader::close
    //	METHOD TYPE : void
    //
    void Reader::close()
    {
      if ( _reader )
        {
          xmlTextReaderClose( _reader );
        }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Reader::seekToNode
    //	METHOD TYPE : bool
    //
    bool Reader::seekToNode( int depth_r, const std::string & name_r )
    {
      do
        {
          if ( _node.depth() == depth_r
               && _node.name() == name_r
               && _node.nodeType() == XML_READER_TYPE_ELEMENT )
            {
              break;
            }
        } while( nextNode() );

      return ! atEnd();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Reader::seekToEndNode
    //	METHOD TYPE : bool
    //
    bool Reader::seekToEndNode( int depth_r, const std::string & name_r )
    {
      // Empty element has no separate end node: <node/>
      do
        {
          if ( _node.depth() == depth_r
               && _node.name() == name_r
               && ( _node.nodeType() == XML_READER_TYPE_END_ELEMENT
                    || ( _node.nodeType() == XML_READER_TYPE_ELEMENT
                         && _node.isEmptyElement() ) ) )
            {
              break;
            }
        } while( nextNode() );

      return ! atEnd();
    }

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
