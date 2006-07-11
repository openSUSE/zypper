/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_YUMPrimaryParser_H
#define ZYPP_YUMPrimaryParser_H

#include "zypp/parser/SAXParser.h"
#include "zypp/data/ResolvableDataConsumer.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace parser
{ /////////////////////////////////////////////////////////////////
namespace yum2
{ /////////////////////////////////////////////////////////////////

  class YUMPrimaryParser : public parser::SAXParser
  {
    public:

    YUMPrimaryParser( zypp::data::ResolvableDataConsumer &consumer );
    virtual void startElement(const std::string name, const xmlChar **atts);
    virtual void characters(const xmlChar *ch, int len);
    virtual void endElement(const std::string name);
    
    private:
    shared_ptr<zypp::data::Package> _package;
    shared_ptr<zypp::data::ResolvableDataConsumer> _consumer;
    std::string _buffer;
  };


} // namespace yum2
} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUMPrimaryParser_H
