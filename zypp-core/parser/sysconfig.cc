/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-core/parser/sysconfig.cc
 *
*/

#include "sysconfig.h"

#include <iostream>
#include <fstream>

#include <zypp-core/base/Logger.h>
#include <zypp-core/base/String.h>
#include <zypp-core/base/Regex.h>
#include <zypp-core/base/IOStream.h>
#include <zypp-core/base/InputStream>
#include <zypp-core/Pathname.h>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/fs/TmpPath.h>

using std::endl;
using namespace zypp::base;

namespace zypp {
  namespace base {
    namespace sysconfig {

      std::map<std::string,std::string> read( const Pathname & _path )
      {
        DBG << "Load '" << _path << "'" << endl;
        std::map<std::string,std::string> ret;

        std::string line;
        std::ifstream in( _path.asString().c_str() );
        if ( in.fail() ) {
          WAR << "Unable to load '" << _path << "'" << endl;
          return ret;
        }

        while( getline( in, line ) ) {
          if ( *line.begin() != '#' ) {

            std::string::size_type pos = line.find( '=', 0 );

            if ( pos != std::string::npos ) {

              std::string key = str::trim( line.substr( 0, pos ) );
              std::string value = str::trim( line.substr( pos + 1, line.length() - pos - 1 ) );

              if ( value.length() >= 2
                   && *(value.begin()) == '"'
                   && *(value.rbegin()) == '"' )
              {
                value = value.substr( 1, value.length() - 2 );
              }
              if ( value.length() >= 2
                   && *(value.begin()) == '\''
                   && *(value.rbegin()) == '\'' )
              {
                value = value.substr( 1, value.length() - 2 );
              }
              XXX << "KEY: '" << key << "' VALUE: '" << value << "'" << endl;
              ret[key] = value;

            } // '=' found

          } // not comment

        } // while getline
        MIL << "done reading '" << _path << "'" << endl;
        return ret;
      }

      bool write( const Pathname & path_r, const std::string & key_r, const std::string & val_r, const std::string & newcomment_r )
      {
        if ( key_r.empty() )
        {
          WAR << "Empty key in write " << path_r << endl;
          return false;
        }

        PathInfo pi( path_r );
        if ( ! pi.isFile() )
          ZYPP_THROW( Exception( str::Str() << path_r << ": " << Errno(ENOENT) ) );
        if ( ! pi.userMayRW() )
          ZYPP_THROW( Exception( str::Str() << path_r << ": " << Errno(EACCES) ) );

        bool found = false;
        filesystem::TmpFile tmpf( filesystem::TmpFile::makeSibling( path_r ) );
        {
          str::regex regex( "^[ \t]*"+key_r+"[ \t]*=" );
          std::ofstream o( tmpf.path().c_str() );
          iostr::forEachLine( InputStream( path_r ),
                              [&]( int num_r, std::string line_r )->bool
                              {
                                if ( !found && regex.matches( line_r ) )
                                {
                                  o << key_r << '=' << val_r << endl;
                                  found = true;
                                  MIL << path_r << ": " << key_r << '=' << val_r << " changed on line " << num_r << endl;
                                }
                                else
                                  o << line_r << endl;
                                return true;
                              } );
          if ( !found )
          {
            if ( newcomment_r.empty() )
            {
              WAR << path_r << ": " << key_r << '=' << val_r << " can not be added (no comment provided)." << endl;
            }
            else
            {
              std::vector<std::string> lines;
              str::split( newcomment_r, std::back_inserter(lines), "\r\n" );
              o << endl;
              for ( const std::string & line : lines )
              {
                if ( line[0] != '#' )
                  o << "# ";
                o << line << endl;
              }
              o << key_r << '=' << val_r << endl;
              found = true;
              MIL << path_r << ": " << key_r << '=' << val_r << " appended. " << endl;
            }
          }

          if ( ! o )
            ZYPP_THROW( Exception( str::Str() << tmpf.path() << ": " << Errno(EIO) ) );
        }

        // If everything is fine, exchange the files:
        int res = exchange( tmpf.path(), path_r );
        if ( res )
        {
          ZYPP_THROW( Exception( str::Str() << tmpf.path() << ": " << Errno(res) ) );
        }
        return found;
      }

      bool writeStringVal( const Pathname & path_r, const std::string & key_r, const std::string & val_r, const std::string & newcomment_r )
      {
        return write( path_r, key_r, str::Str() << '"' << str::escape( val_r, '"' )<< '"', newcomment_r );
      }

    } // namespace sysconfig
  } // namespace base
} // namespace zypp
