/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/RepoindexFileReader.cc
 * Implementation of repoindex.xml file reader.
 */
#include <iostream>
#include <unordered_map>

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/InputStream.h"

#include "zypp/Pathname.h"

#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/ParseException.h"

#include "zypp/RepoInfo.h"

#include "zypp/parser/RepoindexFileReader.h"


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

using std::endl;

namespace zypp
{
  namespace parser
  {
    using xml::Reader;
    using xml::XmlString;

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      class VarReplacer : private base::NonCopyable
      {
      public:
	/** */
	void setVar( const std::string & key_r, const std::string & val_r )
	{
	  MIL << "*** Inject " << key_r << " = " << val_r;
	  _vars[key_r] = replace( val_r );
	  MIL << " (" << _vars[key_r] << ")" << endl;
	}

	std::string replace( const std::string & val_r ) const
	{
	  std::string::size_type vbeg = val_r.find( "%{", 0 );
	  if ( vbeg == std::string::npos )
	    return val_r;

	  str::Str ret;
	  std::string::size_type cbeg = 0;
	  for( ; vbeg != std::string::npos; vbeg = val_r.find( "%{", vbeg ) )
	  {
	    std::string::size_type nbeg = vbeg+2;
	    std::string::size_type nend = val_r.find( "}", nbeg );
	    if ( nend == std::string::npos )
	    {
	      WAR << "Incomplete variable in '" << val_r << "'" << endl;
	      break;
	    }
	    const auto & iter = _vars.find( val_r.substr( nbeg, nend-nbeg ) );
	    if ( iter != _vars.end() )
	    {
	      if ( cbeg < vbeg )
		ret << val_r.substr( cbeg, vbeg-cbeg );
	      ret << iter->second;
	      cbeg = nend+1;
	    }
	    else
	      WAR << "Undefined variable %{" << val_r.substr( nbeg, nend-nbeg ) << "} in '" << val_r << "'" << endl;
	    vbeg = nend+1;
	  }
	  if ( cbeg < val_r.size() )
	    ret << val_r.substr( cbeg );

	  return ret;
	}
      private:
	std::unordered_map<std::string,std::string> _vars;
      };
    } // namespace
    ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : RepoindexFileReader::Impl
  //
  class RepoindexFileReader::Impl : private base::NonCopyable
  {
  public:
    /**
     * CTOR
     *
     * \see RepoindexFileReader::RepoindexFileReader(Pathname,ProcessResource)
     */
    Impl(const InputStream &is, const ProcessResource & callback);

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode( Reader & reader_r );

  private:
    bool getAttrValue( const std::string & key_r, Reader & reader_r, std::string & value_r )
    {
      const XmlString & s( reader_r->getAttribute( key_r ) );
      if ( s.get() )
      {
	value_r = _replacer.replace( s.asString() );
	return !value_r.empty();
      }
      value_r.clear();
      return false;
    }

  private:
    /** Function for processing collected data. Passed-in through constructor. */
    ProcessResource _callback;
    VarReplacer _replacer;
  };
  ///////////////////////////////////////////////////////////////////////

  RepoindexFileReader::Impl::Impl(const InputStream &is,
                                  const ProcessResource & callback)
    : _callback(callback)
  {
    Reader reader( is );
    MIL << "Reading " << is.path() << endl;
    reader.foreachNode( bind( &RepoindexFileReader::Impl::consumeNode, this, _1 ) );
  }

  // --------------------------------------------------------------------------

  /*
   * xpath and multiplicity of processed nodes are included in the code
   * for convenience:
   *
   * // xpath: <xpath> (?|*|+)
   *
   * if multiplicity is ommited, then the node has multiplicity 'one'.
   */

  // --------------------------------------------------------------------------

  bool RepoindexFileReader::Impl::consumeNode( Reader & reader_r )
  {
    if ( reader_r->nodeType() == XML_READER_TYPE_ELEMENT )
    {
      // xpath: /repoindex
      if ( reader_r->name() == "repoindex" )
      {
	while ( reader_r.nextNodeAttribute() )
	  _replacer.setVar( reader_r->localName().asString(), reader_r->value().asString() );
        return true;
      }

      // xpath: /repoindex/data (+)
      if ( reader_r->name() == "repo" )
      {
        RepoInfo info;
        // Set some defaults that are not contained in the repo information
        info.setAutorefresh( true );
	info.setEnabled(false);

	std::string attrValue;

	// required alias
	// mandatory, so we can allow it in var replacement without reset
	if ( getAttrValue( "alias", reader_r, attrValue ) )
	{
	  info.setAlias( attrValue );
	  _replacer.setVar( "alias", attrValue );
	}
	else
	  throw ParseException(str::form(_("Required attribute '%s' is missing."), "alias"));

        // required url
	// SLES HACK: or path, but beware of the hardcoded '/repo' prefix!
	{
	  std::string urlstr;
	  std::string pathstr;
	  getAttrValue( "url", reader_r, urlstr );
	  getAttrValue( "path", reader_r, pathstr );
	  if ( urlstr.empty() )
	  {
	    if ( pathstr.empty() )
	      throw ParseException(str::form(_("One or both of '%s' or '%s' attributes is required."), "url", "path"));
	    else
	      info.setPath( Pathname("/repo") / pathstr );
	  }
	  else
	  {
	    if ( pathstr.empty() )
	      info.setBaseUrl( Url(urlstr) );
	    else
	    {
	      Url url( urlstr );
	      url.setPathName( Pathname(url.getPathName()) / "repo" / pathstr );
	      info.setBaseUrl( url );
	    }
	  }
	}

        // optional name
        if ( getAttrValue( "name", reader_r, attrValue ) )
          info.setName( attrValue );

        // optional targetDistro
        if ( getAttrValue( "distro_target", reader_r, attrValue ) )
          info.setTargetDistribution( attrValue );

        // optional priority
        if ( getAttrValue( "priority", reader_r, attrValue ) )
          info.setPriority( str::strtonum<unsigned>( attrValue ) );


        // optional enabled
        if ( getAttrValue( "enabled", reader_r, attrValue ) )
          info.setEnabled( str::strToBool( attrValue, info.enabled() ) );

        // optional autorefresh
	if ( getAttrValue( "autorefresh", reader_r, attrValue ) )
	  info.setAutorefresh( str::strToBool( attrValue, info.autorefresh() ) );

        DBG << info << endl;

        // ignore the rest
        _callback(info);
        return true;
      }
    }

    return true;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : RepoindexFileReader
  //
  ///////////////////////////////////////////////////////////////////

  RepoindexFileReader::RepoindexFileReader(
      const Pathname & repoindex_file, const ProcessResource & callback)
    :
      _pimpl(new Impl(InputStream(repoindex_file), callback))
  {}

  RepoindexFileReader::RepoindexFileReader(
       const InputStream &is, const ProcessResource & callback )
    : _pimpl(new Impl(is, callback))
  {}

  RepoindexFileReader::~RepoindexFileReader()
  {}


  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
