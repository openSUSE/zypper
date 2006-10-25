/*---------------------------------------------------------------------\
 |                          ____ _   __ __ ___                          |
 |                         |__  / \ / / . \ . \                         |
 |                           / / \ V /|  _/  _/                         |
 |                          / /__ | | | | | |                           |
 |                         /_____||_| |_| |_|                           |
 |                                                                      |
 \---------------------------------------------------------------------*/
/** \file	zypp/ui/PatternContentsImpl.cc
 *
*/

#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/ui/PatternContentsImpl.h"
#include "zypp/ui/PatternExpander.h"

#include "zypp/ZYppFactory.h"
#include "zypp/ResPool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      struct CollectInstallPackages
      {
        CollectInstallPackages( std::set<std::string> & result )
        : _result( &result )
        {}

        void operator()( const Pattern::constPtr & pattern )
        {
          std::set<std::string> s( pattern->install_packages() );
          _result->insert( s.begin(), s.end() );
        }

        std::set<std::string> * _result;
      };

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    PatternContents::Impl::Impl( const Pattern::constPtr & pattern )
    : _pattern( pattern )
    {}

    std::set<std::string> PatternContents::Impl::install_packages() const
    {
      PatternExpander expander( getZYpp()->pool() );

      if ( ! expander.expand( _pattern ) )
        return std::set<std::string>(); // empty pattern set

      std::set<std::string> result;
      for_each( expander.begin(),
                expander.end(),
                CollectInstallPackages( result ) );
      return result;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
