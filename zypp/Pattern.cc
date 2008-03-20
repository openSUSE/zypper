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
  { return lookupBoolAttribute( sat::SolvAttr::isdefault ); }
  /** */
  bool Pattern::userVisible() const
  { return lookupBoolAttribute( sat::SolvAttr::isvisible ); }
  /** */
  std::string Pattern::category( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::category, lang_r ); }
  /** */
  Pathname Pattern::icon() const
  { return lookupStrAttribute( sat::SolvAttr::icon ); }
  /** */
  Pathname Pattern::script() const
  { return lookupStrAttribute( sat::SolvAttr::script ); }

  std::string Pattern::order() const
  { return lookupStrAttribute( sat::SolvAttr::order ); }

#warning implement PATTERN::INSTALL_PACKAGES
#if 0
  std::set<std::string> Pattern::install_packages( const Locale & lang ) const
  {
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
  return std::set<std::string>();
  }
#endif

#warning implement PATTERN::INSTALL_PACKAGES
 const Capabilities & Pattern::includes() const
  {
    static Capabilities _val;
    return _val;
  }

  const Capabilities & Pattern::extends() const
  {
    static Capabilities _val;
    return _val;
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
