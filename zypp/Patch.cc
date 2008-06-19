/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Patch.cc
 *
*/

#include "zypp/base/Logger.h"
#include "zypp/Patch.h"
#include "zypp/Message.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE( Patch );

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::Patch
  //	METHOD TYPE : Ctor
  //
  Patch::Patch( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::~Patch
  //	METHOD TYPE : Dtor
  //
  Patch::~Patch()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Patch interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  Patch::Category Patch::categoryEnum() const
  {
    static const IdString cat_yast( "yast" );
    static const IdString cat_security( "security" );
    static const IdString cat_recommended( "recommended" );
    static const IdString cat_optional( "optional" );
    static const IdString cat_document( "document" );

    IdString cat( sat::LookupAttr( sat::SolvAttr::patchcategory ).begin().idStr() );

    if ( cat == cat_yast )
      return CAT_YAST;
    if ( cat == cat_security )
      return CAT_SECURITY;
    if ( cat == cat_recommended )
      return CAT_RECOMMENDED;
    if ( cat == cat_optional )
      return CAT_OPTIONAL;
    if ( cat == cat_document )
      return CAT_DOCUMENT;

    return CAT_OTHER;
  }

  std::string Patch::message( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::message, lang_r ); }

  std::string Patch::category() const
  { return lookupStrAttribute( sat::SolvAttr::patchcategory ); }

  bool Patch::rebootSuggested() const
  { return lookupBoolAttribute( sat::SolvAttr::rebootSuggested ); }

  bool Patch::restartSuggested() const
  { return lookupBoolAttribute( sat::SolvAttr::restartSuggested ); }

  bool Patch::interactive() const
  {
    if ( rebootSuggested()
         || ! licenseToConfirm().empty() )
    {
      return true;
    }

    Patch::Contents c( contents() );
    for_( it, c.begin(), c.end() )
    {
      if ( it->isKind( ResKind::message )
           || ! licenseToConfirm().empty() )
      {
        return true;
      }
    }

    return false;
  }


  Patch::Contents Patch::contents() const
  {
    Contents result;
    sat::LookupAttr col_name( sat::SolvAttr::updateCollectionName, *this );
    sat::LookupAttr col_evr( sat::SolvAttr::updateCollectionEvr, *this );
    sat::LookupAttr col_arch( sat::SolvAttr::updateCollectionArch, *this );

    sat::LookupAttr::iterator col_name_it(col_name.begin());
    sat::LookupAttr::iterator col_evr_it(col_evr.begin());
    sat::LookupAttr::iterator col_arch_it(col_arch.begin());

    for (;col_name_it != col_name.end(); ++col_name_it, ++col_evr_it, ++col_arch_it)
    {
      /* safety checks, shouldn't happen (tm) */
      if (col_evr_it == col_evr.end()
          || col_arch_it == col_arch.end())
      {
          /* FIXME: Raise exception ?! */
          ERR << *this << " : The thing that should not happen, happened." << endl;
          break;
      }

      IdString nameid( col_name_it.asString() ); /* IdString for fast compare */
      Arch arch( col_arch_it.asString() );

      /* search providers of name */
      sat::WhatProvides providers( Capability( col_name_it.asString() ) );
      MIL << *this << " providers: " << endl;
      MIL << providers << endl;

      if (providers.empty())
      {
          WAR << *this << " misses provider for '" << col_name_it.asString() << "'" << endl;
          continue;
      }

      bool is_relevant = false;
      for_( it, providers.begin(), providers.end() )
      {
          if (it->ident() != nameid) /* package _name_ must match */
              continue;

          if (it->isSystem()  /* only look at installed providers with same arch */
              && it->arch() == arch)
          {
              is_relevant = true;
          }
      }
      if (!is_relevant)
      {
          MIL << *this << " is not relevant to the system" << endl;

          continue;        /* skip if name.arch is not installed */
      }


      /* find exact providers first (this matches the _real_ 'collection content' of the patch */
      sat::WhatProvides exact_providers( Capability( col_name_it.asString(), Rel::EQ, col_evr_it.asString(), ResKind::package ) );
      if (exact_providers.empty())
      {
          /* no exact providers: find 'best' providers: those with a larger evr */
          sat::WhatProvides best_providers( Capability( col_name_it.asString(), Rel::GT, col_evr_it.asString(), ResKind::package ) );
          if (best_providers.empty())
          {
              // Hmm, this patch is not installable, noone is providing the package in the collection
              // FIXME: raise execption ? fake a solvable ?
          }
          else
          {
              // FIXME ?! loop over providers and try to find installed ones ?
              result.get().insert( *(best_providers.begin()) );
          }
      }
      else
      {
          // FIXME ?! loop over providers and try to find installed ones ?
          result.get().insert( *(exact_providers.begin()) );
      }
    } /* while (attribute array) */

    return result;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Patch::ReferenceIterator
  //
  ///////////////////////////////////////////////////////////////////

  Patch::ReferenceIterator::ReferenceIterator( const sat::Solvable & val_r )
  {
    base_reference() = sat::LookupAttr( sat::SolvAttr::updateReferenceId,
                   val_r ).begin();
    _hrefit = sat::LookupAttr( sat::SolvAttr::updateReferenceHref,
                               val_r ).begin();
    _titleit = sat::LookupAttr( sat::SolvAttr::updateReferenceTitle,
                                val_r ).begin();
    _typeit = sat::LookupAttr( sat::SolvAttr::updateReferenceType,
                               val_r ).begin();
  }


  std::string Patch::ReferenceIterator::id() const
  { return base_reference().asString(); }
  std::string Patch::ReferenceIterator::href() const
  { return _hrefit.asString(); }
  std::string Patch::ReferenceIterator::title() const
  { return _titleit.asString(); }
  std::string Patch::ReferenceIterator::type() const
  { return _typeit.asString(); }


  void  Patch::ReferenceIterator::increment()
  {
    ++base_reference();
    ++_hrefit;
    ++_titleit;
    ++_typeit;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
