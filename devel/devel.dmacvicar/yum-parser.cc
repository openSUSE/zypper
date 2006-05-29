#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include <zypp/base/Logger.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/TranslatedText.h>
///////////////////////////////////////////////////////////////////

#include <zypp/parser/yum/YUMParser.h>
#include <zypp/base/Logger.h>
#include <zypp/source/yum/YUMScriptImpl.h>
#include <zypp/source/yum/YUMMessageImpl.h>
#include <zypp/source/yum/YUMPackageImpl.h>
#include <zypp/source/yum/YUMSourceImpl.h>
#include <map>
#include <set>
#include <zypp/CapFactory.h>
#include <libxml/parser.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using namespace zypp::source::yum;

xmlSAXHandler emptySAXHandlerStruct = {
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

xmlSAXHandlerPtr emptySAXHandler = &emptySAXHandlerStruct;
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


/**
 * endElementDebug:
 * @ctxt:  An XML parser context
 * @name:  The element name
 *
 * called when the end of an element has been detected.
 */
static void
endElementDebug(void *ctx ATTRIBUTE_UNUSED, const xmlChar *name)
{
    fprintf(stdout, "SAX.endElement(%s)\n", (char *) name);
}

static void
primaryStartElement(void *ctx ATTRIBUTE_UNUSED, const xmlChar *name, const xmlChar **atts)
{
}

static void
primaryEndElement(void *ctx ATTRIBUTE_UNUSED, const xmlChar *name)
{
}

struct SAXParserBase
{
  static void
  startElement_receiver(void *ctx, const xmlChar *name, const xmlChar **atts)
  {
    SAXParserBase *rcv = (SAXParserBase *) ctx;
    rcv->startElement(std::string( (const char*) name), atts);
  }

  static void
  endElement_receiver(void *ctx, const xmlChar *name)
  {
    SAXParserBase *rcv = (SAXParserBase *) ctx;
    rcv->endElement(std::string( (const char*) name));
  }

  virtual void startElement(const std::string name, const xmlChar **atts)
  {
    MIL << name << std::endl;
  }

  virtual void endElement(const std::string name)
  {
    MIL << name << std::endl;
  }

  private:
  xmlSAXHandler _saxHandler;

  public:

  void parseFile( const Pathname &p)
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

//   void parseFileByChunk( const Pathname &p)
//   {
//     FILE *f = fopen(p.asString().c_str(), "r");
//     if (f != NULL)
//     {
//       int ret;
//       char chars[10];
//       xmlParserCtxtPtr ctxt;
//   
//       ret = fread(chars, 1, 4, f);
//       if (ret > 0)
//       {
//         ctxt = xmlCreatePushParserCtxt(&_saxHandler, NULL,
//           chars, ret, p.asString().c_str());
//         while ((ret = fread(chars, 1, 3, f)) > 0)
//         {
//           xmlParseChunk(ctxt, chars, ret, 0);
//         }
//         ret = xmlParseChunk(ctxt, chars, 0, 1);
//         xmlFreeParserCtxt(ctxt);
//         if (ret != 0)
//         {
//           fprintf(stdout, "xmlSAXUserParseFile returned error %d\n", ret);
//         }
//        }
//        fclose(f);
//     }
//     else
//     {
//       fprintf(stdout, "UPS\n");
//     }
//   }

  SAXParserBase()
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
  }

  virtual ~SAXParserBase()
  {
  }
};


struct myParser : public SAXParserBase
{

};


int main()
{
  myParser parser;
  parser.parseFile("repodata/primary.xml.gz");
}


