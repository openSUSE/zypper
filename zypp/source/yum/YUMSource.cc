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
#include "zypp/source/yum/YUMProductImpl.h"

#include "zypp/base/Logger.h"
#include "zypp/CapFactory.h"

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
	  Dependencies _deps = createDependencies(parsed,
						  ResTraits<Package>::_kind);
	  Package::Ptr package = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel, parsed.epoch ),
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
	  Dependencies _deps = createDependencies(parsed,
						  ResTraits<Message>::_kind);
	  Message::Ptr message = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel, parsed.epoch ),
	    Arch_noarch,
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
	  Dependencies _deps = createDependencies(parsed,
						  ResTraits<Script>::_kind);
	  Script::Ptr script = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel, parsed.epoch ),
	    Arch_noarch,
	    impl
	  );
	  script->setDeps(_deps);
	  return script;
	}

	Product::Ptr YUMSource::createProduct(
	  const zypp::parser::yum::YUMProductData & parsed
	)
	{
	  shared_ptr<YUMProductImpl> impl(new YUMProductImpl(parsed, this));
	  Dependencies _deps = createDependencies(parsed,
						  ResTraits<Product>::_kind);
	  Product::Ptr product = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel, parsed.epoch ),
	    Arch_noarch,
	    impl
	  );
	  product->setDeps(_deps);
	  return product;
	}

	Patch::Ptr YUMSource::createPatch(
	  const zypp::parser::yum::YUMPatchData & parsed
	)
	{
	  shared_ptr<YUMPatchImpl> impl(new YUMPatchImpl(parsed, this));
	  Dependencies _deps = createDependencies(parsed,
						  ResTraits<Patch>::_kind);
	  Patch::Ptr patch = detail::makeResolvableFromImpl(
	    parsed.name,
	    Edition( parsed.ver, parsed.rel, parsed.epoch ),
	    Arch_noarch,
	    impl
	  );
	  patch->setDeps(_deps);
	  return patch;
	}

	Dependencies YUMSource::createDependencies(
	  const zypp::parser::yum::YUMObjectData & parsed,
	  const Resolvable::Kind my_kind
	)
	{
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
	    _provides.insert(createCapability(*it, my_kind));
	  }
  
	  for (std::list<YUMDependency>::const_iterator it = parsed.conflicts.begin();
	       it != parsed.conflicts.end();
	       it++)
	  {
	    _conflicts.insert(createCapability(*it, my_kind));
	  }
  
	  for (std::list<YUMDependency>::const_iterator it = parsed.obsoletes.begin();
	       it != parsed.obsoletes.end();
	       it++)
	  {
	    _obsoletes.insert(createCapability(*it, my_kind));
	  }
  
	  for (std::list<YUMDependency>::const_iterator it = parsed.freshen.begin();
	       it != parsed.freshen.end();
	       it++)
	  {
	    _freshens.insert(createCapability(*it, my_kind));
	  }
  
	  for (std::list<YUMDependency>::const_iterator it = parsed.requires.begin();
	       it != parsed.requires.end();
	       it++)
	  {
	    if (it->pre == "1")
	      _prerequires.insert(createCapability(*it, my_kind));
	    else
	      _requires.insert(createCapability(*it, my_kind));
	  }
  
	  _deps.setProvides(_provides);
	  _deps.setConflicts(_conflicts);
	  _deps.setObsoletes(_obsoletes);
	  _deps.setFreshens(_freshens);
	  _deps.setRequires(_requires);
	  _deps.setPrerequires(_prerequires);
	  return _deps;
	}

	Capability YUMSource::createCapability(const YUMDependency & dep,
					       const Resolvable::Kind & my_kind)
	{
	  CapFactory _f;
	  Resolvable::Kind _kind = dep.kind == ""
	    ? my_kind
	    : Resolvable::Kind(dep.kind);
	  Capability cap = _f.parse(
	    _kind,
	    dep.name,
	    Rel(dep.flags),
	    Edition(dep.ver, dep.rel, dep.epoch)
	  );
	  return cap;
	}
  
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
