/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/WebpinResultFileReader.h
 *
*/
#ifndef ZYPP_PARSER_WEBPINRESULTFILEREADER_H
#define ZYPP_PARSER_WEBPINRESULTFILEREADER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/ProgressData.h"
#include "zypp/Pathname.h"
#include "zypp/ws/WebpinResult.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  
  namespace parser
  { /////////////////////////////////////////////////////////////////
      
   namespace ws
   {
       
    /**
     * \short Read packages and repository search result data from
     * from webpin web search results.
     *
     * For each result, a \ref WebpinResult is prepared and \ref _callback
     * is called with the object passed in.
     *
     * The \ref _callback is provided on construction.
     *
     * \code
     * WebpinResultFileReader reader(repo_file, 
     *     bind( &SomeClass::callbackfunc, &SomeClassInstance, _1, _2 ) );
     * \endcode
     */
    class WebpinResultFileReader
    {
        friend std::ostream & operator<<( std::ostream & str, const WebpinResultFileReader & obj );
    public:
      
     /**
      * Callback definition.
      * First parameter is a \ref WebpinResult object.
      *
      * Return false from the callback to get a \ref AbortRequestException
      * to be thrown and the processing to be cancelled.
      */
        typedef function< bool( const zypp::ws::WebpinResult & )> ProcessWebpinResult;
      
    public:
     /**
      * \short Constructor. Creates the reader and start reading.
      *
      * \param result_file Valid result XML file from Webpin
      * \param callback Callback that will be called for each repository.
      * \param progress Optional progress function. \see ProgressData
      *
      * \throws AbortRequestException If the callback returns false
      * \throws Exception If a error occurs at reading / parsing
      *
      */
      WebpinResultFileReader( const Pathname &result_file,
                      const ProcessWebpinResult & callback/*,
                      const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc()*/);
     
      /**
       * Dtor
       */
      ~WebpinResultFileReader();
    private:
      class Impl;
      RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates WebpinResultFileReader Stream output */
    std::ostream & operator<<( std::ostream & str, const WebpinResultFileReader & obj );

   } //namespace ws
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_WEBPINRESULTFILEREADER_H
