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
#include "zypp/source/yum/YUMSource.h"
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
	const zypp::parser::yum::YUMPatchData & parsed,
	YUMSource * src
      )
      {
	_patch_id = parsed.patchId;
        _timestamp = atol(parsed.timestamp.c_str());
        _category = parsed.category;
        _reboot_needed = parsed.rebootNeeded;
        _affects_pkg_manager = parsed.packageManager;
        std::string updateScript;
/*
        std::string engine;
        MultiLang description;
        _summary = parsed.MultiLang summary;
*/




      /** Patch description */
//      std::list<std::string> _description;





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
            case YUMPatchAtom::Package: {
              shared_ptr<YUMPatchPackage> package_data
                = dynamic_pointer_cast<YUMPatchPackage>(*it);
              Package::Ptr package = src->createPackage(*package_data);
              _atoms.push_back(package);
              break;
            }
            case YUMPatchAtom::Message: {
              shared_ptr<YUMPatchMessage> message_data
                = dynamic_pointer_cast<YUMPatchMessage>(*it);
              Message::Ptr message = src->createMessage(*message_data);
              _atoms.push_back(message);
              break;
            }
            case YUMPatchAtom::Script: {
              shared_ptr<YUMPatchScript> script_data
                = dynamic_pointer_cast<YUMPatchScript>(*it);
              Script::Ptr script = src->createScript(*script_data);
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
      }

      std::string YUMPatchImpl::id() const
      {
        return _patch_id;
      }
      unsigned int YUMPatchImpl::timestamp() const
      {
        return _timestamp;
      }

      Label YUMPatchImpl::summary() const
      { return _summary; }

      Text YUMPatchImpl::description() const
      { return _description; }

      Text YUMPatchImpl::insnotify() const
      { return ResObjectImplIf::insnotify(); }

      Text YUMPatchImpl::delnotify() const
      { return ResObjectImplIf::delnotify(); }

      ByteCount YUMPatchImpl::size() const
      { return ResObjectImplIf::size(); }

      bool YUMPatchImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label YUMPatchImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }

      Vendor YUMPatchImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }

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

      bool YUMPatchImpl::interactive() {
        if (_reboot_needed)
        {
          DBG << "Patch needs reboot" << endl;
          return true;
        }
        AtomList not_installed = not_installed_atoms();
        for (AtomList::iterator it = not_installed.begin();
          it != not_installed.end();
          it++)
        {
          if ((*it)->kind() == "Message")
          {
  //          DBG << "Patch contains a message" << endl;
            return true;
          }
          if ((*it)->kind() == "Package")
          {
                                   // Resolvable*
                                    // Resolvable
                                     // ResolvablePtr


            // <ma> never do somthing like this!!!
  //          Package* p = (Package*)&**it;
            //
            // (*it) is a ResolvablePtr




            // FIXME use the condition
  //          if (p->licenseToConfirm() != "")
            if (false)
            {
  //            DBG << "Package has a license to be shown to user" << endl;
              return true;
            }
          }
        }
        return false;
      }

      YUMPatchImpl::AtomList YUMPatchImpl::all_atoms() {
        return _atoms;
      }

      YUMPatchImpl::AtomList YUMPatchImpl::not_installed_atoms() {
        AtomList ret;
        for (AtomList::iterator it = _atoms.begin();
          it != _atoms.end();
          it++)
        {
          if (true) // FIXME check if atom/resolvable is not installed
          {
            ret.push_back(*it);
          }
        }
        return ret;
      }

  // TODO check necessarity of functions below

      bool YUMPatchImpl::any_atom_selected() {
        for (AtomList::iterator it = _atoms.begin();
          it != _atoms.end();
          it++)
        {
          if (false) // FIXME check if atom/resolvable is selected
          {
            return true;
          }
        }
        return false;
      }

      void YUMPatchImpl::mark_atoms_to_freshen( bool freshen ) {
        for (AtomList::iterator it = _atoms.begin();
          it != _atoms.end();
          it++)
        {
          // TODO mark the resolvable to be or not to be freshed
        }
      }

    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
