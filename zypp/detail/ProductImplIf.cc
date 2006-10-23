/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ProductImplIf.cc
 *
*/

#include "zypp/detail/ProductImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

      /** Get the category of the product - addon or base*/
      std::string ProductImplIf::category() const
      { return std::string(); }

      /** Get the vendor of the product */
      Label ProductImplIf::vendor() const
      { return Label(); }

      Url ProductImplIf::releaseNotesUrl() const
      { return Url(); }

      std::list<Url> ProductImplIf::updateUrls() const
      { return std::list<Url>(); }
      
      std::list<Url>  ProductImplIf::extraUrls() const
      { return std::list<Url>(); }
      
      std::list<Url>  ProductImplIf::optionalUrls() const
      { return std::list<Url>(); }

      std::list<std::string> ProductImplIf::flags() const
      { return std::list<std::string>(); }

      TranslatedText ProductImplIf::shortName() const
      { return TranslatedText(); }

      std::string ProductImplIf::distributionName() const
      { return std::string(); }

      Edition ProductImplIf::distributionEdition() const
      { return Edition(); }

    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

