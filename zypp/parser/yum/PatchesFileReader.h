/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_source_yum_PatchesFileReader_H
#define zypp_source_yum_PatchesFileReader_H

#include "zypp/Date.h"
#include "zypp/CheckSum.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/base/Function.h"
#include "zypp/parser/xml/Reader.h"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace source
  {
    namespace yum
    {
    
      class PatchesFileReader
      {
      public:
        typedef function<bool( const OnMediaLocation &, const string & )> ProcessResource;
        
        enum Tag
        {
          tag_NONE,
          tag_Patches,
          tag_Data,
          tag_Location,
          tag_CheckSum,
          tag_Timestamp,
          tag_OpenCheckSum
        };
        
        PatchesFileReader( const Pathname &repomd_file, ProcessResource callback );
        bool consumeNode( Reader & reader_r );
        
        private:
          OnMediaLocation _location;
          Tag _tag;
          std::string _type;
          ProcessResource _callback;
          CheckSum _checksum;
          std::string _checksum_type;
          Date _timestamp;
      };
    }
  }
}
#endif

