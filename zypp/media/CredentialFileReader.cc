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
#include "zypp/parser/IniDict.h"

#include "zypp/media/CredentialFileReader.h"

using std::endl;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"


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
    parser::IniDict dict(is);
    for (parser::IniDict::section_const_iterator its = dict.sectionsBegin();
         its != dict.sectionsEnd();
         ++its)
    {
      Url storedUrl;
      if (!its->empty())
      {
        try { storedUrl = Url(*its); }
        catch (const url::UrlException &)
        {
          ERR << "invalid URL '" << *its << "' in credentials in file: "
              << crfile << endl;
          continue;
        }
      }

      AuthData_Ptr credentials;
      credentials.reset(new AuthData());

      // set url
      if (storedUrl.isValid())
        credentials->setUrl(storedUrl);

      for (parser::IniDict::entry_const_iterator it = dict.entriesBegin(*its);
           it != dict.entriesEnd(*its);
           ++it)
      {
        if (it->first == "username")
          credentials->setUsername(it->second);
        else if (it->first == "password")
          credentials->setPassword(it->second);
        else
          ERR << "Unknown attribute in [" << crfile << "]: "
              << it->second << " ignored" << endl;
      }

      if (credentials->valid())
        callback(credentials);
      else
        ERR << "invalid credentials in file: " << crfile << endl;
    } // sections
  }


  CredentialFileReader::~CredentialFileReader()
  {}


    /////////////////////////////////////////////////////////////////
  } // media
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // zypp
///////////////////////////////////////////////////////////////////

