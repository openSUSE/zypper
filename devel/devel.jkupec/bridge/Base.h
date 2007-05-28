#ifndef JK_BASE_H_
#define JK_BASE_H_

#include "zypp/base/NonCopyable.h"

namespace jk
{


  class Base : private zypp::base::NonCopyable
  {
  protected:
    class BaseImpl;
  };


} // ns jk


#endif /*JK_BASE_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:

