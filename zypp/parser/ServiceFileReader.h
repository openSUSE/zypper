/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/ServiceFileReader.h
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

  class Service;
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    /**
     * \short Read repository data from a .repo file
     *
     * After each repo is read, a \ref RepoInfo is prepared and \ref _callback
     * is called with the object passed in.
     *
     * The \ref _callback is provided on construction.
     *
     * \code
     * ServiceFileReader reader(repo_file, 
     *                bind( &SomeClass::callbackfunc, &SomeClassInstance, _1, _2 ) );
     * \endcode
     */
    class ServiceFileReader
    {
      friend std::ostream & operator<<( std::ostream & str, const ServiceFileReader & obj );
    public:
      
     /**
      * Callback definition.
      * First parameter is a \ref RepoInfo object with the resource
      * second parameter is the resource type.
      *
      * Return false from the callback to get a \ref AbortRequestException
      * to be thrown and the processing to be cancelled.
      */
      typedef function< bool( const Service & )> ProcessService;
      
      /** Implementation  */
      class Impl;

    public:
     /**
      * \short Constructor. Creates the reader and start reading.
      *
      * \param repo_file A valid .repo file
      * \param callback Callback that will be called for each repository.
      * \param progress Optional progress function. \see ProgressData
      *
      * \throws AbortRequestException If the callback returns false
      * \throws Exception If a error occurs at reading / parsing
      *
      */
      ServiceFileReader( const Pathname & repo_file,
                      const ProcessService & callback/*,
                      const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc()*/);
     
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
