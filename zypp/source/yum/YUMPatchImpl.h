/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMPatchImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMPATCHIMPL_H
#define ZYPP_SOURCE_YUM_YUMPATCHIMPL_H

#include "zypp/source/SourceImpl.h"
#include "zypp/detail/PatchImpl.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/source/yum/YUMSourceImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
namespace yum
{ //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : YUMPatchImpl
//
/** Class representing a patch
*/
class YUMPatchImpl : public detail::PatchImplIf
{
public:
  /** Default ctor */
  YUMPatchImpl(
    Source_Ref source_r,
    const zypp::parser::yum::YUMPatchData & parsed,
    YUMSourceImpl & srcimpl_r
  );
  /** Patch ID */
  std::string id() const;
  /** Patch time stamp */
  Date timestamp() const;
  /** Patch category (recommended, security,...) */
  std::string category() const;
  /** Does the system need to reboot to finish the update process? */
  bool reboot_needed() const;
  /** Does the patch affect the package manager itself? */
  bool affects_pkg_manager() const;
  /** Is the patch installation interactive? (does it need user input?) */
  bool interactive() const;
  /** The list of all atoms building the patch */
  AtomList all_atoms() const;
  /** The list of those atoms which have not been installed */
  AtomList not_installed_atoms() const;

  /** Patch summary */
  virtual TranslatedText summary() const;
  /** Patch description */
  virtual TranslatedText description() const;

  virtual TranslatedText licenseToConfirm() const;

// TODO check necessarity of functions below
  bool any_atom_selected() const;
  void mark_atoms_to_freshen(bool freshen);
protected:
  std::string _patch_id;
  Date _timestamp;
  TranslatedText _summary;
  TranslatedText _description;
  std::string _category;
  bool _reboot_needed;
  bool _affects_pkg_manager;
  AtomList _atoms;
  TranslatedText _license_to_confirm;
private:
  Source_Ref _source;
public:
  Source_Ref source() const;
  friend class YUMSourceImpl;
};
///////////////////////////////////////////////////////////////////
} // namespace yum
/////////////////////////////////////////////////////////////////
} // namespace source
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMPATCHIMPL_H
