/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixSelectionImpl.cc
 *
*/

#include "HelixSelectionImpl.h"
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
//        CLASS NAME : HelixSelectionImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixSelectionImpl::HelixSelectionImpl (Source_Ref source_r, const zypp::HelixParser & parsed)
    : _source (source_r)
    , _summary(parsed.summary)
    , _description(parsed.description)
{
}

Source_Ref
HelixSelectionImpl::source() const
{ return _source; }

/** Selection summary */
TranslatedText HelixSelectionImpl::summary() const
{ return _summary; }

/** Selection description */
TranslatedText HelixSelectionImpl::description() const
{ return _description; }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
