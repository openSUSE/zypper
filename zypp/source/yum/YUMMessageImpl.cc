/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMMessageImpl.cc
 *
*/

#include "zypp/source/yum/YUMMessageImpl.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
namespace yum
{
///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : YUMMessageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
YUMMessageImpl::YUMMessageImpl(
  Source_Ref source_r,
  const zypp::parser::yum::YUMPatchMessage & parsed,
  Patch::constPtr patch
)
    : _source(source_r)
    , _patch(patch)
{
  _text = parsed.text;
}
/** Get the text of the message */
TranslatedText YUMMessageImpl::text() const
{
  return _text;
}

Source_Ref YUMMessageImpl::source() const
{
  return _source;
}

/** Patch the message belongs to - if any */
Patch::constPtr YUMMessageImpl::patch() const
{
  return _patch;
}

} // namespace yum
/////////////////////////////////////////////////////////////////
} // namespace source
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
