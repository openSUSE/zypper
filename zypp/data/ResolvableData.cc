/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/data/ResolvableData.h"

using namespace std;

namespace zypp
{
namespace data
{

IMPL_PTR_TYPE(Resolvable);
IMPL_PTR_TYPE(ResObject);
IMPL_PTR_TYPE(Script);
IMPL_PTR_TYPE(Message);
IMPL_PTR_TYPE(Atom);
IMPL_PTR_TYPE(Patch);
IMPL_PTR_TYPE(Pattern);
IMPL_PTR_TYPE(Product);
IMPL_PTR_TYPE(Packagebase);
IMPL_PTR_TYPE(Package);
IMPL_PTR_TYPE(SrcPackage);

IMPL_PTR_TYPE(DeltaRpm);
IMPL_PTR_TYPE(PatchRpm);
IMPL_PTR_TYPE(PackageAtom);
IMPL_PTR_TYPE(BaseVersion);


std::ostream & ResObject::dumpOn( std::ostream & str ) const
{
  str << "[ " << name << " " << edition << " " << arch << " ]";
  return str;
//       << "  provides: " << provides << endl
//       << "  conflicts: " << conflicts << endl
//       << "  obsoletes: " << obsoletes << endl
//       << "  freshens: " << freshens << endl
//       << "  requires: " << requires << endl
//       << "  recommends:" << endl << recommends << endl
//       << "  suggests:" << endl << suggests << endl
//       << "  supplements:" << endl << supplements << endl
//       << "  enhances:" << endl << enhances << endl
}


std::ostream & RpmBase::dumpOn( std::ostream & str ) const
{
  str << "Patch/Delta[ " << name << " " << edition << " " << arch << " ]";
  return str;
}

} // namespace cache
} // namespace zypp
