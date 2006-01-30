/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImpl.cc
 *
*/
#include "zypp/source/susetags/SuseTagsSelectionImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
  namespace susetags
  {
    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SelectionImpl::SelectionImpl
    //	METHOD TYPE : Ctor
    //
    SuseTagsSelectionImpl::SuseTagsSelectionImpl()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SelectionImpl::~SelectionImpl
    //	METHOD TYPE : Dtor
    //
    SuseTagsSelectionImpl::~SuseTagsSelectionImpl()
    {}


    Label SuseTagsSelectionImpl::summary() const
    { return Label(); }

    Text SuseTagsSelectionImpl::description() const
    { return Text(); }

    Label SuseTagsSelectionImpl::category() const
    { return Label(); }

    bool SuseTagsSelectionImpl::visible() const
    { return false; }

    Label SuseTagsSelectionImpl::order() const
    { return Label(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
