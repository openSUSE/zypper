/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file  zypp/misc/HelixHelpers.h
 *
*/
#ifndef ZYPP_MISC_HELIXHELPERS_H
#define ZYPP_MISC_HELIXHELPERS_H

#include <zypp/AutoDispose.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <string>
#include <map>
#include <string_view>
#include <optional>


namespace helix::detail {

  template <typename Tp>
  struct AutoXmlFree : public zypp::AutoDispose<Tp*>
  {
    AutoXmlFree( Tp* ptr_r = nullptr ) : zypp::AutoDispose<Tp*>( ptr_r, [] ( Tp* ptr_r ) { if ( ptr_r ) ::xmlFree( ptr_r ); } ) {}
  };

  class XmlNode
  {

  private:
    xmlNodePtr _node;

  public:
    XmlNode (const xmlNodePtr node) : _node(node){};
    virtual ~XmlNode (){};

    // ---------------------------------- accessors

    std::string_view name() const { return (std::string_view((const char *)_node->name)); }
    xmlElementType type() const { return (_node->type); }

    xmlNodePtr node() const { return (_node); }
    std::optional<XmlNode> next() const { return (_node->next == NULL ? std::optional<XmlNode>() : XmlNode (_node->next)); }
    std::optional<XmlNode> children() const { return (_node->xmlChildrenNode == NULL ? std::optional<XmlNode>() : XmlNode (_node->xmlChildrenNode)); }

    // ---------------------------------- methods

    bool equals (const std::string_view & n) const { return (strncasecmp ( name().data(), n.data(), n.length() ) == 0); }
    bool isElement (void) const { return (type() == XML_ELEMENT_NODE); }

    std::map<std::string, std::string> getAllProps () const {
      std::map<std::string, std::string> res;
      for( xmlAttrPtr attr = _node->properties; NULL != attr; attr = attr->next ) {
        if ( !attr->children )
          continue;
        AutoXmlFree<xmlChar> value( xmlNodeListGetString( _node->doc, attr->children, 1 ) );
        res.insert( std::make_pair( std::string((char *)attr->name), std::string( (char *)value.value() ) ) );
      }
      return res;
    }

    std::string getContent (void) const {
      AutoXmlFree<xmlChar> buf;
      std::string ret;

      *buf = xmlNodeGetContent (_node);

      ret = std::string ((const char *)buf.value());

      return (ret);
    }

    std::string getProp (const std::string & name, const std::string & deflt = "") const {
      AutoXmlFree<xmlChar> ret;
      std::string gs;

      *ret = xmlGetProp (_node, (const xmlChar *)name.c_str());

      if (ret) {
        gs = std::string ((const char  *)ret.value());
        return gs;
      }
      return deflt;
    }


    template<typename T>
    T getValue ( const std::string & name, const T& deflt ) const;

    template<typename T>
    bool getValue ( const std::string & name, T& target ) const;
  };

  template<>
  bool XmlNode::getValue ( const std::string & name, std::string& target ) const {
    AutoXmlFree<xmlChar> xml_s;
    xmlNode *child;

    *xml_s = xmlGetProp(_node, (const xmlChar *)name.c_str());
    if (xml_s) {
      target = std::string ((const char *)xml_s.value());
      return true;
    }

    child = _node->xmlChildrenNode;

    while (child) {
      if (strcasecmp((const char *)(child->name), name.c_str()) == 0) {
        xml_s = xmlNodeGetContent(child);
        if (xml_s) {
          target = std::string ((const char *)xml_s.value());
          return true;
        }
      }
      child = child->next;
    }
    return false;
  }

  template<>
  std::string XmlNode::getValue ( const std::string & name, const std::string& deflt ) const {
    std::string res;
    if ( !getValue( name, res ) )
      return deflt;
    return res;
  }
}

#endif
