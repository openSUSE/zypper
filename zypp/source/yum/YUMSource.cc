/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMSource.cc
 *
*/

#include "zypp/source/yum/YUMSource.h"
#include "zypp/source/yum/YUMPackageImpl.h"
#include "zypp/source/yum/YUMScriptImpl.h"
#include "zypp/source/yum/YUMMessageImpl.h"
#include "zypp/source/yum/YUMPatchImpl.h"

#include <zypp/CapFactory.h>

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
      //        CLASS NAME : YUMSource
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
      */
        YUMSource::YUMSource()
	{}


	Package::Ptr YUMSource::createPackage(
	  const zypp::parser::yum::YUMPatchPackage & parsed
	)
	{
	  shared_ptr<YUMPackageImpl> impl(new YUMPackageImpl(parsed));
	  Dependencies _deps = createDependencies(parsed);
	  Package::Ptr package = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel ),
	    Arch( parsed.arch ),
	    impl
	  );
	  package->setDeps(_deps);
	  return package;
	}
	Message::Ptr YUMSource::createMessage(
	  const zypp::parser::yum::YUMPatchMessage & parsed
	)
	{
	  shared_ptr<YUMMessageImpl> impl(new YUMMessageImpl(parsed));
	  Dependencies _deps = createDependencies(parsed);
	  Message::Ptr message = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel ),
	    Arch( "noarch" ),
	    impl
	  );
	  message->setDeps(_deps);
	  return message;
	}

	Script::Ptr YUMSource::createScript(
	  const zypp::parser::yum::YUMPatchScript & parsed
	)
	{
	  shared_ptr<YUMScriptImpl> impl(new YUMScriptImpl(parsed));
	  Dependencies _deps = createDependencies(parsed);
	  Script::Ptr script = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel ),
	    Arch( "noarch" ),
	    impl
	  );
	  script->setDeps(_deps);
	  return script;
	}

	Patch::Ptr YUMSource::createPatch(
	  const zypp::parser::yum::YUMPatchData & parsed
	)
	{
	  shared_ptr<YUMPatchImpl> impl(new YUMPatchImpl(parsed, this));
//	  Dependencies _deps = createDependencies(parsed);
	  Patch::Ptr patch = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel ),
	    Arch( "noarch" ),
	    impl
	  );
//	  patch->setDeps(_deps);
	  return patch;
	}

	Dependencies YUMSource::createDependencies(
	  const zypp::parser::yum::YUMPatchAtom & parsed
	)
	{
	  CapFactory _f;
	  CapSet _provides;
	  CapSet _conflicts;
	  CapSet _obsoletes;
	  CapSet _freshens;
	  CapSet _requires;
	  CapSet _prerequires;
	  Dependencies _deps;
	  for (std::list<YUMDependency>::const_iterator it = parsed.provides.begin();
	       it != parsed.provides.end();
	       it++)
	  {
	    // FIXME do not create the string this way
	    // FIXME other types than only packages
	    // FIXME use also the flags
	    _provides.insert (_f.parse(it->name + " = " + it->ver + "-" + it->rel));
	  }
  
	  for (std::list<YUMDependency>::const_iterator it = parsed.conflicts.begin();
	       it != parsed.conflicts.end();
	       it++)
	  {
	    // FIXME do not create the string this way
	    // FIXME other types than only packages
	    // FIXME use also the flags
	    _conflicts.insert (_f.parse(it->name + " = " + it->ver + "-" + it->rel));
	  }
  
	  for (std::list<YUMDependency>::const_iterator it = parsed.obsoletes.begin();
	       it != parsed.obsoletes.end();
	       it++)
	  {
	    // FIXME do not create the string this way
	    // FIXME other types than only packages
	    // FIXME use also the flags
	    _obsoletes.insert (_f.parse(it->name + " = " + it->ver + "-" + it->rel));
	  }
  
	  for (std::list<YUMDependency>::const_iterator it = parsed.freshen.begin();
	       it != parsed.freshen.end();
	       it++)
	  {
	    // FIXME do not create the string this way
	    // FIXME other types than only packages
	    // FIXME use also the flags
	    _freshens.insert (_f.parse(it->name + " = " + it->ver + "-" + it->rel));
	  }
  
	  for (std::list<YUMDependency>::const_iterator it = parsed.requires.begin();
	       it != parsed.requires.end();
	       it++)
	  {
	    // FIXME do not create the string this way
	    // FIXME other types than only packages
	    // FIXME use also the flags
	    if (it->pre == "1")
	      _prerequires.insert (_f.parse(it->name + " = " + it->ver + "-" + it->rel));
	    else
	      _requires.insert (_f.parse(it->name + " = " + it->ver + "-" + it->rel));
	  }
  
	  _deps.setProvides(_provides);
	  _deps.setConflicts(_conflicts);
	  _deps.setObsoletes(_obsoletes);
	  _deps.setFreshens(_freshens);
	  _deps.setRequires(_requires);
	  _deps.setPrerequires(_prerequires);
	  return _deps;
	}
  



    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
