/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/XMLSelectionImpl.cc
 *
*/
#include "zypp/target/store/xml/XMLSelectionImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : XMLSelectionImpl::XMLSelectionImpl
    //	METHOD TYPE : Ctor
    //
    XMLSelectionImpl::XMLSelectionImpl()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : XMLSelectionImpl::~XMLSelectionImpl
    //	METHOD TYPE : Dtor
    //
    XMLSelectionImpl::~XMLSelectionImpl()
    {}

    TranslatedText XMLSelectionImpl::summary() const
    { return _summary; }

    TranslatedText XMLSelectionImpl::description() const
    { return _summary; }

    Label XMLSelectionImpl::category() const
    { return _category; }

    bool XMLSelectionImpl::visible() const
    { return _visible; }

    Label XMLSelectionImpl::order() const
    { return _order; }
    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
