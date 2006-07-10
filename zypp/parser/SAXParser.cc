/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include <zypp/parser/SAXParser.h>
#include <zypp/base/Logger.h>

namespace zypp
{
namespace parser
{

static xmlSAXHandler emptySAXHandlerStruct = {
    NULL, /* internalSubset */
    NULL, /* isStandalone */
    NULL, /* hasInternalSubset */
    NULL, /* hasExternalSubset */
    NULL, /* resolveEntity */
    NULL, /* getEntity */
    NULL, /* entityDecl */
    NULL, /* notationDecl */
    NULL, /* attributeDecl */
    NULL, /* elementDecl */
    NULL, /* unparsedEntityDecl */
    NULL, /* setDocumentLocator */
    NULL, /* startDocument */
    NULL, /* endDocument */
    NULL, /* startElement */
    NULL, /* endElement */
    NULL, /* reference */
    NULL, /* characters */
    NULL, /* ignorableWhitespace */
    NULL, /* processingInstruction */
    NULL, /* comment */
    NULL, /* xmlParserWarning */
    NULL, /* xmlParserError */
    NULL, /* xmlParserError */
    NULL, /* getParameterEntity */
    NULL, /* cdataBlock; */
    NULL,  /* externalSubset; */
    1
};

static xmlSAXHandlerPtr emptySAXHandler = &emptySAXHandlerStruct;
extern xmlSAXHandlerPtr debugSAXHandler;

/**
 * startElementDebug:
 * @ctxt:  An XML parser context
 * @name:  The element name
 *
 * called when an opening tag has been processed.
 */
static void
startElementDebug(void *ctx ATTRIBUTE_UNUSED, const xmlChar *name, const xmlChar **atts)
{
    int i;

    fprintf(stdout, "SAX.startElement(%s", (char *) name);
    if (atts != NULL) {
        for (i = 0;(atts[i] != NULL);i++) {
      fprintf(stdout, ", %s='", atts[i++]);
      if (atts[i] != NULL)
          fprintf(stdout, "%s'", atts[i]);
  }
    }
    fprintf(stdout, ")\n");
}

static xmlEntityPtr
my_getEntity(void *user_data, const xmlChar *name)
{
  return xmlGetPredefinedEntity(name);
}

static void
endElementDebug(void *ctx ATTRIBUTE_UNUSED, const xmlChar *name)
{
    fprintf(stdout, "SAX.endElement(%s)\n", (char *) name);
}

void
SAXParser::startElement_receiver(void *ctx, const xmlChar *name, const xmlChar **atts)
{
  SAXParser *rcv = (SAXParser *)(ctx);
  if ( rcv )
    rcv->startElement(std::string( (const char*) name), atts);
}

void
SAXParser::characters_receiver (void *ctx, const xmlChar *ch, int len)
{
  SAXParser *rcv = (SAXParser *)(ctx);
  if ( rcv )
    rcv->characters( ch, len);
}

void
SAXParser::endElement_receiver(void *ctx, const xmlChar *name)
{
  SAXParser *rcv = (SAXParser *)(ctx);
  if ( rcv )
    rcv->endElement(std::string( (const char*) name));
}

static xmlEntityPtr getEntity_receiver(void *user_data, const xmlChar *name)
{
    return xmlGetPredefinedEntity(name);
}


void SAXParser::startElement(const std::string name, const xmlChar **atts)
{
  MIL << "start-element:" <<  name << std::endl;
}

void SAXParser::endElement(const std::string name)
{
  MIL << "end-element:" << name << std::endl;
}

void SAXParser::characters(const xmlChar *ch, int len)
{
  MIL << "characters:" << std::string( (const char *)ch, len) << std::endl;
}

void SAXParser::parseFile( const Pathname &p)
{
  FILE *f = fopen(p.asString().c_str(), "r");
  if (f != NULL)
  {
    int res = xmlSAXUserParseFile(&_saxHandler, (void *) this, p.asString().c_str());
    if (res != 0)
    {
      fprintf(stdout, "xmlSAXUserParseFile returned error %d\n", res);
    }
    fclose(f);
  }
  else
  {
    fprintf(stdout, "UPS\n");
  }
}

SAXParser::SAXParser()
{
  _saxHandler.internalSubset = NULL;
  _saxHandler.isStandalone = NULL;   
  _saxHandler.hasInternalSubset = NULL;
  _saxHandler.hasExternalSubset = NULL;
  _saxHandler.resolveEntity = NULL;
  _saxHandler.getEntity = NULL;
  _saxHandler.entityDecl = NULL;
  _saxHandler.notationDecl = NULL;
  _saxHandler.attributeDecl = NULL;
  _saxHandler.elementDecl = NULL;
  _saxHandler.unparsedEntityDecl = NULL;
  _saxHandler.setDocumentLocator = NULL;
  _saxHandler.startDocument = NULL;
  _saxHandler.endDocument = NULL;
  _saxHandler.startElement = NULL;
  _saxHandler.endElement = NULL;
  _saxHandler.reference = NULL;
  _saxHandler.characters = NULL;
  _saxHandler.ignorableWhitespace = NULL;
  _saxHandler.processingInstruction = NULL;
  _saxHandler.comment = NULL;
   
  //_saxHandler.xmlParserWarning = NULL;
  //_saxHandler.xmlParserError = NULL;
  //_saxHandler.xmlParserError = NULL;
  
  _saxHandler.getParameterEntity = NULL;
  _saxHandler.cdataBlock = NULL;
  _saxHandler.externalSubset = NULL; 

  _saxHandler = emptySAXHandlerStruct;
  //_saxHandler.startDocument = startElement_receiver;
 _saxHandler.startElement = startElement_receiver;
 _saxHandler.endElement = endElement_receiver;
 _saxHandler.getEntity = getEntity_receiver;
  _saxHandler.characters = characters_receiver;
}

SAXParser::~SAXParser()
{
}

} // ns parser
} // ns zypp