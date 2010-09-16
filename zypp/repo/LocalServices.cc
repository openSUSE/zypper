/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/parser/IniDict.h"
#include "zypp/repo/LocalServices.h"
#include "zypp/ServiceInfo.h"
#include "zypp/PathInfo.h"
#include "zypp/ExternalProgram.h"

using std::endl;
using zypp::parser::IniDict;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    class LocalServices::Impl
    {
    public:
      static void loadServices( const Pathname &path,
          const LocalServices::ProcessService &callback );
    };

    void LocalServices::Impl::loadServices( const Pathname &path,
                                  const LocalServices::ProcessService & callback/*,
                                  const ProgressData::ReceiverFnc &progress*/ )
    {
      std::list<Pathname> entries;
      if (PathInfo(path).isExist())
      {
        if ( filesystem::readdir( entries, path, false ) != 0 )
        {          
          // TranslatorExplanation '%s' is a pathname
            ZYPP_THROW(Exception(str::form(_("Failed to read directory '%s'"), path.c_str())));
        }

        //str::regex allowedServiceExt("^\\.service(_[0-9]+)?$");
        for_(it, entries.begin(), entries.end() )
        {
          const char* argv[] = {
            (*it).c_str(),
            NULL
          };
          ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
          std::string line;
          for(line = prog.receiveLine(); !line.empty(); line = prog.receiveLine())
          {
            std::cout << line << endl;
          }
          prog.close();
        }
      }
    }

    LocalServices::LocalServices( const Pathname &path,
                                  const ProcessService & callback/*,
                                  const ProgressData::ReceiverFnc &progress */)
    {
      Impl::loadServices(path, callback/*, progress*/);
    }

    LocalServices::~LocalServices()
    {}

    std::ostream & operator<<( std::ostream & str, const LocalServices & obj )
    {
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
