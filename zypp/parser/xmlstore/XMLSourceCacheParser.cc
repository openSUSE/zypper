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
              if ( _helper.content(child) == "true" )
                dataPtr->enabled = source::SourceInfo::Enabled;
              if ( _helper.content(child) == "false" )
                dataPtr->enabled = source::SourceInfo::Disabled;
              else
                dataPtr->enabled = source::SourceInfo::NotSet;
            }
            else if (name == "auto-refresh")
            {
              if ( _helper.content(child) == "true" )
                dataPtr->autorefresh = source::SourceInfo::Enabled;
              if ( _helper.content(child) == "false" )
                dataPtr->autorefresh = source::SourceInfo::Disabled;
              else
                dataPtr->autorefresh = source::SourceInfo::NotSet;
            }
            else if (name == "type")
            {
              dataPtr->type = _helper.content(child);
            }
            else if (name == "product-dir")
            {
              dataPtr->product_dir = _helper.content(child);
            }
            else if (name == "cache-dir")
            {
              dataPtr->cache_dir = _helper.content(child);
            }
            else if (name == "alias")
            {
              dataPtr->alias = _helper.content(child);
            }
            else if (name == "url")
            {
              dataPtr->url = _helper.content(child);
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
