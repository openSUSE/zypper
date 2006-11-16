/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMPatchImpl.cc
 *
*/

#include "zypp/source/yum/YUMPatchImpl.h"
#include "zypp/source/yum/YUMSourceImpl.h"
#include <zypp/CapFactory.h>
#include "zypp/parser/yum/YUMParserData.h"
#include <zypp/parser/yum/YUMParser.h>
#include "zypp/Package.h"
#include "zypp/Script.h"
#include "zypp/Message.h"
#include "zypp/base/Logger.h"


using namespace std;
using namespace zypp::detail;
using namespace zypp::parser::yum;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
namespace yum
{
///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : YUMPatchImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
 * \bug CANT BE CONSTUCTED THAT WAY ANYMORE
*/
YUMPatchImpl::YUMPatchImpl(
  Source_Ref source_r,
  const zypp::parser::yum::YUMPatchData & parsed,
  YUMSourceImpl & srcimpl_r
)
    : _source(source_r)
{
  _patch_id = parsed.patchId;
  _timestamp = str::strtonum<time_t>(parsed.timestamp);
  _category = parsed.category;
  _reboot_needed = parsed.rebootNeeded;
  _affects_pkg_manager = parsed.packageManager;
  std::string updateScript;
  _summary = parsed.summary;
  _description = parsed.description;
  _license_to_confirm = parsed.license_to_confirm;
#if 0						// not active any more, see YUMSourceImpl::createPatch()
  // now process the atoms
  CapFactory _f;
  Capability cap( _f.parse(
                    ResType::TraitsType::kind,
                    parsed.name,
                    Rel::EQ,
                    Edition(parsed.ver, parsed.rel, parsed.epoch)
                  ));
  for (std::list<shared_ptr<YUMPatchAtom> >::const_iterator it
       = parsed.atoms.begin();
       it != parsed.atoms.end();
       it++)
  {
    switch ((*it)->atomType())
    {
    case YUMPatchAtom::Package:
      {
        shared_ptr<YUMPatchPackage> package_data
        = dynamic_pointer_cast<YUMPatchPackage>(*it);
        Atom::Ptr atom = srcimpl_r.augmentPackage( _source, *package_data );
        _atoms.push_back(atom);
        break;
      }
    case YUMPatchAtom::Message:
      {
        shared_ptr<YUMPatchMessage> message_data
        = dynamic_pointer_cast<YUMPatchMessage>(*it);
        Message::Ptr message = srcimpl_r.createMessage(_source, *message_data);
        _atoms.push_back(message);
        break;
      }
    case YUMPatchAtom::Script:
      {
        shared_ptr<YUMPatchScript> script_data
        = dynamic_pointer_cast<YUMPatchScript>(*it);
        Script::Ptr script = srcimpl_r.createScript(_source, *script_data);
        _atoms.push_back(script);
        break;
      }
    default:
      ERR << "Unknown type of atom" << endl;
    }
    for (AtomList::iterator it = _atoms.begin();
         it != _atoms.end();
         it++)
    {
      (*it)->injectRequires(cap);
    }

  }
#endif
}

std::string YUMPatchImpl::id() const
{
  return _patch_id;
}
Date YUMPatchImpl::timestamp() const
{
  return _timestamp;
}

TranslatedText YUMPatchImpl::summary() const
{
  return _summary;
}

TranslatedText YUMPatchImpl::description() const
{
  return _description;
}

TranslatedText YUMPatchImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

std::string YUMPatchImpl::category() const
{
  return _category;
}

bool YUMPatchImpl::reboot_needed() const
{
  return _reboot_needed;
}

bool YUMPatchImpl::affects_pkg_manager() const
{
  return _affects_pkg_manager;
}

YUMPatchImpl::AtomList YUMPatchImpl::all_atoms() const
{
  return _atoms;
}

Source_Ref YUMPatchImpl::source() const
{
  return _source;
}

} // namespace yum
/////////////////////////////////////////////////////////////////
} // namespace source
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
