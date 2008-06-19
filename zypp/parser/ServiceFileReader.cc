/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/RepoFileReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/parser/IniDict.h"
#include "zypp/parser/ServiceFileReader.h"
#include "zypp/Service.h"

using std::endl;
using zypp::parser::IniDict;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    class ServiceFileReader::Impl {
      public:
      static void parseServices( const Pathname &file,
          const ServiceFileReader::ProcessService& callback );
    };

    void ServiceFileReader::Impl::parseServices( const Pathname &file,
                                  const ServiceFileReader::ProcessService &callback/*,
                                  const ProgressData::ReceiverFnc &progress*/ )
    {
      InputStream is(file);
      parser::IniDict dict(is);
      for ( parser::IniDict::section_const_iterator its = dict.sectionsBegin();
            its != dict.sectionsEnd();
            ++its )
      {
        MIL << (*its) << endl;

        Service service(*its);

        for ( IniDict::entry_const_iterator it = dict.entriesBegin(*its);
              it != dict.entriesEnd(*its);
              ++it )
        {
          //MIL << (*it).first << endl;
          if (it->first == "url" )
            service.setUrl( Url (it->second) );
          else if ( it->first == "alias" )
            service.addRepo( it->second );
          else
            ERR << "Unknown attribute " << it->second << " ignored" << endl;
        }
        MIL << "Linking repo info with file " << file << endl;
        service.setLocation(file);
        // add it to the list.
        if ( !callback(service) )
          break;
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : RepoFileReader
    //
    ///////////////////////////////////////////////////////////////////

    ServiceFileReader::ServiceFileReader( const Pathname & repo_file,
                                    const ProcessService & callback/*,
                                    const ProgressData::ReceiverFnc &progress */)
    {
      Impl::parseServices(repo_file, callback/*, progress*/);
      //MIL << "Done" << endl;
    }

    ServiceFileReader::~ServiceFileReader()
    {}

    std::ostream & operator<<( std::ostream & str, const ServiceFileReader & obj )
    {
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
