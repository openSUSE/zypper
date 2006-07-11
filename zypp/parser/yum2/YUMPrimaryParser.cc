/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#include "zypp/parser/yum2/YUMPrimaryParser.h"
#include "zypp/data/ResolvableData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace parser
{ /////////////////////////////////////////////////////////////////
namespace yum2
{ /////////////////////////////////////////////////////////////////

    YUMPrimaryParser::YUMPrimaryParser( zypp::data::ResolvableDataConsumer &consumer )
    {
      _consumer.reset(&consumer);
    }
    
    void YUMPrimaryParser::startElement(const std::string name, const xmlChar **atts)
    {
      if ( name == "package" )
        _package.reset(new zypp::data::Package());
    }
  
    void YUMPrimaryParser::characters(const xmlChar *ch, int len)
    {
      //MIL << "append buffer :" << len << " [" << ( (len < 20) ? string((const char *)(ch), len) : "" ) << "]" << std::endl;
      _buffer.append( (const char *)ch, len);
    }
  
    void YUMPrimaryParser::endElement(const std::string name)
    {
      if ( name == "name" )
        _package->name = _buffer;
      if ( name == "arch" )
        _package->arch = Arch(_buffer);
  
      if ( name == "package" )
      {
        _consumer->consumePackage(*_package);
      }
  
      _buffer.clear();
    }
 

} // namespace yum2
} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////

