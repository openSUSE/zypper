/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
* Based on code by Ivan Čukić (BSD/MIT licensed) from the functional cpp book
*/

#ifndef ZYPP_ZYPPNG_MONADIC_MTRY_H
#define ZYPP_ZYPPNG_MONADIC_MTRY_H

#include "expected.h"

namespace zyppng {

  template < typename F
           , typename Ret = typename std::result_of<F()>::type
           , typename Exp = expected<Ret, std::exception_ptr>
           >
  Exp mtry(F f)
  {
      try {
          return Exp::success(f());
      } catch (...) {
          return Exp::error(std::current_exception());
      }
  }

}

#endif /* !MTRY_H */
