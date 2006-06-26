/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/store/xml/XMLSourceCacheParser.cc
 *
*/

#include <zypp/parser/LibXMLHelper.h>
#include <istream>
#include <string>
#include <cassert>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/xmlstore/schemanames.h>

#include "XMLSourceCacheParser.h"

using namespace std;

namespace zypp {
namespace parser {
namespace xmlstore {

      XMLSourceCacheParser::XMLSourceCacheParser()
      { }

      XMLSourceCacheParser::XMLSourceCacheParser(SourceInfo_Ptr &entry)
        : zypp::parser::XMLNodeIterator<SourceInfo_Ptr>(entry)
      { }


      XMLSourceCacheParser::~XMLSourceCacheParser()
      { }


      // select for which elements process() will be called
      bool
      XMLSourceCacheParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "source";
      }

      // do the actual processing
      SourceInfo_Ptr
      XMLSourceCacheParser::process(const xmlTextReaderPtr reader)
      {
        assert(reader);
        SourceInfo_Ptr dataPtr( new source::SourceInfo );
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        assert(dataNode);

        for (xmlNodePtr child = dataNode->children; child && child != dataNode; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "enabled")
            {
              if ( (_helper.content(child) == "true") || (_helper.content(child) == "1") )
                dataPtr->setEnabled(true);
              else if ( (_helper.content(child) == "false") || (_helper.content(child) == "0") )
                dataPtr->setEnabled(false);
              else
                dataPtr->setEnabled(indeterminate);
            }
            else if (name == "auto-refresh")
            {
              if ( (_helper.content(child) == "true") || (_helper.content(child) == "1") )
                dataPtr->setAutorefresh(true);
              if ( (_helper.content(child) == "false") || (_helper.content(child) == "0") )
                dataPtr->setAutorefresh(false);
              else
                dataPtr->setAutorefresh(indeterminate);
            }
            else if (name == "type")
            {
              dataPtr->setType(_helper.content(child));
            }
            else if (name == "product-dir")
            {
              dataPtr->setPath(_helper.content(child));
            }
            else if (name == "cache-dir")
            {
              dataPtr->setCacheDir(_helper.content(child));
            }
            else if (name == "alias")
            {
              dataPtr->setAlias(_helper.content(child));
            }
            else if (name == "url")
            {
              dataPtr->setUrl(_helper.content(child));
            }
            else
            {
              WAR << "SourceCache entry contains the unknown element <" << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        return dataPtr;
      } /* end process */


      XMLSourceCacheParser::XMLSourceCacheParser(istream &is, const string &baseUrl)
        : zypp::parser::XMLNodeIterator<SourceInfo_Ptr>(is, baseUrl, SOURCESCHEMA)
      {
        fetchNext();
      }
}
} // namespace parser
} // namespace zypp
