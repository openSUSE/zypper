/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file  zypp/misc/YamlTestcaseHelpers.h
 *
*/
#ifndef ZYPP_MISC_YAMLTESTCASEHELPERS_H
#define ZYPP_MISC_YAMLTESTCASEHELPERS_H

#include <zypp/base/LogControl.h>
#include "LoadTestcase.h"

#include <yaml-cpp/yaml.h>

#include <type_traits>

namespace yamltest::detail {

  bool parseSetup ( const YAML::Node &setup, zypp::misc::testcase::TestcaseSetup &target, std::string *err ) {

    MIL << "Parsing setup node " << std::endl;
    for ( YAML::const_iterator it = setup.begin(); it != setup.end(); it++ ) {

      const std::string &key = it->first.as<std::string>();
      const auto &data = it->second;

      MIL << "Found key " << key << std::endl;

      // reads a sequence either from a file or inline, depending on the type of "data"
      auto readListInlineOrFromFile = [&]( const auto &cb , std::string *err ) -> bool {
        if ( data.Type() == YAML::NodeType::Sequence ) {
          int cnt = 0;
          for ( const auto &node: data ) {
            if ( !cb( node, err ) ) return false;
            cnt ++;
          }
          MIL << "Loaded " << cnt << " Elements inline" << std::endl;
        } else {
          const std::string &fName = data.as<std::string>();
          MIL << "Trying to load list from file " << fName << std::endl;
          try {
            auto doc = YAML::LoadFile( fName );
            if ( doc.Type() != YAML::NodeType::Sequence ) {
              if ( err ) *err = "Expected the top node to be a sequence in external file for key: ";
              return false;
            }

            int cnt = 0;
            for ( const auto &node : doc ) {
              if ( !cb( node, err ) ) return false;
              cnt ++;
            }
            MIL << "Loaded " << cnt << " Elements from file" << std::endl;
          } catch ( YAML::Exception &e ) {
            if ( err ) *err = e.what();
            return false;
          } catch ( ... )  {
            if ( err ) *err = zypp::str::Str() << "Unknown error when parsing the file for " << key;
            return false;
          }
        }
        return true;
      };

      if ( key == "resolverFlags" ) {
#define if_SolverFlag( N ) if ( data[#N] ) { target.N = data[#N].as<bool>(); }
        if_SolverFlag( ignorealreadyrecommended ) if ( data["ignorealready"] )       { target.ignorealreadyrecommended = data["ignorealready"].as<bool>(); }
        if_SolverFlag( onlyRequires )        if ( data["ignorerecommended"] ) { target.onlyRequires = data["ignorerecommended"].as<bool>(); }
        if_SolverFlag( forceResolve )

        if_SolverFlag( cleandepsOnRemove )

        if_SolverFlag( allowDowngrade )
        if_SolverFlag( allowNameChange )
        if_SolverFlag( allowArchChange )
        if_SolverFlag( allowVendorChange )

        if_SolverFlag( dupAllowDowngrade )
        if_SolverFlag( dupAllowNameChange )
        if_SolverFlag( dupAllowArchChange )
        if_SolverFlag( dupAllowVendorChange )
#undef if_SolverFlag
        if ( data["focus"] ) {
          target.resolverFocus = zypp::resolverFocusFromString( data["focus"].as<std::string>() );
        }
      } else if ( key == ("system") ) {
        target.systemRepo = zypp::misc::testcase::RepoData {
          zypp::misc::testcase::TestcaseRepoType::Testtags,
          "@System",
          99,
          data["file"].as<std::string>()
        };
      }
      else if ( key == ("hardwareInfo") ) {
        target.hardwareInfoFile = data.as<std::string>();
      }
      else if ( key == ("modalias") ) {
        bool success = readListInlineOrFromFile( [&target]( const YAML::Node &dataNode, auto ){
          target.modaliasList.push_back( dataNode.as<std::string>() );
          return true;
        }, err );
        if ( !success ) return false;
      }
      else if ( key == ("multiversion") ) {
        bool success = readListInlineOrFromFile( [&target]( const YAML::Node &dataNode, auto ){
          target.multiversionSpec.insert( dataNode.as<std::string>() );
          return true;
        }, err );
        if ( !success ) return false;
      }
      else if (key ==  ("channels")) {
        bool success = readListInlineOrFromFile( [&target]( const YAML::Node &dataNode, auto ){
          std::string name = dataNode["alias"].as<std::string>();
          std::string file = dataNode["file"].as<std::string>();
          std::string type = dataNode["type"].as<std::string>();

          unsigned prio = 99;
          if ( dataNode["priority"] )
            prio = dataNode["priority"].as<unsigned>();

          target.repos.push_back( zypp::misc::testcase::RepoData{
            zypp::misc::testcase::TestcaseRepoType::Testtags,
            name,
            prio,
            file
          });
          return true;
        }, err );
        if ( !success ) return false;
      }
      else if ( key == ("sources") )
      {
        bool success = readListInlineOrFromFile( [&target]( const YAML::Node &dataNode, auto ){
          std::string url   = dataNode["url"].as<std::string>();
          std::string alias = dataNode["name"].as<std::string>();
          target.repos.push_back( zypp::misc::testcase::RepoData{
            zypp::misc::testcase::TestcaseRepoType::Url,
            alias,
            99,
            url
          });
          return true;
        }, err );
        if ( !success ) return false;
      }
      else if ( key == ("force-install") )
      {
        bool success = readListInlineOrFromFile( [&target]( const YAML::Node &dataNode, auto ){
          target.forceInstallTasks.push_back( zypp::misc::testcase::ForceInstall{
            dataNode["channel"].as<std::string>(),
            dataNode["package"].as<std::string>(),
            dataNode["kind"].as<std::string>()
          });
          return true;
        }, err );
        if ( !success ) return false;
      }
      else if ( key == ("mediaid") )
      {
        target.show_mediaid = data.as<bool>();
      }
      else if ( key == ("arch") ) {
        std::string architecture = data.as<std::string>();
        if ( architecture.empty() ) {
          if (err) *err = zypp::str::Str() << "Property 'arch' in setup can not be empty." << std::endl;
          return false;
        }
        else {
          MIL << "Setting architecture to '" << architecture << "'" << std::endl;
          target.architecture = zypp::Arch( architecture );
        }
      }
      else if ( key == ("locales") )
      {
        bool success = readListInlineOrFromFile( [&target]( const YAML::Node &dataNode, std::string *err ){
          zypp::Locale loc( dataNode["name"].as<std::string>() );
          std::string fate = dataNode["fate"].as<std::string>();
          if ( !loc ) {
            if (err) *err = zypp::str::Str() << "Bad or missing name in locale..." << std::endl;
            return false;
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
          return true;
        }, err );
        if ( !success ) return false;
      }
      else if ( key == ("autoinst") ) {
        bool success = readListInlineOrFromFile( [&]( const YAML::Node &dataNode, auto ){
          target.autoinstalled.push( zypp::IdString( dataNode.as<std::string>() ).id() );
          return true;
        }, err );
        if ( !success ) return false;
      }
      else if ( key == ("systemCheck") ) {
        target.systemCheck = data.as<std::string>();
      }
      else if ( key == ("setlicencebit") ) {
        target.set_licence = data.as<bool>();
      }
      else {
        ERR << "Ignoring unrecognized tag '" << key << "' in setup" << std::endl;
      }
    }
    return true;
  }

  template <typename T>
  bool parseJobs  ( const YAML::Node &trial, std::vector<T> &target, std::string *err );

  template <typename T>
  bool parseSingleJob ( const YAML::Node &jobNode, std::vector<T> &target, std::string *err ) {

    constexpr bool isSubNode = std::is_same_v<T, std::shared_ptr<zypp::misc::testcase::TestcaseTrial::Node>>;
    if ( jobNode["include"] ) {
      //handle include
      const auto &fName = jobNode["include"].as<std::string>();
      MIL << "Including file " << fName << std::endl;
      try {
        auto doc = YAML::LoadFile( fName );
        if ( !parseJobs( doc, target, err ) )
          return false;
        MIL << "Including file " << fName << "was successfull" << std::endl;
      } catch ( YAML::Exception &e ) {
        if ( err ) *err = e.what();
        return false;
      } catch ( ... )  {
        if ( err ) *err = zypp::str::Str() << "Unknown error when parsing the file: " << fName;
        return false;
      }
      return true;
    }

    zypp::misc::testcase::TestcaseTrial::Node n;
    if ( !jobNode["job"] ) {
      if ( err ) {
        auto errStr = zypp::str::Str();
        const auto &mark = jobNode.Mark();
        errStr << "'job' key missing from trial node.";
        if ( !mark.is_null() ) {
          errStr << " Line: " << mark.line << " Col: " << mark.column << " pos: " << mark.pos;
        }
        *err = errStr;
      }
      return false;
    }

    for ( const auto &elem : jobNode ) {
      const std::string &key = elem.first.as<std::string>();
      const auto &data = elem.second;
      if ( key == "job" ) {
        n.name = data.as<std::string>();
      } else if ( key == "__content") {
        n.value = data.as<std::string>();
      } else {
        if( data.IsScalar() ) {
          n.properties.insert( { key, data.as<std::string>() } );
        } if ( data.IsSequence() ) {
          // if the type of a data field is a sequence, we treat all the elements in there
          // as sub elements. Just like in XML you can have sub nodes its the same here
          // the key name is ignored in those cases and can be chosen freely
          if ( !parseJobs( data, n.children, err ) )
            return false;
        } else if ( data.IsMap() ) {
          // if the type of a data field is a map, we build a child node from it.
          // Just like with sequence but a single field.
          // The key name is ignored in those cases and can be chosen freely
          if ( !parseSingleJob( data, n.children, err) )
            return false;
        } else {
          ERR << "Ignoring field " << key << " with unsupported type." << std::endl;
        }
      }
    }
    if constexpr ( isSubNode ) {
      target.push_back( std::make_shared<zypp::misc::testcase::TestcaseTrial::Node>( std::move(n) ) );
    } else {
      target.push_back( std::move(n) );
    }
    return true;
  }

  template <typename T>
  bool parseJobs  ( const YAML::Node &trial, std::vector<T> &target, std::string *err ) {
    for ( const auto &jobNode : trial ) {
      if ( !parseSingleJob( jobNode, target, err ) )
        return false;
    }
    return true;
  }

  bool parseTrial ( const YAML::Node &trial, zypp::misc::testcase::TestcaseTrial &target, std::string *err ) {
    MIL << "Parsing trials." << std::endl;
    return parseJobs( trial, target.nodes, err );
  }
}

#endif // ZYPP_MISC_YAMLTESTCASEHELPERS_H
