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
  virtual std::string id() const;
  /** Patch time stamp */
  virtual Date timestamp() const;
  /** Patch category (recommended, security,...) */
  virtual std::string category() const;
  /** Does the system need to reboot to finish the update process? */
  virtual bool reboot_needed() const;
  /** Does the patch affect the package manager itself? */
  virtual bool affects_pkg_manager() const;
  /** The list of all atoms building the patch */
  virtual AtomList all_atoms() const;

  /** Patch summary */
  virtual TranslatedText summary() const;
  /** Patch description */
  virtual TranslatedText description() const;

  virtual TranslatedText licenseToConfirm() const;

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
