/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_ResolvableDataConsumer_H
#define ZYPP_ResolvableDataConsumer_H



#include "zypp/data/ResolvableData.h"


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace data
{ /////////////////////////////////////////////////////////////////

  class ResolvableDataConsumer
  {
    public:

    ResolvableDataConsumer();
    virtual ~ResolvableDataConsumer();

    virtual void consumePackage( const data::Package &package) = 0;
   
  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ResolvableDataConsumer_H
