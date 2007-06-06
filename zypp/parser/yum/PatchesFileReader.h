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
  namespace parser
  {
    namespace yum
    {


  /**
   * Iterates through a patches.xml file giving on each iteration
   * a \ref OnMediaLocation object with the resource and its
   * patch id.
   * The iteration is done via a callback provided on
   * construction.
   *
   * \code
   * PatchesFileReader reader(patches_file, 
   *                  bind( &SomeClass::callbackfunc, &object, _1, _2 ) );
   * \endcode
   */
  class PatchesFileReader
  {
  public:
    
   /**
    * Callback definition
    * first parameter is a \ref OnMediaLocation object with the resource
    * second parameter is the patch id.
    */
    typedef function<bool( const OnMediaLocation &, const string & )> ProcessResource;
    
    enum Tag
    {
      tag_NONE,
      tag_Patches,
      tag_Patch,
      tag_Location,
      tag_CheckSum,
      tag_Timestamp,
      tag_OpenCheckSum
    };
    
   /**
    * Constructor
    * \param patches_file is the patches.xml file you want to read
    * \param callback is a function. \see PatchesFileReader::ProcessResource
    */
    PatchesFileReader( const Pathname &patches_file, ProcessResource callback );
    
    private:
    /**
    * Callback provided to the XML parser. Don't use it.
    */
    bool consumeNode( Reader & reader_r );
    
    private:
      OnMediaLocation _location;
      Tag _tag;
      std::string _id;
      ProcessResource _callback;
      CheckSum _checksum;
      std::string _checksum_type;
      Date _timestamp;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*zypp_source_yum_PatchesFileReader_H*/

// vim: set ts=2 sts=2 sw=2 et ai:
