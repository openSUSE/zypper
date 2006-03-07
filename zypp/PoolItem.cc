/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PoolItem.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/PoolItem.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolItem_Ref::Impl
  //
  /** PoolItem_Ref implementation. */
  struct PoolItem_Ref::Impl
  {
    Impl()
    {}

    Impl( ResObject::constPtr res_r,
          const ResStatus & status_r = ResStatus() )
    : _status( status_r )
    , _resolvable( res_r )
    {}

    ResStatus & status() const
    { return _status; }

    ResObject::constPtr resolvable() const
    { return _resolvable; }

  private:
    mutable ResStatus   _status;
    ResObject::constPtr _resolvable;

    /** \name Poor man's save/restore state.
     * \todo There may be better save/restore state strategies.
    */
    //@{
  public:
    void saveState() const
    { _savedStatus = _status; }
    void restoreState() const
    { _status = _savedStatus; }
    bool sameState() const
    {
      if (    _status.getTransactValue() != _savedStatus.getTransactValue()
           && !_status.isBySolver() )
        return false;
      if ( _status.isLicenceConfirmed() != _savedStatus.isLicenceConfirmed() )
        return false;
      return true;
    }
  private:
    mutable ResStatus _savedStatus;
    //@}

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PoolItem_Ref::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PoolItem_Ref::Impl & obj )
  {
    str << obj.status();
    if (obj.resolvable())
	str << *obj.resolvable();
    else
	str << "(NULL)";
    return str;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolItem_Ref
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem_Ref::PoolItem_Ref
  //	METHOD TYPE : Ctor
  //
  PoolItem_Ref::PoolItem_Ref()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem_Ref::PoolItem_Ref
  //	METHOD TYPE : Ctor
  //
  PoolItem_Ref::PoolItem_Ref( ResObject::constPtr res_r )
  : _pimpl( new Impl( res_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem_Ref::PoolItem_Ref
  //	METHOD TYPE : Ctor
  //
  PoolItem_Ref::PoolItem_Ref( ResObject::constPtr res_r, const ResStatus & status_r )
  : _pimpl( new Impl( res_r, status_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem_Ref::~PoolItem_Ref
  //	METHOD TYPE : Dtor
  //
  PoolItem_Ref::~PoolItem_Ref()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Forward to Impl:
  //
  ///////////////////////////////////////////////////////////////////

  ResStatus & PoolItem_Ref::status() const
  { return _pimpl->status(); }

  ResObject::constPtr PoolItem_Ref::resolvable() const
  { return _pimpl->resolvable(); }

  void PoolItem_Ref::saveState() const
  { _pimpl->saveState(); }

  void PoolItem_Ref::restoreState() const
  { _pimpl->restoreState(); }

  bool PoolItem_Ref::sameState() const
  { return _pimpl->sameState(); }

  /******************************************************************
   **
   **	FUNCTION NAME : operator<<
   **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const PoolItem_Ref & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
