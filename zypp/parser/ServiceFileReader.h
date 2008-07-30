/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/ServiceFileReader.h
 *
*/
#ifndef ZYPP_REPO_SERVICEFILEREADER_H
#define ZYPP_REPO_SERVICEFILEREADER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/ProgressData.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class ServiceInfo;
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    /**
     * \short Read service data from a .service file
     *
     * After each service is read, a \ref ServiceInfo is prepared and \ref _callback
     * is called with the object passed in.
     *
     * The \ref _callback is provided on construction.
     *
     * \code
     * ServiceFileReader reader(service_file, 
     *                bind( &SomeClass::callbackfunc, &SomeClassInstance, _1 ) );
     * \endcode
     */
    class ServiceFileReader
    {
      friend std::ostream & operator<<( std::ostream & str, const ServiceFileReader & obj );
    public:
      
     /**
      * Callback definition.
      * First parameter is a \ref ServiceInfo object with the resource.
      *
      * Return false from the callback to get a \ref AbortRequestException
      * to be thrown and the processing to be cancelled.
      */
      typedef function< bool( const ServiceInfo & )> ProcessService;
      
      /** Implementation  */
      class Impl;

    public:
     /**
      * \short Constructor. Creates the reader and start reading.
      *
      * \param serviceFile A valid .repo file
      * \param callback Callback that will be called for each repository.
      *
      * \throws AbortRequestException If the callback returns false
      * \throws Exception If a error occurs at reading / parsing
      *
      */
      ServiceFileReader( const Pathname & serviceFile,
                      const ProcessService & callback);
     
      /**
       * Dtor
       */
      ~ServiceFileReader();
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ServiceFileReader Stream output */
    std::ostream & operator<<( std::ostream & str, const ServiceFileReader & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REPO_SERVICEFILEREADER_H
