/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/CredentialFileReader.h
 *
 */
#ifndef ZYPP_MEDIA_CREDENTIALFILEREADER_H
#define ZYPP_MEDIA_CREDENTIALFILEREADER_H

#include "zypp/base/Function.h"
#include "zypp/Url.h"
#include "zypp/Pathname.h"

#include "zypp/media/MediaUserAuth.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace media
  { /////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME : CredentialFileReader 
  //
  class CredentialFileReader
  {
  public:
    /**
      * Callback definition.
      * First parameter is the \ref Url with which the credentials are
      * associated, the second are the credentials.
      *
      * Return false from the callback to get a \ref AbortRequestException
      * to be thrown and the processing to be cancelled.
      */
    typedef function<bool(AuthData_Ptr &)> ProcessCredentials;

    CredentialFileReader(const Pathname & crfile,
                         const ProcessCredentials & callback);
    ~CredentialFileReader();
  private:
    ProcessCredentials _callback;
  };
  //////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
  } // media
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // zypp
///////////////////////////////////////////////////////////////////

#endif /* ZYPP_MEDIA_CREDENTIALFILEREADER_H */
