#include <iostream>
#include <zypp/base/IOStream.h>
#include <zypp/url/UrlUtils.h>

static std::string doEncode( const std::string & str_r )
{ return zypp::url::encode( str_r ); }

static std::string doDecode( const std::string & str_r )
{ return zypp::url::decode( str_r ); }

int main( int argc, const char * argv[] )
{
  bool encode( true );

  --argc,++argv;
  if ( argc )
  {
    if ( *argv == std::string( "-d" ) || *argv == std::string( "--decode" ) )
      encode = false;
    else if ( *argv == std::string( "-h" ) || *argv == std::string( "--help" ) )
    {
      std::cout << "Usage: percent-encode [OPTION]" << std::endl;
      std::cout << "Read lines from stdin and write them percent encoded to stdout." << std::endl;
      std::cout << "" << std::endl;
      std::cout << "Option:" << std::endl;
      std::cout << " -d, --decode  Decode lines read from stdin instead of encoding them." << std::endl;
      std::cout << " -h --help     Print this message." << std::endl;
      return 0;
    }
  }

  std::string (*coder)( const std::string & str_r ) = encode ? doEncode: doDecode;
  for( zypp::iostr::EachLine in( std::cin ); in; in.next() )
  {
    std::cout << coder( *in ) << std::endl;
  }
  return 0;
}
