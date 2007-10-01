/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/CapabilityImpl.cc
 *
*/
#include <iostream>
#include <ext/hash_fun.h>

#include "zypp/base/Logger.h"
#include "zypp/base/Regex.h"
#include "zypp/base/Exception.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/capability/Capabilities.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////
    IMPL_PTR_TYPE(CapabilityImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CapabilityImpl::CapabilityImpl
    //	METHOD TYPE : Ctor
    //
    CapabilityImpl::CapabilityImpl( const Resolvable::Kind & refers_r )
    : _refers( refers_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CapabilityImpl::capImplOrderLess
    //	METHOD TYPE : bool
    //
    bool CapabilityImpl::capImplOrderLess( const constPtr & rhs ) const
    {
      return encode() < rhs->encode();
    }

    std::size_t CapabilityImpl::hash() const
    {
      std::size_t ret = __gnu_cxx::hash<const char*>()( encode().c_str() );
      return ret;
    }

    bool CapabilityImpl::same (const constPtr &rhs) const
    {
      /* refers and kind are known to be the same */
      return encode() == rhs->encode();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CapabilityImpl::capImplOrderLess
    //	METHOD TYPE : bool
    //
    std::ostream & CapabilityImpl::dumpOn( std::ostream & str ) const
    {
      return str << '[' << refers() << "] "
                 << '(' << kind() << ") "
                 << asString();
    }

    /** Assert a valid Resolvable::Kind. */
    static void assertResKind( const Resolvable::Kind & refers_r )
    {
      if ( refers_r == Resolvable::Kind() )
	ZYPP_THROW( Exception("Missing or empty  Resolvable::Kind in Capability") );
    }

    bool isEditionSpec( Rel op_r, const Edition & edition_r )
    {
      switch ( op_r.inSwitch() )
	{
	case Rel::ANY_e:
	  if ( edition_r != Edition::noedition )
	    WAR << "Operator " << op_r << " causes Edition "
	    << edition_r << " to be ignored." << endl;
	  return false;
	  break;

	case Rel::NONE_e:
	  ZYPP_THROW( Exception("Operator NONE is not allowed in Capability") );
	  break;

	case Rel::EQ_e:
	case Rel::NE_e:
	case Rel::LT_e:
	case Rel::LE_e:
	case Rel::GT_e:
	case Rel::GE_e:
	  return true;
	  break;
	}
      // SHOULD NOT GET HERE
      ZYPP_THROW( Exception("Unknown Operator NONE is not allowed in Capability") );
      return false; // not reached
    }

    bool isFileSpec( const std::string & name_r )
    {
      return *name_r.c_str() == '/';
    }

    bool isInterestingFileSpec( const std::string & name_r )
    {
      static str::smatch what;
      static const str::regex filenameRegex(
          "/(s?bin|lib(64)?|etc)/|^/usr/(games/|share/(dict/words|magic\\.mime)$)|^/opt/gnome/games/",
          str::regex::optimize|str::regex::nosubs );

      return str::regex_match( name_r, what, filenameRegex );
    }

    bool isSplitSpec( const std::string & name_r )
    {
      return name_r.find( ":/" ) != std::string::npos;
    }

    bool isHalSpec( const std::string & name_r )
    {
      return name_r.substr(0,4) == "hal(";
    }

    bool isModaliasSpec( const std::string & name_r )
    {
      return name_r.substr(0,9) == "modalias(";
    }

    bool isFilesystemSpec( const std::string & name_r )
    {
      return name_r.substr(0,11) == "filesystem(";
    }

    CapabilityImpl::Ptr buildFile( const Resolvable::Kind & refers_r,
                                          const std::string & name_r )
    {
      // NullCap check first:
      if ( name_r.empty() )
	{
	  // Singleton, so no need to put it into _uset !?
	  return capability::NullCap::instance();
	}

      assertResKind( refers_r );

      return new capability::FileCap( refers_r, name_r );
    }

    CapabilityImpl::Ptr buildNamed( const Resolvable::Kind & refers_r,
				           const std::string & name_r )
    {
      // NullCap check first:
      if ( name_r.empty() )
      {
        // Singleton, so no need to put it into _uset !?
        return capability::NullCap::instance();
      }

      assertResKind( refers_r );

      // file:    /absolute/path
      if ( isFileSpec( name_r ) )
      {
        return new capability::FileCap( refers_r, name_r );
      }
      if ( isFilesystemSpec( name_r ) )
      {
	return buildFilesystem( refers_r, name_r );
      }

      //split:   name:/absolute/path
      if (isSplitSpec (name_r))
      {
        static const str::regex  rx( "([^/]*):(/.*)" );
        str::smatch what;
        if( str::regex_match( name_r, what, rx ) )
        {
          return new capability::SplitCap( refers_r, what[1], what[2] );
        }
      }

      //name:    name
      return new capability::NamedCap( refers_r, name_r );
    }

    CapabilityImpl::Ptr buildVersioned( const Resolvable::Kind & refers_r,
				               const std::string & name_r,
				               Rel op_r,
				               const Edition & edition_r )
    {
      if ( isEditionSpec( op_r, edition_r ) )
	{
	  assertResKind( refers_r );

	  // build a VersionedCap
	  return new capability::VersionedCap( refers_r, name_r, op_r, edition_r );
	}
      //else
      // build a NamedCap

      return buildNamed( refers_r, name_r );
    }

    CapabilityImpl::Ptr buildHal( const Resolvable::Kind & refers_r,
                                         const std::string & name_r,
                                         Rel op_r,
                                         const std::string & value_r )
    {
      if ( op_r != Rel::ANY )
	{
	  ZYPP_THROW( Exception("Unsupported kind of Hal Capability '" + op_r.asString() + "'") );
	}

      //split:   hal(name) [op string]
      static const str::regex  rx( "hal\\(([^)]*)\\)" );
      str::smatch what;
      if( str::regex_match( name_r, what, rx ) )
	{
	  // Hal always refers to 'System' kind of Resolvable.
	  return new capability::HalCap( ResTraits<SystemResObject>::kind,
				    what[1] );
	}
      // otherwise
      ZYPP_THROW( Exception("Unsupported kind of Hal Capability '" + name_r + "'") );
      return NULL; // make gcc happy
    }

    CapabilityImpl::Ptr buildModalias( const Resolvable::Kind & refers_r,
                                              const std::string & name_r,
                                              Rel op_r,
                                              const std::string & value_r )
    {
      if ( op_r != Rel::ANY )
	{
	  ZYPP_THROW( Exception("Unsupported kind of Modalias Capability  '" + op_r.asString() + "'") );
	}

      //split:   modalias(name) [op string]
      static const str::regex  rx( "modalias\\(([^)]*)\\)" );
      str::smatch what;
      if( str::regex_match( name_r, what, rx ) )
	{
	  // Modalias always refers to 'System' kind of Resolvable
	  return new capability::ModaliasCap( ResTraits<SystemResObject>::kind,
                                         what[1] );
	}
      // otherwise
      ZYPP_THROW( Exception("Unsupported kind of Modalias Capability'" + name_r + "'") );
      return NULL; // make gcc happy
    }

    /******************************************************************
    **
    **	FUNCTION NAME : buildFilesystem
    **	FUNCTION TYPE : CapabilityImpl::Ptr
    */
    CapabilityImpl::Ptr buildFilesystem( const Resolvable::Kind & refers_r,
				       const std::string & name_r )
    {
      //split:   filesystem(name) [op string]
      static const str::regex  rx( "filesystem\\(([^)]*)\\)" );
      str::smatch what;
      if( str::regex_match( name_r, what, rx ) )
      {
	// Filesystem always refers to 'System' kind of Resolvable
	return new capability::FilesystemCap( ResTraits<SystemResObject>::kind,
					      what[1] );
      }
      // otherwise
      ZYPP_THROW( Exception("Unsupported kind of Filesystem Capability'" + name_r + "'") );
      return NULL; // make gcc happy
    }


    CapabilityImpl::Ptr parse( const Resolvable::Kind & refers_r,
			       const std::string & strval_r )
  try
    {
      if ( isHalSpec( strval_r ) )
	{
	  return buildHal( refers_r, strval_r );
        }
      if ( isModaliasSpec( strval_r ) )
        {
          return buildModalias( refers_r, strval_r );
        }
      if ( isFilesystemSpec( strval_r ) )
        {
          return buildFilesystem( refers_r, strval_r );
        }
      if ( isFileSpec( strval_r ) )
        {
          return buildFile( refers_r, strval_r );
        }

      // strval_r has at least two words which could make 'op edition'?
      // improve regex!
      static const str::regex  rx( "(.*[^ \t])([ \t]+)([^ \t]+)([ \t]+)([^ \t]+)" );
      str::smatch what;
      if( strval_r.find(' ') != std::string::npos
	  && str::regex_match( strval_r,what, rx ) )
        {
          Rel op;
          Edition edition;
          try
            {
              op = Rel(what[3]);
              edition = Edition(what[5]);
            }
          catch ( Exception & excpt )
            {
              // So they don't make valid 'op edition'
              ZYPP_CAUGHT( excpt );
              DBG << "Trying named cap for: " << strval_r << endl;
              // See whether it makes a named cap.
              return buildNamed( refers_r, strval_r );
            }

          // Valid 'op edition'
          return buildVersioned( refers_r,
                                 what[1], op, edition );
        }
      //else
      // not a VersionedCap
      return buildNamed( refers_r, strval_r );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return NULL; // not reached
    }


  CapabilityImpl::Ptr parse( const Resolvable::Kind & refers_r,
				const std::string & name_r,
				const std::string & op_r,
				const std::string & edition_r )
    try
    {
      if ( isHalSpec( name_r ) )
      {
        return buildHal( refers_r, name_r, Rel(op_r), edition_r );
      }
      if ( isModaliasSpec( name_r ) )
	{
	  return buildModalias( refers_r, name_r, Rel(op_r), edition_r );
	}
      // Try creating Rel and Edition, then parse
      return parse( refers_r, name_r, Rel(op_r), Edition(edition_r) );
    }
    catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return NULL; // not reached
    }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  CapabilityImpl::Ptr parse( const Resolvable::Kind & refers_r,
			     const std::string & name_r,
			     Rel op_r,
			     const Edition & edition_r )
  try
  {
      if ( isHalSpec( name_r ) )
      {
        return buildHal( refers_r, name_r, op_r, edition_r.asString() );
      }
      if ( isModaliasSpec( name_r ) )
      {
        return buildModalias( refers_r, name_r, op_r, edition_r.asString() );
      }
      return buildVersioned( refers_r, name_r, op_r, edition_r );
  }
  catch ( Exception & excpt )
  {
      ZYPP_RETHROW( excpt );
      return NULL; // not reached
  }

  ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
