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
#warning DISBALED INCLUDE BELOW AS IT DOES NOT COMPILE
//#include <zypp/parser/yum/YUMParser.h>
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
	CapFactory _f;
#warning ORIGINAL CODE DISABLED AS IT DOES NOT COMPILE
	Capability cap( _f.parse(
	  parsed.name, Resolvable::Kind("Patch")
	  ) );
#if 0
	Capability cap( _f.parse(
	  Resolvable::Kind("Patch"),
	  parsed.name,
	  Edition(),
	  Arch("noarch")));
#endif
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
	    Dependencies _deps = (*it)->deps();
	    CapSet _req = _deps.requires();
	    _req.insert(cap);
	    _deps.setRequires(_req);
	    (*it)->setDeps(_deps);
	  }

	}
      }


    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
