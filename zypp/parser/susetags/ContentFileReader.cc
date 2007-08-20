/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/ContentFileReader.cc
 *
*/
#include <iostream>
#include <sstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/parser/ParseException.h"

#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/CapFactory.h"

#include "zypp/ZConfig.h"

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
      //	CLASS NAME : ContentFileReader::Impl
      //
      /** ContentFileReader implementation. */
      struct ContentFileReader::Impl
      {
	public:
	  Impl( const ContentFileReader & parent_r )
	  : _parent( parent_r )
	  {}

	  data::Product & product()
	  {
	    if ( !_product )
	      _product = new data::Product;
	    return *_product;
	  }

	  RepoIndex & repoindex()
	  {
	    if ( !_repoindex )
	      _repoindex = new RepoIndex;
	    return *_repoindex;
	  }

	  bool hasProduct() const
	  { return _product; }

	  bool hasRepoIndex() const
	  { return _repoindex; }

	  data::Product_Ptr handoutProduct()
	  {
	    data::Product_Ptr ret;
	    ret.swap( _product );
	    _product = 0;
	    return ret;
	  }

	  RepoIndex_Ptr handoutRepoIndex()
	  {
	    RepoIndex_Ptr ret;
	    ret.swap( _repoindex );
	    _repoindex = 0;
	    return ret;
	  }

	public:
	  bool isRel( const std::string & rel_r ) const
	  {
	    try
	    {
	      Rel( rel_r );
	      return true;
	    }
	    catch (...)
	    {}
	    return false;
	  }

	  bool setUrlList( std::list<Url> & list_r, const std::string & value ) const
	  {
	    bool errors = false;
	    std::list<std::string> urls;
	    if ( str::split( value, std::back_inserter(urls) ) )
	    {
	      for ( std::list<std::string>::const_iterator it = urls.begin();
	            it != urls.end(); ++it )
	      {
		try
		{
		  list_r.push_back( *it );
		}
		catch( const Exception & excpt_r )
		{
		  WAR << *it << ": " << excpt_r << endl;
		  errors = true;
		}
	      }
	    }
	    return errors;
	  }

	  void setDependencies( data::DependencyList & deplist_r, const std::string & value ) const
	  {
	    std::list<std::string> words;
	    str::split( value, std::back_inserter( words ) );

	    for ( std::list<std::string>::const_iterator it = words.begin();
		  it != words.end(); ++it )
	    {
	      Resolvable::Kind kind( ResTraits<Package>::kind );

	      std::string name = *it;
	      std::string::size_type colon = name.find( ":" );
	      if ( colon != std::string::npos )
	      {
		std::string skind( name, 0, colon );
		name.erase( 0, colon+1 );

		if ( skind == ResTraits<Pattern>::kind )
		  kind = ResTraits<Pattern>::kind;
		else if ( skind == ResTraits<Patch>::kind )
		  kind = ResTraits<Patch>::kind;
		else if ( skind == ResTraits<Product>::kind )
		  kind = ResTraits<Product>::kind;
		else if ( skind == ResTraits<Selection>::kind )
		  kind = ResTraits<Selection>::kind;
		else if ( skind != ResTraits<Package>::kind )
		{
		  // colon but no kind ==> colon in a name
		  name = skind + ":" + name;
		}
	      }

	      // check for Rel:
	      std::list<std::string>::const_iterator next = it;
	      if ( ++next != words.end()
	           && (*next).find_first_of( "<>=" ) != std::string::npos )
	      {
		std::string op = *next;
		if ( ++next != words.end() )
		{
		  name += " ";
		  name += op;
		  name += " ";
		  name += *next;
		  it = next;
		}
	      }

	      // Add the dependency
	      deplist_r.insert( capability::parse( kind, name ) );
	    }
	  }

	  bool setFileCheckSum( std::map<std::string, CheckSum> & map_r, const std::string & value ) const
	  {
	    bool error = false;
	    std::vector<std::string> words;
	    if ( str::split( value, std::back_inserter( words ) ) == 3 )
	    {
	      map_r[words[2]] = CheckSum( words[0], words[1] );
	    }
	    else
	    {
	      error = true;
	    }
	    return error;
	  }

	public:
	  std::string _inputname;

	private:
	  const ContentFileReader & _parent;
	  data::Product_Ptr  _product;
	  RepoIndex_Ptr      _repoindex;
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ContentFileReader
      //
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::ContentFileReader
      //	METHOD TYPE : Ctor
      //
      ContentFileReader::ContentFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::~ContentFileReader
      //	METHOD TYPE : Dtor
      //
      ContentFileReader::~ContentFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::beginParse
      //	METHOD TYPE : void
      //
      void ContentFileReader::beginParse()
      {
	_pimpl.reset( new Impl(*this) );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::endParse
      //	METHOD TYPE : void
      //
      void ContentFileReader::endParse()
      {
	// consume oldData
	if ( _pimpl->hasProduct() )
	{
	  if ( _productConsumer )
	    _productConsumer( _pimpl->handoutProduct() );
	}
	if ( _pimpl->hasRepoIndex() )
	{
	  if ( _repoIndexConsumer )
	    _repoIndexConsumer( _pimpl->handoutRepoIndex() );
	}

	MIL << "[Content]" << endl;
	_pimpl.reset();
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::userRequestedAbort
      //	METHOD TYPE : void
      //
      void ContentFileReader::userRequestedAbort( unsigned lineNo_r )
      {
	ZYPP_THROW( AbortRequestException( errPrefix( lineNo_r ) ) );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::errPrefix
      //	METHOD TYPE : std::string
      //
      std::string ContentFileReader::errPrefix( unsigned lineNo_r,
	                                        const std::string & msg_r,
						const std::string & line_r ) const
      {
	return str::form( "%s:%u:%s | %s",
			  _pimpl->_inputname.c_str(),
			  lineNo_r,
			  line_r.c_str(),
			  msg_r.c_str() );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::parse
      //	METHOD TYPE : void
      //
      void ContentFileReader::parse( const InputStream & input_r,
				     const ProgressData::ReceiverFnc & fnc_r )
      {
	MIL << "Start parsing " << input_r << endl;
	if ( ! input_r.stream() )
	{
	  std::ostringstream s;
	  s << "Can't read bad stream: " << input_r;
	  ZYPP_THROW( ParseException( s.str() ) );
	}
	beginParse();
	_pimpl->_inputname = input_r.name();

	ProgressData ticks( makeProgressData( input_r ) );
	ticks.sendTo( fnc_r );
	if ( ! ticks.toMin() )
	  userRequestedAbort( 0 );

	Arch sysarch( ZConfig::instance().systemArchitecture() );

	iostr::EachLine line( input_r );
	for( ; line; line.next() )
	{
	  // strip 1st word from line to separate tag and value.
	  std::string value( *line );
	  std::string key( str::stripFirstWord( value, /*ltrim_first*/true ) );

	  if ( key.empty() || *key.c_str() == '#' ) // empty or comment line
	  {
	    continue;
	  }

	  // strip modifier if exists
	  std::string modifier;
	  std::string::size_type pos = key.rfind( '.' );
	  if ( pos != std::string::npos )
	  {
	    modifier = key.substr( pos+1 );
	    key.erase( pos );
	  }

	  //
	  // Product related data:
	  //
	  if ( key == "PRODUCT" )
	  {
	    std::replace( value.begin(), value.end(), ' ', '_' );
	    _pimpl->product().name = value;
	  }
	  else if ( key == "VERSION" )
	  {
	    _pimpl->product().edition = value;
	  }
	  else if ( key == "ARCH" )
	  {
	    // Default product arch is noarch. We update, if the
	    // ARCH.xxx tag is better than the current product arch
	    // and still compatible with the sysarch.
	    Arch carch( modifier );
	    if ( Arch::compare( _pimpl->product().arch, carch ) < 0
		 &&  carch.compatibleWith( sysarch ) )
	    {
	      _pimpl->product().arch = carch;
	    }
	  }
	  else if ( key == "DISTPRODUCT" )
	  {
	    _pimpl->product().distributionName = value;
	  }
	  else if ( key == "DISTVERSION" )
	  {
	    _pimpl->product().distributionEdition = value;
	  }
	  else if ( key == "VENDOR" )
	  {
	    _pimpl->product().vendor = value;
	  }
	  else if ( key == "LABEL" )
	  {
	    _pimpl->product().summary.setText( value, Locale(modifier) );
	  }
	  else if ( key == "SHORTLABEL" )
	  {
	    _pimpl->product().shortName.setText( value, Locale(modifier) );
	  }
	  else if ( key == "TYPE" )
	  {
	    _pimpl->product().type = value;
	  }
	  else if ( key == "RELNOTESURL" )
	  {
	    for( std::string::size_type pos = value.find("%a");
		 pos != std::string::npos;
		 pos = value.find("%a") )
	    {
	      value.replace( pos, 2, ZConfig::instance().systemArchitecture().asString() );
	    }
	    try
	    {
	      _pimpl->product().releasenotesUrl = value;
	    }
	    catch( const Exception & excpt_r )
	    {
	      WAR << errPrefix( line.lineNo(), excpt_r.asString(), *line ) << endl;
	    }
	  }
	  else if ( key == "UPDATEURLS" )
	  {
	    if ( _pimpl->setUrlList( _pimpl->product().updateUrls, value ) )
	    {
	      WAR << errPrefix( line.lineNo(), "Ignored malformed URL(s)", *line ) << endl;
	    }
	  }
	  else if ( key == "EXTRAURLS" )
	  {
	    if ( _pimpl->setUrlList( _pimpl->product().extraUrls, value ) )
	    {
	      WAR << errPrefix( line.lineNo(), "Ignored malformed URL(s)", *line ) << endl;
	    }
	  }
	  else if ( key == "OPTIONALURLS" )
	  {
	    if ( _pimpl->setUrlList( _pimpl->product().optionalUrls, value ) )
	    {
	      WAR << errPrefix( line.lineNo(), "Ignored malformed URL(s)", *line ) << endl;
	    }
	  }
	  else if ( key == "PREREQUIRES" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::PREREQUIRES], value );
	  }
	  else if ( key == "REQUIRES" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::REQUIRES], value );
	  }
	  else if ( key == "PROVIDES" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::PROVIDES], value );
	  }
	  else if ( key == "CONFLICTS" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::CONFLICTS], value );
	  }
	  else if ( key == "OBSOLETES" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::OBSOLETES], value );
	  }
	  else if ( key == "RECOMMENDS" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::RECOMMENDS], value );
	  }
	  else if ( key == "SUGGESTS" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::SUGGESTS], value );
	  }
	  else if ( key == "SUPPLEMENTS" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::SUPPLEMENTS], value );
	  }
	  else if ( key == "ENHANCES" )
	  {
	    _pimpl->setDependencies( _pimpl->product().deps[Dep::ENHANCES], value );
	  }
	  //
	  // ReppoIndex related data:
	  //
	  else if ( key == "DEFAULTBASE" )
	  {
	    _pimpl->repoindex().defaultBase = Arch(value);
	  }
	  else if ( key == "DESCRDIR" )
	  {
	    _pimpl->repoindex().descrdir = value;
	  }
	  else if ( key == "DATADIR" )
	  {
	    _pimpl->repoindex().datadir = value;
	  }
	  else if ( key == "FLAGS" )
	  {
	    str::split( value, std::back_inserter( _pimpl->repoindex().flags ) );
	  }
	  else if ( key == "KEY" )
	  {
	    if ( _pimpl->setFileCheckSum( _pimpl->repoindex().signingKeys, value ) )
	    {
	      ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Expected [KEY algorithm checksum filename]", *line ) ) );
	    }
	  }
	  else if ( key == "LANGUAGE" )
	  {
	    _pimpl->repoindex().language;
	  }
	  else if ( key == "LINGUAS" )
	  {
	    std::set<std::string> strval;
	    str::split( value, std::inserter( strval, strval.end() ) );
	    for ( std::set<std::string>::const_iterator it = strval.begin(); it != strval.end(); ++it )
	    {
	      _pimpl->repoindex().languages.push_back( Locale(*it) );
	    }
	  }
	  else if ( key == "META" )
	  {
	    if ( _pimpl->setFileCheckSum( _pimpl->repoindex().metaFileChecksums, value ) )
	    {
	      ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Expected [algorithm checksum filename]", *line ) ) );
	    }
	  }
          else if ( key == "HASH" )
	  {
	    if ( _pimpl->setFileCheckSum( _pimpl->repoindex().mediaFileChecksums, value ) )
	    {
	      ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Expected [algorithm checksum filename]", *line ) ) );
	    }
	  }
	  else if ( key == "TIMEZONE" )
	  {
	    _pimpl->repoindex().timezone = value;
	  }
	  else
	  { WAR << errPrefix( line.lineNo(), "Unknown tag", *line ) << endl; }


	  if ( ! ticks.set( input_r.stream().tellg() ) )
	    userRequestedAbort( line.lineNo() );
	}

	//
	// post processing
	//
	if ( _pimpl->hasProduct() )
	{
	  // Insert a "Provides" _dist_name" == _dist_version"
	  if ( ! _pimpl->product().distributionName.empty() )
	  {
	    _pimpl->product().deps[Dep::PROVIDES].insert(
		capability::parse( ResTraits<Product>::kind,
				   _pimpl->product().distributionName,
				   Rel::EQ,
				   _pimpl->product().distributionEdition ) );
	  }
	}
	if ( ! ticks.toMax() )
	  userRequestedAbort( line.lineNo() );

	endParse();
	MIL << "Done parsing " << input_r << endl;
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
