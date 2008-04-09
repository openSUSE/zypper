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
#warning FILL contents
    return Contents();
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
