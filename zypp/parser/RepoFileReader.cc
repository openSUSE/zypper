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
#include <zypp/base/LogTools.h>
#include <zypp/base/String.h>
#include <zypp/base/StringV.h>
#include <zypp-core/base/InputStream>
#include <zypp-core/base/UserRequestException>

#include <zypp-core/parser/IniDict>
#include <zypp/parser/RepoFileReader.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace parser
  {
    ///////////////////////////////////////////////////////////////////
    namespace {

      ///////////////////////////////////////////////////////////////////
      /// \class RepoFileParser
      /// \brief Modified \ref IniDict to allow parsing multiple 'baseurl=' entries
      ///////////////////////////////////////////////////////////////////
      class RepoFileParser : public IniDict
      {
      public:
        RepoFileParser( const InputStream & is_r )
        { read( is_r ); }

        using IniDict::consume;	// don't hide overloads we don't redefine here

        virtual void consume( const std::string & section_r, const std::string & key_r, const std::string & value_r )
        {
          if ( key_r == "baseurl" )
          {
            _inMultiline = MultiLine::baseurl;
            storeUrl( _baseurls[section_r], value_r );
          }
          else if ( key_r == "gpgkey" )
          {
            _inMultiline = MultiLine::gpgkey;
            storeUrl( _gpgkeys[section_r], value_r );
          }
          else if ( key_r == "mirrorlist" )
          {
            _inMultiline = MultiLine::mirrorlist;
            storeUrl( _mirrorlist[section_r], value_r );
          }
          else if ( key_r == "metalink" )
          {
            _inMultiline = MultiLine::metalink;
            storeUrl( _metalink[section_r], value_r );
          }
          else
          {
            _inMultiline = MultiLine::none;
            IniDict::consume( section_r, key_r, value_r );
          }
        }

        virtual void garbageLine( const std::string & section_r, const std::string & line_r )
        {
          switch ( _inMultiline )
          {
            case MultiLine::baseurl:
              storeUrl( _baseurls[section_r], line_r );
              break;

            case MultiLine::gpgkey:
              storeUrl( _gpgkeys[section_r], line_r );
              break;

            case MultiLine::mirrorlist:
              storeUrl( _mirrorlist[section_r], line_r );
              break;

            case MultiLine::metalink:
              storeUrl( _metalink[section_r], line_r );
              break;

            case MultiLine::none:
              IniDict::garbageLine( section_r, line_r );	// throw
              break;
          }
        }

        std::list<Url> & baseurls( const std::string & section_r )
        { return _baseurls[section_r]; }

        std::list<Url> & gpgkeys( const std::string & section_r )
        { return _gpgkeys[section_r]; }

        std::list<Url> & mirrorlist( const std::string & section_r )
        { return _mirrorlist[section_r]; }

        std::list<Url> & metalink( const std::string & section_r )
        { return _metalink[section_r]; }

      private:
        void storeUrl( std::list<Url> & store_r, const std::string & line_r )
        {
          // #285: Fedora/dnf allows WS separated urls (and an optional comma)
          strv::splitRx( line_r, "[,[:blank:]]*[[:blank:]][,[:blank:]]*", [&store_r]( std::string_view w ) {
            if ( ! w.empty() )
              store_r.push_back( Url(std::string(w)) );
          });
        }

        enum class MultiLine { none, baseurl, gpgkey, mirrorlist, metalink };
        MultiLine _inMultiline = MultiLine::none;

        std::map<std::string,std::list<Url>> _baseurls;
        std::map<std::string,std::list<Url>> _gpgkeys;
        std::map<std::string,std::list<Url>> _mirrorlist;
        std::map<std::string,std::list<Url>> _metalink;
      };

    } //namespace
    ///////////////////////////////////////////////////////////////////

    /**
   * \short List of RepoInfo's from a file.
   * \param file pathname of the file to read.
   */
    static void repositories_in_stream( const InputStream &is,
                                        const RepoFileReader::ProcessRepo &callback,
                                        const ProgressData::ReceiverFnc &progress )
    {
      RepoFileParser dict(is);
      for_( its, dict.sectionsBegin(), dict.sectionsEnd() )
      {
        RepoInfo info;
        info.setAlias(*its);
        std::string proxy;
        std::string proxyport;

        for_( it, dict.entriesBegin(*its), dict.entriesEnd(*its) )
        {
          //MIL << (*it).first << endl;
          if (it->first == "name" )
            info.setName(it-> second);
          else if ( it->first == "enabled" )
            info.setEnabled( str::strToTrue( it->second ) );
          else if ( it->first == "priority" )
            info.setPriority( str::strtonum<unsigned>( it->second ) );
          else if ( it->first == "path" )
            info.setPath( Pathname(it->second) );
          else if ( it->first == "type" )
            ; // bsc#1177427 et.al.: type in a .repo file is legacy - ignore it and let RepoManager probe
          else if ( it->first == "autorefresh" )
            info.setAutorefresh( str::strToTrue( it->second ) );
          else if ( it->first == "gpgcheck" )
            info.setGpgCheck( str::strToTriBool( it->second ) );
          else if ( it->first == "repo_gpgcheck" )
            info.setRepoGpgCheck( str::strToTrue( it->second ) );
          else if ( it->first == "pkg_gpgcheck" )
            info.setPkgGpgCheck( str::strToTrue( it->second ) );
          else if ( it->first == "keeppackages" )
            info.setKeepPackages( str::strToTrue( it->second ) );
          else if ( it->first == "service" )
            info.setService( it->second );
          else if ( it->first == "proxy" )
          {
            // Translate it into baseurl queryparams
            // NOTE: The hack here does not add proxy to mirrorlist urls but the
            //       original code worked without complains, so keep it for now.
            static const str::regex ex( ":[0-9]+$" );	// portspec
            str::smatch what;
            if ( str::regex_match( it->second, what, ex ) )
            {
              proxy = it->second.substr( 0, it->second.size() - what[0].size() );
              proxyport = what[0].substr( 1 );
            }
            else
            {
              proxy = it->second;
            }
          }
          else
            ERR << "Unknown attribute in [" << *its << "]: " << it->first << "=" << it->second << " ignored" << endl;
        }

        for ( auto & url : dict.baseurls( *its ) )
        {
          if ( ! proxy.empty() && url.getQueryParam( "proxy" ).empty() )
          {
            url.setQueryParam( "proxy", proxy );
            url.setQueryParam( "proxyport", proxyport );
          }
          info.addBaseUrl( url );
        }

        if ( ! dict.gpgkeys( *its ).empty() )
          info.setGpgKeyUrls( std::move(dict.gpgkeys( *its )) );

        if ( ! dict.mirrorlist( *its ).empty() )
          info.setMirrorListUrls( std::move(dict.mirrorlist( *its )) );

        if ( ! dict.metalink( *its ).empty() )
          info.setMetalinkUrls( std::move(dict.metalink( *its )) );


        info.setFilepath(is.path());
        MIL << info << endl;
        // add it to the list.
        callback(info);
        //if (!progress.tick())
        //  ZYPP_THROW(AbortRequestException());
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : RepoFileReader
    //
    ///////////////////////////////////////////////////////////////////

    RepoFileReader::RepoFileReader( const Pathname & repo_file,
                                    const ProcessRepo & callback,
                                    const ProgressData::ReceiverFnc &progress )
      : _callback(callback)
    {
      repositories_in_stream(InputStream(repo_file), _callback, progress);
    }

    RepoFileReader::RepoFileReader( const InputStream &is,
                                    const ProcessRepo & callback,
                                    const ProgressData::ReceiverFnc &progress )
      : _callback(callback)
    {
      repositories_in_stream(is, _callback, progress);
    }

    RepoFileReader::~RepoFileReader()
    {}


    std::ostream & operator<<( std::ostream & str, const RepoFileReader & obj )
    {
      return str;
    }

  } // namespace parser
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
