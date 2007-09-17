/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/PackagesDuFileReader.cc
 *
*/
#include <iostream>
#include <cstdlib>
#include "zypp/base/Easy.h"
#include "zypp/base/Regex.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/ZYppFactory.h"

#include "zypp/parser/susetags/PackagesDuFileReader.h"
#include "zypp/parser/susetags/FileReaderBaseImpl.h"

using std::endl;
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser::susetags"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PackagesDuFileReader::Impl
      //
      /** PackagesDuFileReader implementation. */
      class PackagesDuFileReader::Impl : public BaseImpl
      {
	public:
	  Impl( const PackagesDuFileReader & parent_r )
	  : BaseImpl( parent_r )
	  {
            _mounts = getZYpp()->getPartitions();

            for ( DiskUsageCounter::MountPointSet::const_iterator it = _mounts.begin();
                  it != _mounts.end();
                  ++ it )
            {
              MIL << "Partition " << *it << endl;
            }

          }

	  virtual ~Impl()
	  {}

	  bool hasPackage() const
	  { return _pkgData; }

	  bool hasSourcepackage() const
	  { return _srcpkgData; }

	  data::Package_Ptr handoutPackage()
	  {
	    data::Package_Ptr ret;
	    ret.swap( _pkgData );
	    _srcpkgData = 0;
	    _data       = 0;
	    return ret;
	  }

	  data::SrcPackage_Ptr handoutSourcepackage()
	  {
	    data::SrcPackage_Ptr ret;
	    ret.swap( _srcpkgData );
	    _pkgData = 0;
	    _data    = 0;
	    return ret;
	  }

	public: // single tags
	  /** Consume =Ver:. */
	  void consumeVer( const SingleTagPtr & tag_r )
	  { /* NOP */; }

	  /** Consume =Pkg:. */
	  void consumePkg( const SingleTagPtr & tag_r )
	  {
	    std::vector<std::string> words;
	    if ( str::split( tag_r->value, std::back_inserter(words) ) != 4 )
	    {
	      ZYPP_THROW( error( tag_r, "Expected [name version release arch]") );
	    }

	    if ( words[3] == "src" || words[3] == "nosrc")
	    {
	      ++_c_srcpkg;
	      _data = _srcpkgData = new data::SrcPackage;
	      _pkgData = 0;
	      // _data->arch is arch_noarch per default
	    }
	    else
	    {
	      ++_c_pkg;
	      _data = _pkgData = new data::Package;
	      _srcpkgData = 0;
	      _data->arch = Arch( words[3] );
	    }
	    _data->name    = words[0];
	    _data->edition = Edition( words[1],words[2] );
	  }


	public: // multi tags
	  /** Consume +Dir:. */
	  void consumeDir( const MultiTagPtr & tag_r )
	  {
            static str::regex  sizeEntryRX( "^(.*/)[[:space:]]+([[:digit:]]+)[[:space:]]+([[:digit:]]+)[[:space:]]+([[:digit:]]+)[[:space:]]+([[:digit:]]+)[[:space:]]*$" );
            static str::smatch what;

            _data->diskusage;

            for_( it, tag_r->value.begin(), tag_r->value.end() )
            {
              if ( str::regex_match( *it, what, sizeEntryRX ) )
              {
                bool skip = true;
                DiskUsage::Entry entry( what[1],
                                        str::strtonum<unsigned>(what[2]) + str::strtonum<unsigned>(what[3]),
                                        str::strtonum<unsigned>(what[4]) + str::strtonum<unsigned>(what[5]) );

		// the partitioning is not known or we are running in instsys (see #308659)
                if ( _mounts.empty() || (getenv("YAST_IS_RUNNING") && (std::string(getenv("YAST_IS_RUNNING")) == "instsys" )))
                {
                  // if no mount information, cut off deeper directory
                  // levels in DiskUsage::Entry to reduce data size.
                  unsigned level             = 3; // number of '/' incl. a trailing one
                  std::string::size_type pos = 0; // we store absolute pathnames
                  while ( --level && pos != std::string::npos )
                  {
                    pos = entry.path.find( "/", pos+1 );
                  }
                  if ( pos != std::string::npos )
                  {
                    // found 'level' number of '/'es.
                    skip = ( entry.path[pos+1] != '\0' );
                  }
                  else
                  {
                    // less than 'level' number of '/'es.
                    skip = false;
                  }
                }
                else
                {
                  // Store entries, if they are equal to or parent of a mountpoint.
                  for ( DiskUsageCounter::MountPointSet::reverse_iterator mit = _mounts.rbegin();
                        mit != _mounts.rend();
                        ++ mit )
                  {
                    // Entry.path has leading and trailing '/' (asserted by DiskUsage::Entry).
                    // So we append a '/' to the mountpoint and test for prefix_r.
                    if ( str::hasPrefix( (mit->dir+"/"), entry.path ) )
                    {
                      skip = false;
                      break;
                    }
                  }
                }
                // try next entry
                if ( skip )
                  continue;

                _data->diskusage.add(entry);
              }
              else
              {
                ZYPP_THROW( error( tag_r, "Expected du-entry [path num num num num]") );
              }
            }
	  }

	public:
	  DefaultIntegral<unsigned,0> _c_pkg;
	  DefaultIntegral<unsigned,0> _c_srcpkg;

	private:
	  data::Packagebase_Ptr   _data;
	  data::Package_Ptr       _pkgData;
	  data::SrcPackage_Ptr    _srcpkgData;
          DiskUsageCounter::MountPointSet _mounts;
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PackagesDuFileReader
      //
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesDuFileReader::PackagesDuFileReader
      //	METHOD TYPE : Ctor
      //
      PackagesDuFileReader::PackagesDuFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesDuFileReader::~PackagesDuFileReader
      //	METHOD TYPE : Dtor
      //
      PackagesDuFileReader::~PackagesDuFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesDuFileReader::beginParse
      //	METHOD TYPE : void
      //
      void PackagesDuFileReader::beginParse()
      {
	_pimpl.reset( new Impl(*this) );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesDuFileReader::consume
      //	METHOD TYPE : void
      //
      void PackagesDuFileReader::consume( const SingleTagPtr & tag_r )
      {
#define TAGN(V)   tag_r->name == #V
#define TAGFWD(V) ( TAGN(V) ) _pimpl->consume##V( tag_r )

	if ( TAGN( Pkg ) )
	{
	  // consume old data
	  if ( _pimpl->hasPackage() )
	  {
	    if ( _pkgConsumer )
	      _pkgConsumer( _pimpl->handoutPackage() );
	  }
	  else if ( _pimpl->hasSourcepackage() )
	  {
	    if ( _srcPkgConsumer )
	      _srcPkgConsumer( _pimpl->handoutSourcepackage() );
	  }
	  // start new data
	  _pimpl->consumePkg( tag_r );
	}
	else if TAGFWD( Ver );
	else
	{ WAR << errPrefix( tag_r, "Unknown tag" ) << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesDuFileReader::consume
      //	METHOD TYPE : void
      //
      void PackagesDuFileReader::consume( const MultiTagPtr & tag_r )
      {
	if TAGFWD( Dir );
	else
	{ WAR << errPrefix( tag_r, "Unknown tag" ) << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesDuFileReader::lastData
      //	METHOD TYPE : void
      //
      void PackagesDuFileReader::endParse()
      {
        // consume oldData
	if ( _pimpl->hasPackage() )
	{
	  if ( _pkgConsumer )
	    _pkgConsumer( _pimpl->handoutPackage() );
	}
	else if ( _pimpl->hasSourcepackage() )
	{
	  if ( _srcPkgConsumer )
	    _srcPkgConsumer( _pimpl->handoutSourcepackage() );
	}
	MIL << "[PackagesDu]" << "(" << _pimpl->_c_pkg << "|" << _pimpl->_c_srcpkg << ")" << endl;
	_pimpl.reset();
      }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
