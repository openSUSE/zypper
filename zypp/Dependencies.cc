/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Dependencies.cc
 *
*/
#include <iostream>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/Dependencies.h"
#include "zypp/CapSet.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Dependencies::Impl
  //
  /** Dependencies implementation. */
  struct Dependencies::Impl
  {
    /**  */
    CapSet _provides;
    /**  */
    CapSet _prerequires;
    /**  */
    CapSet _requires;
    /**  */
    CapSet _conflicts;
    /**  */
    CapSet _obsoletes;
    /**  */
    CapSet _recommends;
    /**  */
    CapSet _suggests;
    /**  */
    CapSet _freshens;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates DependenciesImpl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Dependencies::Impl & obj )
  {
    str << "PROVIDES:"    << endl << obj._provides;
    str << "PREREQUIRES:" << endl << obj._prerequires;
    str << "REQUIRES:"    << endl << obj._requires;
    str << "CONFLICTS:"   << endl << obj._conflicts;
    str << "OBSOLETES:"   << endl << obj._obsoletes;
    str << "RECOMMENDS:"  << endl << obj._recommends;
    str << "SUGGESTS:"    << endl << obj._suggests;
    str << "FRESHENS:"    << endl << obj._freshens;
    return str;
  }
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Dependencies
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Dependencies::Dependencies
  //	METHOD TYPE : Ctor
  //
  Dependencies::Dependencies()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Dependencies::~Dependencies
  //	METHOD TYPE : Dtor
  //
  Dependencies::~Dependencies()
  {}

  const CapSet & Dependencies::provides() const
  { return _pimpl->_provides; }

  const CapSet & Dependencies::prerequires() const
  { return _pimpl->_prerequires; }

  const CapSet & Dependencies::requires() const
  { return _pimpl->_requires; }

  const CapSet & Dependencies::conflicts() const
  { return _pimpl->_conflicts; }

  const CapSet & Dependencies::obsoletes() const
  { return _pimpl->_obsoletes; }

  const CapSet & Dependencies::recommends() const
  { return _pimpl->_recommends; }

  const CapSet & Dependencies::suggests() const
  { return _pimpl->_suggests; }

  const CapSet & Dependencies::freshens() const
  { return _pimpl->_freshens; }

  void Dependencies::setProvides( const CapSet & val_r )
  { _pimpl->_provides = val_r; }

  void Dependencies::setPrerequires( const CapSet & val_r )
  { _pimpl->_prerequires = val_r; }

  void Dependencies::setRequires( const CapSet & val_r )
  { _pimpl->_requires = val_r; }

  void Dependencies::setConflicts( const CapSet & val_r )
  { _pimpl->_conflicts = val_r; }

  void Dependencies::setObsoletes( const CapSet & val_r )
  { _pimpl->_obsoletes = val_r; }

  void Dependencies::setRecommends( const CapSet & val_r )
  { _pimpl->_recommends = val_r; }

  void Dependencies::setSuggests( const CapSet & val_r )
  { _pimpl->_suggests = val_r; }

  void Dependencies::setFreshens( const CapSet & val_r )
  { _pimpl->_freshens = val_r; }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Dependencies & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
