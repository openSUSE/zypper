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

  std::string Patch::category() const
  { return lookupStrAttribute( sat::SolvAttr::patchcategory ); }

  Patch::Category Patch::categoryEnum() const
  { return categoryEnum( category() ); }

  bool Patch::isCategory( const std::string & category_r ) const
  { return( str::compareCI( category_r, category() ) == 0 ); }

  bool Patch::isCategory( Categories category_r ) const
  { return category_r.testFlag( categoryEnum() ); }

  Patch::Category Patch::categoryEnum( const std::string & category_r )
  {
    switch ( category_r[0] )
    {
      //	CAT_YAST
      case 'y':
      case 'Y':
	if ( str::compareCI( category_r, "yast" ) == 0 )
	  return CAT_YAST;
	break;

      //	CAT_SECURITY
      case 's':
      case 'S':
	if ( str::compareCI( category_r, "security" ) == 0 )
	  return CAT_SECURITY;
	break;

      //	CAT_RECOMMENDED
      case 'r':
      case 'R':
	if ( str::compareCI( category_r, "recommended" ) == 0 )
	  return CAT_RECOMMENDED;
	break;
      case 'b':
      case 'B':
	if ( str::compareCI( category_r, "bugfix" ) == 0 )	// rhn
	  return CAT_RECOMMENDED;
	break;

      //	CAT_OPTIONAL
      case 'o':
      case 'O':
	if ( str::compareCI( category_r, "optional" ) == 0 )
	  return CAT_OPTIONAL;
	break;
      case 'f':
      case 'F':
	if ( str::compareCI( category_r, "feature" ) == 0 )
	  return CAT_OPTIONAL;
	break;
      case 'e':
      case 'E':
	if ( str::compareCI( category_r, "enhancement" ) == 0 )	// rhn
	  return CAT_OPTIONAL;
	break;

      //	CAT_DOCUMENT
      case 'd':
      case 'D':
	if ( str::compareCI( category_r, "document" ) == 0 )
	  return CAT_DOCUMENT;
	break;
    }
    // default:
    INT << "Unrecognized Patch::Category string '" << category_r << "'" << endl;
    return CAT_OTHER;
  }

  std::string asString( const Patch::Category & obj )
  {
    switch ( obj )
    {
      case Patch::CAT_OTHER:		return std::string( "other" );		break;
      case Patch::CAT_YAST:		return std::string( "yast" );		break;
      case Patch::CAT_SECURITY:		return std::string( "security" );	break;
      case Patch::CAT_RECOMMENDED:	return std::string( "recommended" );	break;
      case Patch::CAT_OPTIONAL:		return std::string( "optional" );	break;
      case Patch::CAT_DOCUMENT:		return std::string( "document" );	break;
    }
    // make gcc happy:
    return std::string( "other" );
  }

  ///////////////////////////////////////////////////////////////////

  std::string Patch::severity() const
  { return lookupStrAttribute( sat::SolvAttr::severity ); }

  Patch::SeverityFlag Patch::severityFlag() const
  { return severityFlag( severity() ); }

  bool Patch::isSeverity( const std::string & severity_r ) const
  { return( str::compareCI( severity_r, severity() ) == 0 ); }

  bool Patch::isSeverity( SeverityFlags severity_r ) const
  { return severity_r.testFlag( severityFlag() ); }

  Patch::SeverityFlag Patch::severityFlag( const std::string & severity_r )
  {
    switch ( severity_r[0] )
    {
      case 'l':
      case 'L':
	if ( str::compareCI( severity_r, "low" ) == 0 )
	  return SEV_LOW;
	break;

      case 'm':
      case 'M':
	if ( str::compareCI( severity_r, "moderate" ) == 0 )
	  return SEV_MODERATE;
	break;

      case 'i':
      case 'I':
	if ( str::compareCI( severity_r, "important" ) == 0 )
	  return SEV_IMPORTANT;
	break;

      case 'c':
      case 'C':
	if ( str::compareCI( severity_r, "critical" ) == 0 )
	  return SEV_CRITICAL;
	break;

      case 'u':
      case 'U':
	if ( str::compareCI( severity_r, "unspecified" ) == 0 )
	  return SEV_NONE;
	break;

      case '\0':
	return SEV_NONE;
	break;
    }
    // default:
    INT << "Unrecognized Patch::Severity string '" << severity_r << "'" << endl;
    return SEV_OTHER;
  }

  std::string asString( const Patch::SeverityFlag & obj )
  {
    switch ( obj )
    {
      case Patch::SEV_OTHER:	return std::string( "unknown" );	break;
      case Patch::SEV_NONE:	return std::string( "unspecified" );	break;
      case Patch::SEV_LOW:	return std::string( "low" );		break;
      case Patch::SEV_MODERATE:	return std::string( "moderate" );	break;
      case Patch::SEV_IMPORTANT:return std::string( "important" );	break;
      case Patch::SEV_CRITICAL:	return std::string( "critical" );	break;
    }
    // make gcc happy:
    return std::string( "unknown" );
  }

  ///////////////////////////////////////////////////////////////////

  std::string Patch::message( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::message, lang_r ); }

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
