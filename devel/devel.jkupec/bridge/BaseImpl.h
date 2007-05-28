#ifndef JK_BASEIMPL_H_
#define JK_BASEIMPL_H_

#include "zypp/base/NonCopyable.h"
#include "Base.h"

namespace jk
{


  class Base::BaseImpl : private zypp::base::NonCopyable
  {
  public:
    BaseImpl();
  };


} // ns jk

#endif /*JK_BASEIMPL_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:

