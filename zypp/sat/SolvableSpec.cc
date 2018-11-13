/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/SolvableSpec.cc
 */
#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/base/IOStream.h"

#include "zypp/sat/SolvableSpec.h"
#include "zypp/sat/WhatProvides.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    ///////////////////////////////////////////////////////////////////
    /// \class SolvableSpec::Impl
    /// \brief SolvableSpec implementation.
    ///////////////////////////////////////////////////////////////////
    class SolvableSpec::Impl
    {
    public:
      void addIdent( IdString ident_r )
      {
	if ( ! ident_r.empty() )
	  _idents.insert( ident_r );
      }

      void addProvides( Capability provides_r )
      {
	if ( ! provides_r.empty() && _provides.insert( provides_r ).second )
	  setDirty();
      }


      void parse( const C_Str & spec_r )
      {
	if ( str::hasPrefix( spec_r, "provides:" ) )
	  addProvides( Capability(spec_r.c_str()+9) );
	else
	  addIdent( IdString(spec_r) );
      }


      bool needed() const
      { return !_provides.empty(); }

      bool dirty() const
      { return needed() && !_cache; }

      void setDirty() const
      { _cache.reset(); }

      const WhatProvides & cache() const
      {
	if ( !_cache )
	{
	  _cache.reset( new WhatProvides( _provides ) );
	}
	return *_cache;
      }

      bool contains( const sat::Solvable & solv_r ) const
      { return( _idents.count( solv_r.ident() ) || ( needed() && cache().contains( solv_r ) ) ); }


      const IdStringSet & idents() const
      { return _idents; }

      const CapabilitySet & provides() const
      { return _provides; }

    private:
      IdStringSet  _idents;
      CapabilitySet _provides;

      mutable shared_ptr<WhatProvides> _cache;

    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }
    };

    /** \relates SolvableSpec::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SolvableSpec::Impl & obj )
    {
      str << "SolvableSpec {" << endl
          << " Idents " << obj.idents() << endl
          << " Provides " << obj.provides() << endl
          << "}";
      return str;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SolvableSpec
    //
    ///////////////////////////////////////////////////////////////////

    SolvableSpec::SolvableSpec()
      : _pimpl( new Impl )
    {}

    SolvableSpec::~SolvableSpec()
    {}

    void SolvableSpec::addIdent( IdString ident_r )
    { _pimpl->addIdent( ident_r ); }

    void SolvableSpec::addProvides( Capability provides_r )
    { _pimpl->addProvides( provides_r ); }

    void SolvableSpec::parse( const C_Str & spec_r )
    { _pimpl->parse( spec_r ); }

    void SolvableSpec::parseFrom( const InputStream & istr_r )
    {
      iostr::simpleParseFile( istr_r,
			      [this]( int num_r, const std::string & line_r )->bool
			      {
				this->parse( line_r );
				return true;
			      });
    }

    void SolvableSpec::splitParseFrom( const C_Str & multispec_r )
    {
      std::vector<std::string> v;
      str::splitEscaped( multispec_r, std::back_inserter( v ), ", \t" );
      parseFrom( v.begin(), v.end() );
    }

    bool SolvableSpec::contains( const sat::Solvable & solv_r ) const
    { return _pimpl->contains( solv_r ) && !solv_r.isKind( ResKind::srcpackage ); }

    bool SolvableSpec::dirty() const
    { return _pimpl->dirty(); }

    void SolvableSpec::setDirty() const
    { _pimpl->setDirty(); }

    bool SolvableSpec::containsIdent( const IdString & ident_r ) const
    { return _pimpl->idents().count( ident_r ); }

    bool SolvableSpec::containsProvides( const Capability & provides_r ) const
    { return _pimpl->provides().count( provides_r ); }

    std::ostream & operator<<( std::ostream & str, const SolvableSpec & obj )
    { return str << *obj._pimpl; }

  } // namespace sat
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
