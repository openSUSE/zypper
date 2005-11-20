#include <iostream>
#include <string>
#include <map>
#include "main.h"

using std::string;
using std::map;

struct Resolvable::Impl
{
  string _kind;
};

Resolvable::Resolvable( const string kind_r /*NVRA*/ )
: _impl( new Impl )
{
  _impl->_kind = kind_r;
}
Resolvable::~Resolvable()
{}
std::ostream & Resolvable::dumpOn( std::ostream & str ) const
{
  return str << '[' << kind() << ']' << name();
}

string Resolvable::kind() const { return _impl->_kind; }
string Resolvable::name() const { return "n"; }


Object::Object( const string kind_r /*NVRA*/ )
: Resolvable( kind_r )
{}
Object::~Object()
{}

string       Object::summary()         const { return impl().summary(); }
list<string> Object::description()     const { return impl().description(); }


Package::Package( /*NVRA*/ )
: Object( "Package" /*NVRA*/ )
{}
Package::~Package()
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


