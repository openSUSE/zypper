
#include <zypp/parser/tagfile/MediaMetadataParser.h> 
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

using namespace zypp;
using namespace zypp::parser::tagfile;

int main()
{
  MediaMetadataParser parser;
  MediaMetadataParser::MediaEntry entry;

  parser.parse(Pathname("tagfiles/media/media-1-SLES-9-i386-RC5.txt"), entry);
  DBG << "==============================================================" << std::endl;
  
  parser.parse(Pathname("tagfiles/media/media-1-SUSE-9.3-DVD-RC3.txt"), entry);
  //DBG << "arch: " << entry.arch["x86_64"].size() << std::endl;
  return 0;
}
