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
	  const zypp::parser::yum::YUMPrimaryData & parsed,
	  const zypp::parser::yum::YUMFileListData & filelist,
	  const zypp::parser::yum::YUMOtherData & other
	)
	{
	  try
	  {
	    shared_ptr<YUMPackageImpl> impl(
	      new YUMPackageImpl(parsed, filelist, other));
	    Dependencies _deps = createDependencies(parsed,
	  					  ResTraits<Package>::kind);
	    Package::Ptr package = detail::makeResolvableFromImpl(
	      parsed.name,
	      Edition( parsed.ver, parsed.rel, parsed.epoch ),
	      Arch( parsed.arch ),
	      impl
	    );
	    package->setDeps(_deps);
	    return package;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create package object";
	  }
	}

	Package::Ptr YUMSource::createPackage(
	  const zypp::parser::yum::YUMPatchPackage & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMPackageImpl> impl(new YUMPackageImpl(parsed));
	    Dependencies _deps = createDependencies(parsed,
	  					  ResTraits<Package>::kind);
	    Package::Ptr package = detail::makeResolvableFromImpl(
	      parsed.name,
	      Edition( parsed.ver, parsed.rel, parsed.epoch ),
	      Arch( parsed.arch ),
	      impl
	    );
	    package->setDeps(_deps);
	    return package;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create package object";
	  }
	}

	Message::Ptr YUMSource::createMessage(
	  const zypp::parser::yum::YUMPatchMessage & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMMessageImpl> impl(new YUMMessageImpl(parsed));
	    Dependencies _deps = createDependencies(parsed,
						    ResTraits<Message>::kind);
	    Message::Ptr message = detail::makeResolvableFromImpl(
	      parsed.name,
	      Edition( parsed.ver, parsed.rel, parsed.epoch ),
	      Arch_noarch,
	      impl
	    );
	    message->setDeps(_deps);
	    return message;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create message object";
	  }
	}

	Script::Ptr YUMSource::createScript(
	  const zypp::parser::yum::YUMPatchScript & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMScriptImpl> impl(new YUMScriptImpl(parsed));
	    Dependencies _deps = createDependencies(parsed,
						    ResTraits<Script>::kind);
	    Script::Ptr script = detail::makeResolvableFromImpl(
	      parsed.name,
	      Edition( parsed.ver, parsed.rel, parsed.epoch ),
	      Arch_noarch,
	      impl
	    );
	    script->setDeps(_deps);
	    return script;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create script object";
	  }
	}

	Product::Ptr YUMSource::createProduct(
	  const zypp::parser::yum::YUMProductData & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMProductImpl> impl(new YUMProductImpl(parsed, this));
	    Dependencies _deps = createDependencies(parsed,
						    ResTraits<Product>::kind);
	    Product::Ptr product = detail::makeResolvableFromImpl(
	      parsed.name,
	      Edition( parsed.ver, parsed.rel, parsed.epoch ),
	      Arch_noarch,
	      impl
	    );
	    product->setDeps(_deps);
	    return product;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create product object";
	  }
	}

	Patch::Ptr YUMSource::createPatch(
	  const zypp::parser::yum::YUMPatchData & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMPatchImpl> impl(new YUMPatchImpl(parsed, this));
	    Dependencies _deps = createDependencies(parsed,
						    ResTraits<Patch>::kind);
	    Patch::Ptr patch = detail::makeResolvableFromImpl(
	      parsed.name,
	      Edition( parsed.ver, parsed.rel, parsed.epoch ),
	      Arch_noarch,
	      impl
	    );
	    patch->setDeps(_deps);
	    return patch;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create patch object";
	  }
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
