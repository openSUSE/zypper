#ifndef ZYPP_NG_BASE_PRIVATE_BASE_P_H_INCLUDED
#define ZYPP_NG_BASE_PRIVATE_BASE_P_H_INCLUDED

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/base.h>
#include <zypp/zyppng/base/signals.h>
#include <unordered_set>
#include <thread>

namespace zyppng
{

  class BasePrivate : public sigc::trackable
  {
    ZYPP_DECLARE_PUBLIC(Base)
  public:
    virtual ~BasePrivate();
    Base::WeakPtr parent;
    std::unordered_set< Base::Ptr > children;
    Base *z_ptr = nullptr;
    std::thread::id threadId = std::this_thread::get_id();
  };

}


#endif
