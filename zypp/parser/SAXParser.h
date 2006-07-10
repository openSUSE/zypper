/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SAXParser_H
#define ZYPP_SAXParser_H

#include <iosfwd>
#include <string>
#include <vector>
#include <libxml/parser.h>

#include <boost/function.hpp>

#include "zypp/Pathname.h"


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace parser
{ /////////////////////////////////////////////////////////////////

  class SAXParser
  {
    public:

    SAXParser();
    virtual ~SAXParser();

    void parseFile( const Pathname &p);

    virtual void startElement(const std::string name, const xmlChar **atts);
    virtual void endElement(const std::string name);
    virtual void characters(const xmlChar *ch, int len);
  
    static void startElement_receiver(void *ctx, const xmlChar *name, const xmlChar **atts);
    static void endElement_receiver(void *ctx, const xmlChar *name);
    static void characters_receiver (void *data, const xmlChar *ch, int len);
    private:
    xmlSAXHandler _saxHandler;
   
  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SAXParser_H
