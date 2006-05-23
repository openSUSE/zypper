/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/XMLNodeIterator.cc
 *
*/

#include <zypp/parser/XMLNodeIterator.h>
#include <zypp/base/Logger.h>
#include <libxml2/libxml/xmlreader.h>
#include <libxml2/libxml/xmlerror.h>

namespace zypp {

  namespace parser {

    using namespace std;

    namespace{
      /**
       * Internal function to read from the input stream.
       * This feeds the xmlTextReader used in the XMLNodeIterator.
       * @param context points to the istream to read from
       * @param buffer is to be filled with what's been read
       * @param bufferLen max memory bytes to read
       * @return
       */
      int ioread(void *context,
                char *buffer,
                int bufferLen)
      {
        xml_assert(buffer);
        std::istream *streamPtr = (std::istream *) context;
        xml_assert(streamPtr);
        streamPtr->read(buffer,bufferLen);
        return streamPtr->gcount();
      }

      /**
       * Internal function to finish reading.
       * This is required by the xmlTextReader API, but
       * not needed since the stream will be created when
       * the istream object vanishes.
       * @param context points to the istream to read from
       * @return 0 on success.
       */
      int ioclose(void * context)
      {
        /* don't close. destructor will take care. */
        return 0;
      }
    }

    XMLParserError::XMLParserError(const char *msg,
                                       int severity,
                                       xmlTextReaderLocatorPtr locator,
                                       int docLine,
                                       int docColumn)
    throw()
    : _msg(msg), _severity(severity), _locator(locator),
    _docLine(docLine), _docColumn(docColumn)
    { }

    XMLParserError::~XMLParserError() throw()
    { }

    std::string XMLParserError::msg() const throw()
    {
      return _msg;
    }

    int XMLParserError::severity() const throw()
    {
      return _severity;
    }

    xmlTextReaderLocatorPtr XMLParserError::locator() const throw()
    {
      return _locator;
    }

    int XMLParserError::docLine() const throw()
    {
      return _docLine;
    }

    int XMLParserError::docColumn() const throw()
    {
      return _docColumn;
    }

    std::string XMLParserError::position() const throw()
    {
      if (_docLine!=-1 && _docLine!=-1) {
        std::stringstream strm;
        strm << "at line " << _docLine
          <<", column " << _docColumn;
        return strm.str();
      }
      else
        return "";
    }

    std::ostream& operator<<(std::ostream &out, const XMLParserError& error)
    {
      const char *errOrWarn = (error.severity() & XML_PARSER_SEVERITY_ERROR) ? "error" : "warning";
    out << "XML syntax " << errOrWarn << ": " << error.msg();
      if (error.docLine()!=-1) {
        out  << "at line " << error.docLine()
          << ", column " << error.docColumn();
      }
      out << std::endl;
      return out;
    }


    XMLNodeIteratorBase::XMLNodeIteratorBase(std::istream &input,
                                             const std::string &baseUrl,
                                             const char *validationPath)
      : _error(0),
      _input(& input),
      _reader(xmlReaderForIO(ioread, ioclose, _input, baseUrl.c_str(), "utf-8",
                             XML_PARSE_PEDANTIC)),
      _baseUrl(baseUrl)
    {
      xmlTextReaderSetErrorHandler(_reader, (xmlTextReaderErrorFunc) errorHandler, this);
      // xmlTextReaderSetStructuredErrorHandler(_reader, structuredErrorHandler, this);
      if (_reader )
      {
        if ( validationPath )
        {
            if (xmlTextReaderRelaxNGValidate(_reader,validationPath)==-1)
              WAR << "Could not enable validation of document using " << validationPath << std::endl;
        }
        // otherwise validation is disabled.
      }
        /* Derived classes must call fetchNext() in their constructors themselves,
           XMLNodeIterator has no access to their virtual functions during
           construction */
    }

    XMLNodeIteratorBase::XMLNodeIteratorBase()
    : _error(0), _input(0), _reader(0)
    { }



    XMLNodeIteratorBase::~XMLNodeIteratorBase()
    {
      if (_reader != 0)
        xmlFreeTextReader(_reader);
    }


    bool
    XMLNodeIteratorBase::atEnd() const
    {
      return (_error.get() != 0
              || getCurrent() == 0);
    }


    bool
    XMLNodeIteratorBase::operator==(const XMLNodeIteratorBase &other) const
    {
      if (atEnd())
        return other.atEnd();
      else
        return this == & other;
    }


    const XMLParserError *
    XMLNodeIteratorBase::errorStatus() const
    {
      return _error.get();
    }


    void XMLNodeIteratorBase::fetchNext()
    {
      xml_assert(_reader);
      int status;
      /* throw away the old entry */
      setCurrent(0);

      if (_reader == 0) {
          /* this is a trivial iterator over (max) only one element,
             and we reach the end now. */
        ;
      }
      else {
          /* repeat as long as we successfully read nodes
             breaks out when an interesting node has been found */
        while ((status = xmlTextReaderRead(_reader))==1) {
          xmlNodePtr node = xmlTextReaderCurrentNode(_reader);
          if (isInterested(node)) {
              // xmlDebugDumpNode(stdout,node,5);
            _process(_reader);
              // _currentDataPtr.reset(new ENTRYTYPE(process(_reader)));
            status = xmlTextReaderNext(_reader);
            break;
          }
        }
        if (status == -1) {  // error occured
          if (_error.get() == 0) {
            errorHandler(this, "Unknown error while parsing xml file\n",
                         XML_PARSER_SEVERITY_ERROR, 0);
          }
        }
      }
    }


    void
    XMLNodeIteratorBase::errorHandler(void * arg,
                                      const char * msg,
                                      int severity,
                                      xmlTextReaderLocatorPtr locator)
    {
      XMLNodeIteratorBase *obj;
      obj = (XMLNodeIteratorBase*) arg;
      xml_assert(obj);
      xmlTextReaderPtr reader = obj->_reader;
      if (strcmp("%s",msg) == 0) {
          /* This works around a buglet in libxml2, you often get "%s" as message
             and the message is in "severity". Does this work for other
             architectures??? FIXME */
        WAR << "libxml2 error reporting defect, got '%s' as message" << endl;
        msg = (char *) severity;
        severity = XML_PARSER_SEVERITY_WARNING;
      }
      const char *errOrWarn = (severity & XML_PARSER_SEVERITY_ERROR) ? "error" : "warning";

#if 0
      std::ostream& out = (severity & XML_PARSER_SEVERITY_ERROR) ? ERR : WAR;

        /* Log it */
    out << "XML syntax " << errOrWarn << ": " << msg;
      if (obj->_error.get()) {
        out << "(encountered during error recovery!)" << std::endl;
      }
      if (reader && msg[0] != 0) {
        out  << "at ";
        if (! obj->_baseUrl.empty())
          out << obj->_baseUrl << ", ";
        out << "line " << xmlTextReaderGetParserLineNumber(reader)
        << ", column " << xmlTextReaderGetParserColumnNumber(reader);
      }
      out << std::endl;
#endif
        /* save it */
      if ((severity & XML_PARSER_SEVERITY_ERROR)
          && ! obj->_error.get()) {
            if (reader)
              obj->_error.reset(new XMLParserError
                                (msg, severity,locator,
                                 xmlTextReaderLocatorLineNumber(locator),
                                 xmlTextReaderGetParserColumnNumber(reader)));
            else
              obj->_error.reset(new XMLParserError
                                (msg, severity, locator,
                                 -1, -1));
          }
    }

  }
}
