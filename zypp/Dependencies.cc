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

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : DependenciesImpl
    //
    /** Dependencies implementation */
    struct DependenciesImpl : public base::ReferenceCounted, private base::NonCopyable
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

      static DependenciesImpl_Ptr nodeps()
      {
        if ( !_nodeps ) { _nodeps = new DependenciesImpl; _nodeps->ref(); }
        return _nodeps;
      }

    private:
      static DependenciesImpl_Ptr _nodeps;
    };
    ///////////////////////////////////////////////////////////////////
    IMPL_PTR_TYPE(DependenciesImpl)
    DependenciesImpl_Ptr DependenciesImpl::_nodeps;
    ///////////////////////////////////////////////////////////////////

    /** \relates DependenciesImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const DependenciesImpl & obj )
    {
      str << "PROVIDES:" << endl << obj._provides;
      str << "PREREQUIRES:" << endl << obj._prerequires;
      str << "REQUIRES:" << endl << obj._requires;
      str << "CONFLICTS:" << endl << obj._conflicts;
      str << "OBSOLETES:" << endl << obj._obsoletes;
      str << "RECOMMENDS:" << endl << obj._recommends;
      str << "SUGGESTS:" << endl << obj._suggests;
      str << "FRESHENS:" << endl << obj._freshens;
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
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
  : _pimpl( detail::DependenciesImpl::nodeps() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Dependencies::Dependencies
  //	METHOD TYPE : Ctor
  //
  Dependencies::Dependencies( detail::DependenciesImpl_Ptr impl_r )
  : _pimpl( impl_r ? impl_r : detail::DependenciesImpl::nodeps() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Dependencies::~Dependencies
  //	METHOD TYPE : Dtor
  //
  Dependencies::~Dependencies()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Dependencies::sayFriend
  //	METHOD TYPE : detail::DependenciesImplconstPtr
  //
  detail::DependenciesImpl_constPtr Dependencies::sayFriend() const
  { return _pimpl; }

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

#define ZYPP_DEPENDENCIES_COW \
if(_pimpl->refCount()>1) \
{ \
  detail::DependenciesImpl_Ptr _cow_tmp = new detail::DependenciesImpl; \
  _cow_tmp->_provides = _pimpl->_provides; \
  _cow_tmp->_prerequires = _pimpl->_prerequires; \
  _cow_tmp->_requires = _pimpl->_requires; \
  _cow_tmp->_conflicts = _pimpl->_conflicts; \
  _cow_tmp->_obsoletes = _pimpl->_obsoletes; \
  _cow_tmp->_recommends = _pimpl->_recommends; \
  _cow_tmp->_suggests = _pimpl->_suggests; \
  _cow_tmp->_freshens = _pimpl->_freshens; \
  _pimpl= _cow_tmp;\
}

  void Dependencies::setProvides( const CapSet & val_r )
  { ZYPP_DEPENDENCIES_COW; _pimpl->_provides = val_r; }

  void Dependencies::setPrerequires( const CapSet & val_r )
  { ZYPP_DEPENDENCIES_COW; _pimpl->_prerequires = val_r; }

  void Dependencies::setRequires( const CapSet & val_r )
  { ZYPP_DEPENDENCIES_COW; _pimpl->_requires = val_r; }

  void Dependencies::setConflicts( const CapSet & val_r )
  { ZYPP_DEPENDENCIES_COW; _pimpl->_conflicts = val_r; }

  void Dependencies::setObsoletes( const CapSet & val_r )
  { ZYPP_DEPENDENCIES_COW; _pimpl->_obsoletes = val_r; }

  void Dependencies::setRecommends( const CapSet & val_r )
  { ZYPP_DEPENDENCIES_COW; _pimpl->_recommends = val_r; }

  void Dependencies::setSuggests( const CapSet & val_r )
  { ZYPP_DEPENDENCIES_COW; _pimpl->_suggests = val_r; }

  void Dependencies::setFreshens( const CapSet & val_r )
  { ZYPP_DEPENDENCIES_COW; _pimpl->_freshens = val_r; }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Dependencies & obj )
  {
    return str << *obj.sayFriend();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
