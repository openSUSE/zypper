#include <zypp-curl/parser/MediaBlockList>
#include <zypp-curl/parser/MetaLinkParser>
#include <zypp/media/ZsyncParser.h>
#include <zypp-core/Pathname.h>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/fs/TmpPath.h>
#include <zypp-core/base/String.h>
#include <iostream>
#include <algorithm>

int main ( int argc, char *argv[] )
{
  if ( argc < 3 ) {
    std::cerr << "Usage: CalculateReusebleBlocks <metalinkfile> <deltafile>" << std::endl;
    return 1;
  }

  constexpr auto checkFileAccessible = []( const zypp::Pathname &path ){
    zypp::PathInfo pi( path );
    return ( pi.isExist() && pi.isFile() && pi.userMayR() );
  };

  zypp::Pathname metaLink( zypp::str::asString(argv[1]) );
  zypp::Pathname deltaFile( zypp::str::asString(argv[2]) );

  if ( !checkFileAccessible(metaLink) ) {
    std::cerr << "Metalink file at " << metaLink << " not accessible" << std::endl;
    return 1;
  }

  if ( !checkFileAccessible(deltaFile) ) {
    std::cerr << "Deltafile file at " << deltaFile << " not accessible" << std::endl;
    return 1;
  }

  zypp::media::MediaBlockList blocks;
  if ( zypp::str::hasSuffix( metaLink.asString(), "zsync") )  {
      zypp::media::ZsyncParser parser;
      try {
        parser.parse( metaLink.asString() );
        blocks = parser.getBlockList();
      } catch (const zypp::Exception& e) {
        std::cerr << "Failed to parse Metalink file: " << e << std::endl;
        return 1;
      } catch ( const std::exception &e ) {
        std::cerr << "Failed to parse Metalink file: " << e.what() << std::endl;
        return 1;
      }
  } else {
    zypp::media::MetaLinkParser parser;
    try {
      parser.parse( metaLink );
      blocks = parser.getBlockList();
    } catch(const zypp::Exception& e) {
      std::cerr << "Failed to parse Metalink file: " << e << std::endl;
      return 1;
    }
  }

  constexpr auto getDownloadSize = []( const zypp::media::MediaBlockList &blocks ){
    size_t size = 0;
    for ( size_t i = 0; i < blocks.numBlocks(); i++ ) {
      size += blocks.getBlock(i).size;
    }
    return size;
  };

  zypp::filesystem::TmpFile file;
  const auto numBlocksBefore = blocks.numBlocks();
  const zypp::ByteCount sizeBefore = getDownloadSize(blocks);

  std::cout << "Blocks parsed from Metalink file: " << numBlocksBefore << std::endl;
  if ( numBlocksBefore ) {
    zypp::AutoFILE f( fopen( file.path().c_str(), "w"));
    if ( *f ) {
      blocks.reuseBlocks( *f, deltaFile.asString() );
    }
  }

  const size_t numBlocksAfter = blocks.numBlocks();
  const zypp::ByteCount sizeAfter = getDownloadSize(blocks);

  std::cout << "Finished, reused " << ( numBlocksBefore - numBlocksAfter ) << " of " << numBlocksBefore << " blocks." << std::endl;
  std::cout << "Need to download " << sizeAfter << " of " << sizeBefore << std::endl;
  return 0;
}
