#include "BaseImpl.h"
#include "Derived.h"

namespace jk
{


  class Derived::Impl : public BaseImpl
  {
  public:
    Impl();
  };

  Derived::Impl::Impl()
  {}


  Derived::Derived() : _pimpl(new Impl())
  {}

  Derived::~Derived()
  {}


} // ns jk

// vim: set ts=2 sts=2 sw=2 et ai:

