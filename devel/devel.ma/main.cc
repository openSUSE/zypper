#include <iostream>
#include <string>
#include <map>
#include "main.h"

using std::string;
using std::map;

{}

string Package::packagedata() const { return impl().packagedata(); }

Selection::Selection( /*NVRA*/ )
: Object( "Selection" /*NVRA*/ )
{}
Selection::~Selection()
{}

string Selection::selectiondata() const { return impl().selectiondata(); }

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////


