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
extern "C"
{
#include <satsolver/repo.h>
}

#include "zypp/base/Logger.h"
#include "zypp/Patch.h"
#include "zypp/sat/WhatProvides.h"

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

  bool Patch::reloginSuggested() const
  { return lookupBoolAttribute( sat::SolvAttr::reloginSuggested ); }


  bool Patch::interactive() const
  {
    if ( rebootSuggested()
         || ! message().empty()
         || ! licenseToConfirm().empty() )
    {
      return true;
    }

    Patch::Contents c( contents() );
    for_( it, c.begin(), c.end() )
    {
      if ( ! licenseToConfirm().empty() )
      {
        return true;
      }
    }

    return false;
  }


  Patch::Contents Patch::contents() const
  {
      Contents result;

      ::Dataiterator di;
      ::dataiterator_init(&di
          , sat::Pool::instance().get()
          , repository().get()                           // in this repo
          , sat::Solvable::id()                          // in metadata
              , UPDATE_COLLECTION, 0, 0 );

      while (::dataiterator_step(&di))
      {
          ::dataiterator_setpos( &di );
          ::Dataiterator di2;
          ::dataiterator_init(&di2
              , sat::Pool::instance().get()
              , repository().get()                           // in this repo
              , SOLVID_POS                                   // in metadata
                  ,0,0,0 );

          IdString nameid;
          Edition  evr;
          Arch     arch;

          while (::dataiterator_step(&di2))
          {
            switch ( di2.key->name )
            {
              case UPDATE_COLLECTION_NAME:
                nameid = IdString(di2.kv.id);
                break;
              case UPDATE_COLLECTION_EVR:
                evr = Edition(di2.kv.id);
                break;
              case UPDATE_COLLECTION_ARCH:
                arch = Arch(di2.kv.id);
                break;
            }
          }

          if ( nameid.empty() )
            continue;

          /* search providers of name */
          sat::WhatProvides providers( Capability( nameid.c_str() ) );
          MIL << *this << " providers: " << endl;
          MIL << providers << endl;

          if (providers.empty())
          {
            WAR << *this << " misses provider for '" << nameid << "'" << endl;
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
          sat::WhatProvides exact_providers( Capability( nameid.c_str(), Rel::EQ, evr, ResKind::package ) );
          if (exact_providers.empty())
          {
            /* no exact providers: find 'best' providers: those with a larger evr */
            sat::WhatProvides best_providers( Capability( nameid.c_str(), Rel::GT, evr, ResKind::package ) );
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
