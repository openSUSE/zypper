/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_source_yum_RepomdFileReader_H
#define zypp_source_yum_RepomdFileReader_H

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
    
     /**
      * Iterates through a repomd.xml file giving on each iteration
      * a \ref OnMediaLocation object with the resource and its
      * type ( primary, patches, filelists, etc ).
      * The iteration is done via a callback provided on
      * construction.
      *
      * \code
      * RepomdFileReader reader(repomd_file, 
      *                  bind( &SomeClass::callbackfunc, &object, _1, _2 ) );
      * \endcode
      */
      class RepomdFileReader
      {
      public:
       /**
        * Callback definition
        * first parameter is a \ref OnMediaLocation object with the resource
        * second parameter is the resource type
        */
        typedef function<bool( const OnMediaLocation &, const string & )> ProcessResource;
        
        enum Tag
        {
          tag_NONE,
          tag_Repomd,
          tag_Data,
          tag_Location,
          tag_CheckSum,
          tag_Timestamp,
          tag_OpenCheckSum
        };
        
       /**
        * Constructor
        * \param repomd_file is the repomd.xml file you want to read
        * \param callback is a function. \see RepomdFileReader::ProcessResource
        */
        RepomdFileReader( const Pathname &repomd_file, ProcessResource callback );
        
        /**
        * Callback provided to the XML parser. Don't use it.
        */
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


