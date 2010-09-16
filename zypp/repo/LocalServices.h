/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_LOCALSERVICES_H
#define ZYPP_REPO_LOCALSERVICES_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/ProgressData.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class ServiceInfo;
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    class LocalServices
    {
      friend std::ostream & operator<<( std::ostream & str, const LocalServices& obj );
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
      LocalServices(const Pathname &path,
                    const ProcessService & callback);
     
      /**
       * Dtor
       */
      ~LocalServices();
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ServiceFileReader Stream output */
    std::ostream & operator<<( std::ostream & str, const LocalServices & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REPO_LOCALSERVICES_H
