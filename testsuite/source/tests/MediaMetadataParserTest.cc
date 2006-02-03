
#include "zypp/source/susetags/MediaMetadataParser.h"
#include "zypp/source/susetags/MediaPatchesMetadataParser.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

using namespace zypp;
using namespace zypp::source::susetags;

int main()
{
  MediaMetadataParser parser;
  MediaMetadataParser::MediaEntry entry;

  try {
    parser.parse(Pathname("mediafiles/SUSE-10.1-Beta3_i386_CD1_media1"), entry);
  
    parser.parse(Pathname("mediafiles/SLES-10-Beta3_i386_CD1_media1"), entry);
  
    parser.parse(Pathname("mediafiles/NOTTHERE"), entry);

    MediaPatchesMetadataParser patches_parser;
    MediaPatchesMetadataParser::MediaPatchesEntry patches_entry;
    patches_parser.parse("patchfiles/patches-1", patches_entry);

    patches_parser.parse("NOTTHERE", patches_entry);
  }
  catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
  }
  
  return 0;
}
