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

    virtual void consumePackage( const data::RecordId &catalog_id, data::Package_Ptr ) = 0;
    virtual void consumeProduct( const data::RecordId &catalog_id, data::Product_Ptr ) = 0;
    virtual void consumePatch( const data::RecordId &catalog_id, data::Patch_Ptr ) = 0;
    virtual void consumeMessage( const data::RecordId &catalog_id, data::Message_Ptr ) = 0;
    virtual void consumeScript( const data::RecordId &catalog_id, data::Script_Ptr ) = 0;
    
    //virtual void consumeSourcePackage( const data::SrcPackage_Ptr ) = 0;
  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ResolvableDataConsumer_H
