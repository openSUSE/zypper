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


#include "zypp/data/RecordId.h"
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

    virtual void consumePackage( const data::RecordId &catalog_id, const data::Package &package) = 0;
    virtual void consumePackage( const data::Pattern &package) = 0;
    virtual void consumePackage( const data::Patch &package) = 0;
    virtual void consumePackage( const data::Script &package) = 0;
    virtual void consumePackage( const data::Message &package) = 0;
    virtual void consumePackage( const data::Product &package) = 0;
    virtual void consumePackage( const data::SrcPackage &package) = 0;
  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ResolvableDataConsumer_H
