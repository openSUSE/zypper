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
#include "zypp/Repository.h"

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

      Vendor ResObjectImplIf::vendor() const
      { return Vendor(); }

      Repository ResObjectImplIf::repository() const
      { return Repository::noRepository; }

      bool ResObjectImplIf::installOnly() const
      { return false; }

      Date ResObjectImplIf::buildtime() const
      { return Date(); }

      Date ResObjectImplIf::installtime() const
      { return Date(); }
      
      unsigned ResObjectImplIf::mediaNr() const
      { return 0; }
      
      ByteCount ResObjectImplIf::size() const
      { return ByteCount(); }

      ByteCount ResObjectImplIf::downloadSize() const
      { return ByteCount(); }
      
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
