/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/WebpinResultFileReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/ws/WebpinResultFileReader.h"
#include "zypp/ws/WebpinResult.h"

using std::endl;
using namespace zypp::xml;
using namespace zypp::ws;

namespace zypp
{ 
namespace parser
{
namespace ws
{
    
class WebpinResultFileReader::Impl
{
public:
    Impl( const Pathname &result_file,
          const ProcessWebpinResult &callback );
    
    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode( Reader & reader_r );
private:
    shared_ptr<WebpinResult> _result;
    ProcessWebpinResult _callback;
};

bool WebpinResultFileReader::Impl::consumeNode(Reader & reader_r)
{
    if ( reader_r->nodeType() == XML_READER_TYPE_ELEMENT )
    {
        // xpath: /packages
        if ( reader_r->name() == "packages" )
        {
            return true;
        }
        
        // xpath: /packages/package (+)
        if ( reader_r->name() == "package" )
        { _result.reset( new WebpinResult() ); }
        
        // xpath: /packages/name
        if ( reader_r->name() == "name" )
        { _result->setName(reader_r.nodeText().asString());}

        // xpath: /packages/version
        if ( reader_r->name() == "version" )
        { _result->setEdition(Edition(reader_r.nodeText().asString())); }
        
        // xpath: /packages/summary
        if ( reader_r->name() == "summary" )
        { _result->setSummary(reader_r.nodeText().asString()); }
        
        // xpath: /packages/repoURL
        if ( reader_r->name() == "repoURL" )
        { _result->setRepositoryUrl(Url(reader_r.nodeText().asString())); }
        
        // xpath: /packages/checksum
        if ( reader_r->name() == "checksum" )
        { _result->setChecksum(CheckSum::sha1(reader_r.nodeText().asString())); }
        // xpath: /packages/distro
        if ( reader_r->name() == "distro" )
        { _result->setDistribution(reader_r.nodeText().asString());}

        return true;
    }
    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
        // xpath: /packages/package (+)
        if ( reader_r->name() == "package" )
        {
            _callback(*_result);
        }
    }

    return true;
}
    

WebpinResultFileReader::Impl::Impl( const Pathname &result_file,
                                    const ProcessWebpinResult &callback )
    : _callback(callback)
{
    Reader reader( result_file );
    MIL << "Reading " << result_file << endl;
    reader.foreachNode( bind( &WebpinResultFileReader::Impl::consumeNode, this, _1 ) );
}
    

WebpinResultFileReader::WebpinResultFileReader( const Pathname & result_file,
                                                const ProcessWebpinResult & callback )
    : _pimpl(new WebpinResultFileReader::Impl(result_file, callback))
{
}
    
WebpinResultFileReader::~WebpinResultFileReader()
{}
    
std::ostream & operator<<( std::ostream & str, const WebpinResultFileReader & obj )
{
    return str;
}


} // namespace ws
} // namespace parser
} // namespace zypp

