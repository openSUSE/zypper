#ifndef JK_DERIVED_H_
#define JK_DERIVED_H_

#include "zypp/base/PtrTypes.h"
#include "Base.h"

namespace jk
{


  class Derived : public Base
  {
  public:
    Derived();
    ~Derived();

  private:
    class Impl;
    zypp::RW_pointer<Impl,zypp::rw_pointer::Scoped<Impl> > _pimpl;
  };



} // ns jk

#endif /*JK_DERIVED_H_*/
