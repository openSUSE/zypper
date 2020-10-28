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
#include <zypp/base/LogControl.h>
#include <zypp/misc/LoadTestcase.h>
#include <zypp/misc/TestcaseSetupImpl.h>

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

  bool parseSetup ( const XmlNode &setup, zypp::misc::testcase::TestcaseSetup &t, std::string *err )
  {
    auto &target = t.data();
    auto architecture = setup.getProp( "arch" );
    if ( !architecture.empty() )
    {
      try {
        target.architecture = ( zypp::Arch(architecture) );
      }
      catch( const zypp::Exception & excpt_r ) {
        ZYPP_CAUGHT( excpt_r );
        if ( err ) *err = zypp::str::Str() << "Bad architecture '" << architecture << "' in <setup...>";
        return false;
      }
    }

    auto node = setup.children();
    while ( node )
    {
      if ( !node->isElement() ) {
        node = node->next();
        continue;
      }

#define if_SolverFlag( N ) if ( node->equals( #N ) ) { target.N = true; }
      if_SolverFlag( ignorealreadyrecommended )	else if ( node->equals( "ignorealready" ) ) 	{ target.ignorealreadyrecommended = true; }
      else if_SolverFlag( onlyRequires )        else if ( node->equals( "ignorerecommended" ) )	{ target.onlyRequires = true; }
      else if_SolverFlag( forceResolve )

      else if_SolverFlag( cleandepsOnRemove )

      else if_SolverFlag( allowDowngrade )
      else if_SolverFlag( allowNameChange )
      else if_SolverFlag( allowArchChange )
      else if_SolverFlag( allowVendorChange )

      else if_SolverFlag( dupAllowDowngrade )
      else if_SolverFlag( dupAllowNameChange )
      else if_SolverFlag( dupAllowArchChange )
      else if_SolverFlag( dupAllowVendorChange )
#undef if_SolverFlag
      else if ( node->equals("focus") ) {
        target.resolverFocus = zypp::resolverFocusFromString( node->getProp("value") );
      }
      else if ( node->equals("system") ) {
        target.systemRepo = zypp::misc::testcase::RepoDataImpl {
          zypp::misc::testcase::TestcaseRepoType::Helix,
          "@System",
          99,
          node->getProp("file")
        };
      }
      else if ( node->equals("hardwareInfo") ) {
        target.hardwareInfoFile = target.globalPath / node->getProp("path");
      }
      else if ( node->equals("modalias") ) {
        target.modaliasList.push_back( node->getProp("name") );
      }
      else if ( node->equals("multiversion") ) {
        target.multiversionSpec.insert( node->getProp("name") );
      }
      else if (node->equals ("channel")) {
        std::string name = node->getProp("name");
        std::string file = node->getProp("file");
        std::string type = node->getProp("type");

        unsigned prio = 99;
        std::string priority = node->getProp("priority");
        if ( !priority.empty() ) {
          prio = zypp::str::strtonum<unsigned>( priority );
        }

        target.repos.push_back( zypp::misc::testcase::RepoDataImpl{
          zypp::misc::testcase::TestcaseRepoType::Helix,
          name,
          prio,
          file
        });
      }
      else if ( node->equals("source") )
      {
        std::string url = node->getProp("url");
        std::string alias = node->getProp("name");
        target.repos.push_back( zypp::misc::testcase::RepoDataImpl{
          zypp::misc::testcase::TestcaseRepoType::Url,
          alias,
          99,
          url
        });
      }
      else if ( node->equals("force-install") )
      {
        target.forceInstallTasks.push_back( zypp::misc::testcase::ForceInstallImpl{
          node->getProp("channel"),
          node->getProp("package"),
          node->getProp("kind")
        });
      }
      else if ( node->equals("mediaid") )
      {
        target.show_mediaid = true;
      }
      else if ( node->equals("arch") ) {
        MIL << "<arch...> deprecated, use <setup arch=\"...\"> instead" << std::endl;
        std::string architecture = node->getProp("name");
        if ( architecture.empty() ) {
          ERR << "Property 'name=' in <arch.../> missing or empty" << std::endl;
        }
        else {
          MIL << "Setting architecture to '" << architecture << "'" << std::endl;
          target.architecture = zypp::Arch( architecture );
        }
      }
      else if ( node->equals("locale") )
      {
        zypp::Locale loc( node->getProp("name") );
        std::string fate = node->getProp("fate");
        if ( !loc ) {
          ERR << "Bad or missing name in <locale...>" << std::endl;
        }
        else if ( fate == "added" ) {
          target.localesTracker.added().insert( loc );
        }
        else if ( fate == "removed" ) {
          target.localesTracker.removed().insert( loc );
        }
        else {
          target.localesTracker.current().insert( loc );
        }
      }
      else if ( node->equals("autoinst") ) {
        target.autoinstalled.push( zypp::IdString( node->getProp("name") ).id() );
      }
      else if ( node->equals("systemCheck") ) {
        target.systemCheck = target.globalPath / node->getProp("path");
      }
      else if ( node->equals("setlicencebit") ) {
        target.set_licence = true;
      }
      else {
        ERR << "Unrecognized tag '" << node->name() << "' in setup" << std::endl;
      }
      node = node->next();
    }
    return true;
  }

  bool parseTrialNode ( const XmlNode &node, zypp::misc::testcase::TestcaseTrial::Node &testcaseNode )
  {
    testcaseNode.name() = node.name();
    const auto & content = node.getContent();
    if ( !content.empty() ) {
      testcaseNode.value() = content;
    }
    testcaseNode.properties() = node.getAllProps();

    for ( auto childNode = node.children(); childNode; childNode = childNode->next() ) {
      auto testNode = std::make_shared<zypp::misc::testcase::TestcaseTrial::Node>();
      if ( !parseTrialNode( *childNode, *testNode ) )
        return false;
      testcaseNode.children().push_back( testNode );
    }
    return true;
  }

  bool parseTrial ( const XmlNode &trial, zypp::misc::testcase::TestcaseTrial &target, std::string * )
  {
    auto node = trial.children();
    while (node) {
      if (!node->isElement()) {
        node = node->next();
        continue;
      }

      zypp::misc::testcase::TestcaseTrial::Node testcaseNode;
      parseTrialNode( *node, testcaseNode );
      target.nodes().push_back( testcaseNode );
      node = node->next();
    }
    return true;
  }
}

#endif
