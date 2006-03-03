/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/testsuite/solver/HelixLanguageImpl.cc
 *
*/

#include "HelixLanguageImpl.h"
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
//        CLASS NAME : HelixLanguageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixLanguageImpl::HelixLanguageImpl (Source_Ref source_r, const zypp::HelixParser & parsed)
  : LanguageImplIf( TranslatedText( "dummy" ) )
{
}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
