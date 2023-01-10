// A small tool to test the downloader backend in zypp
// takes a yaml file as first argument that has a list of downloadable files and possible deltafiles to
// use when requesting the file:
// - url: "https://dlmirror.foo/file1"
//   delta: "/path/to/delta1"
// - url: "https://dlmirror.foo/file2"
//   delta: "/path/to/file2"


#include <zypp/MediaSetAccess.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp-core/ManagedFile.h>
#include <zypp-core/fs/TmpPath.h>
#include <yaml-cpp/yaml.h>

#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <iostream>

namespace po = boost::program_options;

struct DLEntry {
  zypp::Url _url;
  zypp::Pathname _deltaFile;
};

// progress for downloading a file
struct DownloadProgressReportReceiver : public zypp::callback::ReceiveReport<zypp::media::DownloadProgressReport>
{
  DownloadProgressReportReceiver()
  {
    connect();
  }

  virtual void start( const zypp::Url & uri, zypp::Pathname localfile )
  {
    assert(!_display);
    std::cout << "Starting download of: " << uri << " to " << localfile << std::endl;
    _display = std::make_unique<boost::progress_display>( 100 );
  }

  virtual bool progress(int value, const zypp::Url & uri, double drate_avg, double drate_now)
  {
    auto increase = value - _display->count();
    if ( increase > 0 && ( increase + _display->count() ) <= _display->expected_count() ) {
      (*_display) += increase;
    }
    return true;
  }

  virtual DownloadProgressReport::Action
  problem( const zypp::Url & uri, DownloadProgressReport::Error error, const std::string & description )
  {
    std::cerr << "Problem while downloading file " << description << std::endl;
    return DownloadProgressReport::ABORT;
  }

  // used only to finish, errors will be reported in media change callback (libzypp 3.20.0)
  virtual void finish( const zypp::Url & uri, Error error, const std::string & konreason )
  {
    _display.reset();
    std::cout << std::endl;
  }

private:
  std::unique_ptr<boost::progress_display> _display;
};



