/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/ProductConfReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/PathInfo.h"

#include "zypp/parser/ProductConfReader.h"
#include "zypp/parser/IniDict.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    std::ostream & operator<<( std::ostream & str, const ProductConfData & obj )
    {
      str << str::form( "|product|%s|%s|%s|%s|%s|",
                        obj.name().c_str(),
                        obj.edition().c_str(),
                        obj.arch().c_str(),
                        obj.distName().c_str(),
                        obj.distEdition().c_str() );
      return str;
    }

    bool ProductConfReader::parse( const InputStream & input_r ) const
    {
      parser::IniDict dict( input_r );

      for_( sit, dict.sectionsBegin(), dict.sectionsEnd() )
      {
        std::string section( *sit );
        ProductConfData data;

        // last word is arch (%a substituted by system arch)
        std::string word( str::stripLastWord( section, /*rtrim_first*/true ) );
        if ( word.empty() )
        {
          ERR << "Malformed section without architecture ignored [" << *sit << "]" << endl;
          continue;
        }
        data._arch = Arch( word == "%arch" ? "" : word );

        // one but last word is edition
        word = str::stripLastWord( section, /*rtrim_first*/true );
        if ( word.empty() )
        {
          ERR << "Malformed section without edition ignored [" << *sit << "]" << endl;
          continue;
        }
        data._edition = Edition( word );

        // all the rest is name
        if ( section.empty() )
        {
          ERR << "Malformed section without name ignored [" << *sit << "]" << endl;
          continue;
        }
        data._name = IdString( section );

        // now the attributes:
        for_( it, dict.entriesBegin(*sit), dict.entriesEnd(*sit) )
        {
          std::string entry( it->first );
          std::string value( it->second );
          if ( entry == "distproduct" )
          {
            data._distName = IdString( value );
          }
          else if ( entry == "distversion" )
          {
            data._distEdition = Edition( value );
          }
          else
          {
            WAR << "Unknown Key: "<< entry << "=" << value << endl;
          }
        }

        if ( _consumer && ! _consumer( data ) )
        {
          MIL << "Stop consuming " << input_r << endl;
          return false;
        }
      }
      return true;
    }

    bool ProductConfReader::scanDir( const Consumer & consumer_r, const Pathname & dir_r )
    {
      DBG << "+++ scanDir " << dir_r << endl;

      std::list<Pathname> retlist;
      int res = filesystem::readdir( retlist, dir_r, /*dots*/false );
      if ( res != 0 )
      {
        WAR << "scanDir " << dir_r << " failed (" << res << ")" << endl;
        return true;
      }

      ProductConfReader reader( consumer_r );
      for_( it, retlist.begin(), retlist.end() )
      {
        if ( it->extension() == ".prod" && ! reader.parse( *it ) )
        {
          return false; // consumer_r request to stop parsing.
        }
      }
      DBG << "--- scanDir " << dir_r << endl;
      return true;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
