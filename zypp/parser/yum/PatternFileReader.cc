/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"
#include "zypp/parser/yum/PatternFileReader.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  PatternFileReader::PatternFileReader(const Pathname & pattern_file, ProcessPattern callback)
      : _callback(callback)
  {
    Reader reader(pattern_file);
    MIL << "Reading " << pattern_file << endl;
    reader.foreachNode(bind(&PatternFileReader::consumeNode, this, _1));
  }

  // --------------------------------------------------------------------------

  /*
   * xpath and multiplicity of processed nodes are included in the code
   * for convenience:
   * 
   * // xpath: <xpath> (?|*|+)
   * 
   * if multiplicity is ommited, then the node has multiplicity 'one'. 
   */

  // --------------------------------------------------------------------------

  bool PatternFileReader::consumeNode(Reader & reader_r)
  {
    // dependency block nodes
    if (consumeDependency(reader_r, _pattern->deps))
      return true;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /patterns/pattern
      if (reader_r->name() == "pattern")
      {
        tag(tag_pattern); // just for the case of reuse somewhere/sometimes
        _pattern = new data::Pattern;
        return true;
      }

      // xpath: /patterns/pattern/name
      if (reader_r->name() == "name")
      {
        _pattern->name = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /patterns/pattern/summary (+)
      if (reader_r->name() == "summary")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _pattern->summary.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /patterns/pattern/description (+)
      if (reader_r->name() == "description")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _pattern->description.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /patterns/pattern/default (?)
      if (reader_r->name() == "default")
      {
        _pattern->isDefault = true;
      }

      // xpath: /patterns/pattern/uservisible (?)
      if (reader_r->name() == "uservisible")
      {
        _pattern->userVisible = true;
        return true;
      }

      // xpath: /patterns/pattern/category
      if (reader_r->name() == "category")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _pattern->category.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /patterns/pattern/icon (?)
      if (reader_r->name() == "icon")
      {
        _pattern->icon = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /patterns/pattern/script (?)
      if (reader_r->name() == "script")
      {
        _pattern->script = reader_r.nodeText().asString();
        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patterns/pattern
      if (reader_r->name() == "pattern")
      {
      	if (_callback)
      	  _callback(handoutPattern());

        toParentTag(); // just for the case of reuse somewhere/sometimes

      	return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  data::Pattern_Ptr PatternFileReader::handoutPattern()
  {
    data::Pattern_Ptr ret;
    ret.swap(_pattern);
    return ret;
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
