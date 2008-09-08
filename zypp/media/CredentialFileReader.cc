/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/CredentialFileReader.cc
 *
 */
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/IOStream.h"

#include "zypp/media/CredentialFileReader.h"

using std::endl;

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
  //////////////////////////////////////////////////////////////////////

  CredentialFileReader::CredentialFileReader(
      const Pathname & crfile,
      const ProcessCredentials & callback)
  {
    InputStream is(crfile);

    for(iostr::EachLine in(is); in; in.next())
    {
      try
      {
        Url storedUrl(*in);

        AuthData_Ptr credentials;
        credentials.reset(
          new AuthData(storedUrl));

        if (credentials->valid())
          callback(credentials);
        else
          // report invalid record
          DBG << "invalid record: " << *in << endl;
      }
      catch (const url::UrlException &)
      {} // not a URL
      //! \todo this will need to be a bit more sophisticated to be able to pinpoint bad records
    }
  }

  CredentialFileReader::~CredentialFileReader()
  {}


    /////////////////////////////////////////////////////////////////
  } // media
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // zypp
///////////////////////////////////////////////////////////////////

