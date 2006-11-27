/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapFactory.cc
 *
*/
#include <iostream>
#include <functional>
#include <set>
#include <map>

#include <stdint.h>
#include <ext/hash_set>
#include <ext/hash_map>
#include <ext/hash_fun.h>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/String.h"
#include "zypp/base/Counter.h"

#include "zypp/CapFactory.h"
#include "zypp/capability/Capabilities.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace
{ /////////////////////////////////////////////////////////////////
  using ::zypp::Resolvable;
  using ::zypp::capability::CapabilityImpl;
  using ::zypp::capability::CapImplOrder;

  struct CapImplHashFun
  {
    size_t operator() ( const CapabilityImpl::Ptr & p ) const
    {
      return __gnu_cxx::hash<const char*>()( p->encode().c_str() );
    }
  };

  struct CapImplHashEqual
  {
    bool operator() ( const CapabilityImpl::Ptr & lhs, const CapabilityImpl::Ptr & rhs ) const
    {
      return (    lhs->encode() == rhs->encode()
               && lhs->kind()   == rhs->kind()
               && lhs->refers() == rhs->refers() );
    }
  };

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

struct SuperFastHash
{
  unsigned int operator()( const CapabilityImpl::Ptr &c ) const
  {
    const char *data = c->encode().c_str();
    return hash( data, strlen(data));
  }
  
  unsigned int operator()(const char *data ) const
  {
    return hash( data, strlen(data));
  }
  
  static inline unsigned int hash(const char * data, int len)
  {
  unsigned int hash = len, tmp;
  int rem;
  
      if (len <= 0 || data == NULL) return 0;
  
      rem = len & 3;
      len >>= 2;
  
      /* Main loop */
      for (;len > 0; len--) {
          hash  += get16bits (data);
          tmp    = (get16bits (data+2) << 11) ^ hash;
          hash   = (hash << 16) ^ tmp;
          data  += 2*sizeof (uint16_t);
          hash  += hash >> 11;
      }
  
      /* Handle end cases */
      switch (rem) {
          case 3: hash += get16bits (data);
                  hash ^= hash << 16;
                  hash ^= data[sizeof (uint16_t)] << 18;
                  hash += hash >> 11;
                  break;
          case 2: hash += get16bits (data);
                  hash ^= hash << 11;
                  hash += hash >> 17;
                  break;
          case 1: hash += *data;
                  hash ^= hash << 10;
                  hash += hash >> 1;
      }
  
      /* Force "avalanching" of final 127 bits */
      hash ^= hash << 3;
      hash += hash >> 5;
      hash ^= hash << 4;
      hash += hash >> 17;
      hash ^= hash << 25;
      hash += hash >> 6;
  
      return hash;
  }
};  
  
  /** Set of unique CapabilityImpl. */
  //typedef std::set<CapabilityImpl::Ptr,CapImplOrder> USet;
  typedef __gnu_cxx::hash_set<CapabilityImpl::Ptr, CapImplHashFun, CapImplHashEqual> USet;
  //typedef __gnu_cxx::hash_map<const char *, CapabilityImpl::Ptr, __gnu_cxx::hash<const char *>, __gnu_cxx::equal_to<const char *> > CapabilityCache;
  //typedef __gnu_cxx::hash_map<const char *, CapabilityImpl::Ptr, SuperFastHash, __gnu_cxx::equal_to<const char *> > CapabilityCache;
  typedef std::map<const char *, CapabilityImpl::Ptr> CapabilityCache;
  typedef std::map<Resolvable::Kind, CapabilityCache> CapabilityKindCache;
  
  /** Set to unify created capabilities.
   *
   * This is to unify capabilities. Each CapabilityImpl created
   * by CapFactory, must be inserted into _uset, and the returned
   * CapabilityImpl::Ptr has to be uset to create the Capability.
  */
  USet _uset;
  
  /**
   * Cache which stores pre parsed capabilities
   */
  CapabilityKindCache _cap_cache;
  std::string _current_key;

  /** Each CapabilityImpl created in CapFactory \b must be wrapped.
   *
   * Immediately wrap \a allocated_r, and unified by inserting it into
   * \c _uset. Each CapabilityImpl created by CapFactory, \b must be
   * inserted into _uset, by calling usetInsert.
   *
   * \return CapabilityImpl_Ptr referencing \a allocated_r (or an
   * eqal representation, allocated is deleted then).
  */
  CapabilityImpl::Ptr usetInsert( CapabilityImpl * allocated_r )
  {
    CapabilityImpl::Ptr ptr(allocated_r);
    _cap_cache[ptr->refers()][_current_key.c_str()] = ptr;
    return *(_uset.insert( ptr ).first);
  }

  /** Collect USet statistics.
   * \ingroup DEBUG
  */
  struct USetStatsCollect : public std::unary_function<CapabilityImpl::constPtr, void>
  {
    typedef ::zypp::Counter<unsigned> Counter;

    Counter _caps;
    std::map<CapabilityImpl::Kind,Counter> _capKind;
    std::map<Resolvable::Kind,Counter>     _capRefers;
    static unsigned _cache_hits;
    static unsigned _cache_misses;

    void operator()( const CapabilityImpl::constPtr & cap_r )
    {
      //DBG << *cap_r << endl;
      ++_caps;
      ++(_capKind[cap_r->kind()]);
      ++(_capRefers[cap_r->refers()]);
    }

    std::ostream & dumpOn( std::ostream & str ) const
    {
      str << "  Cache hits: " << _cache_hits << endl;
      str << "  Cache miss: " << _cache_misses << endl;
      str << "  Capabilities total: " << _caps << endl;
      str << "  Capability kinds:" << endl;
      for ( std::map<CapabilityImpl::Kind,Counter>::const_iterator it = _capKind.begin();
	    it != _capKind.end(); ++it )
	{
	  str << "    " << it->first << '\t' << it->second << endl;
	}
      str << "  Capability refers:" << endl;
      for ( std::map<Resolvable::Kind,Counter>::const_iterator it = _capRefers.begin();
	    it != _capRefers.end(); ++it )
	{
	  str << "    " << it->first << '\t' << it->second << endl;
	}
      return str;
    }
  };

  unsigned USetStatsCollect::_cache_hits = 0;
  unsigned USetStatsCollect::_cache_misses = 0;
  
  struct Dummy
  {
    ~Dummy()
    {
      MIL << "[DUMMY] cacheh: " << USetStatsCollect::_cache_hits << " cachem: " << USetStatsCollect::_cache_misses << endl;
    }
  };
  
  static Dummy dummy;
  /////////////////////////////////////////////////////////////////
} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapFactoryImpl
  //
  /** CapFactory implementation.
   *
   * Provides various functions doing checks and log and \c throw.
   * CapFactory::parse usually combines them, and if nothing fails,
   * finaly builds the Capability.
   *
   * \attention Each CapabilityImpl created by CapFactory, \b must
   * be inserted into ::_uset, by calling ::usetInsert, \b before
   * the Capability is created.
   *
   * \li \c file:     /absolute/path
   * \li \c split:    name:/absolute/path
   * \li \c name:     name
   * \li \c vers:     name op edition
   * \li \c hal:      hal(string)
   * \li \c modalias: modalias(string)
  */
  struct CapFactory::Impl
  {
    /** Assert a valid Resolvable::Kind. */
    static void assertResKind( const Resolvable::Kind & refers_r )
    {
      if ( refers_r == Resolvable::Kind() )
	ZYPP_THROW( Exception("Missing or empty  Resolvable::Kind in Capability") );
    }

    /** Check whether \a op_r and \a edition_r make a valid edition spec.
     *
     * Rel::NONE is not usefull thus forbidden. Rel::ANY can be ignored,
     * so no VersionedCap is needed for this. Everything else requires
     * a VersionedCap.
     *
     * \return Whether to build a VersionedCap (i.e. \a op_r
     * is not Rel::ANY.
    */
    static bool isEditionSpec( Rel op_r, const Edition & edition_r )
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
      ZYPP_THROW( Exception("Unknow Operator NONE is not allowed in Capability") );
      return false; // not reached
    }

    /** Test for a FileCap. \a name_r starts with \c "/". */
    static bool isFileSpec( const std::string & name_r )
    {
      return *name_r.c_str() == '/';
    }

    /** Test for a SplitCap. \a name_r constains \c ":/". */
    static bool isSplitSpec( const std::string & name_r )
    {
      return name_r.find( ":/" ) != std::string::npos;
    }

    /** Test for a HalCap. \a name_r starts with  "hal(". */
    static bool isHalSpec( const std::string & name_r )
    {
      return name_r.substr(0,4) == "hal(";
    }

    /** Test for a ModaliasCap. \a name_r starts with  "modalias(". */
    static bool isModaliasSpec( const std::string & name_r )
    {
      return name_r.substr(0,9) == "modalias(";
    }

    static CapabilityImpl::Ptr buildFile( const Resolvable::Kind & refers_r,
                                          const std::string & name_r )
    {
      // NullCap check first:
      if ( name_r.empty() )
	{
	  // Singleton, so no need to put it into _uset !?
	  return capability::NullCap::instance();
	}

      assertResKind( refers_r );

      return usetInsert
      ( new capability::FileCap( refers_r, name_r ) );
    }

    /** Try to build a non versioned cap from \a name_r .
     *
     * The CapabilityImpl is built here and inserted into _uset.
     * The final Capability must be created by CapFactory, as it
     * is a friend of Capability. Here we can't access the ctor.
    */
    static CapabilityImpl::Ptr buildNamed( const Resolvable::Kind & refers_r,
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
	return usetInsert
	( new capability::FileCap( refers_r, name_r ) );

      //split:   name:/absolute/path
      static const str::regex  rx( "([^/]*):(/.*)" );
      str::smatch what;
      if( str::regex_match( name_r.begin(), name_r.end(), what, rx ) )
	{
	  return usetInsert
	  ( new capability::SplitCap( refers_r, what[1].str(), what[2].str() ) );
	}

      //name:    name
      return usetInsert
      ( new capability::NamedCap( refers_r, name_r ) );
    }

    /** Try to build a versioned cap from \a name_r .
     *
     * The CapabilityImpl is built here and inserted into _uset.
     * The final Capability must be created by CapFactory, as it
     * is a friend of Capability. Here we can't access the ctor.
     *
     * \todo Quick check for name not being filename or split.
    */
    static CapabilityImpl::Ptr buildVersioned( const Resolvable::Kind & refers_r,
				               const std::string & name_r,
				               Rel op_r,
				               const Edition & edition_r )
    {
      if ( Impl::isEditionSpec( op_r, edition_r ) )
	{
	  assertResKind( refers_r );

	  // build a VersionedCap
	  return usetInsert
	  ( new capability::VersionedCap( refers_r, name_r, op_r, edition_r ) );
	}
      //else
      // build a NamedCap

      return buildNamed( refers_r, name_r );
    }

    /** Try to build a hal cap from \a name_r .
     *
     * The CapabilityImpl is built here and inserted into _uset.
     * The final Capability must be created by CapFactory, as it
     * is a friend of Capability. Here we can't access the ctor.
     *
     * \todo Fix incaccuracy.
    */
    static CapabilityImpl::Ptr buildHal( const Resolvable::Kind & refers_r,
                                         const std::string & name_r,
                                         Rel op_r = Rel::ANY,
                                         const std::string & value_r = std::string() )
    {
      if ( op_r != Rel::ANY )
	{
	  ZYPP_THROW( Exception("Unsupported kind of Hal Capability '" + op_r.asString() + "'") );
	}

      //split:   hal(name) [op string]
      static const str::regex  rx( "hal\\(([^)]*)\\)" );
      str::smatch what;
      if( str::regex_match( name_r.begin(), name_r.end(), what, rx ) )
	{
	  // Hal always refers to 'System' kind of Resolvable.
	  return usetInsert
	  ( new capability::HalCap( ResTraits<SystemResObject>::kind,
				    what[1].str() ) );
	}
      // otherwise
      ZYPP_THROW( Exception("Unsupported kind of Hal Capability '" + name_r + "'") );
      return NULL; // make gcc happy
    }


    /** Try to build a modalias cap from \a name_r .
     *
     * The CapabilityImpl is built here and inserted into _uset.
     * The final Capability must be created by CapFactory, as it
     * is a friend of Capability. Here we can't access the ctor.
     *
     * \todo Fix incaccuracy.
    */
    static CapabilityImpl::Ptr buildModalias( const Resolvable::Kind & refers_r,
                                              const std::string & name_r,
                                              Rel op_r = Rel::ANY,
                                              const std::string & value_r = std::string() )
    {
      if ( op_r != Rel::ANY )
	{
	  ZYPP_THROW( Exception("Unsupported kind of Modalias Capability  '" + op_r.asString() + "'") );
	}

      //split:   modalias(name) [op string]
      static const str::regex  rx( "modalias\\(([^)]*)\\)" );
      str::smatch what;
      if( str::regex_match( name_r.begin(), name_r.end(), what, rx ) )
	{
	  // Modalias always refers to 'System' kind of Resolvable.
	  return usetInsert
	  ( new capability::ModaliasCap( ResTraits<SystemResObject>::kind,
                                         what[1].str() ) );
	}
      // otherwise
      ZYPP_THROW( Exception("Unsupported kind of Modalias Capability'" + name_r + "'") );
      return NULL; // make gcc happy
    }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapFactory
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::CapFactory
  //	METHOD TYPE : Ctor
  //
  CapFactory::CapFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::~CapFactory
  //	METHOD TYPE : Dtor
  //
  CapFactory::~CapFactory()
  {
  }

  static CapabilityImpl::Ptr cacheLookUp( const Resolvable::Kind &kind_r, const std::string &strval_r )
  {
    CapabilityImpl::Ptr ptr = _cap_cache[kind_r][strval_r.c_str()];
    if (ptr)
      ++USetStatsCollect::_cache_hits;
    else
      ++USetStatsCollect::_cache_misses;
    
    return ptr;
  }
  
  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
      const std::string & strval_r ) const

  try
    {
      CapabilityImpl::Ptr cap_ptr = cacheLookUp( refers_r, strval_r );
      if ( cap_ptr )
        return Capability(cap_ptr);
      
      _current_key = strval_r;
      
      if ( Impl::isHalSpec( strval_r ) )
      {
        return Capability( Impl::buildHal( refers_r, strval_r ) );
      }
      if ( Impl::isModaliasSpec( strval_r ) )
        {
          return Capability( Impl::buildModalias( refers_r, strval_r ) );
        }
      if ( Impl::isFileSpec( strval_r ) )
        {
          return Capability( Impl::buildFile( refers_r, strval_r ) );
        }

      // strval_r has at least two words which could make 'op edition'?
      // improve regex!
      static const str::regex  rx( "(.*[^ \t])([ \t]+)([^ \t]+)([ \t]+)([^ \t]+)" );
      str::smatch what;
      if( str::regex_match( strval_r.begin(), strval_r.end(),what, rx ) )
        {
          Rel op;
          Edition edition;
          try
            {
              op = Rel(what[3].str());
              edition = Edition(what[5].str());
            }
          catch ( Exception & excpt )
            {
              // So they don't make valid 'op edition'
              ZYPP_CAUGHT( excpt );
              DBG << "Trying named cap for: " << strval_r << endl;
              // See whether it makes a named cap.
              return Capability( Impl::buildNamed( refers_r, strval_r ) );
            }

          // Valid 'op edition'
          return Capability ( Impl::buildVersioned( refers_r,
                                                    what[1].str(), op, edition ) );
        }
      //else
      // not a VersionedCap
      return Capability( Impl::buildNamed( refers_r, strval_r ) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
    }


  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
				const std::string & name_r,
				const std::string & op_r,
				const std::string & edition_r ) const
  try
    {
      _current_key = name_r + " " + op_r + " " + edition_r;
      CapabilityImpl::Ptr cap_ptr = cacheLookUp( refers_r, _current_key );
      if ( cap_ptr )
        return Capability(cap_ptr);
      
      if ( Impl::isHalSpec( name_r ) )
	{
	  return Capability( Impl::buildHal( refers_r, name_r, Rel(op_r), edition_r ) );
	}
      if ( Impl::isModaliasSpec( name_r ) )
	{
	  return Capability( Impl::buildModalias( refers_r, name_r, Rel(op_r), edition_r ) );
	}
      // Try creating Rel and Edition, then parse
      return parse( refers_r, name_r, Rel(op_r), Edition(edition_r) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
    }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
				const std::string & name_r,
				Rel op_r,
				const Edition & edition_r ) const
  try
    {
      _current_key = name_r + " " + op_r.asString() + " " + edition_r.asString();
      CapabilityImpl::Ptr cap_ptr = cacheLookUp( refers_r, _current_key );
      if ( cap_ptr )
        return Capability(cap_ptr);
      
      if ( Impl::isHalSpec( name_r ) )
	{
	  return Capability( Impl::buildHal( refers_r, name_r, op_r, edition_r.asString() ) );
	}
      if ( Impl::isModaliasSpec( name_r ) )
	{
	  return Capability( Impl::buildModalias( refers_r, name_r, op_r, edition_r.asString() ) );
	}
      return Capability( Impl::buildVersioned( refers_r, name_r, op_r, edition_r ) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
    }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::halEvalCap
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::halEvalCap() const
  try
    {
      return Capability( Impl::buildHal( Resolvable::Kind(), "hal()" ) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
    }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::modaliasEvalCap
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::modaliasEvalCap() const
  try
    {
      return Capability( Impl::buildModalias( Resolvable::Kind(), "modalias()" ) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
    }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::encode
  //	METHOD TYPE : std::string
  //
  std::string CapFactory::encode( const Capability & cap_r ) const
  {
    return cap_r._pimpl->encode();
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CapFactory & obj )
  {
    str << "CapFactory stats:" << endl;
    return for_each( _uset.begin(), _uset.end(), USetStatsCollect() ).dumpOn( str );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
