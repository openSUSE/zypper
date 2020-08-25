/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file  zypp/misc/LoadTestcase.cc
 *
*/
#include "LoadTestcase.h"
#include "HelixHelpers.h"
#include <zypp/PathInfo.h>
#include <zypp/base/LogControl.h>

namespace helix::detail {

  bool parseSetup ( const XmlNode &setup, zypp::misc::testcase::TestcaseSetup &target, std::string *err )
  {
    auto architecture = zypp::Arch( setup.getProp( "arch" ) );
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
        target.systemRepo = zypp::misc::testcase::RepoData {
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

        target.repos.push_back( zypp::misc::testcase::RepoData{
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
        target.repos.push_back( zypp::misc::testcase::RepoData{
          zypp::misc::testcase::TestcaseRepoType::Url,
          alias,
          99,
          url
        });
      }
      else if ( node->equals("force-install") )
      {
        target.forceInstallTasks.push_back( zypp::misc::testcase::ForceInstall{
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
    testcaseNode.name = node.name();
    const auto & content = node.getContent();
    if ( !content.empty() ) {
      testcaseNode.value = content;
    }
    testcaseNode.properties = node.getAllProps();

    for ( auto childNode = node.children(); childNode; childNode = childNode->next() ) {
      auto testNode = std::make_shared<zypp::misc::testcase::TestcaseTrial::Node>();
      if ( !parseTrialNode( *childNode, *testNode ) )
        return false;
      testcaseNode.children.push_back( testNode );
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
      target.nodes.push_back( testcaseNode );
      node = node->next();
    }
    return true;
  }
}

namespace zypp::misc::testcase {

  static const std::string helixControlFile = "solver-test.xml";
  static const std::string yamlControlFile  = "zypp-control.yaml";

  struct LoadTestcase::Impl {
    TestcaseSetup _setup;
    std::vector<TestcaseTrial> _trials;

    bool loadHelix (const Pathname &filename, std::string *err);

    bool loadYaml  ( const Pathname &path, std::string *err);
  };


  bool LoadTestcase::Impl::loadHelix(const zypp::filesystem::Pathname &filename, std::string *err)
  {
    xmlDocPtr xml_doc = xmlParseFile ( filename.c_str() );
    if (xml_doc == NULL) {
      if ( err ) *err = (str::Str() << "Can't parse test file '" << filename << "'");
      return false;
    }


    auto root = helix::detail::XmlNode (xmlDocGetRootElement (xml_doc));

    DBG << "Parsing file '" << filename << "'" << std::endl;

    if (!root.equals("test")) {
      if ( err ) *err = (str::Str() << "Node not 'test' in parse_xml_test():" << root.name() << "'");
      return false;
    }

    bool setupDone = false;
    auto node = root.children();
    while (node) {
      if (node->type() == XML_ELEMENT_NODE) {
        if (node->equals( "setup" )) {
          if ( setupDone ) {
            if ( err ) *err = "Multiple setup tags found, this is not supported";
            return false;
          }
          setupDone = true;
          if ( !helix::detail::parseSetup( *node, _setup, err ) )
            return false;

        } else if (node->equals( "trial" )) {
          if ( !setupDone ) {
            if ( err ) *err = "Any trials must be preceeded by the setup!";
            return false;
          }
          TestcaseTrial trial;
          if ( !helix::detail::parseTrial( *node, trial, err ) )
            return false;
          _trials.push_back( trial );
        } else {
          ERR << "Unknown tag '" << node->name() << "' in test" << std::endl;
        }
      }
      node = ( node->next() );
    }
    xmlFreeDoc (xml_doc);
    return true;
  }

  bool LoadTestcase::Impl::loadYaml(const zypp::filesystem::Pathname &path, std::string *err)
  {
    return false;
  }

  LoadTestcase::LoadTestcase() : _pimpl( new Impl() )
  { }

  LoadTestcase::~LoadTestcase()
  { }

  bool LoadTestcase::loadTestcaseAt(const zypp::filesystem::Pathname &path, std::string *err)
  {
    const auto t = testcaseTypeAt( path );
    if ( t == LoadTestcase::None ) {
      if ( err ) *err = "Unsopported or no testcase in directory";
      return false;
    }

    // reset everything
    _pimpl.reset( new Impl() );
    _pimpl->_setup.globalPath = path;

    switch (t) {
      case LoadTestcase::Helix:
        return _pimpl->loadHelix( path / helixControlFile, err );
      case LoadTestcase::Yaml:
        return _pimpl->loadYaml( path / yamlControlFile, err );
      default:
        return false;
    }
  }

  LoadTestcase::Type LoadTestcase::testcaseTypeAt(const zypp::filesystem::Pathname &path)
  {
    if ( filesystem::PathInfo( path / helixControlFile ).isFile() ) {
      return LoadTestcase::Helix;
    } else if ( filesystem::PathInfo( path / yamlControlFile ).isFile() ) {
      return LoadTestcase::Yaml;
    }
    return LoadTestcase::None;
  }

  const TestcaseSetup &LoadTestcase::setupInfo() const
  {
    return _pimpl->_setup;
  }

  const std::vector<TestcaseTrial> &LoadTestcase::trialInfo() const
  {
    return _pimpl->_trials;
  }

  const std::string &TestcaseTrial::Node::getProp( const std::string &name, const std::string &def ) const
  {
    if ( properties.find( name) == properties.end() )
      return def;
    return properties.at( name );
  }



}
