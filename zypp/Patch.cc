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
#include "zypp/Patch.h"
#include "zypp/Message.h"

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

  std::string Patch::category() const
  { return lookupStrAttribute( sat::SolvAttr::patchcategory ); }

  bool Patch::reboot_needed() const
  { return lookupBoolAttribute( sat::SolvAttr::needReboot ); }

  bool Patch::affects_pkg_manager() const
  { return lookupBoolAttribute( sat::SolvAttr::needRestart ); }

  bool Patch::interactive() const
  {
    if ( reboot_needed()
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
    sat::LookupAttr::iterator _col_name_it(sat::LookupAttr( sat::SolvAttr::updateCollectionName, *this ).begin());
    sat::LookupAttr::iterator _col_evr_it(sat::LookupAttr( sat::SolvAttr::updateCollectionEvr, *this ).begin());
    sat::LookupAttr::iterator _col_arch_it(sat::LookupAttr( sat::SolvAttr::updateCollectionArch, *this ).begin());

    for (;_col_name_it != sat::LookupAttr( sat::SolvAttr::updateCollectionName, *this ).end(); ++_col_name_it, ++_col_evr_it, ++_col_arch_it)
    {
      /* safety checks, shouldn't happen (tm) */
      if (_col_evr_it == sat::LookupAttr( sat::SolvAttr::updateCollectionEvr, *this ).end()
	  || _col_arch_it == sat::LookupAttr( sat::SolvAttr::updateCollectionArch, *this ).end())
      {
	/* FIXME: Raise exception ?! */
	break;
      }
      IdString nameid( _col_name_it.asString() ); /* IdString for fast compare */
      Arch arch( _col_arch_it.asString() );
      
      /* search providers of name */
      sat::WhatProvides providers( Capability( _col_name_it.asString() ) );
      if (providers.empty())
	continue;
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
	continue;        /* skip if name.arch is not installed */
      
      /* find exact providers first (this matches the _real_ 'collection content' of the patch */
      sat::WhatProvides exact_providers( Capability( _col_name_it.asString(), Rel::EQ, _col_evr_it.asString(), ResKind::package ) );
      if (exact_providers.empty())
      {
	/* no exact providers: find 'best' providers */
	sat::WhatProvides best_providers( Capability( _col_name_it.asString(), Rel::GT, _col_evr_it.asString(), ResKind::package ) );
        if (best_providers.empty())
	{
	  // Hmm, this patch is not installable, noone is providing the package in the collection
	  // raise execption ? fake a solvable ?
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
