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


#include "zypp/DiskUsage.h"
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

    virtual data::RecordId consumePackage      ( const data::Package_Ptr & ) = 0;
    virtual data::RecordId consumeSourcePackage( const data::SrcPackage_Ptr & ) = 0;
    virtual data::RecordId consumeProduct      ( const data::Product_Ptr & ) = 0;
    virtual data::RecordId consumePatch        ( const data::Patch_Ptr & ) = 0;
    virtual data::RecordId consumePattern      ( const data::Pattern_Ptr & ) = 0;

    virtual data::RecordId consumeChangelog    ( const data::RecordId & resolvable_id, const Changelog & ) = 0;
    virtual data::RecordId consumeFilelist     ( const data::RecordId & resolvable_id, const data::Filenames & ) = 0;
    virtual void consumeDiskUsage              ( const data::RecordId &resolvable_id, const DiskUsage &disk ) = 0;

    virtual void updatePackageLang( const data::RecordId & resolvable_id, const data::Packagebase_Ptr & data_r ) = 0;

  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ResolvableDataConsumer_H
