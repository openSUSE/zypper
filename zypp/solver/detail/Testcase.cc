/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/solver/detail/Testcase.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <boost/iterator/function_output_iterator.hpp>

#define ZYPP_USE_RESOLVER_INTERNALS

#include <zypp/solver/detail/Testcase.h>
#include <zypp/base/Logger.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/GzStream.h>
#include <zypp/base/String.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/ReferenceCounted.h>

#include <zypp/AutoDispose.h>
#include <zypp/ZConfig.h>
#include <zypp/PathInfo.h>
#include <zypp/ResPool.h>
#include <zypp/Repository.h>
#include <zypp/target/modalias/Modalias.h>

#include <zypp/sat/detail/PoolImpl.h>
#include <zypp/solver/detail/Resolver.h>
#include <zypp/solver/detail/SystemCheck.h>

#include <yaml-cpp/yaml.h>

extern "C" {
#include <solv/testcase.h>
}

using std::endl;

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      //---------------------------------------------------------------------------

      Testcase::Testcase()
        :dumpPath("/var/log/YaST2/solverTestcase")
      {}

      Testcase::Testcase(const std::string & path)
        :dumpPath(path)
      {}

      Testcase::~Testcase()
      {}

      bool Testcase::createTestcase(Resolver & resolver, bool dumpPool, bool runSolver)
      {
        PathInfo path (dumpPath);

        if ( !path.isExist() ) {
          if (zypp::filesystem::assert_dir (dumpPath)!=0) {
            ERR << "Cannot create directory " << dumpPath << endl;
            return false;
          }
        } else {
          if (!path.isDir()) {
            ERR << dumpPath << " is not a directory." << endl;
            return false;
          }
          // remove old stuff if pool will be dump
          if (dumpPool)
            zypp::filesystem::clean_dir (dumpPath);
        }

        if (runSolver) {
          zypp::base::LogControl::TmpLineWriter tempRedirect;
          zypp::base::LogControl::instance().logfile( dumpPath +"/y2log" );
          zypp::base::LogControl::TmpExcessive excessive;

          resolver.resolvePool();
        }

        ResPool pool 	= resolver.pool();
        PoolItemList	items_to_install;
        PoolItemList 	items_to_remove;
        PoolItemList 	items_locked;
        PoolItemList 	items_keep;


        const std::string slvTestcaseName = "testcase.t";
        const std::string slvResult       = "solver.result";

        zypp::AutoDispose<const char **> repoFileNames( testcase_mangle_repo_names( resolver.get()->pool ),
          [ nrepos = resolver.get()->pool->nrepos ]( auto **x ){
            if (!x) return;
            for ( int i = 1; i < nrepos; i++ )
                solv_free((void *)x[i]);
            solv_free((void *)x);
        });

        if ( ::testcase_write( resolver.get(), dumpPath.c_str(), TESTCASE_RESULT_TRANSACTION | TESTCASE_RESULT_PROBLEMS, slvTestcaseName.c_str(), slvResult.c_str() ) == 0 ) {
          ERR << "Failed to write solv data, aborting." << endl;
          return false;
        }

        // HACK: directly access sat::pool
        const sat::Pool & satpool( sat::Pool::instance() );

        YAML::Emitter yOut;

        const auto addTag = [&]( const std::string & tag_r, bool yesno_r = true ){
          yOut << YAML::Key << tag_r << YAML::Value << yesno_r;
        };

        yOut << YAML::BeginMap  << YAML::Key << "version" << YAML::Value << "1.0";

        yOut << YAML::Key << "setup" << YAML::Value << YAML::BeginMap;

        yOut << YAML::Key << "channels";
        yOut << YAML::Value << YAML::BeginSeq;

        std::set<Repository::IdType> repos;
        for ( const PoolItem & pi : pool ) {
          if ( pi.status().isToBeInstalled()
               && !(pi.status().isBySolver())) {
            items_to_install.push_back( pi );
          }
          if ( pi.status().isKept()
               && !(pi.status().isBySolver())) {
            items_keep.push_back( pi );
          }
          if ( pi.status().isToBeUninstalled()
               && !(pi.status().isBySolver())) {
            items_to_remove.push_back( pi );
          }
          if ( pi.status().isLocked()
               && !(pi.status().isBySolver())) {
            items_locked.push_back( pi );
          }

          const auto &myRepo = pi.repository();
          const auto &myRepoInfo = myRepo.info();
          if ( repos.find( myRepo.id()) == repos.end() ) {
            repos.insert( myRepo.id() );
            yOut << YAML::Value << YAML::BeginMap;
            yOut << YAML::Key << "alias" << YAML::Value << myRepo.alias();
            yOut << YAML::Key << "url" << YAML::BeginSeq;
            for ( auto itUrl = myRepoInfo.baseUrlsBegin(); itUrl != myRepoInfo.baseUrlsEnd(); ++itUrl ) {
              yOut << YAML::Value << itUrl->asString();
            }
            yOut << YAML::EndSeq;
            yOut << YAML::Key << "path" << YAML::Value << myRepoInfo.path().asString();
            yOut << YAML::Key << "type" << YAML::Value << myRepoInfo.type().asString();
            yOut << YAML::Key << "generated" << YAML::Value << myRepo.generatedTimestamp().form( "%Y-%m-%d %H:%M:%S" );
            yOut << YAML::Key << "outdated" << YAML::Value << myRepo.suggestedExpirationTimestamp().form( "%Y-%m-%d %H:%M:%S" );
            yOut << YAML::Key << "priority" << YAML::Value << myRepoInfo.priority();
            yOut << YAML::Key << "file" << YAML::Value << str::Format("%1%.repo.gz") % repoFileNames[myRepo.id()->repoid];

            yOut << YAML::EndMap;
          }

        }

        yOut << YAML::EndSeq;

        yOut << YAML::Key << "arch" << YAML::Value << ZConfig::instance().systemArchitecture().asString() ;
        yOut << YAML::Key << "solverTestcase" << YAML::Value << slvTestcaseName ;
        yOut << YAML::Key << "solverResult" << YAML::Value << slvResult ;

        // RequestedLocales
        const LocaleSet & addedLocales( satpool.getAddedRequestedLocales() );
        const LocaleSet & removedLocales( satpool.getRemovedRequestedLocales() );
        const LocaleSet & requestedLocales( satpool.getRequestedLocales() );

        yOut << YAML::Key << "locales" << YAML::Value << YAML::BeginSeq ;
        for ( Locale l : requestedLocales ) {
          yOut << YAML::Value << YAML::BeginMap;
          yOut << YAML::Key << "fate" << YAML::Value << ( addedLocales.count(l) ? "added" : "" ) ;
          yOut << YAML::Key << "name" << YAML::Value << l.asString() ;
          yOut << YAML::EndMap;
        }

        for ( Locale l : removedLocales ) {
          yOut << YAML::Value << YAML::BeginMap;
          yOut << YAML::Key << "fate" << YAML::Value << "removed" ;
          yOut << YAML::Key << "name" << YAML::Value << l.asString() ;
          yOut << YAML::EndMap;
        }
        yOut << YAML::EndSeq; // locales

        // helper lambda to write a list of elements into a external file instead of the main file
        const auto &writeListOrFile = [&]( const std::string &name, const auto &list, const auto &callback ) {
          if ( list.size() > 10 ) {
            const std::string fName = str::Format("zypp-%1%.yaml") % name;
            yOut << YAML::Key << name << YAML::Value << fName;

            YAML::Emitter yOutFile;
            callback( yOutFile, list );

            std::ofstream fout( dumpPath+"/"+fName );
            fout << yOutFile.c_str();
          } else {
            yOut << YAML::Key << name << YAML::Value ;
            callback( yOut, list );
          }
        };

        // AutoInstalled
        const auto &writeAutoInst = [] ( YAML::Emitter &out, const auto &autoInstalledList ) {
          out << YAML::BeginSeq;
          for ( IdString::IdType n : autoInstalledList ) {
            out << YAML::Value << IdString(n).asString() ;
          }
          out << YAML::EndSeq;
        };
        writeListOrFile( "autoinst", satpool.autoInstalled(), writeAutoInst );

        // ModAlias
        const auto &writeModalias = []( YAML::Emitter &out, const auto &modAliasList ){
          out << YAML::BeginSeq;
          for ( const auto &modAlias : modAliasList ) {
            out << YAML::Value << modAlias ;
          }
          out << YAML::EndSeq;
        };
        writeListOrFile( "modalias", target::Modalias::instance().modaliasList(), writeModalias );

        // Multiversion
        const auto &writeMultiVersion = [] ( YAML::Emitter &out, const auto &multiversionList ) {
          out << YAML::BeginSeq;
          for ( const auto &multiver : multiversionList ) {
            out << YAML::Value << multiver ;
          }
          out << YAML::EndSeq;
        };
        writeListOrFile( "multiversion", ZConfig::instance().multiversionSpec(), writeMultiVersion );


        yOut << YAML::Key << "resolverFlags" << YAML::Value << YAML::BeginMap;
        yOut << YAML::Key << "focus" << YAML::Value << asString( resolver.focus() );

        addTag( "ignorealreadyrecommended", resolver.ignoreAlreadyRecommended() );
        addTag( "onlyRequires",             resolver.onlyRequires() );
        addTag( "forceResolve",             resolver.forceResolve() );

        addTag( "cleandepsOnRemove",        resolver.cleandepsOnRemove() );

        addTag( "allowDowngrade",           resolver.allowDowngrade() );
        addTag( "allowNameChange",          resolver.allowNameChange() );
        addTag( "allowArchChange",          resolver.allowArchChange() );
        addTag( "allowVendorChange",        resolver.allowVendorChange() );

        addTag( "dupAllowDowngrade",        resolver.dupAllowDowngrade() );
        addTag( "dupAllowNameChange",       resolver.dupAllowNameChange() );
        addTag( "dupAllowArchChange",       resolver.dupAllowArchChange() );
        addTag( "dupAllowVendorChange",     resolver.dupAllowVendorChange() );


        yOut << YAML::EndMap; // resolverFlags
        yOut << YAML::EndMap; // setup

        yOut << YAML::Key << "trials" << YAML::Value << YAML::BeginSeq;

        yOut << YAML::Value << YAML::BeginMap << YAML::Key << "trial" << YAML::Value;

        yOut << YAML::BeginSeq;

        const auto &writeJobsToFile = [&]( const std::string &fName, const auto &data, const auto &cb ){
          yOut << YAML::Value << YAML::BeginMap;
          yOut << YAML::Key << "include" << YAML::Value << fName;
          yOut << YAML::EndMap;

          YAML::Emitter yOutFile;
          yOutFile << YAML::BeginSeq;
          cb( yOutFile, data );
          yOutFile << YAML::EndSeq;

          std::ofstream fout( dumpPath+"/"+fName );
          fout << yOutFile.c_str();
        };

        // Multiversion
        const auto &writePoolItemJobs = []( const std::string &jobName ){
          return [ &jobName ] ( YAML::Emitter &yOut, const PoolItemList &poolItems, bool shortInfo = false ) {
            for ( const PoolItem & pi : poolItems ) {
              yOut << YAML::Value << YAML::BeginMap;

              std::stringstream status;
              status << pi.status();

              yOut << YAML::Key << "job"     << YAML::Value << jobName
                   << YAML::Key << "kind"    << YAML::Value << pi.kind().asString()
                   << YAML::Key << "name"    << YAML::Value << pi.name()
                   << YAML::Key << "status"  << YAML::Value << status.str();
              if ( !shortInfo ) {
                yOut << YAML::Key << "channel" << YAML::Value << pi.repoInfo().alias()
                     << YAML::Key << "arch"    << YAML::Value << pi.arch().asString()
                     << YAML::Key << "version" << YAML::Value << pi.edition().version()
                     << YAML::Key << "release" << YAML::Value << pi.edition().release();
              }
              yOut << YAML::EndMap;
            }
          };
        };

        const auto &writeMapJob = []( YAML::Emitter &yOut, const std::string &name, const std::map<std::string, std::string> &values = std::map<std::string, std::string>() ){
          yOut << YAML::Value << YAML::BeginMap;
          yOut << YAML::Key << "job"     << YAML::Value << name;
          for ( const auto &v : values )
            yOut << YAML::Key << v.first << YAML::Value << v.second;
          yOut << YAML::EndMap;
        };

        writePoolItemJobs("install")( yOut, items_to_install );
        writePoolItemJobs("keep")( yOut, items_keep );
        writePoolItemJobs("uninstall")( yOut, items_to_remove, true );

        if ( items_locked.size() )
          writeJobsToFile("zypp-locks.yaml", items_locked, writePoolItemJobs("lock") );

        for ( const auto &v : resolver.extraRequires() )
          writeMapJob( yOut, "addRequire", { { "name", v.asString() } } );
        for ( const auto &v : SystemCheck::instance().requiredSystemCap() )
          writeMapJob( yOut, "addRequire", { { "name", v.asString() } } );

        for ( const auto &v : resolver.extraConflicts() )
          writeMapJob( yOut, "addConflict", { { "name", v.asString() } } );
        for ( const auto &v : SystemCheck::instance().conflictSystemCap() )
          writeMapJob( yOut, "addConflict", { { "name", v.asString() } } );

        for ( const auto &v : resolver.upgradeRepos() )
          writeMapJob( yOut, "upgradeRepo", { { "name", v.alias() } } );

        if ( resolver.isUpgradeMode() )
          writeMapJob( yOut, "distupgrade" );

        if ( resolver.isUpdateMode() )
          writeMapJob( yOut, "update" );

        if ( resolver.isVerifyingMode() )
          writeMapJob( yOut, "verify" );

        yOut << YAML::EndSeq;
        yOut << YAML::EndMap; // trial
        yOut << YAML::EndSeq; // trials list
        yOut << YAML::EndMap; // trials
        yOut << YAML::EndMap; // root

        std::ofstream fout( dumpPath+"/zypp-control.yaml" );
        fout << yOut.c_str();

        return true;
      }
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
