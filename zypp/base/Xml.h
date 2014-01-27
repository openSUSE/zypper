/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Xml.h
 *
*/
#ifndef ZYPP_BASE_XML_H
#define ZYPP_BASE_XML_H

#include <iosfwd>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

#include "zypp/base/Easy.h"
#include "zypp/base/String.h"
#include "zypp/parser/xml/XmlEscape.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace xmlout
  {
    using xml::escape;
    using xml::unescape;

    /** \relates NodeAttr NODE ATTRIBUTE representation of types [str::asString] */
    template <class _Tp>
    std::string asXmlNodeAttr( const _Tp & val_r )
    { return str::asString( val_r ); }

    ///////////////////////////////////////////////////////////////////
    /// \class NodeAttr
    /// \brief (Key, Value) string pair of XML node attributes
    struct NodeAttr : public std::pair<std::string,std::string>
    {
      typedef std::pair<std::string,std::string> Pair;

      template <typename _Type>
      NodeAttr( std::string key_r, const _Type & val_r )
      : Pair( std::move(key_r), asXmlNodeAttr(val_r) )
      {}

      NodeAttr( std::string key_r, std::string val_r )
      : Pair( std::move(key_r), std::move(val_r) )
      {}
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /// \class Node
    /// \brief RAII writing a nodes start/end tag
    /// \code
    /// {
    ///   Node node( std::cout, "node", { "attr", "val" } ); // <node attr="val">
    ///   *node << "write nodes body...."
    /// }                                                          // </node>
    /// \endcode
    struct Node
    {
      NON_COPYABLE_BUT_MOVE( Node );
      typedef NodeAttr Attr;

      /** Ctor taking nodename and attribute list */
      Node( std::ostream & out_r, std::string name_r, const std::initializer_list<Attr> & attrs_r = {} )
      : _out( out_r ), _name( std::move(name_r) )
      {
	if ( ! _name.empty() )
	{
	  _out << "<" << _name;
	  for ( const auto & pair : attrs_r )
	    _out << " " << pair.first << "=\"" << xml::escape( pair.second ) << "\"";
	  _out << ">";
	}
      }

      /** Convenience ctor for one attribute pair */
      Node( std::ostream & out_r, std::string name_r, Attr attr_r )
      : Node( out_r, std::move(name_r), { attr_r } )
      {}

      /** Dtor wrting end tag */
      ~Node()
      { if ( ! _name.empty() ) _out << "</" << _name << ">"; }

      /** Return the output stream */
      std::ostream & operator*() { return _out; }

    private:
      std::ostream & _out;
      std::string _name;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Node Write a leaf node without PCDATA
     * \code
     * <node attr="val"/>
     * \endcode
     */
    inline std::ostream & node( std::ostream & out_r, const std::string & name_r, const std::initializer_list<Node::Attr> & attrs_r = {} )
    {
      out_r << "<" << name_r;
      for ( const auto & pair : attrs_r )
	out_r << " " << pair.first << "=\"" << xml::escape( pair.second ) << "\"";
      return out_r << "/>";
    }
    /** \overload for one attribute pair */
    inline std::ostream & node( std::ostream & out_r, const std::string & name_r, Node::Attr attr_r )
    { return node( out_r, name_r, { attr_r } ); }

  } // namespace xmlout
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_XML_H