int main ( int argc, char *argv[] )
{
  // force the use of the new downloader code
  setenv("ZYPP_MEDIANETWORK", "1", 1);

  auto appname = zypp::Pathname::basename( argv[0] );
  po::positional_options_description p;
  p.add("control-file", 1);

  po::options_description visibleOptions("Allowed options");
  visibleOptions.add_options()
    ("help", "produce help message")
    ("mediabackend" , po::value<std::string>()->default_value("legacy"), "Select the mediabackend to use, possible options: legacy, provider")
    ("target-dir"   , po::value<std::string>()->default_value("."), "Directory where to download the files to." );

  po::options_description positionalOpts;
  positionalOpts.add_options ()
    ("control-file" , po::value<std::string>(), "Control file to read the urls from" );

  po::options_description cmdline_options;
  cmdline_options.add(visibleOptions).add(positionalOpts);

  const auto &usage = [&](){
    std::cerr << "Usage: " << appname << " [OPTIONS] <control-file>" << std::endl;
    std::cerr << visibleOptions << std::endl;
  };

  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv).
               options(cmdline_options).positional(p).run(), vm);
    po::notify(vm);
  } catch ( const po::error & e ) {
    std::cerr << e.what () << std::endl;
    usage();
  }

  if ( vm.count ("help") ) {
    usage();
    return 0;
  }

  if ( !vm.count("control-file") ) {
    std::cerr << "Missing the required control-file argument.\n" << std::endl;
    usage();
    return 1;
  }

  const auto &cFInfo = zypp::PathInfo( vm["control-file"].as<std::string>());
  if ( !cFInfo.isExist() || !cFInfo.userMayR() ) {
    std::cerr << "Control file " << cFInfo.path () << " is not accessible" << std::endl;
    usage();
    return 1;
  }

  if ( !vm.count("mediabackend")
       || ( vm["mediabackend"].as<std::string>() != "legacy" && vm["mediabackend"].as<std::string>() != "provider" ) ) {
    std::cerr << "Invalid value given for mediabackend option: " << vm["mediabackend"].as<std::string>() <<".\n";
    usage();
    return 1;
  }

  if ( !vm.count("target-dir") ) {
    std::cerr << "Targetdir not initialized, this is a bug.\n" << std::endl;
    usage();
    return 1;
  }

  const zypp::PathInfo targetDir( vm["target-dir"].as<std::string>() );
  if ( !targetDir.isDir() || !targetDir.userMayRWX() ) {
    std::cerr << "Target directory is not accessible." << std::endl;
    usage();
    return 1;
  }

  constexpr auto makeError = [] ( const std::string &str ) {
    std::cerr << str << std::endl;
    return 1;
  };

  std::vector<DLEntry> entries;

  YAML::Node control;
  try {
    control = YAML::LoadFile( cFInfo.path().asString() );

    if ( control.Type() != YAML::NodeType::Sequence )
      return makeError("Root node must be of type Sequence.");

    for ( const auto &dlFile : control ) {
      const auto &url = dlFile["url"];
      if ( !url ) {
        return makeError("Each element in the control sequence needs a \"url\" field.");
      }
      if ( url.Type() != YAML::NodeType::Scalar )
        return makeError("Field 'url' must be a scalar.");

      zypp::Url dlUrl( url.as<std::string> () );
      zypp::Pathname deltaFile;

      const auto &delta = dlFile["delta"];
      if ( delta ) {
        if ( delta.Type() != YAML::NodeType::Scalar )
          return makeError("Field 'delta' must be a scalar.");

        deltaFile = zypp::Pathname( delta.as<std::string>() );
      }

      entries.push_back ( { std::move(dlUrl), std::move(deltaFile) } );
    }
  } catch ( YAML::Exception &e ) {
    std::cerr << "YAML exception: " << e.what() << std::endl;
    return 1;
  } catch ( const std::exception &e )  {
    std::cerr << "Error when parsing the control file: " << e.what() << std::endl;
    return 1;
  } catch ( ... )  {
    std::cerr << "Unknown error when parsing the control file" << std::endl;
    return 1;
  }

  std::vector< zypp::ManagedFile > files;

  std::cout << "Using yaml: " << cFInfo.path() << "\n"
            << "Downloading to: " << targetDir.path ().realpath ()<< "\n"
            << "Using backend: " << vm["mediabackend"].as<std::string>() << "\n"
            << "Nr of downloads: " << entries.size() << "\n" << std::endl;

  if ( vm["mediabackend"].as<std::string>() == "provider" ) {
    std::cerr << "The provider support is not yet implemented, please try again later." << std::endl;
    return 1;
  }


  DownloadProgressReportReceiver receiver;

  if ( entries.size() ) {
    for ( const auto &e : entries ) {
      try {

        zypp::Url url(e._url);
        zypp::Pathname path(url.getPathName());

        url.setPathName ("/");
        zypp::MediaSetAccess access(url);

        zypp::ManagedFile locFile( targetDir.path().realpath() / path.dirname(), zypp::filesystem::unlink );
        zypp::filesystem::assert_dir( locFile->dirname() );

        auto file = access.provideFile(
          zypp::OnMediaLocation(path, 1)
            .setOptional  ( false )
            .setDeltafile ( e._deltaFile ));

        //prevent the file from being deleted when MediaSetAccess gets out of scope
        if ( zypp::filesystem::hardlinkCopy(file, locFile) != 0 )
          ZYPP_THROW( zypp::Exception("Can't copy file from " + file.asString() + " to " +  locFile->asString() ));

        std::cout << "File downloaded: " << e._url << std::endl;
        files.push_back ( locFile );

      } catch ( const zypp::Exception &e ) {
        std::cerr << "Failed to download file: " << e << std::endl;
      }
    }
  }

  //std::cout << "Done, press any key to continue" << std::endl;
  //getchar();
  return 0;
}
