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

      TranslatedText ResObjectImplIf::insnotify() const
      { return TranslatedText::notext; }

      TranslatedText ResObjectImplIf::delnotify() const
      { return TranslatedText::notext; }

      TranslatedText ResObjectImplIf::licenseToConfirm() const
      { return TranslatedText::notext; }

      unsigned ResObjectImplIf::mediaId() const
      { return 1; }

      Vendor ResObjectImplIf::vendor() const
      { return Vendor(); }

      ByteCount ResObjectImplIf::size() const
      { return ByteCount(); }

      ByteCount ResObjectImplIf::archivesize() const
      { return ByteCount(); }

      Source_Ref ResObjectImplIf::source() const
      { return Source_Ref::noSource; }

      unsigned ResObjectImplIf::sourceMediaNr() const
      { return 0; }

      bool ResObjectImplIf::installOnly() const
      { return false; }

      Date ResObjectImplIf::buildtime() const
      { return Date(); }

      Date ResObjectImplIf::installtime() const
      { return Date(); }

      ZmdId ResObjectImplIf::zmdid() const
      { return 0; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
