/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Selection.h
 *
*/
#ifndef ZYPP_SELECTION_H
#define ZYPP_SELECTION_H

#include <list>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/Resolvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(SelectionImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  DEFINE_PTR_TYPE(Selection)

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Selection
  //
  /** */
  class Selection : public Resolvable
  {
  public:
    /** Default ctor */
    Selection( detail::SelectionImplPtr impl_r );
    /** Dtor */
    virtual ~Selection();

  public:

    /** */
    std::string summary() const;
    /** */
    std::list<std::string> description() const;

  private:
    /** Pointer to implementation */
    detail::SelectionImplPtr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SELECTION_H
