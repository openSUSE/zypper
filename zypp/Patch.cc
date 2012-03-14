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
#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
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
    static const IdString cat_yast		( "yast" );
    static const IdString cat_security		( "security" );
    static const IdString cat_recommended	( "recommended" );
    static const IdString cat_bugfix		( "bugfix" );		// rhn
    static const IdString cat_optional		( "optional" );
    static const IdString cat_feature		( "feature" );
    static const IdString cat_enhancement	( "enhancement" );	// rnh
    static const IdString cat_document		( "document" );

    // patch category is not poolized in the solv file (i.e. an IdString) ;(
    IdString cat( sat::LookupAttr( sat::SolvAttr::patchcategory, satSolvable() ).begin().c_str() );

    if ( cat == cat_yast )
      return CAT_YAST;
    if ( cat == cat_security )
      return CAT_SECURITY;
    if ( cat == cat_recommended || cat == cat_bugfix )
      return CAT_RECOMMENDED;
    if ( cat == cat_optional || cat == cat_enhancement || cat == cat_feature )
      return CAT_OPTIONAL;
    if ( cat == cat_document )
      return CAT_DOCUMENT;

    return CAT_OTHER;
  }

  std::string Patch::severity() const
  { return lookupStrAttribute( sat::SolvAttr::severity ); }

  Patch::SeverityFlag Patch::severityFlag() const
  {
    std::string sev( severity() );
    switch ( sev[0] )
    {
      case 'l':
      case 'L':
	if ( str::compareCI( sev, "low" ) == 0 )
	  return SEV_LOW;
	break;

      case 'm':
      case 'M':
	if ( str::compareCI( sev, "moderate" ) == 0 )
	  return SEV_MODERATE;
	break;

      case 'i':
      case 'I':
	if ( str::compareCI( sev, "important" ) == 0 )
	  return SEV_IMPORTANT;
	break;

      case 'c':
      case 'C':
	if ( str::compareCI( sev, "critical" ) == 0 )
	  return SEV_CRITICAL;
	break;

      case '\0':
	return SEV_NONE;
	break;
    }
    // default:
    return SEV_OTHER;
  }

  std::string asString( const Patch::SeverityFlag & obj )
  {
    switch ( obj )
    {
      case Patch::SEV_NONE:	return std::string( "unspecified" );	break;
      case Patch::SEV_OTHER:	return std::string( "unknown" );	break;
      case Patch::SEV_LOW:	return std::string( "low" );		break;
      case Patch::SEV_MODERATE:	return std::string( "moderate" );	break;
      case Patch::SEV_IMPORTANT:return std::string( "important" );	break;
      case Patch::SEV_CRITICAL:	return std::string( "critical" );	break;
    }
    // make gcc happy:
    return std::string( "unknown" );
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

  Patch::InteractiveFlags Patch::interactiveFlags() const
  {
    InteractiveFlags patchFlags (NoFlags);
    if ( rebootSuggested() )
      patchFlags |= Reboot;

    if ( ! message().empty() )
      patchFlags |= Message;

    if ( ! licenseToConfirm().empty() )
      patchFlags |= License;

    Patch::Contents c( contents() );
    for_( it, c.begin(), c.end() )
    {
      if ( ! makeResObject(*it)->licenseToConfirm().empty() )
      {
        patchFlags |= License;
        break;
      }
    }
    return patchFlags;
  }

  bool Patch::interactiveWhenIgnoring( InteractiveFlags flags_r ) const
  {
    if ( interactiveFlags() & ( ~flags_r ) )
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool Patch::interactive() const
  {
    return interactiveWhenIgnoring();
  }

  Patch::Contents Patch::contents() const
  {
    Contents result;
    // DBG << *this << endl;
    sat::LookupAttr updateCollection( sat::SolvAttr::updateCollection, satSolvable() );
    for_( entry, updateCollection.begin(), updateCollection.end() )
    {
      IdString name    ( entry.subFind( sat::SolvAttr::updateCollectionName ).idStr() );
      Edition  edition ( entry.subFind( sat::SolvAttr::updateCollectionEvr ).idStr() );
      Arch     arch    ( entry.subFind( sat::SolvAttr::updateCollectionArch ).idStr() );
      if ( name.empty() )
      {
        WAR << "Ignore malformed updateCollection entry: " << name << "-" << edition << "." << arch << endl;
        continue;
      }

      // The entry is relevant if there is an installed
      // package with the same name and arch.
      bool relevant = false;
      sat::WhatProvides providers( (Capability( name.id() )) );
      for_( it, providers.begin(), providers.end() )
      {
        if ( it->isSystem() && it->ident() == name && it->arch() == arch )
        {
          relevant = true;
          break;
        }
      }
      if ( ! relevant )
      {
        // DBG << "Not relevant: " << name << "-" << edition << "." << arch << endl;
        continue;
      }

#warning definition of patch contents is poor - needs review
      /* find exact providers first (this matches the _real_ 'collection content' of the patch */
      providers = sat::WhatProvides( Capability( arch, name.c_str(), Rel::EQ, edition, ResKind::package ) );
      if ( providers.empty() )
      {
        /* no exact providers: find 'best' providers: those with a larger evr */
        providers = sat::WhatProvides( Capability( arch, name.c_str(), Rel::GT, edition, ResKind::package ) );
        if ( providers.empty() )
        {
          // Hmm, this patch is not installable, no one is providing the package in the collection
          // FIXME: raise execption ? fake a solvable ?
          WAR << "Missing provider: " << name << "-" << edition << "." << arch << endl;
          continue;
        }
      }

      // FIXME ?! loop over providers and try to find installed ones ?
      // DBG << "Found " << name << "-" << edition << "." << arch << ": " << *(providers.begin()) << endl;
      result.get().insert( *(providers.begin()) );
    }

    return result;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Patch::ReferenceIterator
  //
  ///////////////////////////////////////////////////////////////////

  Patch::ReferenceIterator::ReferenceIterator( const sat::Solvable & val_r )
  { base_reference() = sat::LookupAttr( sat::SolvAttr::updateReference, val_r ).begin(); }

  std::string Patch::ReferenceIterator::id() const
  { return base_reference().subFind( sat::SolvAttr::updateReferenceId ).asString(); }
  std::string Patch::ReferenceIterator::href() const
  { return base_reference().subFind( sat::SolvAttr::updateReferenceHref ).asString(); }
  std::string Patch::ReferenceIterator::title() const
  { return base_reference().subFind( sat::SolvAttr::updateReferenceTitle ).asString(); }
  std::string Patch::ReferenceIterator::type() const
  { return base_reference().subFind( sat::SolvAttr::updateReferenceType ).asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
