/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/ResObjectImplIf.cc
 *
*/
#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/SourceFactory.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // Default implementation of ResObjectImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

      TranslatedText ResObjectImplIf::summary() const
      { return TranslatedText::notext; }

      TranslatedText ResObjectImplIf::description() const
      { return TranslatedText::notext; }

      Text ResObjectImplIf::insnotify() const
      { return Text(); }

      Text ResObjectImplIf::delnotify() const
      { return Text(); }

      ByteCount ResObjectImplIf::size() const
      { return 0; }

      bool ResObjectImplIf::providesSources() const
      { return false; }

      Source_Ref ResObjectImplIf::source() const
      { return Source_Ref::noSource; }

      ZmdId ResObjectImplIf::zmdid() const
      { return 0; }

      Label ResObjectImplIf::instSrcLabel() const
      { return Label(); }

      Vendor ResObjectImplIf::instSrcVendor() const
      { return Vendor(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
