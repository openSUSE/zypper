/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMPatternParser.cc
 *
*/

#include <zypp/parser/yum/YUMPatternParser.h>
#include <zypp/parser/yum/YUMPrimaryParser.h>
#include <zypp/parser/LibXMLHelper.h>
#include <istream>
#include <string>
#include <cassert>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/yum/schemanames.h>

using namespace std;
namespace zypp {
  namespace parser {
    namespace yum {

      YUMPatternParser::YUMPatternParser()
      { }
      
      YUMPatternParser::YUMPatternParser(YUMPatternData_Ptr& entry)
      : XMLNodeIterator<YUMPatternData_Ptr>(entry)
      { }
      
      
      YUMPatternParser::~YUMPatternParser()
      { }
      
      
      // select for which elements process() will be called
      bool 
      YUMPatternParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "pattern";
      }
      
      // do the actual processing
      YUMPatternData_Ptr
      YUMPatternParser::process(const xmlTextReaderPtr reader)
      {
        assert(reader);
        YUMPatternData_Ptr dataPtr = new YUMPatternData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        assert(dataNode);

        // FIXME move the respective method to a common class, inherit it  
        YUMPrimaryParser prim;
        
        for (xmlNodePtr child = dataNode->children;
             child && child != dataNode;
             child = child->next) {
               if (_helper.isElement(child)) {
                 string name = _helper.name(child);
                 if (name == "name") {
                   dataPtr->name = _helper.content(child);
                 }
                 else if (name == "summary") {
                   dataPtr->summary.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
                 }
                 else if (name == "default") {
                   dataPtr->default_ = _helper.content(child);
                 }
                 else if (name == "uservisible") {
                   dataPtr->userVisible = _helper.content(child);
                 }
                 else if (name == "description") {
                   dataPtr->description.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
                 }
		 else if (name == "category") {
       #warning FIXME pattern category is translatable
		   dataPtr->category.setText(_helper.content(child));
		 }
		 else if (name == "icon") {
		   dataPtr->icon = _helper.content(child);
		 }
		 else if (name == "script") {
		   dataPtr->script = _helper.content(child);
		 }
                 else if (name == "provides") {
		   prim.parseDependencyEntries(& dataPtr->provides, child);
		 }
		 else if (name == "conflicts") {
		   prim.parseDependencyEntries(& dataPtr->conflicts, child);
		 }
		 else if (name == "obsoletes") {
		   prim.parseDependencyEntries(& dataPtr->obsoletes, child);
		 }
		 else if (name == "requires") {
		   prim.parseDependencyEntries(& dataPtr->requires, child);
		 }
		 else if (name == "recommends") {
		   prim.parseDependencyEntries(& dataPtr->recommends, child);
		 }
		 else if (name == "suggests") {
		   prim.parseDependencyEntries(& dataPtr->suggests, child);
		 }
		 else if (name == "enhances") {
		   prim.parseDependencyEntries(& dataPtr->enhances, child);
		 }
		 else if (name == "freshen") {
		   prim.parseDependencyEntries(& dataPtr->freshen, child);
		 }
                 else {
                   WAR << "YUM <pattern> contains the unknown element <" << name << "> "
                     << _helper.positionInfo(child) << ", skipping" << endl;
                 }
               }
             }
        return dataPtr;
      } /* end process */
      
      
      YUMPatternParser::YUMPatternParser(istream &is, const string &baseUrl)
      : XMLNodeIterator<YUMPatternData_Ptr>(is, baseUrl,PATTERNSCHEMA)
      { 
        fetchNext();
      }
      
    } // namespace yum
  } // namespace parser
} // namespace zypp
