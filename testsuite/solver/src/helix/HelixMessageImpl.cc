/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixMessageImpl.cc
 *
*/

#include "HelixMessageImpl.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixMessageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixMessageImpl::HelixMessageImpl (Source_Ref source_r, const zypp::HelixParser & parsed)
    : _source (source_r)
{
}

Source_Ref
HelixMessageImpl::source() const
{ return _source; }

std::string
HelixMessageImpl::text () const
{ return _text; }

std::string
HelixMessageImpl::type () const
{ return _type; }

ByteCount
HelixMessageImpl::size() const
{ return _size_installed; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
