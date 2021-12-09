/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-media/auth/CredentialFileReader
 *
 */
#ifndef ZYPP_MEDIA_AUTH_CREDENTIALFILEREADER_H
#define ZYPP_MEDIA_AUTH_CREDENTIALFILEREADER_H

#include <zypp-core/base/Function.h>
#include <zypp-core/Url.h>
#include <zypp-core/Pathname.h>

#include <zypp-media/auth/AuthData>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace media
  {
    //////////////////////////////////////////////////////////////////////
    /// \class CredentialFileReader
    /// \brief Parse credentials files and catalogs
    class CredentialFileReader
    {
    public:
      /** Callback invoked for each entry found in the file.
       * Return \c false to abort parsing.
       */
      typedef function<bool(AuthData_Ptr &)> ProcessCredentials;

      CredentialFileReader( const Pathname & crfile_r, const ProcessCredentials & callback_r );
      ~CredentialFileReader();
    private:
      ProcessCredentials _callback;
    };
    //////////////////////////////////////////////////////////////////////

  } // namespace media
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif /* ZYPP_MEDIA_AUTH_CREDENTIALFILEREADER_H */
