/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Pattern.cc
 *
*/
#include <iostream>

#include "zypp/Pattern.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Pattern);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Pattern::Pattern
  //	METHOD TYPE : Ctor
  //
  Pattern::Pattern( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Pattern::~Pattern
  //	METHOD TYPE : Dtor
  //
  Pattern::~Pattern()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Pattern interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////
  /** */
  bool Pattern::isDefault() const
  { return false; }
  /** */
  bool Pattern::userVisible() const
  { return true; }
  /** */
  std::string Pattern::category() const
  { return std::string(); }
  /** */
  Pathname Pattern::icon() const
  { return Pathname(); }
  /** */
  Pathname Pattern::script() const
  { return Pathname(); }

  Label Pattern::order() const
  { return Label(); }

  std::set<std::string> Pattern::install_packages( const Locale & lang ) const
  {
#warning implement PATTERN::INSTALL_PACKAGES
#if 0
-    static void copycaps( std::set<std::string> & out, const CapSet & in)
-    {
-	for (CapSet::const_iterator it = in.begin(); it != in.end(); ++it) {
-	    if (isKind<capability::NamedCap>( *it )
-		&& it->refers() == ResTraits<zypp::Package>::kind )
-	    {
-		out.insert( it->index() );
-	    }
-	}
-    }
-
-    std::set<std::string> PatternImplIf::install_packages( const Locale & lang) const
-    {
-	std::set<std::string> result;
-
-	copycaps( result, self()->dep( Dep::REQUIRES ) );
-	copycaps( result, self()->dep( Dep::RECOMMENDS) );
-	copycaps( result, self()->dep( Dep::SUGGESTS) );
-
-	return result;
-    }
-
-
#endif
  return std::set<std::string>();
  }

  const Capabilities & Pattern::includes() const
  { return Capabilities(); }

  const Capabilities & Pattern::extends() const
  { return Capabilities(); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
